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
 * \file captureLink_displaywb_drv.c
 *
 * \brief  This file communicates with driver for capture link.
 *
 *         This file calls the display write back driver commands and APIs.
 *         All application commands and APIs finally gets
 *         translated to driver APIs and commands by this file.
 *
 * \version 0.0 (May 2015) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "captureLink_priv.h"

void CaptureLink_dispWbCreateInst(CaptureLink_Obj * pObj, UInt16 instId)
{
    Int32  retVal = SYSTEM_LINK_STATUS_SOK;
    Vps_CaptCreateParams *createPrms;
    CaptureLink_InstObj *pInst;
    Vps_CaptDssWbParams *dssWbPrms;
    CaptureLink_DssWbInstParams *pDssWbInst;
    Vps_CaptVipOutInfo *pOutInfo;
    UInt32 outFrmPitch[FVID2_MAX_PLANES]={0};
    System_DssWbInputParams *pIpPrms;
    System_DssWbOutputParams *pOpPrms;
    UInt16 queId, queChId, streamId, chId;
    System_LinkChInfo *pQueChInfo;

    dssWbPrms = &pObj->instObj[instId].dssWbPrms;
    pDssWbInst = &pObj->createArgs.dssWbInst[instId];
    pInst = &pObj->instObj[instId];
    pIpPrms = &pDssWbInst->dssWbInputPrms;
    pOpPrms = &pDssWbInst->dssWbOutputPrms;

    pInst->instId = pDssWbInst->dssWbInstId;
    pObj->mapInstId[pDssWbInst->dssWbInstId] = instId;

    UTILS_assert (pObj->createArgs.numDssWbInst == SYSTEM_CAPTURE_DSSWB_INST_MAX);

    if (Fvid2_isDataFmtSemiPlanar(pOpPrms->wbDataFmt))
    {
        outFrmPitch[FVID2_YUV_SP_Y_ADDR_IDX] =
            VpsUtils_align(pOpPrms->wbWidth, VPS_BUFFER_ALIGNMENT);
        outFrmPitch[FVID2_YUV_SP_CBCR_ADDR_IDX] =
            outFrmPitch[FVID2_YUV_SP_Y_ADDR_IDX];
    }
    else if (Fvid2_isDataFmtYuv422I(pOpPrms->wbDataFmt))
    {
        outFrmPitch[FVID2_YUV_INT_ADDR_IDX] =
            VpsUtils_align(pOpPrms->wbWidth * 2U, VPS_BUFFER_ALIGNMENT);
    }
    else if (Fvid2_isDataFmtRgb16bit(pOpPrms->wbDataFmt))
    {
        outFrmPitch[FVID2_YUV_INT_ADDR_IDX] =
            VpsUtils_align(pOpPrms->wbWidth * 2U, VPS_BUFFER_ALIGNMENT);
    }
    else if (Fvid2_isDataFmtRgb24bit(pOpPrms->wbDataFmt))
    {
        /* Align the pitch to BPP boundary as well since the pitch
         * aligined to VPS_BUFFER_ALIGNMENT may not be multiple of 3
         * bytes (1 pixel) */
        outFrmPitch[FVID2_RGB_ADDR_IDX] =
            VpsUtils_align(pOpPrms->wbWidth * 3U,
                           (VPS_BUFFER_ALIGNMENT * 3U));
    }
    else if (Fvid2_isDataFmtRgb32bit(pOpPrms->wbDataFmt))
    {
        /* Align the pitch to BPP boundary as well since the pitch
         * aligined to VPS_BUFFER_ALIGNMENT may not be multiple of 3
         * bytes (1 pixel) */
        outFrmPitch[FVID2_RGB_ADDR_IDX] =
            VpsUtils_align(pOpPrms->wbWidth * 4U,
                           (VPS_BUFFER_ALIGNMENT));
    }

    createPrms = &pInst->createArgs;
    VpsCaptCreateParams_init(createPrms);
    createPrms->videoIfMode  = FVID2_VIFM_SCH_DS_HSYNC_VSYNC;
    createPrms->videoIfWidth = FVID2_VIFW_8BIT;
    createPrms->bufCaptMode  = VPS_CAPT_BCM_LAST_FRM_REPEAT;
    createPrms->numCh        = 1;
    createPrms->numStream    = 1;

    Fvid2CbParams_init(&pInst->cbPrm);
    pInst->cbPrm.appData = pInst;
    pInst->parent = pObj;
    pInst->cbPrm.cbFxn = CaptureLink_drvCallback;

    for (streamId = 0U; streamId < createPrms->numStream; streamId++)
    {
        for (chId = 0U; chId < createPrms->numCh; chId++)
        {
            createPrms->chNumMap[streamId][chId] = SYSTEM_MAX_CH_PER_OUT_QUE-1;
        }
    }

    pInst->captureVipHandle = Fvid2_create(
                                    FVID2_VPS_CAPT_VID_DRV,
                                    pInst->instId,
                                    createPrms,
                                    &pInst->createStatus,
                                    &pInst->cbPrm);
    if ((NULL == pInst->captureVipHandle) ||
        (pInst->createStatus.retVal != SYSTEM_LINK_STATUS_SOK))
    {
        Vps_printf(" CAPTURE: DSS WB Capture Create Failed!!!\n");
        retVal = pInst->createStatus.retVal;
    }

    VpsCaptDssWbParams_init(dssWbPrms);
    dssWbPrms->inFmt.dataFormat = FVID2_DF_BGR24_888;
    dssWbPrms->inFmt.width      = pIpPrms->wbInSourceWidth;
    dssWbPrms->inFmt.height     = pIpPrms->wbInSourceHeight;
    dssWbPrms->inFmt.scanFormat = pIpPrms->wbScanFormat;
    dssWbPrms->outStreamInfo[0].outFmt.chNum          = SYSTEM_MAX_CH_PER_OUT_QUE-1;
    dssWbPrms->outStreamInfo[0].outFmt.height         = pOpPrms->wbHeight;
    dssWbPrms->outStreamInfo[0].outFmt.width          = pOpPrms->wbWidth;
    dssWbPrms->outStreamInfo[0].outFmt.pitch[0]       = outFrmPitch[0];
    dssWbPrms->outStreamInfo[0].outFmt.dataFormat     = pOpPrms->wbDataFmt;
    dssWbPrms->outStreamInfo[0].outFmt.fieldMerged[0] = TRUE;
    dssWbPrms->outStreamInfo[0].outFmt.scanFormat = pOpPrms->wbScanFormat;
    dssWbPrms->outStreamInfo[0].advDmaCfg        = NULL;
    dssWbPrms->outStreamInfo[0].scEnable         = TRUE;
    dssWbPrms->outStreamInfo[0].cscFullRngEnable = TRUE;
    dssWbPrms->inCropCfg.cropHeight = pIpPrms->wbInHeight;
    dssWbPrms->inCropCfg.cropWidth  = pIpPrms->wbInWidth;
    dssWbPrms->inCropCfg.cropStartX = pIpPrms->wbPosx;
    dssWbPrms->inCropCfg.cropStartY = pIpPrms->wbPosy;
    if ((dssWbPrms->inFmt.width != dssWbPrms->inCropCfg.cropWidth) ||
        (dssWbPrms->inFmt.height != dssWbPrms->inCropCfg.cropHeight))
    {
        dssWbPrms->outStreamInfo[0].cropEnable = TRUE;
    }
    else
    {
        dssWbPrms->outStreamInfo[0].cropEnable = FALSE;
    }

    if (SYSTEM_LINK_STATUS_SOK == retVal)
    {
        retVal = Fvid2_control(
            pInst->captureVipHandle,
            IOCTL_VPS_CAPT_SET_DSSWB_PARAMS,
            dssWbPrms,
            NULL);
        if (retVal != SYSTEM_LINK_STATUS_SOK)
        {
            Vps_printf(" CAPTURE: DSS WB Set Params IOCTL Failed!!!\n");
        }
    }

    pInst->numBufs         = pDssWbInst->numBufs;

    pInst->bufferWidth     = pOpPrms->wbWidth;
    pInst->bufferHeight[0] = pOpPrms->wbHeight;
    /* assume second plane will always be for YUV420SP data format */
    pInst->bufferHeight[1] = pOpPrms->wbHeight/2;
    pInst->bufferHeight[2] = 0;

    pOutInfo = &pInst->vipPrms.outStreamInfo[0];

    pOutInfo->outFmt.pitch[0]   = outFrmPitch[0];
    pOutInfo->outFmt.pitch[1]   = outFrmPitch[0];
    pOutInfo->outFmt.pitch[2]   = outFrmPitch[0];
    pOutInfo->outFmt.dataFormat = pOpPrms->wbDataFmt;

    /*
     * Initialize the channel information for the next link
     */
    queId = 0;
    queChId = pObj->info.queInfo[queId].numCh;

    pQueChInfo = &pObj->info.queInfo[0].chInfo[queChId];

    SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(pQueChInfo->flags,
            pOpPrms->wbDataFmt);

    pQueChInfo->width = pOpPrms->wbWidth;
    pQueChInfo->height = pOpPrms->wbHeight;

    pQueChInfo->startX      = 0;
    pQueChInfo->startY      = 0;
    pQueChInfo->pitch[0]    = pOutInfo->outFmt.pitch[0];
    pQueChInfo->pitch[1]    = pOutInfo->outFmt.pitch[1];
    pQueChInfo->pitch[2]    = pOutInfo->outFmt.pitch[2];

    SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(pQueChInfo->flags,
            pOpPrms->wbScanFormat);

    pObj->info.queInfo[queId].numCh++;
    pObj->chToInstMap[queChId] = instId;

    CaptureLink_drvAllocAndQueueFrames(pObj, pInst);

    return;
}


