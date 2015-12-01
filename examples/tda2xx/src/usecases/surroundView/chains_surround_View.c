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
#include "chains_surround_View_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define SYNC_DELTA_IN_MSEC_CAP              (30)
#define SYNC_DELTA_IN_MSEC_SURROUND_VIEW              (50)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define SYNC_DROP_THRESHOLD_IN_MSEC_CAP     (33)
#define SYNC_DROP_THRESHOLD_IN_MSEC_SURROUND_VIEW     (150)

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

    chains_surround_ViewObj ucObj;

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

} Surround_ViewAppObj;


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
 *

 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Display_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numViewCh,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                   )
{
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;
#define MARGINE	16

    pPrm->maxOutBufWidth     = displayWidth;
    pPrm->maxOutBufHeight    = displayHeight;
    pPrm->numOutBuf          = 4;
    pPrm->useLocalEdma       = FALSE;

    pPrm->initLayoutParams.numWin = numViewCh;
    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

	pWinInfo = &pPrm->initLayoutParams.winInfo[0];

	pWinInfo->chId = 0;

	pWinInfo->inStartX = MARGINE;
	pWinInfo->inStartY = MARGINE;
	pWinInfo->outStartX = MARGINE;
	pWinInfo->outStartY = MARGINE;

	pWinInfo->width     =	520;
	pWinInfo->height    =	688;

	pWinInfo = &pPrm->initLayoutParams.winInfo[1];
	pWinInfo->chId = 1;

	pWinInfo->inStartX = 520 + MARGINE;
	pWinInfo->inStartY = 0;
	pWinInfo->outStartX = 520 + MARGINE;
	pWinInfo->outStartY = 0;

	pWinInfo->width     = 712 + MARGINE;
	pWinInfo->height    = 558 + MARGINE;

	/* winId == 0 */

}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 *          This function is used to set the sync params.
 *          It is called in Create function. It is advisable to have
 *          chains_surround_View_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Number of channels to be synced and sync delta and threshold.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_surround_View_SetSyncPrm(
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
    if(numLvdsCh == 2)
    {
		pPrm->chParams.syncDelta = SYNC_DELTA_IN_MSEC_SURROUND_VIEW;
		pPrm->chParams.syncThreshold = SYNC_DELTA_IN_MSEC_SURROUND_VIEW;
    }else
    {
		pPrm->chParams.syncDelta = SYNC_DELTA_IN_MSEC_CAP;
		pPrm->chParams.syncThreshold = SYNC_DELTA_IN_MSEC_CAP;
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

 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_surround_View_SetAlgSurroundViewPrm(
                    AlgorithmLink_SurroundViewCreateParams *pPrm,
                    UInt32 makeViewPart,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                   )
{
    AlgorithmLink_SurroundViewLutInfo *pLutInfo;
    lut_Info* lutViewInfo;
    int i=0;

    pLutInfo = pPrm->initLayoutParams.lutViewInfo;
    pPrm->initLayoutParams.makeViewPart = makeViewPart;

    pPrm->maxOutBufWidth     = displayWidth;
    pPrm->maxOutBufHeight    = displayHeight;
    pPrm->numOutBuf          = 4;
    pPrm->useLocalEdma       = FALSE;

    pPrm->initLayoutParams.numWin = 4;
    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

    pPrm->initLayoutParams.Basic_frontFullView = LUTAlloc(Basic_frontFullView);
    pPrm->initLayoutParams.Basic_frontNT = LUTAlloc(Basic_frontNT);
    pPrm->initLayoutParams.Basic_frontView = LUTAlloc(Basic_frontView);
    pPrm->initLayoutParams.Basic_leftNT = LUTAlloc(Basic_leftNT);
    pPrm->initLayoutParams.Basic_leftSideView = LUTAlloc(Basic_leftSideView);
    pPrm->initLayoutParams.Basic_rearFullView = LUTAlloc(Basic_rearFullView);
    pPrm->initLayoutParams.Basic_rearNT = LUTAlloc(Basic_rearNT);
    pPrm->initLayoutParams.Basic_rearView = LUTAlloc(Basic_rearView);
    pPrm->initLayoutParams.Basic_rightNT = LUTAlloc(Basic_rightNT);
    pPrm->initLayoutParams.Basic_rightSideView = LUTAlloc(Basic_rightSideView);
    pPrm->initLayoutParams.cmaskNT = LUTAlloc(cmaskNT);


    pLutInfo[LUT_VIEW_INFO_FULL_VIEW].startX 	= 16;
    pLutInfo[LUT_VIEW_INFO_FULL_VIEW].startY 	= 16;
    pLutInfo[LUT_VIEW_INFO_FULL_VIEW].width 	= 1280;
    pLutInfo[LUT_VIEW_INFO_FULL_VIEW].height	= 720;
    pLutInfo[LUT_VIEW_INFO_FULL_VIEW].pitch		= 1280;

    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW].startX 	= 550;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW].startY 	= 16;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW].width 	= 712;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW].height	= 508;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW].pitch		= 1280;

    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].startX 	= 0;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].startY 	= 0;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].width 	= 712;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].height	= 508;
    pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT].pitch		= 712;


    pLutInfo[LUT_VIEW_INFO_TOP_VIEW].startX 	= 16;
    pLutInfo[LUT_VIEW_INFO_TOP_VIEW].startY 	= 16;
    pLutInfo[LUT_VIEW_INFO_TOP_VIEW].width 		= 520;
    pLutInfo[LUT_VIEW_INFO_TOP_VIEW].height		= 688;
    pLutInfo[LUT_VIEW_INFO_TOP_VIEW].pitch		= 1280;

    for(i=0; i<LUT_INFO_INDEX_MAX; i++)
    {
    	lutViewInfo = GetLutInfo((LUT_INFO_INDEX)i);
        pLutInfo[LUT_VIEW_INFO_TOP_A00+i].startX 		= lutViewInfo->startX;
        pLutInfo[LUT_VIEW_INFO_TOP_A00+i].startY 		= lutViewInfo->startY;
        pLutInfo[LUT_VIEW_INFO_TOP_A00+i].width 		= lutViewInfo->width;
        pLutInfo[LUT_VIEW_INFO_TOP_A00+i].height		= lutViewInfo->height;
        pLutInfo[LUT_VIEW_INFO_TOP_A00+i].pitch			= 520;
    }

    if(makeViewPart == 0) ///left View in SurroundView
    {
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].startX 	= 0;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].startY 	= 0;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].width 	= 520;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].height	= 558;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].pitch		= 1248;

		pPrm->initLayoutParams.psingleViewLUT = pPrm->initLayoutParams.Basic_frontFullView;
		pPrm->initLayoutParams.psingleViewInfo = &pLutInfo[LUT_VIEW_INFO_FULL_VIEW];
		pPrm->initLayoutParams.psingleViewLUTInfo = &pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT];
		pPrm->initLayoutParams.singleViewInputChannel = 3;
    }else
    {
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].startX 	= 520;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].startY 	= 0;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].width 	= 1248-520;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].height	= 558;
        pLutInfo[LUT_VIEW_INFO_FULL_VIEW_LUT].pitch		= 1248;


		pPrm->initLayoutParams.psingleViewLUT = pPrm->initLayoutParams.Basic_frontView;
		pPrm->initLayoutParams.psingleViewInfo = &pLutInfo[LUT_VIEW_INFO_SIDE_VIEW];
		pPrm->initLayoutParams.psingleViewLUTInfo = &pLutInfo[LUT_VIEW_INFO_SIDE_VIEW_LUT];
		pPrm->initLayoutParams.singleViewInputChannel = 3;
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
Void chains_surround_View_SetAppPrms(chains_surround_ViewObj *pUcObj, Void *appObj)
{
    Surround_ViewAppObj *pObj
        = (Surround_ViewAppObj*)appObj;

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

    chains_surround_View_SetSyncPrm(
                &pUcObj->SyncPrm,
                pObj->numLvdsCh
        );

    chains_surround_View_SetAlgSurroundViewPrm(
                            &pUcObj->Alg_SurroundViewPrm_0,
                            0,	///left View
                            CAPTURE_SENSOR_WIDTH,
                            CAPTURE_SENSOR_HEIGHT
                            );

    chains_surround_View_SetAlgSurroundViewPrm(
                            &pUcObj->Alg_SurroundViewPrm_1,
                            1,	///right View
                            CAPTURE_SENSOR_WIDTH,
                            CAPTURE_SENSOR_HEIGHT
                            );


    chains_surround_View_SetSyncPrm(
                &pUcObj->SyncSurroundViewPrm,
                2
        );

    chains_lvdsVipMultiCam_Display_SetAlgDmaSwMsPrm(
                            &pUcObj->Alg_DmaSwMsPrm,
                            2,
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
 * \param   pObj  [IN] Surround_ViewObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
static Void chains_surround_View_StartApp(Surround_ViewAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_surround_View_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Surround_ViewObj
 *
 *******************************************************************************
*/
static Void chains_surround_View_StopAndDeleteApp(Surround_ViewAppObj *pObj)
{
    chains_surround_View_Stop(&pObj->ucObj);
    chains_surround_View_Delete(&pObj->ucObj);

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
static Void chains_surround_View_SwitchDisplayChannel(
                                    Surround_ViewAppObj *pObj)
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
Surround_ViewAppObj svChainsObj;
char Chains_menuRunTime2();
Void Chains_surround_View(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    //Surround_ViewAppObj svChainsObj;

    svChainsObj.numLvdsCh         = 0; /* KW error fix */
    svChainsObj.displayActiveChId = 0; /* KW error fix */
    svChainsObj.chainsCfg = chainsCfg;

    //gpChainsObj = &svChainsObj; //ryuhs74@20151014

    chains_surround_View_Create(&svChainsObj.ucObj, &svChainsObj);

    chains_surround_View_StartApp(&svChainsObj);

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
                chains_surround_View_printStatistics(&svChainsObj.ucObj);
                break;
            case '1':
                chains_surround_View_SwitchDisplayChannel(&svChainsObj);
                break;
            case '2':
            {
            	//int i = 0; //�������� ���� �ϴ� ������ ����, ���߿� Save Prm ����ü�� �ٲ۴�.
            	gisCapture = 1;
            	Vps_printf("**********************************gisCapture : %d\n", gisCapture);
            	//ryuhs74@20151029 - Add File Save Command

            	/*
            	 System_linkControl(svChainsObj.ucObj.Save_SaveLinkID,
            			           SYSTEM_CMD_FILE_SAVE,
            			           NULL, //Save Prm ����ü�� ���� ����
								   0,//Save Prm ����ü�� ���� ����
								   TRUE);
				*/
            }
            	break;
            	/*
            	 *

#define  (0x00000009)

#define  (0x00000010)

#define  (0x00000011)

#define  (0x00000012)

#define  (0x00000013)
            	*/
            case '3':
            	Vps_printf("In chains_main, SYSTEM_CMD_FRONT_SIDE_VIEW\n");
            	System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_FRONT_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
            	System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_FRONT_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
            	Vps_printf("   CMD Send chains_main\n");
            	break;
            case '4':
            	Vps_printf("In chains_main, SYSTEM_CMD_REAR_SIDE_VIEW\n");
            	System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_REAR_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
            	System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_REAR_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
            	Vps_printf("   CMD Send chains_main\n");
            	break;
            case '5':
            	Vps_printf("In chains_main, SYSTEM_CMD_RIGH_SIDE_VIEW\n");
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_RIGH_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_RIGH_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				Vps_printf("   CMD Send chains_main\n");
            	break;
            case '6':
            	Vps_printf("In chains_main, SYSTEM_CMD_LEFT_SIDE_VIEW\n");
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_LEFT_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_LEFT_SIDE_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				Vps_printf("   CMD Send chains_main\n");
                break;
            case '7':
            	Vps_printf("In chains_main, SYSTEM_CMD_FULL_FRONT_VIEW\n");
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_FULL_FRONT_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_FULL_FRONT_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				Vps_printf("   CMD Send chains_main\n");
                break;
            case '8':
            	Vps_printf("In chains_main, SYSTEM_CMD_FULL_REAR_VIEW\n");
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_FULL_REAR_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_FULL_REAR_VIEW, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				Vps_printf("   CMD Send chains_main\n");
                break;
            case '9':
            	Vps_printf("In chains_main, SYSTEM_CMD_PRINT_STATISTICS\n");
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_0_ID , SYSTEM_CMD_PRINT_STATISTICS, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				System_linkControl(svChainsObj.ucObj.Alg_SurroundViewLink_1_ID , SYSTEM_CMD_PRINT_STATISTICS, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				Vps_printf("   CMD Send chains_main\n");
                break;
            case 'a':
            	Vps_printf("In chains_main, SYSTEM_CMD_PRINT_STATISTICS\n");
				System_linkControl(svChainsObj.ucObj.CaptureLinkID , SYSTEM_CMD_PRINT_STATISTICS, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
				Vps_printf("   CMD Send chains_main\n");
				break;
            case 'b':
            	Vps_printf("In chains_main, SYSTEM_CMD_STOP To CaptureLinkID\n");
            	System_linkControl(svChainsObj.ucObj.CaptureLinkID , SYSTEM_CMD_STOP, NULL, 0, TRUE); //gGrpxSrcLinkID ��ü�� �ΰ�.
            	Vps_printf("   CMD Send chains_main\n");
            	break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_surround_View_StopAndDeleteApp(&svChainsObj);
}
