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
 * \file surroundViewLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for DMA based SW Mosaic
 *
 * \version 0.0 (Sept 2013) : [KC] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "surroundViewLink_priv.h"
#include "singleView.h"
#include "blendView.h"

 Int32 (*AlgorithmLink_surroundViewMake)(void * pObj,
         AlgorithmLink_SurroundViewObj *pSurroundViewObj,
         System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
         System_VideoFrameBuffer *pOutFrameBuffer
         );

 Int32 AlgorithmLink_surroundViewMakeTopView(void * pObj,
                     AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                     System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
                     System_VideoFrameBuffer *pOutFrameBuffer
                     );
 Int32 AlgorithmLink_surroundViewMakeFullView(void * pObj,
                     AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                     System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
                     System_VideoFrameBuffer *pOutFrameBuffer
                     );
/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of this algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_SurroundView_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_surroundViewCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_surroundViewProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_surroundViewControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_surroundViewStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_surroundViewDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_SURROUND_VIEW;
#endif

#ifdef BUILD_M4
    algId = ALGORITHM_LINK_IPU_ALG_SURROUND_VIEW;
#endif

#ifdef BUILD_A15
    algId = ALGORITHM_LINK_A15_ALG_SURROUND_VIEW;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Check layout parameters and whereever possible also try to correct it
 *        instead of returning as invalid. When not possible to correct it
 *        return validity as FALSE
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pSurroundViewObj       [IN] SURROUND VIEW Object handle
 *
 * \return  TRUE, valid parameters, FALSE, invalid parameters
 *
 *******************************************************************************
 */
Bool AlgorithmLink_surroundViewIsLayoutPrmValid(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                    AlgorithmLink_SurroundViewLayoutParams *pLayoutPrm
                    )
{
    Bool isValid = TRUE;


    /* if number of window > max possible then possibly some array overrun
     * occured hence, flag as in valid parameter
     */
    if(pLayoutPrm->numWin>ALGORITHM_LINK_SURROUND_VIEW_MAX_WINDOWS)
        isValid = FALSE;

    /* limit output width x height to max buffer width x height */
    if(pLayoutPrm->outBufWidth > pSurroundViewObj->createArgs.maxOutBufWidth)
    {
        pLayoutPrm->outBufWidth = pSurroundViewObj->createArgs.maxOutBufWidth;
    }

    if(pLayoutPrm->outBufHeight > pSurroundViewObj->createArgs.maxOutBufHeight)
    {
        pLayoutPrm->outBufHeight = pSurroundViewObj->createArgs.maxOutBufHeight;
    }

    return isValid;
}

/**
 *******************************************************************************
 *
 * \brief Init link queue info
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pSurroundViewObj       [IN] SURROUND VIEW Object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void AlgorithmLink_surroundViewInitQueueInfo(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj)
{
    AlgorithmLink_OutputQueueInfo outputQInfo;
    AlgorithmLink_InputQueueInfo  inputQInfo;
    UInt32 maxWidth;

    memset(&outputQInfo, 0, sizeof(outputQInfo));
    memset(&inputQInfo, 0, sizeof(inputQInfo));

    maxWidth = SystemUtils_align(pSurroundViewObj->createArgs.maxOutBufWidth,
                            ALGORITHMLINK_FRAME_ALIGN);

    if(pSurroundViewObj->dataFormat==SYSTEM_DF_YUV422I_UYVY
        ||
        pSurroundViewObj->dataFormat==SYSTEM_DF_YUV422I_YUYV
    )
    {
        pSurroundViewObj->outPitch[0] = maxWidth*2;
        pSurroundViewObj->outPitch[1] = 0;
    }
    else
    if(pSurroundViewObj->dataFormat==SYSTEM_DF_YUV420SP_UV
        ||
        pSurroundViewObj->dataFormat==SYSTEM_DF_YUV420SP_VU
    )
    {
        pSurroundViewObj->outPitch[0] = maxWidth;
        pSurroundViewObj->outPitch[1] = maxWidth;
    }
    else
    {
        /* invalid data format */
        UTILS_assert(0);
    }
    /*
     * Populating parameters corresponding to Q usage of frame copy
     * algorithm link
     */
    inputQInfo.qMode  = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    outputQInfo.qMode = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    outputQInfo.queInfo.numCh = 1;

    outputQInfo.queInfo.chInfo[0].flags = 0;

    SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(
        outputQInfo.queInfo.chInfo[0].flags,
        pSurroundViewObj->dataFormat
        );

    SYSTEM_LINK_CH_INFO_SET_FLAG_MEM_TYPE(
        outputQInfo.queInfo.chInfo[0].flags,
        SYSTEM_MT_NONTILEDMEM
        );

    SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(
        outputQInfo.queInfo.chInfo[0].flags,
        SYSTEM_SF_PROGRESSIVE
        );

    SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(
        outputQInfo.queInfo.chInfo[0].flags,
        SYSTEM_BUFFER_TYPE_VIDEO_FRAME
        );


    outputQInfo.queInfo.chInfo[0].startX = 0;
    outputQInfo.queInfo.chInfo[0].startY = 0;
    outputQInfo.queInfo.chInfo[0].width  =
            pSurroundViewObj->createArgs.initLayoutParams.outBufWidth;
    outputQInfo.queInfo.chInfo[0].height =
            pSurroundViewObj->createArgs.initLayoutParams.outBufHeight;

    outputQInfo.queInfo.chInfo[0].pitch[0] = pSurroundViewObj->outPitch[0];
    outputQInfo.queInfo.chInfo[0].pitch[1] = pSurroundViewObj->outPitch[1];

    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(pObj,
                                1,
                                &inputQInfo,
                                1,
                                &outputQInfo
                                );

}

/**
 *******************************************************************************
 *
 * \brief Alloc and queue output buffers
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void AlgorithmLink_surroundViewAllocAndQueueOutBuf(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj)
{
    Int32  frameId;
    UInt32 maxHeight;
    System_Buffer              * pSystemBuffer;
    System_VideoFrameBuffer    * pSystemVideoFrameBuffer;

    UTILS_assert(
        pSurroundViewObj->createArgs.initLayoutParams.outBufWidth <=
        pSurroundViewObj->createArgs.maxOutBufWidth
        );

    UTILS_assert(
        pSurroundViewObj->createArgs.initLayoutParams.outBufHeight <=
        pSurroundViewObj->createArgs.maxOutBufHeight
        );

    maxHeight = SystemUtils_align(pSurroundViewObj->createArgs.maxOutBufHeight, 2);

    if(pSurroundViewObj->createArgs.numOutBuf>SURROUND_VIEW_LINK_MAX_OUT_BUF)
        pSurroundViewObj->createArgs.numOutBuf = SURROUND_VIEW_LINK_MAX_OUT_BUF;

    pSurroundViewObj->outBufSize = pSurroundViewObj->outPitch[0] * maxHeight * 2;

    /*
     * Creation of output buffers for output buffer Q = 0 (Used)
     *  - Connecting video frame buffer to system buffer payload
     *  - Memory allocation for Luma and Chroma buffers
     *  - Put the buffer into empty queue
     */
    for(frameId = 0; frameId < pSurroundViewObj->createArgs.numOutBuf; frameId++)
    {

        pSystemBuffer           = &(pSurroundViewObj->buffers[frameId]);
        pSystemVideoFrameBuffer = &(pSurroundViewObj->videoFrames[frameId]);

        memset(pSystemBuffer, 0, sizeof(*pSystemBuffer));
        memset(pSystemVideoFrameBuffer, 0, sizeof(*pSystemVideoFrameBuffer));

        /*
         * Properties of pSystemBuffer, which do not get altered during
         * run time (frame exchanges) are initialized here
         */
        pSystemBuffer->payload     = pSystemVideoFrameBuffer;
        pSystemBuffer->payloadSize = sizeof(System_VideoFrameBuffer);
        pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
        pSystemBuffer->chNum       = 0;

        memset((void *)&pSystemVideoFrameBuffer->chInfo,
               0,
               sizeof(System_LinkChInfo));

        /*
         * Buffer allocation done for maxHeight, maxWidth and also assuming
         * worst case num planes = 2, for data Format YUV422
         */
        pSystemVideoFrameBuffer->bufAddr[0] =
                        Utils_memAlloc(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                ( pSurroundViewObj->outBufSize ),
                                ALGORITHMLINK_FRAME_ALIGN
                            );

        /*
         * Carving out memory pointer for chroma which will get used in case of
         * SYSTEM_DF_YUV420SP_UV
         */
        pSystemVideoFrameBuffer->bufAddr[1] = (void*)(
            (UInt32) pSystemVideoFrameBuffer->bufAddr[0] +
            (UInt32)(maxHeight*pSurroundViewObj->outPitch[0])
            );

        UTILS_assert(pSystemVideoFrameBuffer->bufAddr[0] != NULL);

        AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSystemBuffer);
    }
}

/**
 *******************************************************************************
 *
 * \brief Alloc line buffer used to fill output buffer when no input
 *        is available
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void AlgorithmLink_surroundViewAllocFillBufLine(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj)
{
    pSurroundViewObj->dmaFillLineSize = pSurroundViewObj->outPitch[0] * 2;

    pSurroundViewObj->dmaFillLineAddr[0] =
                        Utils_memAlloc(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                ( pSurroundViewObj->dmaFillLineSize ),
                                ALGORITHMLINK_FRAME_ALIGN
                            );
    pSurroundViewObj->dmaFillLineAddr[1] =
        (Ptr)((UInt32)pSurroundViewObj->dmaFillLineAddr[0]
            + pSurroundViewObj->dmaFillLineSize)
            ;

    memset(pSurroundViewObj->dmaFillLineAddr[0], 0x80, pSurroundViewObj->dmaFillLineSize);

    Cache_wbInv(pSurroundViewObj->dmaFillLineAddr[0],
                pSurroundViewObj->dmaFillLineSize,
                Cache_Type_ALL,
                TRUE);
}

/**
 *******************************************************************************
 *
 * \brief Create DMA channel
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void AlgorithmLink_surroundViewCreateDmaCh(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj)
{
    Int32 status;
    UInt32 winId;
    Utils_DmaCopyFill2D *pDmaPrm;

    Utils_DmaChCreateParams_Init(&pSurroundViewObj->dmaCreateArgs);

    pSurroundViewObj->dmaCreateArgs.maxTransfers
        = ALGORITHM_LINK_SURROUND_VIEW_MAX_WINDOWS;

    if(System_getSelfProcId()==SYSTEM_PROC_DSP1
        ||
        System_getSelfProcId()==SYSTEM_PROC_DSP2
        )
    {
        if(pSurroundViewObj->createArgs.useLocalEdma)
        {
            pSurroundViewObj->dmaCreateArgs.edmaInstId
                = UTILS_DMA_LOCAL_EDMA_INST_ID;
        }
    }

    status = Utils_dmaCreateCh(
                &pSurroundViewObj->dmaChObj,
                &pSurroundViewObj->dmaCreateArgs
                );

    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    /* set constant DMA copy/fill params */
    for( winId = 0; winId<ALGORITHM_LINK_SURROUND_VIEW_MAX_WINDOWS; winId++)
    {
        pDmaPrm = &pSurroundViewObj->dmaCopyPrms[winId];

        if(pSurroundViewObj->dataFormat==SYSTEM_DF_YUV422I_UYVY
            ||
            pSurroundViewObj->dataFormat==SYSTEM_DF_YUV422I_YUYV
        )
        {
            pDmaPrm->dataFormat    = SYSTEM_DF_RAW16;
        }
        else
        if(pSurroundViewObj->dataFormat==SYSTEM_DF_YUV420SP_UV
            ||
            pSurroundViewObj->dataFormat==SYSTEM_DF_YUV420SP_VU
        )
        {
            pDmaPrm->dataFormat    = SYSTEM_DF_YUV420SP_UV;
        }

        pDmaPrm->destAddr[0]   = NULL;
        pDmaPrm->destAddr[1]   = NULL;
        pDmaPrm->destPitch[0]  = pSurroundViewObj->outPitch[0];
        pDmaPrm->destPitch[1]  = pSurroundViewObj->outPitch[1];
        pDmaPrm->destStartX    = 0;
        pDmaPrm->destStartY    = 0;
        pDmaPrm->width         = 0;
        pDmaPrm->height        = 0;
        pDmaPrm->srcAddr[0]    = NULL;
        pDmaPrm->srcAddr[1]    = NULL;
        pDmaPrm->srcPitch[0]   = 0;
        pDmaPrm->srcPitch[1]   = 0;
        pDmaPrm->srcStartX     = 0;
        pDmaPrm->srcStartY     = 0;
    }

    pDmaPrm = &pSurroundViewObj->dmaFillPrms;

    pDmaPrm->dataFormat    = pSurroundViewObj->dmaCopyPrms[0].dataFormat;
    pDmaPrm->destAddr[0]   = NULL;
    pDmaPrm->destAddr[1]   = NULL;
    pDmaPrm->destPitch[0]  = pSurroundViewObj->outPitch[0];
    pDmaPrm->destPitch[1]  = pSurroundViewObj->outPitch[1];
    pDmaPrm->destStartX    = 0;
    pDmaPrm->destStartY    = 0;
    pDmaPrm->width         = 0;
    pDmaPrm->height        = 0;
    pDmaPrm->srcAddr[0]    = pSurroundViewObj->dmaFillLineAddr[0];
    pDmaPrm->srcAddr[1]    = pSurroundViewObj->dmaFillLineAddr[1];
    pDmaPrm->srcPitch[0]   = pSurroundViewObj->dmaFillLineSize;
    pDmaPrm->srcPitch[1]   = pSurroundViewObj->dmaFillLineSize;
    pDmaPrm->srcStartX     = 0;
    pDmaPrm->srcStartY     = 0;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for this algorithm
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_surroundViewCreate(void * pObj, void * pCreateParams)
{
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    System_LinkInfo              prevLinkInfo;
    UInt32                       prevLinkQueId;

    AlgorithmLink_SurroundViewObj          * pSurroundViewObj;
    AlgorithmLink_SurroundViewCreateParams * pSurroundViewCreateParams;

    pSurroundViewCreateParams =
        (AlgorithmLink_SurroundViewCreateParams *)pCreateParams;

    /*
     * Space for Algorithm specific object gets allocated here.
     * Pointer gets recorded in algorithmParams
     */
    pSurroundViewObj = (AlgorithmLink_SurroundViewObj *)
                        malloc(sizeof(AlgorithmLink_SurroundViewObj));

    UTILS_assert(pSurroundViewObj!=NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pSurroundViewObj);

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    pSurroundViewObj->createArgs = *pSurroundViewCreateParams;

    /*
     * Channel info of current link will be obtained from previous link.
     */
    status = System_linkGetInfo(pSurroundViewObj->createArgs.inQueParams.prevLinkId,
                                &prevLinkInfo);

    prevLinkQueId = pSurroundViewObj->createArgs.inQueParams.prevLinkQueId;

    UTILS_assert(prevLinkQueId < prevLinkInfo.numQue);

    pSurroundViewObj->prevLinkQueInfo = prevLinkInfo.queInfo[prevLinkQueId];

    /* assuming data format will be same for all channels and hence taking
     * CH0 data format
     */
    pSurroundViewObj->dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(
                    pSurroundViewObj->prevLinkQueInfo.chInfo[0].flags);

    pSurroundViewObj->isLayoutSwitch = TRUE;
    pSurroundViewObj->curLayoutPrm = pSurroundViewObj->createArgs.initLayoutParams;

    if( !AlgorithmLink_surroundViewIsLayoutPrmValid(
                    pObj,
                    pSurroundViewObj,
                    &pSurroundViewObj->curLayoutPrm)
        )
    {
        Vps_printf(" SURROUND_VIEW: Invalid SW Mosaic parameters !!!\n");
        UTILS_assert(0);
    }

    AlgorithmLink_surroundViewInitQueueInfo(pObj, pSurroundViewObj);
    AlgorithmLink_surroundViewAllocAndQueueOutBuf(pObj, pSurroundViewObj);
    AlgorithmLink_surroundViewAllocFillBufLine(pObj, pSurroundViewObj);
    AlgorithmLink_surroundViewCreateDmaCh(pObj, pSurroundViewObj);

    pSurroundViewObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_SURROUND_VIEW");
    UTILS_assert(NULL != pSurroundViewObj->linkStatsInfo);

    pSurroundViewObj->isFirstFrameRecv = FALSE;

    pSurroundViewObj->buf1 =
            Utils_memAlloc(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    ( pSurroundViewObj->outBufSize ),
                    ALGORITHMLINK_FRAME_ALIGN
                );
    pSurroundViewObj->buf2 =
            Utils_memAlloc(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    ( pSurroundViewObj->outBufSize ),
                    ALGORITHMLINK_FRAME_ALIGN
                );

    AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeTopView;

   return status;
}


/**
 *******************************************************************************
 *
 * \brief Check if input buffer is valid or not
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pSurroundViewObj       [IN] SURROUND VIEW object handle
 * \param  pBuffer           [IN] Input buffer handle
 *
 * \return  TRUE, valid input buffer, FALSE, invalid input buffer
 *
 *******************************************************************************
 */
Bool AlgorithmLink_surroundViewIsInputBufValid(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                    System_Buffer *pBuffer
                    )
{
    Bool isValid = TRUE;

    if(pBuffer==NULL)
    {
        isValid = FALSE;
    }
    else if(pBuffer->bufType != SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER
        ||
       pBuffer->chNum > 0
        ||
       pBuffer->payloadSize != sizeof(System_VideoFrameCompositeBuffer)
        ||
       pBuffer->payload == NULL
       )
    {
        isValid = FALSE;
    }

    return isValid;
}

/**
 *******************************************************************************
 *
 * \brief Check and fill output buffer with bank color is required
 *
 * \param  pObj                     [IN] Algorithm link object handle
 * \param  pSurroundViewObj              [IN] SURROUND VIEW object handle
 * \param  pOutFrameBuffer          [IN] Output frame buffer
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_surroundViewDoDmaFill(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                    System_VideoFrameBuffer *pOutFrameBuffer
                    )
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_SurroundViewLayoutParams *pLayoutPrm;
    Utils_DmaCopyFill2D *pDmaPrm;

    pLayoutPrm = &pSurroundViewObj->curLayoutPrm;

    if(pSurroundViewObj->isLayoutSwitch)
    {
        /* Layout switch occured, there could be regions in the output buffer
         * where no input is present, hence clear the full output
         * buffer before copying input from output.
         * This needs to be done only pSurroundViewObj->createArgs.numOutBuf
         * times
         */
        pSurroundViewObj->isLayoutSwitch = FALSE;
        pSurroundViewObj->doFillBuf = pSurroundViewObj->createArgs.numOutBuf;
    }

    if(pSurroundViewObj->doFillBuf)
    {
        pSurroundViewObj->doFillBuf--;

        /* fill complete output buffer with blank color */
        pDmaPrm = &pSurroundViewObj->dmaFillPrms;

        pDmaPrm->destAddr[0]   = pOutFrameBuffer->bufAddr[0];
        pDmaPrm->destAddr[1]   = pOutFrameBuffer->bufAddr[1];
        pDmaPrm->width         = pLayoutPrm->outBufWidth;
        pDmaPrm->height        = pLayoutPrm->outBufHeight;

        status = Utils_dmaFill2D(
                &pSurroundViewObj->dmaChObj,
                &pSurroundViewObj->dmaFillPrms,
                1
                );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Copy data from input into output based on layout parameters
 *
 * \param  pObj                     [IN] Algorithm link object handle
 * \param  pSurroundViewObj              [IN] SURROUND VIEW object handle
 * \param  pInFrameCompositeBuffer  [IN] Input composite buffer
 * \param  pOutFrameBuffer          [IN] Output frame buffer
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

Int32 AlgorithmLink_surroundViewMakeTopView(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                    System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
                    System_VideoFrameBuffer *pOutFrameBuffer
                    )
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_SurroundViewLayoutParams *pLayoutPrm;
    AlgorithmLink_SurroundViewLutInfo *pLutViewInfo;// = pLayoutPrm->lutViewInfo;

//    AlgorithmLink_surroundViewDoDmaFill(pObj, pSurroundViewObj, pOutFrameBuffer);

    pLayoutPrm = &pSurroundViewObj->curLayoutPrm;
    pLutViewInfo = pLayoutPrm->lutViewInfo;



	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							pLayoutPrm->psingleViewInfo,
							pLayoutPrm->psingleViewLUTInfo);

	///front
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][3],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->Basic_frontNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A00]);

	///left
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][1],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->Basic_leftNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A02]);
	///rear
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][0],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->Basic_rearNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A04]);


	///right
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][2],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->Basic_rightNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A06]);

	///left, front
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][1],
					pInFrameCompositeBuffer->bufAddr[0][3],
					pSurroundViewObj->buf1,
					pSurroundViewObj->buf2,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_leftNT,
					pLayoutPrm->Basic_frontNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A01]);

	///left, rear
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][1],
					pInFrameCompositeBuffer->bufAddr[0][0],
					pSurroundViewObj->buf1,
					pSurroundViewObj->buf2,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_leftNT,
					pLayoutPrm->Basic_rearNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A03]);


	///right, front
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][2],
					pInFrameCompositeBuffer->bufAddr[0][3],
					pSurroundViewObj->buf1,
					pSurroundViewObj->buf2,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_rightNT,
					pLayoutPrm->Basic_frontNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A07]);

	///right, rear
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][2],
					pInFrameCompositeBuffer->bufAddr[0][0],
					pSurroundViewObj->buf1,
					pSurroundViewObj->buf2,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_rightNT,
					pLayoutPrm->Basic_rearNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A05]);

    return status;
}
Int32 AlgorithmLink_surroundViewMakeFullView(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj,
                    System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
                    System_VideoFrameBuffer *pOutFrameBuffer
                    )
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_SurroundViewLayoutParams *pLayoutPrm;

//    AlgorithmLink_surroundViewDoDmaFill(pObj, pSurroundViewObj, pOutFrameBuffer);

    pLayoutPrm = &pSurroundViewObj->curLayoutPrm;

	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							pLayoutPrm->psingleViewInfo,
							pLayoutPrm->psingleViewLUTInfo);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process Plugin for this algorithm
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_surroundViewProcess(void * pObj)
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;

    AlgorithmLink_SurroundViewObj  *pSurroundViewObj;
    System_BufferList          inputBufList;
    System_BufferList          outputBufListReturn;
    System_Buffer             *pInBuffer;
    System_Buffer             *pOutBuffer;
    System_VideoFrameBuffer   *pOutFrameBuffer;
    System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer;
    UInt32 bufId;
    Bool  bufDropFlag[SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST];
    System_LinkStatistics      * linkStatsInfo;

    pSurroundViewObj = (AlgorithmLink_SurroundViewObj *)
                    AlgorithmLink_getAlgorithmParamsObj(pObj);

    UTILS_assert(pSurroundViewObj!=NULL);

    linkStatsInfo = pSurroundViewObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    /*
     * Getting input buffers from previous link
     */
    System_getLinksFullBuffers(
            pSurroundViewObj->createArgs.inQueParams.prevLinkId,
            pSurroundViewObj->createArgs.inQueParams.prevLinkQueId,
            &inputBufList);

    if(inputBufList.numBuf)
    {
        if(pSurroundViewObj->isFirstFrameRecv==FALSE)
        {
            pSurroundViewObj->isFirstFrameRecv = TRUE;

            Utils_resetLinkStatistics(
                    &linkStatsInfo->linkStats,
                    pSurroundViewObj->prevLinkQueInfo.numCh,
                    1);

            Utils_resetLatency(&linkStatsInfo->linkLatency);
            Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
        }

        for(bufId=0; bufId<inputBufList.numBuf; bufId++)
        {
            pInBuffer = inputBufList.buffers[bufId];

            if( ! AlgorithmLink_surroundViewIsInputBufValid(
                            pObj,
                            pSurroundViewObj,
                            pInBuffer
                            )
                )
            {
                bufDropFlag[bufId] = TRUE;

                linkStatsInfo->linkStats.inBufErrorCount++;
                continue;
            }

            linkStatsInfo->linkStats.chStats[pInBuffer->chNum].inBufRecvCount++;

            status = AlgorithmLink_getEmptyOutputBuffer(
                                pObj,
                                0,
                                0,
                                &pOutBuffer
                                );

            if(status != SYSTEM_LINK_STATUS_SOK
                ||
                pOutBuffer == NULL
                )
            {

                linkStatsInfo->linkStats.outBufErrorCount++;
                linkStatsInfo->linkStats.chStats
                        [pInBuffer->chNum].inBufDropCount++;
                linkStatsInfo->linkStats.chStats
                        [pInBuffer->chNum].outBufDropCount[0]++;

                bufDropFlag[bufId] = TRUE;
                continue;
            }

            pOutBuffer->srcTimestamp = pInBuffer->srcTimestamp;
            pOutBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

            bufDropFlag[bufId] = FALSE;
            pOutFrameBuffer = (System_VideoFrameBuffer*)pOutBuffer->payload;

            pInFrameCompositeBuffer = (System_VideoFrameCompositeBuffer*)
                                            pInBuffer->payload;

            linkStatsInfo->linkStats.chStats
                    [pInBuffer->chNum].inBufProcessCount++;
            linkStatsInfo->linkStats.chStats
                    [pInBuffer->chNum].outBufCount[0]++;

            AlgorithmLink_surroundViewMake(
                    pObj,
                    pSurroundViewObj,
                    pInFrameCompositeBuffer,
                    pOutFrameBuffer
                    );

            Utils_updateLatency(&pSurroundViewObj->linkLatency,
                                pOutBuffer->linkLocalTimestamp);
            Utils_updateLatency(&pSurroundViewObj->srcToLinkLatency,
                                pOutBuffer->srcTimestamp);

            /*
             * Putting filled buffer into output full buffer Q
             */
            status = AlgorithmLink_putFullOutputBuffer(pObj,
                                                       0,
                                                       pOutBuffer);

            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            /*
             * Informing next link that a new data has peen put for its
             * processing
             */
            System_sendLinkCmd(pSurroundViewObj->createArgs.outQueParams.nextLink,
                               SYSTEM_CMD_NEW_DATA,
                               NULL);

            /*
             * Releasing (Free'ing) output buffer, since algorithm does not
             * need it for any future usage.
             * In case of INPLACE computation, there is no need to free output
             * buffer, since it will be freed as input buffer.
             */
            outputBufListReturn.numBuf     = 1;
            outputBufListReturn.buffers[0] = pOutBuffer;

            AlgorithmLink_releaseOutputBuffer(pObj,
                                              0,
                                              &outputBufListReturn);
        }

        /* release input buffers */
        AlgorithmLink_releaseInputBuffer(
                    pObj,
                    0,
                    pSurroundViewObj->createArgs.inQueParams.prevLinkId,
                    pSurroundViewObj->createArgs.inQueParams.prevLinkQueId,
                    &inputBufList,
                    bufDropFlag
            );
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Control Plugin for this algorithm
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pControlParams    [IN] Pointer to control parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

Int32 AlgorithmLink_surroundViewControl(void * pObj, void * pControlParams)
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_ControlParams *pAlgLinkControlPrm;
    AlgorithmLink_SurroundViewLayoutParams *pLayoutPrm;
    AlgorithmLink_SurroundViewObj  *pSurroundViewObj;

    pSurroundViewObj = (AlgorithmLink_SurroundViewObj *)
                    AlgorithmLink_getAlgorithmParamsObj(pObj);

    UTILS_assert(pSurroundViewObj!=NULL);

    pAlgLinkControlPrm = (AlgorithmLink_ControlParams *)pControlParams;

    switch(pAlgLinkControlPrm->controlCmd)
    {
        case ALGORITHM_LINK_SURROUND_VIEW_CONFIG_CMD_SET_LAYOUT_PARAMS:

            if(pAlgLinkControlPrm->size != sizeof(*pLayoutPrm))
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            else
            {
                pLayoutPrm = (AlgorithmLink_SurroundViewLayoutParams *)
                                    pControlParams;

                if( ! AlgorithmLink_surroundViewIsLayoutPrmValid(
                        pObj,
                        pSurroundViewObj,
                        pLayoutPrm)
                    )
                {
                    status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
                    Vps_printf(" SURROUND_VIEW: Invalid SW Mosaic parameters !!!\n");
                }
                else
                {
                    pSurroundViewObj->curLayoutPrm = *pLayoutPrm;
                    pSurroundViewObj->isLayoutSwitch = TRUE;
                }
            }
            break;
        case ALGORITHM_LINK_SURROUND_VIEW_CONFIG_CMD_GET_LAYOUT_PARAMS:

            if(pAlgLinkControlPrm->size != sizeof(*pLayoutPrm))
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            else
            {
                pLayoutPrm = (AlgorithmLink_SurroundViewLayoutParams *)
                                    pControlParams;

                *pLayoutPrm = pSurroundViewObj->curLayoutPrm;

                pLayoutPrm->baseClassControl.size = sizeof(*pLayoutPrm);
                pLayoutPrm->baseClassControl.controlCmd
                        = ALGORITHM_LINK_SURROUND_VIEW_CONFIG_CMD_GET_LAYOUT_PARAMS;
            }
            break;

        case SYSTEM_CMD_PRINT_STATISTICS:
            AlgorithmLink_surroundViewPrintStatistics(pObj, pSurroundViewObj);
            break;
        default:
            status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of S Plugin for this algorithm
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
Int32 AlgorithmLink_surroundViewStop(void * pObj)
{
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete Plugin for this algorithm
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_surroundViewDelete(void * pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_SurroundViewObj  *pSurroundViewObj;
    System_VideoFrameBuffer   *pSystemVideoFrameBuffer;
    UInt32 frameId;

    pSurroundViewObj = (AlgorithmLink_SurroundViewObj *)
                    AlgorithmLink_getAlgorithmParamsObj(pObj);

    UTILS_assert(pSurroundViewObj!=NULL);

    status = Utils_linkStatsCollectorDeAllocInst(pSurroundViewObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* free DMA channel */
    status = Utils_dmaDeleteCh(&pSurroundViewObj->dmaChObj);
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    /* free memory */
    for(frameId = 0; frameId < pSurroundViewObj->createArgs.numOutBuf; frameId++)
    {
        pSystemVideoFrameBuffer = &(pSurroundViewObj->videoFrames[frameId]);

        status = Utils_memFree(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    pSystemVideoFrameBuffer->bufAddr[0],
                    pSurroundViewObj->outBufSize
                    );
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
    }

	Utils_memFree(	UTILS_HEAPID_DDR_CACHED_SR,
					pSurroundViewObj->buf1,
					pSurroundViewObj->outBufSize);

	Utils_memFree(	UTILS_HEAPID_DDR_CACHED_SR,
					pSurroundViewObj->buf2,
					pSurroundViewObj->outBufSize);

    status = Utils_memFree(
                UTILS_HEAPID_DDR_CACHED_SR,
                pSurroundViewObj->dmaFillLineAddr[0],
                pSurroundViewObj->dmaFillLineSize
                  );
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    free(pSurroundViewObj);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Print link statistics
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pSurroundViewObj       [IN] SURROUND VIEW Object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_surroundViewPrintStatistics(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj)
{
    UTILS_assert(NULL != pSurroundViewObj->linkStatsInfo);

    Utils_printLinkStatistics(&pSurroundViewObj->linkStatsInfo->linkStats, "ALG_SURROUND_VIEW", TRUE);

    Utils_printLatency("ALG_SURROUND_VIEW",
                       &pSurroundViewObj->linkStatsInfo->linkLatency,
                       &pSurroundViewObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
