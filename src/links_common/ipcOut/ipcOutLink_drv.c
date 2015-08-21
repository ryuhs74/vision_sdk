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
 * \file ipcOutLink_drv.c
 *
 * \brief  This file has the implementataion of IPC OUT Link API
 *
 *         This file implements the software logic needed to exchange frames
 *         between processors
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "ipcOutLink_priv.h"


/**
 *******************************************************************************
 *
 * \brief This function is called when next link send a notify or
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
Void IpcOutLink_drvNotifyCb(Utils_TskHndl * pTsk)
{
    IpcOutLink_Obj * pObj = (IpcOutLink_Obj * )pTsk->appData;

    pObj->linkStatsInfo->linkStats.notifyEventCount++;

    if(Utils_ipcQueIsEmpty(&pObj->ipcIn2OutQue)==FALSE)
    {
        /*
         * send command to release frames only if there are elements in the
         * que
         */
        System_sendLinkCmd(pObj->linkId,
                            IPC_OUT_LINK_CMD_RELEASE_FRAMES,
                            NULL);
    }
}

/**
 *******************************************************************************
 *
 * \brief Channel specific initialization
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_drvChCreate(IpcOutLink_Obj *pObj)
{
    IpcOutLink_ChObj *pChObj;
    UInt32 chId;

    for(chId=0; chId<SYSTEM_MAX_CH_PER_OUT_QUE; chId++)
    {
        pChObj = &pObj->chObj[chId];

        /* reset skip buf context to not skip any frames */
        Utils_resetSkipBufContext(&pChObj->bufSkipContext, 30, 30);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Create IPC Out link
 *
 *        Following happens during create phase,
 *        - Call 'get link info” on previous link.
 *          When IPC IN from other processers asks link info it gives back
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
 * \param  pPrm     [IN]  Create arguments
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_drvCreate(IpcOutLink_Obj *pObj, IpcLink_CreateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    char                 tskName[32];

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_OUT_%d   : Create in progress !!!\n",
               pObj->linkInstId
              );
#endif

    UTILS_MEMLOG_USED_START();

    /* keep a copy of create args */
    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    /* get previous link info */
    status = System_linkGetInfo(
                    pObj->createArgs.inQueParams.prevLinkId,
                    &pObj->prevLinkInfo
                );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* validate previous link que ID */
    UTILS_assert(
        pObj->createArgs.inQueParams.prevLinkQueId <
            pObj->prevLinkInfo.numQue
        );

    /*
     * Setup current link que information
     * Current queue is considered to have one output queue
     * with que information same as selected previous link queue
     */
    pObj->linkInfo.numQue = 1;
    memcpy(&pObj->linkInfo.queInfo[0],
           &pObj->prevLinkInfo.queInfo
                [pObj->createArgs.inQueParams.prevLinkQueId],
           sizeof(pObj->linkInfo.queInfo[0])
          );


    /* allocate shared memory for IPC queue */
    pObj->ipcOut2InSharedMemBaseAddr =
                System_ipcGetIpcOut2InQue(pObj->linkId);
    UTILS_assert(pObj->ipcOut2InSharedMemBaseAddr!=NULL);

    pObj->ipcIn2OutSharedMemBaseAddr =
                System_ipcGetIpcIn2OutQue(pObj->linkId);
    UTILS_assert(pObj->ipcIn2OutSharedMemBaseAddr!=NULL);

    /* create IPC queue's */
    status = Utils_ipcQueCreate(
                        &pObj->ipcOut2InQue,
                        SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS,
                        pObj->ipcOut2InSharedMemBaseAddr,
                        sizeof(UInt32)
                    );
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    status = Utils_ipcQueCreate(
                        &pObj->ipcIn2OutQue,
                        SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS,
                        pObj->ipcIn2OutSharedMemBaseAddr,
                        sizeof(UInt32)
                    );
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);


    /* reset IPC queue's */

    Utils_ipcQueReset(&pObj->ipcOut2InQue,
                        pObj->ipcOut2InSharedMemBaseAddr,
                        TRUE,
                        TRUE
                    );
    Utils_ipcQueReset(&pObj->ipcIn2OutQue,
                        pObj->ipcIn2OutSharedMemBaseAddr,
                        TRUE,
                        TRUE
                    );

    {
        UInt32 elemId;

        /* create local queue */
        status = Utils_queCreate(&pObj->localQue,
                             SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS,
                             pObj->localQueMem,
                             UTILS_QUE_FLAG_NO_BLOCK_QUE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);


        /* allocate memory for IPC data structure's in shared memory */
        for(elemId=0; elemId <SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS; elemId++)
        {
            /* queue to local queue
             * local que and IPC queue only stores index's, actual pointer to
             * IPC Buffer can be retrrieved via API System_ipcGetIpcBuffer
             * which takes IPC Out link ID and index as input
             */
            status = Utils_quePut(&pObj->localQue,
                         (Ptr)elemId,
                         0
                    );
            UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
        }
    }

    /* TODO: statistics */

    IpcOutLink_drvChCreate(pObj);

    pObj->isFirstFrameRecv = FALSE;

    sprintf(tskName, "IPC_OUT_%u", (unsigned int)pObj->linkInstId);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->linkId, tskName);
    UTILS_assert(NULL != pObj->linkStatsInfo);

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("IPC_OUT:",
                   pObj->memUsed,
                   UTILS_ARRAYSIZE(pObj->memUsed));

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_OUT_%d   : Create Done !!!\n",
           pObj->linkInstId
          );
#endif


    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete IPC Out link
 *
 *        This function free's resources allocated during create
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_drvDelete(IpcOutLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_OUT_%d   : Delete in progress !!!\n", pObj->linkInstId);
#endif

    /* delete local queue */
    Utils_queDelete(&pObj->localQue);


    /* delete ipc queue */
    Utils_ipcQueDelete(&pObj->ipcIn2OutQue);
    Utils_ipcQueDelete(&pObj->ipcOut2InQue);

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* no need for any channel delete logic */

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_OUT_%d   : Delete Done !!!\n",
           pObj->linkInstId
          );
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Copy information from system buffer to IPC buffer
 *
 * \param  pObj        [IN]  Link object
 * \param  pBuffer     [IN]  Pointer to system buffer information
 * \param  pIpcBuffer  [IN]  Pointer to IPC buffer information
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void  IpcOutLink_drvCopySystemBufferToIpcBuffer(
                    IpcOutLink_Obj *pObj,
                    System_Buffer *pBuffer,
                    System_IpcBuffer *pIpcBuffer
                    )
{
    UInt32 flags = 0;

    SYSTEM_BUFFER_FLAG_SET_BUF_TYPE(flags, pBuffer->bufType);
    SYSTEM_BUFFER_FLAG_SET_CH_NUM(flags, pBuffer->chNum);
    SYSTEM_BUFFER_FLAG_SET_PAYLOAD_SIZE(flags, pBuffer->payloadSize);

    pIpcBuffer->flags               = flags;
    pIpcBuffer->orgSystemBufferPtr  = (UInt32)pBuffer;
    pIpcBuffer->srcTimestamp        = pBuffer->srcTimestamp;
    pIpcBuffer->linkLocalTimestamp  = pBuffer->linkLocalTimestamp;

    pIpcBuffer->ipcPrfTimestamp64[0] = pBuffer->ipcPrfTimestamp64[0];

    UTILS_assert(pBuffer->payloadSize <= SYSTEM_MAX_PAYLOAD_SIZE );

    memcpy(pIpcBuffer->payload, pBuffer->payload, pBuffer->payloadSize);
}

/**
 *******************************************************************************
 *
 * \brief Check if frame needs to be skipped
 *
 * \param  pObj     [IN] Link object
 * \param  chNum    [IN] channel Id
 *
 * \return TRUE: skip frame, FALSE: process frame
 *
 *******************************************************************************
 */
Bool IpcOutLink_drvDoFrameSkip(IpcOutLink_Obj *pObj, UInt32 chNum)
{
    System_LinkQueInfo *pLinkQueInfo;
    IpcOutLink_ChObj *pChObj;

    pLinkQueInfo = &pObj->linkInfo.queInfo[0];

    if(chNum >=  pLinkQueInfo->numCh )
        return TRUE; /* invalid channel, skip frame */

    pChObj = &pObj->chObj[chNum];

    return Utils_doSkipBuf(&pChObj->bufSkipContext);
}

/**
 *******************************************************************************
 *
 * \brief Process buffer's
 *
 *        - Previous link will send a command NEW_DATA to IPC out link when
 *          buffers are available to be sent across processors
 *
 *        - IPC Out link will call 'get full buffer' to get the buffer
 *           information pointers
 *
 *        - For each buffer information pointer it will
 *          -  Apply frame-rate control and release frame immediately if this
 *             frame needs to be dropped
 *             - continue to next buffer pointer
 *          -  If uni-cache operation
 *             - Insert the pointer into the IPC queue (IPC OUT-> IPC IN Q)
 *             - continue to next buffer pointer
 *          -  If non-uni-cache operation
 *             - Get a IPC shared memory buffer information structures
 *               pointer from local queue
 *             - If no buffer information structures pointer in local queue
 *               then release incoming buffer immediately to previous link
 *               and continue. This should normally not happen but idea is
 *               to let processing continue even in this case
 *             - Translate the information from previous link buffer pointer
 *               to a IPC shared memory buffer information structure
 *             - This is where address is translated if required in case
 *               address space between the two core’s is different
 *             - The original system buffer pointer is also set in the IPC
 *               shared memory buffer information structure
 *             - Insert the IPC shared memory buffer information structure
 *               pointer in the IPC queue (IPC OUT-> IPC IN Q)
 *             - continue to next buffer pointer
 *
 *        - If notify is enabled and at least one element was put into
 *          IPC queue then send a notify to the other processor with
 *          IPC IN link ID as the notify payload. The next processor ID
 *          is embedded inside the next link ID
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_drvProcessBuffers(IpcOutLink_Obj *pObj)
{
    Int32             status      = SYSTEM_LINK_STATUS_SOK;
    System_Buffer     *pBuffer;
    UInt32            index;
    System_BufferList bufList;
    System_BufferList freeBufList;
    Bool              sendNotify  = FALSE;
    UInt32            bufId;
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

    linkStatsInfo->linkStats.newDataCmdCount++;

    bufList.numBuf = 0;
    System_getLinksFullBuffers(
                    pObj->createArgs.inQueParams.prevLinkId,
                    pObj->createArgs.inQueParams.prevLinkQueId,
                    &bufList);

    freeBufList.numBuf = 0;

    if(bufList.numBuf)
    {
        for (bufId = 0; bufId < bufList.numBuf; bufId++)
        {

            pBuffer = bufList.buffers[bufId];
            if(pBuffer==NULL
                ||
               pBuffer->chNum >= pObj->linkInfo.queInfo[0].numCh)
            {
                linkStatsInfo->linkStats.inBufErrorCount++;
                continue; /* invalid buffer pointer, skip it */
            }
            pBuffer->ipcPrfTimestamp64[0] = Utils_getCurGlobalTimeInUsec();

            pBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

            linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufRecvCount++;

            if(IpcOutLink_drvDoFrameSkip(pObj, pBuffer->chNum))
            {
                linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufUserDropCount++;

                /* skip frame */
                UTILS_assert(freeBufList.numBuf <
                                SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

                freeBufList.buffers[freeBufList.numBuf] = pBuffer;
                freeBufList.numBuf++;

                continue;
            }

            Utils_updateLatency(&linkStatsInfo->linkLatency,
                                pBuffer->linkLocalTimestamp);
            Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                pBuffer->srcTimestamp);

            {
                System_IpcBuffer *pIpcBuffer;

                index = (UInt32)(-1);

                status =
                    Utils_queGet(&pObj->localQue,
                                (Ptr *) &index,
                                1,
                                BSP_OSAL_NO_WAIT
                                );

                pIpcBuffer = System_ipcGetIpcBuffer(pObj->linkId, index);

                if(status!=SYSTEM_LINK_STATUS_SOK || pIpcBuffer == NULL)
                {
                    linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;

                    /* if could not get free element from local queue,
                     * then free the system buffer
                     */

                    UTILS_assert(freeBufList.numBuf <
                                    SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

                    freeBufList.buffers[freeBufList.numBuf] = pBuffer;
                    freeBufList.numBuf++;

                    continue;
                }

                IpcOutLink_drvCopySystemBufferToIpcBuffer(
                    pObj,
                    pBuffer,
                    pIpcBuffer
                    );

                pIpcBuffer->ipcPrfTimestamp64[1] = Utils_getCurGlobalTimeInUsec();
            }

            status = Utils_ipcQueWrite(
                            &pObj->ipcOut2InQue,
                            (UInt8*)&index,
                            sizeof(UInt32)
                            );

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;

                /* if could not add element to queue, then free the
                 * system buffer
                 */

                UTILS_assert(freeBufList.numBuf <
                                SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

                freeBufList.buffers[freeBufList.numBuf] = pBuffer;
                freeBufList.numBuf++;
            }
            else
            {
                linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufProcessCount++;

                linkStatsInfo->linkStats.chStats[pBuffer->chNum].outBufCount[0]++;

                /* atleast one element successfuly inserted in the IPC que
                 * So send notify to next link
                 */
                sendNotify = TRUE;
            }
        }

        if(freeBufList.numBuf)
        {
            System_putLinksEmptyBuffers(
                pObj->createArgs.inQueParams.prevLinkId,
                pObj->createArgs.inQueParams.prevLinkQueId,
                &freeBufList
                );
        }

        /* if notify mode is enabled and atleast one element added to que
         * then send notify
         */
        if(sendNotify)
        {
            System_ipcSendNotify(pObj->createArgs.outQueParams.nextLink);
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Release buffer returned by IPC IN link to previous link
 *
 * \param  pObj     [IN] Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_drvReleaseBuffers(IpcOutLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Int32 queStatus = SYSTEM_LINK_STATUS_SOK;
    System_Buffer    *pBuffer;
    UInt32 index;
    System_BufferList freeBufList;

    pObj->linkStatsInfo->linkStats.releaseDataCmdCount++;

    freeBufList.numBuf = 0;

    while(queStatus == SYSTEM_LINK_STATUS_SOK)
    {
        while(1)
        {
            index = (UInt32)-1;

            queStatus = Utils_ipcQueRead(
                    &pObj->ipcIn2OutQue,
                    (UInt8*)&index,
                    sizeof(UInt32)
                    );

            if(queStatus!=SYSTEM_LINK_STATUS_SOK)
                break; /* no more data to read from IPC queue */

            {
                System_IpcBuffer *pIpcBuffer;

                pIpcBuffer = System_ipcGetIpcBuffer(pObj->linkId, index);

                if(pIpcBuffer == NULL)
                {
                    pObj->linkStatsInfo->linkStats.inBufErrorCount++;
                    continue; /* this condition will not happen */
                }

                pBuffer = (System_Buffer*)pIpcBuffer->orgSystemBufferPtr;

                /* queue to local queue */
                status = Utils_quePut(&pObj->localQue,
                             (Ptr)index,
                             0
                        );
                UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
            }

            if(pBuffer==NULL)
            {
                pObj->linkStatsInfo->linkStats.inBufErrorCount++;
                continue; /* this condition will not happen */
            }

            freeBufList.buffers[freeBufList.numBuf] = pBuffer;
            freeBufList.numBuf++;

            if(freeBufList.numBuf>=SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST)
                break;
        }

        if(freeBufList.numBuf)
        {
            System_putLinksEmptyBuffers(
                pObj->createArgs.inQueParams.prevLinkId,
                pObj->createArgs.inQueParams.prevLinkQueId,
                &freeBufList
                );
        }
    }
    return queStatus;
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
Int32 IpcOutLink_drvStop(IpcOutLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_IPC
    Vps_printf(" IPC_OUT_%d   : Stop Done !!!\n", pObj->linkInstId);
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Set frame-rate
 *
 *        This function controls the rate at which buffers are
 *        processed and dropped
 *
 * \param  pObj     [IN] Link object
 * \param  pPrm     [IN] Frame rate control parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IpcOutLink_drvSetFrameRate(IpcOutLink_Obj *pObj,
                              IpcLink_FrameRateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_LinkQueInfo *pLinkQueInfo;
    IpcOutLink_ChObj *pChObj;

    pLinkQueInfo = &pObj->linkInfo.queInfo[0];

    if(pPrm->chNum >=  pLinkQueInfo->numCh )
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

    pChObj = &pObj->chObj[pPrm->chNum];

    Utils_resetSkipBufContext(
            &pChObj->bufSkipContext,
            pPrm->inputFrameRate,
            pPrm->outputFrameRate
            );

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
Int32 IpcOutLink_drvPrintStatistics(IpcOutLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    char                 tskName[32];

    sprintf(tskName, "IPC_OUT_%u", (unsigned int)pObj->linkInstId);

    UTILS_assert(NULL != pObj->linkStatsInfo);

    Utils_printLinkStatistics(&pObj->linkStatsInfo->linkStats, tskName, TRUE);

    Utils_printLatency(tskName,
                       &pObj->linkStatsInfo->linkLatency,
                       &pObj->linkStatsInfo->srcToLinkLatency,
                       TRUE);

    return status;
}

