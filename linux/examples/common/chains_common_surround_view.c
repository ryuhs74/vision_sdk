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
#include <linux/examples/common/chains_common.h>
#include <examples/tda2xx/include/chains_main_srv_calibration.h>
#include <osa_file.h>

/*
 *******************************************************************************
 *
 * Surround view use-case parameters
 *
 *******************************************************************************
 */
#define LVDS_CAPTURE_WIDTH              (1280)
#define LVDS_CAPTURE_HEIGHT             (720)

#define SV_OUTPUT_WIDTH                 (880) //(720)
#define SV_OUTPUT_HEIGHT                (1080)
#define SV_OUTPUT_POSX                  (25+320+10)
#define SV_OUTPUT_POSY                  (0)

#define SV_INPUT_WIDTH                  (LVDS_CAPTURE_WIDTH)
#define SV_INPUT_HEIGHT                 (LVDS_CAPTURE_HEIGHT)
#define SV_NUM_VIEWS                    (4)

#define SVORG_SCALED_WIDTH              (LVDS_CAPTURE_WIDTH/4)
#define SVORG_SCALED_HEIGHT             (LVDS_CAPTURE_HEIGHT/4)
#define SVORG_MOSAIC_SPACING_HOR        (0)
#define SVORG_MOSAIC_SPACING_VER        (35)
#define SVORG_MOSAIC_WIDTH              (SVORG_SCALED_WIDTH)
#define SVORG_MOSAIC_HEIGHT             ((SVORG_SCALED_HEIGHT)*SV_NUM_VIEWS + SVORG_MOSAIC_SPACING_VER*(SV_NUM_VIEWS-1))
#define SVORG_MOSAIC_POSX               (25)
#define SVORG_MOSAIC_POSY               (100+35)

#define FRONTCAM_SCALED_WIDTH           (LVDS_CAPTURE_WIDTH/2)
#define FRONTCAM_SCALED_HEIGHT          (LVDS_CAPTURE_HEIGHT/2)
#define FRONTCAM_MOSAIC_SPACING_HOR     (0)
#define FRONTCAM_MOSAIC_SPACING_VER     (50)
#define FRONTCAM_MOSAIC_WIDTH           (FRONTCAM_SCALED_WIDTH)
#define FRONTCAM_MOSAIC_HEIGHT          ((FRONTCAM_SCALED_HEIGHT*2)+FRONTCAM_MOSAIC_SPACING_VER)
#define FRONTCAM_MOSAIC_POSX            (1920-25-640)
#define FRONTCAM_MOSAIC_POSY            (100+50)

#define DOF_WIDTH_ALIGN                 (64)
#define DOF_HEIGHT_ALIGN                (32)

#define FEATUREPLANE_ALG_WIDTH          (640)
#define FEATUREPLANE_ALG_HEIGHT         (360)

#define FEATUREPLANE_NUM_OUT_BUF        (8)

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define TIGHT_SYNC_DELTA_IN_MSEC              (16)
#define LOOSE_SYNC_DELTA_IN_MSEC              (0x7FFFFFFF)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define TIGHT_SYNC_DROP_THRESHOLD_IN_MSEC     (33)
#define LOOSE_SYNC_DROP_THRESHOLD_IN_MSEC     (0x7FFFFFFF)

/**
 *******************************************************************************
 * \brief Menu for Camera position Calibration settings.
 *******************************************************************************
 */
char gChainsCommon_SurroundView_MenuCalibration[] = {
    "\r\n "
    "\r\n ================================================"
    "\r\n Chains Run-time Camera position Calibration Menu"
    "\r\n ================================================"
    "\r\n "
    "\r\n 0: Run with GA LUT from CalibData File (If not available, use default table)"
    "\r\n    File I/O will take place now, will take a couple of minutes"
    "\r\n "
    "\r\n 1: Force GA Calibration - Regenerate calibration tbl"
    "\r\n    Calibration & File I/O will take a couple of minutes"
    "\r\n "
    "\r\n 2: Erase entire calibration tbl from CalibData File"
    "\r\n    File I/O will take a couple of minutes"
    "\r\n "
    "\r\n 3: Go to previous menu"
    "\r\n    File I/O will take place now, will take a couple of minutes"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 * \brief   Run time Menu selection for the Calibration options
 *
 *          This functions set/implement follwing
 *          - Set calibartion option enable
 *          - Set calibartion option disable
 *          - Set QSPI Flash erase option
 *
 * \param   gaCalibInfo    [IN]  ChainsCommon_SurroundView_GACalibrationInfo
 *
 *******************************************************************************
*/
char ChainsCommon_SurroundView_MenuCalibration(
                  Chain_Common_SRV_CalibParams * gaCalibPrm)
{
    char ch;
    UInt32 done = FALSE;

    memset (gaCalibPrm, 0, sizeof (Chain_Common_SRV_CalibParams));
    gaCalibPrm->calibState = CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_RUNTIME;

    while(!done)
    {
        Task_sleep(500);
        Vps_printf(gChainsCommon_SurroundView_MenuCalibration);
        Task_sleep(500);

        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                gaCalibPrm->calType = CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE;
                done = TRUE;
                break;
            case '2':
                gaCalibPrm->calType = CHAIN_COMMON_SRV_GA_ERASE_ENTIRE_TABLE;
                done = TRUE;
                break;
            case '0':
            case '3':
                gaCalibPrm->calType = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;
                done = TRUE;
                break;
        }
    }

    Chain_Common_SRV_Calibration(gaCalibPrm);
    return ch;
}

Void ChainsCommon_SurroundView_InitCalibration(
                  Chain_Common_SRV_CalibParams * gaCalibPrm,
                  Bool startWithCalibration)
{
    /* Ensure that the CalibData file is present*/
    OSA_fileCreateFile(
            CALIBDATA_FILENAME,
            GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET + GA_PERSPECTIVE_MATRIX_SIZE,
            FALSE);

    memset (gaCalibPrm, 0, sizeof (Chain_Common_SRV_CalibParams));
    gaCalibPrm->calibState = CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_CREATETIME;
    gaCalibPrm->startWithCalibration = startWithCalibration;

    Chain_Common_SRV_Calibration(gaCalibPrm);

}

Void ChainsCommon_SurroundView_StopCalibration(
                  Chain_Common_SRV_CalibParams * gaCalibPrm)
{

    memset (gaCalibPrm, 0, sizeof (Chain_Common_SRV_CalibParams));
    gaCalibPrm->calibState = CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_DELETETIME;

    Chain_Common_SRV_Calibration(gaCalibPrm);

}

Void ChainsCommon_SurroundView_SetSynthParams(
                                    AlgorithmLink_SynthesisCreateParams *pPrm,
                                    UInt16 svInWidth,
                                    UInt16 svInHeight,
                                    UInt16 svOutWidth,
                                    UInt16 svOutHeight,
                                    UInt16 svNumViews,
                                    Int16  svCarBoxWidth,
                                    Int16  svCarBoxHeight,
                                    AlgorithmLink_SrvOutputModes svOutputMode,
                                    Bool enableCarOverlayInAlg)
{
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_SYNTHESIS;
    pPrm->maxOutputWidth = svOutWidth;
    pPrm->maxOutputHeight = svOutHeight;
    pPrm->maxInputWidth = svInWidth;
    pPrm->maxInputHeight = svInHeight;
    pPrm->numViews = svNumViews;
    pPrm->carBoxWidth = svCarBoxWidth;
    pPrm->carBoxHeight = svCarBoxHeight;
    pPrm->numOutputFrames = 5;
    pPrm->numPhotometricStatisticsTables = 5;
    pPrm->numSgxBlendLUTables = 1;
    pPrm->synthesisMode = ALGORITHM_LINK_ALG_SIMPLESYNTHESIS;
    pPrm->svOutputMode = svOutputMode; //2D or 3D SRV
    pPrm->enableCarOverlayInAlg = enableCarOverlayInAlg;
}


Void ChainsCommon_SurroundView_SetGAlignParams(
                            AlgorithmLink_GAlignCreateParams *pPrm,
                            UInt16 svInWidth,
                            UInt16 svInHeight,
                            UInt16 svOutWidth,
                            UInt16 svOutHeight,
                            UInt16 svNumViews,
                            Int16  svCarBoxWidth,
                            Int16  svCarBoxHeight,
                            AlgorithmLink_SrvOutputModes svOutputMode,
                            Chain_Common_SRV_CalibParams *gaCalibPrm)
{
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_GALIGNMENT;
    pPrm->maxOutputWidth = svOutWidth;
    pPrm->maxOutputHeight = svOutHeight;
    pPrm->maxInputWidth = svInWidth;
    pPrm->maxInputHeight = svInHeight;
    pPrm->numViews = svNumViews;
    pPrm->carBoxWidth = svCarBoxWidth;
    pPrm->carBoxHeight = svCarBoxHeight;

    pPrm->numOutputTables = 3;
    pPrm->enablePixelsPerCm = 0;
    pPrm->svOutputMode = svOutputMode; //2D or 3D SRV
    pPrm->ignoreFirstNFrames = 2U;
    pPrm->defaultFocalLength = 455U;

    OSA_assert(gaCalibPrm->gaLUTAddr != 0);
    OSA_assert(gaCalibPrm->persMatAddr != 0);
    pPrm->calParams.calMode = gaCalibPrm->calMode;
    pPrm->calParams.gaLUTDDRPtr =
          (void *)(gaCalibPrm->gaLUTAddr + GA_MAGIC_PATTERN_SIZE_IN_BYTES);
    pPrm->calParams.persMatDDRPtr =
          (void *)(gaCalibPrm->persMatAddr + GA_MAGIC_PATTERN_SIZE_IN_BYTES);
}

Void ChainsCommon_SurroundView_SetPAlignParams(
                                    AlgorithmLink_PAlignCreateParams *pPrm,
                                    UInt16 svInWidth,
                                    UInt16 svInHeight,
                                    UInt16 svOutWidth,
                                    UInt16 svOutHeight,
                                    UInt16 svNumViews,
                                    Int16  svCarBoxWidth,
                                    Int16  svCarBoxHeight,
                                    AlgorithmLink_SrvOutputModes svOutputMode
                                    )
{
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_PALIGNMENT;
    pPrm->maxOutputWidth = svOutWidth;
    pPrm->maxOutputHeight = svOutHeight;
    pPrm->maxInputWidth = svInWidth;
    pPrm->maxInputHeight = svInHeight;
    pPrm->numViews = svNumViews;
    pPrm->carBoxWidth = svCarBoxWidth;
    pPrm->carBoxHeight = svCarBoxHeight;
    pPrm->numOutputTables = 5;
    pPrm->dataFormat = SYSTEM_DF_YUV420SP_UV;
    pPrm->svOutputMode = svOutputMode; //2D or 3D SRV
}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 * \param   syncMode [IN]    1 - Tight Sync, 0 - Loose Sync
 *          pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetSyncPrm(
                    SyncLink_CreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 syncMode
                    )
{
    UInt16 chId;

    pPrm->chParams.numCh = numLvdsCh;
    pPrm->chParams.numActiveCh = pPrm->chParams.numCh;
    for(chId = 0; chId < pPrm->chParams.numCh; chId++)
    {
        pPrm->chParams.channelSyncList[chId] = TRUE;
    }

    if(syncMode == 1)
    {
        pPrm->chParams.syncDelta = TIGHT_SYNC_DELTA_IN_MSEC;
        pPrm->chParams.syncThreshold = TIGHT_SYNC_DROP_THRESHOLD_IN_MSEC;
    }
    else
    {
        pPrm->chParams.syncDelta = LOOSE_SYNC_DELTA_IN_MSEC;
        pPrm->chParams.syncThreshold = LOOSE_SYNC_DROP_THRESHOLD_IN_MSEC;
    }

}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 * \param   syncMode [IN]    1 - Tight Sync, 0 - Loose Sync
 *          pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetSyncFcPrm(
                    SyncLink_CreateParams *pPrm
                    )
{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 16;
    pPrm->chParams.syncThreshold = 0xFFFF;
}
#if 0
/**
 *******************************************************************************
 *
 * \brief   Set Display Create Parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetDisplayPrm(
                            DisplayLink_CreateParams *pSvDisplay,
                            DisplayLink_CreateParams *pSvOrgDisplay,
                            DisplayLink_CreateParams *pFrontCamDisplay,
                            DisplayLink_CreateParams *pGrpxDisplay,
                            UInt32 displayWidth,
                            UInt32 displayHeight
                            )
{
    UInt32 displayWidthScale;
    UInt32 displayHeightScale;

    displayWidthScale = (1920)/displayWidth;
    displayHeightScale = (1080)/displayHeight;

    if(pSvDisplay)
    {
        pSvDisplay->rtParams.tarWidth         = SV_OUTPUT_WIDTH / displayWidthScale;
        pSvDisplay->rtParams.tarHeight        = SV_OUTPUT_HEIGHT / displayHeightScale;
        pSvDisplay->rtParams.posX             = SV_OUTPUT_POSX / displayWidthScale;
        pSvDisplay->rtParams.posY             = SV_OUTPUT_POSY / displayHeightScale;
        pSvDisplay->displayId                 = DISPLAY_LINK_INST_DSS_VID1;
    }
    if(pSvOrgDisplay)
    {
        pSvOrgDisplay->rtParams.tarWidth      = SVORG_MOSAIC_WIDTH / displayWidthScale;
        pSvOrgDisplay->rtParams.tarHeight     = SVORG_MOSAIC_HEIGHT / displayHeightScale;
        pSvOrgDisplay->rtParams.posX          = SVORG_MOSAIC_POSX / displayWidthScale;
        pSvOrgDisplay->rtParams.posY          = SVORG_MOSAIC_POSY / displayHeightScale;
        pSvOrgDisplay->displayId              = DISPLAY_LINK_INST_DSS_VID2;
    }
    if(pFrontCamDisplay)
    {
        pFrontCamDisplay->rtParams.tarWidth   = FRONTCAM_MOSAIC_WIDTH / displayWidthScale;
        pFrontCamDisplay->rtParams.tarHeight  = FRONTCAM_MOSAIC_HEIGHT / displayHeightScale;
        pFrontCamDisplay->rtParams.posX       = FRONTCAM_MOSAIC_POSX / displayWidthScale;
        pFrontCamDisplay->rtParams.posY       = FRONTCAM_MOSAIC_POSY / displayHeightScale;
        pFrontCamDisplay->displayId           = DISPLAY_LINK_INST_DSS_VID3;
    }
    if(pGrpxDisplay)
    {
        pGrpxDisplay->rtParams.tarWidth       = displayWidth;
        pGrpxDisplay->rtParams.tarHeight      = displayHeight;
        pGrpxDisplay->rtParams.posX           = 0;
        pGrpxDisplay->rtParams.posY           = 0;
        pGrpxDisplay->displayId               = DISPLAY_LINK_INST_DSS_GFX1;
    }
}
#endif
/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          Chains_lvdsMultiVipCaptureDisplay_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [OUT]    DisplayLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetVpePrm(
                        VpeLink_CreateParams *pSvOrgVpe,
                        VpeLink_CreateParams *pFrontCamVpe,
                        UInt32 frontCamAlgOutWidth,
                        UInt32 frontCamAlgOutHeight
                    )
{
    VpeLink_CreateParams *pPrm;
    UInt32 chId;
    VpeLink_ChannelParams *chPrms;
    UInt32 outId = 0;

    if(pSvOrgVpe)
    {
        pPrm = pSvOrgVpe;
        pPrm->enableOut[0] = TRUE;
        for (chId = 0; chId < 4; chId++)
        {
            chPrms = &pPrm->chParams[chId];
            chPrms->outParams[outId].numBufsPerCh =
                                     VPE_LINK_NUM_BUFS_PER_CH_DEFAULT;

            chPrms->outParams[outId].width = SVORG_SCALED_WIDTH;
            chPrms->outParams[outId].height = SVORG_SCALED_HEIGHT;
            chPrms->outParams[outId].dataFormat = SYSTEM_DF_YUV420SP_UV;

            chPrms->scCfg.bypass       = FALSE;
            chPrms->scCfg.nonLinear    = FALSE;
            chPrms->scCfg.stripSize    = 0;

            chPrms->scCropCfg.cropStartX = 0;
            chPrms->scCropCfg.cropStartY = 0;
            chPrms->scCropCfg.cropWidth = LVDS_CAPTURE_WIDTH;
            chPrms->scCropCfg.cropHeight = LVDS_CAPTURE_HEIGHT;
        }
    }
    if(pFrontCamVpe)
    {
        pPrm = pFrontCamVpe;
        pPrm->enableOut[0] = TRUE;
        for (chId = 0; chId < 2; chId++)
        {
            chPrms = &pPrm->chParams[chId];
            chPrms->outParams[outId].numBufsPerCh =
                                     VPE_LINK_NUM_BUFS_PER_CH_DEFAULT;

            chPrms->outParams[outId].width = FRONTCAM_SCALED_WIDTH;
            chPrms->outParams[outId].height = FRONTCAM_SCALED_HEIGHT;
            chPrms->outParams[outId].dataFormat = SYSTEM_DF_YUV420SP_UV;

            chPrms->scCfg.bypass       = FALSE;
            chPrms->scCfg.nonLinear    = FALSE;
            chPrms->scCfg.stripSize    = 0;


            chPrms->scCropCfg.cropStartX = 0;
            chPrms->scCropCfg.cropStartY = 0;
            chPrms->scCropCfg.cropWidth = LVDS_CAPTURE_WIDTH;
            chPrms->scCropCfg.cropHeight = LVDS_CAPTURE_HEIGHT;

            if(chId==1)
            {
                /* CH1 is front cam algorithm output,
                 * its size depends on the algo that is used
                 *
                 * For Edge detect, output size is sensor W/2 x H/2
                 * For Dense Optical flow, output size is sensor W x H
                 *   aligned to algo specific values
                 */
                chPrms->scCropCfg.cropWidth = frontCamAlgOutWidth;
                chPrms->scCropCfg.cropHeight = frontCamAlgOutHeight;
            }
        }
    }
}

static void ChainsCommon_SurroundView_SetSelectPrm(SelectLink_CreateParams *pPrm)
{
    pPrm->numOutQue = 2;

    pPrm->outQueChInfo[0].outQueId   = 0;
    pPrm->outQueChInfo[0].numOutCh   = 4;
    pPrm->outQueChInfo[0].inChNum[0] = 0;
    pPrm->outQueChInfo[0].inChNum[1] = 1;
    pPrm->outQueChInfo[0].inChNum[2] = 2;
    pPrm->outQueChInfo[0].inChNum[3] = 3;

    pPrm->outQueChInfo[1].outQueId   = 1;
    pPrm->outQueChInfo[1].numOutCh   = 1;
    pPrm->outQueChInfo[1].inChNum[0] = 4;
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
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 channelWidth,
                    UInt32 channelHeight,
                    UInt32 layoutType,
                    UInt32 channelSpacingHor,
                    UInt32 channelSpacingVer
                   )
{
    UInt32 algId, winId;
    UInt32 useLocalEdma;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;
    UInt32 secondRowFlag, numLvdsChBy2;

    useLocalEdma = FALSE;
    algId = ALGORITHM_LINK_IPU_ALG_DMA_SWMS;

    pPrm->baseClassCreate.algId   = algId;
    pPrm->numOutBuf               = 4;
    pPrm->useLocalEdma            = useLocalEdma;
    pPrm->initLayoutParams.numWin = numLvdsCh;

    switch(layoutType)
    {
        default:
        case 0:
             /*
              * vertical strip
              */
            pPrm->maxOutBufWidth     = channelWidth;
            pPrm->maxOutBufHeight    = (channelHeight*(numLvdsCh)) +
                                       (channelSpacingVer*(numLvdsCh-1));

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                pWinInfo->outStartX = 0;
                pWinInfo->outStartY = winId*(channelHeight+channelSpacingVer);
             }

            break;
        case 1:
             /*
              * Horizontal strip
              */
            pPrm->maxOutBufWidth     = (channelWidth*(numLvdsCh)) +
                                       (channelSpacingHor*(numLvdsCh-1));
            pPrm->maxOutBufHeight    = channelHeight;

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                pWinInfo->outStartX = winId*(channelWidth+channelSpacingHor);
                pWinInfo->outStartY = 0;
             }

            break;

        case 2:
             /*
              * Two Horizontal strips
              */
            numLvdsChBy2 = (numLvdsCh+1) / 2;
            pPrm->maxOutBufWidth     = (channelWidth*(numLvdsChBy2)) +
                                       (channelSpacingHor*(numLvdsChBy2-1));
            pPrm->maxOutBufHeight    = (channelHeight*2) + channelSpacingVer;

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                secondRowFlag = ( winId>= numLvdsChBy2 ? 1 : 0);
                pWinInfo->outStartX = (winId % numLvdsChBy2) *(channelWidth+channelSpacingHor);
                pWinInfo->outStartY = secondRowFlag * (channelHeight+channelSpacingVer);
             }

            break;

    }

    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

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
static Void ChainsCommon_SurroundView_SetGrpxSrcPrms(
                                           GrpxSrcLink_CreateParams *pPrm,
                                           UInt32 displayWidth,
                                           UInt32 displayHeight)
{
    pPrm->grpxBufInfo.dataFormat  = SYSTEM_DF_BGR16_565;
    pPrm->grpxBufInfo.height   = displayHeight;
    pPrm->grpxBufInfo.width    = displayWidth;

    pPrm->surroundViewStandaloneLayoutEnable = TRUE;
    pPrm->statsDisplayEnable = TRUE;
    pPrm->enableJeepOverlay = FALSE;
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
 * \param   pPrm    [IN]    AlgorithmLink_EdgeDetectionCreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetEdgeDetectionAlgPrms(
                                     AlgorithmLink_EdgeDetectionCreateParams *pPrm,
                                     UInt32 maxWidth,
                                     UInt32 maxHeight)
{
    pPrm->baseClassCreate.size  = sizeof(AlgorithmLink_EdgeDetectionCreateParams);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_EDGEDETECTION;
    pPrm->maxWidth    = maxWidth;
    pPrm->maxHeight   = maxHeight;
    pPrm->numOutputFrames = 4;
}

/**
 *******************************************************************************
 *
 * \brief   Set Dense Optical Flow parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetDenseOptFlowAlgPrms(
                       AlgorithmLink_DenseOptFlowCreateParams *pPrm,
                       UInt32 width,
                       UInt32 height,
                       UInt32 eveId,
                       UInt32 numOfEve,
                       AlgorithmLink_DenseOptFlowLKnumPyr numPyramids)
{
    AlgorithmLink_DenseOptFlowCreateParams_Init(pPrm);

    pPrm->processPeriodicity = numOfEve;
    pPrm->processStartFrame  = eveId;
    pPrm->roiParams.width    = width;
    pPrm->roiParams.height   = height;
    pPrm->algEnable          = TRUE;
    pPrm->numOutBuf          = 4;

    pPrm->numPyramids     = numPyramids;
    pPrm->enableSmoothing = TRUE;
    pPrm->smoothingSize   = ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_5x5;
    pPrm->maxVectorSizeX  = 16;
    pPrm->maxVectorSizeY  = 16;
}

/**
 *******************************************************************************
 *
 * \brief   Set Vector to image algorithm parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetVectorToImageAlgPrms(
                       AlgorithmLink_VectorToImageCreateParams *pPrm,
                       UInt32 width,
                       UInt32 height,
                       UInt32 lutId,
                       Bool isLutSize_129x129,
                       UInt32 numOfEve)
{
    AlgorithmLink_VectorToImageCreateParams_Init(pPrm);

    pPrm->maxWidth  = width;
    pPrm->maxHeight = height;
    pPrm->numOutputFrames = 5;
    pPrm->lutId   = lutId;
    pPrm->isLutSize_129x129   = isLutSize_129x129;
    pPrm->dataFormat = SYSTEM_DF_YUV422I_YUYV;
}

/**
 *******************************************************************************
 *
 * \brief   Set PD draw parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetObjectDrawPrms(
                   AlgorithmLink_ObjectDrawCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    pPrm->imgFrameWidth    = width;
    pPrm->imgFrameHeight   = height;
    pPrm->numOutBuffers = FEATUREPLANE_NUM_OUT_BUF;
    pPrm->pdRectThickness = 3;
}

/**
 *******************************************************************************
 *
 * \brief   Set Feature Plane Compute Alg parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetFeaturePlaneComputeAlgPrms(
                   AlgorithmLink_FeaturePlaneComputationCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    pPrm->imgFrameHeight = height;
    pPrm->imgFrameWidth  = width;
    pPrm->numOutBuffers  = FEATUREPLANE_NUM_OUT_BUF;
}

/**
 *******************************************************************************
 *
 * \brief   Set Feature Plane Classify Alg parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetObjectDetectPrm(
                   AlgorithmLink_ObjectDetectionCreateParams *pPrm)
{
    pPrm->numOutBuffers  = FEATUREPLANE_NUM_OUT_BUF;
    pPrm->enablePD       = TRUE;
    pPrm->enableTSR      = TRUE;
}

/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetFCVpePrm(
                    VpeLink_CreateParams *pPrm,
                    UInt32 outWidth,
                    UInt32 outHeight,
                    UInt32 srcWidth,
                    UInt32 srcHeight,
                    System_VideoDataFormat dataFormat
                    )
{
    pPrm->enableOut[0] = TRUE;

    pPrm->chParams[0].outParams[0].width = SystemUtils_floor(outWidth, 4);
    pPrm->chParams[0].outParams[0].height = SystemUtils_floor(outHeight, 2);
    pPrm->chParams[0].outParams[0].numBufsPerCh = 3;

    pPrm->chParams[0].scCropCfg.cropStartX = 0;
    pPrm->chParams[0].scCropCfg.cropStartY = 0;
    pPrm->chParams[0].scCropCfg.cropWidth  = srcWidth;
    pPrm->chParams[0].scCropCfg.cropHeight = srcHeight;

    pPrm->chParams[0].outParams[0].dataFormat = dataFormat;
    pPrm->chParams[0].outParams[0].numBufsPerCh = 8;
}

/**
 *******************************************************************************
 *
 * \brief   Set Algorithm related parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetLaneDetectPrm(
                    AlgorithmLink_LaneDetectCreateParams *pAlgPrm,
                    AlgorithmLink_LaneDetectDrawCreateParams *pDrawPrm,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height
                    )
{
    pAlgPrm->imgFrameStartX = startX;
    pAlgPrm->imgFrameStartY = startY;
    pAlgPrm->imgFrameWidth  = width;
    pAlgPrm->imgFrameHeight = height;

    pAlgPrm->roiStartX      = 32 - LD_FILTER_TAP_X;
    pAlgPrm->roiStartY      = 120;
    pAlgPrm->roiWidth       = 576 + 2*LD_FILTER_TAP_X;
    pAlgPrm->roiHeight      = 240;

    pDrawPrm->imgFrameStartX = startX;
    pDrawPrm->imgFrameStartY = startY;
    pDrawPrm->imgFrameWidth  = width;
    pDrawPrm->imgFrameHeight  = height;
    pDrawPrm->enableDrawLines = TRUE;

    pAlgPrm->cannyHighThresh        = 30;
    pAlgPrm->cannyLowThresh         = 20;
    pAlgPrm->houghNmsThresh         = 20;
    pAlgPrm->startThetaLeft         = 100;
    pAlgPrm->endThetaLeft           = 150;
    pAlgPrm->startThetaRight        = 10;
    pAlgPrm->endThetaRight          = 60;
    pAlgPrm->thetaStepSize          = 1;
}

Void ChainsCommon_3DSurroundView_SetParams(
                            CaptureLink_CreateParams *pVipCapture,
                            AvbRxLink_CreateParams *pAvbRxPrm,
                            DecLink_CreateParams *pDecPrm,
                            SelectLink_CreateParams *pCaptureSelect,
                            VpeLink_CreateParams *pSvOrgVpe,
                            VpeLink_CreateParams *pFrontCamVpe,
                            SyncLink_CreateParams *pSvSync,
                            SyncLink_CreateParams *pSvOrgSync,
                            SyncLink_CreateParams *pFrontCamSync,
                            AlgorithmLink_SynthesisCreateParams *pSynthPrm,
                            AlgorithmLink_GAlignCreateParams *pGAlignPrm,
                            AlgorithmLink_PAlignCreateParams *pPAlignPrm,
                            AlgorithmLink_EdgeDetectionCreateParams *pEdgeDetect,
                            AlgorithmLink_DenseOptFlowCreateParams *pDof,
                            AlgorithmLink_VectorToImageCreateParams *pDofVectorToImage,
                            AlgorithmLink_DmaSwMsCreateParams *pSvOrgDmaSwMs,
                            AlgorithmLink_DmaSwMsCreateParams *pFrontCamDmaSwMs,
                            GrpxSrcLink_CreateParams *pGrpxSrc,
                            DisplayLink_CreateParams *pSvDisplay,
                            DisplayLink_CreateParams *pSvOrgDisplay,
                            DisplayLink_CreateParams *pFrontCamDisplay,
                            DisplayLink_CreateParams *pGrpxDisplay,
                            Chains_DisplayType displayType,
                            UInt32 numLvdsCh,
                            AlgorithmLink_SrvOutputModes svOutputMode,
                            VpeLink_CreateParams *VPE_algPdPrm,
                            AlgorithmLink_FeaturePlaneComputationCreateParams *Alg_FeaturePlaneComputationPrm,
                            AlgorithmLink_ObjectDetectionCreateParams *Alg_ObjectDetectionPrm,
                            SyncLink_CreateParams *Sync_algPdPrm,
                            AlgorithmLink_ObjectDrawCreateParams *Alg_ObjectDrawPrm,
                            SyncLink_CreateParams *Sync_algLdPrm,
                            AlgorithmLink_LaneDetectCreateParams *Alg_LaneDetectPrm,
                            AlgorithmLink_LaneDetectDrawCreateParams *Alg_LaneDetectDrawPrm,
                            Chain_Common_SRV_CalibParams *gaCalibPrm,
                            Bool enableCarOverlayInAlg
)
{
    //UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];
    UInt32 displayWidth, displayHeight;
    UInt32 frontCamAlgOutWidth, frontCamAlgOutHeight;

    /* by default assume front cam WxH == sensor WxH
     * But this will be overridden later in the logic below
     */
    frontCamAlgOutWidth  = LVDS_CAPTURE_WIDTH;
    frontCamAlgOutHeight = LVDS_CAPTURE_HEIGHT;

    UTILS_assert(pSynthPrm!=NULL);
    UTILS_assert(pGAlignPrm!=NULL);
    UTILS_assert(pPAlignPrm!=NULL);
    //UTILS_assert(pSvSync!=NULL);

    (void) pVipCapture;
    (void) pAvbRxPrm;
    displayWidth = 1920;
    displayHeight = 1080;
#if 0
    ChainsCommon_GetDisplayWidthHeight(
        displayType,
        &displayWidth,
        &displayHeight
        );

    if(pVipCapture)
    {
        ChainsCommon_MultiCam_StartCaptureDevice(
            CHAINS_CAPTURE_SRC_OV10635,
            portId,
            numLvdsCh
            );

        ChainsCommon_MultiCam_SetCapturePrms(pVipCapture,
                LVDS_CAPTURE_WIDTH,
                LVDS_CAPTURE_HEIGHT,
                portId,
                numLvdsCh
                );
        {
            UInt32 i;
            CaptureLink_VipInstParams *pInstPrm;
            for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
            {
                pInstPrm = &pVipCapture->vipInst[i];
                pInstPrm->numBufs = 6;
            }
            /* skip alternate frame to make it 15fps output for Front camera */
            pVipCapture->vipInst[4].outParams[0].frameSkipMask
                = 0x2AAAAAAA;
        }
    }

    if(pAvbRxPrm && pDecPrm)
    {
        ChainsCommon_SetAvbRxDecodePrm(
            pAvbRxPrm,
            pDecPrm,
            LVDS_CAPTURE_WIDTH,
            LVDS_CAPTURE_HEIGHT,
            numLvdsCh
            );
    }
#endif

    ChainsCommon_SurroundView_SetSynthParams(pSynthPrm,
                                            SV_INPUT_WIDTH,
                                            SV_INPUT_HEIGHT,
                                            SV_OUTPUT_WIDTH,
                                            SV_OUTPUT_HEIGHT,
                                            SV_NUM_VIEWS,
                                            SV_CARBOX_WIDTH,
                                            SV_CARBOX_HEIGHT,
                                            svOutputMode,
                                            enableCarOverlayInAlg);

    ChainsCommon_SurroundView_SetGAlignParams(pGAlignPrm,
                                            SV_INPUT_WIDTH,
                                            SV_INPUT_HEIGHT,
                                            SV_OUTPUT_WIDTH,
                                            SV_OUTPUT_HEIGHT,
                                            SV_NUM_VIEWS,
                                            SV_CARBOX_WIDTH,
                                            SV_CARBOX_HEIGHT,
                                            svOutputMode,
                                            gaCalibPrm);

    ChainsCommon_SurroundView_SetPAlignParams(pPAlignPrm,
                                            SV_INPUT_WIDTH,
                                            SV_INPUT_HEIGHT,
                                            SV_OUTPUT_WIDTH,
                                            SV_OUTPUT_HEIGHT,
                                            SV_NUM_VIEWS,
                                            SV_CARBOX_WIDTH,
                                            SV_CARBOX_HEIGHT,
                                            svOutputMode);

    if(pEdgeDetect)
    {
        frontCamAlgOutWidth  = LVDS_CAPTURE_WIDTH/2;
        frontCamAlgOutHeight = LVDS_CAPTURE_HEIGHT/2;
        ChainsCommon_SurroundView_SetEdgeDetectionAlgPrms(
                                    pEdgeDetect,
                                    LVDS_CAPTURE_WIDTH,
                                    LVDS_CAPTURE_HEIGHT);
    }

    if(pDof && pDofVectorToImage)
    {
        /* align to algorithm required W x H */
        frontCamAlgOutWidth
            = SystemUtils_floor(LVDS_CAPTURE_WIDTH,
                                    DOF_WIDTH_ALIGN);
        frontCamAlgOutHeight
            = SystemUtils_floor(LVDS_CAPTURE_HEIGHT,
                                    DOF_HEIGHT_ALIGN*2);

        ChainsCommon_SurroundView_SetDenseOptFlowAlgPrms(
                pDof,
                frontCamAlgOutWidth,
                frontCamAlgOutHeight,
                0,
                1,
                ALGLINK_DENSEOPTFLOW_LKNUMPYR_1);


        ChainsCommon_SurroundView_SetVectorToImageAlgPrms(
                                pDofVectorToImage,
                                frontCamAlgOutWidth,
                                frontCamAlgOutHeight,
                                1,
                                TRUE,
                                2 );

        /* crop a bit before giving Vector to image output to scalar */
        frontCamAlgOutWidth -= 64;
        frontCamAlgOutHeight -= 64;
    }

    if(pCaptureSelect)
    {
        ChainsCommon_SurroundView_SetSelectPrm(pCaptureSelect);
    }

    if(pAvbRxPrm && pSvSync)
    {
        ChainsCommon_SurroundView_SetSyncPrm(pSvSync, 4, 0);
    }
    else if (pSvSync)
    {
        ChainsCommon_SurroundView_SetSyncPrm(pSvSync, 4, 1);
    }

    if(pSvOrgSync)
    {
        if(pAvbRxPrm)
        {
            ChainsCommon_SurroundView_SetSyncPrm(pSvOrgSync, 4, 0);
        }
        else
        {
            ChainsCommon_SurroundView_SetSyncPrm(pSvOrgSync, 4, 1);
        }

    }
    if(pFrontCamSync)
    {
        ChainsCommon_SurroundView_SetSyncPrm(pFrontCamSync, 2, 0);
    }

    ChainsCommon_SurroundView_SetVpePrm(pSvOrgVpe, pFrontCamVpe,
                    frontCamAlgOutWidth, frontCamAlgOutHeight);

    if(pSvOrgDmaSwMs)
    {
        ChainsCommon_SurroundView_SetAlgDmaSwMsPrm(
                    pSvOrgDmaSwMs,
                    SV_NUM_VIEWS,
                    SVORG_SCALED_WIDTH,
                    SVORG_SCALED_HEIGHT,
                    0,
                    SVORG_MOSAIC_SPACING_HOR,
                    SVORG_MOSAIC_SPACING_VER
                   );
    }
    if(pFrontCamDmaSwMs)
    {
        ChainsCommon_SurroundView_SetAlgDmaSwMsPrm(
                    pFrontCamDmaSwMs,
                    2,
                    FRONTCAM_SCALED_WIDTH,
                    FRONTCAM_SCALED_HEIGHT,
                    0,
                    FRONTCAM_MOSAIC_SPACING_HOR,
                    FRONTCAM_MOSAIC_SPACING_VER
                   );
    }
    if(pGrpxSrc)
    {
        ChainsCommon_SurroundView_SetGrpxSrcPrms(
                            pGrpxSrc,
                            displayWidth,
                            displayHeight
                        );

        if(pEdgeDetect)
        {
            pGrpxSrc->surroundViewEdgeDetectLayoutEnable = TRUE;
            pGrpxSrc->surroundViewDOFLayoutEnable = FALSE;
            pGrpxSrc->surroundViewPdTsrLayoutEnable = FALSE;
            pGrpxSrc->surroundViewLdLayoutEnable = FALSE;
        }
        else
        if(pDof && pDofVectorToImage)
        {
            pGrpxSrc->surroundViewEdgeDetectLayoutEnable = FALSE;
            pGrpxSrc->surroundViewDOFLayoutEnable = TRUE;
            pGrpxSrc->surroundViewPdTsrLayoutEnable = FALSE;
            pGrpxSrc->surroundViewLdLayoutEnable = FALSE;
        }
        else
        if(Alg_ObjectDetectionPrm && Alg_LaneDetectPrm)
        {
            pGrpxSrc->surroundViewEdgeDetectLayoutEnable = FALSE;
            pGrpxSrc->surroundViewDOFLayoutEnable = FALSE;
            pGrpxSrc->surroundViewPdTsrLayoutEnable = TRUE;
            pGrpxSrc->surroundViewLdLayoutEnable = TRUE;
        }
    }
#if 0
    if(pSvDisplay||pSvOrgDisplay||pFrontCamDisplay||pGrpxDisplay)
    {
        ChainsCommon_SurroundView_SetDisplayPrm(
                            pSvDisplay,
                            pSvOrgDisplay,
                            pFrontCamDisplay,
                            pGrpxDisplay,
                            displayWidth,
                            displayHeight
                        );

        ChainsCommon_StartDisplayCtrl(
            displayType,
            displayWidth,
            displayHeight
            );
    }
#endif
    if (VPE_algPdPrm)
    {
        ChainsCommon_SurroundView_SetFCVpePrm(
                        VPE_algPdPrm,
                        FEATUREPLANE_ALG_WIDTH,
                        FEATUREPLANE_ALG_HEIGHT,
                        LVDS_CAPTURE_WIDTH,
                        LVDS_CAPTURE_HEIGHT,
                        SYSTEM_DF_YUV420SP_UV
                    );
    }

    if (Alg_FeaturePlaneComputationPrm && Alg_ObjectDetectionPrm )
    {
        ChainsCommon_SurroundView_SetFeaturePlaneComputeAlgPrms(
                        Alg_FeaturePlaneComputationPrm,
                        FEATUREPLANE_ALG_WIDTH,
                        FEATUREPLANE_ALG_HEIGHT
                    );

        ChainsCommon_SurroundView_SetObjectDetectPrm(
                        Alg_ObjectDetectionPrm
                    );

        ChainsCommon_SurroundView_SetSyncFcPrm(Sync_algPdPrm);

        ChainsCommon_SurroundView_SetObjectDrawPrms(
                        Alg_ObjectDrawPrm,
                        FEATUREPLANE_ALG_WIDTH,
                        FEATUREPLANE_ALG_HEIGHT
                    );
    }

    if (Alg_LaneDetectPrm && Alg_LaneDetectDrawPrm)
    {
        ChainsCommon_SurroundView_SetSyncFcPrm(Sync_algLdPrm);

        AlgorithmLink_LaneDetect_Init(Alg_LaneDetectPrm);
        AlgorithmLink_LaneDetectDraw_Init(Alg_LaneDetectDrawPrm);

        ChainsCommon_SurroundView_SetLaneDetectPrm(
            Alg_LaneDetectPrm,
            Alg_LaneDetectDrawPrm,
            0,
            0,
            FEATUREPLANE_ALG_WIDTH,
            FEATUREPLANE_ALG_HEIGHT
            );
    }
}


