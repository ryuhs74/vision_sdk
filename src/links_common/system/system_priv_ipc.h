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
 *
 * \ingroup SYSTEM_IMPL
 * \defgroup SYSTEM_IPC_IMPL   System implementation related to IPC
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file system_priv_ipc.h
 *
 * \brief  Private header file IPC implementation
 *
 * \version 0.0 (Jun 2013) : [KC] First version taken from DVR RDK and
 *                                cleaned up for Vision_sdk
 * \version 0.1 (Jul 2013) : [HS] Commenting style update as per defined
 *                                format.
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_PRIV_IPC_H_
#define _SYSTEM_PRIV_IPC_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_common.h>
#include <include/link_api/system_ipc_if.h>


//#define SYSTEM_IPC_MSGQ_DEBUG

/*******************************************************************************
 *  Max elements in local que used for message's
 *******************************************************************************
 */
#define SYSTEM_IPC_MSGQ_MAX_ELEMENTS            (20)

/*******************************************************************************
 *  declaration for ipc notify callback function
 *******************************************************************************
 */
typedef Void(*System_ipcNotifyCb) (Utils_TskHndl * pTsk);

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure for book keeping IPC message queue handling.
 *
 *******************************************************************************
*/
typedef struct {

    BspOsal_SemHandle msgQLock;
    /**< Lock to protect messageQ Q elements across threads */

    BspOsal_SemHandle msgQAck;
    /**< ACK for a sent message */

    BspOsal_TaskHandle msgQTask;
    /**< Handle to task handing IPC message queues */

    Utils_QueHandle msgQLocalQ;
    /**< MessageQ to get messages for this task */

    UInt32 msgQLocalQueMem[SYSTEM_IPC_MSGQ_MAX_ELEMENTS];
    /**< Memory for local QUE */

    System_ipcNotifyCb notifyCb[SYSTEM_LINK_ID_MAX];
    /**< Callback function for the IPC notify */

} System_IpcObj;

/**
 *******************************************************************************
 * \brief IPC object extern declaration
 *******************************************************************************
 */
extern System_IpcObj gSystem_ipcObj;

Int32 System_ipcInit();
Int32 System_ipcDeInit();

Void  System_ipcHandler(UInt32 payload);

Void  System_ipcStart();
Void  System_ipcStop();

Int32 System_ipcSendNotify(UInt32 linkId);
Int32 System_ipcSendMsg(UInt32 linkId, UInt32 cmd, Void * pPrm,
                            UInt32 prmSize, Bool waitAck);

Int32 System_ipcRegisterNotifyCb(UInt32 linkId, System_ipcNotifyCb notifyCb);

void  System_ipcMsgQHandler(UInt32 payload);

Int32 System_ipcNotifyInit();
Int32 System_ipcNotifyDeInit();

Int32 System_ipcNotifyRegister();
Int32 System_ipcNotifySendEvent(UInt32 linkId);

Int32 System_ipcMsgQInit();
Int32 System_ipcMsgQDeInit();

Void System_ipcMsgQTaskMain(UArg arg0, UArg arg1);
Int32 System_ipcMsgQSendMsg(UInt32 linkId, UInt32 cmd, Void * pPrm,
                            UInt32 prmSize, Bool waitAck);


Int32 System_rpmsgSendNotify(UInt32 linkId);

Int32 System_rpmsgInit();
Int32 System_rpmsgDeInit();

#endif

/* @} */
