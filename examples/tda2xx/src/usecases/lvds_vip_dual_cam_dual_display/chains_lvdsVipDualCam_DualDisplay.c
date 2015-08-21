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
#include "chains_lvdsVipDualCam_DualDisplay_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH    (1280)
#define CAPTURE_SENSOR_HEIGHT   (720)
#define HDMI_DISPLAY_WIDTH      (1920)
#define HDMI_DISPLAY_HEIGHT     (1080)

#define NUM_CAMERAS             (2)

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

    chains_lvdsVipDualCam_DualDisplayObj    ucObj;

    UInt32                                  displayCtrlLinkId;
    DisplayCtrlLink_ConfigParams            dctrlCfgPrms;
    DisplayCtrlLink_OvlyParams              dctrlOvlyParamsLCD;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsLCD;
    DisplayCtrlLink_OvlyParams              dctrlOvlyParamsHDMI;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsHDMI;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsGRPX;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidthLCD;
    UInt32  displayHeightLCD;
    UInt32  displayWidthHDMI;
    UInt32  displayHeightHDMI;

    UInt32  numLvdsCh;
    /**< Number of channels of LVDS to enable */

    Chains_Ctrl *chainsCfg;

} Chains_LvdsVipDualCam_DualDisplayAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set Select Link Create Parameters
 *
 *          This function is used to set the Select link params.
 *          It is called in Create function. It is advisable to have
 *          chains_lvdsVipDualCam_DualDisplay_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [IN]    SelectLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_lvdsVipDualCam_DualDisplay_SetSelectlinkPrms(SelectLink_CreateParams *pPrm)
{
    pPrm->numOutQue = 2;

    pPrm->outQueChInfo[0].outQueId   = 0;
    pPrm->outQueChInfo[0].numOutCh   = 1;
    pPrm->outQueChInfo[0].inChNum[0] = 0;

    pPrm->outQueChInfo[1].outQueId   = 1;
    pPrm->outQueChInfo[1].numOutCh   = 1;
    pPrm->outQueChInfo[1].inChNum[0] = 1;
}

/**
 *******************************************************************************
 *
 * \brief   Set Display Create Parameters
 *
 *          This function is used to set the Display params.
 *          It is called in Create function. It is advisable to have
 *          chains_lvdsVipDualCam_DualDisplay_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [IN]    DisplayLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_lvdsVipDualCam_DualDisplay_SetDisplayPrms(
                                    DisplayLink_CreateParams *pPrm_VideoHDMI,
                                    DisplayLink_CreateParams *pPrm_VideoLCD,
                                    DisplayLink_CreateParams *pPrm_Grpx,
                                    UInt32 displayWidthHDMI,
                                    UInt32 displayHeightHDMI,
                                    UInt32 displayWidthLCD,
                                    UInt32 displayHeightLCD
                                    )
{
    if(pPrm_VideoHDMI)
    {
        pPrm_VideoHDMI->rtParams.tarWidth         = displayWidthHDMI;
        pPrm_VideoHDMI->rtParams.tarHeight        = displayHeightHDMI;
        pPrm_VideoHDMI->displayId                 = DISPLAY_LINK_INST_DSS_VID1;
    }

    if(pPrm_VideoLCD)
    {
        pPrm_VideoLCD->rtParams.tarWidth         = displayWidthLCD;
        pPrm_VideoLCD->rtParams.tarHeight        = displayHeightLCD;
        pPrm_VideoLCD->displayId                 = DISPLAY_LINK_INST_DSS_VID2;
    }

    if(pPrm_Grpx)
    {
        pPrm_Grpx->displayId                  = DISPLAY_LINK_INST_DSS_GFX1;
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
Void chains_lvdsVipDualCam_DualDisplay_SetAppPrms(chains_lvdsVipDualCam_DualDisplayObj *pUcObj, Void *appObj)
{
    Chains_LvdsVipDualCam_DualDisplayAppObj *pObj
        = (Chains_LvdsVipDualCam_DualDisplayAppObj*)appObj;

    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];

    pObj->numLvdsCh = pObj->chainsCfg->numLvdsCh;
    /* Limit max LVDS channels to 4 */
    if(pObj->numLvdsCh > VIDEO_SENSOR_NUM_LVDS_CAMERAS)
        pObj->numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;

    pObj->captureOutWidth   = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight  = CAPTURE_SENSOR_HEIGHT;

    ChainsCommon_GetDisplayWidthHeight
        (pObj->chainsCfg->displayType,
        &pObj->displayWidthLCD,
        &pObj->displayHeightLCD);

    pObj->displayWidthHDMI  = HDMI_DISPLAY_WIDTH;
    pObj->displayHeightHDMI = HDMI_DISPLAY_HEIGHT;

    ChainsCommon_MultiCam_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        portId,
        pObj->numLvdsCh
        );

    ChainsCommon_MultiCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            portId,
            pObj->numLvdsCh
            );

    chains_lvdsVipDualCam_DualDisplay_SetSelectlinkPrms(&pUcObj->Select_CapturePrm);

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidthHDMI,
                                               pObj->displayHeightHDMI
                                              );

    chains_lvdsVipDualCam_DualDisplay_SetDisplayPrms(&pUcObj->Display_HDMIPrm,
                                               &pUcObj->Display_LCDPrm,
                                               &pUcObj->Display_GrpxPrm,
                                               pObj->displayWidthHDMI,
                                               pObj->displayHeightHDMI,
                                               pObj->displayWidthLCD,
                                               pObj->displayHeightLCD
                                                );

    //The displayType indicates which LCD model it is
    ChainsCommon_DualDisplay_StartDisplayCtrl(pObj->chainsCfg->displayType,
                                pObj->displayWidthLCD, pObj->displayHeightLCD);

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
 * \param   pObj  [IN] Chains_LvdsVipMultiCam_DisplayObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_lvdsVipDualCam_DualDisplay_StartApp(Chains_LvdsVipDualCam_DualDisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_lvdsVipDualCam_DualDisplay_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_LvdsVipMultiCam_DisplayObj
 *
 *******************************************************************************
*/
Void chains_lvdsVipDualCam_DualDisplay_StopAndDeleteApp(Chains_LvdsVipDualCam_DualDisplayAppObj *pObj)
{
    chains_lvdsVipDualCam_DualDisplay_Stop(&pObj->ucObj);
    chains_lvdsVipDualCam_DualDisplay_Delete(&pObj->ucObj);

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
Void chains_lvdsVipDualCam_DualDisplay(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_LvdsVipDualCam_DualDisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_lvdsVipDualCam_DualDisplay_Create(&chainsObj.ucObj, &chainsObj);

    chains_lvdsVipDualCam_DualDisplay_StartApp(&chainsObj);

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
                chains_lvdsVipDualCam_DualDisplay_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_lvdsVipDualCam_DualDisplay_StopAndDeleteApp(&chainsObj);
}

