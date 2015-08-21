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
 * \filechains_vipSingleCamAnalytics_SgxDisplay.c
 *
 * \brief  Usecase file implementation of capture + PD + display usecase.
 *
 *
 *         In this use-case we capture 1 CH of video from OV1063x 720p30
 *         and send it to Pedestian detection algorithm (DSP1 + EVE1).
 *         The output is then send to A15 where A15 is running SgxDisplay.
 *         Link which will render the frames and display via DRM
 *
 *         The data flow daigram is shown is corrsponding
 *         chains_vipSingleCamAnalytics_SgxDisplay.jpg file
 *
 * \version 0.0 (Jun 2014) : [YM] First version ported for linux.
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "chains_vipSingleCamAnalytics_SgxDisplay_priv.h"
#include <linux/examples/tda2xx/include/chains.h>
#include <linux/examples/common/chains_common.h>

#define ENABLE_LD                       (0)

#define CAPTURE_SENSOR_WIDTH            (1280)
#define CAPTURE_SENSOR_HEIGHT           (720)

#define FEATUREPLANE_ALG_WIDTH          (640)
#define FEATUREPLANE_ALG_HEIGHT         (360)

#define FEATUREPLANE_NUM_OUT_BUF        (5)

#define FRAMES_DUMP_TO_MEMORY_ENABLE    (0)
#define FRAMES_DUMP_TO_MEMORY_ADDR      (0xA1000000)
#define FRAMES_DUMP_TO_MEMORY_SIZE      (496*MB)


/**
 *******************************************************************************
 *
 *  \brief  SingleCameraViewObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {
    /**< Link Id's and device IDs to use for this use-case */
    chains_vipSingleCamAnalytics_SgxDisplayObj ucObj;

    UInt32  appCtrlLinkId;
    UInt32  captureOutWidth;
    UInt32  captureOutHeight;

    Chains_Ctrl *chainsCfg;

} chains_vipSingleCamAnalytics_SgxDisplayAppObj;



/**
 *******************************************************************************
 *
 * \brief   Set PD draw parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void chains_vipSingleCamAnalytics_SgxDisplay_SetObjectDrawPrms(
                   AlgorithmLink_ObjectDrawCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    pPrm->imgFrameWidth    = width;
    pPrm->imgFrameHeight   = height;
    pPrm->numOutBuffers    = FEATUREPLANE_NUM_OUT_BUF;
    pPrm->pdRectThickness  = 3;
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
Void chains_vipSingleCamAnalytics_SgxDisplay_SetFeaturePlaneComputeAlgPrms(
                   chains_vipSingleCamAnalytics_SgxDisplayAppObj *pObj,
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
Void chains_vipSingleCamAnalytics_SgxDisplay_SetObjectDetectPrm(
                   AlgorithmLink_ObjectDetectionCreateParams *pPrm)
{
    pPrm->numOutBuffers  = FEATUREPLANE_NUM_OUT_BUF;
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
 *          Chains_VipPedestrainDetection_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCamAnalytics_SgxDisplay_SetSyncPrm(SyncLink_CreateParams *pPrm)

{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 1;
    pPrm->chParams.syncThreshold = 0xFFFF;
}

#if ENABLE_LD
Void chains_vipSingleCamAnalytics_SgxDisplay_SetSyncFcDisplayPrm
                                                (SyncLink_CreateParams *pPrm)

{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 0x7FFFFFFF;
    pPrm->chParams.syncThreshold = 0x7FFFFFFF;
}
#endif
/**
 *******************************************************************************
 *
 * \brief   Set SGXDISPLAY Link Parameters
 *
 *          It is called in Create function.
 *
 *
 * \param   pPrm    [IN]    IpcLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_vipSingleCamAnalytics_SgxDisplay_SetSgxDisplayLinkPrms (
                                  SgxDisplayLink_CreateParams *prms,
                                  UInt32 width, UInt32 height)
{
    prms->displayWidth = width;
    prms->displayHeight = height;
    prms->renderType = SGXDISPLAY_RENDER_TYPE_1x1;
    prms->inBufType = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
}



/**
 *******************************************************************************
 *
 * \brief   Set frame-rate to a fixed value instead of capture frame-rate
 *
 *******************************************************************************
 */
Void chains_vipSingleCamAnalytics_SgxDisplay_SetFrameRatePrms(
                    chains_vipSingleCamAnalytics_SgxDisplayAppObj *pObj)
{
    IpcLink_FrameRateParams frameRatePrms;

    frameRatePrms.chNum = 0;

    frameRatePrms.inputFrameRate = 30;

    frameRatePrms.outputFrameRate = 15;

    if(pObj->chainsCfg->captureSrc==CHAINS_CAPTURE_SRC_HDMI_720P
        ||
       pObj->chainsCfg->captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P
    )
    {
        /* Dont do frame-rate control for HDMI input */
        frameRatePrms.inputFrameRate = 60;
        frameRatePrms.outputFrameRate = 60;

        /* skip alternate frame to make it 30fps output */
        pObj->ucObj.CapturePrm.vipInst[0].outParams[0].frameSkipMask
            = 0x2AAAAAAA;
    }

    System_linkControl(
        pObj->ucObj.IPCOut_IPU1_0_EVE1_0LinkID,
        IPC_OUT_LINK_CMD_SET_FRAME_RATE,
        &frameRatePrms,
        sizeof(frameRatePrms),
        TRUE
        );
}
#if ENABLE_LD
/**
 *******************************************************************************
 *
 * \brief   Set Algorithm related parameters
 *
 *******************************************************************************
*/
Void chains_vipSingleCamAnalytics_SgxDisplay_SetLaneDetectPrm(
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

static Void chains_vipSingleCamAnalytics_SgxDisplay_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 channelWidth,
                    UInt32 channelHeight
                   )
{
    UInt32 algId, winId;
    UInt32 useLocalEdma;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;

    useLocalEdma = FALSE;
    algId = ALGORITHM_LINK_IPU_ALG_DMA_SWMS;

    pPrm->baseClassCreate.algId   = algId;
    pPrm->numOutBuf               = 4;
    pPrm->useLocalEdma            = useLocalEdma;
    pPrm->initLayoutParams.numWin = numLvdsCh;

     /*
      * Horizontal strip
      */
    pPrm->maxOutBufWidth     = (channelWidth*(numLvdsCh)) +
                               (10*(numLvdsCh-1));
    pPrm->maxOutBufHeight    = channelHeight;

    for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
    {
        pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
        pWinInfo->chId = winId;
        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;
        pWinInfo->width    = channelWidth;
        pWinInfo->height   = channelHeight;
        pWinInfo->outStartX = winId*(channelWidth+10);
        pWinInfo->outStartY = 0;
     }

    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

}
#endif

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
Void chains_vipSingleCamAnalytics_SgxDisplay_SetAppPrms(
               chains_vipSingleCamAnalytics_SgxDisplayObj *pUcObj,Void *appObj)
{
    UInt32 displayWidth, displayHeight;

    chains_vipSingleCamAnalytics_SgxDisplayAppObj *pObj
            = (chains_vipSingleCamAnalytics_SgxDisplayAppObj*)appObj;

    pObj->captureOutWidth  = FEATUREPLANE_ALG_WIDTH;
    pObj->captureOutHeight = FEATUREPLANE_ALG_HEIGHT;

    ChainsCommon_SingleCam_SetCapturePrms(&(pUcObj->CapturePrm),
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    chains_vipSingleCamAnalytics_SgxDisplay_SetFeaturePlaneComputeAlgPrms(
                    pObj,
                    &pUcObj->Alg_FeaturePlaneComputationPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT
                );

    chains_vipSingleCamAnalytics_SgxDisplay_SetObjectDetectPrm(
                    &pUcObj->Alg_ObjectDetectionPrm
                );

    chains_vipSingleCamAnalytics_SgxDisplay_SetSyncPrm(
                    &pUcObj->Sync_algPdPrm
                );

    chains_vipSingleCamAnalytics_SgxDisplay_SetObjectDrawPrms(
                    &pUcObj->Alg_ObjectDrawPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT
                );

#if ENABLE_LD
    AlgorithmLink_LaneDetect_Init(&pUcObj->Alg_LaneDetectPrm);
    AlgorithmLink_LaneDetectDraw_Init(&pUcObj->Alg_LaneDetectDrawPrm);

    chains_vipSingleCamAnalytics_SgxDisplay_SetLaneDetectPrm(
        &pUcObj->Alg_LaneDetectPrm,
        &pUcObj->Alg_LaneDetectDrawPrm,
        0,
        0,
        FEATUREPLANE_ALG_WIDTH,
        FEATUREPLANE_ALG_HEIGHT
        );

    chains_vipSingleCamAnalytics_SgxDisplay_SetSyncPrm(
                    &pUcObj->Sync_algLdPrm
                );


    chains_vipSingleCamAnalytics_SgxDisplay_SetSyncFcDisplayPrm(
                &pUcObj->Sync_algDisplayPrm);

    chains_vipSingleCamAnalytics_SgxDisplay_SetAlgDmaSwMsPrm(
                &pUcObj->Alg_DmaSwMs_algPrm,
                2,
                FEATUREPLANE_ALG_WIDTH,
                FEATUREPLANE_ALG_HEIGHT
                );
#endif

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &displayWidth,
        &displayHeight
        );

    chains_vipSingleCamAnalytics_SgxDisplay_SetSgxDisplayLinkPrms
                    (&pUcObj->SgxDisplayPrm,
                     displayWidth,
                     displayHeight
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
 * \param   pObj  [IN] chains_vipSingleCamAnalytics_SgxDisplayAppObj
 *
 *
 *******************************************************************************
*/
Void chains_vipSingleCamAnalytics_SgxDisplay_StartApp(
                     chains_vipSingleCamAnalytics_SgxDisplayAppObj *pObj)
{
    ChainsCommon_memPrintHeapStatus();

    chains_vipSingleCamAnalytics_SgxDisplay_Start(&pObj->ucObj);

    ChainsCommon_prfLoadCalcEnable(TRUE, FALSE, FALSE);

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
 * \param   pObj   [IN]   chains_vipSingleCamAnalytics_SgxDisplayAppObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCamAnalytics_SgxDisplay_StopApp(
                     chains_vipSingleCamAnalytics_SgxDisplayAppObj *pObj)
{

    chains_vipSingleCamAnalytics_SgxDisplay_Stop(&pObj->ucObj);

    chains_vipSingleCamAnalytics_SgxDisplay_Delete(&pObj->ucObj);

    ChainsCommon_prfLoadCalcEnable(FALSE, FALSE, FALSE);

    ChainsCommon_memPrintHeapStatus();

}

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture + PD + Display usecase function
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
Void chains_vipSingleCamAnalytics_SgxDisplay(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    chains_vipSingleCamAnalytics_SgxDisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    ChainsCommon_statCollectorReset();
    ChainsCommon_memPrintHeapStatus();

    chains_vipSingleCamAnalytics_SgxDisplay_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCamAnalytics_SgxDisplay_StartApp(&chainsObj);

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
                ChainsCommon_prfCpuLoadPrint();
                ChainsCommon_statCollectorPrint();
                chains_vipSingleCamAnalytics_SgxDisplay_printStatistics(&chainsObj.ucObj);
                chains_vipSingleCamAnalytics_SgxDisplay_printBufferStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCamAnalytics_SgxDisplay_StopApp(&chainsObj);
}

