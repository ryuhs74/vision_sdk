/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file syncLink_tsk.c
 *
 * \brief  This file has the implementation of Sync Link API
 **
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 * \version 0.0 (Jul 2013) : [NN] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "syncLink_priv.h"



/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Utility macro that returns
 *  0 if x is in between a and b
 *  1 if x is greater than b
 * -1 if x is lesser than a
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define INRANGE(a,b,x)    (x >= a && x <= b ? 0 : x > b ? 1 : -1)



/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
SyncLink_Obj gSyncLink_obj[SYNC_LINK_OBJ_MAX];

Void SyncLink_timerCallback(UArg arg)
{
    SyncLink_Obj *pObj = (SyncLink_Obj *) arg;

    Utils_tskSendCmd(&pObj->tsk, SYSTEM_CMD_NEW_DATA, NULL);

    pObj->linkStatsInfo->linkStats.notifyEventCount++;
}

/**
 *******************************************************************************
 * \brief Sync link is a connector link. Sync Link is particularly targeted to
 *   a set of specific use cases where there is a need for a set of video
 *   frames from multiple channels to be in sync (Frames captured at
 *   approximately at the same time). This function does the following,
 *
 *    - Copies the user passed create params into the link object create params
 *    - Each composite buffer is tied up with a system buffer
 *    - Prepares output queue
 *
 * \param  pObj     [IN]  Sync link instance handle
 * \param  pPrm     [IN]  Create params for Sync link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_drvCreate(SyncLink_Obj * pObj, SyncLink_CreateParams *pPrm)
{
    UInt32 outId, chId, bufId;
    Int32 status;
    SyncLink_ChObj *chObj;
    System_Buffer *pSysBuf;
    System_VideoFrameCompositeBuffer *sysCompBuf;
    SyncLink_OrigBufferPtr *origBufPtr;
    char                 tskName[32];

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    status = System_linkGetInfo(
                   pObj->createArgs.inQueParams.prevLinkId, &pObj->prevLinkInfo);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->prevLinkInfo.numQue);

    pObj->linkInfo.numQue = 1;
    memcpy(&pObj->linkInfo.queInfo[0],
           &pObj->prevLinkInfo.queInfo
                [pObj->createArgs.inQueParams.prevLinkQueId],
           sizeof(pObj->linkInfo.queInfo[0])
          );

    pObj->createArgs.chParams.numCh =
           pObj->prevLinkInfo.queInfo
                  [pObj->createArgs.inQueParams.prevLinkQueId].numCh;
    UTILS_assert(pObj->createArgs.chParams.numCh <= SYNC_LINK_MAX_CHANNELS);

    /*
     *  Initialize each channel
    */

    for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
    {
        chObj = &pObj->chObj[chId];
        chObj->dropCountNoBuffers = 0;
        chObj->dropCountNoSync = 0;
        chObj->forwardCount = 0;

        status = Utils_queCreate(&chObj->localQueHandle,
                                  UTILS_ARRAYSIZE(chObj->queMem),
                                  chObj->queMem,
                                  UTILS_QUE_FLAG_NO_BLOCK_QUE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    /*
     * Associate each composite buffer with a system buffer
    */

    for (outId = 0; outId < SYNC_LINK_MAX_FRAMES_PER_OUT_QUE; outId++)
    {
       pSysBuf = &pObj->outBuf[outId];
       sysCompBuf = &pObj->compBuf[outId];
       origBufPtr = &pObj->origBufPtr[outId];

       pSysBuf->bufType = SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER;
       pSysBuf->chNum = 0;
       pSysBuf->payload = sysCompBuf;
       pSysBuf->payloadSize = sizeof(*sysCompBuf);
       pSysBuf->pSyncLinkOrgBufferPtr = origBufPtr;
    }

    status = Utils_bufCreate(&pObj->outFrameQue, FALSE, FALSE);

    for (bufId = 0; bufId < SYNC_LINK_MAX_FRAMES_PER_OUT_QUE; bufId++)
    {
        pSysBuf = &pObj->outBuf[bufId];
        status = Utils_bufPutEmptyBuffer(&pObj->outFrameQue, pSysBuf);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    sprintf(tskName, "SYNC_%u", (unsigned int)pObj->linkInstId);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->tskId, tskName);
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->isFirstFrame = FALSE;

    memset(&pObj->stats, 0, sizeof(pObj->stats));
    memset(&pObj->dropBufList, 0, sizeof(pObj->dropBufList));

    pObj->timer = BspOsal_clockCreate(
                            (BspOsal_ClockFuncPtr)SyncLink_timerCallback,
                            33, FALSE, pObj);
    UTILS_assert(pObj->timer != NULL);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief This function updates previous timestamp for each active channel and
 *        also handles wrap around of timestamps
 *
 * \param  pObj     [IN]  Sync link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_handleTimeStampWrapAround(UInt32 *chTimeStamps, UInt32 count)
{
    UInt32 chId;
    UInt32 lastQuadrantFlag  = 0;
    UInt32 firstQuadrantFlag = 0;

    UInt32 deltaToWrapAround   =    0x40000000;

    for (chId = 0; chId < count; chId++)
    {
        if (chTimeStamps[chId] & 0xC0000000 == 0xC0000000)
        {
            lastQuadrantFlag = 1;
        }
        if (chTimeStamps[chId] & 0xC0000000 == 0)
        {
            firstQuadrantFlag = 1;
        }
    }

    if (firstQuadrantFlag && lastQuadrantFlag)
    {
        #if SYNC_DEBUG
            Vps_printf("\nWrap Around Happened!!!");
            for (chId = 0; chId < count; chId++)
            {
                Vps_printf("\n Ch%d timestamp = %x \n", count, chTimeStamps[chId]);
            }
        #endif

        /*  Wrap Around Happened
         *  Add a constant value to all the timestamps so that all
         *  channels will wrap
         */
        for (chId = 0; chId < count; chId++)
        {
            chTimeStamps[chId] += deltaToWrapAround;
        }
        /* No Wrap Around Happened */
        return SYSTEM_LINK_STATUS_SOK;
    }
    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 * \brief This function drops buffers in the local queues which are older than
 *        the user specified threshold
 *
 * \param  pObj     [IN]  Sync link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_addBuffersToDropListLesserThanThreshold(SyncLink_Obj * pObj)
{
    UInt32 chId, index, dropedBuffer;
    Int64 diffTime;

    SyncLink_ChObj *chObj;
    System_Buffer *pBuffer;
    SyncLink_ChannelParams *chPrms = &pObj->createArgs.chParams;

    do {
        dropedBuffer= FALSE;
        for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
        {
            chObj = &pObj->chObj[chId];
            Utils_quePeek(&chObj->localQueHandle, (Ptr *) &pBuffer);
            if (chPrms->channelSyncList[chId])
            {
                 if ( pBuffer != NULL )
                 {
                    if( chPrms->syncThreshold >= SYNC_DROP_THRESHOLD_MAX)
                    {
                        /* Dont drop frames if threshold is more than
                            SYNC_DROP_THRESHOLD_MAX
                        */
                    }
                    else
                    {
                        /* diffTime is in msec's */
                        diffTime =
                              (Utils_getCurGlobalTimeInUsec() - pBuffer->srcTimestamp)/1000;
                        UTILS_assert(diffTime >= 0);

                        if (diffTime > chPrms->syncThreshold)
                        {
                            Utils_queGet(&chObj->localQueHandle,
                                         (Ptr *) &pBuffer,
                                         1,
                                         BSP_OSAL_NO_WAIT);
                            pObj->dropBufList.buffers[pObj->dropBufList.numBuf] =
                                                                           pBuffer;
                            pObj->dropBufList.numBuf++;
                            index = pObj->stats.totalDropCount %
                                          SYNC_LINK_MAX_DROP_BUFFER_STATS;

                            pObj->stats.dropStats[index].masterTimestamp =
                                                       SYNC_LINK_INVALID_TIMESTAMP;
                            pObj->stats.dropStats[index].bufferTimestamp =
                                                         pBuffer->srcTimestamp/1000;
                            pObj->stats.dropStats[index].chNum = chId;
                            pObj->stats.totalDropCount++;

                            pObj->linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
                            dropedBuffer = TRUE;
                        }
                    }
                 }
            }
            else
            {
               /*
                * Drop Buffers unconditionally since these channels are
                * no longer active
               */
               if ( pBuffer != NULL)
               {
                   pObj->dropBufList.buffers[pObj->dropBufList.numBuf] = pBuffer;
                   pObj->dropBufList.numBuf++;
                   pObj->linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
               }
            }
        }
    }while(dropedBuffer == TRUE);
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief For each top most buffer in the queue this function computes whether
 *        the timestamp of buffer is in the range specified by user.
 *
 * \param  pObj     [IN]  Sync link instance handle
 * \param  synDone  [OUT] Indicates whether sync is true or not
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_computeDecisionParams(SyncLink_Obj * pObj,
                                                   UInt32 *syncDone)
{
    UInt32 higherTimeStamp, lowerTimeStamp, chId;
    Int32 higherFlag, status;
    SyncLink_ChannelParams *chPrms = &pObj->createArgs.chParams;
    SyncLink_ChDecisionParams *chDecParams = pObj->chDecParams;
    System_Buffer *pBuffer;
    UInt64 temp;
    UInt32 maxUInt32Value = 0xFFFFFFFF - 1;

    temp = pObj->masterTimeStamp + chPrms->syncDelta;

    if (temp > maxUInt32Value)
    {
        temp = maxUInt32Value;
    }

    higherTimeStamp = (UInt32) temp;

    if (pObj->masterTimeStamp < chPrms->syncDelta)
    {
        lowerTimeStamp = 0;
    }
    else
    {
        lowerTimeStamp  = pObj->masterTimeStamp - chPrms->syncDelta;
    }

    for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
    {
        chDecParams[chId].flag = 0;
        chDecParams[chId].drop = FALSE;
    }

    higherFlag = FALSE;
    *syncDone = TRUE;
    if(chPrms->syncDelta >= SYNC_DROP_THRESHOLD_MAX)
    {
        /* Dont drop frames if threshold is more than
            SYNC_DROP_THRESHOLD_MAX
        */
    }
    else
    {
        for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
        {
            if (chPrms->channelSyncList[chId])
            {
                status = Utils_quePeek(
                                &pObj->chObj[chId].localQueHandle,
                                (Ptr *) &pBuffer
                                );
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                chDecParams[chId].flag = INRANGE(
                                               lowerTimeStamp,
                                               higherTimeStamp,
                                               pBuffer->srcTimestamp/1000
                                              );
                if (chDecParams[chId].flag == -1)
                {
                   chDecParams[chId].drop = TRUE;
                   *syncDone = FALSE;
                }
                else if (chDecParams[chId].flag == 1)
                {
                   higherFlag = TRUE;
                   *syncDone = FALSE;
                }
            }
        }
        if (higherFlag)
        {
            /*
             * Drop all channels who are in range also
             */
            for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
            {
                if (chPrms->channelSyncList[chId])
                {
                    if (chDecParams[chId].flag == 0)
                    {
                        chDecParams[chId].drop = TRUE;
                    }
                }
            }
        }
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief This function drops buffers in the local queues which are not in sync
 *
 * \param  pObj     [IN]  Sync link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_addBuffersToDropListNotInSync(SyncLink_Obj * pObj)
{
    UInt32 chId, index;
    Int32 status;
    SyncLink_ChDecisionParams *chDecParams = pObj->chDecParams;
    System_Buffer *pBuffer;

    for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
    {
        if (chDecParams[chId].drop == TRUE)
        {
            status = Utils_queGet(&pObj->chObj[chId].localQueHandle,
                                   (Ptr *) &pBuffer,
                                   1,
                                   BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            pObj->dropBufList.buffers[pObj->dropBufList.numBuf] = pBuffer;
            pObj->dropBufList.numBuf++;

            pObj->linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;

            index = pObj->stats.totalDropCount %
                                  SYNC_LINK_MAX_DROP_BUFFER_STATS;

            pObj->stats.dropStats[index].curTimestamp    =
                Utils_getCurGlobalTimeInUsec()/1000;

            pObj->stats.dropStats[index].masterTimestamp =
                                                 pObj->masterTimeStamp;
            pObj->stats.dropStats[index].bufferTimestamp =
                                                 pBuffer->srcTimestamp/1000;
            pObj->stats.totalDropCount++;
        }
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief This function dequeus buffers from previous link and puts in local
 *        queues based on the channel number.
 *
 * \param  pObj     [IN]  Sync link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_fillLocalQueues(SyncLink_Obj * pObj)
{
    Int32 bufId;
    System_BufferList inputBufList;
    System_Buffer *pBuffer;
    System_LinkInQueParams *pInQueParams = &pObj->createArgs.inQueParams;
    SyncLink_ChannelParams *chPrms = &pObj->createArgs.chParams;
    Int32 status;

    System_getLinksFullBuffers(pInQueParams->prevLinkId,
                               pInQueParams->prevLinkQueId,
                               &inputBufList);

    for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
    {
        pBuffer = inputBufList.buffers[bufId];

        if (pBuffer == NULL
             || pBuffer->chNum >= pObj->linkInfo.queInfo[0].numCh)
        {
            pObj->linkStatsInfo->linkStats.inBufErrorCount++;
            continue;
        }

        pBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

        pObj->linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufRecvCount++;

        if (chPrms->channelSyncList[pBuffer->chNum])
        {
            status = Utils_quePut(&pObj->chObj[pBuffer->chNum].localQueHandle,
                         pBuffer, BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
        else
        {
            pObj->dropBufList.buffers[pObj->dropBufList.numBuf] = pBuffer;
            pObj->dropBufList.numBuf++;
            pObj->linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
            pObj->stats.totalDropCount++;
        }
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief This function creates composite buffer when buffers from all local
 *        queues are available and are in sync
 *
 * \param  pObj     [IN]  Sync link instance handle
 * \param  pSysBuf  [OUT] System Buffer which hold pointer to composite buffer
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_makeCompositeBuffer(SyncLink_Obj * pObj, System_Buffer *pSysBuf)
{
    Int32 status, i, syncDelta;
    UInt32 chId, firstSyncCh;
    System_Buffer *chBuf;
    System_VideoFrameCompositeBuffer *pSysCompBuf;
    SyncLink_ChannelParams *chPrms = &pObj->createArgs.chParams;

    UTILS_assert(pSysBuf != NULL);

    SyncLink_OrigBufferPtr *origBufPtr =
                      (SyncLink_OrigBufferPtr *)pSysBuf->pSyncLinkOrgBufferPtr;

    pSysBuf->srcTimestamp = pObj->masterTimeStamp*1000;

    pSysCompBuf = (System_VideoFrameCompositeBuffer *)pSysBuf->payload;

    pSysCompBuf->numFrames = 0;

    memset(origBufPtr, 0, sizeof(*origBufPtr));

    firstSyncCh = TRUE;

    for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
    {
        if (chPrms->channelSyncList[chId])
        {
            status = Utils_queGet(&pObj->chObj[chId].localQueHandle,
                                   (Ptr *) &chBuf,
                                   1,
                                   BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            UTILS_assert(chBuf != NULL);

            pObj->linkStatsInfo->linkStats.chStats[chBuf->chNum].inBufProcessCount++;

            if(chBuf->bufType==SYSTEM_BUFFER_TYPE_VIDEO_FRAME)
            {
                System_VideoFrameBuffer *pVidBuf;

                pVidBuf = (System_VideoFrameBuffer*) chBuf->payload;

                for (i=0; i < SYSTEM_MAX_PLANES; i++)
                {
                    pSysCompBuf->bufAddr[i][pSysCompBuf->numFrames] =
                                                            pVidBuf->bufAddr[i];
                }
                pSysCompBuf->metaBufAddr[pSysCompBuf->numFrames] =
                                                           pVidBuf->metaBufAddr;

                if(firstSyncCh)
                {
                    firstSyncCh = FALSE;
                    /* Other fields of Composite buffer can be anything,
                     * so we arbitarily
                     * choose the first synced channel
                     */
                    pSysCompBuf->metaBufSize = pVidBuf->metaBufSize;
                    pSysCompBuf->metaFillLength = pVidBuf->metaFillLength;
                    pSysCompBuf->chInfo = pVidBuf->chInfo;
                    pSysCompBuf->flags = pVidBuf->flags;
                }
            }
            else
            if(chBuf->bufType==SYSTEM_BUFFER_TYPE_METADATA)
            {
                System_MetaDataBuffer *pMetaBuf;

                pMetaBuf = (System_MetaDataBuffer*) chBuf->payload;

                for (i=0; i < SYSTEM_MAX_PLANES; i++)
                {
                    pSysCompBuf->bufAddr[i][pSysCompBuf->numFrames] =
                                                         pMetaBuf->bufAddr[i];
                }
                pSysCompBuf->metaBufAddr[pSysCompBuf->numFrames] = NULL;

                if(firstSyncCh)
                {
                    firstSyncCh = FALSE;
                    /* Other fields of Composite buffer can be anything,
                     * so we arbitarily
                     * choose the first synced channel
                     */
                    pSysCompBuf->metaBufSize = pMetaBuf->metaBufSize[0];
                    pSysCompBuf->metaFillLength = pMetaBuf->metaFillLength[0];
                    memset(&pSysCompBuf->chInfo,
                            0,
                            sizeof(pSysCompBuf->chInfo));
                    pSysCompBuf->flags = pMetaBuf->flags;
                }
            }

            pSysCompBuf->numFrames++;
            origBufPtr->bufPtr[chId] = chBuf;
            pObj->latestSyncDelta.bufferTimestamp[chId] = chBuf->srcTimestamp/1000;

            syncDelta = pObj->masterTimeStamp - chBuf->srcTimestamp/1000;

            if(syncDelta<0)
                syncDelta = -syncDelta;

            pObj->stats.totalSyncDelta += syncDelta;
            pObj->stats.syncDeltaCount++;

            Utils_updateLatency(&pObj->linkStatsInfo->linkLatency,
                            chBuf->linkLocalTimestamp);

        }
    }
    origBufPtr->numChannels = pObj->createArgs.chParams.numCh;

    pObj->latestSyncDelta.masterTimestamp = pObj->masterTimeStamp;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief This function drops all buffers present in the drop list and gives
 *        them back to the next link.
 *
 * \param  pObj     [IN]  Sync link instance handle
 *
 *******************************************************************************
*/
Void SyncLink_dropBuffers(SyncLink_Obj * pObj)
{
    if (pObj->dropBufList.numBuf)
    {
        System_putLinksEmptyBuffers(
                                     pObj->createArgs.inQueParams.prevLinkId,
                                     pObj->createArgs.inQueParams.prevLinkQueId,
                                     &pObj->dropBufList
                                   );
    }
    pObj->dropBufList.numBuf = 0;
    return;
}

/**
 *******************************************************************************
 * \brief This function computes the master timestamp only when all active
 *        channels have buffers
 *
 * \param  pObj     [IN]  Sync link instance handle
 * \return FALSE if masterTimeStamp is calculated
 *         TRUE if masterTimeStamp is not calculated if an active channel has
 *         no buffer
 *
 *******************************************************************************
*/
Int32 SyncLink_computeMasterTimeStamp(SyncLink_Obj * pObj)
{
    UInt32 chId;
    UInt64 sumOfAllTimestamps = 0;
    UInt32 averageTimeStamp = 0;
    System_Buffer *pBuffer;
    SyncLink_ChannelParams *chPrms;
    UInt32 chTimeStamps[SYNC_LINK_MAX_CHANNELS];
    Bool flag = FALSE;

    chPrms = &pObj->createArgs.chParams;

    for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
    {
        if (chPrms->channelSyncList[chId])
        {
             Utils_quePeek(
                            &pObj->chObj[chId].localQueHandle,
                            (Ptr *) &pBuffer
                          );
            if (pBuffer)
            {
                chTimeStamps[chId] = pBuffer->srcTimestamp/1000;
            }
            else
            {
                flag = TRUE;
                break;
            }
        }
    }

    if (flag)
    {
        return TRUE;
    }
    else
    {
        /*
         * Check whether these timestamps have wrapped around and apply
         * necessary logic for handling wrap around
         */
         SyncLink_handleTimeStampWrapAround(
                                            chTimeStamps,
                                            pObj->createArgs.chParams.numCh
                                            );
         for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
         {
            sumOfAllTimestamps += chTimeStamps[chId];
         }
         averageTimeStamp = sumOfAllTimestamps / chPrms->numCh;
         pObj->masterTimeStamp = averageTimeStamp;
    }
    return FALSE;
}

/**
 *******************************************************************************
 * \brief This function does the following,
 *
 *     - For each local queue check buffers have become too old, based on the
 *       threshold
 *     - Fill each local queue with the buffers got from previous link
 *     - If all local queues have buffers, then
 *        - Dequeue from local queues
 *        - Apply sync logic
 *        - If sync logic is successful, construct a composite buffer
 *            - Get a system buffer from Empty Queue
 *            - Get the composite buffer from the system buffer
 *            - Fill composite buffer with buffers dequeued from local queues
 *            - Get SyncLink_OrigBufferPtr from system byffer and fill it
 *              with the addresses of dequeued buffers
 *        - Send next link that data is available
 *
 * \param  pObj     [IN]  DUP link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SyncLink_drvProcessData(SyncLink_Obj * pObj)
{
    UInt32 syncDone;
    Int32 localQueEmptyFlag;
    System_Buffer *pBuffer;
    UInt32 status, chId;
    SyncLink_ChannelParams *chPrms = &pObj->createArgs.chParams;

    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    if(pObj->isFirstFrame==FALSE)
    {
        pObj->isFirstFrame = TRUE;

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);

        Utils_resetLinkStatistics(
                              &linkStatsInfo->linkStats,
                              pObj->createArgs.chParams.numCh,
                              1
                             );
    }

    linkStatsInfo->linkStats.newDataCmdCount++;
    pObj->dropBufList.numBuf = 0;

    SyncLink_addBuffersToDropListLesserThanThreshold(pObj);

    SyncLink_dropBuffers(pObj);

    SyncLink_fillLocalQueues(pObj);

    SyncLink_dropBuffers(pObj);

    do{
          localQueEmptyFlag = SyncLink_computeMasterTimeStamp(pObj);

          if (localQueEmptyFlag)
          {
              break;
          }

          syncDone = TRUE;
          SyncLink_computeDecisionParams(pObj, &syncDone);

          if(syncDone == FALSE)
          {
              SyncLink_addBuffersToDropListNotInSync(pObj);
              SyncLink_dropBuffers(pObj);
          }
          else
          {
              status = Utils_bufGetEmptyBuffer(&pObj->outFrameQue,
                                                        &pBuffer, BSP_OSAL_NO_WAIT);

            if(status == SYSTEM_LINK_STATUS_SOK)
            {
                UTILS_assert(pBuffer != NULL);

                SyncLink_makeCompositeBuffer(pObj, pBuffer);

                linkStatsInfo->linkStats.chStats[0].outBufCount[0]++;

                Utils_updateLatency(&pObj->linkStatsInfo->srcToLinkLatency,
                                    pBuffer->srcTimestamp);

                status = Utils_bufPutFullBuffer(&pObj->outFrameQue, pBuffer);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                System_sendLinkCmd(pObj->createArgs.outQueParams.nextLink,
                                    SYSTEM_CMD_NEW_DATA, NULL);
            }
            else
            {
                /*
                 As output queue is not having any free the buffers,
                 We can free the input buffer at this stage.
                 Ideally this should hit this situation and sync link has more
                 buffers than the prior link.
                */
                chPrms = &pObj->createArgs.chParams;
                for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
                {
                    if (chPrms->channelSyncList[chId])
                    {
                        status = Utils_queGet(&pObj->chObj[chId].localQueHandle,
                                            (Ptr *) &pBuffer,
                                            1,
                                            BSP_OSAL_NO_WAIT);
                        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                        UTILS_assert(pBuffer != NULL);

                        pObj->dropBufList.buffers[pObj->dropBufList.numBuf] = pBuffer;
                        pObj->dropBufList.numBuf++;
                        linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
                    }
                }
            }
          }
          if(pObj->dropBufList.numBuf)
          {
            SyncLink_dropBuffers(pObj);
          }
          else
          {
            /* To avoid Misra Error */
          }
      }while(1);

      return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to Sync link to get output queue
 *    Information of DUP link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  info     [OUT] output queues information of DUP link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SyncLink_getLinkInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    SyncLink_Obj *pObj = (SyncLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->linkInfo, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to Sync link to return back
 *    buffers
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [IN]  A List of buffers returned back to DUP link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SyncLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                              System_BufferList * pBufList)
{
    Uint32 bufId, i;
    Int32 status;
    System_Buffer *pBuf, *pOrigBuf;
    SyncLink_OrigBufferPtr *pOrigPtr;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    SyncLink_Obj *pObj = (SyncLink_Obj *) pTsk->appData;
    System_BufferList freeBufferList;

    pObj->linkStatsInfo->linkStats.putEmptyBufCount++;

    freeBufferList.numBuf = 0;

    for (bufId = 0; bufId < pBufList->numBuf; bufId++)
    {
        pBuf = pBufList->buffers[bufId];

        if (pBuf == NULL)
        {
            continue;
            // Do we need to record this in linkStats?? Which one??
        }
        pOrigPtr = (SyncLink_OrigBufferPtr *)pBuf->pSyncLinkOrgBufferPtr;

        for (i = 0; i < pOrigPtr->numChannels; i++)
        {
            pOrigBuf = pOrigPtr->bufPtr[i];
            if (pOrigBuf)
            {
                freeBufferList.buffers[freeBufferList.numBuf] = pOrigBuf;
                freeBufferList.numBuf++;
            }

        }
    }

    System_putLinksEmptyBuffers(pObj->createArgs.inQueParams.prevLinkId,
                               pObj->createArgs.inQueParams.prevLinkQueId,
                               &freeBufferList);
    status = Utils_bufPutEmpty(&pObj->outFrameQue, pBufList);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to Sync link to get data from
 *    the output queue of DUP link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [OUT] A List of buffers needed for the next link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SyncLink_getFullBuffers(Void * ptr, UInt16 queId,
                            System_BufferList * pBufList)
{
    Int32 status;
    SyncLink_Obj *pObj;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    pObj = (SyncLink_Obj *) pTsk->appData;

    pObj->linkStatsInfo->linkStats.getFullBufCount++;
    status = Utils_bufGetFull(&pObj->outFrameQue, pBufList,
                              BSP_OSAL_NO_WAIT);
    return status;
}

/**
 *******************************************************************************
 * \brief Deletes all allocated structures in create phase
 *
 * \param  pObj     [IN]  DUP link instance handle
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SyncLink_drvDelete(SyncLink_Obj *pObj)
{
    Int32 status, chId;
    SyncLink_ChObj *chObj;

    status = Utils_bufDelete(&pObj->outFrameQue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    for (chId = 0; chId < pObj->createArgs.chParams.numCh; chId++)
    {
        chObj = &pObj->chObj[chId];
        status = Utils_queDelete(&chObj->localQueHandle);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    BspOsal_clockDelete(&pObj->timer);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Print Sync link related statistics
 *
 * \param  pObj     [IN] Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 SyncLink_drvPrintStatistics(SyncLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    char                 tskName[32];
    SyncLink_BufferStats *stats = &pObj->stats;

    sprintf(tskName, "SYNC_LINK_%u", (unsigned int)pObj->linkInstId);

    Utils_printLinkStatistics(&pObj->linkStatsInfo->linkStats, tskName, TRUE);

    Utils_printLatency(tskName,
                       &pObj->linkStatsInfo->linkLatency,
                       &pObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    Vps_printf(" \n");
    Vps_printf(" [%s] Additional Statistics, \r\n", tskName);
    Vps_printf(" ******************************** \r\n");
    Vps_printf(" Total Frames Dropped   = %d frames\r\n", stats->totalDropCount);
    if(stats->syncDeltaCount)
    {
        Vps_printf(" Average Sync Diff      = %d ms\r\n",
                stats->totalSyncDelta/stats->syncDeltaCount);
    }
    Vps_printf(" \r\n");

    stats->totalDropCount = 0;
    stats->totalSyncDelta = 0;
    stats->syncDeltaCount = 0;

    #if 0
    {
        /* enable if more detailed stats are required
         * NOTE: this will generate lots of prints
         */
        SyncLink_DropBufferStats *dropStats = stats->dropStats;
        UInt32 i, oldIndex;

        i = stats->totalDropCount % SYNC_LINK_MAX_DROP_BUFFER_STATS;
        oldIndex = i;
        Vps_printf(" \n");
        Vps_printf(" Dropped Frame Statistics, \r\n");
        Vps_printf(" \n");
        do
        {
            if(dropStats[oldIndex].curTimestamp)
            {
                Vps_printf(" "
                           " %3d: "
                           " CH%-2d "
                           " recvTimeStamp = %5d ms "
                           " masterTimeStamp = %5d ms "
                           " bufferTimeStamp = %5d ms\r\n",
                             oldIndex,
                             dropStats[oldIndex].chNum,
                             dropStats[oldIndex].curTimestamp,
                             dropStats[oldIndex].masterTimestamp,
                             dropStats[oldIndex].bufferTimestamp
                          );
            }
            oldIndex = (oldIndex + 1) % SYNC_LINK_MAX_CHANNELS;
        }while(oldIndex != i);
        Vps_printf(" \n");
    }
    #endif

    return status;
}

/**
 ******************************************************************************
 *
 * \brief Function to start the link.
 *
 * This function starts the timer, which will be used to send buffers at a
 * fixed interval on output queue
 *
 * \param  pObj           [IN] Null Src link global handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 ******************************************************************************
 */
Int32 SyncLink_start(SyncLink_Obj * pObj)
{
    BspOsal_clockStart(pObj->timer);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 ******************************************************************************
 *
 * \brief Function to stop the link.
 *
 * Post this call, buffers will not be sent to output queue
 *
 * \param  pObj           [IN] Null Src link global handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 ******************************************************************************
 */
Int32 SyncLink_stop(SyncLink_Obj * pObj)
{
    BspOsal_clockStop(pObj->timer);

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating Sync link
 *     - Arrival of new data
 *     - Deleting Sync link
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Void SyncLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = 0;
    UInt32 flushCmds[1];

    SyncLink_Obj *pObj = (SyncLink_Obj*) pTsk->appData;
    SyncLink_LatestSyncDelta *latestSyncDelta;

    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_CREATE start !!!\n");
//#endif
            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
                status = SyncLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));
                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    pObj->state = SYSTEM_LINK_STATE_CREATED;
                }
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_CREATE end !!!\n");
//#endif
            break;

        case SYSTEM_CMD_NEW_DATA:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_NEW_DATA start !!!\n");
//#endif
            Utils_tskAckOrFreeMsg(pMsg, status);

            flushCmds[0] = SYSTEM_CMD_NEW_DATA;
            Utils_tskFlushMsg(pTsk, flushCmds, 1);

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = SyncLink_drvProcessData(pObj);
            }
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_NEW_DATA end !!!\n");
//#endif
            break;

        case SYSTEM_CMD_START:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_START start !!!\n");
//#endif
            if(pObj->state==SYSTEM_LINK_STATE_CREATED)
            {
                status = SyncLink_start(pObj);
                pObj->state = SYSTEM_LINK_STATE_RUNNING;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_START end !!!\n");
//#endif
            break;

        case SYSTEM_CMD_STOP:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_STOP start !!!\n");
//#endif
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = SyncLink_stop(pObj);
                pObj->state = SYSTEM_LINK_STATE_CREATED;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_STOP end !!!\n");
//#endif
            break;

        case SYSTEM_CMD_DELETE:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_DELETE start !!!\n");
//#endif
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = SyncLink_stop(pObj);
                pObj->state = SYSTEM_LINK_STATE_CREATED;
            }
            if(pObj->state==SYSTEM_LINK_STATE_CREATED)
            {
                status = SyncLink_drvDelete(pObj);
                pObj->state = SYSTEM_LINK_STATE_IDLE;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_INTERVAL
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_DELETE end !!!\n");
//#endif
            break;

        case SYNC_LINK_CMD_GET_LATEST_SYNC_DELTA:
//#ifdef SYSTEM_RT_STATS_LOG_INTERVAL
//    Vps_printf(" SYNC: tskMain SYNC_LINK_CMD_GET_LATEST_SYNC_DELTA start !!!\n");
//#endif
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                latestSyncDelta =
                          (SyncLink_LatestSyncDelta *)Utils_msgGetPrm(pMsg);
                memcpy(
                    latestSyncDelta,
                    &pObj->latestSyncDelta,
                    sizeof(pObj->latestSyncDelta)
                   );
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYNC_LINK_CMD_GET_LATEST_SYNC_DELTA end !!!\n");
//#endif
            break;

        case SYSTEM_CMD_PRINT_STATISTICS:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskRun SYSTEM_CMD_PRINT_STATISTICS start !!!\n");
//#endif
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = SyncLink_drvPrintStatistics(pObj);
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain SYSTEM_CMD_PRINT_STATISTICS end !!!\n");
//#endif
            break;

        default:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" SYNC: tskMain default !!!\n");
//#endif
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

    }
    return;
}

/**
 *******************************************************************************
 *
 * \brief Init function for Sync link. This function does the following for each
 *   Sync link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SyncLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 syncId;
    SyncLink_Obj *pObj;
    UInt32 procId = System_getSelfProcId();

    for(syncId = 0; syncId < SYNC_LINK_OBJ_MAX; syncId++)
    {
        pObj = &gSyncLink_obj[syncId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->tskId = SYSTEM_MAKE_LINK_ID(procId,
                                          SYSTEM_LINK_ID_SYNC_0 + syncId);

        pObj->state = SYSTEM_LINK_STATE_IDLE;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = SyncLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = SyncLink_putEmptyBuffers;
        linkObj.getLinkInfo = SyncLink_getLinkInfo;

        System_registerLink(pObj->tskId, &linkObj);

        pObj->linkInstId = syncId;

        /*
         * Create link task, task remains in IDLE state.
         * DisplayLink_tskMain is called when a message command is received.
         */
        status = SyncLink_tskCreate(syncId);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for DUP link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SyncLink_deInit()
{
    UInt32 syncId;

    for (syncId = 0; syncId < SYNC_LINK_OBJ_MAX; syncId++)
    {
        Utils_tskDelete(&gSyncLink_obj[syncId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
}


