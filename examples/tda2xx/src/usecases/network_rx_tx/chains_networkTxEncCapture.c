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
#include "chains_networkTxEncCapture_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

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

    chains_networkTxEncCaptureObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    UInt32  netProcId;

    Chains_Ctrl *chainsCfg;

} Chains_NetworkTxEncCaptureAppObj;

Void chains_networkTxEncCapture_SetEncPrms(
                                                EncLink_CreateParams *pPrm,
                                                System_IVideoFormat codecType)
{
    int i, chId;
    EncLink_ChCreateParams *pLinkChPrm;
    EncLink_ChDynamicParams *pLinkDynPrm;

    for (i = 0; i < ENC_LINK_MAX_BUF_ALLOC_POOLS; i++)
    {
        pPrm->numBufPerCh[i] = 4;
    }

    for (chId = 0; chId < ENC_LINK_MAX_CH; chId++)
    {
        pLinkChPrm  = &pPrm->chCreateParams[chId];
        pLinkDynPrm = &pLinkChPrm->defaultDynamicParams;

        UTILS_assert (chId < ENC_LINK_MAX_CH);

        switch (codecType)
        {
          default:
          case SYSTEM_IVIDEO_MJPEG: /* MJPEG */
            pLinkChPrm->format                 = SYSTEM_IVIDEO_MJPEG;
            pLinkChPrm->profile                = 0;
            pLinkChPrm->dataLayout             = VENC_FIELD_SEPARATED;
            pLinkChPrm->fieldMergeEncodeEnable = FALSE;
            pLinkChPrm->enableAnalyticinfo     = 0;
            pLinkChPrm->enableWaterMarking     = 0;
            pLinkChPrm->maxBitRate             = 0;
            pLinkChPrm->encodingPreset         = 0;
            pLinkChPrm->rateControlPreset      = 0;
            pLinkChPrm->enableHighSpeed        = 0;
            pLinkChPrm->enableSVCExtensionFlag = 0;
            pLinkChPrm->numTemporalLayer       = 0;
            pLinkChPrm->overrideInputScanFormat= 0;
            pLinkChPrm->fieldPicEncode         = 0;

            pLinkDynPrm->intraFrameInterval    = 0;
            pLinkDynPrm->targetBitRate         = 10*1000*1000;
            pLinkDynPrm->interFrameInterval    = 0;
            pLinkDynPrm->mvAccuracy            = 0;
            pLinkDynPrm->inputFrameRate        = 30;
            pLinkDynPrm->rcAlg                 = 0;
            pLinkDynPrm->qpMin                 = 0;
            pLinkDynPrm->qpMax                 = 0;
            pLinkDynPrm->qpInit                = -1;
            pLinkDynPrm->vbrDuration           = 0;
            pLinkDynPrm->vbrSensitivity        = 0;
            break;

          case SYSTEM_IVIDEO_H264HP: /* H264 */
            pLinkChPrm->format                 = SYSTEM_IVIDEO_H264HP;
            pLinkChPrm->profile                = 100;
            pLinkChPrm->dataLayout             = VENC_FIELD_SEPARATED;
            pLinkChPrm->fieldMergeEncodeEnable = FALSE;
            pLinkChPrm->enableAnalyticinfo     = 0;
            pLinkChPrm->enableWaterMarking     = 0;
            pLinkChPrm->maxBitRate             = -1;
            pLinkChPrm->encodingPreset         = SYSTEM_XDM_MED_SPEED_HIGH_QUALITY;
            pLinkChPrm->rateControlPreset      = SYSTEM_IVIDEO_LOW_DELAY;
            pLinkChPrm->enableHighSpeed        = FALSE;
            pLinkChPrm->enableSVCExtensionFlag = FALSE;
            pLinkChPrm->numTemporalLayer       = 0;
            pLinkChPrm->overrideInputScanFormat= 0;
            pLinkChPrm->fieldPicEncode         = 0;

            pLinkDynPrm->intraFrameInterval    = 30;
            pLinkDynPrm->targetBitRate         = 10*1000*1000;
            pLinkDynPrm->interFrameInterval    = 1;
            pLinkDynPrm->mvAccuracy            = SYSTEM_IVIDENC2_MOTIONVECTOR_QUARTERPEL;
            pLinkDynPrm->inputFrameRate        = 30;
            pLinkDynPrm->rcAlg                 = 0;
            pLinkDynPrm->qpMin                 = 0;
            pLinkDynPrm->qpMax                 = 51;
            pLinkDynPrm->qpInit                = 25;
            pLinkDynPrm->vbrDuration           = 8;
            pLinkDynPrm->vbrSensitivity        = 0;
            break;
        }
    }
}

/**
 *******************************************************************************
 * \brief   Set create parameters for NullSrc
 *******************************************************************************
*/
Void chains_networkTxEncCapture_SetNullPrms(Chains_NetworkTxEncCaptureAppObj *pObj,
                        NullLink_CreateParams *pPrm)
{
    pPrm->dumpDataType = NULL_LINK_COPY_TYPE_NETWORK;
    pPrm->networkServerPort = NETWORK_TX_SERVER_PORT;
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
Void chains_networkTxEncCapture_SetAppPrms(chains_networkTxEncCaptureObj *pUcObj, Void *appObj)
{
    Chains_NetworkTxEncCaptureAppObj *pObj
        = (Chains_NetworkTxEncCaptureAppObj*)appObj;


    pUcObj->IPCIn_A15_0_IPU1_0_0LinkID
        = SYSTEM_MAKE_LINK_ID(
            pObj->netProcId,
            pUcObj->IPCIn_A15_0_IPU1_0_0LinkID);

    pUcObj->NullLinkID
        = SYSTEM_MAKE_LINK_ID(
            pObj->netProcId,
            pUcObj->NullLinkID);

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    if(pObj->chainsCfg->captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P)
    {
        pObj->captureOutWidth = 1920;
        pObj->captureOutHeight = 1080;
    }

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SingleCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    chains_networkTxEncCapture_SetEncPrms(
        &pUcObj->EncodePrm,
        SYSTEM_IVIDEO_MJPEG
        );

    chains_networkTxEncCapture_SetNullPrms(
        pObj,
        &pUcObj->NullPrm
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
 * \param   pObj  [IN] Chains_NetworkTxEncCaptureObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_networkTxEncCapture_StartApp(Chains_NetworkTxEncCaptureAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    /*This is required to select Ethernet port0 device on J6Eco*/
#ifdef TDA2EX_BUILD
    Bsp_boardSelectDevice(BSP_DRV_ID_ENET_PHY_DP83865,BSP_DEVICE_ENET_PHY_DP83865_INST_ID_0);
#endif

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_networkTxEncCapture_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_NetworkTxEncCaptureObj
 *
 *******************************************************************************
*/
Void chains_networkTxEncCapture_StopAndDeleteApp(Chains_NetworkTxEncCaptureAppObj *pObj)
{
    chains_networkTxEncCapture_Stop(&pObj->ucObj);
    chains_networkTxEncCapture_Delete(&pObj->ucObj);

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
Void Chains_networkTxEncCapture(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_NetworkTxEncCaptureAppObj chainsObj;

    chainsObj.netProcId = Utils_netGetProcId();
    if(chainsObj.netProcId==System_getSelfProcId())
    {
        Vps_printf(" \n");
        Vps_printf(" CHAINS: ERROR: Networking/NDK MUST be run on different CPU"
                   " than IPU1-0.\n");
        Vps_printf(" CHAINS: ERROR: If you need to run this use-case with NDK"
                   " on IPU1-0 then regenerate the use-case with NullSrc/Null"
                   " links on IPU1-0.\n"
                   );
        Vps_printf(" \n");
        return;
    }

    chainsObj.chainsCfg = chainsCfg;

    chains_networkTxEncCapture_Create(&chainsObj.ucObj, &chainsObj);

    chains_networkTxEncCapture_StartApp(&chainsObj);

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
                chains_networkTxEncCapture_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_networkTxEncCapture_StopAndDeleteApp(&chainsObj);
}

