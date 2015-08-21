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
    \file utils_buf_ext.h
    \brief APIs defined in this file are used by links-and-chains example
     to exchange buffers between two tasks

    Internally this consists of two queues
    - empty or input queue
    - full or output queue

    The queue implementation uses fixed size array based queue data structure,
    with mutual exclusion built inside the queue implementation.

    Optional blocking of Get and/or Put operation is possible

    The element that can be inserted/extracted from the queue is of
    type System_Buffer *

    The basic operation in the example is as below

    - When a producer task needs to output some data, it first 'gets' a
       empty buffer to output the data from the buffer handle.
    - The task outputs the data to the empty buffer
    - The task then 'puts' this data as full data into the buffer handle
    - The consumer task, then 'gets' this full buffer from the buffer handle
    - After using or consuming this buffer, it 'puts' this buffer as empty
      buffer into this buffer handle.
    - This way buffers are exchanged between a producer and consumer.

 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_BUF_EXT_H_
#define _UTILS_BUF_EXT_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_buf.h>

/** Max number of channels/ resolution pools supported */
#define UTILS_BUF_MAX_ALLOC_POOLS     (16)

/**
    \brief Link private info,
           Mainly used for link internal dup feature support
*/
typedef struct {

    volatile UInt32 dupRefCount;
    /**< Dup reference count, this increases by every Dup */
    Void *pDupOrgFrame;
    /**< Original buffer pointer */
    Bool invalidFrame;
    /**< Flag to mark the frame is invalid,
         used in Dec channel delete feature */
    UInt32 allocPoolID;
    /**< Pool frame from which buf was originally alloced */

} Utils_EncDecLinkPvtInfo;

/**
    \brief Extended Buffer Handle.  This has a single Full_Q and
           channel specific Empty_Q
*/
typedef struct {

    UInt32 numAllocPools;
    /**< Number of allocator pools configured */

    Utils_QueHandle emptyQue[UTILS_BUF_MAX_ALLOC_POOLS];
    /**< Empty or input queue */

    Utils_QueHandle fullQue;
    /**< Full or output queue */

    System_Buffer *emptyQueMem[UTILS_BUF_MAX_ALLOC_POOLS][UTILS_BUF_MAX_QUE_SIZE];
    /**< Memory for empty que data - video buffers */

    System_Buffer *fullQueMem[UTILS_BUF_MAX_QUE_SIZE];
    /**< Memory for empty que data - video buffers */

} Utils_BufHndlExt;

/**
    \brief Create a buffer handle

    When blockOnGet/blockOnPut is TRUE a semaphore gets allocated internally.
    In order to reduce resource usuage keep this as FALSE if application
    doesnt plan to use the blocking API feature.

    \param pHndl        [OUT] Created handle
    \param blockOnGet   [IN]  Enable blocking on 'get' API
    \param blockOnPut   [IN]  Enable blocking on 'put' API
    \param numAllocPools [IN]  Number of allocator pools to create

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufCreateExt(Utils_BufHndlExt * pHndl, Bool blockOnGet,
                         Bool blockOnPut, UInt32 numAllocPools);

/**
    \brief Delete buffer handle

    Free's resources like semaphore allocated during create

    \param pHndl    [IN] Buffer handle

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufDeleteExt(Utils_BufHndlExt * pHndl);

/**
    \brief Print buffer status of full & empty queues

    \param str            [IN] prefix string to print
    \param pHndl        [IN] Buffer handle

    \return None
*/
Void Utils_bufExtPrintStatus(UInt8 *str, Utils_BufHndlExt * pHndl);

/**
    \brief Get buffer's from empty queue

    This API is used to get multiple buffers in a single API call.
    System_BufferList.numbuffers is set to number of buffers
    that are returned.

    When during create
    - 'blockOnGet' = TRUE
      - timeout can be BSP_OSAL_WAIT_FOREVER or BSP_OSAL_NO_WAIT
    - 'blockOnGet' = FALSE
      - timeout must be BSP_OSAL_NO_WAIT

    \param pHndl        [IN] Buffer handle
    \param pBufList     [OUT] buffers's returned by the API
    \param allocPoolID  [IN] Pool from which to alloc this buffer
    \param timeout      [IN] BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufGetEmptyExt(Utils_BufHndlExt * pHndl,
                           System_BufferList * pBufList,
                           UInt32 allocPoolID, UInt32 timeout);

/**
    \brief Put buffer's into full queue

    This API is used to return multiple buffers in a single API call.
    System_BufferList.numbuffers is set to number of buffers
    that are to be returned.

    When during create
    - 'blockOnPut' = TRUE
      - API will block until space is available in the queue to put the buffers
    - 'blockOnPut' = FALSE
      - API will return error in case space is not available in the queue
        to put the buffers

    \param pHndl        [IN] Buffer handle
    \param pBufList     [IN] buffer's to be put

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufPutFullExt(Utils_BufHndlExt * pHndl,
                          System_BufferList * pBufList);

/**
    \brief Get buffer's from full queue

    This API is used to get multiple buffer's in a single API call.
    System_BufferList.numbuffers is set to number of buffers
    that are returned.

    When during create
    - 'blockOnGet' = TRUE
      - timeout can be BSP_OSAL_WAIT_FOREVER or BSP_OSAL_NO_WAIT
    - 'blockOnGet' = FALSE
      - timeout must be BSP_OSAL_NO_WAIT

    \param pHndl        [IN] Buffer handle
    \param pBufList     [OUT] buffer's returned by the API
    \param timeout      [IN] BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufGetFullExt(Utils_BufHndlExt * pHndl,
                          System_BufferList * pBufList, UInt32 timeout);

/**
    \brief Put buffer's into empty queue

    This API is used to return multiple buffers in a single API call.
    System_BufferList.numbuffers is set to number of buffers
    that are to be returned.

    When during create
    - 'blockOnPut' = TRUE
      - API will block until space is available in the queue to put the buffers
    - 'blockOnPut' = FALSE
      - API will return error in case space is not available in the queue
        to put the buffers

    \param pHndl        [IN] Buffer handle
    \param pBufList     [IN] buffer's to be put

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufPutEmptyExt(Utils_BufHndlExt * pHndl,
                           System_BufferList * pBufList);

/**
    \brief Get a buffer from empty queue

    Same as Utils_bufGetEmpty() except that only a single buffer is returned

    \param pHndl        [IN] Buffer handle
    \param pBuf         [OUT] buffer that is returned by the API
    \param allocPoolID  [IN] Pool from which to alloc this buffer
    \param timeout      [IN] BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufGetEmptyBufferExt(Utils_BufHndlExt * pHndl,
                                 System_Buffer ** pBuf,
                                 UInt32 allocPoolID, UInt32 timeout);

/**
    \brief Get a buffer from full queue

    Same as Utils_bufGetFull() except that only a single buffer is returned

    \param pHndl        [IN] Buffer handle
    \param pBuf         [OUT] buffer that is returned by the API
    \param timeout      [IN] BSP_OSAL_NO_WAIT or BSP_OSAL_WAIT_FOREVER

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufGetFullBufferExt(Utils_BufHndlExt * pHndl,
                                System_Buffer ** pBuf, UInt32 timeout);

/**
    \brief Put a buffer into full queue

    Same as Utils_bufPutFull() except that only a single buffer is put

    \param pHndl        [IN] Buffer handle
    \param pBuf         [OUT] buffer that is to be returned to the queue

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufPutFullBufferExt(Utils_BufHndlExt * pHndl, System_Buffer * pBuf);

/**
    \brief Put a buffer into empty queue

    Same as Utils_bufPutEmpty() except that only a single buffer is put

    \param pHndl        [IN] Buffer handle
    \param pBuf         [OUT] buffer that is to be returned to the queue

    \return SYSTEM_LINK_STATUS_SOK on success, else failure
*/
Int32 Utils_bufPutEmptyBufferExt(Utils_BufHndlExt * pHndl, System_Buffer * pBuf);

/**
    \brief Peek into empty queue

    This only peeks at the top of the queue but does not remove the
    buffer from the queue

    \param pHndl        [IN] Buffer handle
    \param allocPoolID  [IN] Allocator pool ID to peek

    \return buffer pointer is buffer is present in the queue, else NULL
*/
static inline System_Buffer *Utils_bufPeekEmptyExt(Utils_BufHndlExt * pHndl,
                                                   UInt32 allocPoolID)
{
    System_Buffer *pBuf;

    UTILS_assert(allocPoolID < UTILS_BUF_MAX_ALLOC_POOLS);
    Utils_quePeek(&pHndl->emptyQue[allocPoolID], (Ptr *) & pBuf);

    return pBuf;
}

/**
    \brief Peek into full queue

    This only peeks at the top of the queue but does not remove the
    buffer from the queue

    \param pHndl        [IN] Buffer handle

    \return buffer pointer is buffer is present in the queue, else NULL
*/
static inline System_Buffer *Utils_bufPeekFullExt(Utils_BufHndlExt * pHndl)
{
    System_Buffer *pBuf;

    Utils_quePeek(&pHndl->fullQue, (Ptr *) & pBuf);

    return pBuf;
}

#endif

/* @} */


