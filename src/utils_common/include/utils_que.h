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
 * \defgroup UTILS_QUE_API Software Queue API
 *
 * \brief This module defines the APIs for a software queue
 *
 *        This software queue is used to build multiple software constructs
 *        like message box, buffer queue.
 *
 *        The software queue implmenetation is a fixed size arrary based queue
 *        for simplicity and performance. The APIs take care fo mutual
 *        exclusion protection via interuupt locks. The API optionally support
 *        blocking 'get' and 'put' APIs
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_que.h
 *
 * \brief Software Queue API
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_QUE_H_
#define _UTILS_QUE_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 * \brief Queue Flag: Do not block on que get and que put,
 *******************************************************************************
 */
#define UTILS_QUE_FLAG_NO_BLOCK_QUE    (0x00000000)

/**
 *******************************************************************************
 * \brief Queue Flag: Block on que put if que is full
 *******************************************************************************
 */
#define UTILS_QUE_FLAG_BLOCK_QUE_PUT   (0x00000001)

/**
 *******************************************************************************
 * \brief Queue Flag: Block on que get if que is empty
 *******************************************************************************
 */
#define UTILS_QUE_FLAG_BLOCK_QUE_GET   (0x00000002)

/**
 *******************************************************************************
 * \brief Queue Flag: Block on que put if que is full, Block on que get if que
 *                    is empty
 *******************************************************************************
 */
#define UTILS_QUE_FLAG_BLOCK_QUE       (0x00000003)

/* @} */

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
 * \brief Queue Handle
 *
 *        Typically user does not need to know internals of queue handle
 *        data structure
 *******************************************************************************
*/
typedef struct {

  UInt32 curRd;
  /**< Current read index */

  UInt32 curWr;
  /**< Current write index  */

  UInt32 count;
  /**< Count of element in queue  */

  UInt32 maxElements;
  /**< Max elements that be present in the queue  */

  Ptr *queue;
  /**< Address of data area of the queue elements */

  BspOsal_SemHandle semRd;
  /**< Read semaphore */

  BspOsal_SemHandle semWr;
  /**< Write semaphore  */

  UInt32 flags;
  /**< Controls how APIs behave internally, i.e blocking wait or non-blocking */

  volatile Bool blockedOnGet;
  /**< Flag indicating queue is blocked on get operation */

  volatile Bool blockedOnPut;
  /**< Flag indicating queue is blocked on put operation */

  volatile Bool forceUnblockGet;
  /**< Flag indicating forced unblock of queueGet */

  volatile Bool forceUnblockPut;
  /**< Flag indicating forced unblock of queuePut */

} Utils_QueHandle;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32  Utils_queCreate(Utils_QueHandle * handle,
                      UInt32 maxElements, Ptr queueMem, UInt32 flags);

Int32  Utils_queDelete(Utils_QueHandle * handle);

Int32  Utils_quePut(Utils_QueHandle * handle, Ptr data, Int32 timeout);

Int32  Utils_queGet(Utils_QueHandle * handle,
                   Ptr * data, UInt32 minCount, Int32 timeout);

Int32  Utils_quePeek(Utils_QueHandle * handle, Ptr * data);

UInt32 Utils_queIsEmpty(Utils_QueHandle * handle);

UInt32 Utils_queIsFull(Utils_QueHandle * handle);

UInt32 Utils_queGetQueuedCount(Utils_QueHandle * handle);

Int32  Utils_queUnBlock(Utils_QueHandle * handle);

Int32  Utils_queReset(Utils_QueHandle * handle);

#endif

/* @} */
