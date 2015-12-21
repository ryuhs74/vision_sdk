/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Cammsys - http://www.cammsys.net/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file surroundViewLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for  Surround View
 *
 * \version 0.0 (Dec 2015) : [Raven] First version
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

#define CAMERA_FRONT	3
#define CAMERA_REAR		0
#define CAMERA_LEFT		1
#define CAMERA_RIGHT	2



 static Int32 AlgorithmLink_surroundViewMakeTopView(	void * pObj,
													AlgorithmLink_SurroundViewLayoutParams* pLayoutPrm,
													System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
													System_VideoFrameBuffer *pOutFrameBuffer);
static Int32 AlgorithmLink_surroundViewMakeSingleView(void * pObj,
													AlgorithmLink_SurroundViewLayoutParams* pLayoutPrm,
													System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
													System_VideoFrameBuffer *pOutFrameBuffer);
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
 *makeSingleView720PNoInter
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
        Vps_printf(" SURROUND_VIEW: Invalid Surround View parameters !!!\n");
        UTILS_assert(0);
    }

    AlgorithmLink_surroundViewInitQueueInfo(pObj, pSurroundViewObj);
    AlgorithmLink_surroundViewAllocAndQueueOutBuf(pObj, pSurroundViewObj);

    pSurroundViewObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_SURROUND_VIEW");
    UTILS_assert(NULL != pSurroundViewObj->linkStatsInfo);

    pSurroundViewObj->isFirstFrameRecv = FALSE;

    pSurroundViewObj->curLayoutPrm.FilterInbuf =
            Utils_memAlloc(
            		UTILS_HEAPID_OCMC_SR,
					BLEND_VIEW_TEMP_BUF_SIZE ,
                    ALGORITHMLINK_FRAME_ALIGN
                );
    UTILS_assert( pSurroundViewObj->curLayoutPrm.FilterInbuf);

    pSurroundViewObj->curLayoutPrm.FilterOutbuf =
            Utils_memAlloc(
            		UTILS_HEAPID_OCMC_SR,
                    BLEND_VIEW_TEMP_BUF_SIZE ,
                    ALGORITHMLINK_FRAME_ALIGN
                );
    UTILS_assert( pSurroundViewObj->curLayoutPrm.FilterOutbuf);

    if(pSurroundViewObj->curLayoutPrm.makeViewPart == 0)
    {
    	pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeTopView;
    }
    else
    {
#if 0 ///full view Test
		pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_frontFullView;
		pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_FULL_VIEW];
		pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_FULL_VIEW_LUT];
		pSurroundViewObj->curLayoutPrm.singleViewInputChannel = 3;
#endif
		pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeSingleView;
    }
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

Int32 AlgorithmLink_surroundViewMakeTopView( void * pObj,
											AlgorithmLink_SurroundViewLayoutParams* pLayoutPrm,
											System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
											System_VideoFrameBuffer *pOutFrameBuffer)
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_SurroundViewLutInfo *pLutViewInfo;// = pLayoutPrm->lutViewInfo;


    pLutViewInfo = pLayoutPrm->lutViewInfo;

    /*
        pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].startX 	= 0;
        pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].startY 	= 0;
        pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].width 	= 712;
        pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].height	= 508;
        pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].pitch		= 712;

    */
#if SURROUND_VIEW_ONE_CORE
#if 1	///for speed
    AlgorithmLink_SurroundViewLutInfo sideViewForLut[4];// = pLayoutPrm->lutViewInfo;

    sideViewForLut[0].pitch = 712;
    sideViewForLut[1].pitch = 712;
    sideViewForLut[2].pitch = 712;
    sideViewForLut[3].pitch = 712;

    sideViewForLut[0].startX = 0;
    sideViewForLut[1].startX = 178;
    sideViewForLut[2].startX = 178*2;
    sideViewForLut[3].startX = 178*3;

    sideViewForLut[0].startY = 0;
    sideViewForLut[1].startY = 0;
    sideViewForLut[2].startY = 0;
    sideViewForLut[3].startY = 0;

    sideViewForLut[0].width = 178;
    sideViewForLut[1].width = 178;
    sideViewForLut[2].width = 178;
    sideViewForLut[3].width = 178;

    sideViewForLut[0].height = 508;
    sideViewForLut[1].height = 508;
    sideViewForLut[2].height = 508;
    sideViewForLut[3].height = 508;

	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							&pLutViewInfo[LUT_VIEW_INFO_SIDE_VIEW],
							&sideViewForLut[0]);


	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							&pLutViewInfo[LUT_VIEW_INFO_SIDE_VIEW],
							&sideViewForLut[1]);

	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							&pLutViewInfo[LUT_VIEW_INFO_SIDE_VIEW],
							&sideViewForLut[2]);

	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							&pLutViewInfo[LUT_VIEW_INFO_SIDE_VIEW],
							&sideViewForLut[3]);

#else

    ///side
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][pLayoutPrm->singleViewInputChannel],
							pOutFrameBuffer->bufAddr[0],
							pLayoutPrm->psingleViewLUT,
							pLayoutPrm->psingleViewInfo,
							pLayoutPrm->psingleViewLUTInfo);

#endif
#endif 	///#if SURROUND_VIEW_ONE_CORE
	///front
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][3],
							pOutFrameBuffer->bufAddr[0],
							(UInt8*)pLayoutPrm->FilterInbuf,
							(UInt8*)pLayoutPrm->FilterOutbuf,
							pLayoutPrm->Basic_frontNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A00]);

	///left
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][1],
							pOutFrameBuffer->bufAddr[0],
							(UInt8*)pLayoutPrm->FilterInbuf,
							(UInt8*)pLayoutPrm->FilterOutbuf,
							pLayoutPrm->Basic_leftNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A02]);
	///rear
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][0],
							pOutFrameBuffer->bufAddr[0],
							(UInt8*)pLayoutPrm->FilterInbuf,
							(UInt8*)pLayoutPrm->FilterOutbuf,
							pLayoutPrm->Basic_rearNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A04]);


	///right
	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][2],
							pOutFrameBuffer->bufAddr[0],
							(UInt8*)pLayoutPrm->FilterInbuf,
							(UInt8*)pLayoutPrm->FilterOutbuf,
							pLayoutPrm->Basic_rightNT,
							&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
							&pLutViewInfo[LUT_VIEW_INFO_TOP_A06]);


	///left, front
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][1],
					pInFrameCompositeBuffer->bufAddr[0][3],
					(UInt8*)pLayoutPrm->FilterInbuf,
					(UInt8*)pLayoutPrm->FilterOutbuf,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_leftNT,
					pLayoutPrm->Basic_frontNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A01]);
	///left, rear
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][1],
					pInFrameCompositeBuffer->bufAddr[0][0],
					(UInt8*)pLayoutPrm->FilterInbuf,
					(UInt8*)pLayoutPrm->FilterOutbuf,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_leftNT,
					pLayoutPrm->Basic_rearNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A03]);


	///right, front
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][2],
					pInFrameCompositeBuffer->bufAddr[0][3],
					(UInt8*)pLayoutPrm->FilterInbuf,
					(UInt8*)pLayoutPrm->FilterOutbuf,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_rightNT,
					pLayoutPrm->Basic_frontNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A07]);

	///right, rear
	makeBlendView(	pInFrameCompositeBuffer->bufAddr[0][2],
					pInFrameCompositeBuffer->bufAddr[0][0],
					(UInt8*)pLayoutPrm->FilterInbuf,
					(UInt8*)pLayoutPrm->FilterOutbuf,
					pOutFrameBuffer->bufAddr[0],
					pLayoutPrm->Basic_rightNT,
					pLayoutPrm->Basic_rearNT,
					pLayoutPrm->cmaskNT,
					&pLutViewInfo[LUT_VIEW_INFO_TOP_VIEW],
					&pLutViewInfo[LUT_VIEW_INFO_TOP_A05]);

    return status;
}
Int32 AlgorithmLink_surroundViewMakeSingleView(	void * pObj,
												AlgorithmLink_SurroundViewLayoutParams* curLayoutPrm,
												System_VideoFrameCompositeBuffer *pInFrameCompositeBuffer,
												System_VideoFrameBuffer *pOutFrameBuffer)
{
#define INTERPOLATION_TEST	0
#define	DEVIDE_WIDTH	200
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_SurroundViewLutInfo indViewforLUT = {0};
    int i=0;
    int devideCount = curLayoutPrm->psingleViewLUTInfo->width/DEVIDE_WIDTH;
    int nextStartX = 0;
    indViewforLUT.pitch = curLayoutPrm->psingleViewLUTInfo->pitch;
	indViewforLUT.startY = curLayoutPrm->psingleViewLUTInfo->startY;
	indViewforLUT.height = curLayoutPrm->psingleViewLUTInfo->height;
	indViewforLUT.width = DEVIDE_WIDTH;

#if INTERPOLATION_TEST
    AlgorithmLink_SurroundViewLutInfo ViewforTest = {0};
    AlgorithmLink_SurroundViewLutInfo ViewforTest1 = {0};
    ViewforTest.pitch = curLayoutPrm->psingleViewInfo->pitch;
    ViewforTest.startY = curLayoutPrm->psingleViewInfo->startY;
    ViewforTest.height = curLayoutPrm->psingleViewInfo->height;
    ViewforTest.width = curLayoutPrm->psingleViewInfo->width;
    ViewforTest.startX = curLayoutPrm->psingleViewInfo->startX + (DEVIDE_WIDTH<<1);

    ViewforTest1.pitch = curLayoutPrm->psingleViewInfo->pitch;
    ViewforTest1.startY = curLayoutPrm->psingleViewInfo->startY;
    ViewforTest1.height = curLayoutPrm->psingleViewInfo->height;
    ViewforTest1.width = curLayoutPrm->psingleViewInfo->width;
    ViewforTest1.startX = curLayoutPrm->psingleViewInfo->startX + DEVIDE_WIDTH;

    for(i=0; i<(devideCount>>1); i++)
    {

    	indViewforLUT.startX = curLayoutPrm->psingleViewLUTInfo->startX + nextStartX;
    	nextStartX += (DEVIDE_WIDTH<<1);

    	status = makeSingleView(pInFrameCompositeBuffer->bufAddr[0][curLayoutPrm->singleViewInputChannel],
    							pOutFrameBuffer->bufAddr[0],
								(UInt8*) curLayoutPrm->FilterInbuf,
								(UInt8*) curLayoutPrm->buf2,
    							curLayoutPrm->psingleViewLUT,
    							curLayoutPrm->psingleViewInfo,
    							&indViewforLUT);

    	status = makeSingleView720P(pInFrameCompositeBuffer->bufAddr[0][curLayoutPrm->singleViewInputChannel],
    							pOutFrameBuffer->bufAddr[0],
								(UInt8*) curLayoutPrm->FilterInbuf,
								(UInt8*) curLayoutPrm->buf2,
    							curLayoutPrm->psingleViewLUT,
    							&ViewforTest1,
    							&indViewforLUT);


    	status = makeSingleView720PNoInter(pInFrameCompositeBuffer->bufAddr[0][curLayoutPrm->singleViewInputChannel],
    							pOutFrameBuffer->bufAddr[0],
								(UInt8*) curLayoutPrm->FilterInbuf,
								(UInt8*) curLayoutPrm->buf2,
    							curLayoutPrm->psingleViewLUT,
    							&ViewforTest,
    							&indViewforLUT);
    }
#else


    for(i=0; i<devideCount; i++)
    {
    	indViewforLUT.startX = curLayoutPrm->psingleViewLUTInfo->startX + nextStartX;
    	nextStartX += DEVIDE_WIDTH;

    	status =
    			makeSingleView(	pInFrameCompositeBuffer->bufAddr[0][curLayoutPrm->singleViewInputChannel],
												pOutFrameBuffer->bufAddr[0],
												(UInt8*)curLayoutPrm->FilterInbuf,
												(UInt8*)curLayoutPrm->FilterOutbuf,
												curLayoutPrm->psingleViewLUT,
												curLayoutPrm->psingleViewInfo,
												&indViewforLUT);
	}

	indViewforLUT.startX = curLayoutPrm->psingleViewLUTInfo->startX + nextStartX;
	indViewforLUT.width = curLayoutPrm->psingleViewLUTInfo->width - nextStartX;

	if (indViewforLUT.width)
	{
		status =
				makeSingleView(	pInFrameCompositeBuffer->bufAddr[0][curLayoutPrm->singleViewInputChannel],
												pOutFrameBuffer->bufAddr[0],
												(UInt8*) curLayoutPrm->FilterInbuf,
												(UInt8*) curLayoutPrm->FilterOutbuf,
												curLayoutPrm->psingleViewLUT,
												curLayoutPrm->psingleViewInfo,
												&indViewforLUT);
	}
#endif

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
    int InbufCount = 0;

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


            for(InbufCount = 0; InbufCount < pInFrameCompositeBuffer->numFrames; InbufCount++)
            {
            	int size = pInFrameCompositeBuffer->chInfo.height * pInFrameCompositeBuffer->chInfo.pitch[InbufCount];

                Cache_inv(pInFrameCompositeBuffer->bufAddr[0][InbufCount],
						  size,
                          Cache_Type_ALLD,
                          TRUE
                         );
            }

            pSurroundViewObj->AlgorithmLink_surroundViewMake(
                    pObj,
                    &pSurroundViewObj->curLayoutPrm,
                    pInFrameCompositeBuffer,
                    pOutFrameBuffer
                    );

            Cache_wb(pOutFrameBuffer->bufAddr[0],
					 pSurroundViewObj->outBufSize,
                     Cache_Type_ALLD,
                     TRUE
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
                    Vps_printf(" SURROUND_VIEW: Invalid Surround View parameters !!!\n");
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
        //ryuhs74@20151127 - START
        case SYSTEM_CMD_FRONT_SIDE_VIEW:
        	Vps_printf("In SYSTEM_CMD_FRONT_SIDE_VIEW\n");

            if(pSurroundViewObj->curLayoutPrm.makeViewPart == 0)
            {
            	pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeTopView;
            }
            else
            {
            	pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_frontView;
                pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW];
                pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT];
                pSurroundViewObj->curLayoutPrm.singleViewInputChannel = CAMERA_FRONT;
            }
        	break;

        case SYSTEM_CMD_REAR_SIDE_VIEW:
        	Vps_printf("In SYSTEM_CMD_REAR_SIDE_VIEW\n");

            if(pSurroundViewObj->curLayoutPrm.makeViewPart == 0)
            {
            	pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeTopView;
            }
            else
            {
            	pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_rearView;
                pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW];
                pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT];
                pSurroundViewObj->curLayoutPrm.singleViewInputChannel = CAMERA_REAR;
            }
            break;
        case SYSTEM_CMD_RIGH_SIDE_VIEW:
        	Vps_printf("In SYSTEM_CMD_RIGH_SIDE_VIEW\n");
            if(pSurroundViewObj->curLayoutPrm.makeViewPart == 0)
            {
            	pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeTopView;
            }
            else
            {
            	pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_rightSideView;
                pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW];
                pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT];
                pSurroundViewObj->curLayoutPrm.singleViewInputChannel = CAMERA_RIGHT;
            }
            break;
        case SYSTEM_CMD_LEFT_SIDE_VIEW:
            if(pSurroundViewObj->curLayoutPrm.makeViewPart == 0)
            {
            	pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeTopView;
            }
            else
            {
            	Vps_printf("In SYSTEM_CMD_LEFT_SIDE_VIEW\n");
            	pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_leftSideView;
                pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW];
                pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT];
                pSurroundViewObj->curLayoutPrm.singleViewInputChannel = CAMERA_LEFT;

            }
            break;
        case SYSTEM_CMD_FULL_FRONT_VIEW:
        	Vps_printf("In SYSTEM_CMD_FULL_FRONT_VIEW\n");

        	pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_frontFullView;
            pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_FULL_VIEW];
            pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_FULL_VIEW_LUT];
            pSurroundViewObj->curLayoutPrm.singleViewInputChannel = CAMERA_FRONT;

            pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeSingleView;
            break;
        case SYSTEM_CMD_FULL_REAR_VIEW:
        	Vps_printf("In SYSTEM_CMD_FULL_REAR_VIEW\n");
        	pSurroundViewObj->curLayoutPrm.psingleViewLUT = pSurroundViewObj->curLayoutPrm.Basic_rearFullView;
            pSurroundViewObj->curLayoutPrm.psingleViewInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_FULL_VIEW];
            pSurroundViewObj->curLayoutPrm.psingleViewLUTInfo = &pSurroundViewObj->curLayoutPrm.lutViewInfo[LUT_VIEW_INFO_FULL_VIEW_LUT];
            pSurroundViewObj->curLayoutPrm.singleViewInputChannel = CAMERA_REAR;

            pSurroundViewObj->AlgorithmLink_surroundViewMake = AlgorithmLink_surroundViewMakeSingleView;
            break;
        //ryuhs74@20151127 - END

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

    if(pSurroundViewObj->curLayoutPrm.makeViewPart == 0)
    {
		Utils_memFree(	UTILS_HEAPID_OCMC_SR,
						pSurroundViewObj->curLayoutPrm.FilterInbuf,
						BLEND_VIEW_TEMP_BUF_SIZE);

		Utils_memFree(	UTILS_HEAPID_OCMC_SR,
						pSurroundViewObj->curLayoutPrm.FilterOutbuf,
						BLEND_VIEW_TEMP_BUF_SIZE);
    }
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
