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
 * \file ipcInLink_tsk.c
 *
 * \brief  This file has the implementation of IPC IN Link API
 *
 *         This file implements the state machine logic for this link.
 *         A message command will cause the state machine to take some
 *         action and then move to a different state.
 *         IPC IN link follows the below state machine
 *
 *            Cmds| CREATE | STOP | ALL OTHER COMMANDS | DELETE |
 *         States |========|======|====================|========|
 *           IDLE | RUN    | -    | -                  | -      |
 *           RUN  | -      | STOP | -                  | -      |
 *           STOP | -      | -    | -                  | IDLE   |
 *
 * \version 0.0 (Aug 2013) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "ipcInLink_priv.h"

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
IpcInLink_obj gIpcInLink_obj[IPC_IN_LINK_OBJ_MAX];

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
Int32 IpcInLink_getLinkInfo(Void *pTsk,
                             System_LinkInfo *info)
{
    OSA_TskHndl * pTskHndl = (OSA_TskHndl *)pTsk;
    IpcInLink_obj * pObj = (IpcInLink_obj * )pTskHndl->appData;

    /* 'info' structure is set with valid values during 'create' phase */

    memcpy(info, &pObj->linkInfo, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to give full buffers to next
 *        link.
 *
 * ipcIn link sends message to next link about availability of buffers.
 * Next link calls this callback function to get full buffers from ipcIn
 * output queue.
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId from which buffers are required.
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/

Int32 IpcInLink_getFullBuffers(Void * ptr, UInt16 queId,
                                 System_BufferList * pBufList)
{
    OSA_TskHndl *pTsk = (OSA_TskHndl *) ptr;

    IpcInLink_obj *pObj = (IpcInLink_obj *) pTsk->appData;

    return IpcInLink_drvGetFullBuffers(pObj, pBufList);
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to get empty buffers from next
 *        link.
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId from which buffers are required.
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IpcInLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                                  System_BufferList * pBufList)
{
    OSA_TskHndl *pTsk = (OSA_TskHndl *) ptr;

    IpcInLink_obj *pObj = (IpcInLink_obj *) pTsk->appData;

    return IpcInLink_drvPutEmptyBuffers(pObj, pBufList);
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
Int32 IpcInLink_tskRun(IpcInLink_obj * pObj, OSA_TskHndl * pTsk)
{
    Int32          status  = SYSTEM_LINK_STATUS_SOK;
    Bool           runDone = FALSE, stopDone = FALSE;
    OSA_MsgHndl *pRunMsg;
    UInt32         cmd;

    /*
     * This while loop implements RUN state. All the commands for IPC IN
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
                    status  = IpcInLink_drvProcessBuffers(pObj);
                }
                break;

            case SYSTEM_CMD_STOP:

                if(!stopDone)
                {
                    status = IpcInLink_drvStop(pObj);

                    stopDone = TRUE;
                }

                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:

                status = IpcInLink_drvPrintStatistics(pObj);

                OSA_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_DELETE:

                if(!stopDone)
                {
                    status = IpcInLink_drvStop(pObj);
                }

                status |= IpcInLink_drvDelete(pObj);

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
Int32 IpcInLink_tskMain(struct OSA_TskHndl * pTsk, OSA_MsgHndl * pMsg, UInt32 curState)
{
    UInt32           cmd = OSA_msgGetCmd(pMsg);
    Int32            status;
    IpcInLink_obj    *pObj;

    pObj = (IpcInLink_obj *) pTsk->appData;

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
     * Create command received, create the IPC related data structure's
     */
    status = IpcInLink_drvCreate(pObj, OSA_msgGetPrm(pMsg));

    OSA_tskAckOrFreeMsg(pMsg, status);

    /*
     * If create status is error then remain in IDLE state
     */
    if (status != SYSTEM_LINK_STATUS_SOK)
        return OSA_SOK;

    /*
     * Entering RUN state
     */
    status = IpcInLink_tskRun(pObj, pTsk);

    /*
     * Entering IDLE state
     */
    return OSA_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Init function for IPC In link. BIOS task for
 *        link gets created / registered in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 IpcInLink_init()
{
    Int32                status;
    UInt32               instId;
    System_LinkObj       linkObj;
    IpcInLink_obj        *pObj;
    char                 tskName[32];
    UInt32               procId = System_getSelfProcId();

    for(instId = 0; instId<IPC_IN_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIpcInLink_obj[instId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId =
            SYSTEM_MAKE_LINK_ID(procId,
                                SYSTEM_LINK_ID_IPC_IN_0 + instId);

        pObj->linkInstId = instId;

        memset(&linkObj, 0, sizeof(linkObj));

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers  = IpcInLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = IpcInLink_putEmptyBuffers;
        linkObj.getLinkInfo         = IpcInLink_getLinkInfo;

        System_registerLink(pObj->linkId, &linkObj);

        sprintf(tskName, "IPC_IN_%u", (unsigned int)instId);

        /* register notify handler with system framework */
        System_ipcRegisterNotifyCb(pObj->linkId, IpcInLink_drvNotifyCb);


        /*
         * Create link task, task remains in IDLE state.
         * IpcInLink_tskMain is called when a message command is received.
         */
        status = OSA_tskCreate(&pObj->tsk,
                                 IpcInLink_tskMain,
                                 IPC_LINK_TSK_PRI,
                                 IPC_IN_LINK_TSK_STACK_SIZE,
                                 0,
                                 pObj);
        OSA_assert(status == OSA_SOK);

        status = OSA_mutexCreate(&(pObj->lock));
        OSA_assert(status == OSA_SOK);

    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-Init function for IPC In link. BIOS task for
 *        link gets deleted in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IpcInLink_deInit()
{
    IpcInLink_obj *pObj;
    UInt32 instId;

    for(instId = 0; instId<IPC_IN_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIpcInLink_obj[instId];

        OSA_tskDelete(&pObj->tsk);
        OSA_mutexDelete(&(pObj->lock));

        System_ipcRegisterNotifyCb(pObj->linkId, NULL);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
