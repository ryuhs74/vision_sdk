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
 * \file laneDetectLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for algorithm plugin
 *         Link
 *
 * \version 0.0 (Feb 2014) : [NN] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "laneDetectLink_priv.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>
#include <math.h>

/* uncomment below to disable calling of alg process API - used for debug ONLY */
//#define ALG_DISABLE

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
Int32 AlgorithmLink_laneDetect_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_laneDetectCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_laneDetectProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_laneDetectControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_laneDetectStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_laneDetectDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_LANE_DETECT;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Feature Plane Alg uses the IVISION standard to interact with the
 *        framework. All process/control calls to the algorithm should adhere
 *        to the IVISION standard. This function initializes input and output
 *        buffers
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_laneDetectAlgParamsInit (
                                  AlgorithmLink_LaneDetectObj *pObj,
                                  System_LinkChInfo *inChInfo
                                )
{
    pObj->algLdInBufs.size    = sizeof(pObj->algLdInBufs);
    pObj->algLdInBufs.numBufs = 1;
    pObj->algLdInBufs.bufDesc = pObj->algLdInBufDesc;

    pObj->algLdInBufDesc[0]   = &pObj->algLdInBufDescImage;

    pObj->algLdOutBufs.size    = sizeof(pObj->algLdOutBufs);
    pObj->algLdOutBufs.numBufs = 1;
    pObj->algLdOutBufs.bufDesc = pObj->algLdOutBufDesc;

    pObj->algLdOutBufDesc[0]   = &pObj->algLdOutBufDescLanePoints;

    memset(&pObj->algLdInArgs, 0, sizeof(pObj->algLdInArgs));
    pObj->algLdInArgs.iVisionInArgs.size         = sizeof(pObj->algLdInArgs);
    pObj->algLdInArgs.iVisionInArgs.subFrameInfo = 0;

    pObj->algLdInArgs.cannyHighThresh        = pObj->algLinkCreateParams.cannyHighThresh;
    pObj->algLdInArgs.cannyLowThresh         = pObj->algLinkCreateParams.cannyLowThresh;
    pObj->algLdInArgs.houghNmsThresh         = pObj->algLinkCreateParams.houghNmsThresh;
    pObj->algLdInArgs.startThetaLeft         = pObj->algLinkCreateParams.startThetaLeft;
    pObj->algLdInArgs.endThetaLeft           = pObj->algLinkCreateParams.endThetaLeft;
    pObj->algLdInArgs.startThetaRight        = pObj->algLinkCreateParams.startThetaRight;
    pObj->algLdInArgs.endThetaRight          = pObj->algLinkCreateParams.endThetaRight;
    pObj->algLdInArgs.thetaStepSize          = pObj->algLinkCreateParams.thetaStepSize;
    pObj->algLdInArgs.numHoughMaximasDet     = pObj->algLinkCreateParams.numHoughMaximasDet;
    pObj->algLdInArgs.numHoughMaximasTrack   = pObj->algLinkCreateParams.numHoughMaximasTrack;
    pObj->algLdInArgs.trackingMethod         = (LD_TI_TRACKMETHOD)pObj->algLinkCreateParams.trackingMethod;
    pObj->algLdInArgs.warningMethod          = (LD_TI_WARNING_INFO)pObj->algLinkCreateParams.warningMethod;
    pObj->algLdInArgs.departThetaLeftMin     = pObj->algLinkCreateParams.departThetaLeftMin;
    pObj->algLdInArgs.departThetaLeftMax     = pObj->algLinkCreateParams.departThetaLeftMax;
    pObj->algLdInArgs.departRhoLeftMin       = pObj->algLinkCreateParams.departRhoLeftMin;
    pObj->algLdInArgs.departRhoLeftMax       = pObj->algLinkCreateParams.departRhoLeftMax;
    pObj->algLdInArgs.departThetaRightMin    = pObj->algLinkCreateParams.departThetaRightMin;
    pObj->algLdInArgs.departThetaRightMax    = pObj->algLinkCreateParams.departThetaRightMax;
    pObj->algLdInArgs.departRhoRightMin      = pObj->algLinkCreateParams.departRhoRightMin;
    pObj->algLdInArgs.departRhoRightMax      = pObj->algLinkCreateParams.departRhoRightMax;

    memset(&pObj->algLdOutArgs, 0, sizeof(pObj->algLdOutArgs));
    pObj->algLdOutArgs.iVisionOutArgs.size         = sizeof(pObj->algLdOutArgs);

    memset(&pObj->algLdInBufDescImage, 0, sizeof(pObj->algLdInBufDescImage));
    pObj->algLdInBufDescImage.numPlanes                        = 1;
    /* Will be filled with input pointer later
     */
    pObj->algLdInBufDescImage.bufPlanes[0].buf                 = NULL;
    pObj->algLdInBufDescImage.bufPlanes[0].width               = inChInfo->pitch[0];
    pObj->algLdInBufDescImage.bufPlanes[0].height              = pObj->algLinkCreateParams.imgFrameHeight;
    pObj->algLdInBufDescImage.bufPlanes[0].frameROI.topLeft.x  = pObj->algLinkCreateParams.roiStartX;
    pObj->algLdInBufDescImage.bufPlanes[0].frameROI.topLeft.y  = pObj->algLinkCreateParams.roiStartY;
    pObj->algLdInBufDescImage.bufPlanes[0].frameROI.width      = pObj->algLinkCreateParams.roiWidth;
    pObj->algLdInBufDescImage.bufPlanes[0].frameROI.height     = pObj->algLinkCreateParams.roiHeight;
    pObj->algLdInBufDescImage.bufPlanes[0].planeType           = 0;
    pObj->algLdInBufDescImage.formatType                       = 0; /* NOT USED */
    pObj->algLdInBufDescImage.bufferId                         = 0xFF; /* NOT USED */

    memset(&pObj->algLdOutBufDescLanePoints, 0, sizeof(pObj->algLdOutBufDescLanePoints));
    pObj->algLdOutBufDescLanePoints.numPlanes                        = 1;
    /* Will be filled with output pointer later
     */
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].buf                 = NULL;
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].width               = LD_TI_MAXLANEPOINTS*sizeof(LD_TI_output);
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].height              = 1;
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].frameROI.topLeft.x  = 0;
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].frameROI.topLeft.y  = 0;
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].frameROI.width      = LD_TI_MAXLANEPOINTS*sizeof(LD_TI_output);
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].frameROI.height     = 1;
    pObj->algLdOutBufDescLanePoints.bufPlanes[0].planeType           = 0;
    pObj->algLdOutBufDescLanePoints.formatType                       = 0; /* NOT USED */
    pObj->algLdOutBufDescLanePoints.bufferId                         = 0xFF; /* NOT USED */

    pObj->algLdCreateParams.visionParams.algParams.size = sizeof(pObj->algLdCreateParams);
    pObj->algLdCreateParams.visionParams.cacheWriteBack = NULL;
    pObj->algLdCreateParams.maxImageWidth              = pObj->algLinkCreateParams.imgFrameWidth;
    pObj->algLdCreateParams.maxImageHeight             = pObj->algLinkCreateParams.imgFrameHeight;
    pObj->algLdCreateParams.maxRho                     = pObj->algLinkCreateParams.maxRho;

    pObj->algLdCreateParams.maxTheta                   = LD_TI_MAXNUMTHETA;
    pObj->algLdCreateParams.edma3RmLldHandle           = NULL;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for feature plane computation alg link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_laneDetectCreate(void * pObj,void * pCreateParams)
{
    UInt32 status;
    UInt32 prevLinkQId;
    UInt32 bufId;
    System_LinkInfo                                     prevLinkInfo;
    AlgorithmLink_LaneDetectObj                  * pAlgObj;
    AlgorithmLink_OutputQueueInfo                       * pOutputQInfo;
    AlgorithmLink_InputQueueInfo                        * pInputQInfo;
    System_LinkChInfo                                   * pOutChInfo;
    System_Buffer                                       * pSystemBuffer;
    System_MetaDataBuffer                               * pMetaDataBuffer;

    pAlgObj = (AlgorithmLink_LaneDetectObj *)
                    Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_LOCAL,
                        sizeof(AlgorithmLink_LaneDetectObj),
                        128);
    UTILS_assert(pAlgObj != NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pAlgObj);

    pInputQInfo       = &pAlgObj->inputQInfo;
    pOutputQInfo      = &pAlgObj->outputQInfo;

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy(
            &pAlgObj->algLinkCreateParams,
            pCreateParams,
            sizeof(pAlgObj->algLinkCreateParams)
           );

    pInputQInfo->qMode          = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo->qMode         = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    status = System_linkGetInfo(
                    pAlgObj->algLinkCreateParams.inQueParams.prevLinkId,
                    &prevLinkInfo
                    );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    UTILS_assert(prevLinkInfo.numQue >= 1);

    prevLinkQId = pAlgObj->algLinkCreateParams.inQueParams.prevLinkQueId;
    pAlgObj->inChInfo = prevLinkInfo.queInfo[prevLinkQId].chInfo[0];

    if(SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pAlgObj->inChInfo.flags)
            != SYSTEM_DF_YUV420SP_UV
       )
    {
        UTILS_assert(NULL);
    }

    pOutputQInfo->queInfo.numCh = 1;

    pOutChInfo          = &pOutputQInfo->queInfo.chInfo[0];
    pOutChInfo->flags   = 0;
    pOutChInfo->height  = 0;
    pOutChInfo->width   = 0;

    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(
                                    pObj,
                                    1,
                                    pInputQInfo,
                                    1,
                                    pOutputQInfo
                               );

    AlgorithmLink_laneDetectAlgParamsInit(pAlgObj, &pAlgObj->inChInfo);

    pAlgObj->algLdHandle = AlgIvision_create(
                        &LD_TI_VISION_FXNS,
                        (IALG_Params *)&pAlgObj->algLdCreateParams
                        );
    UTILS_assert(pAlgObj->algLdHandle!=NULL);

    if(pAlgObj->algLinkCreateParams.numOutBuffers
        > LANEDETECT_LINK_MAX_NUM_OUTPUT)
    {
        pAlgObj->algLinkCreateParams.numOutBuffers
            = LANEDETECT_LINK_MAX_NUM_OUTPUT;
    }

    pAlgObj->tmpBufSize = 4*1024*1024;
    pAlgObj->tmpBuf = Utils_memAlloc(
                                 UTILS_HEAPID_DDR_CACHED_SR,
                                 pAlgObj->tmpBufSize,
                                 128
                                 );
    UTILS_assert(pAlgObj->tmpBuf!=NULL);

    /*
     * Allocate memory for the output buffers and link metadata buffer with
     * system Buffer
     */

    for (bufId = 0; bufId < pAlgObj->algLinkCreateParams.numOutBuffers; bufId++)
    {
        pSystemBuffer       =   &pAlgObj->buffers[bufId];
        pMetaDataBuffer     =   &pAlgObj->metaDataBuffers[bufId];

        /*
         * Properties of pSystemBuffer, which do not get altered during
         * run time (frame exchanges) are initialized here
         */
        pSystemBuffer->bufType      =   SYSTEM_BUFFER_TYPE_METADATA;
        pSystemBuffer->payload      =   pMetaDataBuffer;
        pSystemBuffer->payloadSize  =   sizeof(System_MetaDataBuffer);
        pSystemBuffer->chNum        =   0;

        pMetaDataBuffer->numMetaDataPlanes  =  1;
        pMetaDataBuffer->metaBufSize[0]
            =  LANEDETECT_LINK_LANE_POINTS_BUF_SIZE;

        pMetaDataBuffer->bufAddr[0]         =  Utils_memAlloc(
                                                    UTILS_HEAPID_DDR_CACHED_SR,
                                                    pMetaDataBuffer->metaBufSize[0],
                                                    128
                                                    );

        pMetaDataBuffer->metaFillLength[0] = pMetaDataBuffer->metaBufSize[0];

        UTILS_assert(pMetaDataBuffer->bufAddr[0] != NULL);

        pMetaDataBuffer->flags = 0;

        status = AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSystemBuffer);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    pAlgObj->isFirstFrameRecv    = FALSE;

    /* Assign pointer to link stats object */
    pAlgObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_LANE_DETECT");
    UTILS_assert(NULL != pAlgObj->linkStatsInfo);

    return SYSTEM_LINK_STATUS_SOK;
}


Int32 AlgorithmLink_laneDetectProcess(void * pObj)
{
    UInt32 bufId;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool   bufDropFlag;
    AlgorithmLink_LaneDetectObj                  * pAlgObj;
    System_BufferList                                     inputBufList;
    AlgorithmLink_LaneDetectCreateParams         * pLinkCreatePrms;
    System_Buffer                                       * pSysOutBuffer;
    System_Buffer                                       * pSysInBuffer;
    System_VideoFrameBuffer                             * pInVideoBuf;
    System_MetaDataBuffer                               * pOutMetaBuf;
    System_BufferList                                   bufListReturn;
    System_LinkStatistics *linkStatsInfo;

    pAlgObj = (AlgorithmLink_LaneDetectObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);


    pLinkCreatePrms = &pAlgObj->algLinkCreateParams;

    linkStatsInfo = pAlgObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    if (pAlgObj->isFirstFrameRecv == FALSE)
    {
        pAlgObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(&linkStatsInfo->linkStats, 1, 1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }

    linkStatsInfo->linkStats.newDataCmdCount++;

    System_getLinksFullBuffers(
                        pLinkCreatePrms->inQueParams.prevLinkId,
                        pLinkCreatePrms->inQueParams.prevLinkQueId,
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
                AlgorithmLink_LaneDetectOutput *pLaneDetectOutput;

                pSysOutBuffer->srcTimestamp = pSysInBuffer->srcTimestamp;
                pSysOutBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();
                pOutMetaBuf  = (System_MetaDataBuffer *)pSysOutBuffer->payload;

                pInVideoBuf  = (System_VideoFrameBuffer *) pSysInBuffer->payload;

                pLaneDetectOutput = (AlgorithmLink_LaneDetectOutput*)pOutMetaBuf->bufAddr[0];

                pAlgObj->algLdInBufDescImage.bufPlanes[0].buf =
                    (Void*)( (UInt32)pInVideoBuf->bufAddr[0]
                                +
                            pAlgObj->algLinkCreateParams.imgFrameStartY*
                            pAlgObj->inChInfo.pitch[0]
                                +
                            pAlgObj->algLinkCreateParams.imgFrameStartX
                           )
                            ;

                pAlgObj->algLdOutBufDescLanePoints.bufPlanes[0].buf = (Void*)
                    (&pLaneDetectOutput->laneInfo[0]);

                Cache_inv(
                          pInVideoBuf->bufAddr[0],
                          pAlgObj->inChInfo.pitch[0]*pAlgObj->inChInfo.height,
                          Cache_Type_ALLD,
                          TRUE
                        );

                #ifndef ALG_DISABLE

                #if 0
                Vps_printf(" ALG_LANE_DETECT: Running .... !!!\n"
                    );
                #endif

                /* clear L2MEM, this is required to make sure the scratch memory
                 * for lane detect is set to known state before execution.
                 * When multiple algorithms run on same DSP, this memory
                 * could be filled by other algorithms
                 *
                 * NOTE: This workaround is specific to this algorithm
                 *       and is NOT a general rule to use for DSP programming.
                 */
                memset((void*)0x800000, 0, 24*1024);

                status = AlgIvision_process(
                                pAlgObj->algLdHandle,
                                &pAlgObj->algLdInBufs,
                                &pAlgObj->algLdOutBufs,
                                (IVISION_InArgs*)&pAlgObj->algLdInArgs,
                                (IVISION_OutArgs *)&pAlgObj->algLdOutArgs
                    );

                #if 0
                Vps_printf(" ALG_LANE_DETECT: Done !!!\n"
                    );
                #endif

                #if 1
                if(status!=IALG_EOK)
                {
                    Vps_printf(" ALG_LANEDETECT: ERROR: Alg process !!!\n");
                }
                #endif
                #else
                pAlgObj->algLdOutArgs.numLeftLanePoints = 0;
                pAlgObj->algLdOutArgs.numRightLanePoints = 0;
                pAlgObj->algLdOutArgs.infoFlag = 0;
                #endif

                pLaneDetectOutput->numLeftLanePoints
                    = pAlgObj->algLdOutArgs.numLeftLanePoints;

                pLaneDetectOutput->numRightLanePoints
                    = pAlgObj->algLdOutArgs.numRightLanePoints;

                pLaneDetectOutput->laneCrossInfo
                    = pAlgObj->algLdOutArgs.infoFlag;

                Cache_wb(
                          pOutMetaBuf->bufAddr[0],
                          pOutMetaBuf->metaBufSize[0],
                          Cache_Type_ALLD,
                          TRUE
                        );

                Utils_updateLatency(&linkStatsInfo->linkLatency,
                                    pSysOutBuffer->linkLocalTimestamp);
                Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                    pSysOutBuffer->srcTimestamp);

                linkStatsInfo->linkStats.chStats
                            [0].inBufProcessCount++;
                linkStatsInfo->linkStats.chStats
                            [0].outBufCount[0]++;

                status = AlgorithmLink_putFullOutputBuffer(
                                            pObj,
                                            0,
                                            pSysOutBuffer);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                System_sendLinkCmd(
                        pLinkCreatePrms->outQueParams.nextLink,
                        SYSTEM_CMD_NEW_DATA,
                        NULL);

                bufListReturn.numBuf = 1;
                bufListReturn.buffers[0] = pSysOutBuffer;
                status = AlgorithmLink_releaseOutputBuffer(
                                                   pObj,
                                                   0,
                                                   &bufListReturn
                                                  );
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }

            bufListReturn.numBuf = 1;
            bufListReturn.buffers[0] = pSysInBuffer;
            bufDropFlag = FALSE;
            AlgorithmLink_releaseInputBuffer(
                          pObj,
                          0,
                          pLinkCreatePrms->inQueParams.prevLinkId,
                          pLinkCreatePrms->inQueParams.prevLinkQueId,
                          &bufListReturn,
                          &bufDropFlag);
        }
    }
    return status;
}

Int32 AlgorithmLink_laneDetectControl(void * pObj,
                                               void * pControlParams)
{
    AlgorithmLink_LaneDetectObj* pAlgObj;
    AlgorithmLink_ControlParams         * pAlgLinkControlPrm;

    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pAlgObj = (AlgorithmLink_LaneDetectObj *)
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
            AlgorithmLink_laneDetectPrintStatistics(
                                                                pObj,
                                                                pAlgObj
                                                            );
            break;

        default:
            break;
    }

    return status;
}

Int32 AlgorithmLink_laneDetectStop(void * pObj)
{
    return 0;
}

Int32 AlgorithmLink_laneDetectDelete(void * pObj)
{
    Int32 status;
    UInt32 bufId;
    AlgorithmLink_LaneDetectObj *pAlgObj;
    System_MetaDataBuffer              *pMetaDataBuffer;

    pAlgObj = (AlgorithmLink_LaneDetectObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    status = Utils_linkStatsCollectorDeAllocInst(pAlgObj->linkStatsInfo);
    UTILS_assert(status==0);

    /*
     * Free allocated memory for alg internal objects
     */
    status = AlgIvision_delete(pAlgObj->algLdHandle);
    UTILS_assert(status==0);

    /*
     * Free link buffers
     */
    for (bufId = 0; bufId < pAlgObj->algLinkCreateParams.numOutBuffers; bufId++)
    {
        pMetaDataBuffer     =   &pAlgObj->metaDataBuffers[bufId];

        status = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pMetaDataBuffer->bufAddr[0],
                                pMetaDataBuffer->metaBufSize[0]
                               );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    status = Utils_memFree(
                            UTILS_HEAPID_DDR_CACHED_SR,
                            pAlgObj->tmpBuf,
                            pAlgObj->tmpBufSize
                           );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Utils_memFree(
                       UTILS_HEAPID_DDR_CACHED_LOCAL,
                       pAlgObj,
                       sizeof(AlgorithmLink_LaneDetectObj)
                    );
    return status;

}

Int32 AlgorithmLink_laneDetectPrintStatistics(void *pObj,
                AlgorithmLink_LaneDetectObj *pAlgObj)
{
    UTILS_assert(NULL != pAlgObj->linkStatsInfo);

    Utils_printLinkStatistics(&pAlgObj->linkStatsInfo->linkStats,
                            "ALG_LANE_DETECT",
                            TRUE);

    Utils_printLatency("ALG_LANE_DETECT",
                       &pAlgObj->linkStatsInfo->linkLatency,
                       &pAlgObj->linkStatsInfo->srcToLinkLatency,
                       TRUE);

    return SYSTEM_LINK_STATUS_SOK;
}

