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
 * \file utils_mbx.c
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
 * \version 0.0 First version
 * \version 0.1 Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_mbx.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 * \brief Maximum message pool
 *******************************************************************************
*/
#define UTILS_MBX_MSG_POOL_MAX   (1024)

/* @} */

/**
 *******************************************************************************
 * \brief Global variables for messages.
 *
 *        Memory to create the free messages queue, handle to free message Q
*         and pointer to free message pool
 *
 *******************************************************************************
 */
static Utils_QueHandle gUtils_mbxMsgPoolFreeQue;
static Ptr gUtils_mbxMsgPoolFreeQueMem[UTILS_MBX_MSG_POOL_MAX];
static Utils_MsgHndl gUtils_mbxMsgPool[UTILS_MBX_MSG_POOL_MAX];

/**
 *******************************************************************************
 *
 * \brief Gives a count of number of free messages
 *
 * \return  Count of number of free messages
 *
 *******************************************************************************
*/
UInt32 Utils_mbxGetFreeMsgCount()
{
    UInt32 cookie;
    UInt32 freeMsg;

    cookie = Hwi_disable();

    freeMsg = gUtils_mbxMsgPoolFreeQue.count;

    Hwi_restore(cookie);

    return freeMsg;
}

/**
 *******************************************************************************
 *
 * \brief Allocate mail box message
 *
 * \param  timeout [IN] Timeout value
 *
 * \return  A message handle is returned
 *
 *******************************************************************************
*/
Utils_MsgHndl *Utils_mbxAllocMsg(UInt32 timeout)
{
    Utils_MsgHndl *pMsg = NULL;

    Utils_queGet(&gUtils_mbxMsgPoolFreeQue, (Ptr *) & pMsg, 1, timeout);

    return pMsg;
}

/**
 *******************************************************************************
 *
 * \brief Free up a message
 *
 * \param  pMsg    [IN] Message Handle
 * \param  timeout [IN] Timeout value
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Utils_mbxFreeMsg(Utils_MsgHndl * pMsg, UInt32 timeout)
{
    Int32 status;

    status = Utils_quePut(&gUtils_mbxMsgPoolFreeQue, pMsg, timeout);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief One-time system init for mailbox subsystem
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Utils_mbxInit()
{
    Int32 status, msgId;

    status = Utils_queCreate(&gUtils_mbxMsgPoolFreeQue,
                             UTILS_MBX_MSG_POOL_MAX,
                             gUtils_mbxMsgPoolFreeQueMem,
                             UTILS_QUE_FLAG_BLOCK_QUE);

    if (status != SYSTEM_LINK_STATUS_SOK)
        return status;

    for (msgId = 0; msgId < UTILS_MBX_MSG_POOL_MAX; msgId++)
    {
        status = Utils_mbxFreeMsg(&gUtils_mbxMsgPool[msgId], BSP_OSAL_NO_WAIT);

        if (status != SYSTEM_LINK_STATUS_SOK)
        {
            Utils_mbxDeInit();
            break;
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Free's resources allocated during Utils_mbxInit()
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Utils_mbxDeInit()
{
    Int32 status;

    status = Utils_queDelete(&gUtils_mbxMsgPoolFreeQue);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Creates a message box and related resources
 *
 * \param pMbx    [OUT] Created handle pMbx
 * \param pPrm    [IN]  Create parameters
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
*/
Int32 Utils_mbxCreate(Utils_MbxHndl * pMbx, Utils_MbxCreatePrm *pPrm)
{
    Int32 status;
    Int i;

    pMbx->pTsk = pPrm->pTsk;
    pMbx->pFuncMultiMbxTrigger = pPrm->pFuncMultiMbxTrigger;

    /*
     * Create queues
     */
    status = Utils_queCreate(&pMbx->recvQue,
                             UTILS_MBX_RECV_QUE_LEN_MAX,
                             pMbx->memRecvQue,
                             UTILS_QUE_FLAG_BLOCK_QUE);

    if (status != SYSTEM_LINK_STATUS_SOK)
        return status;

    memset(pMbx->ackQue, 0, sizeof(pMbx->ackQue));

    for (i = 0; i < UTILS_MBX_ACK_QUE_CNT_MAX; i++)
    {
        status = Utils_queCreate(&(pMbx->ackQue[i]),
                                 UTILS_MBX_ACK_QUE_LEN_MAX,
                                 pMbx->memAckQue[i],
                                 UTILS_QUE_FLAG_BLOCK_QUE);

        if (status != SYSTEM_LINK_STATUS_SOK)
        {
            break;
        }
        pMbx->ackQueInUse[i] = FALSE;
    }

    if (i < UTILS_MBX_ACK_QUE_CNT_MAX)
    {
        /*
         * intial value for i is unmodified
         */
        for ( ; i >= 0; i--)
        {
            Utils_queDelete(&pMbx->ackQue[i]);
        }
        Utils_queDelete(&pMbx->recvQue);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Free's resources allocated during Utils_mbxCreate()
 *
 * \param pMbx    [IN] Mail box handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
*/
Int32 Utils_mbxDelete(Utils_MbxHndl * pMbx)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Int i;

    /*
     * Delete queues
     */
    status |= Utils_queDelete(&pMbx->recvQue);
    for (i = 0; i < UTILS_MBX_ACK_QUE_CNT_MAX; i++)
    {
        status |= Utils_queDelete(&(pMbx->ackQue[i]));
    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Send command to another mail box
 *
 *        Same as Utils_mbxSendMsg() except,
 *        - it can be called from interrupt context as well as task, SWI context
 *        - ACK message cannot received
 *
 * \param pTo      [IN] Receiver mail box handle
 * \param cmd      [IN] Command to be sent to the receiver
 * \param payload  [IN] Pointer to the payload to be sent to the receiver
 *
 * \return FVID2_EFAIL in case message could not be sent
 *
 *******************************************************************************
*/
Int32 Utils_mbxSendCmd(Utils_MbxHndl * pTo, UInt32 cmd, Void *payload)
{
    Utils_MsgHndl *pSentMsg;
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;

    if (pTo == NULL)
        return SYSTEM_LINK_STATUS_EFAIL;

    /* alloc message */
    pSentMsg = Utils_mbxAllocMsg(BSP_OSAL_NO_WAIT);
    if (pSentMsg == NULL)
    {
        Vps_printf(" UTILS: MBX: Utils_mbxSendCmd(): Msg Alloc Failed "
                   "(%d)!!!\n", Utils_mbxGetFreeMsgCount());
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    /* set message fields */
    pSentMsg->pFrom = NULL;
    pSentMsg->flags = 0;
    pSentMsg->cmd = cmd;
    pSentMsg->result = 0;
    pSentMsg->pPrm = payload;
    pSentMsg->ackQue = NULL;

    /* send message */
    retVal = Utils_quePut(&pTo->recvQue, pSentMsg, BSP_OSAL_NO_WAIT);

    if (retVal != 0)
    {
        retVal |= Utils_mbxFreeMsg(pSentMsg, BSP_OSAL_NO_WAIT);
    }

    if(pTo->pFuncMultiMbxTrigger)
    {
        retVal = pTo->pFuncMultiMbxTrigger(pTo->pTsk, BSP_OSAL_NO_WAIT);
    }

    return retVal;
}

/**
 *******************************************************************************
 *
 *  \brief Send message from one mailbox to another mailbox
 *
 *          When 'msgFlags' is UTILS_MBX_FLAG_WAIT_ACK,
 *          the function waits until an ack is received
 *
 *          When 'msgFlags' is 0,
 *          the function returns after message is sent to the receiver
 *
 *          User can use 'pPrm' to pass a parameter pointer to the receiver.
 *          Its upto user to manage the memory for this parameter pointer.
 *
 *          'cmd' can be any 32-bit value that is sent to the receiver
 *
 *          When UTILS_MBX_FLAG_WAIT_ACK is set,
 *          return value is the value sent via 'result' by the
 *          receiver when the receiver calls Utils_mbxAckOrFreeMsg()
 *
 *          When UTILS_MBX_FLAG_WAIT_ACK is not set,
 *          return value is message send status
 *
 *  \param pFrom    [IN] Sender Mail box handle
 *  \param pTo      [IN] Receiver mail box handle
 *  \param cmd      [IN] 32-bit command
 *  \param pPrm     [IN] 32-bit parameter pointer
 *  \param msgFlags [IN] UTILS_MBX_FLAG_xxxx
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success else failure
 *******************************************************************************
*/
Int32 Utils_mbxSendMsg(Utils_MbxHndl *pFrom,
                       Utils_MbxHndl *pTo,
                       UInt32         cmd,
                       Void          *pPrm,
                       UInt32         msgFlags)
{
    Utils_MsgHndl *pSentMsg, *pRcvMsg;
    Bool waitAck;
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    Utils_QueHandle *ackQue;
    Int i;
    UInt32 cookie;
    UInt32 ackQIdx = 0;

    if (pTo == NULL)
        return SYSTEM_LINK_STATUS_EFAIL;

    /*
     * set ACK que
     */
    if (pFrom == NULL)
    {
        /*
         * sender mailbox not specified by user
         */
        if (msgFlags & UTILS_MBX_FLAG_WAIT_ACK)
        {
            /*
             * ERROR: if sender mail box is NULL, then cannot wait for ACK
             */
            return SYSTEM_LINK_STATUS_EFAIL;
        }
        ackQue = NULL;
    }
    else
    {
        ackQue = NULL;
        cookie = Hwi_disable();
        for (i = 0; i < UTILS_MBX_ACK_QUE_CNT_MAX; i++)
        {
            if (pFrom->ackQueInUse[i] == FALSE)
            {
                /*
                 * sender mail box
                 */
                ackQue = &(pFrom->ackQue[i]);
                pFrom->ackQueInUse[i] = TRUE;
                break;
            }
        }
        Hwi_restore(cookie);
        if (i == UTILS_MBX_ACK_QUE_CNT_MAX)
        {
            return SYSTEM_LINK_STATUS_EFAIL;
        }
        UTILS_assert(ackQue != NULL);
    }

    /*
     * alloc message
     */
    pSentMsg = Utils_mbxAllocMsg(BSP_OSAL_WAIT_FOREVER);
    if (pSentMsg == NULL)
    {
        Vps_printf(" UTILS: MBX: Utils_mbxSendMsg(): Msg Alloc Failed "
                   "(%d)!!!\n", Utils_mbxGetFreeMsgCount());
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    /*
     * set message fields
     */
    pSentMsg->pFrom = pFrom;
    pSentMsg->flags = msgFlags;
    pSentMsg->cmd = cmd;
    pSentMsg->result = 0;
    pSentMsg->pPrm = pPrm;
    pSentMsg->ackQue = ackQue;

    /*
     * send message
     */
    retVal = Utils_quePut(&pTo->recvQue, pSentMsg, BSP_OSAL_WAIT_FOREVER);

    if (retVal != 0)
        return retVal;

    if(pTo->pFuncMultiMbxTrigger)
    {
        retVal = pTo->pFuncMultiMbxTrigger(pTo->pTsk, BSP_OSAL_WAIT_FOREVER);
    }

    if ((msgFlags & UTILS_MBX_FLAG_WAIT_ACK) && ackQue != NULL)
    {
        /*
         * need to wait for ACK
         */
        waitAck = TRUE;

        do
        {
            /*
             * wait for ACK
             */
            retVal = Utils_queGet(ackQue,
                                  (Ptr *) & pRcvMsg, 1, BSP_OSAL_WAIT_FOREVER);
            if (retVal != 0)
                return retVal;

            if (pRcvMsg == pSentMsg)
            {
                /*
                 * ACK received for sent MSG
                 */
                waitAck = FALSE;

                /*
                 * copy ACK status to return value
                 */
                retVal = pRcvMsg->result;
            }

            /*
             * else ACK received for some other message
             */
            else
            {
                Vps_printf(" UTILS: MBX: WARNING: Received unexpected ack msg"
                             ". Expected: %p, Received: %p !!!\n",
                             pSentMsg,
                             pRcvMsg);
            }

            /*
             * free message
             */
            retVal |= Utils_mbxFreeMsg(pRcvMsg, BSP_OSAL_WAIT_FOREVER);

        } while (waitAck);
    }

    if (ackQue)
    {
        cookie = Hwi_disable();
        ackQIdx = ackQue - &(pFrom->ackQue[0]);
        UTILS_assert((ackQIdx < UTILS_MBX_ACK_QUE_CNT_MAX)
                     && (pFrom->ackQueInUse[ackQIdx] == TRUE));
        pFrom->ackQueInUse[ackQIdx] = FALSE;
        Hwi_restore(cookie);
    }

    return retVal;
}

/**
 *******************************************************************************
 *
 *  \brief Waits for a message to arrive
 *
 *         When 'timeout' is BSP_OSAL_WAIT_FOREVER, it waits until atleast one
 *         message arrives
 *
 *         When 'timeout' is BSP_OSAL_NO_WAIT, it just checks for any available
 *         message. If no message is received then it return's with error
 *
 *         User MUST call Utils_mbxAckOrFreeMsg() for every received message,
 *         else message will not be free'ed and there will be memory leak.
 *
 *  \param pMbxHndl       [IN] Receiver mail box
 *  \param pMsg           [OUT] received message
 *  \param timeout        [IN] BSP_OSAL_WAIT_FOREVER or BSP_OSAL_NO_WAIT
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
*/
Int32 Utils_mbxRecvMsg(Utils_MbxHndl * pMbxHndl,
                       Utils_MsgHndl ** pMsg,
                       UInt32 timeout)
{
    Int32 retVal;

    /*
     * wait for message to arrive
     */
    retVal = Utils_queGet(&pMbxHndl->recvQue, (Ptr *) pMsg, 1, timeout);

    return retVal;
}

/**
 *******************************************************************************
 *
 *  \brief Peek message
 *
 *  \param pMbxHndl       [IN] Mail box Handle
 *  \param pMsg           [OUT] Message handle pointer
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
*/
Int32 Utils_mbxPeekMsg(Utils_MbxHndl * pMbxHndl,
                       Utils_MsgHndl ** pMsg)
{
    Int32 retVal;

    /*
     * wait for message to arrive
     */
    retVal = Utils_quePeek(&pMbxHndl->recvQue, (Ptr *) pMsg);

    return retVal;
}

/**
 *******************************************************************************
 *
 *  \brief Acks or frees a message depending on flags set in the message
 *
 *         If UTILS_MBX_FLAG_WAIT_ACK is set, then an ack message is sent
 *         to the sender
 *
 *         If UTILS_MBX_FLAG_WAIT_ACK is not set,
 *         then it frees the memory associated with this message
 *
 *         User MUST call this API for every message received using
 *         Utils_mbxRecvMsg() or Utils_mbxWaitCmd()
 *         else message will not be free'ed and there will be memory leak.
 *
 *  \param pMsg       [IN] Message to ACK'ed or free'ed
 *  \param ackRetVal     [IN] return code that is sent to the sender if an ack
 *                         message is sent to the sender
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
*/
Int32 Utils_mbxAckOrFreeMsg(Utils_MsgHndl * pMsg, Int32 ackRetVal)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;

    if (pMsg == NULL)
        return SYSTEM_LINK_STATUS_EFAIL;

    if (pMsg->flags & UTILS_MBX_FLAG_WAIT_ACK)
    {
        pMsg->result = ackRetVal;

        /*
         * Send ACK to sender
         */
        if (pMsg->pFrom == NULL)
        {
            retVal = -1;
            retVal |= Utils_mbxFreeMsg(pMsg, BSP_OSAL_WAIT_FOREVER);
            return retVal;
        }

        retVal = Utils_quePut(pMsg->ackQue, pMsg, BSP_OSAL_WAIT_FOREVER);
    }
    else
    {
        /*
         * free message
         */
        retVal = Utils_mbxFreeMsg(pMsg, BSP_OSAL_WAIT_FOREVER);
    }

    return retVal;
}

/**
 *******************************************************************************
 *
 *  \brief Waits until command of value 'cmdToWait' is received
 *
 *         If 'pMsg' is NULL it frees the msg internally when it is received
 *         and returns
 *
 *         If 'pMsg' is NOT NULL then it returns the received message to user.
 *         User needs to free the received message using Utils_mbxAckOrFreeMsg()
 *
 *  \param pMbxHndl       [IN] Receiver mail box
 *  \param pMsg       [OUT] received message
 *  \param waitCmd  [IN] command to wait for
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *******************************************************************************
*/
Int32 Utils_mbxWaitCmd(Utils_MbxHndl * pMbxHndl, Utils_MsgHndl ** pMsg,
                       UInt32 waitCmd)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Utils_MsgHndl *pRcvMsg;

    while (1)
    {
        /*
         * wait for message
         */
        status = Utils_mbxRecvMsg(pMbxHndl, &pRcvMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            return status;

        /*
         * Is message command ID same as expected command ID,
         * If yes, exit loop
         */
        if (Utils_msgGetCmd(pRcvMsg) == waitCmd)
            break;

        /*
         * no, ACK or free received message
         */
        status = Utils_mbxAckOrFreeMsg(pRcvMsg, 0);
        if (status != SYSTEM_LINK_STATUS_SOK)
            return status;
    }

    if (pMsg == NULL)
    {
        /* user does not want to examine the message, so free it here */
        status = Utils_mbxAckOrFreeMsg(pRcvMsg, 0);
    }
    else
    {
        /* user wants to examine the message to return it to user */
        *pMsg = pRcvMsg;
    }

    return status;
}
