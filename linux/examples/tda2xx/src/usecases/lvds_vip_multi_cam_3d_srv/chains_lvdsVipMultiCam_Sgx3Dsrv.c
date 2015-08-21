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
 *                Sgx3Dsrv (A15)
 *
 * \version 0.0 (Jun 2014) : [YM] First version ported for linux.
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "chains_lvdsVipMultiCam_Sgx3Dsrv_priv.h"
#include <linux/examples/tda2xx/include/chains.h>
#include <linux/examples/common/chains_common.h>

#define CAPTURE_SENSOR_WIDTH                 (1280)
#define CAPTURE_SENSOR_HEIGHT                (720)
#define LCD_DISPLAY_WIDTH                    (800)
#define LCD_DISPLAY_HEIGHT                   (480)
#define SGX3DSRV_OUTPUT_FRAME_WIDTH          (880)
#define SGX3DSRV_OUTPUT_FRAME_HEIGHT         (1080)

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define SYNC_DELTA_IN_MSEC                   (16)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define SYNC_DROP_THRESHOLD_IN_MSEC          (33)

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
    chains_lvdsVipMultiCam_Sgx3DsrvObj ucObj;

    UInt32  appCtrlLinkId;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;

    Chains_Ctrl *chainsCfg;
    Chain_Common_SRV_CalibParams gaCalibPrm;

} Chains_lvdsVipMultiCam_Sgx3DsrvAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set SGX3DSRV Link Parameters
 *
 *          It is called in Create function.

 *
 * \param   pPrm    [IN]    IpcLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_lvdsVipMultiCam_Sgx3Dsrv_SetSgx3DsrvLinkPrms (
                                  Sgx3DsrvLink_CreateParams *prms)
{
    prms->maxOutputHeight = SGX3DSRV_OUTPUT_FRAME_HEIGHT;
    prms->maxOutputWidth = SGX3DSRV_OUTPUT_FRAME_WIDTH;
    prms->maxInputHeight = CAPTURE_SENSOR_HEIGHT;
    prms->maxInputWidth = CAPTURE_SENSOR_WIDTH;
    prms->numViews = 1;
    prms->numInQue = SGX3DSRV_LINK_IPQID_GRPX+1;
    prms->inBufType[0] = SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER;
    prms->inBufType[1] = SYSTEM_BUFFER_TYPE_METADATA;
    prms->inBufType[2] = SYSTEM_BUFFER_TYPE_METADATA;
    prms->inBufType[3] = SYSTEM_BUFFER_TYPE_METADATA;
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
static Void chains_lvdsVipMultiCam_Sgx3Dsrv_SetSyncPrm(
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
Void chains_lvdsVipMultiCam_Sgx3Dsrv_SetAppPrms(
            chains_lvdsVipMultiCam_Sgx3DsrvObj *pUcObj, Void *appObj)
{

    Chains_lvdsVipMultiCam_Sgx3DsrvAppObj *pObj
            = (Chains_lvdsVipMultiCam_Sgx3DsrvAppObj*)appObj;

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    ChainsCommon_MultiCam_SetCapturePrms(
                    &pUcObj->CapturePrm,
                    pObj->chainsCfg->numLvdsCh);

    {
        UInt32 i;
        CaptureLink_VipInstParams *pInstPrm;
        for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
        {
            pInstPrm = &pUcObj->CapturePrm.vipInst[i];
            pInstPrm->numBufs = 5;
            /* skip alternate frame to make it 15fps output for Front camera */
            if (i >= 4)
                pInstPrm->outParams[0].frameSkipMask = 0x2AAAAAAA;
        }
    }

    ChainsCommon_3DSurroundView_SetParams(
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &pUcObj->Alg_SynthesisPrm,
        &pUcObj->Alg_GeoAlignPrm,
        &pUcObj->Alg_PhotoAlignPrm,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &pUcObj->GrpxSrcPrm,
        NULL,
        NULL,
        NULL,
        NULL,
        0,
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
        &pObj->gaCalibPrm,
        pObj->chainsCfg->enableCarOverlayInAlg
        );

    chains_lvdsVipMultiCam_Sgx3Dsrv_SetSyncPrm(
                        &pUcObj->SyncPrm,
                        pObj->chainsCfg->numLvdsCh
                        );
    chains_lvdsVipMultiCam_Sgx3Dsrv_SetSgx3DsrvLinkPrms
                        (&pUcObj->Sgx3DsrvPrm);
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
 * \param   pObj  [IN] Chains_lvdsVipMultiCam_Sgx3DsrvAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
void chains_lvdsVipMultiCam_Sgx3Dsrv_StartApp(Chains_lvdsVipMultiCam_Sgx3DsrvAppObj *pObj)
{
    chains_lvdsVipMultiCam_Sgx3Dsrv_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_lvdsVipMultiCam_Sgx3DsrvAppObj
 *
 *******************************************************************************
*/
void chains_lvdsVipMultiCam_Sgx3Dsrv_StopApp(Chains_lvdsVipMultiCam_Sgx3DsrvAppObj *pObj)
{
     chains_lvdsVipMultiCam_Sgx3Dsrv_Stop(&pObj->ucObj);

     chains_lvdsVipMultiCam_Sgx3Dsrv_Delete(&pObj->ucObj);

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
Void chains_lvdsVipMultiCam_Sgx3Dsrv(Chains_Ctrl *chainsCfg)
{
    char ch, chPrev;
    UInt32 done = FALSE;
    Bool startWithCalibration;
    Chains_lvdsVipMultiCam_Sgx3DsrvAppObj chainsObj;
    Chain_Common_SRV_CalibParams * gaCalibPrm;

    chainsObj.chainsCfg = chainsCfg;
    gaCalibPrm = &chainsObj.gaCalibPrm;

    ChainsCommon_statCollectorReset();
    ChainsCommon_memPrintHeapStatus();

    chainsObj.chainsCfg->numLvdsCh = 4;

    do
    {
        done = FALSE;
        /* Set startWithCalibration = TRUE to start the demo with calibration.
           Else it will use the previously calibrated LUTs */
        startWithCalibration = TRUE;
        ChainsCommon_SurroundView_InitCalibration(gaCalibPrm, startWithCalibration);

        chains_lvdsVipMultiCam_Sgx3Dsrv_Create(&chainsObj.ucObj, &chainsObj);

        ChainsCommon_memPrintHeapStatus();

        chains_lvdsVipMultiCam_Sgx3Dsrv_StartApp(&chainsObj);

        ChainsCommon_prfLoadCalcEnable(TRUE, FALSE, FALSE);

        while(!done)
        {
            ch = Chains_menuRunTime();

            switch(ch)
            {
                case '0':
                    chPrev = ChainsCommon_SurroundView_MenuCalibration(gaCalibPrm);
                    done = TRUE;
                    break;
                case 'p':
                case 'P':
                    ChainsCommon_prfCpuLoadPrint();
                    ChainsCommon_statCollectorPrint();
                    chains_lvdsVipMultiCam_Sgx3Dsrv_printStatistics(&chainsObj.ucObj);
                    chains_lvdsVipMultiCam_Sgx3Dsrv_printBufferStatistics(&chainsObj.ucObj);
                    break;
                default:
                    Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                    break;
            }
        }

        chains_lvdsVipMultiCam_Sgx3Dsrv_StopApp(&chainsObj);

        ChainsCommon_SurroundView_StopCalibration(gaCalibPrm);
    } while(chPrev!='3');
}


