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
 * \defgroup UTILS_IPC_QUE_API IPC Software Queue API
 *
 * \brief This module defines the APIs for a software queue which is used for
 *        inter processor communication
 *
 *        The software queue implmentation is a fixed size, NON-LOCKING
 *        array based queue for simplicity and performance.
 *
 *        The APIs take care fo mutual exclusion protection via interuupt locks
 *        for threads running on the same processor.
 *
 *        The API DOES NOT support blocking 'get' and 'put' APIs. This needs to
 *        be taken care by the user of the API
 *
 *        Since the implementation does not take inter processors locks, it
 *        is important that there be exactly one CPU which is the "writer"
 *        and exactly one CPU which is the "reader"
 *
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_ipc_que.h
 *
 * \brief IPC Software Queue API
 *
 * \version 0.0 (May 2013) : [YM] First version
 *
 *******************************************************************************
 */

#ifndef _OSA_IPC_QUE_H_
#define _OSA_IPC_QUE_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <osa_mutex.h>
#include <include/link_api/system_const.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


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
 * \brief Data structure representing IPC Queue in shared memory
 *
 *        Typically user does not need to know internals of this
 *        data structure
 *******************************************************************************
*/
typedef struct {

  volatile UInt32 curRd;
  /**< Current read index */

  volatile UInt32 curWr;
  /**< Current write index  */

  volatile UInt32 elementSize;
  /**< Size of individual element in units of bytes */

  volatile UInt32 maxElements;
  /**< Max elements that be present in the queue  */

} OSA_IpcQueSharedMemObj;


/**
 *******************************************************************************
 * \brief Queue Handle
 *
 *        Typically user does not need to know internals of queue handle
 *        data structure
 *******************************************************************************
*/
typedef struct {

  OSA_MutexHndl mutexLock;
  /**< Size of individual element in units of bytes */

  UInt32 elementSize;
  /**< Size of individual element in units of bytes */

  UInt32 maxElements;
  /**< Max elements that be present in the queue  */

  Ptr    sharedMemBaseAddr;
  /**< Address of shared memory area of the queue */

} OSA_IpcQueHandle;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32  OSA_ipcQueCreate(OSA_IpcQueHandle * handle,
                            UInt32 maxElements,
                            Ptr sharedMemBaseAddr,
                            UInt32 elementSize);

Int32  OSA_ipcQueReset(OSA_IpcQueHandle * handle,
                            Ptr sharedMemBaseAddr,
                            Bool resetRdIdx,
                            Bool resetWrIdx
                        );

Int32  OSA_ipcQueDelete(OSA_IpcQueHandle * handle);

Int32  OSA_ipcQueWrite(OSA_IpcQueHandle * handle,
                            volatile UInt8 *data,
                            volatile UInt32 dataSize);

Int32  OSA_ipcQueRead(OSA_IpcQueHandle * handle,
                            volatile UInt8 *data,
                            volatile UInt32 dataSize);

UInt32 OSA_ipcQueIsEmpty(OSA_IpcQueHandle * handle);

UInt32 OSA_ipcQueIsFull(OSA_IpcQueHandle * handle);

#endif

/* @} */
