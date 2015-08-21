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
#include "chains_lvdsVipMultiCam_Display_tda2xx_mc_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

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

    chains_lvdsVipMultiCam_Display_tda2xx_mcObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    UInt32  displayActiveChId;
    /**< CH ID which is shown on display, by default 2x2 SW Mosaic
     *   is shown on display
     */

    UInt32  numLvdsCh;
    /**< Number of channels of LVDS to enable */

    Chains_Ctrl *chainsCfg;

} Chains_LvdsVipMultiCam_DisplayAppObj;

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
Void chains_lvdsVipMultiCam_Display_tda2xx_mc_SetAppPrms(chains_lvdsVipMultiCam_Display_tda2xx_mcObj *pUcObj, Void *appObj)
{
    Chains_LvdsVipMultiCam_DisplayAppObj *pObj
        = (Chains_LvdsVipMultiCam_DisplayAppObj*)appObj;

    pObj->displayActiveChId = 0;

    pObj->numLvdsCh = pObj->chainsCfg->numLvdsCh;
    /* Limit max LVDS channels to 4 */
    if(pObj->numLvdsCh > VIDEO_SENSOR_NUM_LVDS_CAMERAS)
        pObj->numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;
    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );


    ChainsCommon_MultiCam_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        NULL,
        pObj->numLvdsCh
        );

    ChainsCommon_MultiCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            NULL,
            pObj->numLvdsCh
            );

    ChainsCommon_SetDisplayPrms(&pUcObj->Display_videoPrm,
                                               NULL,
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
 * \param   pObj  [IN] Chains_LvdsVipMultiCam_DisplayObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_StartApp(Chains_LvdsVipMultiCam_DisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_lvdsVipMultiCam_Display_tda2xx_mc_Start(&pObj->ucObj);

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
static Void chains_lvdsVipMultiCam_Display_StopAndDeleteApp(Chains_LvdsVipMultiCam_DisplayAppObj *pObj)
{
    chains_lvdsVipMultiCam_Display_tda2xx_mc_Stop(&pObj->ucObj);
    chains_lvdsVipMultiCam_Display_tda2xx_mc_Delete(&pObj->ucObj);

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
Void Chains_lvdsVipMultiCam_Display_tda2xx_mc(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_LvdsVipMultiCam_DisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_lvdsVipMultiCam_Display_tda2xx_mc_Create(&chainsObj.ucObj, &chainsObj);

    chains_lvdsVipMultiCam_Display_StartApp(&chainsObj);

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
				chains_lvdsVipMultiCam_Display_tda2xx_mc_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_lvdsVipMultiCam_Display_StopAndDeleteApp(&chainsObj);
}

