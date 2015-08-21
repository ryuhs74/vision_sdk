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
 * \file objectDrawLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for rectangle drawing over
 *         a object. This plugin does the following
 *          - Takes the composite buffer from the previous link. The composite
 *            buffer has two channels, one containing the original video
 *            and the other containing metadata (rectangles to be drawn)
 *          - Scales co-ordinates as per the requirement
 *          - Optionally copies the input video into an output buffer
 *            using EDMA
 *          - Draws rectangles on the video and sends to the next link
 *
 * \version 0.0 (Mar 2014) : [NN] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "objectDrawLink_priv.h"
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
Int32 AlgorithmLink_objectDraw_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_objectDrawCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_objectDrawProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_objectDrawControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_objectDrawStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_objectDrawDelete;

#ifdef BUILD_M4
    algId = ALGORITHM_LINK_IPU_ALG_PD_DRAW;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for object draw alg link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_objectDrawCreate(void * pObj,void * pCreateParams)
{
    UInt32 numInputQUsed;
    UInt32 numOutputQUsed;
    UInt32 numChannelsUsed;
    UInt32 channelId;
    UInt32 status;
    UInt32 bufId;
    UInt32 prevLinkQId;
    Utils_DmaChCreateParams dmaParams;
    AlgorithmLink_ObjectDrawObj                         * pObjectDrawObj;
    AlgorithmLink_ObjectDrawCreateParams                * pLinkCreateParams;
    AlgorithmLink_OutputQueueInfo                           * pOutputQInfo;
    AlgorithmLink_InputQueueInfo                            * pInputQInfo;
    System_LinkInfo                                           prevLinkInfo;
    System_LinkChInfo                                       * pOutChInfo;
    System_LinkChInfo                                       * pPrevLinkChInfo;
    System_Buffer                                           * pSystemBuffer;
    System_VideoFrameBuffer                                 * pVideoBuffer;


    pObjectDrawObj = (AlgorithmLink_ObjectDrawObj *)
                    Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_LOCAL,
                        sizeof(AlgorithmLink_ObjectDrawObj), 32);
    UTILS_assert(pObjectDrawObj != NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pObjectDrawObj);

    pLinkCreateParams = (AlgorithmLink_ObjectDrawCreateParams *)
                         pCreateParams;
    pInputQInfo       = &pObjectDrawObj->inputQInfo;
    pOutputQInfo      = &pObjectDrawObj->outputQInfo;
    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy(
            (void*)(&pObjectDrawObj->algLinkCreateParams),
            (void*)(pLinkCreateParams),
            sizeof(AlgorithmLink_ObjectDrawCreateParams)
           );
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
    pOutChInfo->width = pLinkCreateParams->imgFrameWidth;
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

    status = Draw2D_create(&pObjectDrawObj->draw2DHndl);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* Dummy address value, that is overridden later */
    pObjectDrawObj->draw2DBufInfo.bufAddr[0]  = 0xFFFF;
    pObjectDrawObj->draw2DBufInfo.bufAddr[1]  = 0xFFFF;

    pObjectDrawObj->draw2DBufInfo.bufWidth    = pOutChInfo->width;
    pObjectDrawObj->draw2DBufInfo.bufHeight   = pOutChInfo->height;
    pObjectDrawObj->draw2DBufInfo.bufPitch[0] = pOutChInfo->pitch[0];
    pObjectDrawObj->draw2DBufInfo.bufPitch[1] = pOutChInfo->pitch[1];
    pObjectDrawObj->draw2DBufInfo.dataFormat  = SYSTEM_DF_YUV420SP_UV;
    pObjectDrawObj->draw2DBufInfo.transperentColor = DRAW2D_TRANSPARENT_COLOR;
    pObjectDrawObj->draw2DBufInfo.transperentColorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;

    status = Draw2D_setBufInfo(pObjectDrawObj->draw2DHndl,
                                &pObjectDrawObj->draw2DBufInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);


    /*
     * Create DMA channel
     */
    Utils_DmaChCreateParams_Init(&dmaParams);

    status = Utils_dmaCreateCh(
                                &pObjectDrawObj->copyFramesDmaObj,
                                &dmaParams
                               );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);



    /* if thickness value is outside a range, set it to default */
    if(    pObjectDrawObj->algLinkCreateParams.pdRectThickness >  8
        || pObjectDrawObj->algLinkCreateParams.pdRectThickness == 0
        )
    {
        pObjectDrawObj->algLinkCreateParams.pdRectThickness
            = PD_RECTANGLE_THICKNESS;
    }

    pObjectDrawObj->linePrm.lineSize
        = pObjectDrawObj->algLinkCreateParams.pdRectThickness;

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
    pObjectDrawObj->outBufferSize_y = pOutChInfo->pitch[0]
                      *
                      pOutChInfo->height;

    pObjectDrawObj->outBufferSize_uv = (pOutChInfo->pitch[1]
                      *
                      pOutChInfo->height)/2;

    for (bufId = 0; bufId < pLinkCreateParams->numOutBuffers; bufId++)
    {
        pSystemBuffer       =  &pObjectDrawObj->buffers[bufId];
        pVideoBuffer        =  &pObjectDrawObj->videoBuffers[bufId];

        pSystemBuffer->bufType      = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
        pSystemBuffer->chNum        = 0;
        pSystemBuffer->payload      = pVideoBuffer;
        pSystemBuffer->payloadSize  = sizeof(System_VideoFrameBuffer);

        pVideoBuffer->bufAddr[0]    = Utils_memAlloc
                                      (
                                        UTILS_HEAPID_DDR_CACHED_SR,
                                        pObjectDrawObj->outBufferSize_y,
                                        ALGORITHMLINK_FRAME_ALIGN
                                      );
        pVideoBuffer->bufAddr[1]    = Utils_memAlloc
                                      (
                                        UTILS_HEAPID_DDR_CACHED_SR,
                                        pObjectDrawObj->outBufferSize_uv,
                                        ALGORITHMLINK_FRAME_ALIGN
                                      );
        memcpy(
                &pVideoBuffer->chInfo,
                pPrevLinkChInfo,
                sizeof(System_LinkChInfo)
              );
       pVideoBuffer->flags = pPrevLinkChInfo->flags;
       AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSystemBuffer);
    }

    pObjectDrawObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
       AlgorithmLink_getLinkId(pObj), " ALG_OBJECT_DRAW");
    UTILS_assert(NULL != pObjectDrawObj->linkStatsInfo);

    pObjectDrawObj->isFirstFrameRecv    = FALSE;

    return SYSTEM_LINK_STATUS_SOK;
}


Int32 AlgorithmLink_objectDrawRectangles(
                                    AlgorithmLink_ObjectDrawObj *pObj,
                                    Void *lumaAddr,
                                    Void *chromaAddr,
                                    Void *metaBufAddr
                                    )
{
    UInt32 numPds, i;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 bufAddr[SYSTEM_MAX_PLANES];
    TI_OD_output *pObjectDataOutput;
    TI_OD_objectDescriptor objectDataDesc;
    Bool isValidObj;

    pObjectDataOutput = (TI_OD_output*)metaBufAddr;

    numPds = pObjectDataOutput->numObjects;

    if (numPds > 0 && numPds < TI_OD_MAX_DETECTIONS_PER_OBJECT)
    {
        for (i = 0; i < numPds; i++)
        {
            objectDataDesc = pObjectDataOutput->objDesc[i];

            // Check the drawing option first.
            if (pObj->algLinkCreateParams.drawOption != ALGORITHM_LINK_OBJECT_DETECT_DRAW_PD_TSR)
            {
                if ((pObj->algLinkCreateParams.drawOption == ALGORITHM_LINK_OBJECT_DETECT_DRAW_PD) &&
                    (objectDataDesc.objType != TI_OD_PEDESTRIAN))
                {
                    continue;
                }
                if ((pObj->algLinkCreateParams.drawOption == ALGORITHM_LINK_OBJECT_DETECT_DRAW_TSR) &&
                    (objectDataDesc.objType != TI_OD_TRAFFIC_SIGN))
                {
                    continue;
                }
            }
            bufAddr[0] = (UInt32)lumaAddr;
            bufAddr[1] = (UInt32)chromaAddr;

            Draw2D_updateBufAddr(pObj->draw2DHndl, bufAddr);

            isValidObj = FALSE;

            if(objectDataDesc.objType==TI_OD_PEDESTRIAN
                    ||
               objectDataDesc.objType==TI_OD_TRAFFIC_SIGN
                )
            {
                UInt32 scale;

                scale = (UInt32)(objectDataDesc.objScale+0.5);

                if(scale<=2)
                {
                    pObj->linePrm.lineColor = 0x960000; /* in YUV */
                }
                else
                if(scale<=5)
                {
                    pObj->linePrm.lineColor = 0x6B25F3; /* in YUV */
                }
                else
                {
                    pObj->linePrm.lineColor = 0x4C34FF; /* in YUV */
                }
                pObj->linePrm.lineColorFormat = SYSTEM_DF_YUV420SP_UV;
                isValidObj = TRUE;
            }


            if(isValidObj)
            {
                status = Draw2D_drawRect(
                                        pObj->draw2DHndl,
                                        objectDataDesc.xPos,
                                        objectDataDesc.yPos,
                                        objectDataDesc.objWidth,
                                        objectDataDesc.objHeight,
                                        &pObj->linePrm
                                    );
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                #if 0
                Vps_printf(" OBJECT_DETECT: #%d: Type=%d, Score=%d, Scale=%f\n", i, objectDataDesc.objType, objectDataDesc.objScore, objectDataDesc.objScale);
                #endif
            }


            if(objectDataDesc.objType==TI_OD_TRAFFIC_SIGN)
            {
                AlgorithmLink_objectDrawCopyTrafficSign(
                    (UInt8*)bufAddr[0],
                    (UInt8*)bufAddr[1],
                    pObj->draw2DBufInfo.bufPitch[0],
                    pObj->draw2DBufInfo.bufPitch[1],
                    pObj->draw2DBufInfo.bufWidth,
                    pObj->draw2DBufInfo.bufHeight,
                    objectDataDesc.xPos - 16,
                    objectDataDesc.yPos - 16,
                    objectDataDesc.objTag,
                    OBJECTDRAW_TRAFFIC_SIGN_32x32
                    );
            }

        }
    }
    return status;
}

Int32 AlgorithmLink_objectDrawCopyInput(
                                        AlgorithmLink_ObjectDrawObj *pObj,
                                        System_VideoFrameCompositeBuffer *pCompBuffer,
                                        System_Buffer *pOutBuffer
                                                      )
{
    System_VideoFrameBuffer *pVidFrame = (System_VideoFrameBuffer *)pOutBuffer->payload;
    Utils_DmaCopyFill2D dmaPrm;
    Int32 status;


    dmaPrm.dataFormat   = SYSTEM_DF_YUV420SP_UV;
    dmaPrm.destAddr[0]  = (Ptr) pVidFrame->bufAddr[0];
    dmaPrm.destAddr[1]  = (Ptr) pVidFrame->bufAddr[1];
    dmaPrm.destPitch[0] = pObj->outputQInfo.queInfo.chInfo[0].pitch[0];
    dmaPrm.destPitch[1] = pObj->outputQInfo.queInfo.chInfo[0].pitch[1];
    dmaPrm.destStartX   = pObj->outputQInfo.queInfo.chInfo[0].startX;
    dmaPrm.destStartY   = pObj->outputQInfo.queInfo.chInfo[0].startY;
    dmaPrm.width        = pObj->outputQInfo.queInfo.chInfo[0].width;
    dmaPrm.height       = pObj->outputQInfo.queInfo.chInfo[0].height;
    dmaPrm.srcAddr[0]   = (Ptr) pCompBuffer->bufAddr[0][0];
    dmaPrm.srcAddr[1]   = (Ptr) pCompBuffer->bufAddr[1][0];
    dmaPrm.srcPitch[0]  = pObj->outputQInfo.queInfo.chInfo[0].pitch[0];
    dmaPrm.srcPitch[1]  = pObj->outputQInfo.queInfo.chInfo[0].pitch[1];
    dmaPrm.srcStartX    = pObj->algLinkCreateParams.imgFrameStartX;
    dmaPrm.srcStartY    = pObj->algLinkCreateParams.imgFrameStartY;

    status = Utils_dmaCopy2D(&pObj->copyFramesDmaObj,
                             &dmaPrm,
                             1);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}


Int32 AlgorithmLink_objectDrawProcess(void * pObj)
{
    UInt32 bufId;
    Int32  status = SYSTEM_LINK_STATUS_SOK;
    Bool   bufDropFlag;
    AlgorithmLink_ObjectDrawObj           * pObjectDrawObj;
    AlgorithmLink_ObjectDrawCreateParams  * pLinkCreateParams;
    System_Buffer                         * pSysOutBuffer;
    System_Buffer                         * pSysInBuffer;
    System_VideoFrameBuffer               * pVideoBuffer;
    System_VideoFrameCompositeBuffer      * pCompositeBuffer;
    System_BufferList                       inputBufList;
    System_BufferList                       outputBufListReturn;
    System_BufferList                       inputBufListReturn;
    System_LinkStatistics                 * linkStatsInfo;
    Void *lumaBufAddr;
    Void *chromaBufAddr;
    Void *metaDataBufAddr;

    pObjectDrawObj = (AlgorithmLink_ObjectDrawObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    linkStatsInfo = pObjectDrawObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);


    pLinkCreateParams = &pObjectDrawObj->algLinkCreateParams;

    if (pObjectDrawObj->isFirstFrameRecv == FALSE)
    {
        pObjectDrawObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(&linkStatsInfo->linkStats, 1, 1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

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
                /*
                 *   pSysOutBuffer->payload
                 *   pCompositeBuffer->bufAddr[0][0];
                 *
                 * TODO: Copy input frame into output frame using EDMA
                 */
                status = AlgorithmLink_objectDrawCopyInput(
                                                        pObjectDrawObj,
                                                        pCompositeBuffer,
                                                        pSysOutBuffer
                                                      );
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                pSysOutBuffer->srcTimestamp = pSysInBuffer->srcTimestamp;
                pSysOutBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

                metaDataBufAddr = pCompositeBuffer->bufAddr[0][1];

                /*
                 * Invalidate part of the buffer to read size of valid
                 * metadata buffer
                 */
                Cache_inv(metaDataBufAddr,
                          sizeof(TI_OD_output),
                          Cache_Type_ALLD,
                          TRUE
                        );

                pVideoBuffer    = (System_VideoFrameBuffer *)pSysOutBuffer->payload;

                lumaBufAddr     = pVideoBuffer->bufAddr[0];
                chromaBufAddr   = pVideoBuffer->bufAddr[1];

                AlgorithmLink_objectDrawRectangles(
                                                        pObjectDrawObj,
                                                        lumaBufAddr,
                                                        chromaBufAddr,
                                                        metaDataBufAddr
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

Int32 AlgorithmLink_objectDrawControl(void * pObj,
                                               void * pControlParams)
{
    AlgorithmLink_ObjectDrawObj* pObjectDrawObj;
    AlgorithmLink_ControlParams         * pAlgLinkControlPrm;
    AlgorithmLink_ObjectDrawSetROIParams *pROIPrm;

    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pObjectDrawObj = (AlgorithmLink_ObjectDrawObj *)
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
        case ALGORITHM_LINK_OBJECT_DRAW_CMD_SET_ROI:

            if(pAlgLinkControlPrm->size != sizeof(*pROIPrm))
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            else
            {
                pROIPrm = (AlgorithmLink_ObjectDrawSetROIParams *)
                                    pControlParams;

                pObjectDrawObj->algLinkCreateParams.imgFrameStartX
                    = pROIPrm->imgFrameStartX;

                pObjectDrawObj->algLinkCreateParams.imgFrameStartY
                    = pROIPrm->imgFrameStartY;

                pObjectDrawObj->algLinkCreateParams.imgFrameWidth
                    = pROIPrm->imgFrameWidth;

                pObjectDrawObj->algLinkCreateParams.imgFrameHeight
                    = pROIPrm->imgFrameHeight;
            }
            break;

        case SYSTEM_CMD_PRINT_STATISTICS:
            AlgorithmLink_objectDrawPrintStatistics(
                                                             pObj,
                                                             pObjectDrawObj
                                                            );
            break;

        default:
            break;
    }

    return status;
}
Int32 AlgorithmLink_objectDrawStop(void * pObj)
{
    return 0;
}
Int32 AlgorithmLink_objectDrawDelete(void * pObj)
{
    UInt32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 bufId;
    AlgorithmLink_ObjectDrawObj                   * pObjectDrawObj;
    AlgorithmLink_ObjectDrawCreateParams    * pLinkCreateParams;
    System_VideoFrameBuffer                     * pVideoBuffer;

    pObjectDrawObj = (AlgorithmLink_ObjectDrawObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    status = Utils_linkStatsCollectorDeAllocInst(pObjectDrawObj->linkStatsInfo);
    UTILS_assert(status==0);

    pLinkCreateParams = &pObjectDrawObj->algLinkCreateParams;

    /*
     * Free link buffers
     */
    for (bufId = 0; bufId < pLinkCreateParams->numOutBuffers; bufId++)
    {
        pVideoBuffer  =   &pObjectDrawObj->videoBuffers[bufId];

        status = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pVideoBuffer->bufAddr[0],
                                pObjectDrawObj->outBufferSize_y
                               );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        status = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pVideoBuffer->bufAddr[1],
                                pObjectDrawObj->outBufferSize_uv
                               );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    Utils_dmaDeleteCh(&pObjectDrawObj->copyFramesDmaObj);

    status = Draw2D_delete(pObjectDrawObj->draw2DHndl);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Utils_memFree(
                       UTILS_HEAPID_DDR_CACHED_LOCAL,
                       pObjectDrawObj,
                       sizeof(AlgorithmLink_ObjectDrawObj)
                    );

    return status;
}

Int32 AlgorithmLink_objectDrawPrintStatistics(void *pObj,
                AlgorithmLink_ObjectDrawObj *pObjectDrawObj)
{
    UTILS_assert(NULL != pObjectDrawObj->linkStatsInfo);

    Utils_printLinkStatistics(&pObjectDrawObj->linkStatsInfo->linkStats,
                            "ALG_OBJECT_DRAW",
                            TRUE);

    Utils_printLatency("ALG_OBJECT_DRAW",
                       &pObjectDrawObj->linkStatsInfo->linkLatency,
                       &pObjectDrawObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}

