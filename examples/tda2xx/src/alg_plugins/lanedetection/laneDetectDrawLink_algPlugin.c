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
 * \file laneDetectDrawLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for drawing arrows
 *         corresponding to the metadat produced by LANE DETECT algorithm
 *          - Takes the composite buffer from the previous link. The composite
 *            buffer has two channels, one containing the original video
 *            and the other containing metadata
 *          - Scales co-ordinates as per the requirement
 *          - Optionally copies the input video into an output buffer
 *            using EDMA
 *          - Draws arrows on the video and sends to the next link
 *
 * \version 0.0 (May 2014) : [NN] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "laneDetectDrawLink_priv.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of gAlign algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_laneDetectDraw_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_laneDetectDrawCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_laneDetectDrawProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_laneDetectDrawControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_laneDetectDrawStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_laneDetectDrawDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_LANE_DETECT_DRAW;
#endif

#ifdef BUILD_A15
    algId = ALGORITHM_LINK_A15_ALG_LANE_DETECT_DRAW;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for algorithm draw alg link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_laneDetectDrawCreate(void * pObj,void * pCreateParams)
{
    UInt32 numInputQUsed;
    UInt32 numOutputQUsed;
    UInt32 numChannelsUsed;
    UInt32 channelId;
    UInt32 status;
    UInt32 bufId;
    UInt32 outBufferSize_y, outBufferSize_uv;
    UInt32 prevLinkQId;
    Draw2D_BufInfo bufInfo;
    Utils_DmaChCreateParams dmaParams;
    AlgorithmLink_LaneDetectDrawObj          * pLaneDetectDrawObj;
    AlgorithmLink_LaneDetectDrawCreateParams * pLinkCreateParams;
    AlgorithmLink_OutputQueueInfo                   * pOutputQInfo;
    AlgorithmLink_InputQueueInfo                    * pInputQInfo;
    System_LinkInfo                                   prevLinkInfo;
    System_LinkChInfo                               * pOutChInfo;
    System_LinkChInfo                               * pPrevLinkChInfo;
    System_Buffer                                   * pSystemBuffer;
    System_VideoFrameBuffer                         * pVideoBuffer;

    pLaneDetectDrawObj = (AlgorithmLink_LaneDetectDrawObj *)
                        Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_LOCAL,
                            sizeof(AlgorithmLink_LaneDetectDrawObj), 128);
    UTILS_assert(pLaneDetectDrawObj != NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pLaneDetectDrawObj);

    pLinkCreateParams = (AlgorithmLink_LaneDetectDrawCreateParams *)
                         pCreateParams;
    pInputQInfo       = &pLaneDetectDrawObj->inputQInfo;
    pOutputQInfo      = &pLaneDetectDrawObj->outputQInfo;
    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy(
            (void*)(&pLaneDetectDrawObj->algLinkCreateParams),
            (void*)(pLinkCreateParams),
            sizeof(AlgorithmLink_LaneDetectDrawCreateParams)
           );

    pLinkCreateParams = &pLaneDetectDrawObj->algLinkCreateParams;

    /*
     * Populating parameters corresponding to Q usage of geometric alignment
     * algorithm link
     */
    numInputQUsed               = 1;
    numOutputQUsed              = 1;
    numChannelsUsed             = 1;
    channelId                   = 0;

    pInputQInfo->qMode          = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo->qMode         = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    pOutputQInfo->queInfo.numCh = numChannelsUsed;

    status = System_linkGetInfo(
                                  pLinkCreateParams->inQueParams.prevLinkId,
                                  &prevLinkInfo
                                );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    UTILS_assert(prevLinkInfo.numQue >= numInputQUsed);

    channelId           = 0;
    pOutChInfo          = &pOutputQInfo->queInfo.chInfo[channelId];
    prevLinkQId         = pLinkCreateParams->inQueParams.prevLinkQueId;
    pPrevLinkChInfo     = &prevLinkInfo.queInfo[prevLinkQId].chInfo[channelId];

    /*
     * Copy previous link channel information as current link's out
     * channel information
     */
    memcpy(pOutChInfo, pPrevLinkChInfo, sizeof(System_LinkChInfo));

    /* override to take ROI into effect */
    pOutChInfo->width  = pLinkCreateParams->imgFrameWidth;
    pOutChInfo->height = pLinkCreateParams->imgFrameHeight;
    pOutChInfo->startX = 0;
    pOutChInfo->startY = 0;

    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(
                                    pObj,
                                    numInputQUsed,
                                    pInputQInfo,
                                    numOutputQUsed,
                                    pOutputQInfo
                               );

    /*
     * Create DMA channel
     */
    Utils_DmaChCreateParams_Init(&dmaParams);

    status = Utils_dmaCreateCh(
                                &pLaneDetectDrawObj->copyFramesDmaObj,
                                &dmaParams
                               );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /*
     * Algorithm creation happens here
     * - Population of create time parameters
     * - Query for number of memory records needed
     * - Query for the size of each algorithm internal objects
     * - Actual memory allocation for internal alg objects
     */

    /*
     * Allocate memory for the output buffers and link metadata buffer with
     * system Buffer
     */
    outBufferSize_y = pOutChInfo->pitch[0]
                      *
                      pOutChInfo->height;

    outBufferSize_uv = (pOutChInfo->pitch[1] * pOutChInfo->height)/2;

    if(pLinkCreateParams->numOutBuffers>LANEDETECTDRAW_LINK_MAX_NUM_OUTPUT)
        pLinkCreateParams->numOutBuffers = LANEDETECTDRAW_LINK_MAX_NUM_OUTPUT;

    for (bufId = 0; bufId < pLinkCreateParams->numOutBuffers; bufId++)
    {
        pSystemBuffer       =  &pLaneDetectDrawObj->buffers[bufId];
        pVideoBuffer        =  &pLaneDetectDrawObj->videoBuffers[bufId];

        pSystemBuffer->bufType      = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
        pSystemBuffer->chNum        = 0;
        pSystemBuffer->payload      = pVideoBuffer;
        pSystemBuffer->payloadSize  = sizeof(System_VideoFrameBuffer);

        pVideoBuffer->bufAddr[0]    = Utils_memAlloc
                                      (
                                        UTILS_HEAPID_DDR_CACHED_SR,
                                        outBufferSize_y,
                                        128
                                      );
        pVideoBuffer->bufAddr[1]    = Utils_memAlloc
                                      (
                                        UTILS_HEAPID_DDR_CACHED_SR,
                                        outBufferSize_uv,
                                        128
                                      );
        memset(pVideoBuffer->bufAddr[1], 0x80, outBufferSize_uv);
        memcpy(
                &pVideoBuffer->chInfo,
                pPrevLinkChInfo,
                sizeof(System_LinkChInfo)
              );
       pVideoBuffer->flags = pPrevLinkChInfo->flags;
       AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSystemBuffer);
    }

    status = Draw2D_create(&pLaneDetectDrawObj->draw2DHndl);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    bufInfo.bufWidth    = pOutChInfo->width;
    bufInfo.bufHeight   = pOutChInfo->height;
    bufInfo.bufPitch[0] = pOutChInfo->pitch[0];
    bufInfo.bufPitch[1] = pOutChInfo->pitch[1];
    bufInfo.dataFormat  = SYSTEM_DF_YUV420SP_UV;
    bufInfo.transperentColor = DRAW2D_TRANSPARENT_COLOR;
    bufInfo.transperentColorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    bufInfo.bufAddr[0] = 0xFFFF; /* init with dummy address, this gets updated later */
    bufInfo.bufAddr[1] = 0xFFFF; /* init with dummy address, this gets updated later */

    status = Draw2D_setBufInfo(pLaneDetectDrawObj->draw2DHndl, &bufInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);


    pLaneDetectDrawObj->isFirstFrameRecv    = FALSE;

    /* Assign pointer to link stats object */
    pLaneDetectDrawObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_LANE_DETECT_DRAW");
    UTILS_assert(NULL != pLaneDetectDrawObj->linkStatsInfo);

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 AlgorithmLink_laneDetectDrawCopyInput(
                                        AlgorithmLink_LaneDetectDrawObj *pObj,
                                        System_VideoFrameCompositeBuffer *pCompBuffer,
                                        System_Buffer *pOutBuffer
                                                      )
{
    System_VideoFrameBuffer *pVidFrame = (System_VideoFrameBuffer *)pOutBuffer->payload;
    Utils_DmaCopyFill2D dmaPrm;
    Int32 status;


    dmaPrm.dataFormat   = SYSTEM_DF_YUV420SP_UV;
    dmaPrm.destAddr[0]  = (Ptr) pVidFrame->bufAddr[0];
    dmaPrm.destPitch[0] = pObj->outputQInfo.queInfo.chInfo[0].pitch[0];
    dmaPrm.destAddr[1]  = (Ptr) pVidFrame->bufAddr[1];
    dmaPrm.destPitch[1] = pObj->outputQInfo.queInfo.chInfo[0].pitch[1];
    dmaPrm.destStartX   = pObj->outputQInfo.queInfo.chInfo[0].startX;
    dmaPrm.destStartY   = pObj->outputQInfo.queInfo.chInfo[0].startY;
    dmaPrm.width        = pObj->outputQInfo.queInfo.chInfo[0].width;
    dmaPrm.height       = pObj->outputQInfo.queInfo.chInfo[0].height;
    dmaPrm.srcAddr[0]   = (Ptr) pCompBuffer->bufAddr[0][0];
    dmaPrm.srcPitch[0]  = pObj->outputQInfo.queInfo.chInfo[0].pitch[0];
    dmaPrm.srcAddr[1]   = (Ptr) pCompBuffer->bufAddr[1][0];
    dmaPrm.srcPitch[1]  = pObj->outputQInfo.queInfo.chInfo[0].pitch[1];
    dmaPrm.srcStartX    = pObj->algLinkCreateParams.imgFrameStartX;
    dmaPrm.srcStartY    = pObj->algLinkCreateParams.imgFrameStartY;

    status = Utils_dmaCopy2D(&pObj->copyFramesDmaObj,
                             &dmaPrm,
                             1);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

Void AlgorithmLink_laneDetectDrawLane(
                                  AlgorithmLink_LaneDetectDrawObj *pObj,
                                  UInt32 numLanePoints,
                                  UInt32 laneCrossInfo,
                                  LD_TI_output * pLanePoints,
                                  Bool leftLane
                               )
{
    UInt32 i;
    Draw2D_LinePrm linePrm;
    LD_TI_output *pPoint;

    linePrm.lineSize = pObj->algLinkCreateParams.laneThickness;
    linePrm.lineColor = COLOR_GREEN;
    linePrm.lineColorFormat = SYSTEM_DF_YUV420SP_UV;

    if(laneCrossInfo == LD_TI_LEFT_LANE_CROSS || laneCrossInfo == LD_TI_RIGHT_LANE_CROSS)
    {
        linePrm.lineColor = COLOR_RED;
    }

    if (pObj->algLinkCreateParams.enableDrawLines == TRUE)
    {
        if(numLanePoints)
        {
            Int32 maxX, maxY, minX, minY;

            maxX = maxY = 0;
            minX = minY = 0xFFFF;

            for(i=0; i<numLanePoints; i++)
            {
                pPoint = &pLanePoints[i];

                if(pPoint->x < minX)
                    minX = pPoint->x;

                if(pPoint->x > maxX)
                    maxX = pPoint->x;

                if(pPoint->y < minY)
                    minY = pPoint->y;

                if(pPoint->y > maxY)
                    maxY = pPoint->y;
            }

            if(leftLane)
            {
                Draw2D_drawLine(pObj->draw2DHndl,
                    minX,
                    maxY,
                    maxX,
                    minY,
                    &linePrm
                    );
            }
            else
            {
                Draw2D_drawLine(pObj->draw2DHndl,
                    minX,
                    minY,
                    maxX,
                    maxY,
                    &linePrm
                    );
            }
        }
    }
    else
    {
        for(i=0; i<numLanePoints; i++)
        {
            pPoint = &pLanePoints[i];

            Draw2D_drawPixel(
                pObj->draw2DHndl,
                pPoint->x,
                pPoint->y,
                linePrm.lineColor,
                linePrm.lineColorFormat
            );
        }
    }
}

Int32 AlgorithmLink_laneDetectDrawLaneInfo(
                                  AlgorithmLink_LaneDetectDrawObj *pObj,
                                  AlgorithmLink_LaneDetectOutput * pLaneOutput,
                                  System_VideoFrameBuffer *pVideoBuffer
                               )
{
    UInt32 numLanePoints;
    UInt32 bufAddr[SYSTEM_MAX_PLANES];

    bufAddr[0] = (UInt32)pVideoBuffer->bufAddr[0];
    bufAddr[1] = (UInt32)pVideoBuffer->bufAddr[1];
    Draw2D_updateBufAddr(pObj->draw2DHndl, bufAddr);

    numLanePoints = pLaneOutput->numLeftLanePoints + pLaneOutput->numRightLanePoints;

    if(numLanePoints>LD_TI_MAXLANEPOINTS)
    {
        /* assume info is junk in this case and dont draw anything */
        numLanePoints = 0;
    }

    if(numLanePoints)
    {
        AlgorithmLink_laneDetectDrawLane(pObj,
                        pLaneOutput->numLeftLanePoints,
                        pLaneOutput->laneCrossInfo,
                        &pLaneOutput->laneInfo[0],
                        TRUE
                    );

        AlgorithmLink_laneDetectDrawLane(pObj,
                        pLaneOutput->numRightLanePoints,
                        pLaneOutput->laneCrossInfo,
                        &pLaneOutput->laneInfo[pLaneOutput->numLeftLanePoints],
                        FALSE
                    );
    }

    return SYSTEM_LINK_STATUS_SOK;
}


Int32 AlgorithmLink_laneDetectDrawProcess(void * pObj)
{
    UInt32 bufId;
    Int32  status = SYSTEM_LINK_STATUS_SOK;
    Bool   bufDropFlag;
    AlgorithmLink_LaneDetectDrawObj          * pLaneDetectDrawObj;
    AlgorithmLink_LaneDetectDrawCreateParams * pLinkCreateParams;
    System_VideoFrameBuffer          *pVideoBuffer;
    System_LinkChInfo                *outChInfo;
    System_Buffer                    *pSysOutBuffer;
    System_Buffer                    *pSysInBuffer;
    System_VideoFrameCompositeBuffer *pCompositeBuffer;
    System_BufferList                 inputBufList;
    System_BufferList                 outputBufListReturn;
    System_BufferList                 inputBufListReturn;
    System_LinkStatistics *linkStatsInfo;

    pLaneDetectDrawObj = (AlgorithmLink_LaneDetectDrawObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    pLinkCreateParams = &pLaneDetectDrawObj->algLinkCreateParams;

    linkStatsInfo = pLaneDetectDrawObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    if (pLaneDetectDrawObj->isFirstFrameRecv == FALSE)
    {
        pLaneDetectDrawObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(&linkStatsInfo->linkStats, 1, 1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }

    linkStatsInfo->linkStats.newDataCmdCount++;

    System_getLinksFullBuffers(
                        pLinkCreateParams->inQueParams.prevLinkId,
                        pLinkCreateParams->inQueParams.prevLinkQueId,
                        &inputBufList);

    if (inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysInBuffer = inputBufList.buffers[bufId];
            if(pSysInBuffer == NULL)
            {
                linkStatsInfo->linkStats.inBufErrorCount++;
                continue;
            }
            linkStatsInfo->linkStats.chStats[0].inBufRecvCount++;

            UTILS_assert(pSysInBuffer->bufType == SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER);
            /*
             * The input buffer is a composite buffer consisting of two
             * channels. Channel 0 consists of video frame buffer while
             * channel1 consists of meta data.
             */
            pCompositeBuffer =
                   (System_VideoFrameCompositeBuffer *) pSysInBuffer->payload;


            UTILS_assert(pCompositeBuffer->numFrames == 2);

            status = AlgorithmLink_getEmptyOutputBuffer(
                                                        pObj,
                                                        0,
                                                        0,
                                                        &pSysOutBuffer
                                                        );
            if(status != SYSTEM_LINK_STATUS_SOK)
            {
                linkStatsInfo->linkStats.chStats
                            [0].inBufDropCount++;
                linkStatsInfo->linkStats.chStats
                            [0].outBufDropCount[0]++;
            }
            else
            {
                status = AlgorithmLink_laneDetectDrawCopyInput(
                                                        pLaneDetectDrawObj,
                                                        pCompositeBuffer,
                                                        pSysOutBuffer
                                                      );
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                pSysOutBuffer->srcTimestamp = pSysInBuffer->srcTimestamp;
                pSysOutBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

                outChInfo = &pLaneDetectDrawObj->outputQInfo.queInfo.chInfo[0];

                pVideoBuffer    = (System_VideoFrameBuffer *)pSysOutBuffer->payload;

                Cache_inv(
                           pCompositeBuffer->bufAddr[0][1],
                           LANEDETECT_LINK_LANE_POINTS_BUF_SIZE,
                           Cache_Type_ALLD,
                           TRUE
                         );

                Cache_inv(
                          pVideoBuffer->bufAddr[0],
                          outChInfo->pitch[0]*outChInfo->height,
                          Cache_Type_ALLD,
                          TRUE
                        );
                Cache_inv(
                          pVideoBuffer->bufAddr[1],
                          outChInfo->pitch[1]*outChInfo->height/2,
                          Cache_Type_ALLD,
                          TRUE
                        );

                AlgorithmLink_laneDetectDrawLaneInfo(
                                  pLaneDetectDrawObj,
                                  pCompositeBuffer->bufAddr[0][1],
                                  pVideoBuffer
                               );

                Cache_wb(
                          pVideoBuffer->bufAddr[0],
                          outChInfo->pitch[0]*outChInfo->height,
                          Cache_Type_ALLD,
                          TRUE
                        );
                Cache_wb(
                          pVideoBuffer->bufAddr[1],
                          outChInfo->pitch[1]*outChInfo->height/2,
                          Cache_Type_ALLD,
                          TRUE
                        );

                Utils_updateLatency(&linkStatsInfo->linkLatency,
                                    pSysOutBuffer->linkLocalTimestamp);
                Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                    pSysOutBuffer->srcTimestamp);

                linkStatsInfo->linkStats.chStats[0].inBufProcessCount++;
                linkStatsInfo->linkStats.chStats[0].outBufCount[0]++;

                AlgorithmLink_putFullOutputBuffer(
                                                    pObj,
                                                    0,
                                                    pSysOutBuffer
                                                 );
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                /*
                 * Informing next link that a new data has peen put for its
                 * processing
                 */
                System_sendLinkCmd(
                                    pLinkCreateParams->outQueParams.nextLink,
                                    SYSTEM_CMD_NEW_DATA,
                                    NULL

                                  );
                outputBufListReturn.numBuf = 1;
                outputBufListReturn.buffers[0] = pSysOutBuffer;

                AlgorithmLink_releaseOutputBuffer(
                                                  pObj,
                                                  0,
                                                  &outputBufListReturn
                                                 );


            }

            inputBufListReturn.numBuf = 1;
            inputBufListReturn.buffers[0] = pSysInBuffer;
            bufDropFlag = FALSE;
            AlgorithmLink_releaseInputBuffer(
                                  pObj,
                                  0,
                                  pLinkCreateParams->inQueParams.prevLinkId,
                                  pLinkCreateParams->inQueParams.prevLinkQueId,
                                  &inputBufListReturn,
                                  &bufDropFlag
                                 );
        }
    }
    return status;
}

Int32 AlgorithmLink_laneDetectDrawControl(void * pObj,
                                               void * pControlParams)
{
    AlgorithmLink_LaneDetectDrawObj* pLaneDetectDrawObj;
    AlgorithmLink_ControlParams         * pAlgLinkControlPrm;

    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pLaneDetectDrawObj = (AlgorithmLink_LaneDetectDrawObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    pAlgLinkControlPrm = (AlgorithmLink_ControlParams *)pControlParams;

    /*
     * There can be other commands to alter the properties of the alg link
     * or properties of the core algorithm.
     * In this simple example, there is just a control command to print
     * statistics and a default call to algorithm control.
     */
    switch(pAlgLinkControlPrm->controlCmd)
    {

        case SYSTEM_CMD_PRINT_STATISTICS:
            AlgorithmLink_laneDetectDrawPrintStatistics(
                                                             pObj,
                                                             pLaneDetectDrawObj
                                                            );
            break;

        default:
            break;
    }

    return status;
}

Int32 AlgorithmLink_laneDetectDrawStop(void * pObj)
{
    return 0;
}

Int32 AlgorithmLink_laneDetectDrawDelete(void * pObj)
{
    UInt32 status;
    UInt32 bufId;
    UInt32 outBufferSize_y, outBufferSize_uv;
    AlgorithmLink_LaneDetectDrawObj                   * pLaneDetectDrawObj;
    AlgorithmLink_LaneDetectDrawCreateParams    * pLinkCreateParams;
    System_VideoFrameBuffer                     * pVideoBuffer;

    pLaneDetectDrawObj = (AlgorithmLink_LaneDetectDrawObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    pLinkCreateParams = &pLaneDetectDrawObj->algLinkCreateParams;

    outBufferSize_y = pLaneDetectDrawObj->outputQInfo.queInfo.chInfo[0].pitch[0]
                      *
                      pLaneDetectDrawObj->outputQInfo.queInfo.chInfo[0].height;

    outBufferSize_uv = (pLaneDetectDrawObj->outputQInfo.queInfo.chInfo[0].pitch[1]
                        *
                        pLaneDetectDrawObj->outputQInfo.queInfo.chInfo[0].height)
                         / 2;


    status = Utils_linkStatsCollectorDeAllocInst(pLaneDetectDrawObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /*
     * Free link buffers
     */
    for (bufId = 0; bufId < pLinkCreateParams->numOutBuffers; bufId++)
    {
        pVideoBuffer  =   &pLaneDetectDrawObj->videoBuffers[bufId];

        status = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pVideoBuffer->bufAddr[0],
                                outBufferSize_y
                               );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        status = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pVideoBuffer->bufAddr[1],
                                outBufferSize_uv
                               );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    Utils_dmaDeleteCh(&pLaneDetectDrawObj->copyFramesDmaObj);

    status = Draw2D_delete(pLaneDetectDrawObj->draw2DHndl);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Utils_memFree(
                       UTILS_HEAPID_DDR_CACHED_LOCAL,
                       pLaneDetectDrawObj,
                       sizeof(AlgorithmLink_LaneDetectDrawObj)
                    );

    return status;
}

Int32 AlgorithmLink_laneDetectDrawPrintStatistics(void *pObj,
                AlgorithmLink_LaneDetectDrawObj *pLaneDetectDrawObj)
{
    UTILS_assert(NULL != pLaneDetectDrawObj->linkStatsInfo);

    Utils_printLinkStatistics(&pLaneDetectDrawObj->linkStatsInfo->linkStats,
                            "ALG_LANEDETECTDRAW_DRAW",
                            TRUE);

    Utils_printLatency("ALG_LANEDETECTDRAW_DRAW",
                       &pLaneDetectDrawObj->linkStatsInfo->linkLatency,
                       &pLaneDetectDrawObj->linkStatsInfo->srcToLinkLatency,
                        TRUE);

    return SYSTEM_LINK_STATUS_SOK;
}

