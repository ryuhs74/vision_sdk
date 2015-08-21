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
#include "chains_avbRxSurroundView_priv.h"
#include <examples/tda2xx/include/chains_common.h>


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

    chains_avbRxSurroundViewObj ucObj;

    Chains_Ctrl *chainsCfg;

} chains_avbRxSurroundViewAppObj;

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define SYNC_DELTA_IN_MSEC              (0x7FFFFFFF)
#define SYNC_DROP_THRESHOLD_IN_MSEC     (150)

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 * \param   syncMode [IN]    1 - Tight Sync, 0 - Loose Sync
 *          pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_avbRxSurroundView_SetSyncPrm(
                    SyncLink_CreateParams *pPrm,
                    UInt32 numLvdsCh)
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
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          chains_avbRxSurroundView_Display_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Scaling parameters are set .
 *
 * \param   pPrm         [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
static Void chains_avbRxSurroundView_SetVpePrm(
                        VpeLink_CreateParams *pPrm,
                        UInt32 numLvdsCh,
                        UInt32 OutWidth,
                        UInt32 OutHeight
                    )
{
    UInt32 chId;
    VpeLink_ChannelParams *chPrms;
    UInt32 outId = 0;

    pPrm->enableOut[0] = TRUE;
    for (chId = 0; chId < numLvdsCh; chId++)
    {
        chPrms = &pPrm->chParams[chId];
        chPrms->outParams[outId].numBufsPerCh =
                                 VPE_LINK_NUM_BUFS_PER_CH_DEFAULT;

        chPrms->outParams[outId].width = OutWidth;
        chPrms->outParams[outId].height = OutHeight;
        chPrms->outParams[outId].dataFormat = SYSTEM_DF_YUV420SP_UV;

        chPrms->scCfg.bypass       = FALSE;
        chPrms->scCfg.nonLinear    = FALSE;
        chPrms->scCfg.stripSize    = 0;

        chPrms->scCropCfg.cropStartX = 0;
        chPrms->scCropCfg.cropStartY = 0;
        chPrms->scCropCfg.cropWidth  = 1280;
        chPrms->scCropCfg.cropHeight = 720;
    }
}

static void chains_avbRxSurroundView_SetSelectPrm(
                                      SelectLink_CreateParams *pPrm)
{
    pPrm->numOutQue = 2;

    pPrm->outQueChInfo[0].outQueId   = 0;
    pPrm->outQueChInfo[0].numOutCh   = 2;
    pPrm->outQueChInfo[0].inChNum[0] = 0;
    pPrm->outQueChInfo[0].inChNum[1] = 1;

    pPrm->outQueChInfo[1].outQueId   = 1;
    pPrm->outQueChInfo[1].numOutCh   = 2;
    pPrm->outQueChInfo[1].inChNum[0] = 2;
    pPrm->outQueChInfo[1].inChNum[1] = 3;
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
 *
 *******************************************************************************
*/
static Void chains_avbRxSurroundView_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 channelWidth,
                    UInt32 channelHeight,
                    UInt32 channelSpacingHor,
                    UInt32 channelSpacingVer
                   )
{
    UInt32 algId, winId;
    UInt32 useLocalEdma;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;

    useLocalEdma = FALSE;
    algId = ALGORITHM_LINK_IPU_ALG_DMA_SWMS;

    pPrm->baseClassCreate.algId   = algId;
    pPrm->numOutBuf               = 4;
    pPrm->useLocalEdma            = useLocalEdma;
    pPrm->initLayoutParams.numWin = numLvdsCh;

    pPrm->maxOutBufWidth     = channelWidth;
    pPrm->maxOutBufHeight    = (channelHeight*(numLvdsCh)) +
                               (channelSpacingVer*(numLvdsCh-1));

    for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
    {
        pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
        pWinInfo->chId = winId;
        pWinInfo->inStartX = 0;
        pWinInfo->inStartY = 0;
        pWinInfo->width    = channelWidth;
        pWinInfo->height   = channelHeight;
        pWinInfo->outStartX = 0;
        pWinInfo->outStartY = winId*(channelHeight+channelSpacingVer);
     }

    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

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
Void chains_avbRxSurroundView_SetAppPrms(chains_avbRxSurroundViewObj *pUcObj, Void *appObj)
{
    chains_avbRxSurroundViewAppObj *pObj
        = (chains_avbRxSurroundViewAppObj*)appObj;

    ChainsCommon_SurroundView_SetParams(
        NULL,
        &pUcObj->AvbRxPrm,
        &pUcObj->DecodePrm,
        NULL, //&pUcObj->SelectPrm,
        NULL, //&pUcObj->VPE_sv_orgPrm1,
        NULL, //&pUcObj->VPE_sv_orgPrm2,
        &pUcObj->Sync_svPrm,
        NULL, //&pUcObj->Sync_sv_orgPrm1,
        NULL, //&pUcObj->Sync_sv_orgPrm2,
        &pUcObj->Alg_SynthesisPrm,
        &pUcObj->Alg_GeoAlignPrm,
        &pUcObj->Alg_PhotoAlignPrm,
        NULL,
        NULL,
        NULL,
        NULL, //&pUcObj->Alg_DmaSwMs_sv_orgPrm1,
        NULL, //&pUcObj->Alg_DmaSwMs_sv_orgPrm2,
        &pUcObj->GrpxSrcPrm,
        &pUcObj->Display_svPrm,
        NULL, //&pUcObj->Display_sv_orgPrm1,
        NULL, //&pUcObj->Display_sv_orgPrm12,
        &pUcObj->Display_GrpxPrm,
        pObj->chainsCfg->displayType,
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
        pObj->chainsCfg->enableCarOverlayInAlg
        );

    chains_avbRxSurroundView_SetSelectPrm(
                    &pUcObj->SelectPrm);

    chains_avbRxSurroundView_SetVpePrm(
                    &pUcObj->VPE_sv_org1Prm,
                    pObj->chainsCfg->numLvdsCh/2,
                    520,
                    440
                    );
    chains_avbRxSurroundView_SetVpePrm(
                    &pUcObj->VPE_sv_org2Prm,
                    pObj->chainsCfg->numLvdsCh/2,
                    520,
                    440
                    );

    chains_avbRxSurroundView_SetSyncPrm(
                    &pUcObj->Sync_svPrm,
                    pObj->chainsCfg->numLvdsCh);

    chains_avbRxSurroundView_SetSyncPrm(
                    &pUcObj->Sync_sv_org1Prm,
                    pObj->chainsCfg->numLvdsCh/2);

    chains_avbRxSurroundView_SetSyncPrm(
                    &pUcObj->Sync_sv_org2Prm,
                    pObj->chainsCfg->numLvdsCh/2);
    chains_avbRxSurroundView_SetAlgDmaSwMsPrm(
                    &pUcObj->Alg_DmaSwMs_sv_org1Prm,
                    pObj->chainsCfg->numLvdsCh/2,
                    520,
                    440,
                    0,
                    0
                    );
    chains_avbRxSurroundView_SetAlgDmaSwMsPrm(
                    &pUcObj->Alg_DmaSwMs_sv_org2Prm,
                    pObj->chainsCfg->numLvdsCh/2,
                    520,
                    440,
                    0,
                    0
                    );

    pUcObj->GrpxSrcPrm.surroundViewEdgeDetectLayoutEnable = FALSE;
    pUcObj->GrpxSrcPrm.surroundViewStandaloneLayoutEnable = TRUE;

    pUcObj->Display_svPrm.rtParams.posX            = (float)520;
    pUcObj->Display_svPrm.rtParams.posY            = (float)0;

    pUcObj->Display_sv_org1Prm.rtParams.tarWidth   = (float)520;
    pUcObj->Display_sv_org1Prm.rtParams.tarHeight  = (float)880;
    pUcObj->Display_sv_org1Prm.rtParams.posX       = (float)0;
    pUcObj->Display_sv_org1Prm.rtParams.posY       = (float)200;
    pUcObj->Display_sv_org1Prm.displayId           = DISPLAY_LINK_INST_DSS_VID2;

    pUcObj->Display_sv_org2Prm.rtParams.tarWidth   = (float)520;
    pUcObj->Display_sv_org2Prm.rtParams.tarHeight  = (float)880;
    pUcObj->Display_sv_org2Prm.rtParams.posX       = (float)520+880;
    pUcObj->Display_sv_org2Prm.rtParams.posY       = (float)200;
    pUcObj->Display_sv_org2Prm.displayId           = DISPLAY_LINK_INST_DSS_VID3;
}

/**
 *******************************************************************************
 *
 * \brief   Start the avbSurroundView display Links
 *
 *          Function sends a control command to avb  and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] chains_avbRxSurroundViewObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_avbRxSurroundView_StartApp(chains_avbRxSurroundViewAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    chains_avbRxSurroundView_Start(&pObj->ucObj);

    Chains_prfLoadCalcEnable(TRUE, FALSE, FALSE);
}

/**
 *******************************************************************************
 *
 * \brief   Delete the avb display Links
 *
 *          Function sends a control command to all link to
 *          to delete all the prior created links
 *          System_linkDelete is called with LinkId to delete the links.
 *
 * \param   pObj   [IN]   chains_avbRxSurroundViewObj
 *
 *******************************************************************************
*/
Void chains_avbRxSurroundView_StopAndDeleteApp(chains_avbRxSurroundViewAppObj *pObj)
{
    chains_avbRxSurroundView_Stop(&pObj->ucObj);
    chains_avbRxSurroundView_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();
    ChainsCommon_StopDisplayDevice(pObj->chainsCfg->displayType);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

/**
 *******************************************************************************
 *
 * \brief   AVB Surround view Display usecase function
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
Void Chains_avbRxSurroundView(Chains_Ctrl *chainsCfg)
{
    char ch, chPrev;
    UInt32 done = FALSE;
    Bool resetCalib = FALSE;
    chains_avbRxSurroundViewAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    do
    {
        done = FALSE;

        ChainsCommon_SurroundView_CalibInit(resetCalib);
        resetCalib = FALSE;

        chains_avbRxSurroundView_Create(&chainsObj.ucObj, &chainsObj);
        chains_avbRxSurroundView_StartApp(&chainsObj);

        while(!done)
        {
            ch = Chains_menuRunTime();

            switch(ch)
            {
                case '0':
                    chPrev = ChainsCommon_SurroundView_MenuCalibration();
                    done = TRUE;
                    break;
                case 'p':
                case 'P':
                    ChainsCommon_PrintStatistics();
                    chains_avbRxSurroundView_printStatistics(&chainsObj.ucObj);
                    break;
                default:
                    Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                    break;
            }
        }

        chains_avbRxSurroundView_StopAndDeleteApp(&chainsObj);
        ChainsCommon_SurroundView_CalibDeInit();
    } while(chPrev!='3');
}
