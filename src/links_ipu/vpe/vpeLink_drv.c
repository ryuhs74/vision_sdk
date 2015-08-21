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
 * \file vpeLink_drv.c
 *
 * \brief  This file has the implementation of VPE Link
 *
 *   VPE Link can be used to do processing on video input frames. These
 *   frames may be from capture or decoded video frames coming over network.
 *
 *   VPE can do
 *   - Color space conversion on input frames.
 *   - Color space conversion while outputting the frame to memory.
 *   - Scaling on input frames.
 *   - De-Interlacing, (conversion from field to frames )
 *
 *     The VPE link receives the input frames, submitted/queued them into VPS
 *     VPE driver along with a set of output frames to which the VPE driver
 *     write the de-interlaced/scaled output. once the processing is over
 *     the driver invoke a call back. On call back VPE Link reclaim these
 *     frames which are already processed and send back to the previous link.
 *     Also send out the output frames to the next link
 *
 *     VPE is validated only for DEI in Bypass mode (Bypass = TRUE).  This
 *     is because no HW set-up available currently to feed interlaced input
 *
 *     VPE link also supports the run time input and output resolution
 *     change - This feature is NOT verified in this version.
 *
 * \version 0.0 (Sept 2013) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "vpeLink_priv.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 * \brief This function checks and updates SC crop parameters
 *
 *        compare the frame parameters with the previous configuration
 *        stored in the link object. Set the Flag rtPrmUpdate if there
 *        a change and invoke the RT param update functionality
 *
 * \param   pObj        [IN] VPE Link Instance handle
 * \param   chId        [IN] channel number
 * \param   rtPrmUpdate [IN] Flag to check rtPrmUpdate
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvUpdateScCropPrm(VpeLink_Obj * pObj, UInt32 chId,
                                 Bool rtPrmUpdate)
{
    VpeLink_ChObj *pChObj;
    System_LinkChInfo *pInQueChInfo;
    UInt32 outId;
    VpeLink_ChannelParams *chParams;
    System_CropConfig *scCropCfg;

    pChObj = &pObj->chObj[chId];
    pInQueChInfo = &pObj->inQueInfo.chInfo[chId];
    chParams = &pObj->createArgs.chParams[chId];

    for (outId = 0u; outId < VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        scCropCfg = &chParams->scCropCfg;

        if ((scCropCfg->cropStartX) && (!rtPrmUpdate))
            pChObj->scCropCfg[outId].cropStartX = scCropCfg->cropStartX;
        else
            pChObj->scCropCfg[outId].cropStartX = pInQueChInfo->startX;

        if ((scCropCfg->cropWidth) && (!rtPrmUpdate))
            pChObj->scCropCfg[outId].cropWidth = scCropCfg->cropWidth;
        else
            pChObj->scCropCfg[outId].cropWidth = pInQueChInfo->width;

        if ((scCropCfg->cropStartY) && (!rtPrmUpdate))
            pChObj->scCropCfg[outId].cropStartY = scCropCfg->cropStartY;
        else
            pChObj->scCropCfg[outId].cropStartY = pInQueChInfo->startY;

        if ((scCropCfg->cropHeight) && (!rtPrmUpdate))
            pChObj->scCropCfg[outId].cropHeight = scCropCfg->cropHeight;
        else
            pChObj->scCropCfg[outId].cropHeight = pInQueChInfo->height;

        if (SYSTEM_LINK_CH_INFO_GET_FLAG_SCAN_FORMAT(pInQueChInfo->flags) ==
                                                     SYSTEM_SF_INTERLACED &&
            !pObj->createArgs.chParams[chId].deiCfg.bypass)
        {
            pChObj->scCropCfg[outId].cropStartY *= 2;
            pChObj->scCropCfg[outId].cropHeight *= 2;
        }

        if (rtPrmUpdate)
            pChObj->vpeRtPrm.scCropCfg = &pChObj->scCropCfg[outId];
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to enable/disable the film mode
 *
 *        VPE link and driver support run time enable/disable of FILM mode
 *        This mode is supported only for single channel case, i.e.. number of
 *        channel should be 1
 *        This feature is not validated currently with the VPE link
 *
 * \param   pObj      [IN] VPE Link Instance handle
 * \param   fmdEnable [IN] Flag to enable/disable the film mode
 *
 * \return  retVal   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvSetFmdCfg(VpeLink_Obj * pObj, UInt16 fmdEnable)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    Vps_DeiFmdConfig fmdCfg;
    System_LinkChInfo *pInQueChInfo;

    pInQueChInfo = &pObj->inQueInfo.chInfo[0];
    /* Currently FMD only support single channel */
    if (pObj->inQueInfo.numCh > 1u)
    {
        retVal = SYSTEM_LINK_STATUS_EFAIL;
    }
    else
    {
        /* Only single channel is supported for FMD */
        fmdCfg.chNum      = 0;
        /* Set film mode detection to enable/disable */
        fmdCfg.filmMode   = fmdEnable;
        fmdCfg.bed        = 1u;
        fmdCfg.window     = 0u;
        fmdCfg.lock       = 0u;
        fmdCfg.jamDir     = 0u;
        fmdCfg.windowMinx = 0u;
        fmdCfg.windowMiny = 0u;
        fmdCfg.windowMaxx = pInQueChInfo->width - 1u;
        fmdCfg.windowMaxy = pInQueChInfo->height - 1u;

        retVal = Fvid2_control(
            pObj->fvidHandle,
            IOCTL_VPS_SET_DEI_FMD_CFG,
            &fmdCfg,
            NULL);

        UTILS_assert(SYSTEM_LINK_STATUS_SOK == retVal);
        Vps_printf(" VPE: Film mode = %d\n", fmdEnable);
    }

    return (retVal);
}


/**
 *******************************************************************************
 *
 * \brief Function to create the VPE link channel object
 *
 *        Create the VPE link channel object, one per input channel
 *        - Create the intermediate buffer queue per channel
 *        - Create the intermediate Fvid2 frame freeQ
 *        - Updates the create time SC crop parameters
 *        - Populates the VPS driver create and control parameters
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   chId     [IN] channel number
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreateChObj(VpeLink_Obj * pObj, UInt32 chId)
{
    VpeLink_ChObj *pChObj;
    System_LinkChInfo *pInChInfo;
    System_LinkChInfo *pOutChInfo;
    Vps_M2mVpeParams *pDrvChParams;
    FVID2_Format *pFormat;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 outId;
    VpeLink_ChannelParams *chParams;

    UTILS_assert(chId < pObj->inQueInfo.numCh);
    pChObj = &pObj->chObj[chId];
    chParams = &pObj->createArgs.chParams[chId];

    pChObj->pInFrameN_1 = NULL;
    pChObj->pInFrameN_2 = NULL;

    status = Utils_bufCreate(&pChObj->inQue, FALSE, FALSE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    VpeLink_drvCreateFvidFrameQueue(pObj, chId);

    pChObj->nextFid = 0;

    pInChInfo = &pObj->inQueInfo.chInfo[chId];

    pDrvChParams = &pObj->drvChArgs[chId];
    VpsM2mVpeParams_init(pDrvChParams);

    pDrvChParams->chNum = chId;

    pFormat = &pDrvChParams->inFmt;

    pFormat->chNum = chId;
    pFormat->width = pInChInfo->width;
    pFormat->height = pInChInfo->height;

    pFormat->fieldMerged[0] = FALSE;
    if (SYSTEM_LINK_CH_INFO_GET_FLAG_SCAN_FORMAT(pInChInfo->flags) ==
                                               FVID2_SF_INTERLACED &&
        !chParams->deiCfg.bypass)
    {
        pFormat->fieldMerged[0] = TRUE;
    }
    pFormat->fieldMerged[1] = pFormat->fieldMerged[0];
    pFormat->fieldMerged[2] = pFormat->fieldMerged[0];
    pFormat->pitch[0] = pInChInfo->pitch[0];
    pFormat->pitch[1] = pInChInfo->pitch[1];
    pFormat->pitch[2] = pInChInfo->pitch[2];
    pFormat->dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pInChInfo->flags);
    pFormat->scanFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_SCAN_FORMAT(pInChInfo->flags);

    if (chParams->deiCfg.bypass)
    {
        pFormat->scanFormat = FVID2_SF_PROGRESSIVE;
    }
    pFormat->bpp = FVID2_BPP_BITS16;
    pFormat->reserved = NULL;

    pDrvChParams->inMemType = SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(pInChInfo->flags);
    pDrvChParams->outMemType = VPS_VPDMA_MT_NONTILEDMEM;

    pDrvChParams->deiCfg.inpMode = chParams->deiCfg.inpMode;
    pDrvChParams->deiCfg.tempInpEnable = chParams->deiCfg.tempInpEnable;
    pDrvChParams->deiCfg.tempInpChromaEnable = chParams->deiCfg.tempInpChromaEnable;
    pDrvChParams->deiCfg.spatMaxBypass = chParams->deiCfg.spatMaxBypass;
    pDrvChParams->deiCfg.tempMaxBypass = chParams->deiCfg.tempMaxBypass;

    for (outId=0; outId<VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        Utils_resetSkipBufContext(
            &pChObj->frameSkipCtx[outId],
            chParams->outParams[outId].inputFrameRate,
            chParams->outParams[outId].outputFrameRate
            );
    }

    for (outId = 0u; outId < VPE_LINK_OUT_QUE_ID_MAX; outId++)
    {
        pChObj->enableOut[outId] = FALSE;
        if (TRUE == pObj->createArgs.enableOut[outId])
        {
            pChObj->enableOut[outId] = TRUE;

            pOutChInfo = &pObj->info.queInfo[outId].chInfo[chId];

            if (pInChInfo->width < pOutChInfo->width
                || pInChInfo->height < pOutChInfo->height)
            {
                pObj->loadUpsampleCoeffs = TRUE;
            }

            /* initialize the rtparm output resolution from outObj */
            pChObj->chRtOutInfoUpdate[outId] = FALSE;
            pChObj->chRtEnableOutQFlag = FALSE;
            pChObj->chRtOutInfoUpdateWhileDrop = VPE_LINK_OUT_QUE_ID_MAX;

            pChObj->vpeInFrmPrms.width      = pInChInfo->width;
            pChObj->vpeInFrmPrms.height     = pInChInfo->height;
            pChObj->vpeInFrmPrms.pitch[0]   = pInChInfo->pitch[0];
            pChObj->vpeInFrmPrms.pitch[1]   = pInChInfo->pitch[1];
            pChObj->vpeInFrmPrms.pitch[2]   = pInChInfo->pitch[2];
            pChObj->vpeInFrmPrms.memType    =
                    SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(pInChInfo->flags);
            pChObj->vpeInFrmPrms.dataFormat =
                    SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pInChInfo->flags);

            pChObj->vpeRtOutFrmPrm[outId].width = pOutChInfo->width;
            pChObj->vpeRtOutFrmPrm[outId].height = pOutChInfo->height;
            pChObj->vpeRtOutFrmPrm[outId].pitch[0] = pOutChInfo->pitch[0];
            pChObj->vpeRtOutFrmPrm[outId].pitch[1] = pOutChInfo->pitch[1];
            pChObj->vpeRtOutFrmPrm[outId].pitch[2] = pOutChInfo->pitch[2];
            pChObj->vpeRtOutFrmPrm[outId].memType =
                    SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(pOutChInfo->flags);

            pChObj->vpeRtOutFrmPrm[outId].dataFormat =
                    SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pOutChInfo->flags);
        }

        pChObj->scCfg[outId].bypass = chParams->scCfg.bypass;
        pChObj->scCfg[outId].nonLinear = chParams->scCfg.nonLinear;
        pChObj->scCfg[outId].stripSize = chParams->scCfg.stripSize;
        pChObj->scCfg[outId].enablePeaking = TRUE;
        pChObj->scCfg[outId].enableEdgeDetect = TRUE;
        pChObj->scCfg[outId].advCfg = NULL;
    }

    VpeLink_drvUpdateScCropPrm(pObj, chId, FALSE);

    pChObj->deiRtCfg.resetDei = FALSE;
    pChObj->deiRtCfg.fldRepeat = FALSE;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to create the VPE link DEI context buffers
 *
 *        VPE has De-interlacer IP and which required context buffers.
 *        - This function query the VPS driver for context buffer info
 *        - Allocates the context buffers per channel
 *        - Provided the allocated buffer to driver
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  retVal   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvAllocCtxMem(VpeLink_Obj * pObj)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    Vps_DeiCtxInfo deiCtxInfo;
    Vps_DeiCtxBuf deiCtxBuf;
    UInt32 chCnt, bCnt;

    for (chCnt = 0u; chCnt < pObj->drvCreateArgs.numCh; chCnt++)
    {
        /* Get the number of buffers to allocate */
        deiCtxInfo.chNum = chCnt;
        retVal = FVID2_control(pObj->fvidHandle,
                               IOCTL_VPS_GET_DEI_CTX_INFO, &deiCtxInfo, NULL);
        UTILS_assert(SYSTEM_LINK_STATUS_SOK == retVal);

        /* Allocate the buffers as requested by the driver */
        for (bCnt = 0u; bCnt < deiCtxInfo.numFld; bCnt++)
        {
            #ifdef SYSTEM_DEBUG_MEMALLOC
            Vps_printf(" VPE: CTXBUF: FLDBUF: CH%d : BufCnt = %d Size = %d B\n",
                       chCnt,
                       bCnt,
                       deiCtxInfo.fldBufSize);
            #endif /* SYSTEM_DEBUG_MEMALLOC */
            if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)
                ==FALSE
               )
            {
                deiCtxBuf.fldBuf[bCnt] = Utils_memAlloc(
                                                    UTILS_HEAPID_DDR_CACHED_SR,
                                                    deiCtxInfo.fldBufSize,
                                                    VPS_BUFFER_ALIGNMENT);
            }
            else
            {
                deiCtxBuf.fldBuf[bCnt] = System_allocLinkMemAllocInfo(
                                            &pObj->createArgs.memAllocInfo,
                                            deiCtxInfo.fldBufSize,
                                            VPS_BUFFER_ALIGNMENT);
            }
            UTILS_assert(NULL != deiCtxBuf.fldBuf[bCnt]);
        }
        for (bCnt = 0u; bCnt < deiCtxInfo.numMv; bCnt++)
        {
            #ifdef SYSTEM_DEBUG_MEMALLOC
            Vps_printf(" VPE: CTXBUF: MVBUF: CH%d : BufCnt = %d Size = %d B\n",
                       chCnt,
                       bCnt,
                       deiCtxInfo.mvBufSize);
            #endif /* SYSTEM_DEBUG_MEMALLOC */
            if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)
                ==FALSE
               )
            {
                deiCtxBuf.mvBuf[bCnt] = Utils_memAlloc(
                                            UTILS_HEAPID_DDR_CACHED_SR,
                                            deiCtxInfo.mvBufSize,
                                            VPS_BUFFER_ALIGNMENT);
            }
            else
            {
                deiCtxBuf.mvBuf[bCnt] = System_allocLinkMemAllocInfo(
                                            &pObj->createArgs.memAllocInfo,
                                            deiCtxInfo.mvBufSize,
                                            VPS_BUFFER_ALIGNMENT);
            }
            UTILS_assert(NULL != deiCtxBuf.mvBuf[bCnt]);
        }
        for (bCnt = 0u; bCnt < deiCtxInfo.numMvstm; bCnt++)
        {
            #ifdef SYSTEM_DEBUG_MEMALLOC
            Vps_printf(" VPE: CTXBUF: MVSTMBUF: CH%d : BufCnt = %d Size = %d B\n",
                       chCnt,
                       bCnt,
                       deiCtxInfo.mvstmBufSize);
            #endif /* SYSTEM_DEBUG_MEMALLOC */
            if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)
                ==FALSE
               )
            {
                deiCtxBuf.mvstmBuf[bCnt] = Utils_memAlloc(
                                                UTILS_HEAPID_DDR_CACHED_SR,
                                                deiCtxInfo.mvstmBufSize,
                                                VPS_BUFFER_ALIGNMENT);
            }
            else
            {
                deiCtxBuf.mvstmBuf[bCnt] = System_allocLinkMemAllocInfo(
                                                &pObj->createArgs.memAllocInfo,
                                                deiCtxInfo.mvstmBufSize,
                                                VPS_BUFFER_ALIGNMENT);
            }
            UTILS_assert(NULL != deiCtxBuf.mvstmBuf[bCnt]);
        }

        /* Provided the allocated buffer to driver */
        deiCtxBuf.chNum = chCnt;
        retVal = FVID2_control(pObj->fvidHandle,
                               IOCTL_VPS_SET_DEI_CTX_BUF, &deiCtxBuf, NULL);
        UTILS_assert(SYSTEM_LINK_STATUS_SOK == retVal);
    }

    return (retVal);
}

/**
 *******************************************************************************
 *
 * \brief Function to free-up the VPE link DEI context buffers
 *
 *        VPE has De-interlacer IP and which required context buffers.
 *        This function De-Allocates the context buffers, allocated per
 *        channel vise and Provided the to driver
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  retVal   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvFreeCtxMem(VpeLink_Obj * pObj)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    Vps_DeiCtxInfo deiCtxInfo;
    Vps_DeiCtxBuf deiCtxBuf;
    UInt32 chCnt, bCnt;

    for (chCnt = 0u; chCnt < pObj->drvCreateArgs.numCh; chCnt++)
    {
        /* Get the number of buffers to allocate */
        deiCtxInfo.chNum = chCnt;
        retVal = FVID2_control(pObj->fvidHandle,
                               IOCTL_VPS_GET_DEI_CTX_INFO, &deiCtxInfo, NULL);
        UTILS_assert(SYSTEM_LINK_STATUS_SOK == retVal);

        /* Get the allocated buffer back from the driver */
        deiCtxBuf.chNum = chCnt;
        retVal = FVID2_control(pObj->fvidHandle,
                               IOCTL_VPS_GET_DEI_CTX_BUF, &deiCtxBuf, NULL);
        UTILS_assert(SYSTEM_LINK_STATUS_SOK == retVal);

        if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
        {
            /* Free the buffers */
            for (bCnt = 0u; bCnt < deiCtxInfo.numFld; bCnt++)
            {
                Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                              deiCtxBuf.fldBuf[bCnt], deiCtxInfo.fldBufSize);
            }
            for (bCnt = 0u; bCnt < deiCtxInfo.numMv; bCnt++)
            {
                Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                              deiCtxBuf.mvBuf[bCnt], deiCtxInfo.mvBufSize);
            }
            for (bCnt = 0u; bCnt < deiCtxInfo.numMvstm; bCnt++)
            {
                Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                              deiCtxBuf.mvstmBuf[bCnt], deiCtxInfo.mvstmBufSize);
            }
        }
    }

    return (retVal);
}

/**
 *******************************************************************************
 *
 * \brief Function to set the scalar coefficients
 *
 *        Check the input and output resolution and set the appropriate
 *        scalar coefficients. Also Program VPE scalar with these coefficients
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  retVal   [OUT] return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvSetScCoeffs(VpeLink_Obj * pObj)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    Vps_ScCoeffParams coeffPrms;

    VpsScCoeffParams_init(&coeffPrms);
    coeffPrms.scalerId    = VPS_M2M_VPE_SCALER_ID_SC0;

    if (pObj->loadUpsampleCoeffs)
    {
        Vps_printf(" VPE: Loading Up-scaling Co-effs\n");

        coeffPrms.hScalingSet = VPS_SC_US_SET;
        coeffPrms.vScalingSet = VPS_SC_US_SET;
    }
    else
    {
        Vps_printf(" VPE: Loading Down-scaling Co-effs\n");

        coeffPrms.hScalingSet = VPS_SC_DS_SET_8_16;
        coeffPrms.vScalingSet = VPS_SC_DS_SET_8_16;
    }

    /* Program VPE scalar coefficient - Always used */
    retVal = FVID2_control(pObj->fvidHandle,
                           IOCTL_VPS_SET_COEFFS, &coeffPrms, NULL);
    UTILS_assert(SYSTEM_LINK_STATUS_SOK == retVal);

    Vps_printf(" VPE: Co-effs Loading ... DONE !!!\n");

    return (retVal);
}

/**
 *******************************************************************************
 *
 * \brief Function to create the VPE driver handle
 *
 *        Create the VPE link driver object/instance, one per input channel
 *        - Populates the VPS driver create and control parameters
 *        - Create and configure the VPS VPE driver
 *
 * \param   pObj     [IN] VPE Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreateFvidObj(VpeLink_Obj * pObj)
{
    Vps_M2mVpeParams *pChParams;
    VpeLink_ChObj *pChObj;
    UInt32 chId;
    FVID2_CbParams cbParams;
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;

    /* Init create params */
    VpsM2mCreateParams_init(&pObj->drvCreateArgs);
    pObj->drvCreateArgs.numCh = pObj->inQueInfo.numCh;
    pObj->drvCreateArgs.isDeiFmdEnable = FALSE;
    pObj->drvCreateArgs.chInQueueLength = VPS_M2M_DEF_QUEUE_LEN_PER_CH;

    /* VPE Supports FVID2_Stop */
    pObj->isStopSupported = TRUE;

    for (chId = 0u; chId < pObj->drvCreateArgs.numCh; chId++)
    {
        pChParams = &pObj->drvChArgs[chId];
        pChObj = &pObj->chObj[chId];

        if(pChParams->deiCfg.bypass==FALSE)
        {
            /* context buffer handled by user instead of driver */
            pObj->useOverridePrevFldBuf = FALSE;
        }
        pObj->drvInstId = VPS_M2M_INST_VPE1;

        pChParams->scCfg = pChObj->scCfg[VPE_LINK_OUT_QUE_ID_0];
        pChParams->scCropCfg =
            pChObj->scCropCfg[VPE_LINK_OUT_QUE_ID_0];
        pChParams->outFmt = pChObj->outFormat[VPE_LINK_OUT_QUE_ID_0];
    }

    memset(&cbParams, 0, sizeof(cbParams));

    cbParams.cbFxn = VpeLink_drvFvidCb;
    cbParams.errCbFxn = VpeLink_drvFvidErrCb;
    cbParams.errList = &pObj->errProcessList;
    cbParams.appData = pObj;

    pObj->fvidHandle = FVID2_create(FVID2_VPS_M2M_DRV,
                                    pObj->drvInstId,
                                    &pObj->drvCreateArgs,
                                    &pObj->drvCreateStatus, &cbParams);
    UTILS_assert(pObj->fvidHandle != NULL);


    /* Set VPE params for each channel */
    for (chId = 0u; chId < pObj->drvCreateArgs.numCh; chId++)
    {
        retVal = Fvid2_control(pObj->fvidHandle,
                               IOCTL_VPS_M2M_SET_VPE_PARAMS,
                               &pObj->drvChArgs[chId], NULL);

        UTILS_assert(retVal == SYSTEM_LINK_STATUS_SOK);
    }

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief This function prepare the VPS FVID2 frame frameList
 *
 *        VPS driver defines certain input and output frame data structure
 *        and this need to be populated to submit any job to the driver
 *        - Get input buffers from the link intermediate input Queue
 *        - Get free FVID2 frame from input side free FVID2 frame Queue
 *        - Check for RT param update
 *        - Perform output side frame drop operation if any
 *        - Get a free output buffer from the output free queue
 *        - Prepare the input frame list
 *        - Prepare the output frame List
 *
 * \param   pObj           [IN] VPE Link Instance handle
 * \param   inFrameList    [IN] input frame List
 * \param   outFrameList   [IN] output frame List
 * \param   inFrameListN   [IN] Nth input frame List
 * \param   inFrameListN_1 [IN] N-1th input frame List
 * \param   inFrameListN_2 [IN] N-2th input frame List
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvMakeFrameLists(VpeLink_Obj * pObj,
                                FVID2_FrameList * inFrameList,
                                FVID2_FrameList * outFrameList,
                                FVID2_FrameList * inFrameListN,
                                FVID2_FrameList * inFrameListN_1,
                                FVID2_FrameList * inFrameListN_2
                                )
{
    VpeLink_ChObj *pChObj;
    UInt32 chId, outId, frameId;
    FVID2_Frame *pInFrame, *pOutFrame;
    System_Buffer *pInBuffer;
    System_Buffer *pOutBuf;
    Int32 status;
    Bool doFrameDrop;
    Bool repeatFld = FALSE;
    frameId = 0;
    System_VideoFrameBuffer *videoFrame;
    UInt32 flags;
    UInt32 *outQueIdArray;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    outFrameList->numFrames = 0u;

    for (chId = 0; chId < pObj->inQueInfo.numCh; chId++)
    {
        pChObj = &pObj->chObj[chId];

        Utils_bufGetFullBuffer(&pChObj->inQue, &pInBuffer, BSP_OSAL_NO_WAIT);

        if (pInBuffer==NULL)
        {
            continue;
        }

        status  = Utils_queGet(&pChObj->inObj.fvidFrameQueue,
                               (Ptr *)&pInFrame,
                               1,
                               BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        status = Utils_bufInitFrame(pInFrame, pInBuffer);

        memset(&pChObj->vpeRtPrm, 0, sizeof(pChObj->vpeRtPrm));

        inFrameList->frames[frameId] = pInFrame;

        if(pObj->useOverridePrevFldBuf)
        {
            inFrameListN->frames[frameId] = pInFrame;

            /* pChObj->pInFrameN_x == NULL only during start up
                since there are no previous input frames.

                In this case set previous input = current input
            */
            if(pChObj->pInFrameN_1==NULL)
                inFrameListN_1->frames[frameId] = pInFrame;
            else
                inFrameListN_1->frames[frameId] = pChObj->pInFrameN_1;

            /* if N-2 is NULL, set N-2 to be equal to N-1 */
            if(pChObj->pInFrameN_2==NULL)
                inFrameListN_2->frames[frameId] = inFrameListN_1->frames[frameId];
            else
                inFrameListN_2->frames[frameId] = pChObj->pInFrameN_2;
        }

        VpeLink_drvUpdateInputRtPrm(pObj, pInFrame, chId);

        repeatFld = pChObj->deiRtCfg.fldRepeat;


        /* Process for VPE-SC Queue */
        outId = VPE_LINK_OUT_QUE_ID_0;
        pOutBuf = NULL;
        pOutFrame = NULL;

        if (pChObj->enableOut[VPE_LINK_OUT_QUE_ID_1])
        {
            if (pObj->createArgs.chParams[pInFrame->chNum].deiCfg.bypass == TRUE)
            {
                pChObj->chRtEnableOutQFlag ^= TRUE;
            }
            if ((pInFrame->fid == 1) || (pChObj->chRtEnableOutQFlag))
            {
                outId = VPE_LINK_OUT_QUE_ID_1;
            }
            else
            {
                outId = VPE_LINK_OUT_QUE_ID_0;
            }
            pChObj->chRtOutInfoUpdate[outId] = TRUE;
        }

        if ((pObj->createArgs.enableOut[outId]) &&
            (pChObj->enableOut[outId]) &&
            (Utils_queGetQueuedCount (&pObj->outObj[outId].emptyBufQue[chId])))
        {
            doFrameDrop = Utils_doSkipBuf(&(pChObj->frameSkipCtx[outId]));

            if (pChObj->enableOut[VPE_LINK_OUT_QUE_ID_1])
            {
                if (doFrameDrop)
                {
                    pChObj->chRtOutInfoUpdate[outId] = FALSE;
                }
                else
                {
                    if (pChObj->chRtOutInfoUpdateWhileDrop == outId)
                    {
                        pChObj->chRtOutInfoUpdate[outId] = FALSE;
                    }
                }
            }

            if( doFrameDrop == TRUE)
            {
                pOutFrame = &pObj->outFrameDrop;

                linkStatsInfo->linkStats.chStats
                            [pInFrame->chNum].outBufUserDropCount[outId]++;
            }
            else
            {
                status = Utils_queGet(&pObj->outObj[outId].emptyBufQue[chId],
                                      (Ptr *)&pOutBuf, 1,
                                      BSP_OSAL_WAIT_FOREVER);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                UTILS_assert(pOutBuf != NULL);
                pOutBuf->chNum = pInBuffer->chNum;

                linkStatsInfo->linkStats.chStats[
                    pInFrame->chNum].outBufCount[outId]++;

                linkStatsInfo->linkStats.chStats[
                    pInFrame->chNum].inBufProcessCount++;

                pOutBuf->srcTimestamp
                    = pInBuffer->srcTimestamp;

                pOutBuf->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

                pOutFrame = pOutBuf->pVpeLinkPrivate;
                UTILS_assert(pOutFrame != NULL);
                status = Utils_bufInitFrame(pOutFrame, pOutBuf);

                if (pChObj->chRtOutInfoUpdate[outId] == TRUE)
                {
                    pInFrame->perFrameCfg = &pChObj->vpeRtPrm;
                    pChObj->vpeRtPrm.outFrmPrms =
                                     &pChObj->vpeRtOutFrmPrm[outId];
                    pChObj->chRtOutInfoUpdate[outId] = FALSE;
                    pChObj->chRtOutInfoUpdateWhileDrop = outId;
                }
                if(repeatFld)
                {
                    pInFrame->perFrameCfg = &pChObj->vpeRtPrm;
                    pChObj->vpeRtPrm.deiRtCfg = &pChObj->deiRtCfg;
                }

                videoFrame = pOutBuf->payload;
                UTILS_assert(videoFrame != NULL);
                flags = videoFrame->chInfo.flags;

                videoFrame->chInfo.width =
                            pChObj->vpeRtOutFrmPrm[outId].width;
                videoFrame->chInfo.height =
                            pChObj->vpeRtOutFrmPrm[outId].height;
                videoFrame->chInfo.pitch[0] =
                            pChObj->vpeRtOutFrmPrm[outId].pitch[0];
                videoFrame->chInfo.pitch[1] =
                            pChObj->vpeRtOutFrmPrm[outId].pitch[1];
                SYSTEM_LINK_CH_INFO_SET_FLAG_MEM_TYPE(flags,
                  SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(
                    pObj->info.queInfo[outId].chInfo[pInFrame->chNum].flags));
                SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(flags,
                  SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(
                    pObj->info.queInfo[outId].chInfo[pInFrame->chNum].flags));
                SYSTEM_LINK_CH_INFO_SET_FLAG_IS_RT_PRM_UPDATE(flags, 1);
            }

            pOutFrame->chNum = pInFrame->chNum;
            pOutFrame->timeStamp  = pInFrame->timeStamp;
            pOutFrame->fid = pInFrame->fid;
        }
        else
        {
            linkStatsInfo->linkStats.chStats[
                pInFrame->chNum].outBufDropCount[outId]++;
        }

        if (pOutFrame == NULL)
        {
            pOutFrame = &pObj->outFrameDrop;
            pOutFrame->chNum = pInFrame->chNum;
            pOutFrame->timeStamp  = pInFrame->timeStamp;
        }

        outFrameList->frames[frameId] = pOutFrame;
        outQueIdArray = outFrameList[0].appData;
        UTILS_assert(outQueIdArray != NULL);
        outQueIdArray[frameId] = outId;
        frameId++;
    }

    inFrameList->numFrames = frameId;

    if(pObj->useOverridePrevFldBuf)
    {
        inFrameListN  ->numFrames = frameId;
        inFrameListN_1->numFrames = frameId;
        inFrameListN_2->numFrames = frameId;
    }

    outFrameList->numFrames = frameId;

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief This function submit the job to the VPS driver
 *
 *        VPS driver defines certain input and output frame data structure
 *        and this need to be populated to submit any job to the driver
 *        - Call VpeLink_drvQueueFramesToChQue to put the input buffers
 *          into the link internal input buffer queue
 *        - Get the input frame list
 *        - Get the output frame List
 *        - Populate VPS driver the process List
 *        - Call the function to create the in/out frame List
 *        - Call IOCTL if useOverridePrevFldBuf == TRUE
 *        - Submit the job to driver by invoking FVID2_processFrames
 *        - Wait for the process/job completion
 *        - Call FVID2_getProcessedFrames once the completion
 *        Repeat above until all input frames are processed
 *
 * \param   pObj   [IN]  VPE Link Instance handle
 *
 * \param   status [OUT] Return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvProcessData(VpeLink_Obj * pObj)
{
    VpeLink_ReqObj *pReqObj;
    FVID2_ProcessList processList;
    Int32 status;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    if(pObj->isFirstFrameRecv==FALSE)
    {
        pObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(
                &linkStatsInfo->linkStats,
                pObj->inQueInfo.numCh,
                VPE_LINK_OUT_QUE_ID_MAX);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    VpeLink_drvQueueFramesToChQue(pObj);

    while(1)
    {
        pReqObj = &pObj->reqObj;

        pReqObj->outFrameList.appData = &pReqObj->outListQueIdMap[0];
        pReqObj->processList.inFrameList[0] = &pReqObj->inFrameList;
        pReqObj->processList.outFrameList[0] = &pReqObj->outFrameList;
        pReqObj->processList.numInLists = 1;
        pReqObj->processList.numOutLists = 1;

        pReqObj->inFrameList.appData = pReqObj;

        if(pObj->useOverridePrevFldBuf)
        {
            /* previous reference frames in DM814x, DM810x is fixed to 2 */
            pReqObj->prevFldBuf.numFldBufLists = 2;
            pReqObj->prevFldBuf.fldBufFrameList[0] = &pReqObj->inFrameListN_1;
            pReqObj->prevFldBuf.fldBufFrameList[1] = &pReqObj->inFrameListN_2;
        }

        /* submit to driver until input available in the input queue */
        VpeLink_drvMakeFrameLists(pObj,
                                    &pReqObj->inFrameList,
                                    &pReqObj->outFrameList,
                                    &pReqObj->inFrameListN,
                                    &pReqObj->inFrameListN_1,
                                    &pReqObj->inFrameListN_2
                                );

        if (pReqObj->inFrameList.numFrames == 0)
            break;

        pObj->givenInFrames += pReqObj->inFrameList.numFrames;

        if(pObj->useOverridePrevFldBuf)
        {
            status = FVID2_control(
                        pObj->fvidHandle,
                        IOCTL_VPS_VPE_OVERRIDE_PREV_FLD_BUF,
                        &pReqObj->prevFldBuf,
                        NULL
                    );
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }

        status = FVID2_processFrames(pObj->fvidHandle, &pReqObj->processList);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        BspOsal_semWait(pObj->complete, BSP_OSAL_WAIT_FOREVER);

        status =
            FVID2_getProcessedFrames(pObj->fvidHandle, &processList, BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        VpeLink_drvReleaseFrames(pObj, &pReqObj->inFrameList,
                             &pReqObj->outFrameList);

    }
    return SYSTEM_LINK_STATUS_SOK;
}

