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
#include "chains_vipStereoCalibration_priv.h"

#include <examples/tda2xx/include/chains_common.h>

#define CAPTURE_SENSOR_WIDTH                                    1280
#define CAPTURE_SENSOR_HEIGHT                                   720

#define REMAP_WIDTH                                             896
#define REMAP_HEIGHT                                            384

#define STEREO_OUTPUT_WIDTH                                     640
#define STEREO_OUTPUT_HEIGHT                                    360

#define LIVE_CAMERA_DISPLAY_WIDTH                               640
#define LIVE_CAMERA_DISPLAY_HEIGHT                              360


/**
 *******************************************************************************
 *
 *  \brief  StereoCameraDisplayObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
 */
typedef struct {

    chains_vipStereoCalibrationObj ucObj;

    Chains_Ctrl *chainsCfg;

} chains_vipStereoCalibrationAppObj;

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
Void chains_vipStereoCalibration_SetAppPrms(
        chains_vipStereoCalibrationObj *pUcObj, Void *appObj)
{
    UInt32 displayWidth = 0, displayHeight = 0;
    chains_vipStereoCalibrationAppObj *pObj
    = (chains_vipStereoCalibrationAppObj*)appObj;

    if(pObj->chainsCfg->displayType==CHAINS_DISPLAY_TYPE_HDMI_720P)
    {
        displayWidth     = 1280;
        displayHeight    = 720;
    }
    else
        if(pObj->chainsCfg->displayType==CHAINS_DISPLAY_TYPE_HDMI_1080P)
        {
            displayWidth     = 1920;
            displayHeight    = 1080;
        }

    ChainsCommon_Stereo_SetPrms(
            &pUcObj->CapturePrm,
            &pUcObj->VPE_softispPrm,
            &pUcObj->VPE_orgdispPrm,
            NULL,
            &pUcObj->Alg_SoftIspPrm,
            &pUcObj->Alg_RemapMergePrm,
            NULL,
            NULL,
            NULL,
            &pUcObj->Sync_orgPrm,
            NULL,
            &pUcObj->Alg_DmaSwMsPrm,
            &pUcObj->GrpxSrcPrm,
            &pUcObj->Display_orgPrm,
            NULL,
            &pUcObj->Display_GrpxPrm,
            pObj->chainsCfg->captureSrc,
            pObj->chainsCfg->displayType,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            0,
            0,
            STEREO_OUTPUT_WIDTH,
            STEREO_OUTPUT_HEIGHT,
            0,
            0,
            0,
            0,
            0,
            270,
            LIVE_CAMERA_DISPLAY_WIDTH,
            LIVE_CAMERA_DISPLAY_HEIGHT,
            0
    );

    pUcObj->Alg_RemapMergePrm.allocBufferForRawDump = TRUE;

    ChainsCommon_StartDisplayCtrl(
            pObj->chainsCfg->displayType,
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
 * \param   pObj  [IN] chains_vipStereoCalibrationAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Void chains_vipStereoCalibration_StartApp(chains_vipStereoCalibrationAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
            pObj->chainsCfg->captureSrc,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT
    );

    chains_vipStereoCalibration_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   chains_vipStereoCalibrationAppObj
 *
 *******************************************************************************
 */
Void chains_vipStereoCalibration_StopAndDeleteApp(chains_vipStereoCalibrationAppObj *pObj)
{
    chains_vipStereoCalibration_Stop(&pObj->ucObj);
    chains_vipStereoCalibration_Delete(&pObj->ucObj);
    ChainsCommon_Stereo_Delete(&pObj->ucObj.Alg_RemapMergePrm);

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
Void chains_vipStereoCalibration(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    chains_vipStereoCalibrationAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipStereoCalibration_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipStereoCalibration_StartApp(&chainsObj);

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
            chains_vipStereoCalibration_printStatistics(&chainsObj.ucObj);
            break;
        default:
            Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
            break;
        }
    }

    chains_vipStereoCalibration_StopAndDeleteApp(&chainsObj);

}

