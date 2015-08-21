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
#include "chains_networkRxCameraAnalytics_tda2xx_priv.h"
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

    chains_networkRxCameraAnalytics_tda2xxObj ucObj;

    Chains_Ctrl *chainsCfg;

} Chains_NetworkRxCameraAnalyticsTda2xxAppObj;





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
Void chains_networkRxCameraAnalytics_tda2xx_SetAppPrms(chains_networkRxCameraAnalytics_tda2xxObj *pUcObj, Void *appObj)
{
    Chains_NetworkRxCameraAnalyticsTda2xxAppObj *pObj
        = (Chains_NetworkRxCameraAnalyticsTda2xxAppObj*)appObj;

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
 * \param   pObj  [IN] Chains_NetworkRxCameraAnalyticsTda2xxAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_networkRxCameraAnalytics_tda2xx_StartApp(Chains_NetworkRxCameraAnalyticsTda2xxAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_Analytics_Start(FALSE);

    chains_networkRxCameraAnalytics_tda2xx_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_NetworkRxCameraAnalyticsTda2xxAppObj
 *
 *******************************************************************************
*/
Void chains_networkRxCameraAnalytics_tda2xx_StopAndDeleteApp(Chains_NetworkRxCameraAnalyticsTda2xxAppObj *pObj)
{
    chains_networkRxCameraAnalytics_tda2xx_Stop(&pObj->ucObj);
    chains_networkRxCameraAnalytics_tda2xx_Delete(&pObj->ucObj);

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
Void Chains_networkRxCameraAnalyticsTda2xx(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_NetworkRxCameraAnalyticsTda2xxAppObj chainsObj;

    chainsCfg->captureSrc = CHAINS_CAPTURE_SRC_HDMI_1080P;

    chainsObj.chainsCfg = chainsCfg;

    chains_networkRxCameraAnalytics_tda2xx_Create(&chainsObj.ucObj, &chainsObj);

    chains_networkRxCameraAnalytics_tda2xx_StartApp(&chainsObj);

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
                chains_networkRxCameraAnalytics_tda2xx_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_networkRxCameraAnalytics_tda2xx_StopAndDeleteApp(&chainsObj);

}

