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
#include "chains_lvdsVip3dSurroundView_priv.h"
#include <linux/examples/common/chains_common.h>
#include <infoadas_common.h>
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
enum synctype {
    LOOSE_SYNC,
    TIGHT_SYNC
};

typedef struct {
    chains_lvdsVip3dSurroundViewObj ucObj;

    UInt32 num_captures;
    UInt32 num_buffers_per_capture;
    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];
    UInt32 capture_width;
    UInt32 capture_height;

    UInt32 svInWidth;
    UInt32 svInHeight;
    UInt32 svOutWidth;
    UInt32 svOutHeight;
    UInt32 svCarImageWidth;
    UInt32 svCarImageHeight;
    UInt32 svNumViews;
    UInt32 svOutputMode;
    Chain_Common_SRV_CalibParams gaCalibPrm;
    UInt32 enableCarOverlayInAlg;
    /**< Set to 1, if DSP need to create the car image, apply only for 2D SRV */
} Chains_LvdsVip3dSurroundViewAppObj;

Chains_LvdsVip3dSurroundViewAppObj gLvdsVip3dSurroundViewObj;

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
Void chains_lvdsVip3dSurroundView_SetAppPrms(
            chains_lvdsVip3dSurroundViewObj *pUcObj, Void *appObj)
{
    UInt32 i;
    CaptureLink_CreateParams  *pVipCapture;
    Chains_LvdsVip3dSurroundViewAppObj *pObj;

    pObj = (Chains_LvdsVip3dSurroundViewAppObj*)appObj;
    pVipCapture = &pUcObj->CapturePrm;

    ChainsCommon_MultiCam_SetCapturePrms(&pUcObj->CapturePrm, 
                             pObj->num_captures);

    for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++) {
        pVipCapture->vipInst[i].numBufs = pObj->num_buffers_per_capture;
    }

    ChainsCommon_SurroundView_SetSyncPrm(&pUcObj->SyncPrm, pObj->num_captures,
                                              TIGHT_SYNC);

    ChainsCommon_SurroundView_SetSynthParams(&pUcObj->Alg_SynthesisPrm,
                                              pObj->svInWidth, pObj->svInHeight,
                                              pObj->svOutWidth, pObj->svOutHeight,
                                              pObj->svNumViews,
                                              pObj->svCarImageWidth,
                                              pObj->svCarImageHeight,
                                              pObj->svOutputMode,
                                              pObj->enableCarOverlayInAlg);

    ChainsCommon_SurroundView_SetGAlignParams(&pUcObj->Alg_GeoAlignPrm,
                                              pObj->svInWidth, pObj->svInHeight,
                                              pObj->svOutWidth, pObj->svOutHeight,
                                              pObj->svNumViews,
                                              pObj->svCarImageWidth,
                                              pObj->svCarImageHeight,
                                              pObj->svOutputMode,
                                              &pObj->gaCalibPrm);

    ChainsCommon_SurroundView_SetPAlignParams(&pUcObj->Alg_PhotoAlignPrm,
                                              pObj->svInWidth, pObj->svInHeight,
                                              pObj->svOutWidth, pObj->svOutHeight,
                                              pObj->svNumViews,
                                              pObj->svCarImageWidth,
                                              pObj->svCarImageHeight,
                                              pObj->svOutputMode);

}

Void chains_lvdsVip3dSurroundView_InitApp(struct infoadas_chain_params *params)
{
    Bool startWithCalibration;
    Chains_LvdsVip3dSurroundViewAppObj *pObj
        = (Chains_LvdsVip3dSurroundViewAppObj*)&gLvdsVip3dSurroundViewObj;
    Chain_Common_SRV_CalibParams * gaCalibPrm = &gLvdsVip3dSurroundViewObj.gaCalibPrm;

    /* Capture related initialization */
    pObj->num_captures = 4;
    pObj->num_buffers_per_capture = 6;
    pObj->capture_width = LVDS_CAPTURE_WIDTH;
    pObj->capture_height = LVDS_CAPTURE_HEIGHT;

    pObj->portId[0] = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
    pObj->portId[1] = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
    pObj->portId[2] = SYSTEM_CAPTURE_INST_VIP2_SLICE1_PORTA;
    pObj->portId[3] = SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
    pObj->portId[4] = SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
    pObj->portId[5] = SYSTEM_CAPTURE_INST_VIP2_SLICE2_PORTB;

    /* SRV Algo related initialization */
    pObj->svInWidth = SV_INPUT_WIDTH;
    pObj->svInHeight = SV_INPUT_HEIGHT;
    pObj->svOutWidth = SV_OUTPUT_WIDTH;
    pObj->svOutHeight = SV_OUTPUT_HEIGHT;
    pObj->svCarImageWidth = SV_CARBOX_WIDTH;
    pObj->svCarImageHeight = SV_CARBOX_HEIGHT;
    pObj->svNumViews = SV_NUM_VIEWS;
    pObj->svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_3D;
    pObj->enableCarOverlayInAlg = 0;

    /* Set startWithCalibration = TRUE to start the demo with calibration.
       Else it will use the previously calibrated LUTs */
    startWithCalibration = TRUE;
    ChainsCommon_SurroundView_InitCalibration(gaCalibPrm, startWithCalibration);

    ChainsCommon_statCollectorReset();
}

Int32 chains_lvdsVip3dSurroundView_CreateApp()
{
    Int32 status;

    status = chains_lvdsVip3dSurroundView_Create(&gLvdsVip3dSurroundViewObj.ucObj, &gLvdsVip3dSurroundViewObj);
    return status;
}

Int32 chains_lvdsVip3dSurroundView_StartApp()
{
    Int32 status;

    ChainsCommon_memPrintHeapStatus();

    status = chains_lvdsVip3dSurroundView_Start(&gLvdsVip3dSurroundViewObj.ucObj);

    ChainsCommon_prfLoadCalcEnable(TRUE, FALSE, FALSE);

    return status;
}

Int32 chains_lvdsVip3dSurroundView_StopApp()
{
    Int32 status;

    status = chains_lvdsVip3dSurroundView_Stop(&gLvdsVip3dSurroundViewObj.ucObj);

    ChainsCommon_prfLoadCalcEnable(FALSE, TRUE, TRUE);

    return status;
}

Int32 chains_lvdsVip3dSurroundView_DeleteApp()
{
    Int32 status;
    Chain_Common_SRV_CalibParams *gaCalibPrm = &gLvdsVip3dSurroundViewObj.gaCalibPrm;

    status = chains_lvdsVip3dSurroundView_Delete(&gLvdsVip3dSurroundViewObj.ucObj);

    ChainsCommon_SurroundView_StopCalibration(gaCalibPrm);

    return status;
}

Void chains_lvdsVip3dSurroundView_CalibApp()
{
    ChainsCommon_SurroundView_MenuCalibration(&gLvdsVip3dSurroundViewObj.gaCalibPrm);
}

Void chains_lvdsVip3dSurroundView_StatApp()
{
    chains_lvdsVip3dSurroundView_printStatistics(&gLvdsVip3dSurroundViewObj.ucObj);
    ChainsCommon_memPrintHeapStatus();
}
