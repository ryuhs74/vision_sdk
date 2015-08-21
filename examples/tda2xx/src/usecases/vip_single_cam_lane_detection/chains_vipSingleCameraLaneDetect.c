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
#include "chains_vipSingleCameraLaneDetect_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

#define LD_NUM_OUT_BUF  (8)

#define LD_ALG_WIDTH    (640)
#define LD_ALG_HEIGHT   (360)

/**
 *******************************************************************************
 *
 *  \brief  Link Object
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipSingleCameraLaneDetectObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraLaneDetectAppObj;


/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipSingleCameraLaneDetect_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *          Scaling parameters are set .
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraLaneDetect_SetVpePrm(
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
 * \brief   Set Sync Link Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraLaneDetect_SetSyncPrm(SyncLink_CreateParams *pPrm)

{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 1;
    pPrm->chParams.syncThreshold = 0xFFFF;
}

/**
 *******************************************************************************
 *
 * \brief   Set Algorithm related parameters
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraLaneDetect_SetLaneDetectPrm(
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

    pAlgPrm->numOutBuffers  = 2;

    pDrawPrm->imgFrameStartX = startX;
    pDrawPrm->imgFrameStartY = startY;
    pDrawPrm->imgFrameWidth  = width;
    pDrawPrm->imgFrameHeight = height;
    pDrawPrm->numOutBuffers  = 3;
    pDrawPrm->enableDrawLines = TRUE;
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
Void chains_vipSingleCameraLaneDetect_SetAppPrms(chains_vipSingleCameraLaneDetectObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraLaneDetectAppObj *pObj
        = (Chains_VipSingleCameraLaneDetectAppObj*)appObj;

    UInt32 startX, startY;

    startX = 0;
    startY = 0;
    pObj->captureOutWidth  = LD_ALG_WIDTH;
    pObj->captureOutHeight = LD_ALG_HEIGHT;

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

/*
    chains_vipSingleCameraLaneDetect_SetVpePrm(
                                            &pUcObj->VPE_algPrm,
                                            LD_ALG_WIDTH,
                                            LD_ALG_HEIGHT,
                                            pObj->captureOutWidth,
                                            pObj->captureOutHeight,
                                            SYSTEM_DF_YUV420SP_UV
                                            );
*/
    chains_vipSingleCameraLaneDetect_SetSyncPrm(
                    &pUcObj->Sync_algPrm
                );

    chains_vipSingleCameraLaneDetect_SetLaneDetectPrm(
        &pUcObj->Alg_LaneDetectPrm,
        &pUcObj->Alg_LaneDetectDrawPrm,
        startX,
        startY,
        LD_ALG_WIDTH,
        LD_ALG_HEIGHT
        );



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
 * \param   pObj  [IN] Chains_VipSingleCameraLaneDetectObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraLaneDetect_StartApp(Chains_VipSingleCameraLaneDetectAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCameraLaneDetect_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraLaneDetectObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraLaneDetect_StopAndDeleteApp(Chains_VipSingleCameraLaneDetectAppObj *pObj)
{
    chains_vipSingleCameraLaneDetect_Stop(&pObj->ucObj);
    chains_vipSingleCameraLaneDetect_Delete(&pObj->ucObj);

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
Void Chains_vipSingleCameraLaneDetect(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraLaneDetectAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCameraLaneDetect_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCameraLaneDetect_StartApp(&chainsObj);

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
                chains_vipSingleCameraLaneDetect_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCameraLaneDetect_StopAndDeleteApp(&chainsObj);
}

