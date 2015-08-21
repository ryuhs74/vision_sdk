/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file sgxDisplayLink_tsk.c
 *
 * \brief  This file has the implementation of SgxDisplay Link Init and Run API
 *
 *         This file implements the state machine logic for this link.
 *         SgxDisplayLink_init() get calls from system_init and the same create
 *         the link task and basic messaging interfaces. Once the link is
 *         initiated it waits for the create cmd. This create cmd creates the
 *         complete link infrastructure.  Then waits for various data and
 *         control cmds.
 *
 *         This file also implements the sgxDisplay link tear down functionality
 *
 * \version 0.0 (Jun 2014) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "sgxDisplayLink_priv.h"

/**
 *******************************************************************************
 * \brief SgxDisplay Link object, stores all link related information
 *******************************************************************************
 */
SgxDisplayLink_Obj gSgxDisplayLink_obj[SGXDISPLAY_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief This function implements the sgxDisplay link Run/Steady state
 *
 *        In this state link gets commands to
 *         - Stop/delete of link
 *         - Data events/cmds
 *         - All dynamic cmds that the link supports
 *         - All stats/status cmds
 *
 * \param  pObj     [IN]  SgxDisplay link instance handle
 * \param  pTsk     [IN]  Link Task Handle
 * \param  pMsg     [IN]  Message Handle
 * \param  done     [IN]  sgxDisplay link Run state, set to TRUE once it get the
 *                        is DELETE CMD
 * \param  ackMsg   [OUT] ACK message
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_tskRun(SgxDisplayLink_Obj *pObj, OSA_TskHndl *pTsk,
                            OSA_MsgHndl **pMsg, Bool *done, Bool *ackMsg)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool runDone = FALSE, runAckMsg = FALSE;
    OSA_MsgHndl *pRunMsg;
    UInt32 cmd;

    *done = FALSE;
    *ackMsg = FALSE;

    runDone = FALSE;
    runAckMsg = FALSE;

    *pMsg = NULL;

    /*
     * This while loop implements RUN state. All the run time commands for
     * ackMsg Link are received and serviced in this while loop.
     * Control remains in this loop until delete commands arrives.
     */
    while (!runDone)
    {
        status = OSA_tskWaitMsg(pTsk, &pRunMsg);
        if (status != OSA_SOK)
            break;

        cmd = OSA_msgGetCmd(pRunMsg);

        /*
         * Different commands are serviced via this switch case. For each
         * command, after servicing, ACK or free message is sent before
         * proceeding to next state.
         */
        switch (cmd)
        {
            case SYSTEM_CMD_STOP:
                runDone = TRUE;
                runAckMsg = TRUE;
                break;

            case SYSTEM_CMD_NEW_DATA:
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                status = SgxDisplayLink_drvDoProcessFrames(pObj);
                break;

            case SYSTEM_CMD_DELETE:
                *done = TRUE;
                *ackMsg = TRUE;
                *pMsg = pRunMsg;
                runDone = TRUE;
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                SgxDisplayLink_drvPrintStatistics(pObj);
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            default:
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }
    }

    SgxDisplayLink_drvStop(pObj);

    if (runAckMsg)
        OSA_tskAckOrFreeMsg(pRunMsg, status);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the START/RUN state of sgxDisplay Link.
 *
 *        In this state link gets commands to
 *         - Create the SgxDisplay Driver
 *         - Start the SgxDisplay Driver
 *         - Moves to RUN state
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_tskMain(struct OSA_TskHndl *pTsk,
                                    OSA_MsgHndl *pMsg, UInt32 curState)
{
    UInt32 cmd = OSA_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    SgxDisplayLink_Obj *pObj = (SgxDisplayLink_Obj *) pTsk->appData;

    /*
     * At this stage only create command is the expected command.
     * If other message gets received Ack with error status
     */
    if (cmd != SYSTEM_CMD_CREATE)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
        return OSA_EFAIL;
    }

    /*
     * Create command received, create the driver
     */
    status = SgxDisplayLink_drvCreate(pObj, OSA_msgGetPrm(pMsg));

    OSA_tskAckOrFreeMsg(pMsg, status);

    if (status != SYSTEM_LINK_STATUS_SOK)
        return OSA_EFAIL;

    done = FALSE;
    ackMsg = FALSE;

    while (!done)
    {
        status = OSA_tskWaitMsg(pTsk, &pMsg);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        cmd = OSA_msgGetCmd(pMsg);

        switch (cmd)
        {
            /*
             * CMD to prime and start the sgxDisplay driver. Link will be in
             * running state and ready to sgxDisplays the video/graphic frames
             * available at the input side
             */
            case SYSTEM_CMD_START:
                status = SgxDisplayLink_drvStart(pObj);

                OSA_tskAckOrFreeMsg(pMsg, status);

                /*
                 * Entering RUN state
                 */
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    status =
                        SgxDisplayLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);
                }

                break;
            case SYSTEM_CMD_DELETE:
                done = TRUE;
                ackMsg = TRUE;
                break;

            default:
                OSA_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    SgxDisplayLink_drvDelete(pObj);

    if (ackMsg && pMsg != NULL)
        OSA_tskAckOrFreeMsg(pMsg, status);

    return OSA_SOK;
}

/**
 *******************************************************************************
 *
 *   \brief SgxDisplay link register and init function
 *
 *          For each sgxDisplay instance (VID1, VID2, VID3 or GRPX1)
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SgxDisplayLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 sgxDisplayId;
    SgxDisplayLink_Obj *pObj;
    char tskName[32];

    for (sgxDisplayId = 0; sgxDisplayId < SGXDISPLAY_LINK_OBJ_MAX; sgxDisplayId++)
    {
        pObj = &gSgxDisplayLink_obj[sgxDisplayId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId = SYSTEM_LINK_ID_SGXDISPLAY_0 + sgxDisplayId;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = NULL;
        linkObj.linkPutEmptyBuffers = NULL;
        linkObj.getLinkInfo = NULL;

        System_registerLink(pObj->linkId, &linkObj);

        sprintf(tskName, "SGXDISPLAY%u", (unsigned int)sgxDisplayId);

        /*
         * Create link task, task remains in IDLE state.
         * SgxDisplayLink_tskMain is called when a message command is received.
         */
        status = OSA_tskCreate(&pObj->tsk,
                               SgxDisplayLink_tskMain,
                               SGXDISPLAY_LINK_TSK_PRI,
                               SGXDISPLAY_LINK_TSK_STACK_SIZE,
                               0,
                               pObj);
        OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief SgxDisplay link de-register and de-init function
 *
 *          For each sgxDisplay instance (VID1, VID2, VID3 or GRPX1)
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SgxDisplayLink_deInit()
{
    SgxDisplayLink_Obj *pObj;
    UInt32 sgxDisplayId;

    for (sgxDisplayId = 0; sgxDisplayId < SGXDISPLAY_LINK_OBJ_MAX; sgxDisplayId++)
    {
        pObj = &gSgxDisplayLink_obj[sgxDisplayId];

        OSA_tskDelete(&pObj->tsk);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
