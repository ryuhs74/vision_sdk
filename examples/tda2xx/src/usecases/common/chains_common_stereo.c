/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <examples/tda2xx/include/chains_common.h>
#include "chains_common_stereo_defines.h"

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define SYNC_DELTA_IN_MSEC              (16)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define SYNC_DROP_THRESHOLD_IN_MSEC     (33)

/**
 *******************************************************************************
 * \brief Number of buffers per channel per link
 *******************************************************************************
 */
#define LINK_NUM_BUFFERS                (6)

/**
 *******************************************************************************
 * \brief Global Structure for storing configurable stereo parameters
 *******************************************************************************
 */
Stereo_ConfigurableCreateParams gStereoParams;

/**
 *******************************************************************************
 * \brief Global Structure for storing different image dimensions used throughout the execution of the stereo-vision algorithm
 *******************************************************************************
 */
StereoImageDims gStereoImDims;

/**
 *******************************************************************************
 *
 * \brief   This function populates a structure StereoImageDims when the chain is started.
 *
 *  \param  imDim[OUT] Pointer to the StereoImageDims structure to be populated
 *          pStereoParams [IN] Pointer to the stereo parameters
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_initImageDims(StereoImageDims *imDim, Stereo_ConfigurableCreateParams *pStereoParams){


    imDim->disparityInputImageWidth= ALIGN(imDim->disparityOutputRoiWidth + pStereoParams->numDisparities - 1 + pStereoParams->disparitySupportWinWidth - 1, 4);
    imDim->disparityInputImageHeight= ALIGN(imDim->disparityOutputRoiHeight + pStereoParams->disparitySupportWinHeight - 1, 3);

    if (pStereoParams->disparitySearchDir== DISPARITY_TI_RIGHT_TO_LEFT) {
        imDim->disparityInputLeftImageStartX= ((pStereoParams->disparitySupportWinWidth - 1) >> 1);
    }
    else {
        imDim->disparityInputLeftImageStartX= pStereoParams->numDisparities - 1 + ((pStereoParams->disparitySupportWinWidth - 1) >> 1);
    }

    imDim->disparityInputLeftImageStartY= ((pStereoParams->disparitySupportWinHeight - 1) >> 1);

    imDim->disparityInputRightImageStartX= ((pStereoParams->disparitySupportWinWidth - 1) >> 1);
    imDim->disparityInputRightImageStartY= ((pStereoParams->disparitySupportWinHeight - 1) >> 1);

    imDim->censusOutputRoiWidth= imDim->disparityInputImageWidth;
    imDim->censusOutputRoiHeight= imDim->disparityInputImageHeight;

    if (imDim->remapImageWidth== 0 || imDim->remapImageHeight== 0) {

        imDim->censusInputImageWidth= imDim->censusOutputRoiWidth + pStereoParams->censusWinWidth - 1;
        /* round up to multiple of REMAP_OUTPUT_BLOCK_WIDTH */
        imDim->censusInputImageWidth= ((imDim->censusInputImageWidth + (REMAP_OUTPUT_BLOCK_WIDTH-1))/REMAP_OUTPUT_BLOCK_WIDTH)*REMAP_OUTPUT_BLOCK_WIDTH;

        imDim->censusInputImageHeight= imDim->censusOutputRoiHeight + pStereoParams->censusWinHeight - 1;
        /* round up to multiple of REMAP_OUTPUT_BLOCK_HEIGHT */
        imDim->censusInputImageHeight= ((imDim->censusInputImageHeight + (REMAP_OUTPUT_BLOCK_HEIGHT-1))/REMAP_OUTPUT_BLOCK_HEIGHT)*REMAP_OUTPUT_BLOCK_HEIGHT;
    }
    else {
        imDim->censusInputImageWidth= imDim->remapImageWidth;
        imDim->censusInputImageHeight= imDim->remapImageHeight;
    }

    imDim->censusInputImageStartX= ((pStereoParams->censusWinWidth - 1)>>1);
    imDim->censusInputImageStartY= ((pStereoParams->censusWinHeight - 1)>>1);

    imDim->remapImageWidth= imDim->censusInputImageWidth;
    imDim->remapImageHeight= imDim->censusInputImageHeight;

    imDim->origRoiStartX= imDim->censusInputImageStartX + imDim->disparityInputLeftImageStartX;
    imDim->origRoiStartY= imDim->censusInputImageStartY + imDim->disparityInputLeftImageStartY;

}

/**
 *******************************************************************************
 *
 * \brief   callback to handle user defined commands reaching system link
 *
 *  \param  cmd [IN] Command that needs to be handled
 *          pPrm [IN/OUT] Parameters for this command
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_CmdHandler(UInt32 cmd, Void *pPrm)
{
    if(cmd == SYSTEM_LINK_CMD_STEREO_SET_PARAM)
    {
        Stereo_CreateParams_Init((Stereo_ConfigurableCreateParams *)pPrm);
    }
}

/**
 *******************************************************************************
 *
 * \brief   ChainsCommon_Stereo_Init
 *
 *          This function initializes the configurable stereo params.
 *          And it also registers a callback to handle user defined commands reaching system link
 * \param   NULL
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_Init()
{
    Stereo_CreateParams_Init(NULL);
    SystemLink_registerHandler(ChainsCommon_Stereo_CmdHandler);
}

/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          chains_lvdsVipMultiCam_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Scaling parameters are set .
 *
 *          Scale each CH to 1/2x size
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetVpePrm(
        VpeLink_CreateParams *pIspPrm,
        VpeLink_CreateParams *pOrgPrm,
        VpeLink_CreateParams *pDisparityPrm,
        StereoImageDims *imDims,
        UInt32 numLvdsCh,
        UInt32 disparityWinWidth,
        UInt32 disparityWinHeight,
        UInt32 orgVideoWinWidth,
        UInt32 orgVideoWinHeight
)
{
    UInt16 chId;

    if(pIspPrm)
        pIspPrm->enableOut[0] = TRUE;

    if(pOrgPrm)
        pOrgPrm->enableOut[0] = TRUE;

    if(pDisparityPrm)
        pDisparityPrm->enableOut[0] = TRUE;

    for(chId = 0; chId < 2; chId++)
    {
        if(pIspPrm)
        {
            pIspPrm->chParams[chId].outParams[0].width = imDims->remapImageWidth;
            pIspPrm->chParams[chId].outParams[0].height = imDims->remapImageHeight;
            pIspPrm->chParams[chId].outParams[0].dataFormat = SYSTEM_DF_YUV420SP_UV;
            pIspPrm->chParams[chId].outParams[0].numBufsPerCh = 4;
            pIspPrm->chParams[chId].scCropCfg.cropStartX = 0;
            pIspPrm->chParams[chId].scCropCfg.cropStartY = 0;
            /* scCropCfg.cropWidth and scCropCfg.cropHeight should be equal to input resolution
             * to VPE for upscale/downscale to work properly
             */
            pIspPrm->chParams[chId].scCropCfg.cropWidth = CAPTURE_SENSOR_WIDTH;
            pIspPrm->chParams[chId].scCropCfg.cropHeight = SOFTISP_OUTPUT_HEIGHT;
        }

        if(pOrgPrm)
        {
            pOrgPrm->chParams[chId].outParams[0].width   = orgVideoWinWidth;
            pOrgPrm->chParams[chId].outParams[0].height  = orgVideoWinHeight;
            pOrgPrm->chParams[chId].outParams[0].dataFormat
            = SYSTEM_DF_YUV420SP_UV;
            pOrgPrm->chParams[chId].outParams[0].numBufsPerCh = 4;
            pOrgPrm->chParams[chId].scCropCfg.cropStartX = imDims->origRoiStartX;
            pOrgPrm->chParams[chId].scCropCfg.cropStartY = imDims->origRoiStartY;
            /* scCropCfg.cropWidth and scCropCfg.cropHeight should be equal to input resolution
             * to VPE for upscale/downscale to work properly
             */
            pOrgPrm->chParams[chId].scCropCfg.cropWidth  = imDims->disparityOutputRoiWidth;
            pOrgPrm->chParams[chId].scCropCfg.cropHeight = imDims->disparityOutputRoiHeight;
        }

        if(pDisparityPrm)
        {
            pDisparityPrm->chParams[chId].outParams[0].width = disparityWinWidth;
            pDisparityPrm->chParams[chId].outParams[0].height = disparityWinHeight;
            pDisparityPrm->chParams[chId].outParams[0].dataFormat
            = SYSTEM_DF_YUV420SP_UV;
            pDisparityPrm->chParams[chId].outParams[0].numBufsPerCh = 4;
            pDisparityPrm->chParams[chId].scCropCfg.cropStartX = 0;
            pDisparityPrm->chParams[chId].scCropCfg.cropStartY = 0;
            pDisparityPrm->chParams[chId].scCropCfg.cropWidth = imDims->disparityOutputRoiWidth;
            pDisparityPrm->chParams[chId].scCropCfg.cropHeight
            = imDims->disparityOutputRoiHeight;
        }
    }
}


/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function. It is advisable to have
 *          chains_lvdsVipMultiCam_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Number of channels to be synced and sync delta and threshold.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetSyncPrm(
        SyncLink_CreateParams *pOrgPrm,
        SyncLink_CreateParams *pDispPrm,
        UInt32 numLvdsCh
)
{
    UInt16 chId;

    if(pOrgPrm)
    {
        pOrgPrm->chParams.numCh = numLvdsCh;
        pOrgPrm->chParams.numActiveCh = pOrgPrm->chParams.numCh;

        for(chId = 0; chId < pOrgPrm->chParams.numCh; chId++)
        {
            pOrgPrm->chParams.channelSyncList[chId] = TRUE;
        }

        pOrgPrm->chParams.syncDelta = SYNC_DELTA_IN_MSEC;
        pOrgPrm->chParams.syncThreshold = SYNC_DROP_THRESHOLD_IN_MSEC;
    }

    if(pDispPrm)
    {
        pDispPrm->chParams.numCh = numLvdsCh;
        pDispPrm->chParams.numActiveCh = pDispPrm->chParams.numCh;

        for(chId = 0; chId < pDispPrm->chParams.numCh; chId++)
        {
            pDispPrm->chParams.channelSyncList[chId] = TRUE;
        }
        pDispPrm->chParams.syncDelta = SYNC_DELTA_IN_MSEC;
        pDispPrm->chParams.syncThreshold = SYNC_DROP_THRESHOLD_IN_MSEC;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set DMA SW Mosaic Create Parameters
 *
 *          It is called in Create function.
 *          In this function SwMs alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.

 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetAlgDmaSwMsPrm(
        AlgorithmLink_DmaSwMsCreateParams *pPrm,
        UInt32 numLvdsCh,
        UInt32 orgVideoWinWidth,
        UInt32 orgVideoWinHeight,
        UInt32 orgVideoWinGap
)
{
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;

    if(pPrm)
    {
        UTILS_assert(numLvdsCh == 2);

        pPrm->maxOutBufWidth     = orgVideoWinWidth*2 + orgVideoWinGap;
        pPrm->maxOutBufHeight    = orgVideoWinHeight;
        pPrm->numOutBuf          = 4;
        pPrm->useLocalEdma       = FALSE;

        pPrm->initLayoutParams.numWin = 2;
        pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
        pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;


        pWinInfo = &pPrm->initLayoutParams.winInfo[0];
        pWinInfo->chId = 1;
        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;
        pWinInfo->width = orgVideoWinWidth;
        pWinInfo->height = orgVideoWinHeight;
        pWinInfo->outStartX = 0;
        pWinInfo->outStartY = 0;

        pWinInfo = &pPrm->initLayoutParams.winInfo[1];
        pWinInfo->chId = 0;
        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;
        pWinInfo->width = orgVideoWinWidth;
        pWinInfo->height = orgVideoWinHeight;
        pWinInfo->outStartX = pWinInfo->width + orgVideoWinGap;
        pWinInfo->outStartY = 0;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Display Create Parameters
 *
 *          This function is used to set the Display params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipStereoOnly_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [IN]    DisplayLink_CreateParams
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetDisplayPrms(
        DisplayLink_CreateParams *pPrm_OrgVideo,
        DisplayLink_CreateParams *pPrm_DisparityVideo,
        DisplayLink_CreateParams *pPrm_Grpx,
        UInt32 disparityWinStartX,
        UInt32 disparityWinStartY,
        UInt32 disparityWinWidth,
        UInt32 disparityWinHeight,
        UInt32 orgVideoWinStartX,
        UInt32 orgVideoWinStartY,
        UInt32 orgVideoWinWidth,
        UInt32 orgVideoWinHeight,
        UInt32 orgVideoWinGap
)
{
    if(pPrm_OrgVideo)
    {
        pPrm_OrgVideo->rtParams.tarWidth         = orgVideoWinWidth*2 + orgVideoWinGap;
        pPrm_OrgVideo->rtParams.tarHeight        = orgVideoWinHeight;
        pPrm_OrgVideo->rtParams.posX             = orgVideoWinStartX;
        pPrm_OrgVideo->rtParams.posY             = orgVideoWinStartY;
        pPrm_OrgVideo->displayId                 = DISPLAY_LINK_INST_DSS_VID3;
    }

    if(pPrm_DisparityVideo)
    {
        pPrm_DisparityVideo->rtParams.tarWidth  = disparityWinWidth;
        pPrm_DisparityVideo->rtParams.tarHeight = disparityWinHeight;
        pPrm_DisparityVideo->rtParams.posX      = disparityWinStartX;
        pPrm_DisparityVideo->rtParams.posY      = disparityWinStartY;
        pPrm_DisparityVideo->displayId          = DISPLAY_LINK_INST_DSS_VID2;
    }

    if(pPrm_Grpx)
    {
        pPrm_Grpx->displayId                  = DISPLAY_LINK_INST_DSS_GFX1;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Graphics Create Parameters
 *
 *
 *          This function is used to set the Grtaphics Link params.
 *          It is called in Create function.
 *
 * \param   pPrm         [IN]    GrpxSrcLink_CreateParams
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetGrpxSrcPrms(
        GrpxSrcLink_CreateParams *pPrm,
        UInt32 displayWidth,
        UInt32 displayHeight)
{
    if(pPrm)
    {
        pPrm->grpxBufInfo.dataFormat  = SYSTEM_DF_BGR16_565;
        pPrm->grpxBufInfo.height   = displayHeight;
        pPrm->grpxBufInfo.width    = displayWidth;

        pPrm->stereoDisparityLayoutEnable = TRUE;
        pPrm->statsDisplayEnable = TRUE;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_SoftIspCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetSoftIspAlgPrms(
        AlgorithmLink_SoftIspCreateParams *pPrm)
{

    pPrm->imageWidth    = CAPTURE_SENSOR_WIDTH;
    pPrm->imageHeight   = SOFTISP_CROPPED_INPUT_HEIGHT;

    pPrm->numOutBuffers = LINK_NUM_BUFFERS;
}

/**
 *******************************************************************************
 *
 * \brief   Set Remap Merge Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_SoftIspCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetRemapMergeAlgPrms(
        AlgorithmLink_RemapMergeCreateParams *pPrm,
        StereoImageDims *imDims)
{
    UInt32 qspiBufHeader[SYSTEM_QSPI_READ_WRITE_SIZE];

    pPrm->imageWidth    = CAPTURE_SENSOR_WIDTH;
    pPrm->imageHeight   = SOFTISP_OUTPUT_HEIGHT;

    pPrm->coordMapList        = 1;
    pPrm->isSrcMapFloat       = 0;
    pPrm->srcFormat           = 0;  // U8
    pPrm->dstFormat           = 0;  // U8
    pPrm->srcStride           = pPrm->roiWidth  = pPrm->dstStride = imDims->remapImageWidth;
    pPrm->srcHeight           = pPrm->roiHeight = imDims->remapImageHeight;
    pPrm->blockWidthBB        = REMAP_OUTPUT_BLOCK_WIDTH;
    pPrm->blockHeightBB       = REMAP_OUTPUT_BLOCK_HEIGHT;
    pPrm->blockWidthTile      = 80;  // dont bother
    pPrm->blockHeightTile     = 40;  // dont bother
    pPrm->tileWidth           = 40;  // dont bother
    pPrm->tileHeight          = 50;  // dont bother
    pPrm->enableMerge         = 0;
    pPrm->interpolationLuma   = 1;
    pPrm->interpolationChroma = 0;
    pPrm->mapQshift           = 2;
    pPrm->rightShift          = 0;
    pPrm->sat_high            = 255;
    pPrm->sat_high_set        = 255;
    pPrm->sat_low             = 0;
    pPrm->sat_low_set         = 0;

    pPrm->numOutBuffers = LINK_NUM_BUFFERS;
    /* Reading calibration tables from QSPI */
    /* Read the Header first */
    System_qspiReadSector((UInt32)qspiBufHeader,
            STEREO_CALIB_LUT_QSPI_OFFSET,
            SystemUtils_align(STEREO_CALIB_LUT_HEADER_SIZE, SYSTEM_QSPI_READ_WRITE_SIZE));

    if (STEREO_CALIB_LUT_TAG_ID == qspiBufHeader[0])
    {
        /* Read bin file size */
        pPrm->calibLUTBufPrms.calibLUTBufSize 
        = qspiBufHeader[1] - STEREO_CALIB_LUT_HEADER_SIZE;
        pPrm->calibLUTBufPrms.pCalibLUTBuf
        = Utils_memAlloc(
                UTILS_HEAPID_DDR_CACHED_SR,
                SystemUtils_align(pPrm->calibLUTBufPrms.calibLUTBufSize, SYSTEM_QSPI_READ_WRITE_SIZE), /* Padd with QSPI_READ_WRITE_SIZE as  System_qspiReadSector writes in granularity of QSPI_READ_WRITE_SIZE bytes*/
                32);
        UTILS_assert(pPrm->calibLUTBufPrms.pCalibLUTBuf!=NULL);
        pPrm->calibLUTBufPrms.isValid= TRUE;
        /* Read the binary file */
        System_qspiReadSector((UInt32)pPrm->calibLUTBufPrms.pCalibLUTBuf,
                STEREO_CALIB_LUT_QSPI_OFFSET+STEREO_CALIB_LUT_HEADER_SIZE,
                SystemUtils_align(pPrm->calibLUTBufPrms.calibLUTBufSize, SYSTEM_QSPI_READ_WRITE_SIZE));
    }
    else

    {
        pPrm->calibLUTBufPrms.isValid= FALSE;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_SoftIspCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetCensusAlgPrms(
        AlgorithmLink_CensusCreateParams *pPrm,
        StereoImageDims *imDims,
        Stereo_ConfigurableCreateParams *pStereoParams)
{
    pPrm->imageRoiWidth     = imDims->censusOutputRoiWidth;
    pPrm->imageRoiHeight     = imDims->censusOutputRoiHeight;

    pPrm->inputBitDepth     = CENSUS_INPUT_BIT_DEPTH;
    pPrm->winWidth          = pStereoParams->censusWinWidth;
    pPrm->winHeight         = pStereoParams->censusWinHeight;
    pPrm->winHorzStep       = pStereoParams->censusWinHorzStep;
    pPrm->winVertStep       = pStereoParams->censusWinVertStep;

    pPrm->srcImageWidth     = imDims->censusInputImageWidth;
    pPrm->srcImageHeight    = imDims->censusInputImageHeight;

    pPrm->numOutBuffers = LINK_NUM_BUFFERS;
}

/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_SoftIspCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetDisparityAlgPrms(
        AlgorithmLink_DisparityHamDistCreateParams *pPrm,
        StereoImageDims *imDims,
        Stereo_ConfigurableCreateParams *pStereoParams)
{
    pPrm->imageRoiWidth       = imDims->disparityOutputRoiWidth;
    pPrm->imageRoiHeight      = imDims->disparityOutputRoiHeight;
    pPrm->inputBitDepth     = DISPARITY_INPUT_BIT_DEPTH;
    pPrm->winWidth          = pStereoParams->disparitySupportWinWidth;
    pPrm->winHeight         = pStereoParams->disparitySupportWinHeight;
    pPrm->numDisparities    = pStereoParams->numDisparities;
    pPrm->disparityStep     = pStereoParams->disparityStepSize;
    pPrm->searchDir         = pStereoParams->disparitySearchDir;
    pPrm->extraRightLeftMap = pStereoParams->leftRightCheckEna;

    pPrm->srcImageWidth = imDims->disparityInputImageWidth;
    pPrm->srcImageHeight = imDims->disparityInputImageHeight;

    pPrm->inputLeftImageStartX= imDims->disparityInputLeftImageStartX;
    pPrm->inputLeftImageStartY= imDims->disparityInputLeftImageStartY;
    pPrm->inputRightImageStartX= imDims->disparityInputRightImageStartX;
    pPrm->inputRightImageStartY= imDims->disparityInputRightImageStartY;

    pPrm->numOutBuffers             = LINK_NUM_BUFFERS;
}

/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_SoftIspCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetStereoPostProcessPrms(
        AlgorithmLink_StereoPostProcessCreateParams *pPrm,
        StereoImageDims *imDims,
        Stereo_ConfigurableCreateParams *pStereoParams)
{
    pPrm->numOutBuffers             = LINK_NUM_BUFFERS;
    pPrm->maxImageRoiWidth      = imDims->disparityOutputRoiWidth;
    pPrm->maxImageRoiHeight     = imDims->disparityOutputRoiHeight;
    pPrm->inputBitDepth         = CENSUS_INPUT_BIT_DEPTH;
    pPrm->censusWinWidth        = pStereoParams->censusWinWidth;
    pPrm->censusWinHeight       = pStereoParams->censusWinHeight;
    pPrm->disparityWinWidth     = pStereoParams->disparitySupportWinWidth;
    pPrm->disparityWinHeight    = pStereoParams->disparitySupportWinHeight;
    pPrm->numDisparities    = pStereoParams->numDisparities;
    pPrm->disparityStep         = pStereoParams->disparityStepSize;
    pPrm->disparitySearchDir         = pStereoParams->disparitySearchDir;
    pPrm->costMaxThreshold        = POSTPROC_COST_MAX_THRESHOLD;
    pPrm->minConfidenceThreshold  = POSTPROC_CONF_MIN_THRSESHOLD;
    pPrm->holeFillingStrength  = POSTPROC_HOLEFILLING_STRENGTH;
    pPrm->textureLumaLoThresh= POSTPROC_TEXTURE_LUMALOTHRESH;
    pPrm->textureLumaHiThresh= POSTPROC_TEXTURE_LUMAHITHRESH;
    pPrm->textureThreshold= POSTPROC_TEXTURE_THRESHOLD;
    pPrm->lrMaxDiffThreshold= POSTPROC_LEFTRIGHT_MAXDIFF_THRESHOLD;
    pPrm->maxDispDissimilarity= POSTPROC_MAX_DISP_DISSIMILARITY;
    pPrm->minConfidentNSegment= POSTPROC_MIN_CONFIDENT_N_SEG;
    pPrm->censusSrcImageWidth = imDims->censusInputImageWidth;
    pPrm->censusSrcImageHeight = imDims->censusInputImageHeight;
    pPrm->temporalFilterNumFrames= POSTPROC_TEMPORAL_FILTER_NUM_FRAMES;
    pPrm->minDisparityToDisplay= POSTPROC_MIN_DISPARITY_DISPLAY;
    pPrm->colorMapIndex= pStereoParams->postproc_colormap_index;
    pPrm->disparityExtraRightLeft= pStereoParams->leftRightCheckEna;
    pPrm->imageStartX= imDims->origRoiStartX;
    pPrm->imageStartY= imDims->origRoiStartY;

}

/**
 *******************************************************************************
 *
 * \brief   Set link Parameters
 *
 *          It is called in Create function of the auto generated use-case file.
 *
 * \param pUcObj    [IN] Auto-generated usecase object
 * \param appObj    [IN] Application specific object
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_SetPrms(
        CaptureLink_CreateParams *pCapturePrm,
        VpeLink_CreateParams *pVPE_softispPrm,
        VpeLink_CreateParams *pVPE_orgdispPrm,
        VpeLink_CreateParams *pVPE_disparityPrm,
        AlgorithmLink_SoftIspCreateParams *pAlg_SoftIspPrm,
        AlgorithmLink_RemapMergeCreateParams *pAlg_RemapMergePrm,
        AlgorithmLink_CensusCreateParams *pAlg_CensusPrm,
        AlgorithmLink_DisparityHamDistCreateParams *pAlg_DisparityHamDistPrm,
        AlgorithmLink_StereoPostProcessCreateParams *pAlg_StereoPostProcessPrm,
        SyncLink_CreateParams *pSync_orgPrm,
        SyncLink_CreateParams *pSync_dispPrm,
        AlgorithmLink_DmaSwMsCreateParams *pAlg_DmaSwMsPrm,
        GrpxSrcLink_CreateParams *pGrpxSrcPrm,
        DisplayLink_CreateParams *pDisplay_orgPrm,
        DisplayLink_CreateParams *pDisplay_disparityPrm,
        DisplayLink_CreateParams *pDisplay_GrpxPrm,
        Chains_CaptureSrc captureSrc,
        Chains_DisplayType displayType,
        UInt32 captureSensorWidth,
        UInt32 captureSensorHeight,
        UInt32 remapWidth,
        UInt32 remapHeight,
        UInt32 stereoOutputWidth,
        UInt32 stereoOutputHeight,
        UInt32 disparityWinStartX,
        UInt32 disparityWinStartY,
        UInt32 disparityWinWidth,
        UInt32 disparityWinHeight,
        UInt32 orgVideoWinStartX,
        UInt32 orgVideoWinStartY,
        UInt32 orgVideoWinWidth,
        UInt32 orgVideoWinHeight,
        UInt32 orgVideoWinGap
)
{
    UInt32 captureOutWidth = 0, captureOutHeight = 0;
    UInt32 displayWidth = 0, displayHeight = 0;

    gStereoImDims.disparityOutputRoiWidth= stereoOutputWidth;
    gStereoImDims.disparityOutputRoiHeight= stereoOutputHeight;
    gStereoImDims.remapImageWidth= remapWidth;
    gStereoImDims.remapImageHeight= remapHeight;
    ChainsCommon_Stereo_initImageDims(&gStereoImDims, &gStereoParams);

    if(displayType==CHAINS_DISPLAY_TYPE_HDMI_720P)
    {
        captureOutWidth  = captureSensorWidth;
        captureOutHeight = captureSensorHeight;
        displayWidth     = 1280;
        displayHeight    = 720;
    }
    else
        if(displayType==CHAINS_DISPLAY_TYPE_HDMI_1080P)
        {
            captureOutWidth  = captureSensorWidth;
            captureOutHeight = captureSensorHeight;
            displayWidth     = 1920;
            displayHeight    = 1080;
        }

    if(pCapturePrm)
    {
        ChainsCommon_StereoCam_SetCapturePrms(pCapturePrm,
                captureSensorWidth,
                captureSensorHeight,
                captureOutWidth,
                captureOutHeight,
                captureSrc
        );
    }

    ChainsCommon_Stereo_SetVpePrm(pVPE_softispPrm,
            pVPE_orgdispPrm,
            pVPE_disparityPrm,
            &gStereoImDims,
            2,
            disparityWinWidth,
            disparityWinHeight,
            orgVideoWinWidth,
            orgVideoWinHeight
    );

    ChainsCommon_Stereo_SetSyncPrm(
            pSync_orgPrm,
            pSync_dispPrm,
            2
    );

    if(pAlg_DmaSwMsPrm)
        ChainsCommon_Stereo_SetAlgDmaSwMsPrm(
                pAlg_DmaSwMsPrm,
                2,
                orgVideoWinWidth,
                orgVideoWinHeight,
                orgVideoWinGap
        );

    ChainsCommon_Stereo_SetGrpxSrcPrms(pGrpxSrcPrm,
            displayWidth,
            displayHeight
    );

    ChainsCommon_Stereo_SetDisplayPrms(pDisplay_orgPrm,
            pDisplay_disparityPrm,
            pDisplay_GrpxPrm,
            disparityWinStartX,
            disparityWinStartY,
            disparityWinWidth,
            disparityWinHeight,
            orgVideoWinStartX,
            orgVideoWinStartY,
            orgVideoWinWidth,
            orgVideoWinHeight,
            orgVideoWinGap
    );

    if(pAlg_SoftIspPrm)
        ChainsCommon_Stereo_SetSoftIspAlgPrms (pAlg_SoftIspPrm);

    if(pAlg_RemapMergePrm)
        ChainsCommon_Stereo_SetRemapMergeAlgPrms (pAlg_RemapMergePrm, &gStereoImDims);

    if(pAlg_CensusPrm)
        ChainsCommon_Stereo_SetCensusAlgPrms (pAlg_CensusPrm, &gStereoImDims,&gStereoParams);

    if(pAlg_DisparityHamDistPrm)
        ChainsCommon_Stereo_SetDisparityAlgPrms (pAlg_DisparityHamDistPrm, &gStereoImDims, &gStereoParams);

    if(pAlg_StereoPostProcessPrm)
        ChainsCommon_Stereo_SetStereoPostProcessPrms (pAlg_StereoPostProcessPrm, &gStereoImDims, &gStereoParams);
}


/**
 *******************************************************************************
 *
 * \brief   Delete 
 *
 *
 *
 * \param pAlg_RemapMergePrm, Remap alg plugin create params. These contain the allocated
 * LUT param
 *
 *******************************************************************************
 */
Void ChainsCommon_Stereo_Delete(
        AlgorithmLink_RemapMergeCreateParams *pAlg_RemapMergePrm)
{
    if(pAlg_RemapMergePrm->calibLUTBufPrms.isValid	)
    {
        Utils_memFree(
                UTILS_HEAPID_DDR_CACHED_SR,
                pAlg_RemapMergePrm->calibLUTBufPrms.pCalibLUTBuf,
                pAlg_RemapMergePrm->calibLUTBufPrms.calibLUTBufSize);
    }
}
