/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED.
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file system_ipc_msgq.c
 *
 * \brief  This file implements messaging mechanism between A15 and other CPUs
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

System_IpcMsgQObj gSystem_ipcMsgQObj;

/**
 *******************************************************************************
 *
 * \brief   Message handler called from Notify handler
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
void System_ipcMsgQHandler(UInt32 payload)
{
    Uint32 type = SYSTEM_LINK_ID_GET_NOTIFY_TYPE(payload);

    if( type == SYSTEM_LINK_ID_NOTIFY_TYPE_MSG_ACK)
    {
        OSA_semSignal(&gSystem_ipcMsgQObj.msgQAck);
    }
    else
    if( type == SYSTEM_LINK_ID_NOTIFY_TYPE_MSG)
    {
        UInt32 procId = SYSTEM_GET_LINK_ID(payload);

        OSA_quePut(&gSystem_ipcMsgQObj.msgQLocalQ,
                     procId,
                     OSA_TIMEOUT_NONE);
    }

    return;
}

/**
 ******************************************************************************
 *
 * \brief This function is the implementation of task to receive messages.
 *
 * \return  void
 *
 ******************************************************************************
 */
Void* System_ipcMsgQTskMain(Void * prm)
{
    UInt32 prmSize;
    SystemIpcMsgQ_Msg *pMsgCommon;
    UInt32 procId;
    Int32 status;
    Void *pPrm;

    while(1)
    {
        status =
            OSA_queGet(&gSystem_ipcMsgQObj.msgQLocalQ, (Int32 *) &procId,
                         OSA_TIMEOUT_FOREVER);

        if(procId == SYSTEM_PROC_MAX )
        {
            break;
        }

        if (status != 0)
        {
            Vps_printf(" SYSTEM: MSGQ: MsgQ get failed (procId=%d)!!!\n", procId);
            continue;
        }

        pMsgCommon = (SystemIpcMsgQ_Msg *)System_ipcGetMsg(procId);

        if(pMsgCommon==NULL)
        {
            Vps_printf(" SYSTEM: MSGQ: MsgQ get failed (procId=%d) !!!\n", procId);
            continue;
        }

        if (pMsgCommon->waitAck!=1)
        {
            Vps_printf(" SYSTEM: MSGQ: Invalid message, message MUST have ACK flag set (procId=%d)!!!\n", procId);
            continue;
        }

#if 0
        Vps_printf
            (" SYSTEM: MSGQ: Received command [0x%04x] (prmSize = %d)"
             " for [%s][%02d] (waitAck=%d)\n",
             pMsgCommon->cmd, pMsgCommon->prmSize,
             System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
             SYSTEM_GET_LINK_ID(pMsgCommon->linkId), pMsgCommon->waitAck);
#endif

        prmSize = pMsgCommon->prmSize;

        pPrm = SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(pMsgCommon);

        if (pMsgCommon->cmd == SYSTEM_CMD_GET_INFO)
        {
            OSA_assert(prmSize == sizeof(System_LinkInfo));

            pMsgCommon->status =
                System_linkGetInfo_local(pMsgCommon->linkId, pPrm);
        }
        else
        {
            pMsgCommon->status = System_linkControl_local(pMsgCommon->linkId,
                                                          pMsgCommon->cmd,
                                                          pPrm,
                                                          prmSize,
                                                          pMsgCommon->waitAck);
        }

        if (pMsgCommon->waitAck)
        {
#if 0
        Vps_printf
            (" SYSTEM: MSGQ: Acked command [0x%04x] (prmSize = %d)"
             " for [%s][%02d] (waitAck=%d)\n",
             pMsgCommon->cmd, pMsgCommon->prmSize,
             System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
             SYSTEM_GET_LINK_ID(pMsgCommon->linkId), pMsgCommon->waitAck);
#endif

            status =
                System_ipcSendNotify(
                    SYSTEM_LINK_ID_MAKE_NOTIFY_TYPE
                        (
                            procId, /* destination CPU */
                            System_getSelfProcId(), /* source CPU */
                            SYSTEM_LINK_ID_NOTIFY_TYPE_MSG_ACK /* ACK type */
                        )
                    );


            OSA_assert (status == SYSTEM_LINK_STATUS_SOK);
        }
        else
        {
            OSA_assert(FALSE);
        }

    } /* while(1) */

    return(NULL);
}

/**
 *******************************************************************************
 *
 * \brief Utility to initialize message queue sub-system for links and chains
 *        work.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQInit()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    Vps_printf(" SYSTEM: MSGQ: MsgQ init in progress !!!\n");

    memset(&gSystem_ipcMsgQObj, 0, sizeof(gSystem_ipcMsgQObj));

    status = OSA_mutexCreate(&(gSystem_ipcMsgQObj.msgQLock));
    OSA_assert(status == OSA_SOK);

    status = OSA_queCreate(&gSystem_ipcMsgQObj.msgQLocalQ,
                        SYSTEM_IPC_MSGQ_MAX_ELEMENTS);
    OSA_assert(status == OSA_SOK);

    status = OSA_semCreate(&gSystem_ipcMsgQObj.msgQAck, 1, 0);
    OSA_assert(status == OSA_SOK);

    /* Create a thread that blocks on socket to receive message from kernel */
    status = OSA_thrCreate(&gSystem_ipcMsgQObj.thrHndl,
          System_ipcMsgQTskMain,
          OSA_THR_PRI_DEFAULT,
          OSA_THR_STACK_SIZE_DEFAULT,
          NULL);
    OSA_assert(status == OSA_SOK);

    Vps_printf(" SYSTEM: MSGQ: MsgQ init DONE !!!\n");

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Utility to de-initialize notify sub-system for links and chains
 *        work.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQDeInit()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    Vps_printf(" SYSTEM: MSGQ: MsgQ de-init in progress !!!\n");

    OSA_quePut(&gSystem_ipcMsgQObj.msgQLocalQ, SYSTEM_PROC_MAX, OSA_TIMEOUT_FOREVER);

    OSA_thrDelete(&(gSystem_ipcMsgQObj.thrHndl));

    OSA_semDelete(&(gSystem_ipcMsgQObj.msgQAck));

    OSA_queDelete(&(gSystem_ipcMsgQObj.msgQLocalQ));

    OSA_mutexDelete(&(gSystem_ipcMsgQObj.msgQLock));

    Vps_printf(" SYSTEM: MSGQ: MsgQ de-init DONE !!!\n");

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Helper function to send messages to other links.
 *
 *          Allocates the message from free queue and send it to the link
 *          also waits for the acknowledgement for the message.
 *
 * \param  linkId    [IN] LinkId for which message is intended
 * \param  cmd       [IN] Command for message
 * \param  pPrm       [IN] Message parameter
 * \param  prmSize   [IN] Size of parameter
 * \param  waitAck   [IN] If True wait till ack is received, else proceed
 *                        without ack.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQSendMsg(UInt32 linkId, UInt32 cmd, Void * pPrm,
                            UInt32 prmSize, Bool waitAck)
{


    Int32 status = SYSTEM_LINK_STATUS_SOK;
    SystemIpcMsgQ_Msg *pMsgCommon;
    UInt32 procId;
    Void *pMsgPrm;

    OSA_mutexLock(&(gSystem_ipcMsgQObj.msgQLock));

    if (waitAck != TRUE)
    {
        Vps_printf(" SYSTEM: MSGQ: Warning!!! : Forcing waitAck = TRUE as waitAck ="
                   " FALSE is not supported."
                   "Fix send cmd [0x%x] to linkId [0x%x]",cmd,linkId);
        waitAck = TRUE;
    }

    OSA_assert(TRUE == waitAck);
    OSA_assert(prmSize + sizeof(*pMsgCommon) <= SYSTEM_IPC_MSG_SIZE_MAX);
    procId = SYSTEM_GET_PROC_ID(linkId);

    OSA_assert(procId < SYSTEM_PROC_MAX);

    pMsgCommon = (SystemIpcMsgQ_Msg *)System_ipcGetMsg(System_getSelfProcId());
    OSA_assert(pMsgCommon != NULL);

    if (prmSize && pPrm)
    {
        pMsgPrm = SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(pMsgCommon);
        memcpy(pMsgPrm, pPrm, prmSize);
    }

    pMsgCommon->linkId = linkId;
    pMsgCommon->cmd = cmd;
    pMsgCommon->prmSize = prmSize;
    pMsgCommon->waitAck = waitAck;
    pMsgCommon->status = SYSTEM_LINK_STATUS_SOK;

    if(procId == SYSTEM_PROC_EVE1 ||
       procId == SYSTEM_PROC_EVE2 ||
       procId == SYSTEM_PROC_EVE3 ||
       procId == SYSTEM_PROC_EVE4)
    {
        SYSTEM_LINK_ID_SET_ROUTE_BIT(pMsgCommon->linkId);
        procId = SYSTEM_PROC_IPU1_0;
    }

    OSA_semWait(&gSystem_ipcMsgQObj.msgQAck, OSA_TIMEOUT_NONE);

    status = System_ipcSendNotify(
                SYSTEM_LINK_ID_MAKE_NOTIFY_TYPE
                (
                    procId, /* destination CPU */
                    System_getSelfProcId(), /* source CPU */
                    SYSTEM_LINK_ID_NOTIFY_TYPE_MSG /* message type */
                )
             );

    if (status >= 0)
    {
        if (waitAck)
        {
            status = OSA_semWait(&gSystem_ipcMsgQObj.msgQAck, OSA_TIMEOUT_FOREVER);

            if (status >= 0)
            {
                if (prmSize && pPrm)
                {
                    pMsgPrm = SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(pMsgCommon);
                    memcpy(pPrm, pMsgPrm, prmSize);
                }

                status = pMsgCommon->status;
            }
            else
            {
                Vps_printf(" SYSTEM: MSGQ: [%s] MsgQ ACK recvfrom failed  !!!\n",
                    System_getProcName(procId));
            }
        }
    }
    else
    {
        Vps_printf(" SYSTEM: MSGQ: [%s] MsgQ send failed !!!\n",
                System_getProcName(procId));
    }

    OSA_mutexUnlock(&(gSystem_ipcMsgQObj.msgQLock));

    return status;
}







