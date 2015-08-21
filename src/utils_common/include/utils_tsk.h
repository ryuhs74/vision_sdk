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
 * \defgroup UTILS_TSK_API Task wrapper APIs
 *
 * \brief  APIs from this file are used to interface the sysbios task functions
 *
 *         The APIs allow a user to create/delete the Bios tasks and wait
 *         for ACK before proceeding further.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_tsk.h
 *
 * \brief Task wrapper API
 *
 * \version 0.0 (Jan 2013) : [SS] First version
 * \version 0.1 (Feb 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _UTILS_TSK_H_
#define _UTILS_TSK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/utils_common/include/utils_mbx.h>


/*******************************************************************************
 *  Define's
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Number of priority queues in Multi-mbx task
 *******************************************************************************
*/
#define UTILS_TASK_MULTI_MBX_RECV_QUE_MAX    (2)


/**
 *******************************************************************************
 * \brief Highest priority in multi-mbx task recevie queue
 *******************************************************************************
*/
#define UTILS_TASK_MULTI_MBX_PRI_HIGHEST  (0)


/**
 *******************************************************************************
 * \brief Lowest priority in multi-mbx task recevie queue
 *******************************************************************************
 */
#define UTILS_TASK_MULTI_MBX_PRI_LOWEST   (UTILS_TASK_MULTI_MBX_RECV_QUE_MAX-1)

/**
 *******************************************************************************
 * \brief Length of multi-mbx receive queue
 *******************************************************************************
*/
#define UTILS_TASK_MULTI_MBX_RECV_QUE_LEN_MAX   (1024)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Task handle
 *******************************************************************************
*/
struct Utils_TskHndl;

/**
 *******************************************************************************
 *
 * \brief Task main function.
 *
 *        This function is called when a message is received by the task.
 *
 * \param pHndl [OUT] Task handle
 * \param pMsg  [OUT] Received message
 *
 * \return  void
 *
 *******************************************************************************
 */
typedef Void(*Utils_TskFuncMain) (struct Utils_TskHndl * pHndl,
                                  Utils_MsgHndl * pMsg);


struct Utils_TskMultiMbxHndl;

/**
 *******************************************************************************
 * \brief Task handle
 *******************************************************************************
*/
typedef struct Utils_TskHndl {
    BspOsal_TaskHandle tsk;
    /**< BIOS Task handle */

    Utils_MbxHndl mbx;
    /**< Mail box associated with this task */

    UInt8 *stackAddr;
    /**< Task stack address */

    UInt32 stackSize;
    /**< Task stack size */

    UInt32 tskPri;
    /**< Task priority as defined by BIOS */

    Utils_TskFuncMain funcMain;
    /**< Task main,
     *   Note, this is different from BIOS Task, since this function
     *   is entered ONLY when a message is received.
     */

    Ptr appData;
    /**< Application specific data */

    struct Utils_TskMultiMbxHndl *pMultiMbxTsk;
    /**< Handle to multi-mbx task, if NULL this feature is not used */

} Utils_TskHndl;

/**
 *******************************************************************************
 * \brief Multi-mbx Task handle
 *
 *        Multi-mbx task is task in which messages to multiple mailboxes
 *        are handled in a single OS task
 *        This allows to reduce number of task in a CPU and save on
 *        task switching, task stack space.
 *        This also allow user to map multiple 'links' to single task in
 *        a CPU like EVE. This allow the single task stack to be placed
 *        in internal EVE DMEM for higher performance.
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 numRefs;
    /**< When numRefs is 0 and create is called, task is created
     *   When numRefs is 0 and delete is called, task is deleted
     */

    BspOsal_TaskHandle tsk;
    /**< BIOS Task handle */

    UInt8 *stackAddr;
    /**< Task stack address */

    UInt32 stackSize;
    /**< Task stack size */

    UInt32 tskPri;
    /**< Task priority as defined by BIOS */

    BspOsal_SemHandle semTskPend;
    /**< Semaphore on which the task pends until a message is sent to it */

    char tskName[32];
    /**< Name of multi-mbx task */

    UInt32 numPriQue;
    /**< Number of priority queue's,
     *   MUST be <= UTILS_TASK_MULTI_MBX_RECV_QUE_MAX
     */

    Utils_QueHandle recvQue[UTILS_TASK_MULTI_MBX_RECV_QUE_MAX];
    /**< Receive mailbox queue, one for each priority
     *   0 is highest priority, UTILS_TASK_MULTI_MBX_RECV_QUE_MAX-1 is lowest
     */

    Ptr memRecvQue[UTILS_TASK_MULTI_MBX_RECV_QUE_MAX][UTILS_TASK_MULTI_MBX_RECV_QUE_LEN_MAX];
    /**< memory for receive queue */

    Bool doExitTask;
    /**< Flag to indicate task exit */

} Utils_TskMultiMbxHndl;

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */

Int32 Utils_tskCreate(Utils_TskHndl * pHndl,
                      Utils_TskFuncMain funcMain,
                      UInt32 tskPri,
                      UInt8 * stackAddr,
                      UInt32 stackSize, Ptr appData, char *tskName);

Int32 Utils_tskDelete(Utils_TskHndl * pHndl);

Int32 Utils_tskSetPri(Utils_TskHndl * pHndl, UInt32 newPri);

Int32 Utils_tskFlushMsg(Utils_TskHndl * pHndl, UInt32 *flushCmdId,
                        UInt32 numCmds);

Int32 Utils_tskMultiMbxCreate(Utils_TskHndl * pHndl,
                              Utils_TskMultiMbxHndl * pMultiMbxHndl,
                              Utils_TskFuncMain funcMain,
                              UInt32 tskMultiMbxPri,
                              Ptr appData);

Int32 Utils_tskMultiMbxDelete(Utils_TskHndl * pHndl);

Int32 Utils_tskMultiMbxSetupTskHndl(
                        Utils_TskMultiMbxHndl * pMultiMbxHndl,
                        UInt8 *stackAddr,
                        UInt32 stackSize,
                        UInt32 tskPri,
                        UInt32 numPriQue,
                        char tskName[20]
        );

/**
 *******************************************************************************
 *
 * \brief Send message from one task to another task.
 *
 *        Refer to Utils_mbxSendMsg() for details
 *
 * \param  pFrom    [IN] Task Handle of source Task
 * \param  pTo      [IN] Task Handle of destination Task
 * \param  cmd      [IN] A 32 bit command
 * \param  pPrm     [IN] Message Handle
 * \param  msgFlags [OUT] Message Flags, like ACK enabled/disabled etc
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline Int32
Utils_tskSendMsg(Utils_TskHndl * pFrom,
                 Utils_TskHndl * pTo, UInt32 cmd, Void * pPrm, UInt32 msgFlags)
{
    return Utils_mbxSendMsg(&pFrom->mbx, &pTo->mbx, cmd, pPrm, msgFlags);
}

/**
 *******************************************************************************
 *
 * \brief Send 32-bit command to another task
 *
 *        Refer to Utils_mbxSendCmd() for details
 *
 * \param  pTo      [IN] Task Handle of destination Task
 * \param  cmd      [IN] A 32 bit command
 * \param  payload  [IN] Message body
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline Int32 Utils_tskSendCmd(Utils_TskHndl * pTo, UInt32 cmd,
                                     Void *payload)
{
    return Utils_mbxSendCmd(&pTo->mbx, cmd, payload);
}

/**
 *******************************************************************************
 *
 * \brief Wait for a message to arrive
 *
 *        Refer to Utils_mbxRecvMsg() for details
 *
 * \param  pHndl    [IN] Task Handle
 * \param  pMsg     [IN] Message Handle
 * \param  timeout  [IN] Timeout duration in milliseconds
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline Int32
Utils_tskRecvMsg(Utils_TskHndl * pHndl, Utils_MsgHndl ** pMsg, UInt32 timeout)
{
    return Utils_mbxRecvMsg(&pHndl->mbx, pMsg, timeout);
}

/**
 *******************************************************************************
 *
 * \brief ACK or free received message
 *
 *        Refer to Utils_mbxAckOrFreeMsg() for details
 *
 * \param  pMsg     [IN]  Message Handle
 * \param  result   [OUT] success or failure
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline Int32 Utils_tskAckOrFreeMsg(Utils_MsgHndl * pMsg, Int32 result)
{
    return Utils_mbxAckOrFreeMsg(pMsg, result);
}

/**
 *******************************************************************************
 *
 * \brief Wait until user specified command is received
 *
 *        Refer to Utils_mbxWaitCmd() for details
 *
 * \param  pHndl     [IN] Task Handle
 * \param  pMsg      [IN] Message Handle
 * \param  cmdToWait [IN] wait until this particular command received
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline Int32
Utils_tskWaitCmd(Utils_TskHndl * pHndl, Utils_MsgHndl ** pMsg, UInt32 cmdToWait)
{
    return Utils_mbxWaitCmd(&pHndl->mbx, pMsg, cmdToWait);
}

/**
 *******************************************************************************
 *
 * \brief Peek for a message
 *
 *         Refer to Utils_mbxRecvMsg() for details
 *
 * \param  pHndl    [IN] Task Handle
 * \param  pMsg     [IN] Message Handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline Int32
Utils_tskPeekMsg(Utils_TskHndl * pHndl, Utils_MsgHndl ** pMsg)
{
    return Utils_mbxPeekMsg(&pHndl->mbx, pMsg);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
