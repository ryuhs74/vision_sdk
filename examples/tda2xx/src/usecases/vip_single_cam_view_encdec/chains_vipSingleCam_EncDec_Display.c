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
#include "chains_vipSingleCam_EncDec_Display_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)
#define MAX_NUMBER_OF_CHANNELS    (1)
#define ENCDEC_MAX_FRAME_WIDTH    (1280)
#define ENCDEC_MAX_FRAME_HEIGHT   (720)


/**
 *******************************************************************************
 *
 *  \brief  Use-case object
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipSingleCam_EncDec_DisplayObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;
    Char    codecType;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraEncDecAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set Enc Create Parameters
 *
 * \param   pPrm         [IN]  EncLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_vipSingleCam_EncDec_Display_SetEncPrms(
                   EncLink_CreateParams *pPrm,
                   Chains_VipSingleCameraEncDecAppObj *pObj)
{
    int i, chId;
    EncLink_ChCreateParams *pLinkChPrm;
    EncLink_ChDynamicParams *pLinkDynPrm;

    for (i = 0; i < ENC_LINK_MAX_BUF_ALLOC_POOLS; i++)
    {
        pPrm->numBufPerCh[i] = 4;
    }

    for (chId = 0; chId < MAX_NUMBER_OF_CHANNELS; chId++)
    {
        pLinkChPrm  = &pPrm->chCreateParams[chId];
        pLinkDynPrm = &pLinkChPrm->defaultDynamicParams;

        UTILS_assert (chId < ENC_LINK_MAX_CH);

        switch (pObj->codecType)
        {
          case '0': /* MJPEG */
            pLinkChPrm->format                 = SYSTEM_IVIDEO_MJPEG;
            pLinkChPrm->profile                = 0;
            pLinkChPrm->dataLayout             = VENC_FIELD_SEPARATED;
            pLinkChPrm->fieldMergeEncodeEnable = FALSE;
            pLinkChPrm->enableAnalyticinfo     = 0;
            pLinkChPrm->enableWaterMarking     = 0;
            pLinkChPrm->maxBitRate             = 0;
            pLinkChPrm->encodingPreset         = 0;
            pLinkChPrm->rateControlPreset      = 0;
            pLinkChPrm->enableHighSpeed        = 0;
            pLinkChPrm->enableSVCExtensionFlag = 0;
            pLinkChPrm->numTemporalLayer       = 0;
            pLinkChPrm->overrideInputScanFormat= 0;
            pLinkChPrm->fieldPicEncode         = 0;

            pLinkDynPrm->intraFrameInterval    = 0;
            pLinkDynPrm->targetBitRate         = 10*1000*1000;
            pLinkDynPrm->interFrameInterval    = 0;
            pLinkDynPrm->mvAccuracy            = 0;
            pLinkDynPrm->inputFrameRate        = 30;
            pLinkDynPrm->rcAlg                 = 0;
            pLinkDynPrm->qpMin                 = 0;
            pLinkDynPrm->qpMax                 = 0;
            pLinkDynPrm->qpInit                = -1;
            pLinkDynPrm->vbrDuration           = 0;
            pLinkDynPrm->vbrSensitivity        = 0;
            break;

          case '1': /* H264 */
            pLinkChPrm->format                 = SYSTEM_IVIDEO_H264HP;
            pLinkChPrm->profile                = 100;
            pLinkChPrm->dataLayout             = VENC_FIELD_SEPARATED;
            pLinkChPrm->fieldMergeEncodeEnable = FALSE;
            pLinkChPrm->enableAnalyticinfo     = 0;
            pLinkChPrm->enableWaterMarking     = 0;
            pLinkChPrm->maxBitRate             = -1;
            pLinkChPrm->encodingPreset         = SYSTEM_XDM_MED_SPEED_HIGH_QUALITY;
            pLinkChPrm->rateControlPreset      = SYSTEM_IVIDEO_LOW_DELAY;
            pLinkChPrm->enableHighSpeed        = FALSE;
            pLinkChPrm->enableSVCExtensionFlag = FALSE;
            pLinkChPrm->numTemporalLayer       = 0;
            pLinkChPrm->overrideInputScanFormat= 0;
            pLinkChPrm->fieldPicEncode         = 0;

            pLinkDynPrm->intraFrameInterval    = 30;
            pLinkDynPrm->targetBitRate         = 10*1000*1000;
            pLinkDynPrm->interFrameInterval    = 1;
            pLinkDynPrm->mvAccuracy            = SYSTEM_IVIDENC2_MOTIONVECTOR_QUARTERPEL;
            pLinkDynPrm->inputFrameRate        = 30;
            pLinkDynPrm->rcAlg                 = 0;
            pLinkDynPrm->qpMin                 = 0;
            pLinkDynPrm->qpMax                 = 51;
            pLinkDynPrm->qpInit                = 25;
            pLinkDynPrm->vbrDuration           = 8;
            pLinkDynPrm->vbrSensitivity        = 0;
            break;

          default: /* D1 */
            printf("\r\nCodec Type: %d, returning \n", pObj->codecType);
            UTILS_assert(FALSE);
            break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Decode Create Parameters
 *
 * \param   pPrm         [IN]    DecodeLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_vipSingleCam_EncDec_Display_SetDecodePrms(
                   DecLink_CreateParams *pPrm,
                   Chains_VipSingleCameraEncDecAppObj *pObj)
{
    UInt32 chId;
    DecLink_ChCreateParams *decPrm;

    for (chId = 0; chId<MAX_NUMBER_OF_CHANNELS; chId++)
    {
        UTILS_assert (chId < DEC_LINK_MAX_CH);
        decPrm = &pPrm->chCreateParams[chId];

        decPrm->dpbBufSizeInFrames  = DEC_LINK_DPB_SIZE_IN_FRAMES_DEFAULT;
        decPrm->algCreateStatus     = DEC_LINK_ALG_CREATE_STATUS_CREATE;
        decPrm->decodeFrameType     = DEC_LINK_DECODE_ALL;

        decPrm->processCallLevel    = DEC_LINK_FRAMELEVELPROCESSCALL;
        decPrm->targetMaxWidth      = ENCDEC_MAX_FRAME_WIDTH;
        decPrm->targetMaxHeight     = ENCDEC_MAX_FRAME_HEIGHT;
        decPrm->numBufPerCh         = 6;
        decPrm->defaultDynamicParams.targetBitRate = 10*1000*1000;
        decPrm->defaultDynamicParams.targetFrameRate = 30;
        decPrm->fieldMergeDecodeEnable = FALSE;

        switch (pObj->codecType)
        {
            case '0': /* MJPEG */
                decPrm->format = SYSTEM_IVIDEO_MJPEG;
                decPrm->profile = 0;
                decPrm->displayDelay = 0;
                break;

            case '1': /* H264 */
                decPrm->format = SYSTEM_IVIDEO_H264HP;
                decPrm->profile = 3;
                decPrm->displayDelay = 0;
                break;

            default: /* D1 */
                printf("\r\nCodec Type: %d, returning \n", pObj->codecType);
                UTILS_assert(FALSE);
                break;
        }
    }
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
static Void chains_vipSingleCam_EncDec_Display_SetVPEPrms(
                    VpeLink_CreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 displayWidth,
                    UInt32 displayHeight,
                    UInt32 inputWidth,
                    UInt32 inputHeight
                    )
{
    UInt16 chId;

    pPrm->enableOut[0] = TRUE;

    for(chId = 0; chId < numLvdsCh; chId++)
    {
        pPrm->chParams[chId].outParams[0].numBufsPerCh =
                                 VPE_LINK_NUM_BUFS_PER_CH_DEFAULT;

        pPrm->chParams[chId].outParams[0].width = displayWidth;
        pPrm->chParams[chId].outParams[0].height = displayHeight;
        pPrm->chParams[chId].outParams[0].dataFormat = SYSTEM_DF_YUV420SP_UV;

        pPrm->chParams[chId].scCfg.bypass       = FALSE;
        pPrm->chParams[chId].scCfg.nonLinear    = FALSE;
        pPrm->chParams[chId].scCfg.stripSize    = 0;

        pPrm->chParams[chId].scCropCfg.cropStartX = 32;
        pPrm->chParams[chId].scCropCfg.cropStartY = 24;
        pPrm->chParams[chId].scCropCfg.cropWidth = inputWidth-32;
        pPrm->chParams[chId].scCropCfg.cropHeight = inputHeight-24;
    }
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
Void chains_vipSingleCam_EncDec_Display_SetAppPrms(
            chains_vipSingleCam_EncDec_DisplayObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraEncDecAppObj *pObj
        = (Chains_VipSingleCameraEncDecAppObj*)appObj;

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SingleCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );

    pUcObj->GrpxSrcPrm.grpxBufInfo.dataFormat = SYSTEM_DF_BGRA16_4444;

    ChainsCommon_SetDisplayPrms(&pUcObj->Display_VideoPrm,
                                &pUcObj->Display_GrpxPrm,
                                pObj->chainsCfg->displayType,
                                pObj->displayWidth,
                                pObj->displayHeight
                               );

    ChainsCommon_StartDisplayCtrl(
        pObj->chainsCfg->displayType,
        pObj->displayWidth,
        pObj->displayHeight
        );

    chains_vipSingleCam_EncDec_Display_SetVPEPrms(&pUcObj->VPEPrm,
                                        MAX_NUMBER_OF_CHANNELS,
                                        pObj->displayWidth,
                                        pObj->displayHeight,
                                        ENCDEC_MAX_FRAME_WIDTH,
                                        ENCDEC_MAX_FRAME_HEIGHT
                                       );

    chains_vipSingleCam_EncDec_Display_SetDecodePrms(&pUcObj->DecodePrm, pObj);

    chains_vipSingleCam_EncDec_Display_SetEncPrms(&pUcObj->EncodePrm, pObj);
}

/**
 *******************************************************************************
 *
 * \brief   Start the capture Enc Dec display Links
 *
 *          Function sends a control command to capture and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] Chains_VipSingleCameraViewObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_EncDec_Display_StartApp(Chains_VipSingleCameraEncDecAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCam_EncDec_Display_Start(&pObj->ucObj);

    Chains_prfLoadCalcEnable(TRUE, FALSE, FALSE);
}

/**
 *******************************************************************************
 *
 * \brief   Delete the capture Enc Dec display Links
 *
 *          Function sends a control command to capture and display link to
 *          to delete all the prior created links
 *          System_linkDelete is called with LinkId to delete the links.
 *
 * \param   pObj   [IN]   Chains_VipSingleCameraViewObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_EncDec_Display_StopAndDeleteApp(
                                Chains_VipSingleCameraEncDecAppObj *pObj)
{
    chains_vipSingleCam_EncDec_Display_Stop(&pObj->ucObj);
    chains_vipSingleCam_EncDec_Display_Delete(&pObj->ucObj);

    if(pObj->ucObj.CapturePrm.memAllocInfo.memAddr)
    {
        Int32 status;

        status = Utils_memFree(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    (Ptr)pObj->ucObj.CapturePrm.memAllocInfo.memAddr,
                    pObj->ucObj.CapturePrm.memAllocInfo.memSize);
        UTILS_assert(status==0);
    }

    ChainsCommon_StopDisplayCtrl();
    ChainsCommon_StopCaptureDevice(pObj->chainsCfg->captureSrc);
    ChainsCommon_StopDisplayDevice(pObj->chainsCfg->displayType);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

/**
 *******************************************************************************
 * \brief Run Time Menu string for codec Type Selection.
 *******************************************************************************
 */
char chains_vipSingleCam_EncDec_codecTypeSelectMenu[] = {
    "\r\n "
    "\r\n ========================================="
    "\r\n Chains Run-time Codec Type Selection Menu"
    "\r\n ========================================="
    "\r\n "
    "\r\n Enter '0' for MJPEG "
    "\r\n "
    "\r\n Enter '1' for H.264 "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Enc Dec Display usecase function
 *
 *          This functions executes the create, start functions
 *
 *          Further in a while loop displays run time menu and waits
 *          for user inputs to print the statistics or to end the demo.
 *
 *          Once the user inputs end of demo stop and delete
 *          functions are executed.
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCam_EncDec_Display(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraEncDecAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    while(!done)
    {
        Vps_printf(chains_vipSingleCam_EncDec_codecTypeSelectMenu);
        ch = Chains_readChar();
        switch(ch)
        {
            case '0':
            case '1':
                chainsObj.codecType = ch;
                done = TRUE;
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n",
                           chainsObj.codecType);
                break;
        }
    }

    chains_vipSingleCam_EncDec_Display_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCam_EncDec_Display_StartApp(&chainsObj);

    done = FALSE;
    while(!done)
    {
        ch = Chains_menuRunTime();

        switch(ch)
        {
            case '0':
                done = TRUE;
                break;
            case 'p':
            case 'P':
                ChainsCommon_PrintStatistics();
                chains_vipSingleCam_EncDec_Display_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCam_EncDec_Display_StopAndDeleteApp(&chainsObj);
}

