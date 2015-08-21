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
#include "chains_lvdsVipSurroundViewAnalyticsUltrasound_priv.h"
#include <examples/tda2xx/include/chains_common.h>

/**
 *******************************************************************************
 *
 *  \brief  Use-case object
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_lvdsVipSurroundViewAnalyticsUltrasoundObj ucObj;

    Chains_Ctrl *chainsCfg;

} Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj;

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
Void chains_lvdsVipSurroundViewAnalyticsUltrasound_SetAppPrms(
            chains_lvdsVipSurroundViewAnalyticsUltrasoundObj *pUcObj, Void *appObj)
{
    Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj *pObj
        = (Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj*)appObj;

    ChainsCommon_SurroundView_SetParams(
        &pUcObj->CapturePrm,
        NULL,
        NULL,
        &pUcObj->SelectPrm,
        &pUcObj->VPE_sv_orgPrm,
        NULL,
        &pUcObj->Sync_svPrm,
        &pUcObj->Sync_sv_orgPrm,
        &pUcObj->Sync_algDisplayPrm,
        &pUcObj->Alg_SynthesisPrm,
        &pUcObj->Alg_GeoAlignPrm,
        &pUcObj->Alg_PhotoAlignPrm,
        NULL,
        NULL,
        NULL,
        &pUcObj->Alg_DmaSwMs_sv_orgPrm,
        &pUcObj->Alg_DmaSwMs_algPrm,
        &pUcObj->GrpxSrcPrm,
        &pUcObj->Display_svPrm,
        &pUcObj->Display_sv_orgPrm,
        &pUcObj->Display_algPrm,
        &pUcObj->Display_GrpxPrm,
        pObj->chainsCfg->displayType,
        pObj->chainsCfg->numLvdsCh,
        pObj->chainsCfg->svOutputMode,
        &pUcObj->VPE_algPdPrm,
        &pUcObj->Alg_FeaturePlaneComputationPrm,
        &pUcObj->Alg_ObjectDetectionPrm,
        &pUcObj->Sync_algPdPrm,
        NULL, 
        NULL, 
        NULL, 
        NULL,
        pObj->chainsCfg->enableCarOverlayInAlg
        );

    //add setting parameters for ultrasound/pixelPerCm
    pUcObj->Alg_GeoAlignPrm.enablePixelsPerCm = 1;
    pUcObj->GrpxSrcPrm.ultrasonicParams.enable = TRUE;
    pUcObj->GrpxSrcPrm.ultrasonicParams.windowStartX = 25+320+10;
    pUcObj->GrpxSrcPrm.ultrasonicParams.windowStartY = 0;
    pUcObj->GrpxSrcPrm.ultrasonicParams.windowWidth = 880;
    pUcObj->GrpxSrcPrm.ultrasonicParams.windowHeight = 1080;
    /* ultrasonic results format is BGRA4444, match GrpxSrc format to this
     * to allow overlay of ultrasonic results on top of surround view output
     */
    pUcObj->GrpxSrcPrm.grpxBufInfo.dataFormat = SYSTEM_DF_BGRA16_4444;
    pUcObj->Alg_UltrasonicFusionPrm.numOutputTables = 1;
    pUcObj->Alg_UltrasonicFusionPrm.numViews = 4;
    pUcObj->Alg_UltrasonicFusionPrm.numUltrasonic = 6;

    //Graphics Params
    pUcObj->GrpxSrcPrm.surroundViewEdgeDetectLayoutEnable = TRUE;
    pUcObj->GrpxSrcPrm.surroundViewDOFLayoutEnable = FALSE;
    pUcObj->GrpxSrcPrm.surroundViewPdTsrLayoutEnable = TRUE;
    pUcObj->GrpxSrcPrm.surroundViewLdLayoutEnable = FALSE;

    //set parameters for PD/TSR Object Draw
    pUcObj->Alg_ObjectDraw_PdPrm.drawOption = ALGORITHM_LINK_OBJECT_DETECT_DRAW_PD;
    pUcObj->Alg_ObjectDraw_PdPrm.imgFrameHeight = 360;
    pUcObj->Alg_ObjectDraw_PdPrm.imgFrameWidth = 640;
    pUcObj->Alg_ObjectDraw_PdPrm.numOutBuffers = 4;
    pUcObj->Alg_ObjectDraw_TsrPrm.drawOption = ALGORITHM_LINK_OBJECT_DETECT_DRAW_TSR;
    pUcObj->Alg_ObjectDraw_TsrPrm.imgFrameHeight = 360;
    pUcObj->Alg_ObjectDraw_TsrPrm.imgFrameWidth = 640;
    pUcObj->Alg_ObjectDraw_TsrPrm.numOutBuffers = 4;

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
 * \param   pObj  [IN] Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_lvdsVipSurroundViewAnalyticsUltrasound_StartApp(
                           Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_lvdsVipSurroundViewAnalyticsUltrasound_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj
 *
 *******************************************************************************
*/
Void chains_lvdsVipSurroundViewAnalyticsUltrasound_StopAndDeleteApp(
                           Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj *pObj)
{
    chains_lvdsVipSurroundViewAnalyticsUltrasound_Stop(&pObj->ucObj);
    chains_lvdsVipSurroundViewAnalyticsUltrasound_Delete(&pObj->ucObj);

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
Void Chains_lvdsVipSurroundViewAnalyticsUltrasound(Chains_Ctrl *chainsCfg)
{
    char ch, chPrev;
    UInt32 done = FALSE;
    Bool startWithCalibration;
    Chains_lvdsVipSurroundViewAnalyticsUltrasoundAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    do
    {
        done = FALSE;
        /* Set startWithCalibration = TRUE to start the demo with calibration.
           Else it will use the previously calibrated LUTs */
        startWithCalibration = TRUE;
        ChainsCommon_SurroundView_CalibInit(startWithCalibration);

        chains_lvdsVipSurroundViewAnalyticsUltrasound_Create(&chainsObj.ucObj, &chainsObj);
        chains_lvdsVipSurroundViewAnalyticsUltrasound_StartApp(&chainsObj);

        while(!done)
        {
            ch = Chains_menuRunTime();

            switch(ch)
            {
                case '0':
                    chPrev = ChainsCommon_SurroundView_MenuCalibration();
                    done = TRUE;
                    break;
                case 'p':
                case 'P':
                    ChainsCommon_PrintStatistics();
                    chains_lvdsVipSurroundViewAnalyticsUltrasound_printStatistics(&chainsObj.ucObj);
                    break;
                default:
                    Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                    break;
            }
        }

        chains_lvdsVipSurroundViewAnalyticsUltrasound_StopAndDeleteApp(&chainsObj);
        ChainsCommon_SurroundView_CalibDeInit();
    } while(chPrev!='3');
}

