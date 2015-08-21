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
 * \defgroup UTILS_BUF_API System Buffer Exchange API
 *
 * \brief This module define APIs to exchange system buffers between two tasks
 *        or links
 *
 *        Internally a buffer queue this consists of two queues
 *        - empty or input queue
 *        - full or output queue
 *
 *        The queue implementation uses fixed a size array based queue
 *        data structure, with mutual exclusion protection built inside the
 *        queue implementation.
 *
 *        Optional blocking of Get and/or Put operation is possible
 *
 *        The element that can be inserted/extracted from the queue is of
 *        type System_Buffer *
 *
 *        The basic operation is as below
 *
 *        - When a producer task needs to output some data, it firsts 'gets'
 *          an empty system buffer to output the data from the buffer handle.
 *        - The task outputs the data to the empty system buffer
 *        - The task then 'puts' this data as full data into the buffer handle
 *        - The consumer task, then 'gets' this full buffer from the buffer
 *          handle
 *        - After using or consuming this buffer, it 'puts' this buffer
 *          as empty buffer into this buffer handle.
 *        - This way buffers are exchanged between a producer and consumer.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_buf.h
 *
 * \brief System Buffer exchange API
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_BUF_H_
#define _UTILS_BUF_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_que.h>
#include <include/link_api/system_inter_link_api.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Maximum elements that can reside in a buffer queue
 *******************************************************************************
 */
#define UTILS_BUF_MAX_QUE_SIZE       (384)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief System Buffer Handle
 *
 *******************************************************************************
*/
typedef struct {

    Utils_QueHandle emptyQue;
    /**< Empty or input queue */

    Utils_QueHandle fullQue;
    /**< Full or output queue */

    System_Buffer *emptyQueMem[UTILS_BUF_MAX_QUE_SIZE];
    /**< Memory for empty queue elements */

    System_Buffer *fullQueMem[UTILS_BUF_MAX_QUE_SIZE];
    /**< Memory for full queue elements */

} Utils_BufHndl;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32  Utils_bufCreate(Utils_BufHndl * pHndl, Bool blockOnGet, Bool blockOnPut);

Int32  Utils_bufDelete(Utils_BufHndl * pHndl);

Int32  Utils_bufGetEmpty(Utils_BufHndl * pHndl,
                        System_BufferList * pBufList, UInt32 timeout);

Int32  Utils_bufGetEmptyBuffer(Utils_BufHndl * pHndl,
                              System_Buffer ** pBuf, UInt32 timeout);

Int32  Utils_bufPutEmpty(Utils_BufHndl * pHndl, System_BufferList * pBufList);

Int32  Utils_bufPutEmptyBuffer(Utils_BufHndl * pHndl, System_Buffer * pBuf);

UInt32 Utils_bufGetEmptyBufferCount(Utils_BufHndl * pHndl);

/**
 *******************************************************************************
 *
 * \brief Peek into empty queue
 *
 *        This only peeks at the top of the queue but does not remove the
 *        buffer from the queue
 *
 * \param pHndl        [IN] Buffer handle
 *
 * \return buffer pointer if buffer is present in the empty queue, else NULL
 *
 *******************************************************************************
 */
static inline System_Buffer *Utils_bufPeekEmpty(Utils_BufHndl * pHndl)
{
    System_Buffer *pBuf;

    Utils_quePeek(&pHndl->emptyQue, (Ptr *) & pBuf);

    return pBuf;
}

Int32  Utils_bufGetFull(Utils_BufHndl * pHndl,
                       System_BufferList * pBufList, UInt32 timeout);

Int32  Utils_bufGetFullBuffer(Utils_BufHndl * pHndl,
                             System_Buffer ** pBuf, UInt32 timeout);

Int32  Utils_bufPutFull(Utils_BufHndl * pHndl, System_BufferList * pBufList);

Int32  Utils_bufPutFullBuffer(Utils_BufHndl * pHndl, System_Buffer * pBuf);

UInt32 Utils_bufGetFullBufferCount(Utils_BufHndl * pHndl);

/**
 *******************************************************************************
 *
 * \brief Peek into full queue
 *
 *        This only peeks at the top of the queue but does not remove the
 *        buffer from the queue
 *
 * \param pHndl        [IN] Buffer handle
 *
 * \return buffer pointer if buffer is present in the full queue, else NULL
 *
 *******************************************************************************
*/
static inline System_Buffer *Utils_bufPeekFull(Utils_BufHndl * pHndl)
{
    System_Buffer *pBuf;

    Utils_quePeek(&pHndl->fullQue, (Ptr *) & pBuf);

    return pBuf;
}

Void   Utils_bufPrintStatus(UInt8 *str, Utils_BufHndl * pHndl);

Int32  Utils_bufInitFrame(FVID2_Frame *pFrame, System_Buffer *pBuffer);

#endif

/* @} */

