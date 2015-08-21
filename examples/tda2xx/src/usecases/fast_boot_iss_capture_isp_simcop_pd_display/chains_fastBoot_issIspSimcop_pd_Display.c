/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "chains_fastBoot_issIspSimcop_pd_Display_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

#define FEATUREPLANE_ALG_WIDTH    (640)
#define FEATUREPLANE_ALG_HEIGHT   (360)

#define CAPTURE_SENSOR_AR140_WIDTH      (1280)
#define CAPTURE_SENSOR_AR140_HEIGHT     (800)

#define CAPTURE_SENSOR_IMX224_NON_WDR_WIDTH  (1280)
#define CAPTURE_SENSOR_IMX224_NON_WDR_HEIGHT (960)


#define CAPTURE_SENSOR_IMX224_WDR_WIDTH  (1312)
#define CAPTURE_SENSOR_IMX224_WDR_HEIGHT (2164)
#define CAPTURE_SENSOR_IMX224_ISP_WIDTH  (1280)
#define CAPTURE_SENSOR_IMX224_ISP_HEIGHT (960)

#define CAPTURE_SENSOR_IMX224_LONG_LINE_OFFSET      (9U)
#define CAPTURE_SENSOR_IMX224_SHORT_LINE_OFFSET     (142U)
#define CAPTURE_SENSOR_IMX224_LONG_PIXEL_OFFSET     (0)
#define CAPTURE_SENSOR_IMX224_SHORT_PIXEL_OFFSET    (0)

#define ENABLE_WDR_MERGE_PARAMS_CFG

#define IMAGE_UCLATE_OFFSET_QSPI 0xA80000

#define GRPX_BOOT_TIME_DISPLAY_DURATION (24*60*60*1000)
#define GRPX_BOOT_TIME_DISPLAY_FONTID   (5)
#define GRPX_BOOT_TIME_DISPLAY_X_OFFSET (30)
#define GRPX_BOOT_TIME_DISPLAY_Y_OFFSET (25)

/**
 *******************************************************************************
 *
 *  \brief  SingleCameraFrameCopyObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_fastBoot_issIspSimcop_pd_DisplayObj ucObj;

    IssIspConfigurationParameters ispConfig;

    IssM2mSimcopLink_ConfigParams simcopConfig;
    vpsissldcConfig_t             ldcCfg;
    vpsissvtnfConfig_t            vtnfCfg;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

    IssM2mSimcopLink_OperatingMode simcopMode;
    Bool bypassVtnf;
    Bool bypassLdc;

} chains_fastBoot_issIspSimcop_pd_DisplayAppObj;


char gChains_FastBootIssIspSimcop_Pd_Display_runTimeMenu[] = {
"\r\n "
"\r\n ===================="
"\r\n Chains Run-time Menu"
"\r\n ===================="
"\r\n "
"\r\n 0: Stop Chain"
"\r\n "
"\r\n 1: Save Captured Frame"
"\r\n 2: Save SIMCOP Output Frame"
"\r\n 3: Gate ON"
"\r\n 4: Gate OFF"
"\r\n "
"\r\n p: Print Performance Statistics "
"\r\n "
"\r\n Enter Choice: "
"\r\n "
};

Void chains_fastBoot_issIspSimcop_pd_Display_Config_Aewb(
        IssAewbAlgOutParams *pAewbAlgOut,
        Void *appData)
{
    chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj =
        (chains_fastBoot_issIspSimcop_pd_DisplayAppObj *)appData;

    UTILS_assert(NULL != pObj);
    UTILS_assert(NULL != pAewbAlgOut);

    /* AEWB Output parameters are already converted and stored in
       ispCfg parameter of alg out, so set it in the ISP using ISP
       Link */
    System_linkControl(
        pObj->ucObj.IssM2mIspLinkID,
        ISSM2MISP_LINK_CMD_SET_AEWB_PARAMS,
        pAewbAlgOut,
        sizeof(IssAewbAlgOutParams),
        TRUE);

    /* Set the Sensor exposure and analog Gain */
    ChainsCommon_UpdateAewbParams(pAewbAlgOut);
}

Void chains_fastBoot_issIspSimcop_pd_Display_Config_DCC_Params(
        IssIspConfigurationParameters *ispCfg,
        IssM2mSimcopLink_ConfigParams *simcopCfg,
        Void                          *appData)
{
    chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj =
        (chains_fastBoot_issIspSimcop_pd_DisplayAppObj *)appData;

    UTILS_assert(NULL != pObj);
    UTILS_assert(NULL != ispCfg);
    UTILS_assert(NULL != simcopCfg);

    System_linkControl(
        pObj->ucObj.IssM2mIspLinkID,
        ISSM2MISP_LINK_CMD_SET_ISPCONFIG,
        ispCfg,
        sizeof(IssIspConfigurationParameters),
        TRUE);

    if (NULL != simcopCfg->ldcConfig)
    {
        /* Copy LDC configuration and apply it,
           Will use same  */
        memcpy(
            &pObj->ldcCfg,
            simcopCfg->ldcConfig,
            sizeof(vpsissldcConfig_t));

        pObj->simcopConfig.ldcConfig = &pObj->ldcCfg;
        pObj->simcopConfig.vtnfConfig = &pObj->vtnfCfg;

        /* MUST be called after link create and before link start */
        System_linkControl(
                pObj->ucObj.IssM2mSimcopLinkID,
                ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG,
                &pObj->simcopConfig,
                sizeof(pObj->simcopConfig),
                TRUE);
    }
}

#ifdef ENABLE_WDR_MERGE_PARAMS_CFG
/* Function to get the exposure parameters from the sensor and update
   the merge parameters in the ISP. Called only when WDR merge is enabled. */
Void chains_fastBoot_issIspSimcop_pd_Display_Config_Merge(
        IssAewbAlgOutParams *pAewbAlgOut,
        Void *appData)
{
    chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj =
        (chains_fastBoot_issIspSimcop_pd_DisplayAppObj *)appData;

    UTILS_assert(NULL != pObj);
    UTILS_assert(NULL != pAewbAlgOut);

    /* Get Exposure ratio parameters from the sensor */
    VidSensor_control(
        ChainsCommon_GetSensorCreateParams(),
        VID_SENSOR_CMD_GET_EXP_RATIO_PARAMS,
        pAewbAlgOut,
        NULL);

    /* When dgain is applied only to long after split, there is a
        different ratio for split and merge */
    if(TRUE == pAewbAlgOut->outPrms[0].useAeCfg)
    {
        /* pAewbAlgOut->exposureRatio =
            (pAewbAlgOut->exposureRatio *
                pAewbAlgOut->outPrms[0].digitalGain) / 512; */

        System_linkControl(
            pObj->ucObj.IssM2mIspLinkID,
            ISSM2MISP_LINK_CMD_SET_WDR_MERGE_PARAMS,
            pAewbAlgOut,
            sizeof(IssAewbAlgOutParams),
            TRUE);
    }
}
#endif

/**
 *******************************************************************************
 *
 * \brief   Set PD draw parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_SetObjectDrawPrms(
                   chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj,
                   AlgorithmLink_ObjectDrawCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    pPrm->imgFrameWidth    = width;
    pPrm->imgFrameHeight   = height;
    pPrm->numOutBuffers = 3;
    pPrm->pdRectThickness = 1;
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
Void chains_fastBoot_issIspSimcop_pd_Display_SetFeaturePlaneComputeAlgPrms(
                   chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj,
                   AlgorithmLink_FeaturePlaneComputationCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    pPrm->imgFrameHeight = height;
    pPrm->imgFrameWidth  = width;
    pPrm->numOutBuffers  = 3;

    pPrm->roiEnable      = FALSE;
    pPrm->roiCenterX     = width/2;
    pPrm->roiCenterY     = height/2;
    pPrm->roiWidth       = width;
    pPrm->roiHeight      = (height*30)/100;
    pPrm->numScales      = 17;
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
Void chains_fastBoot_issIspSimcop_pd_Display_SetObjectDetectPrm(
                   chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj,
                   AlgorithmLink_ObjectDetectionCreateParams *pPrm
                   )
{
    pPrm->numOutBuffers  = 2;
    pPrm->enablePD       = TRUE;
    pPrm->enableTSR      = TRUE;
}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Link Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipObjectDetection_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_SetSyncPrm(SyncLink_CreateParams *pPrm)

{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 1;
    pPrm->chParams.syncThreshold = 0xFFFF;
}



/**
 *******************************************************************************
 *
 * \brief   Set Gate Create Parameters
 *
 *          This function is used to set the Gate params.
 *          It is called in Create function. It is advisable to have
 *          chains_fastBoot_issIspSimcop_pd_Display_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [IN]    GateLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_SetGatePrms(GateLink_CreateParams *pPrm_GateAlgFeatureCompute,
                                                         GateLink_CreateParams *pPrm_GateAlgCapture,
                                                         GateLink_CreateParams *pPrm_GateAlgDraw,
                                                         UInt32 inWidth,
                                                         UInt32 inHeight)
{

    pPrm_GateAlgFeatureCompute->prevLinkIsCreated = TRUE;

    pPrm_GateAlgCapture->prevLinkIsCreated = TRUE;

    pPrm_GateAlgDraw->prevLinkIsCreated = FALSE;
    pPrm_GateAlgDraw->prevLinkInfo.numQue = 1;
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].numCh              = 1;
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].flags    = 0;
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].pitch[0] = SystemUtils_align(inWidth, 32);
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].pitch[1] = SystemUtils_align(inWidth, 32);
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].startX   = 0;
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].startY   = 0;
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].width    = inWidth;
    pPrm_GateAlgDraw->prevLinkInfo.queInfo[0].chInfo[0].height   = inHeight;

}

/**
 *******************************************************************************
 *
 * \brief   Set Display Create Parameters
 *
 *          This function is used to set the Display params.
 *          It is called in Create function. It is advisable to have
 *          chains_fastBoot_issIspSimcop_pd_Display_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [IN]    DisplayLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_SetDisplayPrms(
                                    DisplayLink_CreateParams *pPrm_Video,
                                    DisplayLink_CreateParams *pPrm_Grpx,
                                    DisplayLink_CreateParams *pPrm_VideoRszB,
                                    Chains_DisplayType displayType,
                                    UInt32 displayWidth,
                                    UInt32 displayHeight,
                                    UInt32 captureWidth,
                                    UInt32 captureHeight)
{

    if(pPrm_Video)
    {
        if((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
          (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
        {
            pPrm_Video->displayScanFormat = SYSTEM_SF_INTERLACED;
        }

        pPrm_Video->rtParams.tarWidth  = displayWidth;
        pPrm_Video->rtParams.tarHeight = displayHeight;
        pPrm_Video->rtParams.posX      = 0;
        pPrm_Video->rtParams.posY      = 0;

        pPrm_Video->displayId          = DISPLAY_LINK_INST_DSS_VID1;
    }

    if(pPrm_VideoRszB)
    {
        if((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
          (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
        {
            pPrm_VideoRszB->displayScanFormat = SYSTEM_SF_INTERLACED;
        }

        pPrm_VideoRszB->rtParams.tarWidth  = displayWidth/3;
        pPrm_VideoRszB->rtParams.tarHeight = displayHeight/3;
        pPrm_VideoRszB->rtParams.posX      = displayWidth - pPrm_VideoRszB->rtParams.tarWidth - 10;
        pPrm_VideoRszB->rtParams.posY      = displayHeight - pPrm_VideoRszB->rtParams.tarHeight - 10;
        pPrm_VideoRszB->displayId          = DISPLAY_LINK_INST_DSS_VID2;
    }

    if(pPrm_Grpx)
    {
        if((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
          (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
        {
            pPrm_Grpx->displayScanFormat = SYSTEM_SF_INTERLACED;
        }

        pPrm_Grpx->displayId = DISPLAY_LINK_INST_DSS_GFX1;
    }
}

Void chains_fastBoot_issIspSimcop_pd_Display_SetVpePrms(
        VpeLink_CreateParams *vpePrms,
        UInt32 outWidth,
        UInt32 outHeight)
{
    VpeLink_ChannelParams *pChPrms = NULL;

    vpePrms->enableOut[0U] = TRUE;
    vpePrms->procCore = VPE_PROCCORE_ISS;

    pChPrms = &vpePrms->chParams[0U];

    pChPrms->outParams[0U].width = outWidth;
    pChPrms->outParams[0U].height = outHeight;
    pChPrms->outParams[0U].dataFormat = SYSTEM_DF_YUV420SP_UV;

    pChPrms->scCfg.bypass = FALSE;
}


Void chains_fastBoot_issIspSimcop_pd_Display_SetSimcopConfig(
            chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj)
{
    ChainsCommon_SetIssSimcopLdcVtnfRtConfig(
        &pObj->ldcCfg,
        &pObj->vtnfCfg,
        pObj->bypassVtnf,
        pObj->bypassLdc);

    pObj->simcopConfig.ldcConfig = &pObj->ldcCfg;
    pObj->simcopConfig.vtnfConfig = &pObj->vtnfCfg;

    /* MUST be called after link create and before link start */
    System_linkControl(
            pObj->ucObj.IssM2mSimcopLinkID,
            ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG,
            &pObj->simcopConfig,
            sizeof(pObj->simcopConfig),
            TRUE);
}


Void chains_fastBoot_issIspSimcop_pd_Display_SetIspConfig(
            chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj)
{
    Int32 status;

    /* set default params */
    IssM2mSimcopLink_ConfigParams_Init(&pObj->simcopConfig);
    vpsissLdcCfg_init(&pObj->ldcCfg);
    vpsissVtnfCfg_init(&pObj->vtnfCfg);

    pObj->simcopConfig.ldcConfig = &pObj->ldcCfg;
    pObj->simcopConfig.vtnfConfig = &pObj->vtnfCfg;

    /* Set the Default SimCop configuration,
        This could get overwriten by DCC */
    ChainsCommon_SetIssSimcopConfig(
        &pObj->simcopConfig,
        pObj->bypassVtnf,
        pObj->bypassLdc,
        1);

    ChainsCommon_GetIssIspConfig(
        pObj->chainsCfg->captureSrc,
        pObj->ucObj.Alg_IssAewbLinkID,
        pObj->ucObj.IssM2mIspPrm.channelParams[0U].operatingMode,
        &pObj->ispConfig,
        &pObj->simcopConfig);

    /* MUST be called after link create and before link start */
    status = System_linkControl(
            pObj->ucObj.IssM2mIspLinkID,
            ISSM2MISP_LINK_CMD_SET_ISPCONFIG,
            &pObj->ispConfig,
            sizeof(pObj->ispConfig),
            TRUE);
    UTILS_assert(0 == status);

    /* MUST be called after link create and before link start */
    status = System_linkControl(
            pObj->ucObj.IssM2mSimcopLinkID,
            ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG,
            &pObj->simcopConfig,
            sizeof(pObj->simcopConfig),
            TRUE);
    UTILS_assert(0 == status);
}

Void chains_fastBoot_issIspSimcop_pd_Display_SetDccConfig(
            chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj)
{
    Int32 status;
    VidSensor_DccInfo dccInfo;
    AlgorithmLink_IssAewbDccCameraInfo camInfo;

    memset((void*) &dccInfo, 0, sizeof(dccInfo));
    VidSensor_control(
        ChainsCommon_GetSensorCreateParams(),
        VID_SENSOR_CMD_GET_DCC_INFO,
        &dccInfo,
        NULL);
    if (TRUE == dccInfo.isDccCfgSupported)
    {
        camInfo.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_SET_CAMERA_INFO;
        camInfo.baseClassControl.size = sizeof(camInfo);

        camInfo.cameraId = dccInfo.cameraId;

        if (pObj->chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL)
        {
            camInfo.width = CAPTURE_SENSOR_WIDTH;
            camInfo.height = CAPTURE_SENSOR_HEIGHT;
        }
        else
        {
            camInfo.width = CAPTURE_SENSOR_AR140_WIDTH;
            camInfo.height = CAPTURE_SENSOR_AR140_HEIGHT;
        }

        status = System_linkControl(
                    pObj->ucObj.Alg_IssAewbLinkID,
                    ALGORITHM_LINK_CMD_CONFIG,
                    &camInfo,
                    sizeof(camInfo),
                    TRUE);
        UTILS_assert(0 == status);
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
Void chains_fastBoot_issIspSimcop_pd_Display_SetAppPrms(chains_fastBoot_issIspSimcop_pd_DisplayObj *pUcObj, Void *appObj)
{
    chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj
        = (chains_fastBoot_issIspSimcop_pd_DisplayAppObj*)appObj;
    IssM2mIspLink_WdrOffsetParams_t wdrOffsetPrms;
    IssM2mIspLink_OperatingMode ispOpMode;

    ispOpMode = ISSM2MISP_LINK_OPMODE_12BIT_LINEAR;

    if (pObj->chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        if (CHAINS_ISS_WDR_MODE_TWO_PASS == pObj->chainsCfg->issWdrMode)
        {
            pObj->captureOutWidth  = CAPTURE_SENSOR_IMX224_WDR_WIDTH;
            pObj->captureOutHeight = CAPTURE_SENSOR_IMX224_WDR_HEIGHT;
        }
        else
        {
            pObj->captureOutWidth  = CAPTURE_SENSOR_IMX224_NON_WDR_WIDTH;
            pObj->captureOutHeight = CAPTURE_SENSOR_IMX224_NON_WDR_HEIGHT;
        }
    }
    if (pObj->chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL)
    {
       pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
       pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    }
    else
    {
         pObj->captureOutWidth  = CAPTURE_SENSOR_AR140_WIDTH;
         pObj->captureOutHeight = CAPTURE_SENSOR_AR140_HEIGHT;
    }

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    pObj->simcopMode = ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF;
    pObj->bypassVtnf = FALSE;
    pObj->bypassLdc  = FALSE;

    /* Initialize Simcop Pointers */
    pObj->simcopConfig.ldcConfig = &pObj->ldcCfg;
    pObj->simcopConfig.vtnfConfig = &pObj->vtnfCfg;

    if (pObj->chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        if (CHAINS_ISS_WDR_MODE_TWO_PASS == pObj->chainsCfg->issWdrMode)
        {
            ispOpMode = ISSM2MISP_LINK_OPMODE_2PASS_WDR;
        }
        if (CHAINS_ISS_WDR_MODE_SINGLE_PASS == pObj->chainsCfg->issWdrMode)
        {
            ispOpMode = ISSM2MISP_LINK_OPMODE_1PASS_WDR;
        }

        ChainsCommon_SetIssCreatePrms(
            &pUcObj->IssCapturePrm,
            &pUcObj->IssM2mIspPrm,
            &pUcObj->IssM2mSimcopPrm,
            &pUcObj->Alg_IssAewbPrm,
            pObj->chainsCfg->captureSrc,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            0,
            0,
            pObj->simcopMode,
            ispOpMode,
            NULL);
    }
    else
    {
        if (CHAINS_ISS_WDR_MODE_TWO_PASS == pObj->chainsCfg->issWdrMode)
        {
            ispOpMode = ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED;
        }

        wdrOffsetPrms.longLineOffset = CAPTURE_SENSOR_IMX224_LONG_LINE_OFFSET;
        wdrOffsetPrms.shortLineOffset = CAPTURE_SENSOR_IMX224_SHORT_LINE_OFFSET;
        wdrOffsetPrms.longPixelOffset = CAPTURE_SENSOR_IMX224_LONG_PIXEL_OFFSET;
        wdrOffsetPrms.shortPixelOffset = CAPTURE_SENSOR_IMX224_SHORT_PIXEL_OFFSET;

        wdrOffsetPrms.width = CAPTURE_SENSOR_IMX224_ISP_WIDTH;
        wdrOffsetPrms.height = CAPTURE_SENSOR_IMX224_ISP_HEIGHT;

        /* In case of WDR Line Interleaved mode,
           ISP frame size is different from capture frame size */
        ChainsCommon_SetIssCreatePrms(
            &pUcObj->IssCapturePrm,
            &pUcObj->IssM2mIspPrm,
            &pUcObj->IssM2mSimcopPrm,
            &pUcObj->Alg_IssAewbPrm,
            pObj->chainsCfg->captureSrc,
            CAPTURE_SENSOR_IMX224_ISP_WIDTH,
            CAPTURE_SENSOR_IMX224_ISP_HEIGHT,
            ((pObj->displayWidth/3) & ~0x1),
            (pObj->displayHeight/3),
            pObj->simcopMode,
            ispOpMode,
            &wdrOffsetPrms);
    }

    pUcObj->Alg_IssAewbPrm.appData = pObj;
    pUcObj->Alg_IssAewbPrm.cfgCbFxn = chains_fastBoot_issIspSimcop_pd_Display_Config_Aewb;

    #ifdef ENABLE_WDR_MERGE_PARAMS_CFG
    /* No Need to enable merge function for single pass wdr flow */
    if (CHAINS_ISS_WDR_MODE_TWO_PASS == pObj->chainsCfg->issWdrMode)
    {
        pUcObj->Alg_IssAewbPrm.mergeCbFxn =
            chains_fastBoot_issIspSimcop_pd_Display_Config_Merge;
    }
    #endif

    pUcObj->Alg_IssAewbPrm.dccIspCfgFxn =
        chains_fastBoot_issIspSimcop_pd_Display_Config_DCC_Params;

    pUcObj->IssCapturePrm.allocBufferForRawDump = TRUE;

    chains_fastBoot_issIspSimcop_pd_Display_SetFeaturePlaneComputeAlgPrms(
                    pObj,
                    &pUcObj->Alg_FeaturePlaneComputationPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT
                );

    chains_fastBoot_issIspSimcop_pd_Display_SetObjectDetectPrm(
                    pObj,
                    &pUcObj->Alg_ObjectDetectionPrm
                );

    chains_fastBoot_issIspSimcop_pd_Display_SetSyncPrm(
                    &pUcObj->Sync_algPrm
                );

    chains_fastBoot_issIspSimcop_pd_Display_SetObjectDrawPrms(
                    pObj,
                    &pUcObj->Alg_ObjectDrawPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT
                );

    chains_fastBoot_issIspSimcop_pd_Display_SetGatePrms(&pUcObj->Gate_algFeatureComputePrm,
                                                        &pUcObj->Gate_algCapturePrm,
                                                        &pUcObj->Gate_algDrawPrm,
                                                        FEATUREPLANE_ALG_WIDTH,
                                                        FEATUREPLANE_ALG_HEIGHT);

    chains_fastBoot_issIspSimcop_pd_Display_SetVpePrms(&pUcObj->VPEPrm,
                                                        FEATUREPLANE_ALG_WIDTH,
                                                        FEATUREPLANE_ALG_HEIGHT);


    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                 pObj->displayWidth,
                                 pObj->displayHeight);


    if (pObj->chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        chains_fastBoot_issIspSimcop_pd_Display_SetDisplayPrms(&pUcObj->Display_VideoPrm,
                                               &pUcObj->Display_GrpxPrm,
                                               NULL,
                                               pObj->chainsCfg->displayType,
                                               pObj->displayWidth,
                                               pObj->displayHeight,
                                               pObj->captureOutWidth,
                                               pObj->captureOutHeight
                                               );
    }
    else
    {
        chains_fastBoot_issIspSimcop_pd_Display_SetDisplayPrms(&pUcObj->Display_VideoPrm,
                                               &pUcObj->Display_GrpxPrm,
                                               NULL,
                                               //NULL,
                                               pObj->chainsCfg->displayType,
                                               pObj->displayWidth,
                                               pObj->displayHeight,
                                               pObj->captureOutWidth,
                                               pObj->captureOutHeight
                                               );
    }

    ChainsCommon_StartDisplayCtrl(
        pObj->chainsCfg->displayType,
        pObj->displayWidth,
        pObj->displayHeight
        );
}

/**
 *******************************************************************************
 *
 * \brief   Start the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] chains_fastBoot_issIspSimcop_pd_DisplayAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_StartApp_UcEarly(chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj)
{

    ChainsCommon_SetWdrMode(pObj->chainsCfg->issWdrMode);

    /* Video sensor layer is used to get the dcc bin file and other information,
       to set the ISP configuation, so Video sensor is createed first */
    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight);

    chains_fastBoot_issIspSimcop_pd_Display_SetDccConfig(pObj);

    /* Sets the Simcop Config also */
    chains_fastBoot_issIspSimcop_pd_Display_SetIspConfig(pObj);

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_fastBoot_issIspSimcop_pd_Display_Start_UcEarly(&pObj->ucObj);

}


/**
 *******************************************************************************
 *
 * \brief   Start the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] chains_fastBoot_issIspSimcop_pd_DisplayAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_StartApp_UcLate(chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    chains_fastBoot_issIspSimcop_pd_Display_Start_UcLate(&pObj->ucObj);

    Chains_prfLoadCalcEnable(TRUE, FALSE, FALSE);
}

/**
 *******************************************************************************
 *
 * \brief   Delete the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to delete all the prior created links
 *          System_linkDelete is called with LinkId to delete the links.
 *
 * \param   pObj   [IN]   chains_fastBoot_issIspSimcop_pd_DisplayAppObj
 *
 *******************************************************************************
*/
Void chains_fastBoot_issIspSimcop_pd_Display_StopAndDeleteApp(chains_fastBoot_issIspSimcop_pd_DisplayAppObj *pObj)
{
    chains_fastBoot_issIspSimcop_pd_Display_Stop(&pObj->ucObj);
    chains_fastBoot_issIspSimcop_pd_Display_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();

    ChainsCommon_StopCaptureDevice(pObj->chainsCfg->captureSrc);

    ChainsCommon_StopDisplayDevice(pObj->chainsCfg->displayType);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase function
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
Void Chains_fastBootIssIspSimcop_pd_Display(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    chains_fastBoot_issIspSimcop_pd_DisplayAppObj chainsObj;
    chainsObj.bypassVtnf = 0; /* KW error fix */
    chainsObj.bypassLdc  = 0; /* KW error fix */
    DisplayLink_SwitchChannelParams displayPrm;
    Utils_BootSlaves_Params bootParams;
    GrpxSrcLink_StringRunTimePrintParams printPrms;
    UInt32 poweOnToDisplay, poweOnToAlg;


    if(chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_OV10640_CSI2
        &&
       chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_IMX224_CSI2
        &&
       chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_OV10640_PARALLEL
        &&
       chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL
        &&
       chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL
        )
    {
        Vps_printf(" CHAINS: Unsupported sensor for this usecase. Cannot run use-case !!!\n");
        return;
    }

    if ((chainsCfg->captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL) &&
        (CHAINS_ISS_WDR_MODE_DISABLED != chainsCfg->issWdrMode))
    {
        Vps_printf(" CHAINS: WDR Mode is not supported for AR132 sensor !!!\n");
        return;
    }
    if ((chainsCfg->captureSrc != CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL) &&
        (CHAINS_ISS_WDR_MODE_SINGLE_PASS == chainsCfg->issWdrMode))
    {
        Vps_printf(" CHAINS: Single Pass WDR is only support on AR0140 !!!\n");
        return;
    }

    if ((CHAINS_ISS_WDR_MODE_TWO_PASS == chainsCfg->issWdrMode) ||
        (CHAINS_ISS_WDR_MODE_SINGLE_PASS == chainsCfg->issWdrMode))
    {
        Vps_printf(" CHAINS: Please make sure BSP is build with WDR and LDC enabled !!!\n");
    }

    chainsObj.chainsCfg = chainsCfg;

    /* Initialize Video Sensor, so that Algorithm can use Params
       from Vid Sensor layer */
    ChainsCommon_InitCaptureDevice(chainsCfg->captureSrc);

    chains_fastBoot_issIspSimcop_pd_Display_Create_UcEarly(&chainsObj.ucObj, &chainsObj);
    chains_fastBoot_issIspSimcop_pd_Display_StartApp_UcEarly(&chainsObj);

    poweOnToDisplay = Utils_getCurGlobalTimeInMsec();

    /* boot & sync other cores here */
    Utils_bootSlaves_paramsInit(&bootParams);
    bootParams.offset  = IMAGE_UCLATE_OFFSET_QSPI;
    bootParams.useEdma = TRUE;
    Utils_bootSlaves(&bootParams);
    Utils_syncSlaves();

    /* create & start late usecase */
    chains_fastBoot_issIspSimcop_pd_Display_Create_UcLate(&chainsObj.ucObj, &chainsObj);
    chains_fastBoot_issIspSimcop_pd_Display_StartApp_UcLate(&chainsObj);

    /* Send command to toggle operation status to all gates */
    System_linkControl(
            chainsObj.ucObj.Gate_algFeatureComputeLinkID,
            GATE_LINK_CMD_SET_OPERATION_MODE_ON,
            NULL,
            0,
            TRUE);

    System_linkControl(
            chainsObj.ucObj.Gate_algCaptureLinkID,
            GATE_LINK_CMD_SET_OPERATION_MODE_ON,
            NULL,
            0,
            TRUE);

    System_linkControl(
            chainsObj.ucObj.Gate_algDrawLinkID,
            GATE_LINK_CMD_SET_OPERATION_MODE_ON,
            NULL,
            0,
            TRUE);

    /* Switch channel from UcEarly to output of the algorithm */
    displayPrm.activeChId = 1;
    System_linkControl(chainsObj.ucObj.Display_VideoLinkID,
                    DISPLAY_LINK_CMD_SWITCH_CH,
                    &displayPrm,
                    sizeof(displayPrm),
                    TRUE);

    poweOnToAlg = Utils_getCurGlobalTimeInMsec();


    snprintf(printPrms.stringInfo.string,
             sizeof(printPrms.stringInfo.string) - 1,
             "Boot Time: Power On To Preview %d ms, Power On To Obj Detect %d ms!!!", poweOnToDisplay, poweOnToAlg);

    chains_fastBoot_issIspSimcop_pd_Display_CreateAndStart_GrpxSrc(&chainsObj.ucObj);

    printPrms.stringInfo.string[sizeof(printPrms.stringInfo.string) - 1] = 0;
    printPrms.duration_ms = GRPX_BOOT_TIME_DISPLAY_DURATION;
    printPrms.stringInfo.fontType = GRPX_BOOT_TIME_DISPLAY_FONTID;
    printPrms.stringInfo.startX   = GRPX_BOOT_TIME_DISPLAY_X_OFFSET;
    printPrms.stringInfo.startY   = chainsObj.displayHeight-GRPX_BOOT_TIME_DISPLAY_Y_OFFSET;

    System_linkControl(IPU1_0_LINK(SYSTEM_LINK_ID_GRPX_SRC_0),
                       GRPX_SRC_LINK_CMD_PRINT_STRING,
                       &printPrms,
                       sizeof(printPrms),
                       TRUE);

    while(!done)
    {
        Vps_printf(gChains_FastBootIssIspSimcop_Pd_Display_runTimeMenu);

        ch = Chains_readChar();

        switch(ch)
        {
            case '0':
                done = TRUE;
                break;
            case '1':
                /* Send command to Capture Link to save a frame */
                System_linkControl(
                        chainsObj.ucObj.IssCaptureLinkID,
                        ISSCAPTURE_LINK_CMD_SAVE_FRAME,
                        NULL,
                        0,
                        TRUE);
                break;
            case '2':
                /* Send command to Capture Link to save a frame */
                System_linkControl(
                        chainsObj.ucObj.IssM2mSimcopLinkID,
                        ISSM2MSIMCOP_LINK_CMD_SAVE_FRAME,
                        NULL,
                        0,
                        TRUE);
                break;
            case '3':
                /* Send command to toggle operation status */
                System_linkControl(
                        chainsObj.ucObj.Gate_algFeatureComputeLinkID,
                        GATE_LINK_CMD_SET_OPERATION_MODE_ON,
                        NULL,
                        0,
                        TRUE);

                System_linkControl(
                        chainsObj.ucObj.Gate_algCaptureLinkID,
                        GATE_LINK_CMD_SET_OPERATION_MODE_ON,
                        NULL,
                        0,
                        TRUE);

                System_linkControl(
                        chainsObj.ucObj.Gate_algDrawLinkID,
                        GATE_LINK_CMD_SET_OPERATION_MODE_ON,
                        NULL,
                        0,
                        TRUE);

                displayPrm.activeChId = 1;

                System_linkControl(chainsObj.ucObj.Display_VideoLinkID,
                                DISPLAY_LINK_CMD_SWITCH_CH,
                                &displayPrm,
                                sizeof(displayPrm),
                                TRUE);


                break;
            case '4':
                displayPrm.activeChId = 0;

                System_linkControl(chainsObj.ucObj.Display_VideoLinkID,
                                DISPLAY_LINK_CMD_SWITCH_CH,
                                &displayPrm,
                                sizeof(displayPrm),
                                TRUE);


                /* Send command to toggle operation status */
                System_linkControl(
                        chainsObj.ucObj.Gate_algFeatureComputeLinkID,
                        GATE_LINK_CMD_SET_OPERATION_MODE_OFF,
                        NULL,
                        0,
                        TRUE);

                System_linkControl(
                        chainsObj.ucObj.Gate_algCaptureLinkID,
                        GATE_LINK_CMD_SET_OPERATION_MODE_OFF,
                        NULL,
                        0,
                        TRUE);

                System_linkControl(
                        chainsObj.ucObj.Gate_algDrawLinkID,
                        GATE_LINK_CMD_SET_OPERATION_MODE_OFF,
                        NULL,
                        0,
                        TRUE);
            break;

            case 'p':
            case 'P':
                ChainsCommon_PrintStatistics();
                chains_fastBoot_issIspSimcop_pd_Display_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_fastBoot_issIspSimcop_pd_Display_StopAndDeleteApp(&chainsObj);

}

