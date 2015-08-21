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
 * \file ipcOutLink_tsk.c
 *
 * \brief  This file has the implementataion of IPC OUT Link API
 *
 *         This file implements the state machine logic for this link.
 *         A message command will cause the state machine to take some
 *         action and then move to a different state.
 *         IPC OUT link follows the below state machine
 *
 *            Cmds| CREATE | STOP | ALL OTHER COMMANDS | DELETE |
 *         States |========|======|====================|========|
 *           IDLE | RUN    | -    | -                  | -      |
 *           RUN  | -      | STOP | -                  | -      |
 *           STOP | -      | -    | -                  | IDLE   |
 *
 *
 * \version 0.0 (May 2014) : [YM] First version ported to linux
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "ipcOutLink_priv.h"

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
IpcOutLink_Obj gIpcOutLink_obj[IPC_OUT_LINK_OBJ_MAX];


/**
 *******************************************************************************
 *
 * \brief This function return the channel info to the next link
 *
 * \param  pTsk     [IN]  Task Handle
 * \param  pTsk     [OUT] channel info
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_getLinkInfo(Void *pTsk,
                             System_LinkInfo *info)
{
    OSA_TskHndl * pTskHndl = (OSA_TskHndl *)pTsk;
    IpcOutLink_Obj * pObj = (IpcOutLink_Obj * )pTskHndl->appData;

    /* 'info' structure is set with valid values during 'create' phase
     * Simply pass on previous link info to next link
     */

    memcpy(info, &pObj->linkInfo, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function is the implementation of Run and stop state.
 *
 *        In this state link gets commands to process incoming frames from
 *        previous link and release processed frames from next link to previous
 *        link
 *
 * \param  pTsk     [IN] Task Handle
 * \param  pObj     [IN] Link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_tskRun(IpcOutLink_Obj * pObj, OSA_TskHndl * pTsk)
{
    Int32          status  = SYSTEM_LINK_STATUS_SOK;
    Bool           runDone = FALSE, stopDone = FALSE;
    OSA_MsgHndl *pRunMsg;
    UInt32         cmd;

    /*
     * This while loop implements RUN state. All the commands for IPC OUT
     * are received and serviced in this while loop. Control remains
     * in this loop until delete commands arrives.
     */
    while (!runDone)
    {
        /*
         * Wait for message indefinitely here. Once message arrives, extract
         * the command.
         */
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
            case SYSTEM_CMD_NEW_DATA:

                OSA_tskAckOrFreeMsg(pRunMsg, status);

                if(!stopDone)
                {
                    /* if STOP state then dont handle buffers */
                    status  = IpcOutLink_drvProcessBuffers(pObj);
                }
                break;

            case IPC_OUT_LINK_CMD_RELEASE_FRAMES:

                OSA_tskAckOrFreeMsg(pRunMsg, status);

                if(!stopDone)
                {
                    /* if STOP state then dont handle buffers */
                    status = IpcOutLink_drvReleaseBuffers(pObj);
                }
                break;

            case SYSTEM_CMD_STOP:

                if(!stopDone)
                {
                    status = IpcOutLink_drvStop(pObj);

                    stopDone = TRUE;
                }

                OSA_tskAckOrFreeMsg(pRunMsg, status);

                break;

            case IPC_OUT_LINK_CMD_SET_FRAME_RATE:

                status = IpcOutLink_drvSetFrameRate(
                                        pObj,
                                        OSA_msgGetPrm(pRunMsg));

                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:

                status = IpcOutLink_drvPrintStatistics(pObj);

                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_DELETE:

                if(!stopDone)
                {
                    status = IpcOutLink_drvStop(pObj);
                }

                status |= IpcOutLink_drvDelete(pObj);

                OSA_tskAckOrFreeMsg(pRunMsg, status);

                runDone = TRUE;
                break;

            /*
             * Start is made as dummy for this link, since there is
             * no need.
             */
            case SYSTEM_CMD_START:
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            /*
             * Invalid command for this state.  ACK it and continue RUN
             */
            default:
                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }
    }


    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function is the implementation of Idle state.
 *
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Int32 IpcOutLink_tskMain(struct OSA_TskHndl * pTsk, OSA_MsgHndl * pMsg, UInt32 curState)
{
    UInt32               cmd = OSA_msgGetCmd(pMsg);
    Int32                status;
    IpcOutLink_Obj      *pObj;

    pObj = (IpcOutLink_Obj *) pTsk->appData;

    /*
     * At this stage only create command is the expected command.
     * If other message gets received Ack with error status
     */
    if (cmd != SYSTEM_CMD_CREATE)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    /*
     * Create command received, create the IPC related data structure's
     */
    status = IpcOutLink_drvCreate(pObj, OSA_msgGetPrm(pMsg));

    OSA_tskAckOrFreeMsg(pMsg, status);

    /*
     * If create status is error then remain in IDLE state
     */
    if (status != SYSTEM_LINK_STATUS_SOK)
        return SYSTEM_LINK_STATUS_EFAIL;

    /*
     * Entering RUN state
     */
    status = IpcOutLink_tskRun(pObj, pTsk);

    /*
     * Entering IDLE state
     */
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Init function for IPC Out link. OSA_tsk for
 *        link gets created / registered in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 IpcOutLink_init()
{
    Int32                status;
    UInt32               instId;
    System_LinkObj       linkObj;
    IpcOutLink_Obj      *pObj;
    UInt32               procId = System_getSelfProcId();

    for(instId = 0; instId < IPC_OUT_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIpcOutLink_obj[instId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId =
            SYSTEM_MAKE_LINK_ID(procId,
                                SYSTEM_LINK_ID_IPC_OUT_0) + instId;

        pObj->linkInstId = instId;

        memset(&linkObj, 0, sizeof(linkObj));

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers  = NULL;
        linkObj.linkPutEmptyBuffers = NULL;
        linkObj.getLinkInfo         = IpcOutLink_getLinkInfo;

        System_registerLink(pObj->linkId, &linkObj);

        /* register notify handler with system framework */
        System_ipcRegisterNotifyCb(pObj->linkId, IpcOutLink_drvNotifyCb);


        /* allocate shared memory for IPC queue */
        pObj->ipcOut2InSharedMemBaseAddr =
            System_ipcGetIpcOut2InQue(pObj->linkId);
        OSA_assert(pObj->ipcOut2InSharedMemBaseAddr!=NULL);

        /* Translate to virtual */
        pObj->ipcIn2OutSharedMemBaseAddr =
            System_ipcGetIpcIn2OutQue(pObj->linkId);
        OSA_assert(pObj->ipcIn2OutSharedMemBaseAddr!=NULL);

        /* create IPC queue's */
        status = OSA_ipcQueCreate(
                            &pObj->ipcOut2InQue,
                            SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS,
                            pObj->ipcOut2InSharedMemBaseAddr,
                            sizeof(UInt32)
                        );
        OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

        status = OSA_ipcQueCreate(
                            &pObj->ipcIn2OutQue,
                            SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS,
                            pObj->ipcIn2OutSharedMemBaseAddr,
                            sizeof(UInt32)
                        );
        OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

        /*
         * Create link task, task remains in IDLE state.
         * ipcOutLink_tskMain is called when a message command is received.
         */
        status = OSA_tskCreate(&pObj->tsk,
                               IpcOutLink_tskMain,
                               IPC_LINK_TSK_PRI,
                               IPC_OUT_LINK_TSK_STACK_SIZE,
                               0,
                               pObj);
        OSA_assert(status == OSA_SOK);

    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-Init function for IPC Out link. BIOS task for
 *        link gets deleted in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IpcOutLink_deInit()
{
    IpcOutLink_Obj *pObj;
    UInt32 instId;

    for(instId = 0; instId<IPC_OUT_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIpcOutLink_obj[instId];

        OSA_tskDelete(&pObj->tsk);

        System_ipcRegisterNotifyCb(pObj->linkId, NULL);

        OSA_ipcQueDelete(&pObj->ipcIn2OutQue);
        OSA_ipcQueDelete(&pObj->ipcOut2InQue);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
