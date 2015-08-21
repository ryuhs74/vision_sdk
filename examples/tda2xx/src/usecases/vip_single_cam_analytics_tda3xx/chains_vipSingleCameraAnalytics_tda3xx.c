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
#include "chains_vipSingleCameraAnalytics_tda3xx_priv.h"
#include <examples/tda2xx/include/chains_common.h>

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

    chains_vipSingleCameraAnalytics_tda3xxObj ucObj;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraAnalyticsTda3xxAppObj;

Chains_VipSingleCameraAnalyticsTda3xxAppObj gChains_vipSingleCameraAnalyticsTda3xxAppObj;

/**
 *******************************************************************************
 *
 * \brief   Handler for System Events.
 *
 * \param   cmd[IN]     System Command
 * \param   pPrm[IN]    Pointer to the Data
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraAnalyticsTda3xx_EventHandler(UInt32 cmd, Void *pPrm)
{
    chains_vipSingleCameraAnalytics_tda3xxObj *pUcObj =
        &gChains_vipSingleCameraAnalyticsTda3xxAppObj.ucObj;


    if (UTILS_TEMP_CMD_EVENT_HOT == cmd)
    {
        ChainsCommon_tempHotEventHandler(pUcObj->CaptureLinkID, pPrm);
    }
    else if (UTILS_TEMP_CMD_EVENT_COLD == cmd)
    {
        ChainsCommon_tempColdEventHandler(pUcObj->CaptureLinkID, pPrm);
    }
    else
    {
        ; /* Nothing to do */
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
Void chains_vipSingleCameraAnalytics_tda3xx_SetAppPrms(chains_vipSingleCameraAnalytics_tda3xxObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraAnalyticsTda3xxAppObj *pObj
        = (Chains_VipSingleCameraAnalyticsTda3xxAppObj*)appObj;

    ChainsCommon_Analytics_SetPrms(
        &pUcObj->CapturePrm,
        NULL,
        NULL,
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
        &pUcObj->Alg_DmaSwMsPrm,
        &pUcObj->Display_algPrm,
        &pUcObj->GrpxSrcPrm,
        &pUcObj->Display_GrpxPrm,
        pObj->chainsCfg->captureSrc,
        pObj->chainsCfg->displayType,
        pUcObj->Alg_FeaturePlaneComputationLinkID,
        pUcObj->Alg_ObjectDetectionLinkID,
        pUcObj->Alg_ObjectDrawLinkID,
        CHAINS_COMMON_FC_ANALYTICS_LAYOUT_HORZ_STRIP
        );

    pUcObj->GrpxSrcPrm.statsDisplayEnable = TRUE;
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
 * \param   pObj  [IN] Chains_VipSingleCameraAnalyticsTda3xxAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraAnalytics_tda3xx_StartApp(Chains_VipSingleCameraAnalyticsTda3xxAppObj *pObj)
{
    /* Initialize the system to take up temperature events */
    Utils_tempConfigInit();
    SystemLink_registerHandler(Chains_vipSingleCameraAnalyticsTda3xx_EventHandler);
    Chains_memPrintHeapStatus();

    ChainsCommon_Analytics_Start(TRUE);

    chains_vipSingleCameraAnalytics_tda3xx_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraAnalyticsTda3xxAppObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraAnalytics_tda3xx_StopAndDeleteApp(Chains_VipSingleCameraAnalyticsTda3xxAppObj *pObj)
{
    chains_vipSingleCameraAnalytics_tda3xx_Stop(&pObj->ucObj);
    chains_vipSingleCameraAnalytics_tda3xx_Delete(&pObj->ucObj);

    ChainsCommon_Analytics_Stop(TRUE);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);

    /* De-Initialize the Temperature configuration */
    Utils_tempConfigDeInit();
    SystemLink_unregisterHandler(Chains_vipSingleCameraAnalyticsTda3xx_EventHandler);
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
Void Chains_vipSingleCameraAnalyticsTda3xx(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;

    gChains_vipSingleCameraAnalyticsTda3xxAppObj.chainsCfg = chainsCfg;

    chains_vipSingleCameraAnalytics_tda3xx_Create(
            &gChains_vipSingleCameraAnalyticsTda3xxAppObj.ucObj,
            &gChains_vipSingleCameraAnalyticsTda3xxAppObj);

    chains_vipSingleCameraAnalytics_tda3xx_StartApp(
            &gChains_vipSingleCameraAnalyticsTda3xxAppObj);

    while(!done)
    {
        ch = ChainsCommon_menuRunTime_SingleCamAnalytics();

        switch(ch)
        {
            case '0':
                done = TRUE;
                break;
            case 'p':
            case 'P':
                ChainsCommon_PrintStatistics();
                chains_vipSingleCameraAnalytics_tda3xx_printStatistics(
                        &gChains_vipSingleCameraAnalyticsTda3xxAppObj.ucObj);
                break;
            case 't':
            case 'T':
                ChainsCommon_thermalConfig();
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCameraAnalytics_tda3xx_StopAndDeleteApp(
            &gChains_vipSingleCameraAnalyticsTda3xxAppObj);
}

