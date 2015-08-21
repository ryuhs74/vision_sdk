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
 * \file featurePlaneCompLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for feature Plane computation
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

#include "sparseOpticalFlowLink_priv.h"
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
Int32 AlgorithmLink_sparseOpticalFlow_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_sparseOpticalFlowCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_sparseOpticalFlowProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_sparseOpticalFlowControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_sparseOpticalFlowStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_sparseOpticalFlowDelete;

#ifdef BUILD_ARP32
    algId = ALGORITHM_LINK_EVE_ALG_SPARSE_OPTICAL_FLOW;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}
/**
 *******************************************************************************
 *
 * \brief This function allocates memory for the algorithm internal objects
 *
 *
 * \param  numMemRec         [IN] Number of objects
 * \param  memRec            [IN] pointer to the memory records
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
UInt32 AlgorithmLink_sparseOpticalFlowAlgCreate(
                                       AlgorithmLink_SparseOpticalFlowChObj *pObj)
{
    /*
     * Intialize the algorithm instance with the alocated memory
     * and user create parameters
     */
    pObj->algSofHandle = AlgIvision_create(
                            &SOF_TI_VISION_FXNS,
                            (IALG_Params*)&pObj->algSofCreateParams
                            );
    UTILS_assert(pObj->algSofHandle!=NULL);

    pObj->algSofOutBufSizeKeyPoints     =
        pObj->algSofCreateParams.maxNumKeyPoints
        *
        sizeof(UInt16)
        *
        4
        ;

    pObj->algSofOutBufSizeErrEst        =
        pObj->algSofCreateParams.maxNumKeyPoints
        *
        sizeof(UInt16)
        ;

    pObj->algSofOutBufSizeTrackedPoints =
        SOF_ALGLINK_FLOW_TRACK_POINTS_BUF_SIZE
        ;

    pObj->pAlgSofOutBufKeyPoints =
        Utils_memAlloc(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->algSofOutBufSizeKeyPoints,
            128
          );
    UTILS_assert(pObj->pAlgSofOutBufKeyPoints!=NULL);

    pObj->pAlgSofOutBufErrEst =
        Utils_memAlloc(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->algSofOutBufSizeErrEst,
            128
          );
    UTILS_assert(pObj->pAlgSofOutBufErrEst!=NULL);

    pObj->pAlgSofOutBufTrackedPoints =
        Utils_memAlloc(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->algSofOutBufSizeTrackedPoints,
            128
          );
    UTILS_assert(pObj->pAlgSofOutBufTrackedPoints!=NULL);

    pObj->algSofOutBufDescKeyPoints.bufPlanes[0].buf
        = pObj->pAlgSofOutBufKeyPoints;
    pObj->algSofOutBufDescErrEst.bufPlanes[0].buf
        = pObj->pAlgSofOutBufErrEst;
    pObj->algSofOutBufDescTrackedPoints.bufPlanes[0].buf
        = pObj->pAlgSofOutBufTrackedPoints;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function frees memory for the algorithm internal objects
 *
 *
 * \param  numMemRec         [IN] Number of objects
 * \param  memRec            [IN] pointer to the memory records
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
UInt32 AlgorithmLink_sparseOpticalFlowAlgDelete(
                                      AlgorithmLink_SparseOpticalFlowChObj *pObj)
{
    Int32 status;

    status = AlgIvision_delete(pObj->algSofHandle);
    UTILS_assert(status==0);

    status = Utils_memFree(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->pAlgSofOutBufKeyPoints,
            pObj->algSofOutBufSizeKeyPoints
          );
    UTILS_assert(status==0);

    status = Utils_memFree(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->pAlgSofOutBufErrEst,
            pObj->algSofOutBufSizeErrEst
          );
    UTILS_assert(status==0);

    status = Utils_memFree(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->pAlgSofOutBufTrackedPoints,
            pObj->algSofOutBufSizeTrackedPoints
          );
    UTILS_assert(status==0);

    return status;
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
Int32 AlgorithmLink_sparseOpticalFlowAlgParamsInit (
                                  AlgorithmLink_SparseOpticalFlowObj *pLinkObj,
                                  AlgorithmLink_SparseOpticalFlowChObj *pObj,
                                  System_LinkChInfo *inChInfo
                                )
{
    UInt32 i;

    pObj->algSofInBufs.size    = sizeof(pObj->algSofInBufs);
    pObj->algSofInBufs.numBufs = 2;
    pObj->algSofInBufs.bufDesc = pObj->algSofInBufDesc;

    pObj->algSofInBufDesc[0]   = &pObj->algSofInBufDescImage;
    pObj->algSofInBufDesc[1]   = &pObj->algSofInBufDescKeyPoints;

    pObj->algSofOutBufs.size    = sizeof(pObj->algSofOutBufs);
    pObj->algSofOutBufs.numBufs = 3;
    pObj->algSofOutBufs.bufDesc = pObj->algSofOutBufDesc;

    pObj->algSofOutBufDesc[0]   = &pObj->algSofOutBufDescKeyPoints;
    pObj->algSofOutBufDesc[1]   = &pObj->algSofOutBufDescErrEst;
    pObj->algSofOutBufDesc[2]   = &pObj->algSofOutBufDescTrackedPoints;

    memset(&pObj->algSofInArgs, 0, sizeof(pObj->algSofInArgs));
    pObj->algSofInArgs.iVisionInArgs.size         = sizeof(pObj->algSofInArgs);
    pObj->algSofInArgs.iVisionInArgs.subFrameInfo = 0;
    pObj->algSofInArgs.numCorners                 = 0;
    pObj->algSofInArgs.trackErrThr                = 600;
    pObj->algSofInArgs.trackMinFlowQ4             = 32;
    pObj->algSofInArgs.trackNmsWinSize            = 0;
    pObj->algSofInArgs.reservered0                = 0;

    memset(&pObj->algSofOutArgs, 0, sizeof(pObj->algSofOutArgs));
    pObj->algSofOutArgs.iVisionOutArgs.size         = sizeof(pObj->algSofOutArgs);

    memset(&pObj->algSofInBufDescImage, 0, sizeof(pObj->algSofInBufDescImage));
    pObj->algSofInBufDescImage.numPlanes                        = 1;
    /* Will be filled with input pointer later
     */
    pObj->algSofInBufDescImage.bufPlanes[0].buf                 = NULL;
    pObj->algSofInBufDescImage.bufPlanes[0].width               = inChInfo->pitch[0];
    pObj->algSofInBufDescImage.bufPlanes[0].height              = inChInfo->height;
    pObj->algSofInBufDescImage.bufPlanes[0].frameROI.topLeft.x  = pLinkObj->algLinkCreateParams.imgFrameStartX;
    pObj->algSofInBufDescImage.bufPlanes[0].frameROI.topLeft.y  = pLinkObj->algLinkCreateParams.imgFrameStartY;
    pObj->algSofInBufDescImage.bufPlanes[0].frameROI.width      = pLinkObj->algLinkCreateParams.imgFrameWidth;
    pObj->algSofInBufDescImage.bufPlanes[0].frameROI.height     = pLinkObj->algLinkCreateParams.imgFrameHeight;
    pObj->algSofInBufDescImage.formatType                       = 0; /* NOT USED */
    pObj->algSofInBufDescImage.bufferId                         = 0xFF; /* NOT USED */

    memset(&pObj->algSofInBufDescKeyPoints, 0, sizeof(pObj->algSofInBufDescKeyPoints));
    pObj->algSofInBufDescKeyPoints.numPlanes                        = 1;
    /* Will be filled with input pointer later
     */
    pObj->algSofInBufDescKeyPoints.bufPlanes[0].buf                 = NULL;

    memset(&pObj->algSofOutBufDescKeyPoints, 0, sizeof(pObj->algSofOutBufDescKeyPoints));
    pObj->algSofOutBufDescKeyPoints.numPlanes                        = 1;
    /* Will be filled with input pointer later
     */
    pObj->algSofOutBufDescKeyPoints.bufPlanes[0].buf                 = NULL;

    memset(&pObj->algSofOutBufDescErrEst, 0, sizeof(pObj->algSofOutBufDescErrEst));
    pObj->algSofOutBufDescErrEst.numPlanes                        = 1;
    /* Will be filled with input pointer later
     */
    pObj->algSofOutBufDescErrEst.bufPlanes[0].buf                 = NULL;

    memset(&pObj->algSofOutBufDescTrackedPoints, 0, sizeof(pObj->algSofOutBufDescTrackedPoints));
    pObj->algSofOutBufDescTrackedPoints.numPlanes                        = 1;
    /* Will be filled with input pointer later
     */
    pObj->algSofOutBufDescTrackedPoints.bufPlanes[0].buf                 = NULL;

    pObj->algSofCreateParams.visionParams.algParams.size = sizeof(pObj->algSofCreateParams);
    pObj->algSofCreateParams.visionParams.cacheWriteBack = NULL;
    pObj->algSofCreateParams.imWidth                     = inChInfo->pitch[0];
    pObj->algSofCreateParams.imHeight                    = inChInfo->height;
    pObj->algSofCreateParams.roiWidth
        = SystemUtils_floor(
                pLinkObj->algLinkCreateParams.imgFrameWidth - SOF_ALGLINK_PAD_PIXELS*2,
                SOF_ALGLINK_PAD_PIXELS);
    pObj->algSofCreateParams.roiHeight
        = SystemUtils_floor(
                pLinkObj->algLinkCreateParams.imgFrameHeight - SOF_ALGLINK_PAD_PIXELS*2,
                SOF_ALGLINK_PAD_PIXELS);
    pObj->algSofCreateParams.startX                      = pLinkObj->algLinkCreateParams.imgFrameStartX + SOF_ALGLINK_PAD_PIXELS;
    pObj->algSofCreateParams.startY                      = pLinkObj->algLinkCreateParams.imgFrameStartY + SOF_ALGLINK_PAD_PIXELS;
    pObj->algSofCreateParams.numLevels                   = 5;
    if(pObj->algSofCreateParams.imHeight < 300)
    {
        pObj->algSofCreateParams.numLevels                   = 3;
    }
    pObj->algSofCreateParams.keyPointDetectMethod        = 1;
    pObj->algSofCreateParams.maxNumKeyPoints             = SOF_ALGLINK_TRACK_POINTS_MAX;
    pObj->algSofCreateParams.fast9Threshold              = 40;
    pObj->algSofCreateParams.scoreMethod                 = FAST9_BFTF_TI_THRESH_METHOD;
    pObj->algSofCreateParams.harrisScaling               = 1310;
    pObj->algSofCreateParams.nmsThreshold                = 286870912;
    pObj->algSofCreateParams.keyPointDetectInterval      = 5;

    pObj->algSofCreateParams.harrisScoreMethod           = 0;
    pObj->algSofCreateParams.harrisWindowSize            = 0;
    pObj->algSofCreateParams.suppressionMethod           = 2;

    for(i=0; i<PYRAMID_LK_TRACKER_TI_MAXLEVELS;i++)
    {
        if(i<2)
            pObj->algSofCreateParams.maxItersLK[i]  = 20;
        else
            pObj->algSofCreateParams.maxItersLK[i]  = 10;
        pObj->algSofCreateParams.minErrValue[i] = 64;
        pObj->algSofCreateParams.searchRange[i] = 12;
    }

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
Int32 AlgorithmLink_sparseOpticalFlowCreate(void * pObj,void * pCreateParams)
{
    UInt32 status;
    UInt32 prevLinkQId;
    UInt32 bufId, chId;
    System_LinkInfo                                     prevLinkInfo;
    AlgorithmLink_SparseOpticalFlowObj                  * pSOFObj;
    AlgorithmLink_OutputQueueInfo                       * pOutputQInfo;
    AlgorithmLink_InputQueueInfo                        * pInputQInfo;
    System_LinkChInfo                                   * pOutChInfo;
    System_LinkChInfo                                   * pPrevChInfo;
    System_Buffer                                       * pSystemBuffer;
    System_MetaDataBuffer                               * pMetaDataBuffer;

    pSOFObj = (AlgorithmLink_SparseOpticalFlowObj *)
                    Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_LOCAL,
                        sizeof(AlgorithmLink_SparseOpticalFlowObj),
                        32);
    UTILS_assert(pSOFObj != NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pSOFObj);

    pInputQInfo       = &pSOFObj->inputQInfo;
    pOutputQInfo      = &pSOFObj->outputQInfo;

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy(
            &pSOFObj->algLinkCreateParams,
            pCreateParams,
            sizeof(pSOFObj->algLinkCreateParams)
           );

    pInputQInfo->qMode          = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo->qMode         = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    status = System_linkGetInfo(
                    pSOFObj->algLinkCreateParams.inQueParams.prevLinkId,
                    &prevLinkInfo
                    );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    UTILS_assert(prevLinkInfo.numQue >= 1);

    prevLinkQId = pSOFObj->algLinkCreateParams.inQueParams.prevLinkQueId;

    UTILS_assert(prevLinkInfo.queInfo[prevLinkQId].numCh
            <
            SPARSEOPTICALFLOW_LINK_MAX_CH
         );

    if(pSOFObj->algLinkCreateParams.numOutBuffers
        > SPARSEOPTICALFLOW_LINK_MAX_NUM_OUTPUT)
    {
        pSOFObj->algLinkCreateParams.numOutBuffers
            = SPARSEOPTICALFLOW_LINK_MAX_NUM_OUTPUT;
    }

    pOutputQInfo->queInfo.numCh = prevLinkInfo.queInfo[prevLinkQId].numCh;

    for(chId=0; chId<prevLinkInfo.queInfo[prevLinkQId].numCh; chId++)
    {
        pOutChInfo          = &pOutputQInfo->queInfo.chInfo[chId];
        pOutChInfo->flags   = 0;
        pOutChInfo->height  = 0;
        pOutChInfo->width   = 0;
    }

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

    for(chId=0; chId<prevLinkInfo.queInfo[prevLinkQId].numCh; chId++)
    {
        UTILS_assert(chId < SPARSEOPTICALFLOW_LINK_MAX_CH);
        pPrevChInfo = &prevLinkInfo.queInfo[prevLinkQId].chInfo[chId];

        if(SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pPrevChInfo->flags)
                != SYSTEM_DF_YUV420SP_UV
        )
        {
            UTILS_assert(NULL);
        }

        AlgorithmLink_sparseOpticalFlowAlgParamsInit(
                        pSOFObj, &pSOFObj->chObj[chId], pPrevChInfo);

        AlgorithmLink_sparseOpticalFlowAlgCreate(
                &pSOFObj->chObj[chId]);

        /*
         * Allocate memory for the output buffers and link metadata buffer with
         * system Buffer
         */

        for (bufId = 0; bufId < pSOFObj->algLinkCreateParams.numOutBuffers; bufId++)
        {
            pSystemBuffer       =   &pSOFObj->chObj[chId].buffers[bufId];
            pMetaDataBuffer     =   &pSOFObj->chObj[chId].metaDataBuffers[bufId];

            /*
             * Properties of pSystemBuffer, which do not get altered during
             * run time (frame exchanges) are initialized here
             */
            pSystemBuffer->bufType      =   SYSTEM_BUFFER_TYPE_METADATA;
            pSystemBuffer->payload      =   pMetaDataBuffer;
            pSystemBuffer->payloadSize  =   sizeof(System_MetaDataBuffer);
            pSystemBuffer->chNum        =   chId;

            pMetaDataBuffer->numMetaDataPlanes  =  2;
            pMetaDataBuffer->metaBufSize[0]
                =  pSOFObj->chObj[chId].algSofOutBufSizeTrackedPoints;

            pMetaDataBuffer->bufAddr[0] =  Utils_memAlloc(
                                                UTILS_HEAPID_DDR_CACHED_SR,
                                                pMetaDataBuffer->metaBufSize[0],
                                                ALGORITHMLINK_FRAME_ALIGN
                                                );

            pMetaDataBuffer->metaFillLength[0] = pMetaDataBuffer->metaBufSize[0];

            UTILS_assert(pMetaDataBuffer->bufAddr[0] != NULL);

            pMetaDataBuffer->metaBufSize[1]
                = SPARSEOPTICALFLOW_SFM_META_DATA_MAX_SIZE;

            pMetaDataBuffer->bufAddr[1]  =  Utils_memAlloc(
                                                 UTILS_HEAPID_DDR_CACHED_SR,
                                                 pMetaDataBuffer->metaBufSize[1],
                                                 ALGORITHMLINK_FRAME_ALIGN
                                                 );

            pMetaDataBuffer->metaFillLength[1] = 0;

            UTILS_assert(pMetaDataBuffer->bufAddr[1] != NULL);

            pMetaDataBuffer->flags = 0;

            status = AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSystemBuffer);

            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    pSOFObj->isFirstFrameRecv    = FALSE;

    /* Assign pointer to link stats object */
    pSOFObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_SPARSE_OPTICAL_FLOW");
    UTILS_assert(NULL != pSOFObj->linkStatsInfo);

    return SYSTEM_LINK_STATUS_SOK;
}


Int32 AlgorithmLink_sparseOpticalFlowProcess(void * pObj)
{
    UInt32 bufId, chId;
    Int32  status = SYSTEM_LINK_STATUS_SOK;
    Bool   bufDropFlag;
    AlgorithmLink_SparseOpticalFlowObj                  * pSOFObj;
    AlgorithmLink_SparseOpticalFlowChObj                * pChObj;
    System_BufferList                                     inputBufList;
    AlgorithmLink_SparseOpticalFlowCreateParams         * pLinkCreatePrms;
    System_Buffer                                       * pSysOutBuffer;
    System_Buffer                                       * pSysInBuffer;
    System_VideoFrameBuffer                             * pInVideoBuf;
    System_MetaDataBuffer                               * pOutMetaBuf;
    System_BufferList                                   bufListReturn;
    System_LinkStatistics *linkStatsInfo;

    pSOFObj = (AlgorithmLink_SparseOpticalFlowObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);


    pLinkCreatePrms = &pSOFObj->algLinkCreateParams;

    linkStatsInfo = pSOFObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    if (pSOFObj->isFirstFrameRecv == FALSE)
    {
        pSOFObj->isFirstFrameRecv = TRUE;

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
            if(pSysInBuffer->chNum >= pSOFObj->outputQInfo.queInfo.numCh)
            {
                linkStatsInfo->linkStats.inBufErrorCount++;
                continue;
            }

            chId = pSysInBuffer->chNum;

            pChObj = &pSOFObj->chObj[chId];

            linkStatsInfo->linkStats.chStats[chId].inBufRecvCount++;

            status = AlgorithmLink_getEmptyOutputBuffer(
                                                    pObj,
                                                    0,
                                                    chId,
                                                    &pSysOutBuffer
                                                    );
            if(status != SYSTEM_LINK_STATUS_SOK)
            {
                linkStatsInfo->linkStats.chStats
                            [chId].inBufDropCount++;
                linkStatsInfo->linkStats.chStats
                            [chId].outBufDropCount[0]++;
            }
            else
            {
                pSysOutBuffer->srcTimestamp = pSysInBuffer->srcTimestamp;
                pSysOutBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();
                pOutMetaBuf  = (System_MetaDataBuffer *)pSysOutBuffer->payload;

                pInVideoBuf  = (System_VideoFrameBuffer *) pSysInBuffer->payload;

                pChObj->algSofInBufDescImage.bufPlanes[0].buf = pInVideoBuf->bufAddr[0];

                if(pInVideoBuf->metaBufAddr!=NULL
                    &&
                   pInVideoBuf->metaBufSize!=0
                    &&
                    pInVideoBuf->metaFillLength!=0
                    &&
                    pInVideoBuf->metaFillLength <=
                        pOutMetaBuf->metaBufSize[1]
                    )
                {
                    memcpy(
                        pOutMetaBuf->bufAddr[1],
                        pInVideoBuf->metaBufAddr,
                        pInVideoBuf->metaFillLength
                        );

                    pOutMetaBuf->metaFillLength[1] = pInVideoBuf->metaFillLength;
                }

                status = AlgIvision_process(
                                pChObj->algSofHandle,
                                &pChObj->algSofInBufs,
                                &pChObj->algSofOutBufs,
                                (IVISION_InArgs*)&pChObj->algSofInArgs,
                                (IVISION_OutArgs *)&pChObj->algSofOutArgs
                    );

                UTILS_assert(status==IALG_EOK);

                pChObj->algSofInArgs.numCorners = pChObj->algSofOutArgs.numCorners;

                EDMA_UTILS_memcpy2D(
                       pOutMetaBuf->bufAddr[0],
                       pChObj->pAlgSofOutBufTrackedPoints,
                       sizeof(strackInfo),
                       pChObj->algSofCreateParams.maxNumKeyPoints,
                       sizeof(strackInfo),
                       sizeof(strackInfo)
                       );

                pOutMetaBuf->flags = 0;

                Utils_updateLatency(&linkStatsInfo->linkLatency,
                                    pSysOutBuffer->linkLocalTimestamp);
                Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                    pSysOutBuffer->srcTimestamp);

                linkStatsInfo->linkStats.chStats
                            [chId].inBufProcessCount++;
                linkStatsInfo->linkStats.chStats
                            [chId].outBufCount[0]++;

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

Int32 AlgorithmLink_sparseOpticalFlowControl(void * pObj,
                                               void * pControlParams)
{
    AlgorithmLink_SparseOpticalFlowObj* pSOFObj;
    AlgorithmLink_ControlParams         * pAlgLinkControlPrm;

    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pSOFObj = (AlgorithmLink_SparseOpticalFlowObj *)
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
            AlgorithmLink_sparseOpticalFlowPrintStatistics(
                                                                pObj,
                                                                pSOFObj
                                                            );
            break;

        default:
            break;
    }

    return status;
}

Int32 AlgorithmLink_sparseOpticalFlowStop(void * pObj)
{
    return 0;
}

Int32 AlgorithmLink_sparseOpticalFlowDelete(void * pObj)
{

    Int32  status = SYSTEM_LINK_STATUS_SOK;
    UInt32 bufId, chId;
    AlgorithmLink_SparseOpticalFlowObj *pSOFObj;
    System_MetaDataBuffer              *pMetaDataBuffer;

    pSOFObj = (AlgorithmLink_SparseOpticalFlowObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    status = Utils_linkStatsCollectorDeAllocInst(pSOFObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    for(chId=0; chId<pSOFObj->outputQInfo.queInfo.numCh; chId++)
    {
        UTILS_assert(chId < SPARSEOPTICALFLOW_LINK_MAX_CH);
        /*
         * Free allocated memory for alg internal objects
         */
        AlgorithmLink_sparseOpticalFlowAlgDelete(&pSOFObj->chObj[chId]);

        /*
         * Free link buffers
         */
        for (bufId = 0; bufId < pSOFObj->algLinkCreateParams.numOutBuffers; bufId++)
        {
            pMetaDataBuffer     =   &pSOFObj->chObj[chId].metaDataBuffers[bufId];

            status = Utils_memFree(
                                    UTILS_HEAPID_DDR_CACHED_SR,
                                    pMetaDataBuffer->bufAddr[0],
                                    pMetaDataBuffer->metaBufSize[0]
                                   );
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            status = Utils_memFree(
                                    UTILS_HEAPID_DDR_CACHED_SR,
                                    pMetaDataBuffer->bufAddr[1],
                                    pMetaDataBuffer->metaBufSize[1]
                                   );
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    Utils_memFree(
                   UTILS_HEAPID_DDR_CACHED_LOCAL,
                   pSOFObj,
                   sizeof(AlgorithmLink_SparseOpticalFlowObj)
                );

    return status;
}

Int32 AlgorithmLink_sparseOpticalFlowPrintStatistics(void *pObj,
                AlgorithmLink_SparseOpticalFlowObj *pSOFObj)
{
    UTILS_assert(NULL != pSOFObj->linkStatsInfo);

    Utils_printLinkStatistics(&pSOFObj->linkStatsInfo->linkStats,
                            "ALG_SPARSE_OPTICAL_FLOW",
                            TRUE);

    Utils_printLatency("ALG_SPARSE_OPTICAL_FLOW",
                       &pSOFObj->linkStatsInfo->linkLatency,
                       &pSOFObj->linkStatsInfo->srcToLinkLatency,
                        TRUE);

    return SYSTEM_LINK_STATUS_SOK;
}

