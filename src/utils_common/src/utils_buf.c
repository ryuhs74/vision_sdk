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
 * \file utils_buf.c
 *
 * \brief This file has the implementation of the system buffer exchange queue
 *        API
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_buf.h>

/**
 *******************************************************************************
 *
 * \brief Create a system buffer handle
 *
 *        When blockOnGet/blockOnPut is TRUE a semaphore gets allocated
 *        internally. In order to reduce resource usuage keep this as FALSE
 *        if application does not plan to use the blocking API feature.
 *
 * \param pHndl        [OUT] Created handle
 * \param blockOnGet   [IN]  Enable blocking on 'get' API
 * \param blockOnPut   [IN]  Enable blocking on 'put' API
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufCreate(Utils_BufHndl * pHndl, Bool blockOnGet, Bool blockOnPut)
{
    Int32 status;
    UInt32 flags;

    flags = UTILS_QUE_FLAG_NO_BLOCK_QUE;

    if (blockOnGet)
        flags |= UTILS_QUE_FLAG_BLOCK_QUE_GET;
    if (blockOnPut)
        flags |= UTILS_QUE_FLAG_BLOCK_QUE_PUT;

    status = Utils_queCreate(&pHndl->emptyQue,
                             UTILS_BUF_MAX_QUE_SIZE, pHndl->emptyQueMem, flags);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_queCreate(&pHndl->fullQue,
                             UTILS_BUF_MAX_QUE_SIZE, pHndl->fullQueMem, flags);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete system buffer handle
 *
 *        Free's resources like semaphore allocated during create
 *
 * \param pHndl    [IN] Buffer handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufDelete(Utils_BufHndl * pHndl)
{
    Utils_queDelete(&pHndl->emptyQue);
    Utils_queDelete(&pHndl->fullQue);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get system buffers from empty queue
 *
 *        This API is used to get multiple empty buffers in a single API call.
 *        When timeout == BSP_OSAL_NO_WAIT,
 *             maximum possible buffer are returned.
 *        When timeout != BSP_OSAL_NO_WAIT,
 *             upto max pBufList->numBuf are returned.
 *
 *        On return pBufList->numBuf is set to number of buffers
 *        that are returned.
 *
 *        When during create
 *        - 'blockOnGet' = TRUE
 *            - timeout can be BSP_OSAL_WAIT_FOREVER or BSP_OSAL_NO_WAIT or
 *              amount of time in units of OS ticks that it should block
 *        - 'blockOnGet' = FALSE
 *            - timeout must be BSP_OSAL_NO_WAIT
 *
 * \param pHndl        [IN]     Buffer handle
 * \param pBufList     [IN/OUT] Buffers returned by the API
 * \param timeout      [IN]     BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER or
 *                              amount of time in units of OS ticks that
 *                              it should block
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufGetEmpty(Utils_BufHndl * pHndl, System_BufferList * pBufList,
                        UInt32 timeout)
{
    UInt32 idx, maxBufs;
    Int32  status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);

    if (timeout == BSP_OSAL_NO_WAIT)
        maxBufs = SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST;
    else
        maxBufs = pBufList->numBuf;

    UTILS_assert(maxBufs <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < maxBufs; idx++)
    {
        status =
            Utils_queGet(&pHndl->emptyQue, (Ptr *) & pBufList->buffers[idx], 1,
                         timeout);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;
    }

    pBufList->numBuf = idx;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get a system buffer from empty queue
 *
 *        Same as Utils_bufGetEmpty() except that only a single buffer is
 *        returned
 *
 * \param pHndl        [IN]  Buffer handle
 * \param pBuf         [OUT] Buffer that is returned by the API
 * \param timeout      [IN]  BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER or
 *                           amount of time in units of OS ticks that
 *                           it should block
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufGetEmptyBuffer(Utils_BufHndl * pHndl,
                              System_Buffer ** pBuf, UInt32 timeout)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf != NULL);

    *pBuf = NULL;

    status = Utils_queGet(&pHndl->emptyQue, (Ptr *) pBuf, 1, timeout);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Get number of system buffers available in the empty queue
 *
 * \param pHndl        [IN] Buffer handle
 *
 * \return Empty buffers count
 *
 *******************************************************************************
 */
UInt32 Utils_bufGetEmptyBufferCount(Utils_BufHndl * pHndl)
{
    UInt32 emptyBufCnt;

    UTILS_assert(pHndl != NULL);
    emptyBufCnt = Utils_queGetQueuedCount(&pHndl->emptyQue);

    return emptyBufCnt;
}

/**
 *******************************************************************************
 *
 * \brief Put system buffers into empty queue
 *
 *        This API is used to put multiple buffers in the empty queue in a
 *        single API call. pBufList->numBuf is set to number of buffers
 *        that are to be returned.
 *
 * \param pHndl        [IN] Buffer handle
 * \param pBufList     [IN] Buffers to be put
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufPutEmpty(Utils_BufHndl * pHndl, System_BufferList * pBufList)
{
    UInt32 idx;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBuf <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < pBufList->numBuf; idx++)
    {
        status =
            Utils_quePut(&pHndl->emptyQue, pBufList->buffers[idx],
                         BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Put a system buffer into empty queue
 *
 *        Same as Utils_bufPutEmpty() except that only a single buffer is put
 *
 * \param pHndl        [IN] Buffer handle
 * \param pBuf         [OUT] Buffer that is to be returned to the queue
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufPutEmptyBuffer(Utils_BufHndl * pHndl, System_Buffer * pBuf)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);

    status = Utils_quePut(&pHndl->emptyQue, pBuf, BSP_OSAL_NO_WAIT);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get system buffers from full queue
 *
 *        This API is used to get multiple full buffers in a single API call.
 *        When timeout == BSP_OSAL_NO_WAIT,
 *             maximum possible buffer are returned.
 *        When timeout != BSP_OSAL_NO_WAIT,
 *             upto max pBufList->numBuf are returned.
 *
 *        On return pBufList->numBuf is set to number of buffers
 *        that are returned.
 *
 *        When during create
 *        - 'blockOnGet' = TRUE
 *            - timeout can be BSP_OSAL_WAIT_FOREVER or BSP_OSAL_NO_WAIT or
 *              amount of time in units of OS ticks that it should block
 *        - 'blockOnGet' = FALSE
 *            - timeout must be BSP_OSAL_NO_WAIT
 *
 * \param pHndl        [IN]     Buffer handle
 * \param pBufList     [IN/OUT] Buffers returned by the API
 * \param timeout      [IN]     BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER or
 *                              amount of time in units of OS ticks that
 *                              it should block
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufGetFull(Utils_BufHndl * pHndl, System_BufferList * pBufList,
                       UInt32 timeout)
{
    UInt32 idx, maxBufs;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);

    if (timeout == BSP_OSAL_NO_WAIT)
        maxBufs = SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST;
    else
        maxBufs = pBufList->numBuf;

    UTILS_assert(maxBufs <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < maxBufs; idx++)
    {
        status =
            Utils_queGet(&pHndl->fullQue, (Ptr *) & pBufList->buffers[idx], 1,
                         timeout);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;
    }

    pBufList->numBuf = idx;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get number of system buffers available in the full queue
 *
 * \param pHndl        [IN] Buffer handle
 *
 * \return Full buffers count
 *
 *******************************************************************************
 */
UInt32 Utils_bufGetFullBufferCount(Utils_BufHndl * pHndl)
{
    UInt32 fullBufCnt;

    UTILS_assert(pHndl != NULL);
    fullBufCnt = Utils_queGetQueuedCount(&pHndl->fullQue);

    return fullBufCnt;
}

/**
 *******************************************************************************
 *
 * \brief Get a system buffer from full queue
 *
 *        Same as Utils_bufGetFull() except that only a single buffer is
 *        returned
 *
 * \param pHndl        [IN]  Buffer handle
 * \param pBuf         [OUT] Buffer that is returned by the API
 * \param timeout      [IN]  BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER or
 *                           amount of time in units of OS ticks that
 *                           it should block
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufGetFullBuffer(Utils_BufHndl * pHndl,
                            System_Buffer ** pBuf, UInt32 timeout)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf != NULL);

    *pBuf = NULL;

    status = Utils_queGet(&pHndl->fullQue, (Ptr *) pBuf, 1, timeout);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Put system buffers into full queue
 *
 *        This API is used to put multiple buffers in the full queue in a
 *        single API call. pBufList->numBuf is set to number of buffers
 *        that are to be returned.
 *
 * \param pHndl        [IN] Buffer handle
 * \param pBufList     [IN] Buffers to be put
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufPutFull(Utils_BufHndl * pHndl, System_BufferList * pBufList)
{
    UInt32 idx;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBuf <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < pBufList->numBuf; idx++)
    {
        status =
            Utils_quePut(&pHndl->fullQue, pBufList->buffers[idx],
                         BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Put a system buffer into full queue
 *
 *        Same as Utils_bufPutFull() except that only a single buffer is put
 *
 * \param pHndl        [IN] Buffer handle
 * \param pBuf         [OUT] Buffer that is to be returned to the queue
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_bufPutFullBuffer(Utils_BufHndl * pHndl, System_Buffer * pBuf)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);

    status = Utils_quePut(&pHndl->fullQue, pBuf, BSP_OSAL_NO_WAIT);
    if (status != SYSTEM_LINK_STATUS_SOK)
    {
#if 1
        Vps_printf
            (" UTILS: BUF: ERROR: In Utils_bufPutFullBuffer(), Utils_quePut() failed !!!\n");
#endif
    }

    return status;
}


/**
 *******************************************************************************
 *
 * \brief Initialize the given FVID2 Frame using the given System_Buffer
 *
 *        The links communicate with each other using the System_Buffer.
 *        However, the driver interface uses FVID2_Frame. Links that interface
 *        with FVID2 drivers need to convert System_Buffer from/to FVID2_Frame.
 *
 *        This function converts the given System_Buffer into FVID2_Frame.
 *
 * \param pFrame       [OUT] FVID2 Frame to be initialized
 * \param pBuffer      [IN] System_Buffer used to initialize the frame
 *
 *   \return SYSTEM_LINK_SOK on success
 *
 *******************************************************************************
 */
Int32 Utils_bufInitFrame(FVID2_Frame *pFrame, System_Buffer *pBuffer)
{
    UInt32 idx, planes;
    System_VideoFrameBuffer *pVideoFrame;

    UTILS_assert(pBuffer != NULL);
    UTILS_assert(pBuffer->bufType == SYSTEM_BUFFER_TYPE_VIDEO_FRAME);


    pVideoFrame = pBuffer->payload;
    UTILS_assert(pVideoFrame != NULL);

    UTILS_assert(UTILS_ARRAYSIZE(pVideoFrame->bufAddr) >=
                 UTILS_ARRAYSIZE(pFrame->addr[0]));

    memset(pFrame, 0, sizeof(*pFrame));

    pFrame->chNum       = pBuffer->chNum;
    pFrame->timeStamp   = pBuffer->linkLocalTimestamp;
    pFrame->appData     = pBuffer;

    planes = UTILS_ARRAYSIZE(pFrame->addr[0]);

    for (idx = 0; idx < planes; idx++)
    {
        pFrame->addr[0][idx] = pVideoFrame->bufAddr[idx];
    }

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Print buffer status of full & empty queues
 *
 * \param str            [IN] prefix string to print
 * \param pHndl          [IN] Buffer handle
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_bufPrintStatus(UInt8 *str, Utils_BufHndl * pHndl)
{
    Vps_printf(" [%s] Buffer Q Status,\n", str);
    Vps_printf(" Empty Q :"
               " Elements in Q = %3d, Write Idx = %3d, Read Idx = %3d\n",
                    pHndl->emptyQue.count,
                    pHndl->emptyQue.curWr,
                    pHndl->emptyQue.curRd);
    Vps_printf(" Full  Q :"
               " Elements in Q = %3d, Write Idx = %3d, Read Idx = %3d\n",
                    pHndl->fullQue.count,
                    pHndl->fullQue.curWr,
                    pHndl->fullQue.curRd);
}


