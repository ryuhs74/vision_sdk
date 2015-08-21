/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file vpeLink_drvCommon.c
 *
 * \brief  This file has the implementation of VPE Link
 *
 *   VPE Link can be used to do processing on video input frames. These
 *   frames may be from capture or decoded video frames coming over network.
 *   Video processing capability is present on several sub systems (VPE, ISS, DSS etc) in the SOC.
 *   Code which is common across sub systems is placed in this file.
 *
 *   VPE can do
 *   - Color space conversion on input frames.
 *   - Color space conversion while outputting the frame to memory.
 *   - Scaling on input frames.
 *   - De-Interlacing, (conversion from field to frames )
 *
 *     The VPE link receives the input frames, submitted/queued them into VPS
 *     VPE driver along with a set of output frames to which the VPE driver
 *     write the de-interlaced/scaled output. once the processing is over
 *     the driver invoke a call back. On call back VPE Link reclaim these
 *     frames which are already processed and send back to the previous link.
 *     Also send out the output frames to the next link
 *
 *     VPE is validated only for DEI in Bypass mode (Bypass = TRUE).  This
 *     is because no HW set-up available currently to feed interlaced input
 *
 *     VPE link also supports the run time input and output resolution
 *     change - This feature is NOT verified in this version.
 *
 * \version 0.0 (Nov 2014) : [PS] Created First version from existing VPE link
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "vpeLink_priv.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 * \brief This is the Call back function registered with VPE driver
 *
 *        This call back function will be invoked by VPE driver once it
 *        processed the frames from a FVID2_frame list
 *
 * \param  handle    [IN] VPE driver handle
 * \param  appData   [IN] Set with link instance handle address
 * \param  reserved  [IN] Not used
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvFvidCb(FVID2_Handle handle, Ptr appData, Ptr reserved)
{
    VpeLink_Obj *pObj = (VpeLink_Obj *) appData;

    BspOsal_semPost(pObj->complete);

    pObj->linkStatsInfo->linkStats.notifyEventCount++;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This is the Error Call back function registered with VPE driver
 *
 *        This call back function will be invoked by driver once the
 *        processed frames from a FVID2_frame list are in error state
 *
 * \param  handle    [IN] VPE driver handle
 * \param  appData   [IN] Set with link instance handle address
 * \param  errList   [IN] Set with link instance handle address
 * \param  reserved  [IN] Not used
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvFvidErrCb(FVID2_Handle handle,
                           Ptr appData, Ptr errList, Ptr reserved)
{
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function create the VPE link Output buffer free queue
 *
 *        Creates VPE links Output system buffer Queue, Also put the free
 *        buffers into this queue. initialize the FVID2_frames and videoFrames
 *        VPE link de-queue the free output buffers from this freeQ,
 *        while submitting to the VPE driver
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   pObj     [IN] Output queue ID
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
static Int32 VpeLink_createVpeOutChBufferQueue (VpeLink_Obj * pObj, UInt32 outId)
{
    Int32 status;
    UInt32 chId, bufId, planes;
    VpeLink_OutObj *pOutObj;
    FVID2_Frame *frames;
    System_Buffer   *buffers;
    System_VideoFrameBuffer *videoFrames;
    UInt32 flags;

    pOutObj = &pObj->outObj[outId];

    for (chId=0; chId<pObj->inQueInfo.numCh; chId++)
    {
        if (pOutObj->numFrames[chId] > 0)
        {
            status = Utils_queCreate(&pOutObj->emptyBufQue[chId],
                                     pOutObj->numFrames[chId],
                                     pOutObj->bufsMem[chId],
                                     UTILS_QUE_FLAG_BLOCK_QUE_GET);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            for (bufId = 0; bufId < pOutObj->numFrames[chId]; bufId++)
            {
                frames = &pOutObj->frames[chId][bufId];
                buffers = &pOutObj->buffers[chId][bufId];
                videoFrames = &pOutObj->videoFrames[chId][bufId];

                memset(buffers, 0, sizeof(*buffers));
                memset(videoFrames, 0, sizeof(*videoFrames));

                buffers->bufType      = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
                buffers->chNum        = chId;
                buffers->payloadSize  = sizeof(System_VideoFrameBuffer);
                buffers->payload      = videoFrames;
                buffers->pVpeLinkPrivate = frames;

                frames->perFrameCfg   = NULL;
                frames->subFrameInfo  = NULL;
                frames->reserved      = NULL;
                frames->appData       = buffers;
                frames->chNum         = buffers->chNum;

                flags = videoFrames->chInfo.flags;
                SYSTEM_LINK_CH_INFO_SET_FLAG_IS_RT_PRM_UPDATE(flags, 0);

                for (planes = 0; planes < SYSTEM_MAX_PLANES; planes++)
                {
                    videoFrames->bufAddr[planes] = frames->addr[0][planes];
                }

                status = Utils_quePut(&pOutObj->emptyBufQue[chId],
                               &pOutObj->buffers[chId][bufId], BSP_OSAL_NO_WAIT);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function delete the VPE link Output buffer free queue
 *
 *        Delete VPE links Output system buffer Queue
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   pObj     [IN] Output queue ID
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
static Int32 VpeLink_deleteVpeOutChBufferQueue (VpeLink_Obj * pObj, UInt32 outId)
{
    Int32 status;
    UInt32 chId;
    VpeLink_OutObj *pOutObj = &pObj->outObj[outId];

    for (chId=0; chId<pObj->inQueInfo.numCh; chId++)
    {
        if (pOutObj->numFrames[chId] > 0)
        {
            status = Utils_queDelete(&pOutObj->emptyBufQue[chId]);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function create the free FVID2_frame objects and the Queue
 *
 *        Creates the FVID2_frame object and placed in the Queue.
 *        While submitting the video frames to driver, De-queue
 *        one FVID2_frame, populate and queue to the Driver
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   chId     [IN] channel number
 *
 * \return  None
 *
 *******************************************************************************
 */
Void VpeLink_drvCreateFvidFrameQueue(VpeLink_Obj *pObj, Int32 chId)
{
    Int32 status;
    Int32 i;
    VpeLink_InObj *pChInObj;

    pChInObj = &pObj->chObj[chId].inObj;

    status = Utils_queCreate(&pChInObj->fvidFrameQueue,
                             UTILS_ARRAYSIZE(pChInObj->frames),
                             pChInObj->fvidFrameQueueMem,
                             UTILS_QUE_FLAG_NO_BLOCK_QUE);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    for (i = 0; i < UTILS_ARRAYSIZE(pChInObj->frames); i++)
    {
        status = Utils_quePut(&pChInObj->fvidFrameQueue,
                              &pChInObj->frames[i],
                              BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
}

/**
 *******************************************************************************
 *
 * \brief This function delete the free FVID2_frame objects and the Queue
 *
 *        Delete the FVID2_frame freeQ and frames placed in the Queue.
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   chId     [IN] channel number
 *
 * \return  None
 *
 *******************************************************************************
 */
static
Void VpeLink_drvDeleteFvidFrameQueue(VpeLink_Obj *pObj, Int32 chId)
{
    Int32 status;
    VpeLink_InObj *pChInObj;

    pChInObj = &pObj->chObj[chId].inObj;

    status = Utils_queDelete(&pChInObj->fvidFrameQueue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
}

/**
 *******************************************************************************
 *
 * \brief This function free-up the FVID2_frames after processing the same
 *
 *        Freeing of FVID2_frames is done once the driver return
 *        FIVID2_frame list after processing the frames.
 *        Pick the free frames and queued into the free frame Queue,
 *        this will be used again during the next FVID2_queue process
 *
 * \param   pObj          [IN] VPE Link Instance handle
 * \param   freeFrameList [IN] FIVID2_frame list after processing the frames
 *
 * \return  None
 *
 *******************************************************************************
 */
void VpeLink_drvFreeFvidFrames(VpeLink_Obj *pObj,
                               FVID2_FrameList *freeFrameList)
{
    Int32 idx, status;
    FVID2_Frame *pFrame;
    VpeLink_InObj *pChInObj;

    for (idx = 0; idx < freeFrameList->numFrames; idx++)
    {
        pFrame = freeFrameList->frames[idx];
        UTILS_assert (pFrame->chNum < pObj->inQueInfo.numCh);
        pChInObj = &pObj->chObj[pFrame->chNum].inObj;
        status = Utils_quePut(&pChInObj->fvidFrameQueue,
                              pFrame,
                              BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
}

/**
 *******************************************************************************
 *
 * \brief Function to create the VPE link output object
 *
 *        Create the VPE link Output object, one per output queue
 *        - Create the Output Buffer Queue
 *        - populates the output frame parameters
 *        - Allocate the memory for the output frame buffers
 *        - Creates the output FVID2 frames and populaces with frame pointers
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  status   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreateOutObj(VpeLink_Obj * pObj)
{
    VpeLink_OutObj *pOutObj;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 outId, chId, frameId, size, cOffset;
    FVID2_Format *pFormat;
    FVID2_Frame * pFrame;
    System_LinkChInfo *pInChInfo;
    System_LinkChInfo *pOutChInfo;
    VpeLink_ChannelParams *chParams;
    UInt8 *pBaseAddr;

    memset(&pObj->outFrameDrop, 0, sizeof(pObj->outFrameDrop));

    pObj->info.numQue = VPE_LINK_OUT_QUE_ID_MAX;

#ifndef VPE_INCLUDE
    UTILS_assert(pObj->createArgs.enableOut[VPE_LINK_OUT_QUE_ID_1] == FALSE);
#endif

    for (outId = 0u; outId < VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        pObj->info.queInfo[outId].numCh = 0;
        if (TRUE == pObj->createArgs.enableOut[outId])
        {
            pObj->info.queInfo[outId].numCh = pObj->inQueInfo.numCh;
        }
    }

    for (outId = 0u; outId < VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        if (pObj->createArgs.enableOut[outId])
        {
            pOutObj = &pObj->outObj[outId];

            status = Utils_bufCreate(&pOutObj->bufOutQue, TRUE, FALSE);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            memset(pOutObj->numFrames, 0, sizeof(pOutObj->numFrames));

            for (chId = 0u; chId < pObj->inQueInfo.numCh; chId++)
            {
                chParams = &pObj->createArgs.chParams[chId];
                if ((chParams->outParams[outId].numBufsPerCh <= 0) ||
                    (chParams->outParams[outId].numBufsPerCh >
                                                VPE_LINK_MAX_OUT_FRAMES_PER_CH))
                {
                    chParams->outParams[outId].numBufsPerCh =
                                               VPE_LINK_MAX_OUT_FRAMES_PER_CH;
                }
                UTILS_assert(chParams->outParams[outId].numBufsPerCh <=
                                       VPE_LINK_MAX_OUT_FRAMES_PER_CH);
                pOutObj->numFrames[chId] = chParams->outParams[outId].numBufsPerCh;

                pFormat = &pObj->chObj[chId].outFormat[outId];
                pFormat->chNum = chId;
                pFormat->pitch[2] = 0;
                pFormat->fieldMerged[0] = FALSE;
                pFormat->fieldMerged[1] = FALSE;
                pFormat->fieldMerged[2] = FALSE;
                pFormat->scanFormat = FVID2_SF_PROGRESSIVE;
                pFormat->bpp = FVID2_BPP_BITS16;
                pFormat->reserved = NULL;

                pInChInfo = &pObj->inQueInfo.chInfo[chId];

                pFormat->width = pInChInfo->width;
                if (SYSTEM_LINK_CH_INFO_GET_FLAG_SCAN_FORMAT(pInChInfo->flags) ==
                                                         FVID2_SF_INTERLACED &&
                    !pObj->createArgs.chParams[chId].deiCfg.bypass)
                {
                    pFormat->height = pInChInfo->height * 2u;
                }
                else
                {
                    pFormat->height = pInChInfo->height;
                }

                if (pObj->createArgs.chParams[chId].outParams[outId].height)
                    pFormat->height =
                      pObj->createArgs.chParams[chId].outParams[outId].height;
                if (pObj->createArgs.chParams[chId].outParams[outId].width)
                    pFormat->width =
                      pObj->createArgs.chParams[chId].outParams[outId].width;

                /*Width aligned to satisfy encoder requirement*/
                pFormat->width = SystemUtils_floor(pFormat->width, 16);
                pFormat->pitch[0] =
                    VpsUtils_align(pFormat->width, VPS_BUFFER_ALIGNMENT);

                switch (pObj->createArgs.chParams[chId].outParams[outId].dataFormat)
                {
                    case SYSTEM_DF_YUV422I_YUYV:
                        pFormat->dataFormat = FVID2_DF_YUV422I_YUYV;
                        pFormat->pitch[0] *= 2;
                        pFormat->bpp = FVID2_BPP_BITS16;
                        break;
                    case SYSTEM_DF_YUV420SP_UV:
                        pFormat->dataFormat = FVID2_DF_YUV420SP_UV;
                        pFormat->bpp = FVID2_BPP_BITS12;
                        break;
#ifdef VPE_INCLUDE
                    case SYSTEM_DF_YUV422SP_UV:
                        pFormat->dataFormat = FVID2_DF_YUV422SP_UV;
                        pFormat->bpp = FVID2_BPP_BITS16;
                        break;
                    case SYSTEM_DF_RGB24_888:
                        pFormat->dataFormat = FVID2_DF_RGB24_888;
                        pFormat->pitch[0] *= 3;
                        pFormat->bpp = FVID2_BPP_BITS24;
                        break;
                    case SYSTEM_DF_YUV444I:
                        pFormat->dataFormat = FVID2_DF_YUV444I;
                        pFormat->pitch[0] *= 3;
                        pFormat->bpp = FVID2_BPP_BITS24;
                        break;
#endif
                    default:
                        pFormat->dataFormat = FVID2_DF_YUV422I_YUYV;
                        pFormat->pitch[0] *= 2;
                        pFormat->bpp = FVID2_BPP_BITS16;
                        break;
                }

                pFormat->pitch[1] = pFormat->pitch[0];
                pFormat->pitch[2] = pFormat->pitch[0];

                pOutChInfo = &pObj->info.queInfo[outId].chInfo[chId];
                pOutChInfo->startX = 0;
                pOutChInfo->startY = 0;
                SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(pOutChInfo->flags,
                                                         pFormat->scanFormat);
                SYSTEM_LINK_CH_INFO_SET_FLAG_MEM_TYPE(pOutChInfo->flags,
                                                      VPS_VPDMA_MT_NONTILEDMEM);

                pOutChInfo->width = pFormat->width;
                pOutChInfo->height = pFormat->height;
                pOutChInfo->pitch[0] = pFormat->pitch[0];
                pOutChInfo->pitch[1] = pFormat->pitch[1];
                pOutChInfo->pitch[2] = pFormat->pitch[2];
                SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(pOutChInfo->flags,
                                                         pFormat->dataFormat);

                cOffset = 0;

                status = Utils_memFrameGetSize(
                                pFormat,
                                &size,
                                &cOffset,
                                0);
                UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

                pOutObj->bufSize[chId] = size * pOutObj->numFrames[chId];

                if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)
                    ==FALSE
                   )
                {
                    pOutObj->bufBaseAddr[chId] = Utils_memAlloc(
                                    UTILS_HEAPID_DDR_CACHED_SR,
                                    pOutObj->bufSize[chId],
                                    VPS_BUFFER_ALIGNMENT
                            );
                }
                else
                {
                    pOutObj->bufBaseAddr[chId] =
                             System_allocLinkMemAllocInfo(
                                    &pObj->createArgs.memAllocInfo,
                                    pOutObj->bufSize[chId],
                                    VPS_BUFFER_ALIGNMENT
                                );
                }

                UTILS_assert(pOutObj->bufBaseAddr[chId]!=NULL);

                pBaseAddr = pOutObj->bufBaseAddr[chId];

                for(frameId=0; frameId<pOutObj->numFrames[chId]; frameId++)
                {
                    pFrame = &pOutObj->frames[chId][frameId];

                    memset(pFrame, 0, sizeof(*pFrame));

                    pFrame->chNum = pFormat->chNum;
                    pFrame->addr[0][0] = pBaseAddr;
                    pFrame->addr[0][1] = pBaseAddr + cOffset;

                    pBaseAddr += size;

#ifdef SYSTEM_VERBOSE_PRINTS
                Vps_printf(" VPE: OUT%d: CH%2d: Frame %d: 0x%08x, 0x%08x, %d x %d\n",
                            outId,
                            chId,
                            frameId,
                            pOutObj->frames[chId][frameId].addr[0][0],
                            pOutObj->frames[chId][frameId].addr[0][1],
                            pOutChInfo->width,
                            pOutChInfo->height
                            );
#endif
                }
            }
            VpeLink_createVpeOutChBufferQueue(pObj, outId);
        }
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief VPE link create function
 *
 *        This Set the Link and driver create time parameters.
 *        - Get the channel info from previous link
 *        - create the semaphore required for VPE link
 *        - Set the internal data structures
 *        - Create the output object per output queue
 *        - Create the Link channel object per channel
 *        - Create the VPE driver instance
 *        - Allocate the DEI context buffers
 *        - Choose the scalar coefficients and configure the driver
 *        - Create the link intermediate request object
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   pPrm     [IN] VPE link create parameters
 *                        This need to be configured by the application
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreate(VpeLink_Obj * pObj, VpeLink_CreateParams * pPrm)
{
    UInt32 chId;
    Int32 status;

#ifdef SYSTEM_DEBUG_VPE
    Vps_printf(" VPE: Create in progress !!!\n");
#endif

    UTILS_MEMLOG_USED_START();
    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    System_resetLinkMemAllocInfo(&pObj->createArgs.memAllocInfo);

    status = System_linkGetInfo(pPrm->inQueParams.prevLinkId, &pObj->inTskInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    memcpy(&pObj->inQueInfo,
           &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId],
           sizeof(pObj->inQueInfo));
    UTILS_assert(pObj->inQueInfo.numCh <= VPE_LINK_MAX_CH);

    pObj->useOverridePrevFldBuf = FALSE;

    pObj->givenInFrames = 0x0;
    pObj->returnedInFrames = 0x0;
    pObj->loadUpsampleCoeffs = FALSE;
    pObj->isStopSupported = FALSE;

    /* Create semaphores */
    pObj->complete = BspOsal_semCreate(0u, TRUE);
    UTILS_assert(pObj->complete != NULL);

    VpeLink_drvCreateOutObj(pObj);

    for (chId = 0; chId < pObj->inQueInfo.numCh; chId++)
    {
        VpeLink_drvCreateChObj(pObj, chId);
    }
    VpeLink_drvCreateFvidObj(pObj);

#ifdef VPE_INCLUDE
    VpeLink_drvAllocCtxMem(pObj);
    VpeLink_drvSetScCoeffs(pObj);
#endif

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->linkId, "VPE");
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->isFirstFrameRecv = FALSE;

    System_assertLinkMemAllocOutOfMem(
        &pObj->createArgs.memAllocInfo,
        "VPE"
        );

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("VPE",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));

#ifdef SYSTEM_DEBUG_VPE
    Vps_printf(" VPE: Create Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function get/collect the input buffers from previous link
 *
 *        - Call System_getLinksFullBuffers to get the full frames from
 *          previous link
 *        - Put these input buffers into the intermediate channel specific Queue
 *        - Free any input buffers which does not follow the proper FID sequence
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvQueueFramesToChQue(VpeLink_Obj * pObj)
{
    UInt32 bufId, freeBufNum;
    FVID2_Frame *pFrame;
    System_LinkInQueParams *pInQueParams;
    System_BufferList bufList;
    VpeLink_ChObj *pChObj;
    Int32 status;
    System_Buffer *pBuffer;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    pInQueParams = &pObj->createArgs.inQueParams;

    System_getLinksFullBuffers(pInQueParams->prevLinkId,
                               pInQueParams->prevLinkQueId, &bufList);

    if (bufList.numBuf)
    {
        freeBufNum = 0;

        for (bufId = 0; bufId < bufList.numBuf; bufId++)
        {
            pBuffer = bufList.buffers[bufId];

            if (pBuffer->chNum >= pObj->inQueInfo.numCh)
            {
                bufList.buffers[freeBufNum] = pBuffer;
                freeBufNum++;
                linkStatsInfo->linkStats.inBufErrorCount++;
                continue;
            }
            pChObj = &pObj->chObj[pBuffer->chNum];
            pFrame = &pObj->inFrame;
            status = Utils_bufInitFrame(pFrame, pBuffer);

            linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufRecvCount++;

            /* in bypass mode only pick even fields */
            if(pObj->createArgs.chParams[pBuffer->chNum].deiCfg.bypass)
            {
                pChObj->nextFid = 0;
            }
            if (pChObj->nextFid == pFrame->fid)
            {
                // frame is of the expected FID use it, else drop it
                status = Utils_bufPutFullBuffer(&pChObj->inQue, pBuffer);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                // toggle to next required FID
                pChObj->nextFid ^= 1;
#ifdef VPE_INCLUDE
                pChObj->deiRtCfg.fldRepeat = FALSE;
#endif
            }
            else
            {
                linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;

                // frame is not of expected FID, so release frame
                bufList.buffers[freeBufNum] = pBuffer;
                freeBufNum++;
#ifdef VPE_INCLUDE
                pChObj->deiRtCfg.fldRepeat = TRUE;
#endif
            }
        }

        if (freeBufNum)
        {
            bufList.numBuf = freeBufNum;

            System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                        pInQueParams->prevLinkQueId, &bufList);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function set/initialize the RT parameters
 *
 *        Sets/store the current RT parameters in the VPE instance
 *        object. This function also update the scalar crop parameters.
 *        These will be used to compare and identify any run time
 *        VPE DEI or Scalar parameter change
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvUpdateInputRtPrm(VpeLink_Obj * pObj,
                                  FVID2_Frame *pInFrame, UInt32 chId)
{
    VpeLink_ChObj *pChObj;
    System_LinkChInfo *pInQueChInfo;
    System_Buffer *buffer;
    System_VideoFrameBuffer *videoFrame;
    UInt32 flags;

    pChObj = &pObj->chObj[chId];
    buffer = pInFrame->appData;
    UTILS_assert(buffer != NULL);

    videoFrame = buffer->payload;
    UTILS_assert(videoFrame != NULL);
    flags = videoFrame->chInfo.flags;

    /* Monitor input resolution and configure parameters when change happens */
    if (SYSTEM_LINK_CH_INFO_GET_FLAG_IS_RT_PRM_UPDATE(flags)
        == TRUE)
    {
        pInQueChInfo = &pObj->inQueInfo.chInfo[chId];

        if ((videoFrame->chInfo.width   != pInQueChInfo->width) ||
            (videoFrame->chInfo.height  != pInQueChInfo->height)||
            (SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(flags) !=
             SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(pInQueChInfo->flags)))
        {
            pInQueChInfo->width    = videoFrame->chInfo.width;
            pInQueChInfo->height   = videoFrame->chInfo.height;
            SYSTEM_LINK_CH_INFO_SET_FLAG_MEM_TYPE (pInQueChInfo->flags,
                           SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(flags));
            pInQueChInfo->pitch[0] = videoFrame->chInfo.pitch[0];
            pInQueChInfo->pitch[1] = videoFrame->chInfo.pitch[1];
            pInQueChInfo->pitch[2] = videoFrame->chInfo.pitch[2];

            pChObj->vpeInFrmPrms.width      = pInQueChInfo->width;
            pChObj->vpeInFrmPrms.height     = pInQueChInfo->height;
            pChObj->vpeInFrmPrms.memType    =
                    SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(pInQueChInfo->flags);
            pChObj->vpeInFrmPrms.pitch[0]   = pInQueChInfo->pitch[0];
            pChObj->vpeInFrmPrms.pitch[1]   = pInQueChInfo->pitch[1];
            pChObj->vpeInFrmPrms.pitch[2]   = pInQueChInfo->pitch[2];
            pChObj->vpeInFrmPrms.dataFormat =
                    SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(flags);

#ifdef VPE_INCLUDE
            pChObj->vpeRtPrm.inFrmPrms = &pChObj->vpeInFrmPrms;

            VpeLink_drvUpdateScCropPrm(pObj, chId, TRUE);

            pInFrame->perFrameCfg = &pChObj->vpeRtPrm;
#endif

#ifdef SYSTEM_DEBUG_VPE_RT
                Vps_printf(" VPE: CH%d: Input size is changed"
                           " to %d x %d !!!\n",
                           chId,
                           pInQueChInfo->width,
                           pInQueChInfo->height
                          );
#endif
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function free-up/send-out the VPE link input/output buffers
 *
 *        Function to
 *        - send-out the filled VPE link output buffers to the next Link
 *        - free-up the input buffers which are already processed
 *        - free the input FVID2 frame by putting them back to the
 *          input side Fvid2 frame freeQ
 *        - Send notification to the next link on the output buffer availability
 *
 * \param   pObj           [IN] VPE Link Instance handle
 * \param   inFrameList    [IN] Input frame List
 * \param   outFrameList   [IN] Output frame List
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvReleaseFrames(VpeLink_Obj * pObj,
                               FVID2_FrameList * inFrameList,
                               FVID2_FrameList * outFrameList)
{
    UInt32 frameId, outId, outputId, sendCmd[VPE_LINK_OUT_QUE_ID_MAX];
    FVID2_Frame *pFrame;
    System_LinkInQueParams *pInQueParams;
    Int32 status;
    Int32 actualFrameIdx;
    VpeLink_ChObj *pChObj;
    VpeLink_ReqObj *pReqObj;
    System_Buffer *pBuf;
    System_BufferList bufList;
    FVID2_FrameList freeFrameList;
    UInt32 *outQueIdArray;

    UTILS_assert(NULL != pObj->linkStatsInfo);

    pReqObj = &pObj->reqObj;

    pInQueParams = &pObj->createArgs.inQueParams;

    for (outputId = 0; outputId < VPE_LINK_OUT_QUE_ID_MAX; outputId++)
        sendCmd[outputId] = FALSE;

    for (frameId = 0; frameId < outFrameList->numFrames; frameId++)
    {
        outQueIdArray = outFrameList[0].appData;
        outId = outQueIdArray[frameId];
        UTILS_assert(outId < VPE_LINK_OUT_QUE_ID_MAX);
        if (pObj->createArgs.enableOut[outId])
        {
            pFrame = outFrameList->frames[frameId];
            if ((pFrame) && (pFrame != &pObj->outFrameDrop))
            {
                /* Fid should be always 0 after VPE explicitly setting
                   here to avoid drop in SWMS/display based on fid */
                pFrame->fid = 0;

                pChObj = &pObj->chObj[pFrame->chNum];

                pBuf = pFrame->appData;
                UTILS_assert(pBuf != NULL);

                Utils_updateLatency(&pObj->linkStatsInfo->linkLatency,
                                    pBuf->linkLocalTimestamp);
                Utils_updateLatency(&pObj->linkStatsInfo->srcToLinkLatency,
                                    pBuf->srcTimestamp);

                status =
                  Utils_bufPutFullBuffer(&pObj->outObj[outId].bufOutQue,
                                         pBuf);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                sendCmd[outId] = TRUE;
            }
        }
    }

    if (inFrameList->numFrames > 0x0)
    {
        actualFrameIdx = -1;

        if(pObj->useOverridePrevFldBuf)
        {
            for (frameId = 0x0; frameId < pReqObj->inFrameListN.numFrames; frameId++)
            {
                pFrame = pReqObj->inFrameListN.frames[frameId];

                /* in this mode inFrame will never be NULL */
                UTILS_assert(pFrame!=NULL);

                pChObj = &pObj->chObj[pFrame->chNum];

                if(pChObj->pInFrameN_2!=NULL)
                {
                    actualFrameIdx++;
                    pBuf = pChObj->pInFrameN_2->appData;
                    bufList.buffers[actualFrameIdx] =  pBuf;
                    freeFrameList.frames[actualFrameIdx] = pChObj->pInFrameN_2;
                }
                pChObj->pInFrameN_2 = pChObj->pInFrameN_1;
                pChObj->pInFrameN_1 = pFrame;
            }
        }
        else
        {
            /* There could be holes in the frame list, as the driver could
             * have decided to hold back couple of frames as context, ensure
             * the frame list is compacted */
            for (frameId = 0x0; frameId < inFrameList->numFrames; frameId++)
            {
                if (NULL != inFrameList->frames[frameId])
                {
                    actualFrameIdx++;
                    pBuf = inFrameList->frames[frameId]->appData;
                    bufList.buffers[actualFrameIdx] = pBuf;
                    freeFrameList.frames[actualFrameIdx] =
                                   inFrameList->frames[frameId];
                }
                else
                {
                    /* Do nothing */
                    continue;
                }
            }
        }

        if (actualFrameIdx != -1)
        {
            bufList.numBuf = actualFrameIdx + 0x01u;
            freeFrameList.numFrames = actualFrameIdx + 0x01u;

            pObj->returnedInFrames += inFrameList->numFrames;

            VpeLink_drvFreeFvidFrames(pObj, &freeFrameList);
            System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                        pInQueParams->prevLinkQueId,
                                        &bufList);
        }
    }

    for (outId = 0; outId < VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        if (sendCmd[outId])
        {
            System_sendLinkCmd(pObj->createArgs.outQueParams[outId].nextLink,
                               SYSTEM_CMD_NEW_DATA, NULL);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to stop the VPE link
 *
 *        - Invoke the VPS driver stop (FVID2_stop)
 *        - Call VpeLink_drvGetProcessedData if any request pending
 *        - Release context fields if any held inside the driver
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  rtnValue [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvStop(VpeLink_Obj * pObj)
{
    Int32 rtnValue = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_VPE
    Vps_printf(" VPE: Stop in progress !!!\n");
#endif

    if (TRUE == pObj->isStopSupported)
    {
        rtnValue = FVID2_stop(pObj->fvidHandle, NULL);
        if (rtnValue != SYSTEM_LINK_STATUS_SOK)
        {
#ifdef SYSTEM_DEBUG_VPE
            Vps_printf(" VPE: Stop ERROR !!!\n");
#endif
        }

        /* Even though all the requests are addressed, the driver would have held
         * back couple of input fields as context fields, get them */
        if ((pObj->returnedInFrames - pObj->givenInFrames) != 0x0)
        {
            VpeLink_drvReleaseContextField(pObj);
        }
    }

#ifdef SYSTEM_DEBUG_VPE
    Vps_printf(" VPE: Stop Done !!!\n");
#endif

    return (rtnValue);
}

/**
 *******************************************************************************
 *
 * \brief Function to Delete the VPE link
 *
 *        - Free-up the DEI context buffers
 *        - Delete the VPE driver (FVID2_delete)
 *        - Delete the Link "process complete" semaphore
 *        - Remove the output buffer queue
 *        - De-allocate the output video frame memory
 *        - Delete the output buffer empty buffer queue
 *        - Delete the FVID2_frame freeQ
 *        - Delete the Link intermediate channel specific input queue
 *        - Delete the link internal request object queue
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  rtnValue [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvDelete(VpeLink_Obj * pObj)
{
    UInt32 outId, chId;
    VpeLink_ChObj *pChObj;
    VpeLink_OutObj *pOutObj;
    Int32 status;

#ifdef SYSTEM_DEBUG_VPE
    Vps_printf(" VPE: Delete in progress !!!\n");
#endif

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status==0);

#ifdef VPE_INCLUDE
    VpeLink_drvFreeCtxMem(pObj);
#endif

    status = FVID2_delete(pObj->fvidHandle, NULL);
    UTILS_assert(SYSTEM_LINK_STATUS_SOK == status);

    /* Delete semaphores */
    BspOsal_semDelete(&pObj->complete);

    for (outId = 0; outId < VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        if (pObj->createArgs.enableOut[outId])
        {
            pOutObj = &pObj->outObj[outId];

            Utils_bufDelete(&pOutObj->bufOutQue);
            for (chId = 0; chId < pObj->inQueInfo.numCh; chId++)
            {
                if (pOutObj->numFrames[chId] > 0)
                {
                    if(System_useLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo)
                        == FALSE
                        )
                    {
                        status = Utils_memFree(
                            UTILS_HEAPID_DDR_CACHED_SR,
                            pOutObj->bufBaseAddr[chId],
                            pOutObj->bufSize[chId]);
                        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
                    }
                }
            }
            VpeLink_deleteVpeOutChBufferQueue(pObj, outId);
        }
    }

    for (chId = 0; chId < pObj->inQueInfo.numCh; chId++)
    {
        pChObj = &pObj->chObj[chId];

        VpeLink_drvDeleteFvidFrameQueue(pObj, chId);
        Utils_bufDelete(&pChObj->inQue);
    }

#ifdef SYSTEM_DEBUG_VPE
    Vps_printf(" VPE: Delete Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to set run time output frame rate
 *
 *        Set run time output frame rate, frame rate can be set for a
 *        - specific output queue
 *        - of a specific channel
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   VpeLink_ChFpsParams* [IN] VPE FPS parameters
 *
 * \return  status   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvSetFrameRate(VpeLink_Obj * pObj, VpeLink_ChFpsParams * params)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VpeLink_ChObj *pChObj;

    if (params->chId < pObj->inQueInfo.numCh)
    {
        pChObj = &pObj->chObj[params->chId];

        /*
         * Stream 0 maps to Queue 0 - VPE_LINK_OUT_QUE_ID_0
         * Stream 1 maps to Queue 1 - VPE_LINK_OUT_QUE_ID_1
         */

        Utils_resetSkipBufContext(
            &pChObj->frameSkipCtx[params->outQueId],
            params->inputFrameRate,
            params->outputFrameRate
            );
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Function to run time enable/disable a channel
 *
 *        Set run time enable/disable a channel of a specific output queue
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   VpeLink_ChannelInfo* [IN] VPE channel info parameters
 *
 * \return  status   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvSetChannelInfo(VpeLink_Obj * pObj,
                                VpeLink_ChannelInfo *channelInfo)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VpeLink_ChObj *pChObj;

    UTILS_assert(channelInfo->chId < pObj->inQueInfo.numCh);
    UTILS_assert(channelInfo->outQueId  < (VPE_LINK_OUT_QUE_ID_MAX));

    pChObj = &pObj->chObj[channelInfo->chId];
    pChObj->enableOut[channelInfo->outQueId] = channelInfo->enable;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Function to get the output resolution of a channel at run time
 *
 *        Get the current output resolution
 *        - of a specific channel
 *        - of a specific output queue
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   VpeLink_ChDynamicSetOutRes* [IN] VPE CH resolution parameters
 *
 * \return  status   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvGetChDynamicOutputRes(VpeLink_Obj * pObj,
                                       VpeLink_ChDynamicSetOutRes * params)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VpeLink_ChObj *pChObj;
    UInt32 outId, chId;

    if (params->chId < pObj->inQueInfo.numCh)
    {
        chId = params->chId;
        outId = params->outQueId;
        pChObj = &pObj->chObj[chId];
        params->width = pChObj->vpeRtOutFrmPrm[outId].width;
        params->height = pChObj->vpeRtOutFrmPrm[outId].height;
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Function to Set the output resolution of a channel at run time
 *
 *        Set the output resolution
 *        - of a specific channel
 *        - of a specific output queue
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   VpeLink_ChDynamicSetOutRes* [IN] VPE CH resolution parameters
 *
 * \return  status   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvSetChDynamicOutputRes(VpeLink_Obj * pObj,
                                       VpeLink_ChDynamicSetOutRes * params)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VpeLink_ChObj *pChObj;
    UInt32 outId, chId;

    if (params->chId < pObj->inQueInfo.numCh)
    {
        chId = params->chId;
        outId = params->outQueId;
        pChObj = &pObj->chObj[chId];

        pChObj->chRtOutInfoUpdate[outId] = TRUE;

        pChObj->vpeRtOutFrmPrm[outId].width    = params->width;
        pChObj->vpeRtOutFrmPrm[outId].height   = params->height;
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Function to prints the VPE link statistics
 *
 *        Also reset the Link statistics after the print controlled by a flag
 *
 * \param   pObj            [IN] Display Link Instance handle
 * \param   resetAfterPrint [IN] Flag to reset the statistics after print
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_printStatistics (VpeLink_Obj *pObj, Bool resetAfterPrint)
{
    UTILS_assert(NULL != pObj->linkStatsInfo);

    Utils_printLinkStatistics(&pObj->linkStatsInfo->linkStats, "VPE", TRUE);

    Utils_printLatency("VPE",
                       &pObj->linkStatsInfo->linkLatency,
                       &pObj->linkStatsInfo->srcToLinkLatency,
                        TRUE);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to release the context fields if any held inside the driver
 *
 *        Invoke the FVID2_getProcessedFrames() call in a loop, till the
 *        VPE driver free-up all the context fields, if any are held inside
 *        the driver after stopping (FVID2_stop) of the driver.
 *        Also free-up the fields by sending them back to the previous link
 *
 * \param   pObj     [IN] Display Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
static Int32 VpeLink_drvReleaseContextField(VpeLink_Obj * pObj)
{
    Int32 status;
    FVID2_ProcessList processList;
    FVID2_FrameList outFrameList;

    outFrameList.numFrames = 0x0u;

 RETRY:
    status =
        FVID2_getProcessedFrames(pObj->fvidHandle, &processList, BSP_OSAL_NO_WAIT);
    if (status == FVID2_ENO_MORE_BUFFERS)
        return SYSTEM_LINK_STATUS_SOK;
    if (status == FVID2_EAGAIN)
        goto RETRY;

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(processList.numOutLists == 0x0u);

    VpeLink_drvReleaseFrames(pObj, processList.inFrameList[0], &outFrameList);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to prints the VPE link output buffer status
 *
 * \param   pObj            [IN] Display Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_printBufferStatus(VpeLink_Obj * pObj)
{
    Int32 i;
    UInt8 str[32];

    for (i=0; i<VPE_LINK_OUT_QUE_ID_MAX; i++)
    {
        sprintf ((char *)str, "VPE OUT%d", i);
        Utils_bufPrintStatus(str, &pObj->outObj[i].bufOutQue);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */

