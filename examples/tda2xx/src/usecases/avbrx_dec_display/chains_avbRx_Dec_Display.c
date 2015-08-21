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
#include "chains_avbRx_Dec_Display_priv.h"
#include <examples/tda2xx/include/chains_common.h>

#define AVB_TALKER_MAX_FRAME_WIDTH 1280
#define AVB_TALKER_MAX_FRAME_HEIGHT 800
#define MAX_NUMBER_OF_CHANNELS 4

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define SYNC_DELTA_IN_MSEC              (66)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define SYNC_DROP_THRESHOLD_IN_MSEC     (150)

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

    chains_avbRx_Dec_DisplayObj ucObj;

    UInt32  numOfChannels;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

} Chains_AvbRx_Dec_DisplayAppObj;


/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          chains_avbRx_Dec_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Scaling parameters are set .
 *
 *          Scale each CH to 1/2x size
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_avbRx_Dec_Display_SetVpePrm(
                    VpeLink_CreateParams *pPrm,
                    UInt32 numChannels,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                    )
{
    UInt16 chId;
    UInt32 widthFactor, heightFactor;

    pPrm->enableOut[0] = TRUE;

    switch (numChannels)
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

    for(chId = 0; chId < numChannels; chId++)
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
 *          chains_avbRx_Dec_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Number of channels to be synced and sync delta and threshold.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_avbRx_Dec_Display_SetSyncPrm( SyncLink_CreateParams *pPrm,
                                            UInt32 nunChannel )
{
    UInt16 chId;

    pPrm->chParams.numCh = nunChannel;
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
Void chains_avbRx_Dec_Display_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numChannel,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                   )
{
    UInt32 winId;
    UInt32 useLocalEdma;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;
    UInt32 widthFactor, heightFactor;

    useLocalEdma = FALSE;

    pPrm->baseClassCreate.algId = ALGORITHM_LINK_IPU_ALG_DMA_SWMS;

    pPrm->maxOutBufWidth     = displayWidth;
    pPrm->maxOutBufHeight    = displayHeight;
    pPrm->numOutBuf          = 4;
    pPrm->useLocalEdma       = useLocalEdma;

    pPrm->initLayoutParams.numWin = numChannel;
    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

    switch (numChannel)
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

    /* assuming 4Ch and 2x2 layout */
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
Void chains_avbRx_Dec_Display_SetAppPrms(chains_avbRx_Dec_DisplayObj *pUcObj, Void *appObj)
{
    Chains_AvbRx_Dec_DisplayAppObj *pObj
        = (Chains_AvbRx_Dec_DisplayAppObj*)appObj;

    pObj->numOfChannels = 4;

    if(pObj->numOfChannels > MAX_NUMBER_OF_CHANNELS);
        pObj->numOfChannels = MAX_NUMBER_OF_CHANNELS;

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SetAvbRxDecodePrm(
        &pUcObj->AvbRxPrm,
        &pUcObj->DecodePrm,
        AVB_TALKER_MAX_FRAME_WIDTH,
        AVB_TALKER_MAX_FRAME_HEIGHT,
        MAX_NUMBER_OF_CHANNELS
            );

    chains_avbRx_Dec_Display_SetVpePrm(
        &pUcObj->VPEPrm,
        pObj->numOfChannels,
        pObj->displayWidth,
        pObj->displayHeight
        );

    chains_avbRx_Dec_Display_SetSyncPrm(
        &pUcObj->SyncPrm,
        pObj->numOfChannels
        );

    chains_avbRx_Dec_Display_SetAlgDmaSwMsPrm(
        &pUcObj->Alg_DmaSwMsPrm,
        pObj->numOfChannels,
        pObj->displayWidth,
        pObj->displayHeight
        );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );

    ChainsCommon_SetDisplayPrms(&pUcObj->Display_VideoPrm,
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
 * \param   pObj  [IN] Chains_AvbRx_Dec_DisplayObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_avbRx_Dec_Display_StartApp(Chains_AvbRx_Dec_DisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

/*This is required to select Ethernet port0 device on J6Eco*/
#ifdef TDA2EX_BUILD    
    Bsp_boardSelectDevice(BSP_DRV_ID_ENET_PHY_DP83865,BSP_DEVICE_ENET_PHY_DP83865_INST_ID_0);
#endif

    chains_avbRx_Dec_Display_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_AvbRx_Dec_DisplayObj
 *
 *******************************************************************************
*/
Void chains_avbRx_Dec_Display_StopAndDeleteApp(Chains_AvbRx_Dec_DisplayAppObj *pObj)
{
    chains_avbRx_Dec_Display_Stop(&pObj->ucObj);
    chains_avbRx_Dec_Display_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();
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
Void Chains_avbRx_Dec_Display(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_AvbRx_Dec_DisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_avbRx_Dec_Display_Create(&chainsObj.ucObj, &chainsObj);

    chains_avbRx_Dec_Display_StartApp(&chainsObj);

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
                chains_avbRx_Dec_Display_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_avbRx_Dec_Display_StopAndDeleteApp(&chainsObj);
}

