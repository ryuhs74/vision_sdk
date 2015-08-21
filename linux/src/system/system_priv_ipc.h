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
 * \brief  Private header file RPMessage based ipc implementation
 *
 * \version 0.0 (APR 2014) : [YM] First version implemented
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_PRIV_IPC_H_
#define _SYSTEM_PRIV_IPC_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <linux/src/system/system_priv_common.h>
#include <linux/src/osa/include/osa_thr.h>
#include <linux/src/osa/include/osa_mutex.h>
#include <linux/src/osa/include/osa_que.h>
#include <linux/src/osa/include/osa_tsk.h>
#include <linux/src/osa/include/osa_debug.h>
#include <linux/src/osa/include/osa_mem.h>
#include <linux/src/osa/include/osa_buf.h>
#include <linux/src/osa/include/osa_sem.h>
#include <linux/src/utils/multiproc/_MultiProc.h>
#include <ti/ipc/MultiProc.h>
#include <stdint.h>
#include <include/link_api/system_ipc_if.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Max elements in local que used for message's
 *******************************************************************************
 */
#define SYSTEM_IPC_MSGQ_MAX_ELEMENTS            (20)

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })



/*******************************************************************************
 *  Enums
 *******************************************************************************
 */


 /**
 *******************************************************************************
 *
 *  \brief These types distinguish between different type of RPMessages channels
 *
 *
 *******************************************************************************
 */
typedef enum
{
    SYSTEM_RPMSG_RX_CHANNEL = 0,
    /**< Used to create RP Message endpoint to receive messages from
         remote core */

    SYSTEM_RPMSG_TX_CHANNEL,
    /**< Used to create RP Message endpoint to transmit messages to
         remote core */

} SYSTEM_RPMSG_SOCKETTYPE;


/*******************************************************************************
 *  declaration for ipc notify callback function
 *******************************************************************************
 */
typedef Void(*System_ipcNotifyCb) (OSA_TskHndl * pTthread);


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
    OSA_MutexHndl msgQLock;
    /**< Lock to protect messageQ Q elements across threads */

    OSA_SemHndl msgQAck;
    /**< Ack semaphore */

    OSA_QueHndl msgQLocalQ;
    /**< local Command que */

    OSA_ThrHndl thrHndl;
    /**< receive thread handle */

} System_IpcMsgQObj;

typedef struct {

    Int32 sockFdRx[SYSTEM_PROC_MAX];
    /**< socket fds for Rx channels */

    Int32 sockFdTx[SYSTEM_PROC_MAX];
    /**< socket fds for Tx channels */

    Int32 unblockFd;
    /**< descriptor unblocking select()  */

    OSA_ThrHndl thrHndl;
    /**< receive thread handle */

    System_ipcNotifyCb notifyCb[SYSTEM_LINK_ID_MAX];

} System_IpcNotifyObj;

/**
 *******************************************************************************
 * \brief IPC object extern declaration
 *******************************************************************************
 */

Int32 System_ipcInit();
Int32 System_ipcDeInit();

Int32 System_ipcNotifyInit();
Int32 System_ipcNotifyDeInit();
Int32 System_ipcRegisterNotifyCb(UInt32 linkId, System_ipcNotifyCb notifyCb);
Int32 System_ipcSendNotify(UInt32 linkId);


Int32 System_ipcMsgQInit();
Int32 System_ipcMsgQDeInit();
Int32 System_ipcMsgQSendMsg(UInt32 linkId, UInt32 cmd, Void * pPrm,
                            UInt32 prmSize, Bool waitAck);

void System_ipcMsgQHandler(UInt32 payload);

Int32 System_ipcCreateChannel(UInt32 procId, UInt32 endpt, UInt32 channelType);
Int32 System_ipcDeleteChannel(Int32 fd);

/* System_ipcMemAlloc returns physical address to allocated buffer */
UInt32 System_ipcMemAlloc(UInt32 heapId, UInt32 memSize, UInt32 memAlign);
/* System_ipcMemFree expects physical address of the buffer to be freed*/
Void   System_ipcMemFree(UInt32 heapId, UInt32 addr, UInt32 memSize);


#endif

/* @} */
