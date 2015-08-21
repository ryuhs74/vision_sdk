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
 * \file utils_ipc_que.c
 *
 * \brief  This file implements the OSA_IPC_QUE_API APIs
 *
 * \version 0.0 (May 2014) : [YM] First version taken from utils_common on bios 
                                  side
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <osa_ipc_que.h>


/**
 *******************************************************************************
 *
 * \brief Create a IPC queue handle
 *
 *        The size of memory pointed by sharedMemBaseAddr allocated by the user
 *        - should atleast be
 *            maxElements*elementSize + sizeof(OSA_IpcQueSharedMemObj)
 *        - it should be allocated in a non-cache region (for both CPUs)
 *          which is visible to both the CPUs acessing this memory
 *
 * \param handle             [OUT] Initialized queue handle
 * \param maxElements        [IN]  Maximum elements that can reside in the queue
 *                                 at any given point of time
 * \param sharedMemBaseAddr  [IN]  Address of queue data area
 * \param elementSize        [IN]  Size of each element of the queue
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32  OSA_ipcQueCreate(OSA_IpcQueHandle * handle,
                            UInt32 maxElements,
                            Ptr sharedMemBaseAddr,
                            UInt32 elementSize)
{
    Int32 status;
    volatile OSA_IpcQueSharedMemObj *pShm;

    if(     handle==NULL
        ||  maxElements == 0
        ||  sharedMemBaseAddr == NULL
        ||  elementSize == 0
        )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    /* set values in local handle */
    handle->elementSize = elementSize;
    handle->maxElements = maxElements;
    handle->sharedMemBaseAddr = sharedMemBaseAddr;

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    
    status = OSA_mutexCreate(&(handle->mutexLock));
    OSA_assert(status == OSA_SOK);

    OSA_mutexLock(&(handle->mutexLock));

    /* reset read and write index in shared memory handle */
    pShm->curRd = 0;
    pShm->curWr = 0;

    /* set element size and max number of elements in shared memory handle */
    pShm->elementSize = handle->elementSize;
    pShm->maxElements = handle->maxElements;

    OSA_mutexUnlock(&(handle->mutexLock));

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Reset local handle and optionally reset IPC shared memory
 *        queue index's
 *
 * \param handle             [OUT] Initialized queue handle
 * \param sharedMemBaseAddr  [IN]  Address of queue data area
 * \param resetRdIdx         [IN]  TRUE: reset read index,
 *                                 FALSE: read index not modified
 * \param resetWrIdx         [IN]  TRUE: reset write index,
 *                                 FALSE: write index not modified
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32  OSA_ipcQueReset(OSA_IpcQueHandle * handle,
                            Ptr sharedMemBaseAddr,
                            Bool resetRdIdx,
                            Bool resetWrIdx
                        )
{
    
    volatile OSA_IpcQueSharedMemObj *pShm;
    volatile UInt32 value;

    if(     handle==NULL
        ||  sharedMemBaseAddr == NULL
        )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    /* set values in local handle */
    handle->sharedMemBaseAddr = sharedMemBaseAddr;

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    /*
     * set element size and max number of elements in local handle
     * from shared memory handle
     */
    handle->elementSize = pShm->elementSize;
    handle->maxElements = pShm->maxElements;

    if(     handle->maxElements == 0
        ||  handle->elementSize == 0
        )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    OSA_mutexLock(&(handle->mutexLock));

    /*
     * reset read and write index in shared memory handle,
     * if requested by user
     */
    if(resetRdIdx)
        pShm->curRd = 0;

    if(resetWrIdx)
        pShm->curWr = 0;

    /* dummy read to ensure write to shared memory has completed */
    value = pShm->curWr;
    (void)value;

    OSA_mutexUnlock(&(handle->mutexLock));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Free resource's allocated during create
 *
 * \param handle             [IN]  queue handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32  OSA_ipcQueDelete(OSA_IpcQueHandle * handle)
{
    
    volatile OSA_IpcQueSharedMemObj *pShm;
    volatile UInt32 value;

    if(    handle == NULL
        || handle->sharedMemBaseAddr == NULL
      )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    OSA_mutexLock(&(handle->mutexLock));

    /*
     * set element size and max number of elements in shared memory handle
     * to 0
     */
    pShm->elementSize = 0;
    pShm->maxElements = 0;

    /* dummy read to ensure write to shared memory has completed */
    value = pShm->curWr;
    (void)value;

    OSA_mutexUnlock(&(handle->mutexLock));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Write data into the queue
 *
 *        'dataSize' MUST be <= to elementSize set during queue create
 *
 *        Calling OSA_ipcQueWrite() once is equivalent to adding one element
 *        in the queue, even if dataSize < elementSize.
 *
 * \param handle             [IN]  queue handle
 * \param data               [IN]  local buffer from where data is written to
 *                                 the queue
 * \param dataSize           [IN]  amount of data to write to the queue
 *
 * \return TRUE, queue is empty
 *         FALSE, queue is not empty
 *
 *******************************************************************************
 */
Int32  OSA_ipcQueWrite(OSA_IpcQueHandle * handle,
                            volatile UInt8 *data,
                            volatile UInt32 dataSize)
{
    
    volatile OSA_IpcQueSharedMemObj *pShm;
    volatile UInt8 *pWrite;
    volatile UInt32 writeIdx;

    if(    handle == NULL
        || handle->sharedMemBaseAddr == NULL
        || dataSize > handle->elementSize
      )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    if(     pShm->maxElements != handle->maxElements
        ||  pShm->elementSize != handle->elementSize
        )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    if(OSA_ipcQueIsFull(handle))
    {
        /* if queue is empty then return */
        return SYSTEM_LINK_STATUS_EAGAIN;
    }

    /* queue is not empty, read one element from the queue */
    OSA_mutexLock(&(handle->mutexLock));

    writeIdx = pShm->curWr;

    pWrite =  (UInt8*)handle->sharedMemBaseAddr
            + sizeof(OSA_IpcQueSharedMemObj)
            + writeIdx*handle->elementSize;

    memcpy((void*)pWrite, (void*)data, dataSize);

    /* move readIdx */
    pShm->curWr = (writeIdx+1)%handle->maxElements;

    /* dummy readback to ensure idx is written to memory */
    writeIdx = pShm->curWr;

    OSA_mutexUnlock(&(handle->mutexLock));

    return SYSTEM_LINK_STATUS_SOK;

}

/**
 *******************************************************************************
 *
 * \brief Read data from the queue
 *
 *        'dataSize' MUST be <= to elementSize set during queue create
 *
 *        Reader should aware before hand what is size of element that it needs
 *        to read
 *
 *        Calling OSA_ipcQueRead() once is equivalent to removing  one element
 *        from the queue, even if dataSize < elementSize.
 *
 *        It is users responsibility to ensure it does not read partial elements
 *
 * \param handle             [IN]  queue handle
 * \param data               [IN]  local buffer into which data is read
 * \param dataSize           [IN]  amount of data to read from the queue
 *
 * \return TRUE, queue is empty
 *         FALSE, queue is not empty
 *
 *******************************************************************************
 */
Int32  OSA_ipcQueRead(OSA_IpcQueHandle * handle,
                            volatile UInt8 *data,
                            volatile UInt32 dataSize)
{
    
    volatile OSA_IpcQueSharedMemObj *pShm;
    volatile UInt8 *pRead;
    volatile UInt32 readIdx;

    if(    handle == NULL
        || handle->sharedMemBaseAddr == NULL
        || dataSize > handle->elementSize
      )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    if(     pShm->maxElements != handle->maxElements
        ||  pShm->elementSize != handle->elementSize
        )
    {
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    if(OSA_ipcQueIsEmpty(handle))
    {
        /* if queue is empty then return */
        return SYSTEM_LINK_STATUS_EAGAIN;
    }

    /* queue is not empty, read one element from the queue */
    OSA_mutexLock(&(handle->mutexLock));

    readIdx = pShm->curRd;

    pRead =   (UInt8*)handle->sharedMemBaseAddr
            + sizeof(OSA_IpcQueSharedMemObj)
            + readIdx*handle->elementSize;

    memcpy((void*)data, (void*)pRead, dataSize);

    /* move readIdx */
    pShm->curRd = (readIdx+1)%handle->maxElements;

    /* dummy readback to ensure idx is written to memory */
    readIdx = pShm->curRd;

    OSA_mutexUnlock(&(handle->mutexLock));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Check if queue is empty
 *
 * \param handle             [IN]  queue handle
 *
 * \return TRUE, queue is empty
 *         FALSE, queue is not empty
 *
 *******************************************************************************
 */
UInt32 OSA_ipcQueIsEmpty(OSA_IpcQueHandle * handle)
{
    
    volatile UInt32 readIdx, writeIdx, numEmpty;
    volatile OSA_IpcQueSharedMemObj *pShm;

    if(    handle == NULL
        || handle->sharedMemBaseAddr == NULL
      )
    {
        return FALSE;
    }

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    OSA_mutexLock(&(handle->mutexLock));

    readIdx = pShm->curRd;
    writeIdx = pShm->curWr;

    if(writeIdx < readIdx)
        numEmpty = readIdx - writeIdx;
    else
        numEmpty = (handle->maxElements - writeIdx) + readIdx;

    OSA_mutexUnlock(&(handle->mutexLock));

    return (numEmpty == handle->maxElements) ? TRUE : FALSE;
}

/**
 *******************************************************************************
 *
 * \brief Check if queue is full
 *
 * \param handle             [IN]  queue handle
 *
 * \return TRUE, queue is full
 *         FALSE, queue is not full
 *
 *******************************************************************************
 */
UInt32 OSA_ipcQueIsFull(OSA_IpcQueHandle * handle)
{
    
    volatile UInt32 readIdx, writeIdx, numFull;
    volatile OSA_IpcQueSharedMemObj *pShm;

    if(    handle == NULL
        || handle->sharedMemBaseAddr == NULL
      )
    {
        return FALSE;
    }

    pShm = (OSA_IpcQueSharedMemObj*)handle->sharedMemBaseAddr;

    OSA_mutexLock(&(handle->mutexLock));

    readIdx = pShm->curRd;
    writeIdx = pShm->curWr;

    if(readIdx <= writeIdx)
        numFull = writeIdx - readIdx;
    else
        numFull = (handle->maxElements - readIdx) + writeIdx;

    OSA_mutexUnlock(&(handle->mutexLock));

    return (numFull == handle->maxElements) ? TRUE : FALSE;
}

