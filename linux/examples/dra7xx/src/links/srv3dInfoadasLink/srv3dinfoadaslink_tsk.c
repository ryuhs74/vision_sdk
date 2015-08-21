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
 * \file sgx3DsrvLink_tsk.c
 *
 * \brief  This file has the implementation of Sgx3Dsrv Link Init and Run API
 *
 *         This file implements the state machine logic for this link.
 *         Sgx3DsrvLink_init() get calls from system_init and the same create
 *         the link task and basic messaging interfaces. Once the link is
 *         initiated it waits for the create cmd. This create cmd creates the
 *         complete link infrastructure.  Then waits for various data and
 *         control cmds.
 *
 *         This file also implements the sgx3Dsrv link tear down functionality
 *
 * \version 0.0 (Sept 2014) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "srv3dinfoadaslink_priv.h"

/**
 *******************************************************************************
 * \brief Sgx3Dsrv Link object, stores all link related information
 *******************************************************************************
 */
srv3dinfoadaslink_Obj gsrv3dInfoAdasLink_obj[SRV3DINFOADAS_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief This function implements the sgx3Dsrv link Run/Steady state
 *
 *        In this state link gets commands to
 *         - Stop/delete of link
 *         - Data events/cmds
 *         - All dynamic cmds that the link supports
 *         - All stats/status cmds
 *
 * \param  pObj     [IN]  Sgx3Dsrv link instance handle
 * \param  pTsk     [IN]  Link Task Handle
 * \param  pMsg     [IN]  Message Handle
 * \param  done     [IN]  sgx3Dsrv link Run state, set to TRUE once it get the
 *                        is DELETE CMD
 * \param  ackMsg   [OUT] ACK message
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_tskRun(srv3dinfoadaslink_Obj *pObj, OSA_TskHndl *pTsk,
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
                status = srv3dinfoadaslink_drvDoProcessFrames(pObj);
                break;

            case SYSTEM_CMD_DELETE:
                *done = TRUE;
                *ackMsg = TRUE;
                *pMsg = pRunMsg;
                runDone = TRUE;
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                srv3dinfoadaslink_drvPrintStatistics(pObj);
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            default:
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }
    }

    srv3dinfoadaslink_drvStop(pObj);

    if (runAckMsg)
        OSA_tskAckOrFreeMsg(pRunMsg, status);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the START/RUN state of sgx3Dsrv Link.
 *
 *        In this state link gets commands to
 *         - Create the Sgx3Dsrv Driver
 *         - Start the Sgx3Dsrv Driver
 *         - Moves to RUN state
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_tskMain(struct OSA_TskHndl *pTsk,
                                    OSA_MsgHndl *pMsg, UInt32 curState)
{
    UInt32 cmd = OSA_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    srv3dinfoadaslink_Obj *pObj = (srv3dinfoadaslink_Obj *) pTsk->appData;

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
    status = srv3dinfoadaslink_drvCreate(pObj, OSA_msgGetPrm(pMsg));

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
             * CMD to prime and start the sgx3Dsrv driver. Link will be in
             * running state and ready to sgx3Dsrvs the video/graphic frames
             * available at the input side
             */
            case SYSTEM_CMD_START:
                status = srv3dinfoadaslink_drvStart(pObj);

                OSA_tskAckOrFreeMsg(pMsg, status);

                /*
                 * Entering RUN state
                 */
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    status =
                        srv3dinfoadaslink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);
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

    srv3dinfoadaslink_drvDelete(pObj);

    if (ackMsg && pMsg != NULL)
        OSA_tskAckOrFreeMsg(pMsg, status);

    return OSA_SOK;
}

/**
 *******************************************************************************
 *
 *   \brief Sgx3Dsrv link register and init function
 *
 *          For each sgx3Dsrv instance (VID1, VID2, VID3 or GRPX1)
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 srv3dinfoadaslink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 sgx3DsrvId;
    srv3dinfoadaslink_Obj *pObj;
    char tskName[32];

    for (sgx3DsrvId = 0; sgx3DsrvId < SRV3DINFOADAS_LINK_OBJ_MAX; sgx3DsrvId++)
    {
        pObj = &gsrv3dInfoAdasLink_obj[sgx3DsrvId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId = SYSTEM_LINK_ID_SRV3DINFOADAS_0 + sgx3DsrvId;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = NULL;
        linkObj.linkPutEmptyBuffers = NULL;
        linkObj.getLinkInfo = NULL;

        System_registerLink(pObj->linkId, &linkObj);

        sprintf(tskName, "SRV3DINFOADAS%u", (unsigned int)sgx3DsrvId);

        /*
         * Create link task, task remains in IDLE state.
         * Sgx3DsrvLink_tskMain is called when a message command is received.
         */
        status = OSA_tskCreate(&pObj->tsk,
                               srv3dinfoadaslink_tskMain,
                               SRV3DINFOADAS_LINK_TSK_PRI,
                               SRV3DINFOADAS_LINK_TSK_STACK_SIZE,
                               0,
                               pObj);
        OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Sgx3Dsrv link de-register and de-init function
 *
 *          For each sgx3Dsrv instance (VID1, VID2, VID3 or GRPX1)
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 srv3dinfoadaslink_deInit()
{
    srv3dinfoadaslink_Obj *pObj;
    UInt32 sgx3DsrvId;

    for (sgx3DsrvId = 0; sgx3DsrvId < SRV3DINFOADAS_LINK_OBJ_MAX; sgx3DsrvId++)
    {
        pObj = &gsrv3dInfoAdasLink_obj[sgx3DsrvId];

        OSA_tskDelete(&pObj->tsk);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
