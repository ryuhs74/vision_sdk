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
 * \ingroup UTILS_API
 * \defgroup UTILS_MBX_API Message exchange API
 *
 * \brief  APIs from this file are used to exchange messages between two
 *         tasks in the links and chains examples.
 *
 *         A message consists of a 32-bit command and optional 32-bit parameter
 *         value.
 *
 *         The 32-bit command is defined by the user.
 *
 *         The 32-bit parameter could inturn point to a bigger data structure
 *         as defined by user.
 *
 *         The APIs allow a user to send a message and wait for ACK before
 *         proceeding further.
 *
 *         Internally message passing is implemented using queue's
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_mbx.h
 *
 * \brief Message exchange API
 *
 * \version 0.0 First version
 * \version 0.1 Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _UTILS_MBX_H_
#define _UTILS_MBX_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_que.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 * \brief Maximum receive que length
 *******************************************************************************
*/
#define UTILS_MBX_RECV_QUE_LEN_MAX (1024)

/**
 *******************************************************************************
 * \brief Maximum acknowledgement que length
 *******************************************************************************
*/
#define UTILS_MBX_ACK_QUE_LEN_MAX  (2)

/**
 *******************************************************************************
 * \brief Message flag: wait for ACK when this flag is set
 *******************************************************************************
*/
#define UTILS_MBX_FLAG_WAIT_ACK    (0x1)

/**
 *******************************************************************************
 * \brief Maximum number of ack queues.This allows multiple tasks to wait
 *        on different ack msgs
 *******************************************************************************
*/
#define UTILS_MBX_ACK_QUE_CNT_MAX  (4)

/**
 *******************************************************************************
 * \brief Get 32-bit command from message pointer
 *******************************************************************************
*/
#define Utils_msgGetCmd(pMsg)      ((pMsg)->cmd)

/**
 *******************************************************************************
 * \brief Get 32-bit parameter pointer from message pointer
 *******************************************************************************
*/
#define Utils_msgGetPrm(pMsg)      ((pMsg)->pPrm)

/* @} */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Mailbox handle
 *
 *******************************************************************************
*/
typedef struct {
    Utils_QueHandle recvQue;
  /**< message receive queue  */

    Utils_QueHandle ackQue[UTILS_MBX_ACK_QUE_CNT_MAX];
  /**< ack message receive queue  */

    Bool ackQueInUse[UTILS_MBX_ACK_QUE_CNT_MAX];
  /**< flag indicating whether this que is expecting ack */

    Ptr memRecvQue[UTILS_MBX_RECV_QUE_LEN_MAX];
  /**< memory for receive queue */

    Ptr memAckQue[UTILS_MBX_ACK_QUE_CNT_MAX][UTILS_MBX_ACK_QUE_LEN_MAX];
  /**< memory for ack queue */

    Void *pTsk;
    /**< Handle to tsk handle
     *   MUST be VALID, if pFuncMultiMbxTrigger is NOT NULL
     */

    Int32 (*pFuncMultiMbxTrigger)(Void *pTsk,
                                Int32 timeout);
    /**< Function to invoke to trigger the multi-mbx task
     *   If set to NULL, this feature is not used
     */

} Utils_MbxHndl;

/**
 *******************************************************************************
 *
 *  \brief  Message structure
 *
 *******************************************************************************
*/
typedef struct {
    Utils_MbxHndl *pFrom;
  /**< sender mailbox */

    Int32 result;
  /**< result to be sent as part of ACK */

    Void *pPrm;
  /**< parameters sent by sender  */

    UInt32 cmd;
  /**< command sent by sender */

    UInt32 flags;
  /**< message flags set by sender */

    Utils_QueHandle *ackQue;
  /**< ack queue to which msg will be acknowledged */

} Utils_MsgHndl;

/**
 *******************************************************************************
 * \brief Utils_mbxCreate create parameters
 *******************************************************************************
 */
typedef struct {

    Void *pTsk;
    /**< Handle to tsk handle
     *   MUST be VALID, if pFuncMultiMbxTrigger is NOT NULL
     */

    Int32 (*pFuncMultiMbxTrigger)(Void *pTsk, Int32 timeout);
    /**< Function to invoke to trigger the multi-mbx task
     *   If set to NULL, this feature is not used
     */

} Utils_MbxCreatePrm;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

static inline void Utils_MbxCreatePrm_Init(Utils_MbxCreatePrm *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
}

Int32 Utils_mbxInit();

Int32 Utils_mbxDeInit();

Int32 Utils_mbxCreate(Utils_MbxHndl * pHandle, Utils_MbxCreatePrm *pPrm);

Int32 Utils_mbxDelete(Utils_MbxHndl * pHandle);

Int32 Utils_mbxSendMsg(Utils_MbxHndl * pFrom,
                       Utils_MbxHndl * pTo,
                       UInt32 cmd,
                       Void * pPrm,
                       Uint32 msgFlags);

Int32 Utils_mbxSendCmd(Utils_MbxHndl * pTo, UInt32 cmd, Void *payload);

Int32 Utils_mbxRecvMsg(Utils_MbxHndl * pMbx,
                       Utils_MsgHndl ** pMsg,
                       UInt32 timeout);

Int32 Utils_mbxPeekMsg(Utils_MbxHndl * pMbx, Utils_MsgHndl ** pMsg);

Int32 Utils_mbxAckOrFreeMsg(Utils_MsgHndl * pMsg, Int32 result);

Int32 Utils_mbxWaitCmd(Utils_MbxHndl * pMbx,
                       Utils_MsgHndl ** pMsg,
                       UInt32 cmdToWait);

UInt32 Utils_mbxGetFreeMsgCount();

#endif

/* @} */
