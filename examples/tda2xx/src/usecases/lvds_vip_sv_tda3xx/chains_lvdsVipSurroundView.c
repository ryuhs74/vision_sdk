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
#include "chains_lvdsVipSurroundView_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

#define SV_SOF_WIDTH              (1024)
#define SV_SOF_HEIGHT             (768)

#define SV_NUM_VIEWS              (4)

#define SV_CARBOX_WIDTH           (160)
#define SV_CARBOX_HEIGHT          (320)

#define SV_HOR_WIDTH              (1000)
#define SV_HOR_HEIGHT             (760)

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

    chains_lvdsVipSurroundViewObj ucObj;
    UInt32 captureOutWidth;
    UInt32 captureOutHeight;
    UInt32 displayWidth;
    UInt32 displayHeight;

    Chains_Ctrl *chainsCfg;

} Chains_LvdsVipSurroundViewAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function. It is advisable to have
 *          chains_lvdsVipMultiCam_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Number of channels to be synced and sync delta and threshold.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_lvdsVipSurroundView_SetSyncPrm(
                    SyncLink_CreateParams *pPrm1
                    )
{
/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define SYNC_DELTA_IN_MSEC              (16)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define SYNC_DROP_THRESHOLD_IN_MSEC     (33)

    UInt16 chId;

    pPrm1->chParams.numCh = 4;
    pPrm1->chParams.numActiveCh = 4;
    for(chId = 0; chId < pPrm1->chParams.numCh; chId++)
    {
        pPrm1->chParams.channelSyncList[chId] = TRUE;
    }
    pPrm1->chParams.syncDelta = SYNC_DELTA_IN_MSEC;
    pPrm1->chParams.syncThreshold = SYNC_DROP_THRESHOLD_IN_MSEC;
}

Void chains_lvdsVipSurroundView_SetVPEParams(
                    VpeLink_CreateParams *pPrm,
                    UInt32 numChannels,
                    UInt32 outWidth,
                    UInt32 outHeight
                    )
{
    UInt16 chId;
    pPrm->enableOut[0] = TRUE;

    for(chId = 0; chId < numChannels; chId++)
    {
        pPrm->chParams[chId].outParams[0].width
            = SystemUtils_floor(outWidth, 16);

        pPrm->chParams[chId].outParams[0].height
            = outHeight;

        pPrm->chParams[chId].outParams[0].dataFormat
            = SYSTEM_DF_YUV420SP_UV;
        pPrm->chParams[chId].outParams[0].numBufsPerCh = 4;
    }
}

/**
 *******************************************************************************
 * \brief   Set link Parameters
 *
 *          It is called in Create function of the auto generated use-case file.
 *
 * \param pUcObj    [IN] Auto-generated usecase object
 * \param appObj    [IN] Application specific object
 *
 *******************************************************************************
*/
Void chains_lvdsVipSurroundView_SetAppPrms(chains_lvdsVipSurroundViewObj *pUcObj, Void *appObj)
{
    Chains_LvdsVipSurroundViewAppObj *pObj
        = (Chains_LvdsVipSurroundViewAppObj*)appObj;

    Int16 carBoxWidth;
    Int16 carBoxHeight;

    ChainsCommon_SurroundView_SetParams(
        &pUcObj->CapturePrm,
        NULL,
        NULL,
        NULL, //&pUcObj->SelectPrm,
        NULL, //&pUcObj->VPE_sv_orgPrm,
        NULL, //&pUcObj->VPE_algPrm,
        &pUcObj->Sync_svPrm,
        NULL, //&pUcObj->Sync_sv_orgPrm,
        NULL, //&pUcObj->Sync_algPrm,
        &pUcObj->Alg_SynthesisPrm,
        &pUcObj->Alg_GeoAlignPrm,
        &pUcObj->Alg_PhotoAlignPrm,
        NULL,
        NULL, //&pUcObj->Alg_DenseOptFlowPrm,
        NULL, //&pUcObj->Alg_VectorToImagePrm,
        NULL, //&pUcObj->Alg_DmaSwMs_sv_orgPrm,
        NULL, //&pUcObj->Alg_DmaSwMs_algPrm,
        &pUcObj->GrpxSrcPrm,
        &pUcObj->Display_svPrm,
        NULL, //&pUcObj->Display_sv_orgPrm,
        NULL, //&pUcObj->Display_algPrm,
        &pUcObj->Display_GrpxPrm,
        pObj->chainsCfg->displayType,
        pObj->chainsCfg->numLvdsCh,
        pObj->chainsCfg->svOutputMode,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        pObj->chainsCfg->enableCarOverlayInAlg
        );


    pUcObj->GrpxSrcPrm.tda3xxSvFsRotLayoutEnable = TRUE;

    if(pUcObj->GrpxSrcPrm.tda3xxSvFsRotLayoutEnable == TRUE)
    {
    	//swap height and width
    	carBoxWidth = SV_CARBOX_HEIGHT;
    	carBoxHeight = SV_CARBOX_WIDTH;

    }
    else
    {
    	carBoxWidth = SV_CARBOX_WIDTH;
    	carBoxHeight = SV_CARBOX_HEIGHT;
    }

    pUcObj->Display_svPrm.rtParams.tarWidth   = SV_HOR_WIDTH;
    pUcObj->Display_svPrm.rtParams.tarHeight  = SV_HOR_HEIGHT;
    pUcObj->Display_svPrm.rtParams.posX       = ((SV_SOF_WIDTH - SV_HOR_WIDTH)/2);
    pUcObj->Display_svPrm.rtParams.posY       = ((SV_SOF_HEIGHT - SV_HOR_HEIGHT)/2);
    pUcObj->Display_svPrm.displayId           = DISPLAY_LINK_INST_DSS_VID1;

    ChainsCommon_SurroundView_SetSynthParams(&pUcObj->Alg_SynthesisPrm,
                                            CAPTURE_SENSOR_WIDTH,
                                            CAPTURE_SENSOR_HEIGHT,
                                            SV_HOR_WIDTH,
                                            SV_HOR_HEIGHT,
                                            SV_NUM_VIEWS,
                                            carBoxWidth,
                                            carBoxHeight,
                                            pObj->chainsCfg->svOutputMode,
                                            pObj->chainsCfg->enableCarOverlayInAlg);

    ChainsCommon_SurroundView_SetGAlignParams(&pUcObj->Alg_GeoAlignPrm,
                                            CAPTURE_SENSOR_WIDTH,
                                            CAPTURE_SENSOR_HEIGHT,
                                            SV_HOR_WIDTH,
                                            SV_HOR_HEIGHT,
                                            SV_NUM_VIEWS,
                                            carBoxWidth,
                                            carBoxHeight,
                                            pObj->chainsCfg->svOutputMode);

    ChainsCommon_SurroundView_SetPAlignParams(&pUcObj->Alg_PhotoAlignPrm,
                                            CAPTURE_SENSOR_WIDTH,
                                            CAPTURE_SENSOR_HEIGHT,
                                            SV_HOR_WIDTH,
                                            SV_HOR_HEIGHT,
                                            SV_NUM_VIEWS,
                                            carBoxWidth,
                                            carBoxHeight,
                                            pObj->chainsCfg->svOutputMode);

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
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
 * \param   pObj  [IN] Chains_LvdsVipSurroundViewObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_lvdsVipSurroundView_StartApp(Chains_LvdsVipSurroundViewAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_lvdsVipSurroundView_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_LvdsVipSurroundViewObj
 *
 *******************************************************************************
*/
Void chains_lvdsVipSurroundView_StopAndDeleteApp(Chains_LvdsVipSurroundViewAppObj *pObj)
{
    chains_lvdsVipSurroundView_Stop(&pObj->ucObj);
    chains_lvdsVipSurroundView_Delete(&pObj->ucObj);

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
Void Chains_lvdsVipSurroundView(Chains_Ctrl *chainsCfg)
{
    char ch, chPrev;
    UInt32 done = FALSE;
    Bool startWithCalibration;
    Chains_LvdsVipSurroundViewAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    do
    {
        done = FALSE;
        /* Set startWithCalibration = TRUE to start the demo with calibration.
           Else it will use the previously calibrated LUTs */
        startWithCalibration = TRUE;
        ChainsCommon_SurroundView_CalibInit(startWithCalibration);

        chains_lvdsVipSurroundView_Create(&chainsObj.ucObj, &chainsObj);
        chains_lvdsVipSurroundView_StartApp(&chainsObj);

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
                    chains_lvdsVipSurroundView_printStatistics(&chainsObj.ucObj);
                    break;
                default:
                    Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                    break;
            }
        }

        chains_lvdsVipSurroundView_StopAndDeleteApp(&chainsObj);
        ChainsCommon_SurroundView_CalibDeInit();
    } while(chPrev!='3');
}

