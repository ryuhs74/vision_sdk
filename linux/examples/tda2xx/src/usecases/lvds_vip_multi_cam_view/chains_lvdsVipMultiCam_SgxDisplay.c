/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file chains_vipSingleCameraView.c
 *
 * \brief  Usecase file implementation of multi-ch capture display
 *
 *         The data flow daigram is shown below
 *
 *             Capture (VIP) 4CH 30fps 1280x720
 *                   |
 *                 Sync
 *                   |
 *                 IPC OUT
 *                   |
 *                 IPC IN
 *                   |
 *                SgxDisplay (A15)
 *
 * \version 0.0 (Jun 2014) : [YM] First version ported for linux.
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "chains_lvdsVipMultiCam_SgxDisplay_priv.h"
#include <linux/examples/tda2xx/include/chains.h>
#include <linux/examples/common/chains_common.h>

#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)
#define LCD_DISPLAY_WIDTH         (800)
#define LCD_DISPLAY_HEIGHT        (480)

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
 *  \brief  SingleCameraViewObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {
    /**< Link Id's and device IDs to use for this use-case */
    chains_lvdsVipMultiCam_SgxDisplayObj ucObj;

    UInt32  appCtrlLinkId;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;

    Chains_Ctrl *chainsCfg;

} Chains_lvdsVipMultiCam_SgxDisplayAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set SGXDISPLAY Link Parameters
 *
 *          It is called in Create function.

 *
 * \param   pPrm    [IN]    IpcLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_SgxDisplay_SetSgxDisplayLinkPrms (
                                  SgxDisplayLink_CreateParams *prms,
                                  UInt32 width, UInt32 height)
{
    prms->displayWidth = width;
    prms->displayHeight = height;
    prms->renderType = SGXDISPLAY_RENDER_TYPE_2x2;
    prms->inBufType = SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER;
}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function. It is advisable to have
 *          Chains_lvdsMultiVipCaptureDisplay_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Number of channels to be synced and sync delta and threshold.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_SgxDisplay_SetSyncPrm(
                    SyncLink_CreateParams *pPrm,
                    UInt32 numLvdsCh
                    )
{
    UInt16 chId;

    pPrm->chParams.numCh = numLvdsCh;
    pPrm->chParams.numActiveCh = pPrm->chParams.numCh;
    for(chId = 0; chId < pPrm->chParams.numCh; chId++)
    {
        pPrm->chParams.channelSyncList[chId] = TRUE;
    }

    pPrm->chParams.syncDelta = SYNC_DELTA_IN_MSEC;
    pPrm->chParams.syncThreshold = SYNC_DROP_THRESHOLD_IN_MSEC;
}

/**
 *******************************************************************************
 * *
 * \brief   Set link Parameters
 *
 *          It is called in Create function of the auto generated use-case file.
 *
 * \param pUcObj    [IN] Auto-generated usecase object
 * \param appObj    [IN] Application specific object
 *
 *******************************************************************************
*/
Void chains_lvdsVipMultiCam_SgxDisplay_SetAppPrms(chains_lvdsVipMultiCam_SgxDisplayObj *pUcObj, Void *appObj)
{
    UInt32 displayWidth, displayHeight;

    Chains_lvdsVipMultiCam_SgxDisplayAppObj *pObj
            = (Chains_lvdsVipMultiCam_SgxDisplayAppObj*)appObj;

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    ChainsCommon_MultiCam_SetCapturePrms(
                    &pUcObj->CapturePrm,
                    pObj->chainsCfg->numLvdsCh);
    chains_lvdsVipMultiCam_SgxDisplay_SetSyncPrm(
                        &pUcObj->SyncPrm,
                        pObj->chainsCfg->numLvdsCh
                        );

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &displayWidth,
        &displayHeight
        );

    chains_lvdsVipMultiCam_SgxDisplay_SetSgxDisplayLinkPrms
                                 (&pUcObj->SgxDisplayPrm,
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
 * \param   pObj  [IN] Chains_lvdsVipMultiCam_SgxDisplayAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
void chains_lvdsVipMultiCam_SgxDisplay_StartApp(Chains_lvdsVipMultiCam_SgxDisplayAppObj *pObj)
{
    chains_lvdsVipMultiCam_SgxDisplay_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_lvdsVipMultiCam_SgxDisplayAppObj
 *
 *******************************************************************************
*/
void chains_lvdsVipMultiCam_SgxDisplay_StopApp(Chains_lvdsVipMultiCam_SgxDisplayAppObj *pObj)
{
     chains_lvdsVipMultiCam_SgxDisplay_Stop(&pObj->ucObj);

     chains_lvdsVipMultiCam_SgxDisplay_Delete(&pObj->ucObj);

     ChainsCommon_prfLoadCalcEnable(FALSE, FALSE, FALSE);

     ChainsCommon_memPrintHeapStatus();

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
Void chains_lvdsVipMultiCam_SgxDisplay(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_lvdsVipMultiCam_SgxDisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    ChainsCommon_statCollectorReset();
    ChainsCommon_memPrintHeapStatus();

    chains_lvdsVipMultiCam_SgxDisplay_Create(&chainsObj.ucObj, &chainsObj);

    ChainsCommon_memPrintHeapStatus();

    chains_lvdsVipMultiCam_SgxDisplay_StartApp(&chainsObj);

    ChainsCommon_prfLoadCalcEnable(TRUE, FALSE, FALSE);

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
                ChainsCommon_prfCpuLoadPrint();
                ChainsCommon_statCollectorPrint();
                chains_lvdsVipMultiCam_SgxDisplay_printStatistics(&chainsObj.ucObj);
                chains_lvdsVipMultiCam_SgxDisplay_printBufferStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_lvdsVipMultiCam_SgxDisplay_StopApp(&chainsObj);

}


