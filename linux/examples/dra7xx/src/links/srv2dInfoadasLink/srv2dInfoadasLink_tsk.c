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
 * \file srv2DInfoAdasLink_tsk.c
 *
 * \brief  This file has the implementation of SRV2D Link Init and Run API
 *
 *         This file implements the state machine logic for this link.
 *         srv2DInfoAdasLink_init() get calls from system_init and the same create
 *         the link task and basic messaging interfaces. Once the link is
 *         initiated it waits for the create cmd. This create cmd creates the
 *         complete link infrastructure.  Then waits for various data and
 *         control cmds.
 *
 *         This file also implements the SRV2D link tear down functionality
 *
 * \version 0.0 (Sept 2014) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "srv2dInfoadasLink_priv.h"

/**
 *******************************************************************************
 * \brief SRV2D Link object, stores all link related information
 *******************************************************************************
 */
srv2dInfoAdasLink_Obj gsrv2dInfoAdasLink_obj[SRV2DINFOADAS_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief This function implements the SRV2D link Run/Steady state
 *
 *        In this state link gets commands to
 *         - Stop/delete of link
 *         - Data events/cmds
 *         - All dynamic cmds that the link supports
 *         - All stats/status cmds
 *
 * \param  pObj     [IN]  SRV2D link instance handle
 * \param  pTsk     [IN]  Link Task Handle
 * \param  pMsg     [IN]  Message Handle
 * \param  done     [IN]  SRV2D link Run state, set to TRUE once it get the
 *                        is DELETE CMD
 * \param  ackMsg   [OUT] ACK message
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_tskRun(srv2dInfoAdasLink_Obj *pObj, OSA_TskHndl *pTsk,
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
                status = srv2dInfoAdasLink_drvDoProcessFrames(pObj);
                break;

            case SYSTEM_CMD_DELETE:
                *done = TRUE;
                *ackMsg = TRUE;
                *pMsg = pRunMsg;
                runDone = TRUE;
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                //srv2dInfoAdasLink_drvPrintStatistics(pObj);
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            default:
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }
    }

    srv2dInfoAdasLink_drvStop(pObj);

    if (runAckMsg)
        OSA_tskAckOrFreeMsg(pRunMsg, status);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the START/RUN state of SRV2D Link.
 *
 *        In this state link gets commands to
 *         - Create the SRV2D Driver
 *         - Start the SRV2D Driver
 *         - Moves to RUN state
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_tskMain(struct OSA_TskHndl *pTsk,
                                    OSA_MsgHndl *pMsg, UInt32 curState)
{
    UInt32 cmd = OSA_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    srv2dInfoAdasLink_Obj *pObj = (srv2dInfoAdasLink_Obj *) pTsk->appData;

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
    status = srv2dInfoAdasLink_drvCreate(pObj, OSA_msgGetPrm(pMsg));

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
             * CMD to prime and start the SRV2D driver. Link will be in
             * running state and ready to SRV2Ds the video/graphic frames
             * available at the input side
             */
            case SYSTEM_CMD_START:
                status = srv2dInfoAdasLink_drvStart(pObj);

                OSA_tskAckOrFreeMsg(pMsg, status);

                /*
                 * Entering RUN state
                 */
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    status =
                        srv2dInfoAdasLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);
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

    srv2dInfoAdasLink_drvDelete(pObj);

    if (ackMsg && pMsg != NULL)
        OSA_tskAckOrFreeMsg(pMsg, status);

    return OSA_SOK;
}

/**
 *******************************************************************************
 *
 *   \brief SRV2D link register and init function
 *
 *          For each SRV2D instance (VID1, VID2, VID3 or GRPX1)
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 srv2dInfoAdasLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 SRV2DId;
    srv2dInfoAdasLink_Obj *pObj;
    char tskName[32];

    for (SRV2DId = 0; SRV2DId < SRV2DINFOADAS_LINK_OBJ_MAX; SRV2DId++)
    {
        pObj = &gsrv2dInfoAdasLink_obj[SRV2DId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId = SYSTEM_LINK_ID_SRV2DINFOADAS_0 + SRV2DId;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = NULL;
        linkObj.linkPutEmptyBuffers = NULL;
        linkObj.getLinkInfo = NULL;

        System_registerLink(pObj->linkId, &linkObj);

        sprintf(tskName, "SRV2D%u", (unsigned int)SRV2DId);

        /*
         * Create link task, task remains in IDLE state.
         * srv2dInfoAdasLink_tskMain is called when a message command is received.
         */
        status = OSA_tskCreate(&pObj->tsk,
                               srv2dInfoAdasLink_tskMain,
                               SRV2DINFOADAS_LINK_TSK_PRI,
                               SRV2DINFOADAS_LINK_TSK_STACK_SIZE,
                               0,
                               pObj);
        OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
    }


    return status;
}

/**
 *******************************************************************************
 *
 *   \brief SRV2D link de-register and de-init function
 *
 *          For each SRV2D instance (VID1, VID2, VID3 or GRPX1)
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 srv2dInfoAdasLink_deInit()
{
    srv2dInfoAdasLink_Obj *pObj;
    UInt32 SRV2DId;


    for (SRV2DId = 0; SRV2DId < SRV2DINFOADAS_LINK_OBJ_MAX; SRV2DId++)
    {
        pObj = &gsrv2dInfoAdasLink_obj[SRV2DId];

        OSA_tskDelete(&pObj->tsk);
    }


    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
