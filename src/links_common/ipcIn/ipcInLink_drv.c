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
 * \file ipcInLink.c
 *
 * \brief  This file has the implementation of IPC IN Link API
 *
 *         This file implements the software logic needed to exchange frames
 *         between processors
 *
 * \version 0.0 (Aug 2013) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "ipcInLink_priv.h"

/**
 *******************************************************************************
 *
 * \brief This function is called when previous link send a notify or
 *        periodic timer expires
 *
 *        A command is sent to the IPC thread to read buffer's from IPC IN ->
 *        IPC OUT queue and release the buffers back to the previous link
 *
 * \param  pTsk     [IN]  Task Handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void IpcInLink_drvNotifyCb(Utils_TskHndl * pTsk)
{
    IpcInLink_obj * pObj = (IpcInLink_obj * )pTsk->appData;

    pObj->linkStatsInfo->linkStats.notifyEventCount++;

    if(Utils_ipcQueIsEmpty(&pObj->ipcOut2InQue)==FALSE)
    {
        /*
         * send command to process frames only if there are elements in the
         * que
         */
        System_sendLinkCmd(pObj->linkId,
                            SYSTEM_CMD_NEW_DATA,
                            NULL);
    }
}

/**
 *******************************************************************************
 *
 * \brief Create IPC In link
 *
 *        Following happens during create phase,
 *        - Call 'get link info” on previous link.
 *          When link from other processors asks link info it gives back
 *          this link info to the next link (IPC IN)
 *        - IPC shared memory buffer information structures are allocated
 *          for non uni-cache IPC OUT/IN pair
 *        - IPC shared memory buffer information structure pointers
 *          are placed in internal local queue for non uni-cache
 *          IPC OUT/IN pair
 *        - Both IPC shared memory queue's
 *          (IPC OUT -> IPC IN, IPC IN -> IPC OUT) is reset to empty state.
 *        - Clock object is created for periodic polling if requested
 *          during create params
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcInLink_drvCreate(IpcInLink_obj *pObj, IpcLink_CreateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_Buffer *pSysBuf;
    UInt32 elemId;
    char                 tskName[32];

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_IN_%d   : Create in progress !!!\n",
               pObj->linkInstId
               );
#endif

    UTILS_MEMLOG_USED_START();

    /* keep a copy of create args */
    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    /* get previous link info */
    status = System_linkGetInfo(
                    pObj->createArgs.inQueParams.prevLinkId,
                    &pObj->prevLinkInfo);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* validate previous link que ID */
    UTILS_assert(pObj->createArgs.inQueParams.prevLinkQueId <
                  pObj->prevLinkInfo.numQue);

    /*
     * Setup current link que information
     * Current queue is considered to have one output queue
     * with que information same as selected previous link queue
     */
    pObj->linkInfo.numQue = 1;
    memcpy(&pObj->linkInfo.queInfo[0],
           &pObj->prevLinkInfo.queInfo
                [pObj->createArgs.inQueParams.prevLinkQueId],
           sizeof(pObj->linkInfo.queInfo[0]));

    pObj->lock = BspOsal_semCreate(1u, TRUE);
    UTILS_assert(pObj->lock != NULL);

    status = Utils_ipcQueReset(&pObj->ipcOut2InQue,
                        (void *)System_ipcGetIpcOut2InQue(
                                    pObj->createArgs.inQueParams.prevLinkId),
                        TRUE,
                        FALSE);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_ipcQueReset(&pObj->ipcIn2OutQue,
                        (void *)System_ipcGetIpcIn2OutQue(
                                    pObj->createArgs.inQueParams.prevLinkId),
                        FALSE,
                        TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_bufCreate(&pObj->outBufQue, FALSE, FALSE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* allocate memory for IPC data structure's in shared memory */
    for(elemId=0; elemId <IPC_IN_LINK_IPC_QUE_MAX_ELEMENTS; elemId++)
    {
        pObj->buffers[elemId].payload = pObj->payload[elemId];
        /* queue to free Frame queue */
        pSysBuf = &pObj->buffers[elemId];

        status = Utils_bufPutEmptyBuffer(&pObj->outBufQue, pSysBuf);

        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
    }

    pObj->isFirstFrameRecv = FALSE;

    sprintf(tskName, "IPC_IN_%u", (unsigned int)pObj->linkInstId);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->linkId, tskName);
    UTILS_assert(NULL != pObj->linkStatsInfo);

    IpcInLink_latencyStatsReset(pObj);

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("IPC_IN:",
                   pObj->memUsed,
                   UTILS_ARRAYSIZE(pObj->memUsed));

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_IN_%d   : Create Done !!!\n",
           pObj->linkInstId
          );
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete IPC In link
 *
 *        This function free's resources allocated during create
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcInLink_drvDelete(IpcInLink_obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_IN_%d   : Delete in progress !!!\n",
           pObj->linkInstId
          );
#endif

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* delete local queue */
    status = Utils_bufDelete(&pObj->outBufQue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    BspOsal_semDelete(&pObj->lock);

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_IN_%d   : Delete Done !!!\n",
           pObj->linkInstId
          );
#endif
    return status;
}


/**
 *******************************************************************************
 *
 * \brief Copy information from IPC buffer to system buffer
 *
 * \param  pObj        [IN]  Link object
 * \param  pBuffer     [IN]  Pointer to system buffer information
 * \param  pIpcBuffer  [IN]  Pointer to IPC buffer information
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void  IpcInLink_drvCopyIpcBufferToSystemBuffer(
                    IpcInLink_obj *pObj,
                    System_Buffer *pBuffer,
                    System_IpcBuffer *pIpcBuffer,
                    UInt32 index)
{
    UInt32 flags = 0;

    flags = pIpcBuffer->flags;

    pBuffer->bufType            = (System_BufferType)SYSTEM_BUFFER_FLAG_GET_BUF_TYPE(flags);
    pBuffer->chNum              = SYSTEM_BUFFER_FLAG_GET_CH_NUM(flags);
    pBuffer->payloadSize        = SYSTEM_BUFFER_FLAG_GET_PAYLOAD_SIZE(flags);
    pBuffer->srcTimestamp       = pIpcBuffer->srcTimestamp;
    pBuffer->ipcInOrgQueElem    = (UInt32)index;

    pBuffer->ipcPrfTimestamp64[0] = pIpcBuffer->ipcPrfTimestamp64[0];
    pBuffer->ipcPrfTimestamp64[1] = pIpcBuffer->ipcPrfTimestamp64[1];

    UTILS_assert(pBuffer->payloadSize <= SYSTEM_MAX_PAYLOAD_SIZE );
    UTILS_assert(pBuffer->payload != NULL );
    UTILS_assert(pIpcBuffer->payload != NULL );

    memcpy(pBuffer->payload, pIpcBuffer->payload, pBuffer->payloadSize);
}

/**
 *******************************************************************************
 *
 * \brief Process buffer's
 *
 *        - Previous link will notify when buffers are available
 *            to be sent across processors
 *
 *        - IPC In link picks the buffers from the ipcOut2InQue
 *
 *        - For each buffer information pointer it will
 *          -  If uni-cache operation
 *             - Pick a Queue Element from free queue, copy the data and put the
 *              Frame back to out queue
 *             - Translate the information from IPC shared memory buffer
 *               to system buffer information
 *             - The original IPC shared memory buffer pointer is also set in
 *                 system buffer information structure
 *          -  If non-uni-cache operation
 *             - Place the same buffer into out queue
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcInLink_drvProcessBuffers(IpcInLink_obj *pObj)
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    Int32 queStatus = SYSTEM_LINK_STATUS_SOK;
    UInt32            index;
    UInt32 numBufs = 0;
    System_Buffer     *pSysBuffer;
    Bool sendNotifyToPrevLink = FALSE;
    UInt64 tmpTimestamp64;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    if(pObj->isFirstFrameRecv == FALSE)
    {
        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);

        Utils_resetLinkStatistics(&linkStatsInfo->linkStats,
                          pObj->linkInfo.queInfo[0].numCh,
                          1);

        pObj->isFirstFrameRecv = TRUE;
    }

    while(1)
    {
        queStatus = Utils_ipcQueRead( &pObj->ipcOut2InQue,
                                     (UInt8*)&index,
                                      sizeof(UInt32));

        tmpTimestamp64 = Utils_getCurGlobalTimeInUsec();

        if(queStatus!=SYSTEM_LINK_STATUS_SOK)
            break; /* no more data to read from IPC queue */

        {
            System_IpcBuffer *pIpcBuffer;

            status = Utils_bufGetEmptyBuffer(&pObj->outBufQue,&pSysBuffer,
                                            BSP_OSAL_NO_WAIT);
            if(status != SYSTEM_LINK_STATUS_SOK)
            {
                /* failed to get the buffer returning ipcbuffer to free */
                status = Utils_ipcQueWrite( &pObj->ipcIn2OutQue,
                                            (UInt8*)&index,
                                            sizeof(UInt32));
                sendNotifyToPrevLink = TRUE;
                continue;
            }

            pIpcBuffer = System_ipcGetIpcBuffer(
                            pObj->createArgs.inQueParams.prevLinkId,
                            index
                         );

            if(pIpcBuffer == NULL)
            {
                pObj->linkStatsInfo->linkStats.inBufErrorCount++;
            }

            UTILS_assert(pIpcBuffer != NULL);
            if(pSysBuffer == NULL)
            {
                linkStatsInfo->linkStats.inBufErrorCount++;
            }
            UTILS_assert(pSysBuffer != NULL);

            pSysBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

            IpcInLink_drvCopyIpcBufferToSystemBuffer(
                pObj,
                pSysBuffer,
                pIpcBuffer,
                index);
        }

        linkStatsInfo->linkStats.chStats[pSysBuffer->chNum].inBufRecvCount++;

        Utils_updateLatency(&linkStatsInfo->linkLatency,
                            pSysBuffer->linkLocalTimestamp);
        Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                            pSysBuffer->srcTimestamp);

        pSysBuffer->ipcPrfTimestamp64[0]
            = Utils_getCurGlobalTimeInUsec() - pSysBuffer->ipcPrfTimestamp64[0];
        pSysBuffer->ipcPrfTimestamp64[1]
            = tmpTimestamp64 - pSysBuffer->ipcPrfTimestamp64[1];

        IpcInLink_latencyStatsUpdate(pObj,
                pSysBuffer->ipcPrfTimestamp64[0],
                pSysBuffer->ipcPrfTimestamp64[1]
                        );

        status = Utils_bufPutFullBuffer(&pObj->outBufQue, pSysBuffer);
        if(status != SYSTEM_LINK_STATUS_SOK)
        {
            linkStatsInfo->linkStats.chStats[pSysBuffer->chNum].inBufDropCount++;
            linkStatsInfo->linkStats.chStats[pSysBuffer->chNum].outBufDropCount[0]++;
        }
        else
        {
            linkStatsInfo->linkStats.chStats[pSysBuffer->chNum].inBufProcessCount++;
            linkStatsInfo->linkStats.chStats[pSysBuffer->chNum].outBufCount[0]++;
        }
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        numBufs++;
    }
    if(sendNotifyToPrevLink)
    {
        System_ipcSendNotify(pObj->createArgs.inQueParams.prevLinkId);
        sendNotifyToPrevLink = FALSE;
    }
    if(numBufs)
    {
        System_sendLinkCmd(pObj->createArgs.outQueParams.nextLink,
                            SYSTEM_CMD_NEW_DATA, NULL);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to get empty buffers from next
 *        link.
 *
 * \param  pObj     [IN]  Link object
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IpcInLink_drvPutEmptyBuffers(IpcInLink_obj *pObj,
                                    System_BufferList *pBufList)
{

    Int32 status = SYSTEM_LINK_STATUS_EFAIL;
    UInt32 bufId;
    System_Buffer *pBuf;
    UInt32 index;

    BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);

    for (bufId = 0; bufId < pBufList->numBuf; bufId++)
    {
        pBuf = pBufList->buffers[bufId];
        if(pBuf==NULL)
            continue;

        index = pBuf->ipcInOrgQueElem;

        pObj->linkStatsInfo->linkStats.putEmptyBufCount++;
        status = Utils_ipcQueWrite( &pObj->ipcIn2OutQue,
                                    (UInt8*)&index,
                                    sizeof(UInt32));
    }

    if(pBufList->numBuf)
    {
        System_ipcSendNotify(pObj->createArgs.inQueParams.prevLinkId);
    }


    status = Utils_bufPutEmpty(&pObj->outBufQue, pBufList);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    BspOsal_semPost(pObj->lock);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to give full buffers to next
 *        link.
 *
 * ipcIn link sends message to next link about availability of buffers.
 * Next link calls this callback function to get full buffers from ipcIn
 * output queue.
 *
 * \param  pObj     [IN]  Link object
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IpcInLink_drvGetFullBuffers(IpcInLink_obj *pObj,
                                    System_BufferList *pBufList)
{
    Int32 status;

    status =  Utils_bufGetFull(&pObj->outBufQue, pBufList, BSP_OSAL_NO_WAIT);
    if(status == SYSTEM_LINK_STATUS_SOK)
    {
        pObj->linkStatsInfo->linkStats.getFullBufCount++;
    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Stop buffer processing
 *
 *        This function must be called before calling delete
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcInLink_drvStop(IpcInLink_obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_IN_%d   : Stop Done !!!\n",
           pObj->linkInstId
          );
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Print IPC link related statistics
 *
 * \param  pObj     [IN] Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcInLink_drvPrintStatistics(IpcInLink_obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    char  tskName[32];

    sprintf(tskName, "IPC_IN_%u", (unsigned int)pObj->linkInstId);

    UTILS_assert(NULL != pObj->linkStatsInfo);

    Utils_printLinkStatistics(&pObj->linkStatsInfo->linkStats, tskName, TRUE);

    Utils_printLatency(tskName,
                       &pObj->linkStatsInfo->linkLatency,
                       &pObj->linkStatsInfo->srcToLinkLatency,
                       TRUE);

    IpcInLink_latencyStatsPrint(pObj, TRUE);

    return status;
}


/**
 *******************************************************************************
 *
 * \brief Reset IPC latency stats
 *
 * \param   pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Void IpcInLink_latencyStatsReset(IpcInLink_obj *pObj)
{
    UInt32 oldIntState;

    oldIntState = Hwi_disable();

    memset(&pObj->ipcLatencyStats, 0, sizeof(pObj->ipcLatencyStats));

    pObj->ipcLatencyStats.minIpcLatency = 0xFFFFFFFF;
    pObj->ipcLatencyStats.minNotifyLatency = 0xFFFFFFFF;

    Hwi_restore(oldIntState);
}

/**
 *******************************************************************************
 *
 * \brief Update IPC latency stats
 *
 * \param   pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Void IpcInLink_latencyStatsUpdate(IpcInLink_obj *pObj,
                                UInt64 ipcLatency,
                                UInt64 notifyLatency )
{
    UInt32 oldIntState;

    oldIntState = Hwi_disable();

    if(notifyLatency < 1000)
    {
        /* threshold below which measurement is done
         * Anything higher is considered as a anomaly
         */
        pObj->ipcLatencyStats.count++;
        pObj->ipcLatencyStats.totalIpcLatency += ipcLatency;
        pObj->ipcLatencyStats.totalNotifyLatency
                            += notifyLatency;

        if(ipcLatency < pObj->ipcLatencyStats.minIpcLatency)
            pObj->ipcLatencyStats.minIpcLatency = ipcLatency;

        if(ipcLatency > pObj->ipcLatencyStats.maxIpcLatency)
            pObj->ipcLatencyStats.maxIpcLatency = ipcLatency;


        if(notifyLatency
                <
            pObj->ipcLatencyStats.minNotifyLatency)
        {
            pObj->ipcLatencyStats.minNotifyLatency
                = notifyLatency;
        }

        if(notifyLatency
                >
            pObj->ipcLatencyStats.maxNotifyLatency)
        {
            pObj->ipcLatencyStats.maxNotifyLatency = notifyLatency;
        }
    }

    Hwi_restore(oldIntState);
}


/**
 *******************************************************************************
 *
 * \brief Print IPC latency stats
 *
 * \param   pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Void IpcInLink_latencyStatsPrint(IpcInLink_obj *pObj, Bool resetStats)
{
    char   tskName[32];
    UInt64 avgTime64, minTime64, maxTime64;

    sprintf(tskName, "IPC_IN_%u", (unsigned int)pObj->linkInstId);

    Vps_printf(" [ %s ] Detailed IPC Latency Statistics [ %s -> %s ] ,\n",
                tskName,
                System_getProcName(
                            SYSTEM_GET_PROC_ID(
                                pObj->createArgs.inQueParams.prevLinkId
                                        )
                            ),
                System_getProcName(System_getSelfProcId())
                );
    Vps_printf(
            " ***************************************************************\n");

    if (pObj->ipcLatencyStats.count)
    {

        /* One way buffer passing time is
            round trip time - one-way interrupt handling time
         */
        avgTime64 = (pObj->ipcLatencyStats.totalIpcLatency
                        /
                        pObj->ipcLatencyStats.count);
                    ;

        minTime64 = pObj->ipcLatencyStats.minIpcLatency;
                        ;

        maxTime64 = pObj->ipcLatencyStats.maxIpcLatency;
                        ;

        Vps_printf(" IPC One-way Buffer Passing Latency   (usecs) :"
                   " Avg = %6d, Min = %6d, Max = %6d\n",
                    (UInt32)avgTime64,
                    (UInt32)minTime64,
                    (UInt32)maxTime64
                   );

        avgTime64 = (pObj->ipcLatencyStats.totalNotifyLatency
                        /
                        pObj->ipcLatencyStats.count);
                    ;

        minTime64 = pObj->ipcLatencyStats.minNotifyLatency;
                        ;

        maxTime64 = pObj->ipcLatencyStats.maxNotifyLatency;

        /* since two interrupts are exchanged per IPC, we divide the value by 2 */
        Vps_printf(" IPC One-way Notify Interrupt Latency (usecs) :"
                   " Avg = %6d, Min = %6d, Max = %6d\n",
                    (UInt32)avgTime64,
                    (UInt32)minTime64,
                    (UInt32)maxTime64
                   );
    }

    if(resetStats)
    {
        IpcInLink_latencyStatsReset(pObj);
    }
}
