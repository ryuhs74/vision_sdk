/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "chains_issMultCaptIspSimcopSv_Display_priv.h"
#include <examples/tda2xx/include/chains_common.h>

#define SV_ISP_OUT_HEIGHT           (720)
#define SV_CARBOX_WIDTH             (160)
#define SV_CARBOX_HEIGHT            (160)

#define SV_NUM_VIEWS                (4)

#define SV_HOR_WIDTH                (1000)
#define SV_HOR_HEIGHT               (760)

#define SV_CAPT_SENSOR_AR140_WIDTH  (1280)
#define SV_CAPT_SENSOR_AR140_HEIGHT (800)

#define SV_ALGO_ALIGN_IGNORE_FIRST_N_FRAMES (160U)
#define SV_ALGO_ALIGN_DEFAULT_FOCAL_LENGTH  (407U)

#define ENABLE_WDR_MERGE_PARAMS_CFG

/**
 *******************************************************************************
 *
 *  \brief  chains_issMultCaptIspSimcopSv_DisplayAppObj
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_issMultCaptIspSimcopSv_DisplayObj ucObj;

    IssIspConfigurationParameters ispConfig;

    IssM2mSimcopLink_ConfigParams simcopConfig;
    vpsissldcConfig_t             ldcCfg;
    vpsissvtnfConfig_t            vtnfCfg;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

    /* Not used for now */
    IssM2mSimcopLink_OperatingMode simcopMode;
    Bool bypassVtnf;
    Bool bypassLdc;

} chains_issMultCaptIspSimcopSv_DisplayAppObj;


char gChains_IssMultCaptIspSimcopSv_Display_runTimeMenu[] = {
"\r\n "
"\r\n ===================="
"\r\n Chains Run-time Menu"
"\r\n ===================="
"\r\n "
"\r\n 0: Stop Chain"
"\r\n 1: Camera Position Calibration"
"\r\n 2: Save a captured RAW frame from channel 0"
"\r\n "
"\r\n "
"\r\n p: Print Performance Statistics "
"\r\n "
"\r\n Enter Choice: "
"\r\n "
};

static Void chains_issMultCaptIspSimcopSv_Display_SetSyncPrm(
                    SyncLink_CreateParams *pPrm,
                    UInt32 numCh,
                    UInt32 syncPeriod)
{
    UInt16 chId;

    pPrm->chParams.numCh = numCh;
    pPrm->chParams.numActiveCh = pPrm->chParams.numCh;
    for(chId = 0; chId < pPrm->chParams.numCh; chId++)
    {
        pPrm->chParams.channelSyncList[chId] = TRUE;
    }

    pPrm->chParams.syncDelta = 37U; /* round up 1 / 27.0 */
    pPrm->chParams.syncThreshold = pPrm->chParams.syncDelta * 2U;

}


static Void chains_issMultCaptIspSimcopSv_Display_SetSelectlinkPrms(SelectLink_CreateParams *pPrm)
{
    pPrm->numOutQue = 1U;

    pPrm->outQueChInfo[0].outQueId   = 0;
    pPrm->outQueChInfo[0].numOutCh   = 1;
    pPrm->outQueChInfo[0].inChNum[0] = 0;
}


static Void chains_issMultCaptIspSimcopSv_Display_Config_Aewb(
        IssAewbAlgOutParams *pAewbAlgOut,
        Void *appData)
{
    UInt32 chId;

    chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj =
        (chains_issMultCaptIspSimcopSv_DisplayAppObj *)appData;

    UTILS_assert(NULL != pObj);
    UTILS_assert(NULL != pAewbAlgOut);

    for ( chId = 0U; chId < pObj->ucObj.IssCapturePrm.numCh; chId++)
    {
        pAewbAlgOut->channelId = chId;
        /* AEWB Output parameters are already converted and stored in
           ispCfg parameter of alg out, so set it in the ISP using ISP
           Link */
        System_linkControl(
            pObj->ucObj.IssM2mIspLinkID,
            ISSM2MISP_LINK_CMD_SET_AEWB_PARAMS,
            pAewbAlgOut,
            sizeof(IssAewbAlgOutParams),
            TRUE);

        ChainsCommon_UpdateAewbParams(pAewbAlgOut);
    }
}

static Void chains_issMultCaptIspSimcopSv_Display_Config_DCC_Params(
        IssIspConfigurationParameters *ispCfg,
        IssM2mSimcopLink_ConfigParams *simcopCfg,
        Void                          *appData)
{
    UInt32 chId;
    chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj =
        (chains_issMultCaptIspSimcopSv_DisplayAppObj *)appData;

    UTILS_assert(NULL != pObj);
    UTILS_assert(NULL != ispCfg);
    UTILS_assert(NULL != simcopCfg);

    for ( chId = 0U; chId < pObj->ucObj.IssCapturePrm.numCh; chId++)
    {
        ispCfg->channelId = chId;

        System_linkControl(
            pObj->ucObj.IssM2mIspLinkID,
            ISSM2MISP_LINK_CMD_SET_ISPCONFIG,
            ispCfg,
            sizeof(IssIspConfigurationParameters),
            TRUE);
    }

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

#ifdef INCLUDE_SIMCOP
        /* MUST be called after link create and before link start */
        System_linkControl(
                pObj->ucObj.IssM2mSimcopLinkID,
                ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG,
                &pObj->simcopConfig,
                sizeof(pObj->simcopConfig),
                TRUE);
#endif /* #ifdef INCLUDE_SIMCOP */

    }
}

#ifdef ENABLE_WDR_MERGE_PARAMS_CFG
/* Function to get the exposure parameters from the sensor and update
   the merge parameters in the ISP. Called only when WDR merge is enabled. */
static Void chains_issMultCaptIspSimcopSv_Display_Config_Merge(
        IssAewbAlgOutParams *pAewbAlgOut,
        Void *appData)
{
    chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj =
        (chains_issMultCaptIspSimcopSv_DisplayAppObj *)appData;

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


static Void chains_issMultCaptIspSimcopSv_Display_SetIspConfig(
            chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj)
{
    Int32 status;
    UInt32 numCh;

    /* set default params */
    IssM2mSimcopLink_ConfigParams_Init(&pObj->simcopConfig);
    vpsissLdcCfg_init(&pObj->ldcCfg);
    vpsissVtnfCfg_init(&pObj->vtnfCfg);

    pObj->simcopConfig.ldcConfig = &pObj->ldcCfg;
    pObj->simcopConfig.vtnfConfig = &pObj->vtnfCfg;

    /* Set the Default SimCop configuration,
        This could get overwritten by DCC */
    ChainsCommon_SetIssSimcopConfig(
        &pObj->simcopConfig,
        pObj->bypassVtnf,
        pObj->bypassLdc,
        0);

    /* Assuming all channels will be using same isp operating mode */
    ChainsCommon_GetIssIspConfig(
        pObj->chainsCfg->captureSrc,
        pObj->ucObj.Alg_IssAewbLinkID,
        pObj->ucObj.IssM2mIspPrm.channelParams[0U].operatingMode,
        &pObj->ispConfig,
        &pObj->simcopConfig);

    /* MUST be called after link create and before link start */
    /* Apply the same config to all channel,
        right now the sensor are the same so its fine. */
    for (numCh = 0U; numCh < pObj->ucObj.IssCapturePrm.numCh; numCh++)
    {
        pObj->ispConfig.channelId = numCh;
        status = System_linkControl(
                pObj->ucObj.IssM2mIspLinkID,
                ISSM2MISP_LINK_CMD_SET_ISPCONFIG,
                &pObj->ispConfig,
                sizeof(pObj->ispConfig),
                TRUE);
        UTILS_assert(0 == status);
    }

#ifdef INCLUDE_SIMCOP
    /* MUST be called after link create and before link start */
    status = System_linkControl(
            pObj->ucObj.IssM2mSimcopLinkID,
            ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG,
            &pObj->simcopConfig,
            sizeof(pObj->simcopConfig),
            TRUE);
    UTILS_assert(0 == status);
#endif /* #ifdef INCLUDE_SIMCOP */

}

static Void chains_issMultCaptIspSimcopSv_Display_SetDccConfig(
            chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj)
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

        if (pObj->chainsCfg->captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
        {
            camInfo.width = SV_CAPT_SENSOR_AR140_WIDTH;
            camInfo.height = SV_CAPT_SENSOR_AR140_HEIGHT;
        }
        else
        {
            /* Un Recognized Capture source */
            UTILS_assert(FALSE);
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


Void chains_issMultCaptIspSimcopSv_Display_SetAppPrms(chains_issMultCaptIspSimcopSv_DisplayObj *pUcObj, Void *appObj)
{
    chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj
        = (chains_issMultCaptIspSimcopSv_DisplayAppObj*)appObj;
    IssM2mIspLink_OperatingMode ispOpMode;
    Int16 carBoxWidth;
    Int16 carBoxHeight;

    pObj->captureOutWidth  = SV_CAPT_SENSOR_AR140_WIDTH;
    pObj->captureOutHeight = SV_CAPT_SENSOR_AR140_HEIGHT;

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    pObj->simcopMode = ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF;
    pObj->bypassVtnf = FALSE;
    pObj->bypassLdc  = FALSE;

    if(pObj->chainsCfg->issLdcEnable == FALSE &&
       pObj->chainsCfg->issVtnfEnable == TRUE)
    {
        pObj->simcopMode = ISSM2MSIMCOP_LINK_OPMODE_VTNF;
        pObj->bypassVtnf = FALSE;
        pObj->bypassLdc  = TRUE;
    }
    else if(pObj->chainsCfg->issLdcEnable == TRUE &&
            pObj->chainsCfg->issVtnfEnable == FALSE)
    {
        pObj->simcopMode = ISSM2MSIMCOP_LINK_OPMODE_LDC;
        pObj->bypassVtnf = TRUE;
        pObj->bypassLdc  = FALSE;
    }
    else if(pObj->chainsCfg->issLdcEnable == FALSE &&
       pObj->chainsCfg->issVtnfEnable == FALSE)
    {
        pObj->simcopMode = ISSM2MSIMCOP_LINK_OPMODE_VTNF;
        pObj->bypassVtnf = TRUE;
        pObj->bypassLdc  = TRUE;
    }

    /* Initialize Simcop Pointers */
    pObj->simcopConfig.ldcConfig = &pObj->ldcCfg;
    pObj->simcopConfig.vtnfConfig = &pObj->vtnfCfg;

    ispOpMode = ISSM2MISP_LINK_OPMODE_12BIT_LINEAR;

#ifdef INCLUDE_SIMCOP
    ChainsCommon_SetIssCreatePrms(
        &pUcObj->IssCapturePrm,
        &pUcObj->IssM2mIspPrm,
        &pUcObj->IssM2mSimcopPrm,
        &pUcObj->Alg_IssAewbPrm,
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight,
        0U, /* RSZ B */
        0U, /* RSZ B */
        pObj->simcopMode,
        ispOpMode,
        NULL);
#else
    ChainsCommon_SetIssCreatePrms(
        &pUcObj->IssCapturePrm,
        &pUcObj->IssM2mIspPrm,
        NULL,
        &pUcObj->Alg_IssAewbPrm,
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        SV_ISP_OUT_HEIGHT,
        0U, /* RSZ B */
        0U, /* RSZ B */
        pObj->simcopMode,
        ispOpMode,
        NULL);
#endif /* #ifdef INCLUDE_SIMCOP */

    /* Override for multiple channel capture */
    ChainsCommon_MultipleCam_UpdateIssCapturePrms (
        &pUcObj->IssCapturePrm,
        pObj->chainsCfg->captureSrc,
        ispOpMode, pObj->captureOutWidth, pObj->captureOutHeight);

    /* Since we are operating in Linear mode, disable backlight compensation */
    if (ispOpMode == ISSM2MISP_LINK_OPMODE_12BIT_LINEAR)
    {
        pUcObj->Alg_IssAewbPrm.aeDynParams.enableBlc = FALSE;
    }

    pUcObj->Alg_IssAewbPrm.appData = pObj;
    pUcObj->Alg_IssAewbPrm.cfgCbFxn = chains_issMultCaptIspSimcopSv_Display_Config_Aewb;

    #ifdef ENABLE_WDR_MERGE_PARAMS_CFG
    /* Only support for two pass wdr flow */
    if (CHAINS_ISS_WDR_MODE_TWO_PASS == pObj->chainsCfg->issWdrMode)
    {
        pUcObj->Alg_IssAewbPrm.mergeCbFxn =
            chains_issMultCaptIspSimcopSv_Display_Config_Merge;
    }
    else
    {
        pUcObj->Alg_IssAewbPrm.mergeCbFxn = NULL;
    }
    #endif

    pUcObj->Alg_IssAewbPrm.dccIspCfgFxn =
        chains_issMultCaptIspSimcopSv_Display_Config_DCC_Params;

    pUcObj->IssCapturePrm.allocBufferForRawDump = TRUE;

    chains_issMultCaptIspSimcopSv_Display_SetSelectlinkPrms(&pUcObj->SelectPrm);


    ChainsCommon_SurroundView_SetParams(
        NULL, // Capture Params
        NULL,
        NULL,
        NULL, //&pUcObj->SelectPrm,
        NULL, //&pUcObj->VPE_sv_orgPrm1,
        NULL, //&pUcObj->VPE_sv_orgPrm2,
        &pUcObj->Sync_svPrm,
        NULL, //&pUcObj->Sync_sv_orgPrm1,
        NULL, //&pUcObj->Sync_sv_orgPrm2,
        &pUcObj->Alg_SynthesisPrm,
        &pUcObj->Alg_GeoAlignPrm,
        &pUcObj->Alg_PhotoAlignPrm,
        NULL,
        NULL,
        NULL,
        NULL, //&pUcObj->Alg_DmaSwMs_sv_orgPrm1,
        NULL, //&pUcObj->Alg_DmaSwMs_sv_orgPrm2,
        &pUcObj->GrpxSrcPrm,
        &pUcObj->Display_VideoPrm,
        NULL, //&pUcObj->Display_sv_orgPrm1,
        NULL, //&pUcObj->Display_sv_orgPrm12,
        &pUcObj->Display_GrpxPrm,
        pObj->chainsCfg->displayType,
        pUcObj->IssCapturePrm.numCh,
        ALGORITHM_LINK_SRV_OUTPUT_2D, //pObj->chainsCfg->svOutputMode,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        TRUE /* Enable CAR overlay */
        );

        carBoxWidth = SV_CARBOX_HEIGHT;
        carBoxHeight = SV_CARBOX_WIDTH;

    chains_issMultCaptIspSimcopSv_Display_SetSyncPrm(
                    &pUcObj->Sync_svPrm,
                    pUcObj->IssCapturePrm.numCh, 0x0); /* Sync period not used
                                                            in this func now */

    ChainsCommon_SurroundView_SetSynthParams(&pUcObj->Alg_SynthesisPrm,
                                            SV_CAPT_SENSOR_AR140_WIDTH,
                                            SV_ISP_OUT_HEIGHT,
                                            SV_HOR_WIDTH,
                                            SV_HOR_HEIGHT,
                                            SV_NUM_VIEWS,
                                            carBoxWidth,
                                            carBoxHeight,
                                            ALGORITHM_LINK_SRV_OUTPUT_2D,
                                            TRUE); /* Enable CAR overlay */

    ChainsCommon_SurroundView_SetGAlignParams(&pUcObj->Alg_GeoAlignPrm,
                                            SV_CAPT_SENSOR_AR140_WIDTH,
                                            SV_ISP_OUT_HEIGHT,
                                            SV_HOR_WIDTH,
                                            SV_HOR_HEIGHT,
                                            SV_NUM_VIEWS,
                                            carBoxWidth,
                                            carBoxHeight,
                                            ALGORITHM_LINK_SRV_OUTPUT_2D);

    /* Override parameters specific to this use case */
    pUcObj->Alg_GeoAlignPrm.ignoreFirstNFrames =
                                    SV_ALGO_ALIGN_IGNORE_FIRST_N_FRAMES;
    pUcObj->Alg_GeoAlignPrm.defaultFocalLength =
                                    SV_ALGO_ALIGN_DEFAULT_FOCAL_LENGTH;

    ChainsCommon_SurroundView_SetPAlignParams(&pUcObj->Alg_PhotoAlignPrm,
                                            SV_CAPT_SENSOR_AR140_WIDTH,
                                            SV_ISP_OUT_HEIGHT,
                                            SV_HOR_WIDTH,
                                            SV_HOR_HEIGHT,
                                            SV_NUM_VIEWS,
                                            carBoxWidth,
                                            carBoxHeight,
                                            ALGORITHM_LINK_SRV_OUTPUT_2D);
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
 * \param   pObj  [IN] chains_issMultCaptIspSimcopSv_DisplayAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
static Void chains_issMultCaptIspSimcopSv_Display_StartApp(chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_SetWdrMode(pObj->chainsCfg->issWdrMode);

    /* Video sensor layer is used to get the dcc bin file and other information,
       to set the ISP configuration, so Video sensor is created first */
    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight);

    chains_issMultCaptIspSimcopSv_Display_SetDccConfig(pObj);

    /* Sets the Simcop Config also */
    chains_issMultCaptIspSimcopSv_Display_SetIspConfig(pObj);

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_issMultCaptIspSimcopSv_Display_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   chains_issMultCaptIspSimcopSv_DisplayAppObj
 *
 *******************************************************************************
*/
static Void chains_issMultCaptIspSimcopSv_Display_StopAndDeleteApp(chains_issMultCaptIspSimcopSv_DisplayAppObj *pObj)
{
    chains_issMultCaptIspSimcopSv_Display_Stop(&pObj->ucObj);
    chains_issMultCaptIspSimcopSv_Display_Delete(&pObj->ucObj);

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
 * \brief   4 Channel surround view usecase entry function
 *
 *          This function configure, creates, link various links to establish
 *          usecase.
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
Void Chains_issMultCaptIspSimcopSv_Display(Chains_Ctrl *chainsCfg)
{
    char ch, chPrev, tempCh;
    UInt32 done = FALSE;
    Bool startWithCalibration;
    chains_issMultCaptIspSimcopSv_DisplayAppObj chainsObj;
    Chains_CaptureSrc oldCaptSrc;
    chainsObj.bypassVtnf = 0; /* KW error fix */
    chainsObj.bypassLdc  = 0; /* KW error fix */

    if ((CHAINS_ISS_WDR_MODE_SINGLE_PASS != chainsCfg->issWdrMode) &&
        (CHAINS_ISS_WDR_MODE_DISABLED != chainsCfg->issWdrMode))
    {
        Vps_printf(" CHAINS: WDR should either be disabled or "
                    "in One Pass mode!!!\n");
        return;
    }

    oldCaptSrc = chainsCfg->captureSrc;
    chainsCfg->displayType = CHAINS_DISPLAY_TYPE_HDMI_720P;
    chainsCfg->captureSrc = CHAINS_CAPTURE_SRC_UB960_TIDA00262;
    chainsObj.chainsCfg = chainsCfg;

    do
    {
        done = FALSE;
        /* Set startWithCalibration = TRUE to start the demo with calibration.
           Else it will use the previously calibrated LUTs */
        startWithCalibration = TRUE;
        ChainsCommon_SurroundView_CalibInit(startWithCalibration);
        /* Initialize Video Sensor, so that Algorithm can use Params
           from Vid Sensor layer */
        ChainsCommon_InitCaptureDevice(chainsCfg->captureSrc);
        chains_issMultCaptIspSimcopSv_Display_Create(&chainsObj.ucObj, &chainsObj);
        chains_issMultCaptIspSimcopSv_Display_StartApp(&chainsObj);

        do
        {
            Vps_printf(gChains_IssMultCaptIspSimcopSv_Display_runTimeMenu);

            ch = Chains_readChar();

            switch(ch)
            {
                case '0':
                    done = TRUE;
                    chPrev = '3';
                    break;
                case '1':
                    chPrev = ChainsCommon_SurroundView_MenuCalibration();
                    done = TRUE;
                    break;
                case '2':
                    System_linkControl(
                            chainsObj.ucObj.IssCaptureLinkID,
                            ISSCAPTURE_LINK_CMD_SAVE_FRAME,
                            NULL,
                            0,
                            TRUE);
                    break;
                case 'p':
                case 'P':
                    ChainsCommon_PrintStatistics();
                    chains_issMultCaptIspSimcopSv_Display_printStatistics(
                                    &chainsObj.ucObj);
                    break;
                default:
                    Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                    break;
            }
        } while ((chPrev!='3') && (FALSE == done));

        chains_issMultCaptIspSimcopSv_Display_StopAndDeleteApp(&chainsObj);
        ChainsCommon_SurroundView_CalibDeInit();
        Vps_printf("\n Power Cycle UB960 and enter a number\n");
        tempCh = Chains_readChar();
        Vps_printf("\n Restarting %d \n", tempCh);

    } while (chPrev!='3');
    chainsCfg->captureSrc = oldCaptSrc;
}

