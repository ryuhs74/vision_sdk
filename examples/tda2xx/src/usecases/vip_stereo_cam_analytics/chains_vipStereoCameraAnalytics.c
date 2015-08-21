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
#include "chains_vipStereoCameraAnalytics_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH                                    1280
#define CAPTURE_SENSOR_HEIGHT                                   720

#define REMAP_WIDTH                                             896
#define REMAP_HEIGHT                                            384

#define STEREO_OUTPUT_WIDTH                                     640
#define STEREO_OUTPUT_HEIGHT                                    360

/**
 *******************************************************************************
 *
 *  \brief  This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipStereoCameraAnalyticsObj ucObj;

    Chains_Ctrl *chainsCfg;

} Chains_VipStereoCameraAnalyticsAppObj;





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
Void chains_vipStereoCameraAnalytics_SetAppPrms(chains_vipStereoCameraAnalyticsObj *pUcObj, Void *appObj)
{
    Chains_VipStereoCameraAnalyticsAppObj *pObj
        = (Chains_VipStereoCameraAnalyticsAppObj*)appObj;

    ChainsCommon_Stereo_SetPrms(
        &pUcObj->CapturePrm,
        &pUcObj->VPE_softispPrm,
        &pUcObj->VPE_org_stereoPrm,
        &pUcObj->VPE_disparityPrm,
        &pUcObj->Alg_SoftIspPrm,
        &pUcObj->Alg_RemapMergePrm,
        &pUcObj->Alg_CensusPrm,
        &pUcObj->Alg_DisparityHamDistPrm,
        &pUcObj->Alg_StereoPostProcessPrm,
        &pUcObj->Sync_org_stereoPrm,
        &pUcObj->Sync_disparityPrm,
        &pUcObj->Alg_DmaSwMs_stereoPrm,
        NULL,
        &pUcObj->Display_org_stereoPrm,
        &pUcObj->Display_disparityPrm,
        NULL,
        pObj->chainsCfg->captureSrc,
        pObj->chainsCfg->displayType,
        CAPTURE_SENSOR_WIDTH,
        CAPTURE_SENSOR_HEIGHT,
        REMAP_WIDTH,
        REMAP_HEIGHT,
        STEREO_OUTPUT_WIDTH,
        STEREO_OUTPUT_HEIGHT,
        160+800,
        450,
        800,
        450,
        160+800+60,
        900,
        320,
        180,
        40
        );

    pUcObj->Alg_RemapMergePrm.allocBufferForRawDump = FALSE;

    ChainsCommon_Analytics_SetPrms(
        NULL,
        &pUcObj->NullSourcePrm,
        &pUcObj->DecodePrm,
        &pUcObj->Alg_LaneDetectPrm,
        &pUcObj->Alg_LaneDetectDrawPrm,
        &pUcObj->Sync_ldPrm,
        &pUcObj->Alg_FeaturePlaneComputationPrm,
        &pUcObj->Alg_ObjectDetectionPrm,
        &pUcObj->Alg_ObjectDrawPrm,
        &pUcObj->Sync_pd_tsrPrm,
        &pUcObj->Alg_SparseOpticalFlowPrm,
        &pUcObj->Alg_SparseOpticalFlowDrawPrm,
        &pUcObj->Sync_sofPrm,
        &pUcObj->Sync_algPrm,
        &pUcObj->Alg_DmaSwMs_algPrm,
        &pUcObj->Display_algPrm,
        &pUcObj->GrpxSrcPrm,
        &pUcObj->Display_GrpxPrm,
        CHAINS_CAPTURE_SRC_HDMI_1080P,
        pObj->chainsCfg->displayType,
        pUcObj->Alg_FeaturePlaneComputationLinkID,
        pUcObj->Alg_ObjectDetectionLinkID,
        pUcObj->Alg_ObjectDrawLinkID,
        CHAINS_COMMON_FC_ANALYTICS_LAYOUT_HORZ_AND_VERT
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
 * \param   pObj  [IN] Chains_VipStereoCameraAnalyticsAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipStereoCameraAnalytics_StartApp(Chains_VipStereoCameraAnalyticsAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_Analytics_Start(FALSE);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        CAPTURE_SENSOR_WIDTH,
        CAPTURE_SENSOR_HEIGHT
        );

    chains_vipStereoCameraAnalytics_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipStereoCameraAnalyticsAppObj
 *
 *******************************************************************************
*/
Void chains_vipStereoCameraAnalytics_StopAndDeleteApp(Chains_VipStereoCameraAnalyticsAppObj *pObj)
{
    chains_vipStereoCameraAnalytics_Stop(&pObj->ucObj);
    chains_vipStereoCameraAnalytics_Delete(&pObj->ucObj);
    ChainsCommon_Stereo_Delete(&pObj->ucObj.Alg_RemapMergePrm);

    ChainsCommon_StopCaptureDevice(pObj->chainsCfg->captureSrc);
    ChainsCommon_Analytics_Stop(FALSE);

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
Void Chains_vipStereoCameraAnalytics(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipStereoCameraAnalyticsAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipStereoCameraAnalytics_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipStereoCameraAnalytics_StartApp(&chainsObj);

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
                chains_vipStereoCameraAnalytics_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipStereoCameraAnalytics_StopAndDeleteApp(&chainsObj);
}

