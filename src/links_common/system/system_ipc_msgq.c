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
 * \brief  This file implements the function for the IPC message queue
 *         functionality
 *
 *         This files implements message queue functions. It wraps the
 *         IPC message queue functionality under helper function. It also
 *         performs book keeping like allocating and freeing up of messages.
 *         Its also creates queue for free message queues.
 *
 * \version 0.0 (Jun 2013) : [KC] First version taken from DVR RDK and
 *                                cleaned up for Vision_sdk
 * \version 0.1 (Jul 2013) : [HS] Commenting style update as per defined
 *                                format.
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"



/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSystem_tskMsgQStack, 32)
#pragma DATA_SECTION(gSystem_tskMsgQStack, ".bss:taskStackSection")
UInt8 gSystem_tskMsgQStack[SYSTEM_MSGQ_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief   Create task for receiving IPC messages.
 *
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQTskCreate()
{
    Int32 status;

    gSystem_ipcObj.msgQLock = BspOsal_semCreate(1u, TRUE);
    UTILS_assert(gSystem_ipcObj.msgQLock != NULL);

    gSystem_ipcObj.msgQAck = BspOsal_semCreate(0u, TRUE);
    UTILS_assert(gSystem_ipcObj.msgQAck != NULL);

    status = Utils_queCreate(
                &gSystem_ipcObj.msgQLocalQ,
                SYSTEM_IPC_MSGQ_MAX_ELEMENTS,
                gSystem_ipcObj.msgQLocalQueMem,
                UTILS_QUE_FLAG_BLOCK_QUE
                );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /*
     * Create task
     */
    gSystem_ipcObj.msgQTask = BspOsal_taskCreate(
                                (BspOsal_TaskFuncPtr)System_ipcMsgQTaskMain,
                                SYSTEM_MSGQ_TSK_PRI,
                                gSystem_tskMsgQStack,
                                sizeof(gSystem_tskMsgQStack),
                                NULL);

    UTILS_assert(gSystem_ipcObj.msgQTask != NULL);

    {
        Int32 status;

        status = Utils_prfLoadRegister(gSystem_ipcObj.msgQTask, "SYSTEM_MSGQ");
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
    }
    return SYSTEM_LINK_STATUS_SOK;
}

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
        BspOsal_semPost(gSystem_ipcObj.msgQAck);
    }
    else
    if( type == SYSTEM_LINK_ID_NOTIFY_TYPE_MSG)
    {
        UInt32 procId = SYSTEM_GET_LINK_ID(payload);

        Utils_quePut(&gSystem_ipcObj.msgQLocalQ,
                    (Ptr)procId,
                     0);
    }

    return;
}

/**
 *******************************************************************************
 *
 * \brief   Delete task created for receiving IPC messages.
 *
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQTskDelete()
{
    Vps_printf(" SYSTEM: IPC MsgQ Task Delete in progress !!!\n");

    /* unblock task */
    Utils_quePut(&gSystem_ipcObj.msgQLocalQ,
                    (Ptr)SYSTEM_PROC_MAX,
                    BSP_OSAL_WAIT_FOREVER);

    /* wait for command to be received and task to be exited */
    BspOsal_sleep(1);

    Utils_prfLoadUnRegister(gSystem_ipcObj.msgQTask);

    BspOsal_taskDelete(&gSystem_ipcObj.msgQTask);

    BspOsal_semDelete(&gSystem_ipcObj.msgQLock);
    BspOsal_semDelete(&gSystem_ipcObj.msgQAck);
    Utils_queDelete(&gSystem_ipcObj.msgQLocalQ);

    Vps_printf(" SYSTEM: IPC MsgQ Task Delete DONE !!!\n");

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   Initialize the message queue sub-system.
 *
 *          Local processor creates message queues. All remote processors
 *          open the message queues of remote processors. Heap is create and
 *          registered with message queue. This is requirement of message Q.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQInit()
{
    System_ipcMsgQTskCreate();

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   De-Initialize the message queue sub-system.
 *
 *          Local processor creates message queues. All remote processors
 *          open the message queues of remote processors. Heap is create and
 *          registered with message queue. This is requirement of message Q.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcMsgQDeInit()
{
    System_ipcMsgQTskDelete();

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   Task for receiving the messages on the local processor from
 *          remote processor.
 *
 *          Messages from the remote processors are received by this tasks.
 *          After that messages will be directed to tasks on local processors
 *          for which they are intended.
 * \param arg0 Reserved
 * \param arg1 Reserved
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Void System_ipcMsgQTaskMain(UArg arg0, UArg arg1)
{
    UInt32 prmSize;
    SystemIpcMsgQ_Msg *pMsgCommon;
    UInt32 procId;
    Int32 status;
    Void *pPrm;
    volatile UInt32 readValue;

    while (1)
    {
        status =
            Utils_queGet(&gSystem_ipcObj.msgQLocalQ, (Ptr *) &procId,
                         1,
                         BSP_OSAL_WAIT_FOREVER);

        if(procId == SYSTEM_PROC_MAX )
        {
            break;
        }

        if (status != 0)
        {
            Vps_printf(" MSGQ: MsgQ get failed (procId=%d)!!!\n", procId);
            continue;
        }

        pMsgCommon = (SystemIpcMsgQ_Msg *)System_ipcGetMsg(procId);

        if(pMsgCommon==NULL)
        {
            Vps_printf(" MSGQ: MsgQ get failed (procId=%d) !!!\n", procId);
            continue;
        }

        if (pMsgCommon->waitAck!=1)
        {
            Vps_printf(" MSGQ: Invalid message, message MUST have ACK flag set (procId=%d)!!!\n", procId);
            continue;
        }

        #ifdef A15_TARGET_OS_LINUX
            #ifdef BUILD_M4
            if(SYSTEM_LINK_ID_TEST_ROUTE_BIT_TRUE(pMsgCommon->linkId))
            {
                /* A15 message, clear the route bit and forward the message */
                SYSTEM_LINK_ID_CLEAR_ROUTE_BIT(pMsgCommon->linkId);

                #ifdef SYSTEM_IPC_MSGQ_DEBUG
                Vps_printf
                    (" MSGQ: Received command [0x%04x] (prmSize = %d)"
                     " for [%s][%02d] (waitAck=%d)  from [%s] .. forwarding it !!!\n",
                     pMsgCommon->cmd, pMsgCommon->prmSize,
                     System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
                     SYSTEM_GET_LINK_ID(pMsgCommon->linkId), pMsgCommon->waitAck,
                     System_getProcName(procId)
                     );
                #endif
                /* Route bit can is only set by A15,
                 * hence source CPU is hard coded to A15
                 */

                status =
                    System_ipcSendNotify(
                        SYSTEM_LINK_ID_MAKE_NOTIFY_TYPE
                            (
                                SYSTEM_GET_PROC_ID(pMsgCommon->linkId), /* destination CPU */
                                SYSTEM_PROC_A15_0, /* source CPU */
                                SYSTEM_LINK_ID_NOTIFY_TYPE_MSG /* ACK type */
                            )
                        );

                UTILS_assert (status == SYSTEM_LINK_STATUS_SOK);
                continue;
            }
            #endif
        #endif
        {
#ifdef SYSTEM_IPC_MSGQ_DEBUG
        Vps_printf
            (" MSGQ: Received command [0x%04x] (prmSize = %d)"
             " for [%s][%02d] (waitAck=%d) from [%s] \n",
             pMsgCommon->cmd, pMsgCommon->prmSize,
             System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
             SYSTEM_GET_LINK_ID(pMsgCommon->linkId), pMsgCommon->waitAck,
             System_getProcName(procId)
             );
#endif

            prmSize = pMsgCommon->prmSize;
            pPrm = SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(pMsgCommon);

            if (pMsgCommon->cmd == SYSTEM_CMD_GET_INFO)
            {
                UTILS_assert(prmSize == sizeof(System_LinkInfo));

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

            /* dummay read to ensure, previous write to shared memory is complete */
            readValue = pMsgCommon->status;

            if (pMsgCommon->waitAck)
            {
#ifdef SYSTEM_IPC_MSGQ_DEBUG
            Vps_printf
                (" MSGQ: Acked command [0x%04x] (prmSize = %d)"
                 " for [%s][%02d] to [%s] (waitAck=%d)\n",
                 pMsgCommon->cmd, pMsgCommon->prmSize,
                 System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
                 SYSTEM_GET_LINK_ID(pMsgCommon->linkId),
                 System_getProcName(procId),
                 pMsgCommon->waitAck
                 );
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


                UTILS_assert (status == SYSTEM_LINK_STATUS_SOK);
            }
            else
            {
                UTILS_assert(FALSE);
            }
        }
    }
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
 * \param  pPrm      [IN] Message parameter
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
    volatile UInt32 readValue;

    if (waitAck != TRUE)
    {
        Vps_printf(" MSGQ: Warning!!! : Forcing waitAck = TRUE as waitAck ="
                   " FALSE is not supported."
                   "Fix send cmd [0x%x] to linkId [0x%x",cmd,linkId);
        waitAck = TRUE;
    }

    UTILS_assert(TRUE == waitAck);

    if(prmSize > (SYSTEM_IPC_MSG_SIZE_MAX - sizeof(SystemIpcMsgQ_Msg)))
    {
        Vps_printf(" MSGQ: Parameter size of %d B is greater than max possible"
                   " parameter size of %d B !!!"
                   , prmSize, SYSTEM_IPC_MSG_SIZE_MAX - sizeof(SystemIpcMsgQ_Msg));
        UTILS_assert(0);
    }

    procId = SYSTEM_GET_PROC_ID(linkId);

    UTILS_assert(procId < SYSTEM_PROC_MAX);

    BspOsal_semWait(gSystem_ipcObj.msgQLock, BSP_OSAL_WAIT_FOREVER);

    pMsgCommon = (SystemIpcMsgQ_Msg *)System_ipcGetMsg(System_getSelfProcId());

    UTILS_assert(pMsgCommon != NULL);

    if (prmSize && pPrm)
    {
        pMsgPrm = SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(pMsgCommon);
        memcpy(pMsgPrm, pPrm, prmSize);
    }

    pMsgCommon->linkId = linkId;
    pMsgCommon->prmSize = prmSize;
    pMsgCommon->waitAck = waitAck;
    pMsgCommon->status = SYSTEM_LINK_STATUS_SOK;
    pMsgCommon->cmd = cmd;

    /* Dummy read to ensure previous writes to shared memory are complete */
    readValue = pMsgCommon->cmd;

#ifdef SYSTEM_IPC_MSGQ_DEBUG
    Vps_printf
        (" MSGQ: Sending command [0x%04x] (prmSize = %d)"
         " to [%s][%02d] (waitAck=%d)\n",
         pMsgCommon->cmd, pMsgCommon->prmSize,
         System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
         SYSTEM_GET_LINK_ID(pMsgCommon->linkId), pMsgCommon->waitAck);
#endif

#ifdef A15_TARGET_OS_LINUX
    #ifdef BUILD_ARP32
    /* IPU and DSPs should never call this function directly
     * They must use System_ipcSendMsg()
     */
    if(procId == SYSTEM_PROC_A15_0)
    {
        /* Set route bit and send it to IPU1 */
        SYSTEM_LINK_ID_SET_ROUTE_BIT(pMsgCommon->linkId);
        procId = SYSTEM_PROC_IPU1_0;
    }
    #endif
#endif

    /* clear any pending ACK's */
    BspOsal_semWait(gSystem_ipcObj.msgQAck, BSP_OSAL_NO_WAIT);

    status =
        System_ipcSendNotify(
            SYSTEM_LINK_ID_MAKE_NOTIFY_TYPE
                (
                    procId, /* destination CPU */
                    System_getSelfProcId(), /* source CPU */
                    SYSTEM_LINK_ID_NOTIFY_TYPE_MSG /* message type */
                )
            );

    if (status != 0)
    {
        Vps_printf(" MSGQ: MsgQ put for [%s] failed !!!\n",
                   System_getProcName(procId));
        BspOsal_semPost(gSystem_ipcObj.msgQLock);
        return status;
    }

    if (waitAck)
    {
        Bool semStatus;

        semStatus =
            BspOsal_semWait(gSystem_ipcObj.msgQAck, BSP_OSAL_WAIT_FOREVER);

        if(semStatus==TRUE)
            status = 0;
        else
            status = SYSTEM_LINK_STATUS_EFAIL;

        if (status != 0)
        {
            Vps_printf(" MSGQ: MsgQ Ack get from [%s] failed !!!\n",
                        System_getProcName(procId));
            BspOsal_semPost(gSystem_ipcObj.msgQLock);
            return status;
        }

        if (prmSize && pPrm)
        {
            pMsgPrm = SYSTEM_IPC_MSGQ_MSG_PAYLOAD_PTR(pMsgCommon);
            memcpy(pPrm, pMsgPrm, prmSize);
        }

        status = pMsgCommon->status;

#ifdef SYSTEM_IPC_MSGQ_DEBUG
        Vps_printf
            (" MSGQ: Recived ack for command [0x%04x] (prmSize = %d)"
             " form [%s][%02d] (waitAck=%d)\n",
             pMsgCommon->cmd, pMsgCommon->prmSize,
             System_getProcName(SYSTEM_GET_PROC_ID(pMsgCommon->linkId)),
             SYSTEM_GET_LINK_ID(pMsgCommon->linkId), pMsgCommon->waitAck);
#endif

    }

    BspOsal_semPost(gSystem_ipcObj.msgQLock);

    return status;
}
