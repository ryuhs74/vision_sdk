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
#include "chains_lvdsVipMultiCam_Display_tda3xx_priv.h"
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

    chains_lvdsVipMultiCam_Display_tda3xxObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    UInt32  numLvdsCh;
    /**< Number of channels of LVDS to enable */

    Chains_Ctrl *chainsCfg;

} Chains_LvdsVipMultiCam_DisplayAppObj;


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
static Void chains_lvdsVipMultiCam_Display_SetSyncPrm(
                    SyncLink_CreateParams *pPrm1,
                    SyncLink_CreateParams *pPrm2,
                    UInt32 numLvdsCh
                    )
{
    UInt16 chId;

    pPrm2->chParams.numCh = numLvdsCh / 2;
    pPrm2->chParams.numActiveCh = pPrm2->chParams.numCh;
    for(chId = 0; chId < pPrm2->chParams.numCh; chId++)
    {
        pPrm2->chParams.channelSyncList[chId] = TRUE;
    }
    pPrm2->chParams.syncDelta = SYNC_DELTA_IN_MSEC;
    pPrm2->chParams.syncThreshold = SYNC_DROP_THRESHOLD_IN_MSEC;

    pPrm1->chParams.numCh = numLvdsCh - pPrm2->chParams.numCh;
    pPrm1->chParams.numActiveCh = pPrm1->chParams.numCh;
    for(chId = 0; chId < pPrm1->chParams.numCh; chId++)
    {
        pPrm1->chParams.channelSyncList[chId] = TRUE;
    }
    pPrm1->chParams.syncDelta = SYNC_DELTA_IN_MSEC;
    pPrm1->chParams.syncThreshold = SYNC_DROP_THRESHOLD_IN_MSEC;
}

static Void chains_lvdsVipMultiCam_Display_SetSelectPrm(
                    SelectLink_CreateParams *pPrm1,
                    UInt32 numLvdsCh
                    )
{
    UInt16 chId;

    pPrm1->outQueChInfo[1].numOutCh = numLvdsCh / 2;
    pPrm1->outQueChInfo[0].numOutCh = numLvdsCh - pPrm1->outQueChInfo[1].numOutCh;
    pPrm1->outQueChInfo[0].outQueId = 0;
    pPrm1->outQueChInfo[1].outQueId = 1;
    for(chId = 0; chId < pPrm1->outQueChInfo[0].numOutCh; chId++)
    {
        /* Vertical mosaic - even channels comes in first column */
        pPrm1->outQueChInfo[0].inChNum[chId] = chId * 2;
    }
    for(chId = 0; chId < pPrm1->outQueChInfo[1].numOutCh; chId++)
    {
        /* Vertical mosaic - Odd channels comes in second column */
        pPrm1->outQueChInfo[1].inChNum[chId] = chId * 2 + 1;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set DMA SW Mosaic Create Parameters
 *
 *          It is called in Create function.
 *          In this function SwMs alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.

 * \param   pPrm    [OUT]    AlgorithmLink_DmaSwMsCreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm1,
                    AlgorithmLink_DmaSwMsCreateParams *pPrm2,
                    UInt32 numLvdsCh,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                   )
{
    UInt32 winId;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;
    UInt32 widthFactor, heightFactor;

    pPrm2->maxOutBufWidth     = displayWidth;
    pPrm2->maxOutBufHeight    = displayHeight * 2;
    pPrm2->numOutBuf          = 4;
    pPrm2->useLocalEdma       = FALSE;
    pPrm1->maxOutBufWidth     = displayWidth;
    pPrm1->maxOutBufHeight    = displayHeight * 2;
    pPrm1->numOutBuf          = 4;
    pPrm1->useLocalEdma       = FALSE;

    pPrm2->initLayoutParams.numWin = numLvdsCh / 2;
    pPrm2->initLayoutParams.outBufWidth  = pPrm2->maxOutBufWidth;
    pPrm2->initLayoutParams.outBufHeight = pPrm2->maxOutBufHeight;
    pPrm1->initLayoutParams.numWin =  numLvdsCh - pPrm2->initLayoutParams.numWin;
    pPrm1->initLayoutParams.outBufWidth  = pPrm1->maxOutBufWidth;
    pPrm1->initLayoutParams.outBufHeight = pPrm1->maxOutBufHeight;

    switch (numLvdsCh)
    {
        case 1:
            widthFactor  = 1;
            heightFactor = 1;
            pPrm1->initLayoutParams.numWin = 1;
            pPrm2->initLayoutParams.numWin = 0;
            break;
        case 2:
            widthFactor  = 1;
            heightFactor = 1;
            pPrm1->initLayoutParams.numWin = 1;
            pPrm2->initLayoutParams.numWin = 1;
            break;
        case 3:
            widthFactor  = 1;
            heightFactor = 2;
            pPrm1->initLayoutParams.numWin = 2;
            pPrm2->initLayoutParams.numWin = 1;
            break;
        case 4:
            widthFactor  = 1;
            heightFactor = 2;
            pPrm1->initLayoutParams.numWin = 2;
            pPrm2->initLayoutParams.numWin = 2;
            break;
        default:
            widthFactor  = 1;
            heightFactor = 2;
            pPrm1->initLayoutParams.numWin = 2;
            pPrm2->initLayoutParams.numWin = 2;
            break;
    }

    /* assuming 4Ch LVDS and 2x2 layout */
    for(winId=0; winId<pPrm1->initLayoutParams.numWin; winId++)
    {
        pWinInfo = &pPrm1->initLayoutParams.winInfo[winId];

        pWinInfo->chId = winId;

        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;

        pWinInfo->width     =
            SystemUtils_floor(pPrm1->initLayoutParams.outBufWidth/widthFactor, 16);
        pWinInfo->height    =
            pPrm1->initLayoutParams.outBufHeight/heightFactor;

        /* winId == 0 */
        pWinInfo->outStartX = 0;
        pWinInfo->outStartY = 0;

        if(winId==1)
        {
            pWinInfo->outStartX = 0;
            pWinInfo->outStartY = pWinInfo->height;
        }
    }
    for(winId=0; winId<pPrm2->initLayoutParams.numWin; winId++)
    {
        pWinInfo = &pPrm2->initLayoutParams.winInfo[winId];

        pWinInfo->chId = winId;

        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;

        pWinInfo->width     =
            SystemUtils_floor(pPrm2->initLayoutParams.outBufWidth/widthFactor, 16);
        pWinInfo->height    =
            pPrm2->initLayoutParams.outBufHeight/heightFactor;

        /* winId == 0 */
        pWinInfo->outStartX = 0;
        pWinInfo->outStartY = 0;
        if(winId==1)
        {
            pWinInfo->outStartX = 0;
            pWinInfo->outStartY = pWinInfo->height;
        }
    }
}

static Void chains_lvdsVipMultiCam_Display_SetDisplayPrm(
                    DisplayLink_CreateParams *pPrm1,
                    DisplayLink_CreateParams *pPrm2,
                    DisplayLink_CreateParams *pPrm_Grpx,
                    Chains_DisplayType displayType,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                   )
{
    pPrm1->displayId          = DISPLAY_LINK_INST_DSS_VID1;
    pPrm1->rtParams.tarWidth  = displayWidth / 2;
    pPrm1->rtParams.tarHeight = displayHeight;
    pPrm1->rtParams.posX      = 0;
    pPrm1->rtParams.posY      = 0;
    if ((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
        (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
    {
        pPrm1->displayScanFormat = SYSTEM_SF_INTERLACED;
    }

    pPrm2->displayId          = DISPLAY_LINK_INST_DSS_VID2;
    pPrm2->rtParams.tarWidth  = displayWidth / 2;
    pPrm2->rtParams.tarHeight = displayHeight;
    pPrm2->rtParams.posX      = displayWidth / 2;
    pPrm2->rtParams.posY      = 0;
    if ((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
        (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
    {
        pPrm2->displayScanFormat = SYSTEM_SF_INTERLACED;
    }

    pPrm_Grpx->displayId = DISPLAY_LINK_INST_DSS_GFX1;
    if ((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
        (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
    {
        pPrm_Grpx->displayScanFormat = SYSTEM_SF_INTERLACED;
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
Void chains_lvdsVipMultiCam_Display_tda3xx_SetAppPrms(chains_lvdsVipMultiCam_Display_tda3xxObj *pUcObj, Void *appObj)
{
    Chains_LvdsVipMultiCam_DisplayAppObj *pObj
        = (Chains_LvdsVipMultiCam_DisplayAppObj*)appObj;
    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];

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
        portId,
        pObj->numLvdsCh
        );

    ChainsCommon_MultiCam_SetCapturePrms(&pUcObj->CapturePrm,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            portId,
            pObj->numLvdsCh
            );

    chains_lvdsVipMultiCam_Display_SetSelectPrm(
                &pUcObj->SelectPrm,
                pObj->numLvdsCh
        );

    chains_lvdsVipMultiCam_Display_SetSyncPrm(
                &pUcObj->Sync_1Prm,
                &pUcObj->Sync_2Prm,
                pObj->numLvdsCh
        );

    chains_lvdsVipMultiCam_Display_SetAlgDmaSwMsPrm(
                            &pUcObj->Alg_DmaSwMs_1Prm,
                            &pUcObj->Alg_DmaSwMs_2Prm,
                            pObj->numLvdsCh,
                            CAPTURE_SENSOR_WIDTH,
                            CAPTURE_SENSOR_HEIGHT
                            );

    chains_lvdsVipMultiCam_Display_SetDisplayPrm(
                            &pUcObj->Display_video1Prm,
                            &pUcObj->Display_video2Prm,
                            &pUcObj->Display_GrpxPrm,
                            pObj->chainsCfg->displayType,
                            pObj->displayWidth,
                            pObj->displayHeight
                            );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
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
 * \param   pObj  [IN] chains_lvdsVipMultiCam_Display_tda3xxObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_StartApp(Chains_LvdsVipMultiCam_DisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_lvdsVipMultiCam_Display_tda3xx_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   chains_lvdsVipMultiCam_Display_tda3xxObj
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_StopAndDeleteApp(Chains_LvdsVipMultiCam_DisplayAppObj *pObj)
{
    chains_lvdsVipMultiCam_Display_tda3xx_Stop(&pObj->ucObj);
    chains_lvdsVipMultiCam_Display_tda3xx_Delete(&pObj->ucObj);

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
Void Chains_lvdsVipMultiCam_Display_tda3xx(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_LvdsVipMultiCam_DisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_lvdsVipMultiCam_Display_tda3xx_Create(&chainsObj.ucObj, &chainsObj);

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
                chains_lvdsVipMultiCam_Display_tda3xx_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_lvdsVipMultiCam_Display_StopAndDeleteApp(&chainsObj);
}
