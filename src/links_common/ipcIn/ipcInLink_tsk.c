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
    Utils_TskHndl * pTskHndl = (Utils_TskHndl *)pTsk;
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
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

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
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    IpcInLink_obj *pObj = (IpcInLink_obj *) pTsk->appData;

    return IpcInLink_drvPutEmptyBuffers(pObj, pBufList);
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
Void IpcInLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32           cmd = Utils_msgGetCmd(pMsg);
    Int32            status = SYSTEM_LINK_STATUS_SOK;
    IpcInLink_obj    *pObj;
    UInt32          flushCmds[1];

    pObj = (IpcInLink_obj *) pTsk->appData;

    /*
     * Different commands are serviced via this switch case. For each
     * command, after servicing, ACK or free message is sent before
     * proceeding to next state.
     */
    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:

            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
                /*
                 * Create command received, create the IPC related data structure's
                 */
                status = IpcInLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    pObj->state = SYSTEM_LINK_STATE_RUNNING;
                }
            }

            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_NEW_DATA:

            Utils_tskAckOrFreeMsg(pMsg, status);

            flushCmds[0] = SYSTEM_CMD_NEW_DATA;
            Utils_tskFlushMsg(pTsk, flushCmds, 1);

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                /* if STOP state then dont handle buffers */
                status  = IpcInLink_drvProcessBuffers(pObj);
            }
            break;

        case SYSTEM_CMD_STOP:

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = IpcInLink_drvStop(pObj);
                pObj->state = SYSTEM_LINK_STATE_STOPPED;
            }

            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_PRINT_STATISTICS:

            if(pObj->state!=SYSTEM_LINK_STATE_IDLE)
            {
                status = IpcInLink_drvPrintStatistics(pObj);
            }

            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_DELETE:

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
               status = IpcInLink_drvStop(pObj);
               pObj->state = SYSTEM_LINK_STATE_STOPPED;
            }
            if(pObj->state==SYSTEM_LINK_STATE_STOPPED)
            {
                status |= IpcInLink_drvDelete(pObj);

                pObj->state = SYSTEM_LINK_STATE_IDLE;
            }

            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        /*
         * Start is made as dummy for this link, since there is
         * no need.
         */
        case SYSTEM_CMD_START:
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        /*
         * Invalid command for this state.  ACK it and continue RUN
         */
        default:
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
    }

    return;
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

        pObj->state = SYSTEM_LINK_STATE_IDLE;

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

        status = IpcInLink_tskCreate(instId);
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
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

        Utils_tskDelete(&pObj->tsk);

        System_ipcRegisterNotifyCb(pObj->linkId, NULL);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
