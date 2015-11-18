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
 * \version 0.0 (July 2013) : [KC] First version
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
    Utils_TskHndl * pTskHndl = (Utils_TskHndl *)pTsk;
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
 * \brief This function is the implementation of Idle state.
 *
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void IpcOutLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32               cmd = Utils_msgGetCmd(pMsg);
    Int32                status = SYSTEM_LINK_STATUS_SOK;
    IpcOutLink_Obj      *pObj;
    UInt32              flushCmds[1];

    pObj = (IpcOutLink_Obj *) pTsk->appData;



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
                status = IpcOutLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    pObj->state = SYSTEM_LINK_STATE_RUNNING;
                }
            }

            Utils_tskAckOrFreeMsg(pMsg, status);

            break;

        case SYSTEM_CMD_NEW_DATA:
//Vps_printf("IPCOut: tskMain SYSTEM_CMD_NEW_DATA LinkId : %dstart !!!\n",pObj->linkId);
            Utils_tskAckOrFreeMsg(pMsg, status);

            flushCmds[0] = SYSTEM_CMD_NEW_DATA;
            Utils_tskFlushMsg(pTsk, flushCmds, 1);

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                /* if STOP state then dont handle buffers */
                status  = IpcOutLink_drvProcessBuffers(pObj);
            }
//Vps_printf("IPCOut: tskMain SYSTEM_CMD_NEW_DATA end !!!\n");
            break;

        case IPC_OUT_LINK_CMD_RELEASE_FRAMES:

            Utils_tskAckOrFreeMsg(pMsg, status);

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                /* if STOP state then dont handle buffers */
                status = IpcOutLink_drvReleaseBuffers(pObj);
            }
            break;

        case SYSTEM_CMD_STOP:

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = IpcOutLink_drvStop(pObj);

                pObj->state = SYSTEM_LINK_STATE_STOPPED;
            }

            Utils_tskAckOrFreeMsg(pMsg, status);

            break;

        case IPC_OUT_LINK_CMD_SET_FRAME_RATE:

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = IpcOutLink_drvSetFrameRate(
                                    pObj,
                                    Utils_msgGetPrm(pMsg));
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_PRINT_STATISTICS:

            if(pObj->state!=SYSTEM_LINK_STATE_IDLE)
            {
                status = IpcOutLink_drvPrintStatistics(pObj);
            }

            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_DELETE:

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = IpcOutLink_drvStop(pObj);
                pObj->state = SYSTEM_LINK_STATE_STOPPED;
            }
            if(pObj->state==SYSTEM_LINK_STATE_STOPPED)
            {
                status |= IpcOutLink_drvDelete(pObj);
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
 * \brief Init function for IPC Out link. BIOS task for
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

    for(instId = 0; instId<IPC_OUT_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIpcOutLink_obj[instId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId =
            SYSTEM_MAKE_LINK_ID(procId,
                                SYSTEM_LINK_ID_IPC_OUT_0) + instId;

        pObj->state = SYSTEM_LINK_STATE_IDLE;
        pObj->linkInstId = instId;

        memset(&linkObj, 0, sizeof(linkObj));

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers  = NULL;
        linkObj.linkPutEmptyBuffers = NULL;
        linkObj.getLinkInfo         = IpcOutLink_getLinkInfo;

        System_registerLink(pObj->linkId, &linkObj);

        /* register notify handler with system framework */
        System_ipcRegisterNotifyCb(pObj->linkId, IpcOutLink_drvNotifyCb);

        /*
         * Create link task, task remains in IDLE state.
         * ipcOutLink_tskMain is called when a message command is received.
         */
        status = IpcOutLink_tskCreate(instId);
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
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

        Utils_tskDelete(&pObj->tsk);

        System_ipcRegisterNotifyCb(pObj->linkId, NULL);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
