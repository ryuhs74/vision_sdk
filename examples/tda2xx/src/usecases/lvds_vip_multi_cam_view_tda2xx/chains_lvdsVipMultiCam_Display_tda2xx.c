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
#include "chains_lvdsVipMultiCam_Display_tda2xx_priv.h"
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

    chains_lvdsVipMultiCam_Display_tda2xxObj ucObj;

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
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          chains_lvdsVipMultiCam_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Scaling parameters are set .
 *
 *          Scale each CH to 1/2x size
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_SetVpePrm(
                    VpeLink_CreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                    )
{
    UInt16 chId;
    UInt32 widthFactor, heightFactor;

    pPrm->enableOut[0] = TRUE;

    switch (numLvdsCh)
    {
        case 1:
            widthFactor  = 1;
            heightFactor = 1;
            break;
        case 2:
            widthFactor  = 2;
            heightFactor = 1;
            break;
        case 3:
        case 4:
            widthFactor  = 2;
            heightFactor = 2;
            break;
        case 5:
        case 6:
            widthFactor  = 2;
            heightFactor = 3;
            break;
        default:
            widthFactor  = 2;
            heightFactor = 2;
            break;
    }

    for(chId = 0; chId < numLvdsCh; chId++)
    {
        pPrm->chParams[chId].outParams[0].width
            = SystemUtils_floor(displayWidth/widthFactor, 16);

        pPrm->chParams[chId].outParams[0].height
            = displayHeight/heightFactor;

        pPrm->chParams[chId].outParams[0].dataFormat
            = SYSTEM_DF_YUV420SP_UV;
        pPrm->chParams[chId].outParams[0].numBufsPerCh = 4;
    }
}

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
 *
 * \brief   Set DMA SW Mosaic Create Parameters
 *
 *          It is called in Create function.
 *          In this function SwMs alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.

 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                   )
{
    UInt32 winId;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;
    UInt32 widthFactor, heightFactor;

    pPrm->maxOutBufWidth     = displayWidth;
    pPrm->maxOutBufHeight    = displayHeight;
    pPrm->numOutBuf          = 4;
    pPrm->useLocalEdma       = FALSE;

    pPrm->initLayoutParams.numWin = numLvdsCh;
    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

    switch (numLvdsCh)
    {
        case 1:
            widthFactor  = 1;
            heightFactor = 1;
            pPrm->initLayoutParams.numWin = 1;
            break;
        case 2:
            widthFactor  = 2;
            heightFactor = 1;
            pPrm->initLayoutParams.numWin = 2;
            break;
        case 3:
        case 4:
            widthFactor  = 2;
            heightFactor = 2;
            pPrm->initLayoutParams.numWin = 4;
            break;
        case 5:
        case 6:
            widthFactor  = 2;
            heightFactor = 3;
            pPrm->initLayoutParams.numWin = 6;
            break;
        default:
            widthFactor  = 2;
            heightFactor = 2;
            pPrm->initLayoutParams.numWin = 4;
            break;
    }

    /* assuming 4Ch LVDS and 2x2 layout */
    for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
    {
        pWinInfo = &pPrm->initLayoutParams.winInfo[winId];

        pWinInfo->chId = winId;

        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;

        pWinInfo->width     =
            SystemUtils_floor(pPrm->initLayoutParams.outBufWidth/widthFactor, 16);
        pWinInfo->height    =
            pPrm->initLayoutParams.outBufHeight/heightFactor;

        /* winId == 0 */
        pWinInfo->outStartX = 0;
        pWinInfo->outStartY = 0;

        if(winId==1)
        {
            pWinInfo->outStartX = pWinInfo->width;
            pWinInfo->outStartY = 0;
        } else
        if(winId==2)
        {
            pWinInfo->outStartX = 0;
            pWinInfo->outStartY = pWinInfo->height;
        } else
        if(winId==3)
        {
            pWinInfo->outStartX = pWinInfo->width;
            pWinInfo->outStartY = pWinInfo->height;
        } else
        if(winId==4)
        {
            pWinInfo->outStartX = 0;
            pWinInfo->outStartY = 2 * pWinInfo->height;
        } else
        if(winId==5)
        {
            pWinInfo->outStartX = pWinInfo->width;
            pWinInfo->outStartY = 2 * pWinInfo->height;
        }
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
Void chains_lvdsVipMultiCam_Display_tda2xx_SetAppPrms(chains_lvdsVipMultiCam_Display_tda2xxObj *pUcObj, Void *appObj)
{
    Chains_LvdsVipMultiCam_DisplayAppObj *pObj
        = (Chains_LvdsVipMultiCam_DisplayAppObj*)appObj;

    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];

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
        portId,
        pObj->numLvdsCh
        );

    ChainsCommon_MultiCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            portId,
            pObj->numLvdsCh
            );

    chains_lvdsVipMultiCam_Display_SetVpePrm(&pUcObj->VPEPrm,
                                                pObj->numLvdsCh,
                                                CAPTURE_SENSOR_WIDTH,
                                                CAPTURE_SENSOR_HEIGHT);
    chains_lvdsVipMultiCam_Display_SetSyncPrm(
                &pUcObj->SyncPrm,
                pObj->numLvdsCh
        );

    chains_lvdsVipMultiCam_Display_SetAlgDmaSwMsPrm(
                            &pUcObj->Alg_DmaSwMsPrm,
                            pObj->numLvdsCh,
                            CAPTURE_SENSOR_WIDTH,
                            CAPTURE_SENSOR_HEIGHT
                            );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );

    ChainsCommon_SetDisplayPrms(&pUcObj->Display_videoPrm,
                                               &pUcObj->Display_GrpxPrm,
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

    chains_lvdsVipMultiCam_Display_tda2xx_Start(&pObj->ucObj);

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
    chains_lvdsVipMultiCam_Display_tda2xx_Stop(&pObj->ucObj);
    chains_lvdsVipMultiCam_Display_tda2xx_Delete(&pObj->ucObj);

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
 * \brief   Switch Display Channel
 *
 *          Function sends a control command to display link to
 *          switch the input channel to display
 *          System_linkControl is called with linkId , displayActiveChId
 *          and the cmd DISPLAY_LINK_CMD_SWITCH_CH.
 *
 * \param   pObj    [IN]   Chains_LvdsMultiVipCaptureDisplayObj
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_SwitchDisplayChannel(
                                    Chains_LvdsVipMultiCam_DisplayAppObj *pObj)
{
    DisplayLink_SwitchChannelParams displayPrm;

    pObj->displayActiveChId++;
    if(pObj->displayActiveChId >= (pObj->numLvdsCh+1))
        pObj->displayActiveChId = 0;

    displayPrm.activeChId = pObj->displayActiveChId;

    System_linkControl(pObj->ucObj.Display_videoLinkID,
                                DISPLAY_LINK_CMD_SWITCH_CH,
                                &displayPrm,
                                sizeof(displayPrm),
                                TRUE);
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
extern int gisCapture;
//Chains_LvdsVipMultiCam_DisplayAppObj* gpChainsObj = NULL; //ryuhs74@20151014
Chains_LvdsVipMultiCam_DisplayAppObj chainsObj;
char Chains_menuRunTime2();
Void Chains_lvdsVipMultiCam_Display_tda2xx(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    //Chains_LvdsVipMultiCam_DisplayAppObj chainsObj;

    chainsObj.numLvdsCh         = 0; /* KW error fix */
    chainsObj.displayActiveChId = 0; /* KW error fix */
    chainsObj.chainsCfg = chainsCfg;

    //gpChainsObj = &chainsObj; //ryuhs74@20151014

    chains_lvdsVipMultiCam_Display_tda2xx_Create(&chainsObj.ucObj, &chainsObj);

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
                chains_lvdsVipMultiCam_Display_tda2xx_printStatistics(&chainsObj.ucObj);
                break;
            case '1':
                chains_lvdsVipMultiCam_Display_SwitchDisplayChannel(&chainsObj);
                break;
            case '2':
            {
            	//int i = 0; //컴파일을 위해 일단 선언한 변수, 나중에 Save Prm 구조체로 바꾼다.
            	gisCapture = 1;
            	Vps_printf("**********************************gisCapture : %d\n", gisCapture);
            	//ryuhs74@20151029 - Add File Save Command

            	/*
            	 System_linkControl(chainsObj.ucObj.Save_SaveLinkID,
            			           SYSTEM_CMD_FILE_SAVE,
            			           NULL, //Save Prm 구조체로 수정 예정
								   0,//Save Prm 구조체로 수정 예정
								   TRUE);
				*/
            }
            	break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_lvdsVipMultiCam_Display_StopAndDeleteApp(&chainsObj);
}

void Stop_AVM_E500( ) //ryuhs74@20151014
{
	//if( gpChainsObj != NULL){
		chains_lvdsVipMultiCam_Display_StopAndDeleteApp(&chainsObj);//gpChainsObj);
		//gpChainsObj = NULL;
	//}
}

