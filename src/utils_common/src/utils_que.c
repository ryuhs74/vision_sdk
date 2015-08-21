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
 * \file utils_que.c
 *
 * \brief  This file implements the UTILS_QUE_API "Software Queue" APIs
 *
 *        This software queue is used to build multiple software constructs
 *        like message box, buffer queue.
 *
 *        The software queue implmenetation is a fixed size arrary based queue
 *        for simplicity and performance. The APIs take care fo mutual
 *        exclusion protection via interuupt locks. The API optionally support
 *        blocking 'get' and 'put' APIs
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_que.h>


/**
 *******************************************************************************
 *
 * \brief Create a queue handle
 *
 *        The size of queueMem allocated by the user should be
 *        maxElements*sizeof(Ptr)
 *
 * \param handle        [OUT] Initialized queue handle
 * \param maxElements   [IN]  Maximum elements that can reside in the queue
 *                             at any given point of time
 * \param queueMem      [IN]  Address of queue element data area
 * \param flags         [IN]  #UTILS_QUE_FLAG_NO_BLOCK_QUE or
 *                            #UTILS_QUE_FLAG_BLOCK_QUE_PUT or
 *                            #UTILS_QUE_FLAG_BLOCK_QUE_GET or
 *                            #UTILS_QUE_FLAG_BLOCK_QUE
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_queCreate(Utils_QueHandle * handle,
                      UInt32 maxElements, Ptr queueMem, UInt32 flags)
{
    /*
     * init handle to 0's
     */
    memset(handle, 0, sizeof(*handle));

    /*
     * init handle with user parameters
     */
    handle->maxElements = maxElements;
    handle->flags = flags;

    /*
     * queue data element memory cannot be NULL
     */
    UTILS_assert(queueMem != NULL);

    handle->queue = queueMem;

    if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_GET)
    {
        /*
         * user requested block on que get
         */

        /*
         * create semaphore for it
         */
        handle->semRd = BspOsal_semCreate(0, TRUE);
        UTILS_assert(handle->semRd != NULL);
    }

    if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_PUT)
    {
        /*
         * user requested block on que put
         */

        /*
         * create semaphore for it
         */
        handle->semWr = BspOsal_semCreate(0, TRUE);

        UTILS_assert(handle->semWr != NULL);
    }
    handle->blockedOnGet = FALSE;
    handle->blockedOnPut = FALSE;
    handle->forceUnblockGet = FALSE;
    handle->forceUnblockPut = FALSE;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Delete queue handle
 *
 *        Releases all resources allocated during queue create
 *
 * \param handle        [IN] Queue handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
*/
Int32 Utils_queDelete(Utils_QueHandle * handle)
{
    if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_GET)
    {
        /*
         * user requested block on que get
         */

        /*
         * delete associated semaphore
         */

        BspOsal_semDelete(&handle->semRd);

    }
    if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_PUT)
    {
        /*
         * user requested block on que put
         */

        /*
         * delete associated semaphore
         */

        BspOsal_semDelete(&handle->semWr);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Add a element into the queue
 *
 * \param handle   [IN] Queue Handle
 * \param data     [IN] data element to insert
 * \param timeout  [IN] BSP_OSAL_NO_WAIT: non-blocking,
 *                         if queue is full, error is returned\n
 *                      BSP_OSAL_WAIT_FOREVER: Blocking forever,
 *                         if queue is full function blocks until
 *                         atleast one element in the queue is free \n
 *                      Amount of time in units of OS ticks that
 *                         it should block: Error is returned if time
 *                          elaspes and no space is available in the queue
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
*/
Int32 Utils_quePut(Utils_QueHandle * handle, Ptr data, Int32 timeout)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;/* init status to error */
    UInt32 cookie;

    do
    {
        /*
         * disable interrupts
         */
        cookie = Hwi_disable();

        if (handle->count < handle->maxElements)
        {
            /*
             * free space available in que
             */

            /*
             * insert element
             */
            handle->queue[handle->curWr] = data;

            /*
             * increment put pointer
             */
            handle->curWr = (handle->curWr + 1) % handle->maxElements;

            /*
             * increment count of number element in que
             */
            handle->count++;

            /*
             * restore interrupts
             */
            Hwi_restore(cookie);

            /*
             * mark status as success
             */
            status = SYSTEM_LINK_STATUS_SOK;

            if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_GET)
            {
                /*
                 * blocking on que get enabled
                 */

                /*
                 * post semaphore to unblock, blocked tasks
                 */
                BspOsal_semPost(handle->semRd);
            }

            /*
             * exit, with success
             */
            break;

        }
        else
        {
            /*
             * que is full
             */

            /*
             * restore interrupts
             */
            Hwi_restore(cookie);

            if (timeout == BSP_OSAL_NO_WAIT)
                break;                                     /* non-blocking
                                                            * function call,
                                                            * exit with error
                                                            */

            if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_PUT)
            {
                Bool semPendStatus;

                /*
                 * blocking on que put enabled
                 */

                /*
                 * take semaphore and block until timeout occurs or
                 * semaphore is posted
                 */
                handle->blockedOnPut = TRUE;
                semPendStatus = BspOsal_semWait(handle->semWr, timeout);
                handle->blockedOnPut = FALSE;
                if (!semPendStatus || handle->forceUnblockPut)
                {
                    handle->forceUnblockPut = FALSE;
                    break;                                 /* timeout
                                                            * happend, exit
                                                            * with error */
                }
                /*
                 * received semaphore, recheck for available space in the que
                 */
            }
            else
            {
                /*
                 * blocking on que put disabled
                 */

                /*
                 * exit with error
                 */
                break;
            }
        }
    }
    while (1);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Get a element from the queue
 *
 * \param handle   [IN]  Queue Handle
 * \param data     [OUT] Extracted data element from the queue
 * \param minCount [IN ] Data will be extracted only if
 *                       atleast 'minCount' elements are present in the queue
 * \param timeout  [IN] BSP_OSAL_NO_WAIT: non-blocking,
 *                         if queue is empty, error is returned\n
 *                      BSP_OSAL_WAIT_FOREVER: Blocking forever,
 *                         if queue is empty function blocks until
 *                         atleast one element in the queue is available \n
 *                      Amount of time in units of OS ticks that
 *                         it should block: Error is returned if time
 *                          elaspes and no element is available in the queue
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_queGet(Utils_QueHandle * handle, Ptr * data,
                   UInt32 minCount, Int32 timeout)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;/* init status to error */
    UInt32 cookie;

    /*
     * adjust minCount between 1 and handle->maxElements
     */
    if (minCount == 0)
        minCount = 1;
    if (minCount > handle->maxElements)
        minCount = handle->maxElements;

    do
    {
        /*
         * disable interrupts
         */
        cookie = Hwi_disable();

        if (handle->count >= minCount)
        {
            /*
             * data elements available in que is >=
             * minimum data elements requested by user
             */

            /*
             * extract the element
             */
            *data = handle->queue[handle->curRd];

            /*
             * increment get pointer
             */
            handle->curRd = (handle->curRd + 1) % handle->maxElements;

            /*
             * decrmeent number of elements in que
             */
            handle->count--;

            /*
             * restore interrupts
             */
            Hwi_restore(cookie);

            /*
             * set status as success
             */
            status = SYSTEM_LINK_STATUS_SOK;

            if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_PUT)
            {
                /*
                 * blocking on que put enabled
                 */

                /*
                 * post semaphore to unblock, blocked tasks
                 */
                BspOsal_semPost(handle->semWr);
            }

            /*
             * exit with success
             */
            break;

        }
        else
        {
            /*
             * no elements or not enough element (minCount) in que to extract
             */

            /*
             * restore interrupts
             */
            Hwi_restore(cookie);

            if (timeout == BSP_OSAL_NO_WAIT)
                break;                                     /* non-blocking
                                                            * function call,
                                                            * exit with error
                                                            */

            if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_GET)
            {
                Bool semPendStatus;

                /*
                 * blocking on que get enabled
                 */

                /*
                 * take semaphore and block until timeout occurs or
                 * semaphore is posted
                 */

                handle->blockedOnGet = TRUE;
                semPendStatus = BspOsal_semWait(handle->semRd, timeout);
                handle->blockedOnGet = FALSE;
                if (!semPendStatus || (handle->forceUnblockGet == TRUE))
                {
                    handle->forceUnblockGet = FALSE;
                    break;                                 /* timeout
                                                            * happened, exit
                                                            * with error */
                }

                /*
                 * received semaphore, check que again
                 */
            }
            else
            {
                /*
                 * blocking on que get disabled
                 */

                /*
                 * exit with error
                 */
                break;
            }
        }
    }
    while (1);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Returns TRUE if queue is empty else retunrs false
 *
 * \param handle   [IN] Queue Handle
 *
 * \return Returns TRUE is queue is empty else retunrs FALSE
 *
 *******************************************************************************
 */
UInt32 Utils_queIsEmpty(Utils_QueHandle * handle)
{
    UInt32 isEmpty;
    UInt32 cookie;

    /*
     * disable interrupts
     */
    cookie = Hwi_disable();

    /*
     * check if que is empty
     */
    if (handle->count)
        isEmpty = FALSE;                                   /* not empty */
    else
        isEmpty = TRUE;                                    /* empty */

    /*
     * restore interrupts
     */
    Hwi_restore(cookie);

    return isEmpty;
}

/**
 *******************************************************************************
 *
 * \brief Peek at the first element from the queue, but do not extract it
 *
 * \param handle   [IN]  Queue Handle
 * \param data     [OUT] "peeked" data element from the queue
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_quePeek(Utils_QueHandle * handle, Ptr * data)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL; /* init status as error */
    UInt32 cookie;

    *data = NULL;

    /*
     * disable interrupts
     */
    cookie = Hwi_disable();

    if (handle->count)
    {
        /*
         * que is not empty
         */

        /*
         * get value of top element, but do not extract it from que
         */
        *data = handle->queue[handle->curRd];

        /*
         * set status as success
         */
        status = SYSTEM_LINK_STATUS_SOK;
    }

    /*
     * restore interrupts
     */
    Hwi_restore(cookie);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Returns number of elements queued
 *
 * \param handle   [IN] Queue Handle
 *
 * \return Returns number of elements queued
 *
 *******************************************************************************
 */
UInt32 Utils_queGetQueuedCount(Utils_QueHandle * handle)
{
    UInt32 count;
    UInt32 cookie;

    /*
     * disable interrupts
     */
    cookie = Hwi_disable();

    count = handle->count;

    /*
     * restore interrupts
     */
    Hwi_restore(cookie);

    return count;
}

/**
 *******************************************************************************
 * \brief Force unblock of queue.
 *
 *        If any entity was waiting on queue get/queue put
 *        it will get unblocked with error code indicating queue was unblocked
 *
 * \param handle   [IN] Queue Handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_queUnBlock(Utils_QueHandle * handle)
{
    UInt tskKey ;

    /* lock task scheduler */
    tskKey = Task_disable();

    if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_GET)
    {
        if (handle->semRd)
        {
            handle->forceUnblockGet = TRUE;
            BspOsal_semPost(handle->semRd);
        }
    }

    if (handle->flags & UTILS_QUE_FLAG_BLOCK_QUE_PUT)
    {
        if (handle->semWr)
        {
            handle->forceUnblockPut = TRUE;
            BspOsal_semPost(handle->semWr);
        }
    }

    Task_restore(tskKey);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Returns TRUE if queue is full else returns FALSE
 *
 * \param handle   [IN] Queue Handle
 *
 * \return Returns TRUE if queue is full else returns FALSE
 *
 *******************************************************************************
 */
UInt32 Utils_queIsFull(Utils_QueHandle * handle)
{
    UInt32 isFull;
    UInt32 cookie;

    /*
     * disable interrupts
     */
    cookie = Hwi_disable();

    /*
     * check if que is empty
     */
    if (handle->count < handle->maxElements)
        isFull = FALSE;                                    /* not full */
    else
        isFull = TRUE;                                     /* full */

    /*
     * restore interrupts
     */
    Hwi_restore(cookie);

    return isFull;
}


/**
 *******************************************************************************
 *
 * \brief Reset the Queue
 *
 * \param handle   [IN] Queue Handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_queReset(Utils_QueHandle * handle)
{
    UInt32 cookie;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    /*
     * disable interrupts
     */
    cookie = Hwi_disable();

    /*
     * Reset the queue
     */
    handle->count = 0;
    handle->curRd = 0;
    handle->curWr = 0;
    handle->blockedOnGet = FALSE;
    handle->blockedOnPut = FALSE;
    handle->forceUnblockGet = FALSE;
    handle->forceUnblockPut = FALSE;

    /*
     * restore interrupts
     */
    Hwi_restore(cookie);

    return status;
}

