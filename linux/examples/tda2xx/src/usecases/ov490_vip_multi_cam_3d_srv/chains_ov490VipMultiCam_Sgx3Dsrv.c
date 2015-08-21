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
 *         The data flow diagram is shown below
 *
 *          Capture (VIP) 2CH 30fps 2560x720
 *                        |
 *              Split 4CH 30fps 1280x720
 *                        |
 *                      Sync
 *                        |
 *                      IPC OUT
 *                        |
 *                      IPC IN
 *                        |
 *                     Sgx3Dsrv (A15)
 *
 * \version 0.0 (Jun 2014) : [YM] First version ported for linux.
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "chains_ov490VipMultiCam_Sgx3Dsrv_priv.h"
#include <linux/examples/tda2xx/include/chains.h>
#include <linux/examples/common/chains_common.h>

/* Use OV1063x based mult-deser setup - only for debug */
static volatile UInt32 gUseOv1063x = 0;

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
    chains_ov490VipMultiCam_Sgx3DsrvObj ucObj;

    UInt32  appCtrlLinkId;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  useOV1063x;

    Chains_Ctrl *chainsCfg;
    Chain_Common_SRV_CalibParams gaCalibPrm;

} Chains_ov490VipMultiCam_Sgx3DsrvAppObj;

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
static Void chains_ov490VipMultiCam_Sgx3Dsrv_SetSgx3DsrvLinkPrms (
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
 *          Chains_ov490MultiVipCaptureDisplay_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Number of channels to be synced and sync delta and threshold.
 *
 * \param   pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_ov490VipMultiCam_Sgx3Dsrv_SetSyncPrm(
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
 * \brief   Set Capture Create Parameters for multi camera capture mode
 *
 * \param   pPrm         [IN]  CaptureLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_ov490VipMultiCam_Sgx3Dsrv_SetCapturePrms(
                        CaptureLink_CreateParams *pPrm,
                        UInt32 captureInWidth,
                        UInt32 captureInHeight,
                        UInt32 portId[],
                        UInt32 numLvdsCh
                        )
{
    UInt32 i, streamId;

    CaptureLink_VipInstParams *pInstPrm;
    CaptureLink_InParams *pInprms;
    CaptureLink_OutParams *pOutprms;
    CaptureLink_VipScParams *pScPrm;
    CaptureLink_VipPortConfig    *pPortCfg;

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->numVipInst = numLvdsCh;

    for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
    {
        pInstPrm = &pPrm->vipInst[i];

        UTILS_assert(portId != NULL);
        pInstPrm->vipInstId     =   portId[i];

        pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC;
        pInstPrm->videoIfWidth  =   SYSTEM_VIFW_8BIT;
        pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
        pInstPrm->numStream     =   1;

        pInprms = &pInstPrm->inParams;

        pInprms->width      =   captureInWidth;
        pInprms->height     =   captureInHeight;
        pInprms->dataFormat =   SYSTEM_DF_YUV422P;
        pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;

        for (streamId = 0; streamId < CAPTURE_LINK_MAX_OUTPUT_PER_INST;
                streamId++)
        {
            pOutprms = &pInstPrm->outParams[streamId];
            pOutprms->width         =   pInprms->width;
            pOutprms->height        =   pInprms->height;
            pOutprms->dataFormat    =   SYSTEM_DF_YUV420SP_UV;
            pOutprms->maxWidth      =   pOutprms->width;
            pOutprms->maxHeight     =   pOutprms->height;
            pOutprms->scEnable      =   FALSE;

            /* sub-frame not supported, set to FALSE */
            pOutprms->subFrmPrms.subFrameEnable = FALSE;
            pOutprms->subFrmPrms.numLinesPerSubFrame = 0;

        }
        pScPrm = &pInstPrm->scPrms;
        pScPrm->inCropCfg.cropStartX = 0;
        pScPrm->inCropCfg.cropStartY = 0;
        pScPrm->inCropCfg.cropWidth = pInprms->width;
        pScPrm->inCropCfg.cropHeight = pInprms->height;

        pScPrm->scCfg.bypass       = FALSE;
        pScPrm->scCfg.nonLinear    = FALSE;
        pScPrm->scCfg.stripSize    = 0;

        pScPrm->userCoeff = FALSE;

        /* pScPrm->scCoeffCfg is not reuquired in case
         * pScPrm->userCoeff is FALSE
         */
        pPortCfg = &pInstPrm->vipPortCfg;
        pPortCfg->syncType          =   SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_YUV;
        pPortCfg->ancCropEnable     =   FALSE;

        pPortCfg->intfCfg.clipActive    =   FALSE;
        pPortCfg->intfCfg.clipBlank     =   FALSE;
        pPortCfg->intfCfg.intfWidth     =   SYSTEM_VIFW_16BIT;

        pPortCfg->disCfg.fidSkewPostCnt     =   0;
        pPortCfg->disCfg.fidSkewPreCnt      =   0;
        pPortCfg->disCfg.lineCaptureStyle   =
                                SYSTEM_VIP_LINE_CAPTURE_STYLE_ACTVID;
        pPortCfg->disCfg.fidDetectMode      =   SYSTEM_VIP_FID_DETECT_MODE_PIN;
        pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.discreteBasicMode  =   TRUE;

        pPortCfg->comCfg.ctrlChanSel        =   SYSTEM_VIP_CTRL_CHAN_SEL_7_0;
        pPortCfg->comCfg.ancChSel8b         =
                            SYSTEM_VIP_ANC_CH_SEL_8B_LUMA_SIDE;
        pPortCfg->comCfg.pixClkEdgePol      =   SYSTEM_EDGE_POL_RISING;
        pPortCfg->comCfg.invertFidPol       =   FALSE;
        pPortCfg->comCfg.enablePort         =   FALSE;
        pPortCfg->comCfg.expectedNumLines   =   pInprms->height;
        pPortCfg->comCfg.expectedNumPix     =   pInprms->width;
        pPortCfg->comCfg.repackerMode       =   SYSTEM_VIP_REPACK_CBA_TO_CBA;

        pPortCfg->actCropEnable                         =   TRUE;
        pPortCfg->actCropCfg.srcNum                     =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->actCropCfg.cropCfg.cropWidth          =   pInprms->width;
        pPortCfg->actCropCfg.cropCfg.cropHeight         =   pInprms->height;

        pPortCfg->ancCropCfg.srcNum                     =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropWidth          =   0;
        pPortCfg->ancCropCfg.cropCfg.cropHeight         =   0;

        pInstPrm->numBufs = CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT;
    }
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
Void chains_ov490VipMultiCam_Sgx3Dsrv_SetAppPrms(
            chains_ov490VipMultiCam_Sgx3DsrvObj *pUcObj, Void *appObj)
{

    Chains_ov490VipMultiCam_Sgx3DsrvAppObj *pObj
            = (Chains_ov490VipMultiCam_Sgx3DsrvAppObj*)appObj;

    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];
    portId[0] = 0;
    portId[1] = 1;
    portId[2] = 2;
    portId[3] = 3;

    pObj->useOV1063x = gUseOv1063x;
    if(TRUE == pObj->useOV1063x)
    {
        portId[0] = 0;
        portId[1] = 2;
        portId[2] = 4;
        portId[3] = 8;
    }

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    chains_ov490VipMultiCam_Sgx3Dsrv_SetCapturePrms(
            &pUcObj->CapturePrm,
            2560,
            720,
            portId,
            2
            );

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

    chains_ov490VipMultiCam_Sgx3Dsrv_SetSyncPrm(
                        &pUcObj->SyncPrm,
                        pObj->chainsCfg->numLvdsCh
                        );
    chains_ov490VipMultiCam_Sgx3Dsrv_SetSgx3DsrvLinkPrms
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
 * \param   pObj  [IN] Chains_ov490VipMultiCam_Sgx3DsrvAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
void chains_ov490VipMultiCam_Sgx3Dsrv_StartApp(Chains_ov490VipMultiCam_Sgx3DsrvAppObj *pObj)
{
    chains_ov490VipMultiCam_Sgx3Dsrv_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_ov490VipMultiCam_Sgx3DsrvAppObj
 *
 *******************************************************************************
*/
void chains_ov490VipMultiCam_Sgx3Dsrv_StopApp(Chains_ov490VipMultiCam_Sgx3DsrvAppObj *pObj)
{
     chains_ov490VipMultiCam_Sgx3Dsrv_Stop(&pObj->ucObj);

     chains_ov490VipMultiCam_Sgx3Dsrv_Delete(&pObj->ucObj);

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
Void chains_ov490VipMultiCam_Sgx3Dsrv(Chains_Ctrl *chainsCfg)
{
    char ch, chPrev;
    UInt32 done = FALSE;
    Bool startWithCalibration;
    Chains_ov490VipMultiCam_Sgx3DsrvAppObj chainsObj;
    Chain_Common_SRV_CalibParams * gaCalibPrm;

    chainsObj.chainsCfg = chainsCfg;
    gaCalibPrm = &chainsObj.gaCalibPrm;
    chPrev = '1';

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

        chains_ov490VipMultiCam_Sgx3Dsrv_Create(&chainsObj.ucObj, &chainsObj);

        ChainsCommon_memPrintHeapStatus();

        chains_ov490VipMultiCam_Sgx3Dsrv_StartApp(&chainsObj);

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
                    chains_ov490VipMultiCam_Sgx3Dsrv_printStatistics(&chainsObj.ucObj);
                    chains_ov490VipMultiCam_Sgx3Dsrv_printBufferStatistics(&chainsObj.ucObj);
                    break;
                case 'o':
                case 'O':
                    Vps_printf("Debug option- toggling between OV1063x and OV490 capture\n");
                    Task_sleep(2000U);
                    gUseOv1063x ^= 0x1;
                    done = TRUE;
                    break;
                default:
                    Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                    break;
            }
        }

        chains_ov490VipMultiCam_Sgx3Dsrv_StopApp(&chainsObj);

        ChainsCommon_SurroundView_StopCalibration(gaCalibPrm);
    } while(chPrev!='3');
}


