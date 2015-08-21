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
 *                 AvbRx 4CH 30fps 1280x720
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

#include <linux/examples/tda2xx/include/chains.h>
#include <linux/examples/common/chains_common.h>
#include "chains_avbRxDecodeDisplay_priv.h"


#define AVB_TALKER_MAX_FRAME_WIDTH 1280
#define AVB_TALKER_MAX_FRAME_HEIGHT 720
#define MAX_NUMBER_OF_CHANNELS 4

#define AVB_TALKER_MAX_FRAME_SIZE 300000

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

    chains_avbRxDecodeDisplayObj ucObj;
    Chains_Ctrl *chainsCfg;

} Chains_AvbRxDecodeSgxDisplayAppObj;


/**
 *******************************************************************************
 *
 * \brief   Set Decode Create Parameters
 *
 *          It is called in Create function.
 *          All decoder parameters are set.
 *
 * \param   pPrm         [IN]    DecodeLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SetAvbRxDecodePrm(
                                     AvbRxLink_CreateParams *pAvbPrm,
                                     DecLink_CreateParams *pDecPrm,
                                     UInt32 maxWidth,
                                     UInt32 maxHeight,
                                     UInt32 numCh)
{
    UInt32 chId;
    DecLink_ChCreateParams *decPrm;
    UInt32 nIdx;
    UInt8 stream_ID[][8] =
    {
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x01},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x02},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x03},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x04}
    };
    UInt8 SrcMACAdd[][6] =
    {
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}
    };

    for (nIdx = 0; nIdx < numCh; nIdx++)
    {
        memcpy(pAvbPrm->streamId[nIdx], stream_ID[nIdx],sizeof(stream_ID[nIdx]));
        memcpy(pAvbPrm->srcMacId[nIdx], SrcMACAdd[nIdx],sizeof(SrcMACAdd[nIdx]));
    }
    pAvbPrm->numCameras = numCh;
    pAvbPrm->numBufs = 6;
    pAvbPrm->buffSize = AVB_TALKER_MAX_FRAME_SIZE;
    pAvbPrm->width  = maxWidth;
    pAvbPrm->height = maxHeight;

    for (chId = 0; chId<numCh; chId++)
    {
        decPrm = &pDecPrm->chCreateParams[chId];

        decPrm->dpbBufSizeInFrames  = DEC_LINK_DPB_SIZE_IN_FRAMES_DEFAULT;
        decPrm->algCreateStatus     = DEC_LINK_ALG_CREATE_STATUS_CREATE;
        decPrm->decodeFrameType     = DEC_LINK_DECODE_ALL;

        decPrm->format              = SYSTEM_IVIDEO_MJPEG;
        decPrm->processCallLevel    = DEC_LINK_FRAMELEVELPROCESSCALL;
        decPrm->targetMaxWidth      = maxWidth;
        decPrm->targetMaxHeight     = maxHeight;
        decPrm->numBufPerCh         = 2;
        decPrm->defaultDynamicParams.targetBitRate = 10 * 1000 * 1000;
        decPrm->defaultDynamicParams.targetFrameRate = 30;
        decPrm->fieldMergeDecodeEnable = FALSE;

        switch (decPrm->format)
        {
            case SYSTEM_IVIDEO_MJPEG: /* MJPEG */
                decPrm->profile = 0;
                decPrm->displayDelay = 0;
                break;
            default:
                Vps_printf(" CHAINS: ERROR: Un-supported codec type: %d !!! \n", decPrm->format);
                UTILS_assert(FALSE);
                break;
        }
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
Void ChainsCommon_SetSyncPrm( SyncLink_CreateParams *pPrm,
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
 * \brief   Set Decode Create Parameters
 *
 *          It is called in Create function.
 *          All decoder parameters are set.
 *
 * \param   pPrm         [IN]    DecodeLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SetSgxDisplayPrm(
                                  SgxDisplayLink_CreateParams *prms,
                                  UInt32 width, UInt32 height)
{
    prms->displayWidth = width;
    prms->displayHeight = height;
    prms->renderType = SGXDISPLAY_RENDER_TYPE_2x2;
    prms->inBufType  = SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER;
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
Void chains_avbRxDecodeDisplay_SetAppPrms(chains_avbRxDecodeDisplayObj *pUcObj, Void *appObj)
{
    UInt32 displayWidth, displayHeight;

    Chains_AvbRxDecodeSgxDisplayAppObj *pObj
        = (Chains_AvbRxDecodeSgxDisplayAppObj*)appObj;

    ChainsCommon_SetAvbRxDecodePrm(
        &pUcObj->AvbRxPrm,
        &pUcObj->DecodePrm,
        AVB_TALKER_MAX_FRAME_WIDTH,
        AVB_TALKER_MAX_FRAME_HEIGHT,
        MAX_NUMBER_OF_CHANNELS
        );

    ChainsCommon_SetSyncPrm(
        &pUcObj->Sync_svPrm,
        pObj->chainsCfg->numLvdsCh
        );

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &displayWidth,
        &displayHeight
        );

    ChainsCommon_SetSgxDisplayPrm(&pUcObj->SgxDisplayPrm,
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
 * \param   pObj  [IN] Chains_VipSingleCameraViewObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_avbRxDecodeDisplay_StartApp(Chains_AvbRxDecodeSgxDisplayAppObj *pObj)
{
    ChainsCommon_memPrintHeapStatus();

    chains_avbRxDecodeDisplay_Start(&pObj->ucObj);
    ChainsCommon_prfLoadCalcEnable(TRUE, FALSE, FALSE);
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
 * \param   pObj   [IN]   Chains_VipSingleCameraViewObj
 *
 *******************************************************************************
*/
Void chains_avbRxDecodeDisplay_StopAndDeleteApp(Chains_AvbRxDecodeSgxDisplayAppObj *pObj)
{
    chains_avbRxDecodeDisplay_Stop(&pObj->ucObj);
    chains_avbRxDecodeDisplay_Delete(&pObj->ucObj);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    ChainsCommon_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

/**
 *******************************************************************************
 *
 * \brief   Print Statistics
 *
 *******************************************************************************
*/
Void ChainsCommon_PrintStatistics()
{
    ChainsCommon_prfCpuLoadPrint();
    ChainsCommon_memPrintHeapStatus();
    ChainsCommon_statCollectorPrint();
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
Void Chains_AvbRxDecodeSgxDisplay(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_AvbRxDecodeSgxDisplayAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_avbRxDecodeDisplay_Create(&chainsObj.ucObj, &chainsObj);

    chains_avbRxDecodeDisplay_StartApp(&chainsObj);

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
                chains_avbRxDecodeDisplay_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_avbRxDecodeDisplay_StopAndDeleteApp(&chainsObj);
}




