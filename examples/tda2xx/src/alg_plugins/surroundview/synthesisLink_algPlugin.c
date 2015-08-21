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
 * \file synthesisLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for synthesis Link
 *
 * \version 0.0 (Oct 2013) : [PS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "synthesisLink_priv.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of synthesis algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_Synthesis_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_synthesisCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_synthesisProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_synthesisControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_synthesisStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_synthesisDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_SYNTHESIS;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for synthesis algorithm link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_synthesisCreate(void * pObj, void * pCreateParams)
{
    void                       * algHandle;
    Int32                        frameIdx;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       maxHeight;
    UInt32                       maxWidth;
    System_Buffer              * pSystemBuffer;
    System_VideoFrameBuffer    * pSystemVideoFrameBuffer;
    System_MetaDataBuffer      * pSystemMetaDataBuffer;
    System_MetaDataBuffer      * pBlendLUTBuffer = NULL;
    System_LinkInfo              prevLinkInfo;
    AlgorithmLink_SynthesisOutputQueId   outputQId;
    AlgorithmLink_SynthesisInputQueId    inputQId;
    Int32                        channelId;
    Int32                        numChannelsUsed;
    Int32                        numInputQUsed;
    Int32                        numOutputQUsed;
    UInt32                       prevLinkQueId;
    UInt32                       metaBufSize;
    UInt32                       dataFormat;
    System_LinkChInfo          * pOutChInfo;
    System_LinkChInfo          * pPrevChInfo;
    UInt32                       prevChInfoFlags;
    AlgLink_MemRequests          memRequests;

    AlgorithmLink_SynthesisObj           * pSynthesisObj;
    AlgorithmLink_SynthesisCreateParams  * pSynthesisLinkCreateParams;
    SV_Synthesis_CreationParamsStruct    * pAlgCreateParams;
    AlgorithmLink_OutputQueueInfo        * pOutputQInfo;
    AlgorithmLink_InputQueueInfo         * pInputQInfo;
    UInt32                                 memTabId;

    pSynthesisLinkCreateParams =
        (AlgorithmLink_SynthesisCreateParams *)pCreateParams;

    /*
     * Space for Algorithm specific object gets allocated here.
     * Pointer gets recorded in algorithmParams
     */
    if(sizeof(AlgorithmLink_SynthesisObj) > SV_ALGLINK_SRMEM_THRESHOLD)
    {
        pSynthesisObj = (AlgorithmLink_SynthesisObj *)
                        Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_SR,
                                       sizeof(AlgorithmLink_SynthesisObj), 32);
    }
    else
    {
        pSynthesisObj = (AlgorithmLink_SynthesisObj *)
                        malloc(sizeof(AlgorithmLink_SynthesisObj));
    }

    UTILS_assert(pSynthesisObj!=NULL);

    pAlgCreateParams = &pSynthesisObj->algCreateParams;

    pOutputQInfo = &pSynthesisObj->outputQInfo[0];
    pInputQInfo  = &pSynthesisObj->inputQInfo[0];

    AlgorithmLink_setAlgorithmParamsObj(pObj, pSynthesisObj);

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy((void*)(&pSynthesisObj->algLinkCreateParams),
           (void*)(pSynthesisLinkCreateParams),
           sizeof(AlgorithmLink_SynthesisCreateParams)
          );

    /*
     * Populating parameters corresponding to Q usage of synthesis
     * algorithm link
     */
    numInputQUsed     = ALGLINK_SYNTHESIS_IPQID_MAXIPQ;
    numOutputQUsed    = ALGLINK_SYNTHESIS_OPQID_MAXOPQ;
    numChannelsUsed   = 1;
    channelId         = 0;

    pInputQInfo[ALGLINK_SYNTHESIS_IPQID_MULTIVIEW].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pInputQInfo[ALGLINK_SYNTHESIS_IPQID_GALUT].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pInputQInfo[ALGLINK_SYNTHESIS_IPQID_PALUT].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    pOutputQInfo[ALGLINK_SYNTHESIS_OPQID_OPFRAME].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo[ALGLINK_SYNTHESIS_OPQID_PASTATS].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo[ALGLINK_SYNTHESIS_OPQID_SGXLUT].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    pOutputQInfo[ALGLINK_SYNTHESIS_OPQID_OPFRAME].queInfo.numCh = numChannelsUsed;
    pOutputQInfo[ALGLINK_SYNTHESIS_OPQID_PASTATS].queInfo.numCh = numChannelsUsed;
    pOutputQInfo[ALGLINK_SYNTHESIS_OPQID_SGXLUT].queInfo.numCh = numChannelsUsed;
    /*
     * Channel info population for output Q Id - ALGLINK_SYNTHESIS_OPQID_OPFRAME
     */
    outputQId = ALGLINK_SYNTHESIS_OPQID_OPFRAME;
    inputQId  = ALGLINK_SYNTHESIS_IPQID_MULTIVIEW;

    status = System_linkGetInfo(
                pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
                &prevLinkInfo);

    prevLinkQueId =
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId;
    pPrevChInfo   =
        &(prevLinkInfo.queInfo[prevLinkQueId].chInfo[channelId]);

    maxHeight = pSynthesisObj->algLinkCreateParams.maxOutputHeight;
    maxWidth  = pSynthesisObj->algLinkCreateParams.maxOutputWidth;

    UTILS_assert(maxWidth  <= SV_ALGLINK_OUTPUT_FRAME_WIDTH);
    UTILS_assert(maxHeight <= SV_ALGLINK_OUTPUT_FRAME_HEIGHT);

    /*
     * Make pitch a multiple of ALGORITHMLINK_FRAME_ALIGN, so that if the frame
     * origin is aligned, then individual lines are also aligned
     * Also note that the pitch is kept same independent of width of
     * individual channels
     */
    pSynthesisObj->outPitch[0] = maxWidth; //SystemUtils_align(SV_ALGLINK_OUTPUT_FRAME_WIDTH,
                                           //ALGORITHMLINK_FRAME_ALIGN);
    pSynthesisObj->outPitch[1] = pSynthesisObj->outPitch[0];
    pSynthesisObj->outPitch[2] = pSynthesisObj->outPitch[0];

    pOutChInfo = &(pOutputQInfo[outputQId].queInfo.chInfo[channelId]);
    pOutChInfo->startX = 0;
    pOutChInfo->startY = 0;
    pOutChInfo->width  = maxWidth;
    pOutChInfo->height = maxHeight;
    prevChInfoFlags    = pPrevChInfo->flags;
    pOutChInfo->flags  = prevChInfoFlags;

    dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(prevChInfoFlags);

    if(dataFormat != SYSTEM_DF_YUV420SP_UV)
    {
      UTILS_assert(NULL);
    }

    pSynthesisObj->dataFormat = dataFormat;
    pSynthesisObj->inPitch[0] = pPrevChInfo->pitch[0];
    pSynthesisObj->inPitch[1] = pPrevChInfo->pitch[1];

    if((pPrevChInfo->width > pSynthesisObj->algLinkCreateParams.maxInputWidth)
       ||
       (pPrevChInfo->height > pSynthesisObj->algLinkCreateParams.maxInputHeight)
      )
    {
      UTILS_assert(NULL);
    }

    pOutChInfo->pitch[0] = pSynthesisObj->outPitch[0];
    pOutChInfo->pitch[1] = pSynthesisObj->outPitch[1];
    pOutChInfo->pitch[2] = pSynthesisObj->outPitch[2];

    /*
     * Channel info population for output Q Id - ALGLINK_SYNTHESIS_OPQID_PASTATS
     */
    outputQId = ALGLINK_SYNTHESIS_OPQID_PASTATS;
    channelId = 0;
    pOutChInfo = &(pOutputQInfo[outputQId].queInfo.chInfo[channelId]);
    pOutChInfo->flags = 0;

    /*
     * Channel info population for output Q Id - ALGLINK_SYNTHESIS_OPQID_PASTATS
     */
    outputQId = ALGLINK_SYNTHESIS_OPQID_SGXLUT;
    channelId = 0;
    pOutChInfo = &(pOutputQInfo[outputQId].queInfo.chInfo[channelId]);
    pOutChInfo->flags = 0;

    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(pObj,
                                numInputQUsed,
                                pInputQInfo,
                                numOutputQUsed,
                                pOutputQInfo
                                );

    /*
     * Algorithm creation happens here
     * - Population of create time parameters
     * - Create call for algorithm
     * - Algorithm handle gets recorded inside link object
     */

    pAlgCreateParams->SVInCamFrmHeight =
        pSynthesisLinkCreateParams->maxInputHeight;
    pAlgCreateParams->SVInCamFrmWidth =
        pSynthesisLinkCreateParams->maxInputWidth;
    pAlgCreateParams->SVOutDisplayHeight =
        pSynthesisLinkCreateParams->maxOutputHeight;
    pAlgCreateParams->SVOutDisplayWidth =
        pSynthesisLinkCreateParams->maxOutputWidth;
    pAlgCreateParams->numCameras =
        pSynthesisLinkCreateParams->numViews;

    /*
     * Block sizes need to be powers of 2
     */
    pAlgCreateParams->blockSizeV       = 40;
    pAlgCreateParams->blockSizeH       = 40;
    if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_3D)
    {
       pAlgCreateParams->subsampleratio   = 2;
    }
    else
    {
       pAlgCreateParams->subsampleratio   = 4;
    }
    pAlgCreateParams->outputMode = pSynthesisLinkCreateParams->svOutputMode;
    pAlgCreateParams->enableCarOverlayInAlg = pSynthesisLinkCreateParams->enableCarOverlayInAlg;

    pAlgCreateParams->numColorChannels = 3;
    pAlgCreateParams->blendlen         = 20;
    pAlgCreateParams->seam_offset      = 0;//-100;
    //positive value to shift the seam horizontally and negative to shift it vertically
    pAlgCreateParams->svCarBoxParams.CarBoxCenter_x =
        (pAlgCreateParams->SVOutDisplayWidth / 2);
    pAlgCreateParams->svCarBoxParams.CarBoxCenter_y =
        (pAlgCreateParams->SVOutDisplayHeight / 2);
    pAlgCreateParams->svCarBoxParams.CarBox_height =
            pSynthesisLinkCreateParams->carBoxHeight;
    pAlgCreateParams->svCarBoxParams.CarBox_width =
            pSynthesisLinkCreateParams->carBoxWidth;

    /*
     * First time call is just to get size for algorithm handle.
     *
     * TBD - Currently since memquery function is dependent on alg handle
     * space, there are two calls - first for alg handle and then for other
     * requests. In future, once this dependency is removed, there will be
     * only call of MemQuery
     */
    Alg_SynthesisMemQuery(pAlgCreateParams, &memRequests, 1);
    memTabId = 0;
    memRequests.memTab[memTabId].basePtr = malloc(
                                            memRequests.memTab[memTabId].size);
    UTILS_assert(memRequests.memTab[memTabId].basePtr != NULL);

    /*
     * Memory allocations for the requests done by algorithm
     * For now treating all requests as persistent and allocating in DDR
     */
    Alg_SynthesisMemQuery(pAlgCreateParams, &memRequests, 0);
    for(memTabId = 1 ; memTabId < memRequests.numMemTabs ; memTabId++)
    {
        /*
         * TBD: Temporary allocation of L2 memory directly.
         */
        if(memRequests.memTab[memTabId].memLocation	== ALGORITHM_LINK_MEM_DSPL2)
        {
           memRequests.memTab[memTabId].basePtr = (void *)
                Utils_memAlloc(
                        UTILS_HEAPID_L2_LOCAL,
                        memRequests.memTab[memTabId].size,
                        memRequests.memTab[memTabId].alignment
                        );
        }
        else
        {
        if(memRequests.memTab[memTabId].size > 0)
        {

        if(memRequests.memTab[memTabId].size > SV_ALGLINK_SRMEM_THRESHOLD)
        {
            memRequests.memTab[memTabId].basePtr = Utils_memAlloc(
                                        UTILS_HEAPID_DDR_CACHED_SR,
                                        memRequests.memTab[memTabId].size,
                                        memRequests.memTab[memTabId].alignment);
        }
        else
        {
            memRequests.memTab[memTabId].basePtr =
                malloc(memRequests.memTab[memTabId].size);
        }
        UTILS_assert(memRequests.memTab[memTabId].basePtr != NULL);

        }
    }

    }

    /*
     * Creation of output buffers for output Q of ALGLINK_SYNTHESIS_OPQID_OPFRAME
     *  - Connecting video frame buffer to system buffer payload
     *  - Memory allocation for Luma and Chroma buffers (Assume 420 SP format)
     *  - Put the buffer into empty queue
     */
    /* Set the second output Q only incase of 2D SRV mode */
    if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
    {
        outputQId = ALGLINK_SYNTHESIS_OPQID_OPFRAME;
        channelId = 0;

        for(frameIdx = 0;
            frameIdx < pSynthesisObj->algLinkCreateParams.numOutputFrames;
            frameIdx++)
        {
            pSystemBuffer           =
                                 &(pSynthesisObj->buffers[outputQId][frameIdx]);
            pSystemVideoFrameBuffer =
                                 &(pSynthesisObj->videoFrames[frameIdx]);

            /*
             * Properties of pSystemBuffer, which do not get altered during
             * run time (frame exchanges) are initialized here
             */
            pSystemBuffer->payload     = pSystemVideoFrameBuffer;
            pSystemBuffer->payloadSize = sizeof(System_VideoFrameBuffer);
            pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
            pSystemBuffer->chNum       = channelId;

            memcpy((void *)&pSystemVideoFrameBuffer->chInfo,
                   (void *)&pOutputQInfo[outputQId].queInfo.chInfo[channelId],
                   sizeof(System_LinkChInfo));

            /*
             * Buffer allocation done for maxHeight, maxWidth and for 420SP format
             */
            pSystemVideoFrameBuffer->bufAddr[0] = Utils_memAlloc(
                                           UTILS_HEAPID_DDR_CACHED_SR,
                                           (SV_ALGLINK_OUTPUT_FRAME_HEIGHT*
                                           (pSynthesisObj->outPitch[0])*
                                           (1.5)),
                                           ALGORITHMLINK_FRAME_ALIGN);

            /*
             * Carving out memory pointer for chroma which will get used in case of
             * SYSTEM_DF_YUV420SP_UV
             */
            pSystemVideoFrameBuffer->bufAddr[1] = (void*)(
                (UInt32) pSystemVideoFrameBuffer->bufAddr[0] +
                (UInt32)(maxHeight*(pSynthesisObj->outPitch[0]))
                );

            UTILS_assert(pSystemVideoFrameBuffer->bufAddr[0] != NULL);

            AlgorithmLink_putEmptyOutputBuffer(pObj, outputQId, pSystemBuffer);
        }
    }

    /*
     * For first frame synthesis PALUT will not be available from PA Link.
     * So a temporary buffer space is allocated just for first frame synthesis
     */
    //metaBufSize = SV_ALGLINK_PALUT_SIZE;
    metaBufSize = sizeof(PAlignOutStruct);

    pSynthesisObj->pLinkStaticPALUT = (UInt8 *) Utils_memAlloc(
                                                    UTILS_HEAPID_DDR_CACHED_SR,
                                                    metaBufSize,
                                                    ALGORITHMLINK_FRAME_ALIGN);

    UTILS_assert(pSynthesisObj->pLinkStaticPALUT != NULL);

    /*
     * Creation of output buffers for output Q of ALGLINK_SYNTHESIS_OPQID_PASTATS
     *  - Connecting metadata buffer to system buffer payload
     *  - Memory allocation for buffers
     *  - Put the buffer into empty queue
     */
    outputQId = ALGLINK_SYNTHESIS_OPQID_PASTATS;
    channelId = 0;

    for(frameIdx = 0;
        frameIdx < pSynthesisObj->algLinkCreateParams
                    .numPhotometricStatisticsTables;
        frameIdx++)
    {
        pSystemBuffer         =
                             &(pSynthesisObj->buffers[outputQId][frameIdx]);
        pSystemMetaDataBuffer =
                             &(pSynthesisObj->photoAlignStats[frameIdx]);

        /*
         * Properties of pSystemBuffer, which do not get altered during
         * run time (frame exchanges) are initialized here
         */
        pSystemBuffer->payload     = pSystemMetaDataBuffer;
        pSystemBuffer->payloadSize = sizeof(System_MetaDataBuffer);
        pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_METADATA;
        pSystemBuffer->chNum       = channelId;

        /*
         * Buffer allocation done considering following factors -
         *  - Sub sampled height
         *  - Sub sampled width
         *  - Number of planes (3)
         *  - Number of non-overlapping view pairs
         *    (Ex: For 4 views case, 1-3 and 2-4 are non overlapping view pairs)
         * An additional 256 bytes for any round off etc during division etc..
         */
        metaBufSize =
            (SV_ALGLINK_OUTPUT_FRAME_HEIGHT / pAlgCreateParams->blockSizeV) *
            (SV_ALGLINK_OUTPUT_FRAME_WIDTH / pAlgCreateParams->blockSizeH) *
            pAlgCreateParams->numColorChannels *
            PAlignStat_BitPerEntry *
            2
            + 256
            ;

        //metaBufSize =
        //            (SV_ALGLINK_OUTPUT_FRAME_HEIGHT / pAlgCreateParams->blockSizeV) *
        //            (SV_ALGLINK_OUTPUT_FRAME_WIDTH / pAlgCreateParams->blockSizeH) *
        //            3 *
        //            (SV_ALGLINK_MAX_NUM_VIEWS/2)
        //            + 256
        //            ;

        pSystemMetaDataBuffer->numMetaDataPlanes = 1;
        pSystemMetaDataBuffer->bufAddr[0] =  Utils_memAlloc(
                                                UTILS_HEAPID_DDR_CACHED_SR,
                                                metaBufSize,
                                                ALGORITHMLINK_FRAME_ALIGN);
        pSystemMetaDataBuffer->metaBufSize[0]    = metaBufSize;
        pSystemMetaDataBuffer->metaFillLength[0] = metaBufSize;
        pSystemMetaDataBuffer->flags             = 0;

        UTILS_assert(pSystemMetaDataBuffer->bufAddr[0] != NULL);

        AlgorithmLink_putEmptyOutputBuffer(pObj, outputQId, pSystemBuffer);
    }

    /*
     * Creation of output buffers for output Q of ALGLINK_SYNTHESIS_OPQID_SGXLUT
     *  - Connecting metadata buffer to system buffer payload
     *  - Memory allocation for buffers
     *  - Put the buffer into empty queue
     */

    outputQId = ALGLINK_SYNTHESIS_OPQID_SGXLUT;
    channelId = 0;

    for(frameIdx = 0;
        frameIdx < pSynthesisObj->algLinkCreateParams
                    .numSgxBlendLUTables;
        frameIdx++)
    {
        pSystemBuffer         =
                             &(pSynthesisObj->buffers[outputQId][frameIdx]);
        pSystemMetaDataBuffer =
                             &(pSynthesisObj->opSgxBlendLUT[frameIdx]);

        /*
         * Properties of pSystemBuffer, which do not get altered during
         * run time (frame exchanges) are initialized here
         */
        pSystemBuffer->payload     = pSystemMetaDataBuffer;
        pSystemBuffer->payloadSize = sizeof(System_MetaDataBuffer);
        pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_METADATA;
        pSystemBuffer->chNum       = channelId;

        /*
         * Buffer allocation done considering following factors -
         *  - Sub sampled height
         *  - Sub sampled width
         *  - Number of planes (3)
         *  - Number of non-overlapping view pairs
         *    (Ex: For 4 views case, 1-3 and 2-4 are non overlapping view pairs)
         * An additional 256 bytes for any round off etc during division etc..
         */
        metaBufSize = (SV_ALGLINK_OUTPUT_FRAME_HEIGHT * \
                       SV_ALGLINK_OUTPUT_FRAME_WIDTH * 4 );

        pSystemMetaDataBuffer->numMetaDataPlanes = 1;
        pSystemMetaDataBuffer->bufAddr[0] =  Utils_memAlloc(
                                                UTILS_HEAPID_DDR_CACHED_SR,
                                                metaBufSize,
                                                ALGORITHMLINK_FRAME_ALIGN);
        pSystemMetaDataBuffer->metaBufSize[0]    = metaBufSize;
        pSystemMetaDataBuffer->metaFillLength[0] = metaBufSize;
        pSystemMetaDataBuffer->flags             = 0;

        UTILS_assert(pSystemMetaDataBuffer->bufAddr[0] != NULL);

        if (frameIdx == 0)
        {
            pSynthesisObj->sysBufferBlendLUT = pSystemBuffer;
            pBlendLUTBuffer = (System_MetaDataBuffer*) pSystemBuffer->payload;
        }

        /* Put in the Empty output Q only incase of 3D SRV mode */
        if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_3D)
        {
            AlgorithmLink_putEmptyOutputBuffer(pObj, outputQId, pSystemBuffer);
        }
    }

    UTILS_assert(pBlendLUTBuffer != NULL);
    algHandle = Alg_SynthesisCreate(pAlgCreateParams, &memRequests,
                                    pBlendLUTBuffer->bufAddr[0]);
    UTILS_assert(algHandle != NULL);

    pSynthesisObj->algHandle = algHandle;

    Cache_wb(pBlendLUTBuffer->bufAddr[0],
             pBlendLUTBuffer->metaBufSize[0],
             Cache_Type_ALLD,
             TRUE);

    /*
     * Creation of local input Qs for ALGLINK_SYNTHESIS_IPQID_MULTIVIEW and
     * ALGLINK_SYNTHESIS_IPQID_PALUT.
     * For ALGLINK_SYNTHESIS_IPQID_GALUT, always just one entry is kept.
     */
    inputQId = ALGLINK_SYNTHESIS_IPQID_MULTIVIEW;
    status  = Utils_queCreate(&(pSynthesisObj->localInputQ[inputQId].queHandle),
                               SYNTHESIS_LINK_MAX_LOCALQUEUELENGTH,
                              (pSynthesisObj->localInputQ[inputQId].queMem),
                               UTILS_QUE_FLAG_NO_BLOCK_QUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = ALGLINK_SYNTHESIS_IPQID_PALUT;
    status  = Utils_queCreate(&(pSynthesisObj->localInputQ[inputQId].queHandle),
                               SYNTHESIS_LINK_MAX_LOCALQUEUELENGTH,
                              (pSynthesisObj->localInputQ[inputQId].queMem),
                               UTILS_QUE_FLAG_NO_BLOCK_QUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pSynthesisObj->frameDropCounter          = 0;

    pSynthesisObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj),"ALG_SYNTHESIS");
    UTILS_assert(NULL != pSynthesisObj->linkStatsInfo);

    pSynthesisObj->numInputChannels = 1;

    pSynthesisObj->isFirstFrameRecv   = FALSE;
    pSynthesisObj->receivedGALUTFlag  = FALSE;
    pSynthesisObj->isFirstOPGenerated = FALSE;
    pSynthesisObj->isSGXBlendLUTOPGenerated = FALSE;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process Plugin for synthesis algorithm link
 *
 *        This function and the algorithm process function execute
 *        on same processor core. Hence processor gets
 *        locked with execution of the function, until completion. Only a
 *        link with higher priority can pre-empt this function execution.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_synthesisProcess(void * pObj)
{
    AlgorithmLink_SynthesisObj * pSynthesisObj;
    void                       * algHandle;
    AlgorithmLink_SynthesisInputQueId    inputQId;
    AlgorithmLink_SynthesisOutputQueId   outputQId;
    UInt32                       channelId = 0;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       bufId;
    UInt32                       viewId;
    System_BufferList            inputBufList;
    System_BufferList            inputBufListReturn;
    System_BufferList            outputBufListReturn;
    System_Buffer              * pSysBufferInput;
    System_Buffer              * pSystemBufferMultiview;
    System_Buffer              * pSystemBufferPALUT;
    System_Buffer              * pSysBufferPAStats;
    System_Buffer              * pSysBufferOutput;
    System_Buffer              * pSysBufferBlendLUT;
    Bool                         bufDropFlag;
    Bool                         isProcessCallDoneFlag;
    System_MetaDataBuffer      * pPAStatsBuffer;
    System_MetaDataBuffer      * pPALUTBuffer;
    System_MetaDataBuffer      * pGALUTBuffer;
    System_MetaDataBuffer      * pBlendLUTBuffer;
    System_VideoFrameBuffer    * pVideoOutputBuffer;
    void                       * inPAlignLUTPtr;
    PAlignOutStruct            * PAlignOutStruct_ptr;
    Uint32                       i,j;

    System_VideoFrameCompositeBuffer     * pCompositeBuffer;
    AlgorithmLink_SynthesisCreateParams  * pSynthesisLinkCreateParams;
    System_LinkStatistics      * linkStatsInfo;

    pSynthesisObj = (AlgorithmLink_SynthesisObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    linkStatsInfo = pSynthesisObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    algHandle                  = pSynthesisObj->algHandle;
    pSynthesisLinkCreateParams = (AlgorithmLink_SynthesisCreateParams *)
                                    &pSynthesisObj->algLinkCreateParams;

    /*
     * Get Input buffers from previous link for
     * Qid = ALGLINK_SYNTHESIS_IPQID_MULTIVIEW and queue them up locally.
     */
    inputQId = ALGLINK_SYNTHESIS_IPQID_MULTIVIEW;

    System_getLinksFullBuffers(
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysBufferInput = inputBufList.buffers[bufId];

            /*TBD: Check for parameter correctness. If in error, return input*/

            if (pSysBufferInput != NULL)
            {
                status = Utils_quePut(
                            &(pSynthesisObj->localInputQ[inputQId].queHandle),
                            pSysBufferInput,
                            BSP_OSAL_NO_WAIT);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = ALGLINK_SYNTHESIS_IPQID_PALUT and queue them up locally.
     */
    /* Set the second output Q only incase of 2D SRV mode */
    if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
    {
        inputQId = ALGLINK_SYNTHESIS_IPQID_PALUT;

        System_getLinksFullBuffers(
            pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
            pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
            &inputBufList);

        if(inputBufList.numBuf)
        {
            for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
            {
                pSysBufferInput = inputBufList.buffers[bufId];
                /*TBD: Check for parameter correctness. If in error, return input*/
                status = Utils_quePut(
                            &(pSynthesisObj->localInputQ[inputQId].queHandle),
                            pSysBufferInput,
                            BSP_OSAL_NO_WAIT);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = ALGLINK_SYNTHESIS_IPQID_GALUT and store latest copy locally.
     */
    inputQId = ALGLINK_SYNTHESIS_IPQID_GALUT;

    System_getLinksFullBuffers(
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            /*
             * At any point in time, Synthesis link will hold only one GA LUT.
             * So whenever GA LUT is received, the previously received one
             * will be released and the newly received one will be archived.
             */
            if(pSynthesisObj->receivedGALUTFlag == TRUE)
            {
                inputBufListReturn.numBuf     = 1;
                inputBufListReturn.buffers[0] = pSynthesisObj->sysBufferGALUT;
                bufDropFlag = FALSE;

                AlgorithmLink_releaseInputBuffer(
                    pObj,
                    inputQId,
                    pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
                    pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
                    &inputBufListReturn,
                    &bufDropFlag);

                pSynthesisObj->receivedGALUTFlag = FALSE;
            }

            pSynthesisObj->sysBufferGALUT = inputBufList.buffers[bufId];
            /*TBD: Check for parameter correctness. If in error, return input*/
            pSynthesisObj->receivedGALUTFlag = TRUE;
        }
    }


    if(pSynthesisObj->isFirstFrameRecv==FALSE)
    {
        pSynthesisObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(
            &linkStatsInfo->linkStats,
            pSynthesisObj->numInputChannels,
            1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
        //Task_sleep(100);
    }

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    /*
     * Continous loop to perform synthesis as long as input and output
     * buffers are available. Exit this loop, when, if for some reason
     * the alg process call is not done (tracked via isProcessCallDoneFlag).
     */
    while(1)
    {

    isProcessCallDoneFlag = FALSE;
    /*
     * Checking if all the inputs are available. For first frame, PALUT is
     * not expected to come. This is tracked via isFirstOPGenerated.
     */
    if(pSynthesisObj->receivedGALUTFlag == TRUE
            &&
      Utils_queGetQueuedCount(
      &(pSynthesisObj->localInputQ[ALGLINK_SYNTHESIS_IPQID_MULTIVIEW].queHandle))>0
            &&
      ((Utils_queGetQueuedCount(
      &(pSynthesisObj->localInputQ[ALGLINK_SYNTHESIS_IPQID_PALUT].queHandle))>0
       ||
       (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_3D))
       ||
       pSynthesisObj->isFirstOPGenerated == FALSE)
     )
     {

      pVideoOutputBuffer = NULL;
      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
        outputQId = ALGLINK_SYNTHESIS_OPQID_OPFRAME;
        channelId = 0;
        status = AlgorithmLink_getEmptyOutputBuffer(pObj,
                                                    outputQId,
                                                    channelId,
                                                    &pSysBufferOutput);

        if(status != SYSTEM_LINK_STATUS_SOK)
        {
          linkStatsInfo->linkStats.outBufErrorCount++;
          break;
        }
        UTILS_assert(pSysBufferOutput != NULL);
        pVideoOutputBuffer = (System_VideoFrameBuffer*)pSysBufferOutput->payload;
      }

      outputQId = ALGLINK_SYNTHESIS_OPQID_PASTATS;
      channelId = 0;
      status = AlgorithmLink_getEmptyOutputBuffer(pObj,
                                                  outputQId,
                                                  channelId,
                                                  &pSysBufferPAStats);

      if(status != SYSTEM_LINK_STATUS_SOK)
      {
        if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
        {
            AlgorithmLink_putEmptyOutputBuffer(pObj,
                                           ALGLINK_SYNTHESIS_OPQID_OPFRAME,
                                           pSysBufferOutput);
        }
        linkStatsInfo->linkStats.outBufErrorCount++;
        break;
      }

      UTILS_assert(pSysBufferPAStats != NULL);
      pPAStatsBuffer = (System_MetaDataBuffer*)pSysBufferPAStats->payload;

      /*
       * Reaching here means output buffers are available.
       * Hence getting inputs from local Queus
       */
      status = Utils_queGet(
                &(pSynthesisObj->localInputQ[ALGLINK_SYNTHESIS_IPQID_MULTIVIEW].
                    queHandle),
                (Ptr *)&pSystemBufferMultiview,
                1,
                BSP_OSAL_NO_WAIT);

      UTILS_assert(pSystemBufferMultiview != NULL);

      pCompositeBuffer = (System_VideoFrameCompositeBuffer *)
                            (pSystemBufferMultiview->payload);

      inPAlignLUTPtr = NULL;
      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
        if(pSynthesisObj->isFirstOPGenerated == TRUE)
        {
          status = Utils_queGet(
                      &(pSynthesisObj->localInputQ[ALGLINK_SYNTHESIS_IPQID_PALUT].
                          queHandle),
                      (Ptr *)&pSystemBufferPALUT,
                      1,
                      BSP_OSAL_NO_WAIT);
          pPALUTBuffer = (System_MetaDataBuffer *)(pSystemBufferPALUT->payload);
          inPAlignLUTPtr = pPALUTBuffer->bufAddr[0];
        }
        else
        {
           /*
            * For first frame case temporary memory buffer is provided.
            * TBD - PA LUT buffer is uninitialized.
            * Currently nothing is initialized since only first frame looks
            * corrupted.
            */
           PAlignOutStruct_ptr = (PAlignOutStruct *) pSynthesisObj->pLinkStaticPALUT;

           //Initialization
           for(i=0;i<(MAX_NUM_VIEWS*NUM_MAX_COLORPLANES);i++){
               PAlignOutStruct_ptr->PAlignOut_Gain[i]=256;
           }
           for(i=0;i<(MAX_NUM_VIEWS*NUM_MAX_COLORPLANES);i++){
               for(j=0;j<256;j++){
                   PAlignOutStruct_ptr->PAlignOut_LUT[i*256+j]=j;
               }
           }
           inPAlignLUTPtr = (UInt32 *) PAlignOutStruct_ptr;
        }
      }

      pGALUTBuffer = (System_MetaDataBuffer *)pSynthesisObj->
                        sysBufferGALUT->payload;

      for(viewId = 0; viewId < SV_ALGLINK_MAX_NUM_VIEWS ;viewId++)
      {
        Cache_inv(pCompositeBuffer->bufAddr[0][viewId],
         (SV_ALGLINK_INPUT_FRAME_WIDTH*SV_ALGLINK_INPUT_FRAME_HEIGHT),
         Cache_Type_ALLD, TRUE);
        Cache_inv(pCompositeBuffer->bufAddr[1][viewId],
         ((SV_ALGLINK_INPUT_FRAME_WIDTH*SV_ALGLINK_INPUT_FRAME_HEIGHT)/2),
         Cache_Type_ALLD, TRUE);
      }

      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
          pSysBufferOutput->srcTimestamp = pSystemBufferMultiview->srcTimestamp;
          pSysBufferOutput->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();
      }

      pSysBufferPAStats->srcTimestamp = pSystemBufferMultiview->srcTimestamp;
      pSysBufferPAStats->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_3D)
      {
          pSynthesisObj->isSGXBlendLUTOPGenerated = FALSE;
          outputQId = ALGLINK_SYNTHESIS_OPQID_SGXLUT;
          channelId = 0;
          status = AlgorithmLink_getEmptyOutputBuffer(pObj,
                                                      outputQId,
                                                      channelId,
                                                      &pSysBufferBlendLUT);

          if ((status == SYSTEM_LINK_STATUS_SOK) && (pSysBufferBlendLUT != NULL))
          {
              pSynthesisObj->sysBufferBlendLUT = pSysBufferBlendLUT;
              pSynthesisObj->isSGXBlendLUTOPGenerated = TRUE;

              pSysBufferBlendLUT->srcTimestamp = pSystemBufferMultiview->srcTimestamp;
              pSysBufferBlendLUT->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();
          }
      }

      pSysBufferBlendLUT = pSynthesisObj->sysBufferBlendLUT;
      pBlendLUTBuffer = (System_MetaDataBuffer*) pSysBufferBlendLUT->payload;
      UTILS_assert(pBlendLUTBuffer != NULL);

      /* TBD - Doubt on passing composite buffer */
      status = Alg_SynthesisProcess(
                           algHandle,
                           pCompositeBuffer,
                           pVideoOutputBuffer,
                           &pSynthesisObj->inPitch[0],
                           &pSynthesisObj->outPitch[0],
                           pGALUTBuffer->bufAddr[0],
                           inPAlignLUTPtr,
                           pPAStatsBuffer->bufAddr[0],
                           pBlendLUTBuffer->bufAddr[0],
                           pSynthesisObj->dataFormat,
                           pSynthesisLinkCreateParams->synthesisMode
                           );

      UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

      /*
       * Putting filled buffer into output full buffer for
       * outputQId = ALGLINK_SYNTHESIS_OPQID_SGXLUT
       */
      if ((pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_3D)
          && (pSynthesisObj->isSGXBlendLUTOPGenerated == TRUE))
      {
          Cache_wb(pBlendLUTBuffer->bufAddr[0],
                   pBlendLUTBuffer->metaBufSize[0],
                   Cache_Type_ALLD,
                   TRUE);

          Utils_updateLatency(&linkStatsInfo->linkLatency,
                              pSysBufferBlendLUT->linkLocalTimestamp);
          Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                              pSysBufferBlendLUT->srcTimestamp);

          outputQId = ALGLINK_SYNTHESIS_OPQID_SGXLUT;
          status    = AlgorithmLink_putFullOutputBuffer(pObj,
                                                        outputQId,
                                                        pSysBufferBlendLUT);

          UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

          /*
           * Informing next link that a new data has peen put for its
           * processing
           */
          System_sendLinkCmd(
            pSynthesisLinkCreateParams->outQueParams[outputQId].nextLink,
            SYSTEM_CMD_NEW_DATA,
            NULL);

          outputQId                      = ALGLINK_SYNTHESIS_OPQID_SGXLUT;
          outputBufListReturn.numBuf     = 1;
          outputBufListReturn.buffers[0] = pSysBufferBlendLUT;
          AlgorithmLink_releaseOutputBuffer(pObj,
                                            outputQId,
                                            &outputBufListReturn);
      }

      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
          Cache_wb(pVideoOutputBuffer->bufAddr[0],
                   (SV_ALGLINK_OUTPUT_FRAME_WIDTH*SV_ALGLINK_OUTPUT_FRAME_HEIGHT),
                   Cache_Type_ALLD,
                   TRUE);
          Cache_wb(pVideoOutputBuffer->bufAddr[1],
                   (SV_ALGLINK_OUTPUT_FRAME_WIDTH*SV_ALGLINK_OUTPUT_FRAME_HEIGHT)/2,
                   Cache_Type_ALLD,
                   TRUE);

          Utils_updateLatency(&linkStatsInfo->linkLatency,
                              pSysBufferOutput->linkLocalTimestamp);
          Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                pSysBufferOutput->srcTimestamp);
      }

      Cache_wb(pPAStatsBuffer->bufAddr[0],
               pPAStatsBuffer->metaBufSize[0],
               Cache_Type_ALLD,
               TRUE);

      Utils_updateLatency(&linkStatsInfo->linkLatency,
                          pSysBufferPAStats->linkLocalTimestamp);
      Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                          pSysBufferPAStats->srcTimestamp);

      isProcessCallDoneFlag = TRUE;

      if (pSystemBufferMultiview != NULL)
      {
        linkStatsInfo->linkStats.chStats
                    [pSystemBufferMultiview->chNum].inBufProcessCount++;
        linkStatsInfo->linkStats.chStats
                    [pSystemBufferMultiview->chNum].outBufCount[0]++;
      }

      /*
       * Putting filled buffer into output full buffer for
       * outputQId = ALGLINK_SYNTHESIS_OPQID_OPFRAME
       */
      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
          outputQId = ALGLINK_SYNTHESIS_OPQID_OPFRAME;
          status    = AlgorithmLink_putFullOutputBuffer(pObj,
                                                        outputQId,
                                                        pSysBufferOutput);

          UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

          /*
           * Informing next link that a new data has peen put for its
           * processing
           */
          System_sendLinkCmd(
            pSynthesisLinkCreateParams->outQueParams[outputQId].nextLink,
            SYSTEM_CMD_NEW_DATA,
            NULL);
      }
      /*
       * Putting filled buffer into output full buffer for
       * outputQId = ALGLINK_SYNTHESIS_OPQID_PASTATS
       */
      outputQId = ALGLINK_SYNTHESIS_OPQID_PASTATS;
      status    = AlgorithmLink_putFullOutputBuffer(pObj,
                                                    outputQId,
                                                    pSysBufferPAStats);

      UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

      /*
       * Informing next link that a new data has peen put for its
       * processing
       */
      System_sendLinkCmd(
        pSynthesisLinkCreateParams->outQueParams[outputQId].nextLink,
        SYSTEM_CMD_NEW_DATA,
        NULL);

      /*
       * Releasing (Free'ing) output buffers, since algorithm does not need
       * it for any future usage.
       */
      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
          outputQId                      = ALGLINK_SYNTHESIS_OPQID_OPFRAME;
          outputBufListReturn.numBuf     = 1;
          outputBufListReturn.buffers[0] = pSysBufferOutput;
          AlgorithmLink_releaseOutputBuffer(pObj,
                                            outputQId,
                                            &outputBufListReturn);
      }

      outputQId                      = ALGLINK_SYNTHESIS_OPQID_PASTATS;
      outputBufListReturn.numBuf     = 1;
      outputBufListReturn.buffers[0] = pSysBufferPAStats;
      AlgorithmLink_releaseOutputBuffer(pObj,
                                        outputQId,
                                        &outputBufListReturn);

      /*
       * Releasing (Free'ing) Input buffers, since algorithm does not need
       * it for any future usage.
       */
      bufDropFlag = FALSE;

      inputQId                      = ALGLINK_SYNTHESIS_IPQID_MULTIVIEW;
      inputBufListReturn.numBuf     = 1;
      inputBufListReturn.buffers[0] = pSystemBufferMultiview;
      AlgorithmLink_releaseInputBuffer(
        pObj,
        inputQId,
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
        pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
        &inputBufListReturn,
        &bufDropFlag);

      if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
      {
          if(pSynthesisObj->isFirstOPGenerated == TRUE)
          {
            inputQId                      = ALGLINK_SYNTHESIS_IPQID_PALUT;
            inputBufListReturn.numBuf     = 1;
            inputBufListReturn.buffers[0] = pSystemBufferPALUT;
            AlgorithmLink_releaseInputBuffer(
                pObj,
                inputQId,
                pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkId,
                pSynthesisLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
                &inputBufListReturn,
                &bufDropFlag);
          }

          pSynthesisObj->isFirstOPGenerated = TRUE;
      }
    }

    if(isProcessCallDoneFlag == FALSE)
        break;
            /* TBD - Take care of error stats

            pSynthesisObj->linkStats.inBufErrorCount++;
            pSynthesisObj->linkStats.chStats
                            [pSysBufferInput->chNum].inBufDropCount++;
            pSynthesisObj->linkStats.chStats
                            [pSysBufferInput->chNum].outBufDropCount[0]++;
            */

    } /* End of while(1) */

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Control Plugin for synthesis algorithm link
 *
 *
 * \param  pObj               [IN] Algorithm link object handle
 * \param  pControlParams     [IN] Pointer to control parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

Int32 AlgorithmLink_synthesisControl(void * pObj, void * pControlParams)
{
    AlgorithmLink_SynthesisObj     * pSynthesisObj;
    AlgorithmLink_ControlParams    * pAlgLinkControlPrm;
    void                           * algHandle;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pSynthesisObj = (AlgorithmLink_SynthesisObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);
    algHandle     = pSynthesisObj->algHandle;

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
            AlgorithmLink_synthesisPrintStatistics(pObj, pSynthesisObj);
            break;

        default:
            status = Alg_SynthesisControl(algHandle,
                                          &(pSynthesisObj->controlParams)
                                          );
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop Plugin for synthesis algorithm link
 *
 *        For this algorithm there is no locking of frames and hence no
 *        flushing of frames. Also there are no any other functionality to be
 *        done at the end of execution of this algorithm. Hence this function
 *        is an empty function for this algorithm.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_synthesisStop(void * pObj)
{
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete Plugin for synthesis algorithm link
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_synthesisDelete(void * pObj)
{
    Int32                        frameIdx;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       maxHeight;
    UInt32                       metaBufSize;
    System_VideoFrameBuffer    * pSystemVideoFrameBuffer;
    System_MetaDataBuffer      * pSystemMetaDataBuffer;
    AlgorithmLink_SynthesisInputQueId    inputQId;
    AlgLink_MemRequests          memRequests;

    AlgorithmLink_SynthesisObj           * pSynthesisObj;
    AlgorithmLink_SynthesisCreateParams  * pSynthesisLinkCreateParams;
    SV_Synthesis_CreationParamsStruct    * pAlgCreateParams;
    UInt32                                 memTabId;

    pSynthesisObj = (AlgorithmLink_SynthesisObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    status = Utils_linkStatsCollectorDeAllocInst(pSynthesisObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pSynthesisLinkCreateParams = &pSynthesisObj->algLinkCreateParams;
    pAlgCreateParams = (&pSynthesisObj->algCreateParams);

    maxHeight = pSynthesisLinkCreateParams->maxOutputHeight;

    Alg_SynthesisDelete(pSynthesisObj->algHandle, &memRequests);

    /*
     * Memory allocations for the requests done by algorithm
     */
    for(memTabId = 0 ; memTabId < memRequests.numMemTabs ; memTabId++)
    {

        if(memRequests.memTab[memTabId].memLocation	== ALGORITHM_LINK_MEM_DSPL2)
        {
            Utils_memFree(
                    UTILS_HEAPID_L2_LOCAL,
                    memRequests.memTab[memTabId].basePtr,
                    memRequests.memTab[memTabId].size
                    );
        }
        if(memRequests.memTab[memTabId].memLocation	!= ALGORITHM_LINK_MEM_DSPL2)
        {

        if(memRequests.memTab[memTabId].size > 0)
        {
        if(memRequests.memTab[memTabId].size > SV_ALGLINK_SRMEM_THRESHOLD)
        {
            status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                                   memRequests.memTab[memTabId].basePtr,
                                   memRequests.memTab[memTabId].size);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
        else
        {
            free(memRequests.memTab[memTabId].basePtr);
        }
        }

        }

    }

    /*
     * Deletion of output buffers for output Q of ALGLINK_SYNTHESIS_OPQID_OPFRAME
     */
    /* Set the second output Q only incase of 2D SRV mode */
    if (pSynthesisLinkCreateParams->svOutputMode == ALGORITHM_LINK_SRV_OUTPUT_2D)
    {
        for(frameIdx = 0;
            frameIdx < pSynthesisObj->algLinkCreateParams.numOutputFrames;
            frameIdx++)
        {
            pSystemVideoFrameBuffer =
                                 &(pSynthesisObj->videoFrames[frameIdx]);

            /*
             * Buffer allocation done for maxHeight, outPitch and for 420SP format
             */
            status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                                   pSystemVideoFrameBuffer->bufAddr[0],
                                  (maxHeight*(pSynthesisObj->outPitch[0])*(1.5)));

            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        }
    }

    /*
     * Deletion of output buffers for output Q of ALGLINK_SYNTHESIS_OPQID_PASTATS
     */
    for(frameIdx = 0;
        frameIdx < pSynthesisObj->algLinkCreateParams
                    .numPhotometricStatisticsTables;
        frameIdx++)
    {
        pSystemMetaDataBuffer =
                             &(pSynthesisObj->photoAlignStats[frameIdx]);

        /*
         * Buffer allocation done considering following factors -
         *  - Sub sampled height
         *  - Sub sampled width
         *  - Number of planes (3)
         *  - Number of non-overlapping view pairs
         *    (Ex: For 4 views case, 1-3 and 2-4 are non overlapping view pairs)
         * An additional 256 bytes for any round off etc during division etc..
         */
        metaBufSize =
            (SV_ALGLINK_OUTPUT_FRAME_HEIGHT / pAlgCreateParams->blockSizeV) *
            (SV_ALGLINK_OUTPUT_FRAME_WIDTH / pAlgCreateParams->blockSizeH) *
            pAlgCreateParams->numColorChannels *
            PAlignStat_BitPerEntry *
            2
            + 256
            ;

        status =  Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                                pSystemMetaDataBuffer->bufAddr[0],
                                metaBufSize);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    /*
     * Deletion of output buffers for output Q of ALGLINK_SYNTHESIS_OPQID_PASTATS
     */
    for(frameIdx = 0;
        frameIdx < pSynthesisObj->algLinkCreateParams
                    .numSgxBlendLUTables;
        frameIdx++)
    {
        pSystemMetaDataBuffer = &(pSynthesisObj->opSgxBlendLUT[frameIdx]);

        /*
         * Buffer allocation done considering following factors -
         *  - Sub sampled height
         *  - Sub sampled width
         *  - Number of planes (3)
         *  - Number of non-overlapping view pairs
         *    (Ex: For 4 views case, 1-3 and 2-4 are non overlapping view pairs)
         * An additional 256 bytes for any round off etc during division etc..
         */
        metaBufSize = (SV_ALGLINK_OUTPUT_FRAME_HEIGHT * \
                       SV_ALGLINK_OUTPUT_FRAME_WIDTH * 4 );

        status =  Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                                pSystemMetaDataBuffer->bufAddr[0],
                                metaBufSize);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    /*
     * Deletion of local input Qs for ALGLINK_SYNTHESIS_IPQID_MULTIVIEW and
     * ALGLINK_SYNTHESIS_IPQID_PALUT.
     * For ALGLINK_SYNTHESIS_IPQID_GALUT, always just one entry is kept.
     */
    inputQId = ALGLINK_SYNTHESIS_IPQID_MULTIVIEW;
    status = Utils_queDelete(&(pSynthesisObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = ALGLINK_SYNTHESIS_IPQID_PALUT;
    status = Utils_queDelete(&(pSynthesisObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /*
     * Space for Algorithm specific object gets freed here.
     */
    if(sizeof(AlgorithmLink_SynthesisObj) > SV_ALGLINK_SRMEM_THRESHOLD)
    {
        status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                               pSynthesisObj,
                               sizeof(AlgorithmLink_SynthesisObj));
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
    else
    {
        free(pSynthesisObj);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Print link statistics
 *
 * \param  pObj                [IN] Algorithm link object handle
 * \param  pSynthesisObj       [IN] Frame copy link Object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_synthesisPrintStatistics(void *pObj,
                       AlgorithmLink_SynthesisObj *pSynthesisObj)
{
    UTILS_assert(NULL != pSynthesisObj->linkStatsInfo);

    Utils_printLinkStatistics(&pSynthesisObj->linkStatsInfo->linkStats, "ALG_SYNTHESIS", TRUE);

    Utils_printLatency("ALG_SYNTHESIS",
                       &pSynthesisObj->linkStatsInfo->linkLatency,
                       &pSynthesisObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
