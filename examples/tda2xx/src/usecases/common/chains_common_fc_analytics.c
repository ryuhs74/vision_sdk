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

#define PD_TSR_SWITCH_TIME_IN_SECS      (30)

#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

#define FEATUREPLANE_ALG_WIDTH    (640)
#define FEATUREPLANE_ALG_HEIGHT   (360)

#define LD_ALG_WIDTH              (640)
#define LD_ALG_HEIGHT             (360)

#define FEATUREPLANE_NUM_OUT_BUF  (3)
#define LIMP_HOME_DISPLAY_DURATION_MS   (24*60*60*1000)
#define LIMP_HOME_DISPLAY_FONTID        (5)

/**
 *******************************************************************************
 *
 *  \brief  SingleCameraAnalyticsObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    UInt32  objectDetectStartX;
    UInt32  objectDetectStartY;
    UInt32  objectDetectStartX2;
    UInt32  objectDetectStartY2;
    UInt32  objectDetectWidth;
    UInt32  objectDetectHeight;

    UInt32  laneDetectStartX;
    UInt32  laneDetectStartY;
    UInt32  laneDetectWidth;
    UInt32  laneDetectHeight;

    UInt32  sofStartX;
    UInt32  sofStartY;
    UInt32  sofWidth;
    UInt32  sofHeight;

    UInt32  enablePD;
    UInt32  enableTSR;

    UInt32 Alg_ObjectDrawLinkID;
    UInt32 Alg_FeaturePlaneComputationLinkID;
    UInt32 Alg_ObjectDetectionLinkID;

    Chains_CaptureSrc captureSrc;
    Chains_DisplayType displayType;

    Bool   taskExit;
    BspOsal_TaskHandle taskHndl;

} ChainsCommon_AnalyticsObj;

/**
 *******************************************************************************
 * \brief stack for PersMat table write task
 *******************************************************************************
 */
#pragma DATA_ALIGN(gChainsCommon_Analytics_tskStack, 32)
#pragma DATA_SECTION(gChainsCommon_Analytics_tskStack, ".bss:taskStackSection")
UInt8 gChainsCommon_Analytics_tskStack[4*1024];

ChainsCommon_AnalyticsObj gChainsCommon_analyticsObj;


/**
 *******************************************************************************
 *
 * \brief   Set PD draw parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_Analytics_SetObjectDetectPrms(
                    ChainsCommon_AnalyticsObj *pObj,
                   AlgorithmLink_FeaturePlaneComputationCreateParams *pFeatureComputePrm,
                   AlgorithmLink_ObjectDetectionCreateParams *pObjectDetectPrm,
                   AlgorithmLink_ObjectDrawCreateParams *pDrawPrm,
                   Chains_CaptureSrc captureSrc,
                   UInt32 width,
                   UInt32 height,
                   Bool enablePD,
                   Bool enableTSR
)
{
    pFeatureComputePrm->numOutBuffers  = 6;
    pObjectDetectPrm->numOutBuffers  = 6;
    pDrawPrm->numOutBuffers = 6;

    pDrawPrm->imgFrameWidth    = width;
    pDrawPrm->imgFrameHeight   = height;

    pDrawPrm->pdRectThickness = 1;

    pFeatureComputePrm->imgFrameHeight = height;
    pFeatureComputePrm->imgFrameWidth  = width;

    pObjectDetectPrm->enablePD       = enablePD;
    pObjectDetectPrm->enableTSR      = enableTSR;

    if(captureSrc==CHAINS_CAPTURE_SRC_HDMI_720P
        ||
       captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P
    )
    {
        if(captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P)
        {
            pFeatureComputePrm->imgFrameStartX = pObj->objectDetectStartX;
            pFeatureComputePrm->imgFrameStartY = pObj->objectDetectStartY;

            pDrawPrm->imgFrameStartX = pObj->objectDetectStartX;
            pDrawPrm->imgFrameStartY = pObj->objectDetectStartY;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Algorithm related parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_Analytics_SetLaneDetectPrm(
                    AlgorithmLink_LaneDetectCreateParams *pAlgPrm,
                    AlgorithmLink_LaneDetectDrawCreateParams *pDrawPrm,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height
                    )
{
    pAlgPrm->numOutBuffers  = 6;
    pDrawPrm->numOutBuffers  = 6;

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
    pDrawPrm->imgFrameHeight = height;
    pDrawPrm->enableDrawLines = TRUE;
}

/**
 *******************************************************************************
 *
 * \brief   Set Algorithm related parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_Analytics_SetSparseOpticalFlowPrm(
                    AlgorithmLink_SparseOpticalFlowCreateParams *pAlgPrm,
                    AlgorithmLink_sparseOpticalFlowDrawCreateParams *pDrawPrm,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height
                    )
{
    pAlgPrm->numOutBuffers = 6;
    pDrawPrm->numOutBuffers = 6;

    pAlgPrm->imgFrameStartX = startX;
    pAlgPrm->imgFrameStartY = startY;
    pAlgPrm->imgFrameWidth  = width;
    pAlgPrm->imgFrameHeight  = height;


    pDrawPrm->imgFrameStartX = startX;
    pDrawPrm->imgFrameStartY = startY;
    pDrawPrm->imgFrameWidth  = width;
    pDrawPrm->imgFrameHeight  = height;


}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Link Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipAnalyticsion_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_Analytics_SetSyncPrm(SyncLink_CreateParams *pPrm)

{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 1;
    pPrm->chParams.syncThreshold = 0x7FFFFFFF;
}


/**
 *******************************************************************************
 *
 * \brief   Set DMA SW Mosaic Create Parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_Analytics_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numCh,
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
    UInt32 secondRowFlag, numChBy2;

    useLocalEdma = FALSE;
    algId = ALGORITHM_LINK_IPU_ALG_DMA_SWMS;

    pPrm->baseClassCreate.algId   = algId;
    pPrm->numOutBuf               = 4;
    pPrm->useLocalEdma            = useLocalEdma;
    pPrm->initLayoutParams.numWin = numCh;

    switch(layoutType)
    {
        default:
        case CHAINS_COMMON_FC_ANALYTICS_LAYOUT_VERT_STRIP:
             /*
              * vertical strip
              */
            pPrm->maxOutBufWidth     = channelWidth;
            pPrm->maxOutBufHeight    = (channelHeight*(numCh)) +
                                       (channelSpacingVer*(numCh-1));

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
        case CHAINS_COMMON_FC_ANALYTICS_LAYOUT_HORZ_STRIP:
             /*
              * Horizontal strip
              */
            pPrm->maxOutBufWidth     = (channelWidth*(numCh)) +
                                       (channelSpacingHor*(numCh-1));
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

        case CHAINS_COMMON_FC_ANALYTICS_LAYOUT_HORZ_AND_VERT:
             /*
              * Two Horizontal strips
              */
            numChBy2 = (numCh+1) / 2;
            pPrm->maxOutBufWidth     = (channelWidth*(numChBy2)) +
                                       (channelSpacingHor*(numChBy2-1));
            pPrm->maxOutBufHeight    = (channelHeight*2) + channelSpacingVer;

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                secondRowFlag = ( winId>= numChBy2 ? 1 : 0);
                pWinInfo->outStartX = (winId % numChBy2) *(channelWidth+channelSpacingHor);
                pWinInfo->outStartY = secondRowFlag * (channelHeight+channelSpacingVer);
             }

            break;

    }

    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;
}

Void ChainsCommon_Analytics_SwitchPdTsr()
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;

    AlgorithmLink_ObjectDetectEnableAlgParams enableAlg;
    AlgorithmLink_FeaturePlaneComputationSetROIParams   roiPrm;
    AlgorithmLink_ObjectDrawSetROIParams   drawRoiPrm;

    if(pObj->captureSrc != CHAINS_CAPTURE_SRC_HDMI_1080P)
        return;

    /* ONLY valid for HDMI input */

    pObj->enablePD ^= 1;
    pObj->enableTSR ^= 1;

    enableAlg.baseClassControl.size = sizeof(enableAlg);
    enableAlg.baseClassControl.controlCmd = ALGORITHM_LINK_OBJECT_DETECT_CMD_ENABLE_ALG;
    enableAlg.enablePD = pObj->enablePD;
    enableAlg.enableTSR = pObj->enableTSR;

    roiPrm.baseClassControl.size = sizeof(roiPrm);
    roiPrm.baseClassControl.controlCmd = ALGORITHM_LINK_FEATURE_PLANE_COMPUTE_CMD_SET_ROI;
    roiPrm.imgFrameStartX = pObj->objectDetectStartX;
    roiPrm.imgFrameStartY = pObj->objectDetectStartY;
    roiPrm.imgFrameWidth  = pObj->objectDetectWidth;
    roiPrm.imgFrameHeight = pObj->objectDetectHeight;

    drawRoiPrm.baseClassControl.size = sizeof(drawRoiPrm);
    drawRoiPrm.baseClassControl.controlCmd = ALGORITHM_LINK_FEATURE_PLANE_COMPUTE_CMD_SET_ROI;
    drawRoiPrm.imgFrameStartX = pObj->objectDetectStartX;
    drawRoiPrm.imgFrameStartY = pObj->objectDetectStartY;
    drawRoiPrm.imgFrameWidth  = pObj->objectDetectWidth;
    drawRoiPrm.imgFrameHeight = pObj->objectDetectHeight;

    if(pObj->enableTSR)
    {
        roiPrm.imgFrameStartX = pObj->objectDetectStartX2;
        roiPrm.imgFrameStartY = pObj->objectDetectStartY2;

        drawRoiPrm.imgFrameStartX = pObj->objectDetectStartX2;
        drawRoiPrm.imgFrameStartY = pObj->objectDetectStartY2;
    }

    System_linkControl(
        pObj->Alg_FeaturePlaneComputationLinkID,
        ALGORITHM_LINK_CMD_CONFIG,
        &roiPrm,
        sizeof(roiPrm),
        TRUE
        );

    System_linkControl(
        pObj->Alg_ObjectDrawLinkID,
        ALGORITHM_LINK_CMD_CONFIG,
        &drawRoiPrm,
        sizeof(drawRoiPrm),
        TRUE
        );

    System_linkControl(
        pObj->Alg_ObjectDetectionLinkID,
        ALGORITHM_LINK_CMD_CONFIG,
        &enableAlg,
        sizeof(enableAlg),
        TRUE
        );
}

static Void ChainsCommon_Analytics_TskMain(UArg arg1, UArg arg2)
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;
    UInt32 curTime, elaspedTime;

    curTime = Utils_getCurTimeInMsec();

    while(!pObj->taskExit)
    {
        Task_sleep(10);

        elaspedTime = Utils_getCurTimeInMsec() - curTime;

        if(elaspedTime > PD_TSR_SWITCH_TIME_IN_SECS*1000)
        {
            curTime = Utils_getCurTimeInMsec();

            ChainsCommon_Analytics_SwitchPdTsr();
        }
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
Void ChainsCommon_Analytics_SetPrms(
            CaptureLink_CreateParams                *pCapturePrm,
            NullSrcLink_CreateParams                *pNullSrcPrm,
            DecLink_CreateParams                    *pDecPrm,
            AlgorithmLink_LaneDetectCreateParams    *pAlg_LaneDetectPrm,
            AlgorithmLink_LaneDetectDrawCreateParams *pAlg_LaneDetectDrawPrm,
            SyncLink_CreateParams                   *pSync_ldPrm,
            AlgorithmLink_FeaturePlaneComputationCreateParams *pAlg_FeaturePlaneComputationPrm,
            AlgorithmLink_ObjectDetectionCreateParams *pAlg_ObjectDetectionPrm,
            AlgorithmLink_ObjectDrawCreateParams    *pAlg_ObjectDrawPrm,
            SyncLink_CreateParams                   *pSync_pd_tsrPrm,
            AlgorithmLink_SparseOpticalFlowCreateParams *pSofAlgPrm,
            AlgorithmLink_sparseOpticalFlowDrawCreateParams *pSofDrawPrm,
            SyncLink_CreateParams                   *pSync_sofPrm,
            SyncLink_CreateParams                   *pSync_algPrm,
            AlgorithmLink_DmaSwMsCreateParams       *pAlg_DmaSwMsPrm,
            DisplayLink_CreateParams                *pDisplay_algPrm,
            GrpxSrcLink_CreateParams                *pGrpxSrcPrm,
            DisplayLink_CreateParams                *pDisplay_GrpxPrm,
            Chains_CaptureSrc captureSrc,
            Chains_DisplayType displayType,
            UInt32 Alg_FeaturePlaneComputationLinkID,
            UInt32 Alg_ObjectDetectionLinkID,
            UInt32 Alg_ObjectDrawLinkID,
            UInt32 dmaSwMsLayoutType
        )
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;

    pObj->Alg_FeaturePlaneComputationLinkID = Alg_FeaturePlaneComputationLinkID;
    pObj->Alg_ObjectDetectionLinkID = Alg_ObjectDetectionLinkID;
    pObj->Alg_ObjectDrawLinkID = Alg_ObjectDrawLinkID;

    pObj->captureSrc  = captureSrc;
    pObj->displayType = displayType;

    pObj->captureOutWidth  = FEATUREPLANE_ALG_WIDTH;
    pObj->captureOutHeight = FEATUREPLANE_ALG_HEIGHT;
    pObj->objectDetectStartX = 0;
    pObj->objectDetectStartY = 0;
    pObj->objectDetectStartX2 = 0;
    pObj->objectDetectStartY2 = 0;
    pObj->objectDetectWidth = FEATUREPLANE_ALG_WIDTH;
    pObj->objectDetectHeight = FEATUREPLANE_ALG_HEIGHT;
    pObj->laneDetectStartX = 0;
    pObj->laneDetectStartY = 0;
    pObj->laneDetectWidth = LD_ALG_WIDTH;
    pObj->laneDetectHeight = LD_ALG_HEIGHT;
    pObj->sofStartX = 0;
    pObj->sofStartY = 0;
    pObj->sofWidth = LD_ALG_WIDTH;
    pObj->sofHeight = LD_ALG_HEIGHT;

    pObj->enablePD = TRUE;
    pObj->enableTSR = TRUE;

    if(captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P)
    {
        pObj->captureOutWidth = 1920;
        pObj->captureOutHeight = 1080;

        pObj->objectDetectStartX  = 240;
        pObj->objectDetectStartY  = 120;
        pObj->objectDetectStartX2 = 240;
        pObj->objectDetectStartY2 = 600;

        pObj->laneDetectStartX = 1040;
        pObj->laneDetectStartY = 120;

        pObj->sofStartX = 240;
        pObj->sofStartY = 600;

        pObj->enablePD = TRUE;
        pObj->enableTSR = FALSE;

    }

    ChainsCommon_GetDisplayWidthHeight(
        displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    if(pNullSrcPrm && pDecPrm)
    {
        ChainsCommon_SetNetworkRxPrms(
            pNullSrcPrm,
            pDecPrm,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            1,
            30);
    }

    if(pCapturePrm)
    {
        ChainsCommon_SingleCam_SetCapturePrms(pCapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            captureSrc
            );
    }

    ChainsCommon_SetGrpxSrcPrms(pGrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );

    ChainsCommon_SetDisplayPrms(pDisplay_algPrm,
                                pDisplay_GrpxPrm,
                                displayType,
                                pObj->displayWidth,
                                pObj->displayHeight
                                );


    /* PD + TSR + LD */
    pDisplay_algPrm->rtParams.tarHeight = pObj->displayHeight/2;
    pDisplay_algPrm->rtParams.posY      = pObj->displayHeight/4;
    pGrpxSrcPrm->pdTsrLdLayoutEnable = TRUE;
    pGrpxSrcPrm->pdTsrLdSofLayoutEnable = FALSE;
    pGrpxSrcPrm->pdTsrLdSofStereoLayoutEnable = FALSE;

    if(pSofAlgPrm
        &&
       pSofDrawPrm
        &&
       pSync_sofPrm
        )
    {
        /* PD + TSR + LD + SOF */
        pGrpxSrcPrm->pdTsrLdLayoutEnable = FALSE;
        pGrpxSrcPrm->pdTsrLdSofLayoutEnable = TRUE;
    }

    if(dmaSwMsLayoutType==CHAINS_COMMON_FC_ANALYTICS_LAYOUT_HORZ_AND_VERT)
    {
        /* PD + TSR + LD + SOF + Stereo */
        pDisplay_algPrm->rtParams.tarWidth  = 800*2;
        pDisplay_algPrm->rtParams.posX      = 160;
        pDisplay_algPrm->rtParams.tarHeight = 450*2;
        pDisplay_algPrm->rtParams.posY      = 0;

        pGrpxSrcPrm->pdTsrLdLayoutEnable = FALSE;
        pGrpxSrcPrm->pdTsrLdSofLayoutEnable = FALSE;
        pGrpxSrcPrm->pdTsrLdSofStereoLayoutEnable = TRUE;
    }

    ChainsCommon_Analytics_SetObjectDetectPrms(
                    pObj,
                    pAlg_FeaturePlaneComputationPrm,
                    pAlg_ObjectDetectionPrm,
                    pAlg_ObjectDrawPrm,
                    captureSrc,
                    pObj->objectDetectWidth,
                    pObj->objectDetectHeight,
                    pObj->enablePD,
                    pObj->enableTSR
                );

    ChainsCommon_Analytics_SetLaneDetectPrm(
        pAlg_LaneDetectPrm,
        pAlg_LaneDetectDrawPrm,
        pObj->laneDetectStartX,
        pObj->laneDetectStartY,
        pObj->laneDetectWidth,
        pObj->laneDetectHeight
        );

    ChainsCommon_Analytics_SetSyncPrm(
                    pSync_ldPrm
                );

    ChainsCommon_Analytics_SetSyncPrm(
                    pSync_pd_tsrPrm
                );

    pSync_algPrm->chParams.numCh = 2;
    pSync_algPrm->chParams.syncDelta = 0x7FFFFFFF;
    pSync_algPrm->chParams.syncThreshold = 0x7FFFFFFF;

    if(pSofAlgPrm
        &&
       pSofDrawPrm
        &&
       pSync_sofPrm
        )
    {
        pSync_algPrm->chParams.numCh = 3;

        ChainsCommon_Analytics_SetSyncPrm(
                    pSync_sofPrm
                );

        ChainsCommon_Analytics_SetSparseOpticalFlowPrm(
            pSofAlgPrm,
            pSofDrawPrm,
            pObj->sofStartX,
            pObj->sofStartY,
            pObj->sofWidth,
            pObj->sofHeight
            );
    }

    ChainsCommon_Analytics_SetAlgDmaSwMsPrm(
        pAlg_DmaSwMsPrm,
        pSync_algPrm->chParams.numCh,
        pObj->objectDetectWidth,
        pObj->objectDetectHeight,
        dmaSwMsLayoutType,
        0,
        0
        );

    ChainsCommon_StartDisplayCtrl(
        displayType,
        pObj->displayWidth,
        pObj->displayHeight
        );
}

Void ChainsCommon_Analytics_Start(Bool useVipCapture)
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;
    Int32 status;

    ChainsCommon_StartDisplayDevice(pObj->displayType);

    if(useVipCapture)
    {
        ChainsCommon_StartCaptureDevice(
            pObj->captureSrc,
            pObj->captureOutWidth,
            pObj->captureOutHeight
        );
    }

    /*
     * Create a task to update the peraMat table into QSPI flash after 5 min
     */
    pObj->taskExit = FALSE;

    pObj->taskHndl = BspOsal_taskCreate(
                            (BspOsal_TaskFuncPtr)ChainsCommon_Analytics_TskMain,
                            4,
                            gChainsCommon_Analytics_tskStack,
                            sizeof(gChainsCommon_Analytics_tskStack),
                            NULL
                           );
    UTILS_assert(pObj->taskHndl != NULL);

    status = Utils_prfLoadRegister(pObj->taskHndl,
                                   "CHAINS_ANALYTICS_TSK");
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

}

Void ChainsCommon_Analytics_Stop(Bool useVipCapture)
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;

    ChainsCommon_StopDisplayCtrl();
    if(useVipCapture)
    {
        ChainsCommon_StopCaptureDevice(pObj->captureSrc);
    }
    ChainsCommon_StopDisplayDevice(pObj->displayType);

    pObj->taskExit = TRUE;

    Utils_prfLoadUnRegister(pObj->taskHndl);

    BspOsal_taskDelete(&pObj->taskHndl);
}
/**
 *******************************************************************************
 * \brief Run Time Menu string.
 *******************************************************************************
 */
char gChainsCommon_runTimeMenu_singleCamAnalytics[] = {
    "\r\n "
    "\r\n ===================="
    "\r\n Chains Run-time Menu"
    "\r\n ===================="
    "\r\n "
    "\r\n 0: Stop Chain"
    "\r\n "
    "\r\n p: Print Performance Statistics "
    "\r\n "
    "\r\n t: Show Thermal Configuration Menu "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 * \brief   Run time Menu selection for the single camera analytics usecase
 *
 *          This functions displays the run time options available
 *          And receives user input and calls corrosponding functions run time
 *          Instrumentation logs are printing routine is called in same function
 *
 *******************************************************************************
*/
char ChainsCommon_menuRunTime_SingleCamAnalytics()
{
    Vps_printf(gChainsCommon_runTimeMenu_singleCamAnalytics);

    return Chains_readChar();
}

/**
 *******************************************************************************
 * \brief Run Time Menu Thermal Menu string.
 *******************************************************************************
 */
char gChainsCommon_thermalRunTimeDesc[] = {
    "\r\n "
    "\r\n =============================="
    "\r\n Thermal Management Description"
    "\r\n =============================="
    "\r\n "
    "\r\n The Thermal Management of the device involves reducing the power"
    "\r\n consumption when the temperature of the device becomes hotter than"
    "\r\n the desired THOT temperature. When the temperature becomes lower than"
    "\r\n TCOLD temperature the device power consumption can be restored."
    "\r\n "
    "\r\n The control of power consumption is done by reducing the FPS of"
    "\r\n of the usecase by dropping frames in the capture thread."
    "\r\n "
    "\r\n After every thermal event the temperature thresholds would be changed"
    "\r\n to make sure THOT and TCOLD are as below:"
    "\r\n "
    "\r\n      THOT Temperature    ------------------------"
    "\r\n                                 ^"
    "\r\n                                 |"
    "\r\n                             Step Size"
    "\r\n                                 |"
    "\r\n                                 V"
    "\r\n      Current Temperature ------------------------"
    "\r\n                                 ^"
    "\r\n                                 |"
    "\r\n                             Step Size"
    "\r\n                                 |"
    "\r\n                                 V"
    "\r\n      TCOLD Temperature    ------------------------"
    "\r\n "
};
char gChainsCommon_thermalRunTimeMenu[] = {
    "\r\n "
    "\r\n ============================"
    "\r\n Thermal Menu Options:"
    "\r\n ============================"
    "\r\n "
    "\r\n 1: Change THOT Temperature"
    "\r\n 2: Change TCOLD Temperature"
    "\r\n 3: Show current THOT Temperature"
    "\r\n 4: Show current TCOLD Temperature"
    "\r\n 5: Change Threshold Step Size"
    "\r\n 6: Show Limp Home Status"
    "\r\n x: Exit Thermal Menu"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChainsCommon_thermalTemperatureMenu[] = {
    "\r\n Available options for Temperature: "
    "\r\n "
    "\r\n 1: 100 deg C"
    "\r\n 2: 80 deg C"
    "\r\n 3: 60 deg C"
    "\r\n 4: 50 deg C"
    "\r\n 5: 45 deg C"
    "\r\n 6: 40 deg C"
    "\r\n 7: 35 deg C"
    "\r\n 8: 30 deg C"
    "\r\n 9: 20 deg C"
    "\r\n a: 10 deg C"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChainsCommon_thermalStepSizeMenu[] = {
    "\r\n Available options for Step Size: "
    "\r\n "
    "\r\n 1: 3  deg C"
    "\r\n 2: 5  deg C"
    "\r\n 3: 10 deg C"
    "\r\n 4: 15 deg C"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 * \brief   Run time Update of the Thermal Thresholds.
 *
 *          This functions displays the run time options available
 *          And receives user input and calls corrosponding functions run time
 *
 *******************************************************************************
*/
Void ChainsCommon_thermalConfig()
{
    char ch, ch_temp;
    Vps_printf(gChainsCommon_thermalRunTimeDesc);
    do
    {
        Vps_printf(gChainsCommon_thermalRunTimeMenu);
        ch = Chains_readChar();
        switch (ch)
        {
            case '1':
                    Vps_printf(gChainsCommon_thermalTemperatureMenu);
                    ch_temp = Chains_readChar();
                    ChainsCommon_modifyTemperatureThreshold(ch_temp, CHAINS_HOT_TEMPERATURE);
                    break;
            case '2':
                    Vps_printf(gChainsCommon_thermalTemperatureMenu);
                    ch_temp = Chains_readChar();
                    ChainsCommon_modifyTemperatureThreshold(ch_temp, CHAINS_COLD_TEMPERATURE);
                    break;
            case '3':
                    ChainsCommon_readTemperatureThreshold(CHAINS_HOT_TEMPERATURE);
                    break;
            case '4':
                    ChainsCommon_readTemperatureThreshold(CHAINS_COLD_TEMPERATURE);
                    break;
            case '5':
                    Vps_printf(gChainsCommon_thermalStepSizeMenu);
                    ch_temp = Chains_readChar();
                    ChainsCommon_modifyTempThresholdStepSize(ch_temp);
                    break;
            case '6':
                    if (Utils_tempGetLimpHomeState())
                    {
                        Vps_printf("\n -------------------------");
                        Vps_printf("\n Limp Home Mode = ACTIVE!!\n");
                        Vps_printf("\n -------------------------");
                    }
                    else
                    {
                        Vps_printf("\n ----------------------------");
                        Vps_printf("\n Limp Home Mode = IN-ACTIVE!!\n");
                        Vps_printf("\n ----------------------------");
                    }
                    break;
            case 'X':
                    ch = 'x';
            case 'x':
                    break;
            default:
                    Vps_printf("\nUnsupported option '%c'. Please try again. \n", ch);
                    break;
        }
    } while (ch != 'x');
}

/**
 *******************************************************************************
 *
 * \brief   Run time Update of the Thermal Thresholds.
 *
 *          This function updates the THOD or TCOLD thresholds in the software
 *          structures and the hardware registers.
 *
 * \param   ch_temp[IN]     User selected THOT/TCOLD threshold
 * \param   eventType[IN]   THOT or TCOLD selection
 *
 *******************************************************************************
*/
Void ChainsCommon_modifyTemperatureThreshold(char ch_temp, UInt32 eventType)
{
    Int32 threshold = 0;
    UInt32 retVal = 0U;
    switch (ch_temp)
    {
        case '1': threshold = 100000;
                  break;
        case '2': threshold = 80000;
                  break;
        case '3': threshold = 60000;
                  break;
        case '4': threshold = 50000;
                  break;
        case '5': threshold = 45000;
                  break;
        case '6': threshold = 40000;
                  break;
        case '7': threshold = 35000;
                  break;
        case '8': threshold = 30000;
                  break;
        case '9': threshold = 20000;
                  break;
        case 'a': threshold = 10000;
                  break;
        default :
              Vps_printf("\nUnsupported option '%c'. \n", ch_temp);
              retVal = 1U;
    }
    if ( 0U == retVal )
    {
        if (eventType == CHAINS_HOT_TEMPERATURE)
        {
            Utils_tempChangeHotThreshold(threshold);
        }
        else if (eventType == CHAINS_COLD_TEMPERATURE)
        {
            Utils_tempChangeColdThreshold(threshold);
        }
        else
        {
            ; /* Should not reach here */
        }
    }
}
/**
 *******************************************************************************
 *
 * \brief   Run time read of the Thermal Thresholds.
 *
 *          This function reads the THOD or TCOLD thresholds in the software
 *          structures.
 *
 * \param   eventType[IN]   THOT or TCOLD selection
 *
 *******************************************************************************
*/
Void ChainsCommon_readTemperatureThreshold(UInt32 eventType)
{

    if ( eventType == CHAINS_HOT_TEMPERATURE)
    {
        Utils_tempReadAllHotThreshold();
    }
    else if (eventType == CHAINS_COLD_TEMPERATURE)
    {
        Utils_tempReadAllColdThreshold();
    }
    else
    {
        ; /* Should not reach here */
    }
}
/**
 *******************************************************************************
 *
 * \brief   Run time Update of the Thermal Threshold Step Size.
 *
 *          This function updates the Step Size in the software
 *          structures.
 *
 * \param   ch_temp[IN]     User selected THOT/TCOLD threshold
 *
 *******************************************************************************
*/
Void ChainsCommon_modifyTempThresholdStepSize(char ch_temp)
{
    UInt32 stepSize = 0U;
    UInt32 retVal = 0U;
    switch (ch_temp)
    {
        case '1':  stepSize = 3000U; break;
        case '2':  stepSize = 5000U; break;
        case '3':  stepSize = 10000U; break;
        case '4':  stepSize = 15000U; break;
        default :
              Vps_printf("\nUnsupported option '%c'. \n", ch_temp);
              retVal = 1U;
    }
    if (0U == retVal)
    {
        Utils_tempChangeStepSize(stepSize);
    }
}

/**
 *******************************************************************************
 *
 * \brief   Thermal Event Handler to handle Hot Events.
 *
 *          This function updates the frame skip parameter and sends a message
 *          to the capture link to update the fps. The hot and cold threshold
 *          is updated and the temperature interrupt is enabled.
 *          Handling VD_CORE is sufficient as other VDs are usually +/-5
 *          degrees different wrt VD_CORE temperature.
 *
 * \param   captureLinkId [IN]  Id to identify capture link.
 * \param   pPrm [IN]           Paramter to identify which voltage domain
 *                              generated the hot event.
 *
 *******************************************************************************
*/
Void ChainsCommon_tempHotEventHandler(UInt32 captureLinkId, Void* pPrm)
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;
    Chains_CaptureSrc captureSrc = pObj->captureSrc;
    pmhalPrcmVdId_t voltId = *(pmhalPrcmVdId_t *)pPrm;
    GrpxSrcLink_StringRunTimePrintParams printPrms;
    UInt32 frameSkip;
    Int32 threshold;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if (voltId == PMHAL_PRCM_VD_CORE)
    {
        Vps_printf(" CHAINS: TEMPERATURE: VD_CORE Hit Thermal Hot Event !\n");

        if(captureSrc==CHAINS_CAPTURE_SRC_HDMI_720P
                ||
               captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P
                ||
               captureSrc==CHAINS_CAPTURE_SRC_DM388
            )
        {
            /* Drop the FPS to 10 fps */
            frameSkip = 0x3EFBEFBE;
        }
        else
        {
            /* Drop the FPS to 10 fps */
            frameSkip = 0x36DB6DB6;
        }
        System_linkControl(captureLinkId, CAPTURE_LINK_CMD_SET_FRAME_SKIP_MASK,
                    &frameSkip, sizeof(frameSkip), TRUE);
        /* Change the threshold for all */
        threshold = Utils_tempGetCurrTemperature(PMHAL_PRCM_VD_CORE);
        if (threshold != UTILS_TEMP_INVALID)
        {
            /* Change Hot Threshold first before Cold for a Hot event
             * to make sure the Hot event is not generated when the
             * interrupt is enabled.
             */
            Utils_tempChangeHotThreshold(threshold + Utils_tempGetStepSize(PMHAL_PRCM_VD_CORE));
            Utils_tempChangeColdThreshold(threshold - Utils_tempGetStepSize(PMHAL_PRCM_VD_CORE));
        }
        else
        {
            Vps_printf(" CHAINS: TEMPERATURE: Get Temperature Failed !!\n");
        }
        Vps_printf(" CHAINS: TEMPERATURE:            HOT Threshold = [%d.%d]\n",
             Utils_tempGetHotThreshold(voltId)/1000,
             abs(Utils_tempGetHotThreshold(voltId))%1000);
        Vps_printf(" CHAINS: TEMPERATURE:            COLD Threshold = [%d.%d]\n",
             Utils_tempGetColdThreshold(voltId)/1000,
             abs(Utils_tempGetColdThreshold(voltId))%1000);
        Utils_tempUpdateAllVoltLimpHomeState(UTILS_TEMP_LIMP_HOME_ACTIVE);
        snprintf(printPrms.stringInfo.string,
                     sizeof(printPrms.stringInfo.string) - 1,
                     "LIMP HOME MODE ACTIVE \n");
        printPrms.stringInfo.string[sizeof(printPrms.stringInfo.string) - 1] = 0;
        printPrms.duration_ms = LIMP_HOME_DISPLAY_DURATION_MS;
        printPrms.stringInfo.fontType = LIMP_HOME_DISPLAY_FONTID;
        printPrms.stringInfo.startX  = pObj->displayWidth/2 + 200;
        printPrms.stringInfo.startY  = pObj->displayHeight-100;

        status = System_linkControl(IPU1_0_LINK(SYSTEM_LINK_ID_GRPX_SRC_0),
                           GRPX_SRC_LINK_CMD_PRINT_STRING,
                           &printPrms,
                           sizeof(printPrms),
                           TRUE);
        if (status != SYSTEM_LINK_STATUS_SOK)
        {
            Vps_printf(" CHAINS: TEMPERATURE: Send Command for Limp Home failed \n");
        }


    }
}

/**
 *******************************************************************************
 *
 * \brief   Thermal Event Handler to handle Cold Events.
 *
 *          This function updates the frame skip parameter and sends a message
 *          to the capture link to update the fps. The hot and cold threshold
 *          is updated and the temperature interrupt is enabled.
 *          Handling VD_CORE is sufficient as other VDs are usually +/-5
 *          degrees different wrt VD_CORE temperature.
 *
 * \param   captureLinkId [IN]  Id to identify capture link.
 * \param   pPrm [IN]           Paramter to identify which voltage domain
 *                              generated the hot event.
 *
 *******************************************************************************
*/
Void ChainsCommon_tempColdEventHandler(UInt32 captureLinkId, Void* pPrm)
{
    ChainsCommon_AnalyticsObj *pObj = &gChainsCommon_analyticsObj;
    Chains_CaptureSrc captureSrc = pObj->captureSrc;
    pmhalPrcmVdId_t voltId = *(pmhalPrcmVdId_t *)pPrm;
    GrpxSrcLink_StringRunTimePrintParams printPrms;
    UInt32 frameSkip;
    Int32 threshold;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if (voltId == PMHAL_PRCM_VD_CORE)
    {
        Vps_printf(" CHAINS: TEMPERATURE: VD_CORE Hit Thermal Cold Event !\n");

        if(captureSrc==CHAINS_CAPTURE_SRC_HDMI_720P
                ||
               captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P
                ||
               captureSrc==CHAINS_CAPTURE_SRC_DM388
            )
        {
            /* Restore the FPS to 30 fps */
            frameSkip = 0x2AAAAAAA;
        }
        else
        {
            /* Restore the FPS to 30 fps */
            frameSkip = 0x0;
        }
        System_linkControl(captureLinkId, CAPTURE_LINK_CMD_SET_FRAME_SKIP_MASK,
                    &frameSkip, sizeof(frameSkip), TRUE);
        /* Change the threshold for all */
        threshold = Utils_tempGetCurrTemperature(PMHAL_PRCM_VD_CORE);
        if (threshold != UTILS_TEMP_INVALID)
        {
            /* Change Cold Threshold first before Hot for a cold event
             * to make sure the cold event is not generated when the
             * interrupt is enabled.
             */
            Utils_tempChangeColdThreshold(threshold - Utils_tempGetStepSize(PMHAL_PRCM_VD_CORE));
            Utils_tempChangeHotThreshold(threshold + Utils_tempGetStepSize(PMHAL_PRCM_VD_CORE));
        }
        else
        {
            Vps_printf(" CHAINS: TEMPERATURE: Get Temperature Failed !!\n");
        }
        Vps_printf(" CHAINS: TEMPERATURE:            HOT Threshold = [%d.%d]\n",
             Utils_tempGetHotThreshold(voltId)/1000,
             abs(Utils_tempGetHotThreshold(voltId))%1000);
        Vps_printf(" CHAINS: TEMPERATURE:            COLD Threshold = [%d.%d]\n",
             Utils_tempGetColdThreshold(voltId)/1000,
             abs(Utils_tempGetColdThreshold(voltId))%1000);
        Utils_tempUpdateAllVoltLimpHomeState(UTILS_TEMP_LIMP_HOME_INACTIVE);
        snprintf(printPrms.stringInfo.string,
                     sizeof(printPrms.stringInfo.string) - 1,
                     "                      \n");
        printPrms.stringInfo.string[sizeof(printPrms.stringInfo.string) - 1] = 0;
        printPrms.duration_ms = LIMP_HOME_DISPLAY_DURATION_MS;
        printPrms.stringInfo.fontType = LIMP_HOME_DISPLAY_FONTID;
        printPrms.stringInfo.startX  = pObj->displayWidth/2 + 200;
        printPrms.stringInfo.startY  = pObj->displayHeight-100;

        status = System_linkControl(IPU1_0_LINK(SYSTEM_LINK_ID_GRPX_SRC_0),
                           GRPX_SRC_LINK_CMD_PRINT_STRING,
                           &printPrms,
                           sizeof(printPrms),
                           TRUE);
        if (status != SYSTEM_LINK_STATUS_SOK)
        {
            Vps_printf(" CHAINS: TEMPERATURE: Send Command for Limp Home failed \n");
        }

    }
}


