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
#include "chains_vipSingleCameraObjectDetect_priv.h"
#include <examples/tda2xx/include/chains_common.h>

#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

#define FEATUREPLANE_ALG_WIDTH    (640)
#define FEATUREPLANE_ALG_HEIGHT   (360)


#define FRAMES_DUMP_TO_MEMORY_ENABLE    (0)
#define FRAMES_DUMP_TO_MEMORY_ADDR      (0xA1000000)
#define FRAMES_DUMP_TO_MEMORY_SIZE      (496*MB)

static char usecase_menu[] = {
    "\r\n "
    "\r\n Select use-case options,"
    "\r\n ------------------------"
    "\r\n 1: Enable Pedestrain Detect (PD)"
    "\r\n 2: Enable Traffic Sign Regonition (TSR)"
    "\r\n 3: Enable PD+TSR"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 *  \brief  SingleCameraObjectDetectObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipSingleCameraObjectDetectObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Bool    enablePD;
    Bool    enableTSR;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraObjectDetectAppObj;


/**
 *******************************************************************************
 *
 * \brief   Set PD draw parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraObjectDetect_SetObjectDrawPrms(
                   Chains_VipSingleCameraObjectDetectAppObj *pObj,
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
Void chains_vipSingleCameraObjectDetect_SetFeaturePlaneComputeAlgPrms(
                   Chains_VipSingleCameraObjectDetectAppObj *pObj,
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
Void chains_vipSingleCameraObjectDetect_SetObjectDetectPrm(
                   Chains_VipSingleCameraObjectDetectAppObj *pObj,
                   AlgorithmLink_ObjectDetectionCreateParams *pPrm
                   )
{
    pPrm->numOutBuffers  = 2;
    pPrm->enablePD       = pObj->enablePD;
    pPrm->enableTSR      = pObj->enableTSR;
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
Void chains_vipSingleCameraObjectDetect_SetSyncPrm(SyncLink_CreateParams *pPrm)

{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 1;
    pPrm->chParams.syncThreshold = 0xFFFF;
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
Void chains_vipSingleCameraObjectDetect_SetVpePrm(
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
    pPrm->chParams[0].outParams[0].numBufsPerCh = 2;
}

/**
 *******************************************************************************
 *
 * \brief   Set NULL Create Parameters
 *
 * \param   pPrm    [OUT]    NullLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraObjectDetect_SetNullPrm(
                    NullLink_CreateParams *pPrm
                    )
{
    pPrm->dumpDataType   = (NullLink_CopyType)NULL_LINK_COPY_TYPE_2D_MEMORY;
    pPrm->dumpFramesMemoryAddr = FRAMES_DUMP_TO_MEMORY_ADDR;
    pPrm->dumpFramesMemorySize = FRAMES_DUMP_TO_MEMORY_SIZE;
}

/**
 *******************************************************************************
 *
 * \brief   Set frame-rate to a fixed value instead of capture frame-rate
 *
 *******************************************************************************
 */
Void chains_vipSingleCameraObjectDetect_SetFrameRatePrms(
                    Chains_VipSingleCameraObjectDetectAppObj *pObj)
{
    IpcLink_FrameRateParams frameRatePrms;

    frameRatePrms.chNum = 0;

    frameRatePrms.inputFrameRate = 30;
    frameRatePrms.outputFrameRate = 30;

    System_linkControl(
        pObj->ucObj.IPCOut_IPU1_0_EVE1_0LinkID,
        IPC_OUT_LINK_CMD_SET_FRAME_RATE,
        &frameRatePrms,
        sizeof(frameRatePrms),
        TRUE
        );
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
Void chains_vipSingleCameraObjectDetect_SetAppPrms(chains_vipSingleCameraObjectDetectObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraObjectDetectAppObj *pObj
        = (Chains_VipSingleCameraObjectDetectAppObj*)appObj;

    pObj->captureOutWidth  = FEATUREPLANE_ALG_WIDTH;
    pObj->captureOutHeight = FEATUREPLANE_ALG_HEIGHT;

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SingleCam_SetCapturePrms(&(pUcObj->CapturePrm),
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    /* skip alternate frame to make it 15fps output for sensor and 30fps for HDMI */
    pObj->ucObj.CapturePrm.vipInst[0].outParams[0].frameSkipMask
        = 0x2AAAAAAA;

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );


    ChainsCommon_SetDisplayPrms(&pUcObj->Display_algPrm,
                                               &pUcObj->Display_GrpxPrm,
                                               pObj->chainsCfg->displayType,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                                );

    chains_vipSingleCameraObjectDetect_SetFeaturePlaneComputeAlgPrms(
                    pObj,
                    &pUcObj->Alg_FeaturePlaneComputationPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT
                );

    chains_vipSingleCameraObjectDetect_SetObjectDetectPrm(
                    pObj,
                    &pUcObj->Alg_ObjectDetectionPrm
                );

    chains_vipSingleCameraObjectDetect_SetSyncPrm(
                    &pUcObj->Sync_algPrm
                );

/*
    chains_vipSingleCameraObjectDetect_SetVpePrm(
                    &pUcObj->VPE_algPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT,
                    pObj->captureOutWidth,
                    pObj->captureOutHeight,
                    SYSTEM_DF_YUV420SP_UV
                );

    chains_vipSingleCameraObjectDetect_SetNullPrm(
                    &pUcObj->NullPrm
                );
*/
    chains_vipSingleCameraObjectDetect_SetObjectDrawPrms(
                    pObj,
                    &pUcObj->Alg_ObjectDrawPrm,
                    FEATUREPLANE_ALG_WIDTH,
                    FEATUREPLANE_ALG_HEIGHT
                );

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
 * \param   pObj  [IN] Chains_VipSingleCameraObjectDetectAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraObjectDetect_StartApp(Chains_VipSingleCameraObjectDetectAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCameraObjectDetect_SetFrameRatePrms(pObj);

    chains_vipSingleCameraObjectDetect_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraObjectDetectAppObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraObjectDetect_StopAndDeleteApp(Chains_VipSingleCameraObjectDetectAppObj *pObj)
{
    chains_vipSingleCameraObjectDetect_Stop(&pObj->ucObj);
    chains_vipSingleCameraObjectDetect_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();
    ChainsCommon_StopCaptureDevice(pObj->chainsCfg->captureSrc);
    ChainsCommon_StopDisplayDevice(pObj->chainsCfg->displayType);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

Void chains_vipSingleCameraObjectDetect_SelectOption(Chains_VipSingleCameraObjectDetectAppObj *pObj)
{
    Bool done = FALSE;
    char ch;

    while(!done)
    {
        Vps_printf(usecase_menu);

        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                pObj->enablePD = TRUE;
                pObj->enableTSR = FALSE;
                done = TRUE;
                break;
            case '2':
                pObj->enablePD = FALSE;
                pObj->enableTSR = TRUE;
                done = TRUE;
                break;
            case '3':
                pObj->enablePD = TRUE;
                pObj->enableTSR = TRUE;
                done = TRUE;
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
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
Void Chains_vipSingleCameraObjectDetect(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraObjectDetectAppObj chainsObj;

    chainsObj.enablePD = TRUE;
    chainsObj.enableTSR = TRUE;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCameraObjectDetect_SelectOption(&chainsObj);

    chains_vipSingleCameraObjectDetect_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCameraObjectDetect_StartApp(&chainsObj);

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
                chains_vipSingleCameraObjectDetect_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCameraObjectDetect_StopAndDeleteApp(&chainsObj);

}

