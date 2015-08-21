/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file issCaptureLink_drv.c
 *
 * \brief  This file communicates with driver for iss capture link.
 *
 *         This file calls the driver commands and APIs for the application
 *         commands and APIs. All application commands and APIs finally gets
 *         translated to driver APIs and commands by this file.
 *
 * \version 0.0 (Apr 2014) : [PS] First version
 * \version 0.1 (Apr 2015) : [SUJ] Updated for multi-channel support
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "issCaptureLink_priv.h"

/* ========================================================================== */
/*                          Local Function Prototypes                         */
/* ========================================================================== */
static UInt32 getPitchMultiplier(IssCaptureLink_OutParams *pOutPrm,
                            System_VideoIfMode ifMode,
                            System_VideoIfWidth ifWidth);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
/**
 *******************************************************************************
 *
 * \brief Callback function from driver to application
 *
 * Callback function gets called from Driver to application on every Vsync
 * interrupt.
 *
 * \param  handle       [IN] Driver handle for which callback has come.
 * \param  appData      [IN] Application specific data which is registered
 *                           during the callback registration.
 * \param  reserved     [IN] Reserved.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvCallback(FVID2_Handle handle, Ptr appData, Ptr reserved)
{
    IssCaptureLink_Obj *pObj = (IssCaptureLink_Obj*)appData;

    pObj->linkStatsInfo->linkStats.notifyEventCount++;

    Utils_tskSendCmd(&pObj->tsk, SYSTEM_CMD_NEW_DATA, NULL);

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 IssCaptureLink_drvAllocFrames(IssCaptureLink_Obj *pObj)
{
    Int32 status = FVID2_SOK;
    Fvid2_Frame *fvid2Frame;
    System_Buffer *sysBuffer;
    System_VideoFrameBuffer *videoFrame;
    Fvid2_FrameList frmList;
    UInt32 frameId, strmId, numFramesPerCh;
    Utils_DmaChCreateParams dmaParams;

    /* for every stream and channel in a capture handle */
    Fvid2FrameList_init(&frmList);

    for (strmId = 0U; strmId < pObj->createArgs.numCh; strmId++)
    {
        pObj->outBufSize = pObj->info.queInfo[0U].chInfo[strmId].pitch[0U] *
                            pObj->createArgs.outParams[strmId].maxHeight;

        if(pObj->createArgs.outParams[strmId].numOutBuf >
                ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE)
        {
            pObj->createArgs.outParams[strmId].numOutBuf =
                ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE;
        }

        frameId = SYSTEM_LINK_MAX_FRAMES_PER_CH * strmId;
        frmList.numFrames = 0U;
        numFramesPerCh = pObj->createArgs.outParams[strmId].numOutBuf;

        for(; frameId <
                (numFramesPerCh + (strmId * SYSTEM_LINK_MAX_FRAMES_PER_CH));
                frameId++)
        {
            if (ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE <= frameId)
            {
                /* This should not happen */
                UTILS_assert(0);
            }

            sysBuffer  = &pObj->buffers[frameId];
            videoFrame = &pObj->videoFrames[frameId];
            fvid2Frame = &pObj->fvid2Frames[frameId];

            memset(sysBuffer, 0, sizeof(*sysBuffer));
            memset(videoFrame, 0, sizeof(*videoFrame));
            memset(fvid2Frame, 0, sizeof(*fvid2Frame));

            fvid2Frame->perFrameCfg = NULL;
            fvid2Frame->subFrameInfo = NULL;
            fvid2Frame->appData = sysBuffer;
            fvid2Frame->reserved = NULL;
            fvid2Frame->chNum = Vps_captMakeChNum(pObj->drvInstId, strmId, 0U);

            sysBuffer->bufType     = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
            sysBuffer->chNum       = strmId;
            sysBuffer->payloadSize = sizeof(System_VideoFrameBuffer);
            sysBuffer->payload     = videoFrame;

            if(System_useLinkMemAllocInfo(
                    &pObj->createArgs.memAllocInfo)==FALSE)
            {
                fvid2Frame->addr[0][0] =
                    Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        pObj->outBufSize,
                        ISSCAPTURE_LINK_BUF_ALIGNMENT
                        );
            }
            else
            {
                fvid2Frame->addr[0][0] =
                     System_allocLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo,
                            pObj->outBufSize,
                            ISSCAPTURE_LINK_BUF_ALIGNMENT
                        );
            }
            UTILS_assert(fvid2Frame->addr[0][0]!=NULL);

            /*
             * Link the frame to the system buffer. Note that the Frame's
             * appData already points to the system buffer.
             */
            sysBuffer->pCaptureOrgBufferPtr = fvid2Frame;

            videoFrame->bufAddr[0] = fvid2Frame->addr[0][0];

            frmList.frames[frmList.numFrames] = fvid2Frame;
            frmList.numFrames++;
        }

        status = FVID2_queue(pObj->drvHandle, &frmList, strmId);
        if (FVID2_SOK != status)
        {
            Vps_printf(" ISSCAPTURE: ERROR: FVID2 Queue Stream %d Failed !!!\n"
                        , strmId);
            UTILS_assert(0);
        }
    }

    /* Allocate Extra frame for saving captured frame */
    if (pObj->createArgs.allocBufferForRawDump)
    {
            if(System_useLinkMemAllocInfo(
                    &pObj->createArgs.memAllocInfo)==FALSE)
            {
                pObj->saveFrameBufAddr =
                    Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        pObj->outBufSize,
                        ISSCAPTURE_LINK_BUF_ALIGNMENT);
            }
            else
            {
                pObj->saveFrameBufAddr =
                    System_allocLinkMemAllocInfo(
                        &pObj->createArgs.memAllocInfo,
                        pObj->outBufSize,
                        ISSCAPTURE_LINK_BUF_ALIGNMENT);
            }
            UTILS_assert(pObj->saveFrameBufAddr != NULL);

        /* Initialize DMA parameters and create object for Frame Dumping */
        Utils_DmaChCreateParams_Init(&dmaParams);
        status = Utils_dmaCreateCh(
                        &pObj->dumpFramesDmaObj,
                        &dmaParams
                        );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        pObj->saveFrame = 0xFF;
    }

    return status;
}

Int32 IssCaptureLink_drvFreeFrames(IssCaptureLink_Obj *pObj)
{
    Int32 status = FVID2_SOK;
    Fvid2_Frame *fvid2Frame;
    UInt32 frameId, strmId, numFramesPerCh;

    if (System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo) == FALSE)
    {
        for (strmId = 0U; strmId < pObj->createArgs.numCh; strmId++)
        {
            frameId = SYSTEM_LINK_MAX_FRAMES_PER_CH * strmId;
            numFramesPerCh = pObj->createArgs.outParams[strmId].numOutBuf;

            for(; frameId <
                    (numFramesPerCh + (strmId * SYSTEM_LINK_MAX_FRAMES_PER_CH));
                    frameId++)
            {
                fvid2Frame = &pObj->fvid2Frames[frameId];

                status = Utils_memFree(
                            UTILS_HEAPID_DDR_CACHED_SR,
                            fvid2Frame->addr[0][0],
                            pObj->outBufSize
                            );
                UTILS_assert(status==0);
            }
        }
    }

    /* Free up Extra frame for saving captured frame */
    if (pObj->createArgs.allocBufferForRawDump)
    {
        /* Initialize this flag to 0 so that it can't be used */
        pObj->saveFrame = 0xFF;

        if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
        {
            /* Free up the extra buffer memory space */
            status = Utils_memFree(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        pObj->saveFrameBufAddr,
                        pObj->outBufSize
                        );
            UTILS_assert(status==0);
        }
        pObj->saveFrameBufAddr = NULL;

        /* Free up the DMA channel object */
        Utils_dmaDeleteCh(&pObj->dumpFramesDmaObj);
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Process the captured data in response to driver callback.
 *
 * This function gets called in response to driver callback. It dequeues the
 * captured frame from driver, puts it into link output queue and sends message
 * to next link
 *
 * \param  pObj           [IN] Capture link global handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvProcessData(IssCaptureLink_Obj * pObj)
{
    UInt32 frameId, strmId;
    Fvid2_FrameList frmList;
    FVID2_Frame     *fvid2Frame;
    Int32 status;
    System_Buffer *sysBuffer;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    for (strmId = 0U; strmId < pObj->createArgs.numCh; strmId++)
    {
        status = Fvid2_dequeue(
                    pObj->drvHandle,
                    &frmList,
                    strmId,
                    FVID2_TIMEOUT_NONE);

        if ((0U != frmList.numFrames) && (FVID2_SOK == status))
        {
            if (pObj->isFirstFrameRecv == FALSE)
            {
                pObj->isFirstFrameRecv = TRUE;

                Utils_resetLinkStatistics(
                    &linkStatsInfo->linkStats,
                    pObj->info.queInfo[0].numCh,
                    1);
            }

            if (((TRUE == pObj->createArgs.allocBufferForRawDump) &&
                (TRUE == pObj->saveFrame)) && (0U == strmId))
            {
                Utils_DmaCopyFill2D dmaPrm;

                /* This functionality typically will be used for saving
                   RAW frames, which will be typically be more that 8bpp in size,
                   so hard coding dataformat for the DMA to be RAW16 */
                dmaPrm.dataFormat = SYSTEM_DF_RAW16;

                dmaPrm.destAddr[0]  = pObj->saveFrameBufAddr;
                dmaPrm.destPitch[0] = pObj->drvCalCfg.inFmt[0].pitch[0];
                dmaPrm.destStartX   = 0;
                dmaPrm.destStartY   = 0;
                dmaPrm.width        = pObj->createArgs.outParams[0U].width;
                dmaPrm.height       = pObj->createArgs.outParams[0U].height;
                dmaPrm.srcAddr[0]   = frmList.frames[0]->addr[0][0];
                dmaPrm.srcPitch[0]  = pObj->drvCalCfg.inFmt[0].pitch[0];
                dmaPrm.srcStartX    = 0;
                dmaPrm.srcStartY    = 0;

                status = Utils_dmaCopy2D(&pObj->dumpFramesDmaObj, &dmaPrm, 1);
                UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

                /* Reset the flag */
                pObj->saveFrame = FALSE;

                Vps_printf(" **************************************************** \n");
                Vps_printf(" ####### Save Frame from location 0x%x ####### \n",
                    pObj->saveFrameBufAddr);
                Vps_printf(" saveRaw(0, 0x%x, filename, %d, 32, false); ",
                    pObj->saveFrameBufAddr,
                    pObj->drvCalCfg.inFmt[0].pitch[0] *
                    pObj->createArgs.outParams[0U].height / 4);
                Vps_printf(" **************************************************** \n");
            }

            for (frameId = 0; frameId < frmList.numFrames; frameId++)
            {
                fvid2Frame = frmList.frames[frameId];

                sysBuffer = fvid2Frame->appData;
                /* Update the timestamp at this point when frame is available
                 * from driver
                 */
                sysBuffer->srcTimestamp = Utils_getCurGlobalTimeInUsec();
                sysBuffer->linkLocalTimestamp = sysBuffer->srcTimestamp;

                linkStatsInfo->linkStats.chStats[
                    sysBuffer->chNum].outBufCount[0]++;

                if(pObj->createArgs.callback)
                {
                    pObj->createArgs.callback(
                        pObj->createArgs.appObj,
                        sysBuffer);
                }

                status = Utils_bufPutFullBuffer(&pObj->bufQue, sysBuffer);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }

            /*
             * Send command to link for putting the buffer in output queue
             *  of the buffer
             */

            System_sendLinkCmd(pObj->createArgs.outQueParams.nextLink,
                               SYSTEM_CMD_NEW_DATA,
                               NULL);
        }
    }

    return FVID2_SOK;
}


Void IssCaptureLink_drvSetDrvCfg(IssCaptureLink_Obj *pObj)
{
    Vps_CaptCreateParams        *drvCreatePrms;
    vpsissCalCmplxIoCfg_t       *drvCmplxIoCfg;
    IssCaptureLink_Csi2Params   *csi2Params;
    vpsissCalCfg_t              *drvCalCfg;
    IssCaptureLink_OutParams    *outPrms;
    UInt32 strmId;

    drvCreatePrms = &pObj->drvCreatePrms;
    csi2Params = &pObj->createArgs.csi2Params;
    drvCmplxIoCfg = &pObj->drvIssCaptureCreatePrms.cmplxIoCfg[0U];

    Fvid2CbParams_init(&pObj->drvCbPrms);
    VpsCaptCreateParams_init(drvCreatePrms);
    vpsissCalCfg_t_init(&pObj->drvCalCfg);

    pObj->drvInstId    = VPS_CAPT_INST_ISS_CAL_A;

    pObj->drvCbPrms.cbFxn   = &IssCaptureLink_drvCallback;
    pObj->drvCbPrms.appData = pObj;

    drvCreatePrms->videoIfMode     = pObj->createArgs.videoIfMode;
    drvCreatePrms->videoIfWidth    = pObj->createArgs.videoIfWidth;
    drvCreatePrms->bufCaptMode     = pObj->createArgs.bufCaptMode;
    drvCreatePrms->numCh           = 1U;
    /* CAL Treats channels as streams with in CAL */
    drvCreatePrms->numStream       = pObj->createArgs.numCh;
    drvCreatePrms->pAdditionalArgs = &pObj->drvIssCaptureCreatePrms;


    for (strmId = 0U; strmId < pObj->createArgs.numCh; strmId++)
    {
        drvCreatePrms->chNumMap[strmId][0U] =
                Vps_captMakeChNum(pObj->drvInstId, strmId, 0U);
    }

    memset(&pObj->drvIssCaptureCreatePrms, 0U,
            sizeof(pObj->drvIssCaptureCreatePrms));

    pObj->drvCalCfg.numStream = pObj->createArgs.numCh;
    /* Default the PHY clock to reset value */
    pObj->drvIssCaptureCreatePrms.csi2PhyClock = 400U;
    for (strmId = 0U; strmId < pObj->createArgs.numCh; strmId++)
    {
        drvCalCfg = &pObj->drvCalCfg;
        outPrms   = &pObj->createArgs.outParams[strmId];

        drvCalCfg->streamId[strmId] = strmId;
        drvCalCfg->inFmt[strmId].width  = outPrms->width;
        drvCalCfg->inFmt[strmId].height = outPrms->height;
        drvCalCfg->inFmt[strmId].pitch[0U] =
                                pObj->info.queInfo[0].chInfo[strmId].pitch[0];
        drvCalCfg->inFmt[strmId].bpp = FVID2_BPP_BITS16;
        drvCalCfg->inFmt[strmId].dataFormat = 0U;
        drvCalCfg->writeToMem[strmId] = TRUE;
        drvCalCfg->streamType[strmId] = VPS_ISS_CAL_TAG_PIX_DATA;
        drvCalCfg->isPixProcCfgValid[strmId] = FALSE;
        drvCalCfg->isBysOutCfgValid[strmId]  = FALSE;
        drvCalCfg->bysInEnable[strmId]       = FALSE;
        drvCalCfg->isVportCfgValid[strmId]   = FALSE;

        if(SYSTEM_VIFM_SCH_CPI == pObj->createArgs.videoIfMode)
        {
            UTILS_assert(0U == strmId);
            drvCreatePrms->numCh = 1U;

            pObj->drvIssCaptureCreatePrms.subModules[strmId] =
                (VPS_ISS_CAPT_CAL_SUB_CPORT_ID |
                 VPS_ISS_CAPT_CAL_SUB_DMA_WR_ID |
                 VPS_ISS_CAPT_CAL_SUB_DPCM_ENC_ID |
                 VPS_ISS_CAPT_CAL_SUB_PIX_PACK_ID |
                 VPS_ISS_CAPT_CAL_SUB_BYS_IN_ID);

            drvCalCfg->bysInEnable[strmId] = TRUE;

            if(pObj->createArgs.videoIfWidth == SYSTEM_VIFW_10BIT)
            {
                drvCalCfg->csi2DataFormat[strmId] = VPS_ISS_CAL_CSI2_RAW10;
            }
            else if(pObj->createArgs.videoIfWidth == SYSTEM_VIFW_12BIT)
            {
                drvCalCfg->csi2DataFormat[strmId] = VPS_ISS_CAL_CSI2_RAW12;
            }
            else if(pObj->createArgs.videoIfWidth == SYSTEM_VIFW_14BIT)
            {
                drvCalCfg->csi2DataFormat[strmId] = VPS_ISS_CAL_CSI2_RAW14;
            }
            else
            {
                drvCalCfg->csi2DataFormat[strmId] = VPS_ISS_CAL_CSI2_RAW12;
            }
        }
        else if(SYSTEM_VIFM_SCH_CSI2 == pObj->createArgs.videoIfMode)
        {
            pObj->drvIssCaptureCreatePrms.subModules[strmId] =
                                (VPS_ISS_CAPT_CAL_SUB_CSI2_ID |
                                 VPS_ISS_CAPT_CAL_SUB_CPORT_ID |
                                 VPS_ISS_CAPT_CAL_SUB_DMA_WR_ID |
                                 VPS_ISS_CAPT_CAL_SUB_PIX_EXTRACT_ID |
                                 VPS_ISS_CAPT_CAL_SUB_DPCM_DEC_ID |
                                 VPS_ISS_CAPT_CAL_SUB_DPCM_ENC_ID |
                                 VPS_ISS_CAPT_CAL_SUB_PIX_PACK_ID);

            /* We have only 1 instance of PPI, its required to receive streams,
                request it for the first stream only */
            if (0U == strmId)
            {
                pObj->drvIssCaptureCreatePrms.subModules[strmId] |=
                                VPS_ISS_CAPT_CAL_SUB_PPI_ID;
            }

            pObj->drvIssCaptureCreatePrms.csi2PhyClock = csi2Params->csi2PhyClk;

            pObj->drvIssCaptureCreatePrms.isCmplxIoCfgValid = TRUE;

            drvCmplxIoCfg->enable             = TRUE;
            drvCmplxIoCfg->pwrAuto            = TRUE;
            drvCmplxIoCfg->clockLane.pol      =
                                    csi2Params->cmplxIoCfg.clockLane.pol;
            drvCmplxIoCfg->clockLane.position =
                                    csi2Params->cmplxIoCfg.clockLane.position;
            drvCmplxIoCfg->data1Lane.pol      =
                                    csi2Params->cmplxIoCfg.data1Lane.pol;
            drvCmplxIoCfg->data1Lane.position =
                                    csi2Params->cmplxIoCfg.data1Lane.position;
            drvCmplxIoCfg->data2Lane.pol      =
                                    csi2Params->cmplxIoCfg.data2Lane.pol;
            drvCmplxIoCfg->data2Lane.position =
                                    csi2Params->cmplxIoCfg.data2Lane.position;
            drvCmplxIoCfg->data3Lane.pol      =
                                    csi2Params->cmplxIoCfg.data3Lane.pol;
            drvCmplxIoCfg->data3Lane.position =
                                    csi2Params->cmplxIoCfg.data3Lane.position;
            drvCmplxIoCfg->data4Lane.pol      =
                                    csi2Params->cmplxIoCfg.data4Lane.pol;
            drvCmplxIoCfg->data4Lane.position =
                                    csi2Params->cmplxIoCfg.data4Lane.position;

            drvCalCfg->csi2DataFormat[strmId] =
                            (vpsissCalCsi2DataFormat) outPrms->inCsi2DataFormat;

        }
        else
        {
            /* Illegal interface */
            UTILS_assert(0);
        }

        drvCalCfg->csi2VirtualChanNo[strmId] = outPrms->inCsi2VirtualChanNum;
        drvCalCfg->isPixProcCfgValid[strmId] = TRUE;
        drvCalCfg->pixProcCfg[strmId].decCodec = VPS_ISS_CAL_DPCM_DEC_BYPASS;
        drvCalCfg->pixProcCfg[strmId].enableDpcmInitContext = FALSE;
        drvCalCfg->pixProcCfg[strmId].encCodec = VPS_ISS_CAL_DPCM_ENC_BYPASS;
        drvCalCfg->pixProcCfg[strmId].pack     = VPS_ISS_CAL_PIX_PACK_B16;

        switch(drvCalCfg->csi2DataFormat[strmId])
        {
            case SYSTEM_CSI2_RAW10:
                drvCalCfg->pixProcCfg[strmId].extract = VPS_ISS_CAL_PIX_EXRCT_B10_MIPI;
                break;
            case SYSTEM_CSI2_RAW12:
                drvCalCfg->pixProcCfg[strmId].extract = VPS_ISS_CAL_PIX_EXRCT_B12_MIPI;
                break;
            case SYSTEM_CSI2_RAW14:
                drvCalCfg->pixProcCfg[strmId].extract = VPS_ISS_CAL_PIX_EXRCT_B14_MIPI;
                break;
            default:
                UTILS_assert(0);
                break;
        }
    }
}

Int32 IssCaptureLink_drvCreateDrv(IssCaptureLink_Obj *pObj)
{
    Int32 status = FVID2_SOK;

    pObj->drvHandle = Fvid2_create(
        FVID2_VPS_CAPT_VID_DRV,
        pObj->drvInstId,
        &pObj->drvCreatePrms,
        &pObj->drvCreateStatus,
        &pObj->drvCbPrms);

    if ((NULL == pObj->drvHandle) ||
        (pObj->drvCreateStatus.retVal != FVID2_SOK))
    {
        Vps_printf(" ISSCAPTURE: ERROR: FVID2 Create Failed !!!\n");
        status = pObj->drvCreateStatus.retVal;
    }

    if(status==FVID2_SOK)
    {
        status = Fvid2_control(pObj->drvHandle,
                                IOCTL_VPS_CAPT_SET_ISS_PARAMS,
                                &pObj->drvCalCfg, NULL);
        if (FVID2_SOK != status)
        {
            Vps_printf(
              " ISSCAPTURE: ERROR: IOCTL_VPS_CAPT_SET_ISS_PARAMS Failed !!!\n");
        }
    }

    return (status);
}

Int32 IssCaptureLink_drvDeleteDrv(IssCaptureLink_Obj *pObj)
{
    Int32   status = FVID2_SOK;
    UInt32  strmId;
    Fvid2_FrameList frmList;

    /* Dequeue all the request from the driver */
    for (strmId = 0U; strmId < pObj->createArgs.numCh; strmId++)
    {
        while (1U)
        {
            status = Fvid2_dequeue(
                pObj->drvHandle,
                &frmList,
                strmId,
                FVID2_TIMEOUT_NONE);
            if (FVID2_SOK != status)
            {
                break;
            }
        }
    }

    status = Fvid2_delete(pObj->drvHandle, NULL);
    if (FVID2_SOK != status)
    {
        Vps_printf(" ISSCAPTURE: ERROR: FVID2 Delete Failed !!!\n");
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief Create API for link.
 *
 *      Creates & configures the driver, allocates required memory to store the
 *      received frames and other infrasture required by capture link.
 *
 * \param  pObj     [IN] Capture link global handle
 * \param  pPrm     [IN] Capture link create parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvCreate(IssCaptureLink_Obj          *pObj,
                               IssCaptureLink_CreateParams *pPrm)
{
    Int32 status;
    System_LinkChInfo *pQueChInfo;
    UInt32 chIdx, pitchMul;

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Create in progress !!!\n");
#endif

    UTILS_assert(NULL != pPrm);
    UTILS_MEMLOG_USED_START();
    memcpy(&pObj->createArgs, pPrm, sizeof (*pPrm));

    System_resetLinkMemAllocInfo(&pObj->createArgs.memAllocInfo);

    status = Utils_bufCreate(&pObj->bufQue, FALSE, FALSE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    UTILS_assert(ISSCAPT_LINK_MAX_CH >= pPrm->numCh);
    pObj->info.numQue = ISSCAPTURE_LINK_MAX_OUT_QUE;
    pObj->info.queInfo[0U].numCh = pPrm->numCh;

    for (chIdx = 0U; chIdx < pPrm->numCh; chIdx++)
    {
        pitchMul = getPitchMultiplier(&pPrm->outParams[chIdx],
                                        pObj->createArgs.videoIfMode,
                                        pObj->createArgs.videoIfWidth);

        pQueChInfo = &pObj->info.queInfo[0U].chInfo[chIdx];
        pQueChInfo->width       = pPrm->outParams[chIdx].width;
        pQueChInfo->height      = pPrm->outParams[chIdx].height;
        pQueChInfo->startX      = 0;
        pQueChInfo->startY      = 0;
        pQueChInfo->pitch[0]    =
            SystemUtils_align(pObj->createArgs.outParams[chIdx].maxWidth *
                                    pitchMul, ISSCAPTURE_LINK_BUF_ALIGNMENT);
        pQueChInfo->pitch[1]    = 0;
        pQueChInfo->pitch[2]    = 0;
        SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(pQueChInfo->flags,
                    pObj->createArgs.outParams[chIdx].dataFormat);
        SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(pQueChInfo->flags,
                SYSTEM_SF_PROGRESSIVE);
    }

    IssCaptureLink_drvSetDrvCfg(pObj);
    IssCaptureLink_drvCreateDrv(pObj);
    IssCaptureLink_drvAllocFrames(pObj);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->linkId, "ISSCAPTURE");
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->isFirstFrameRecv = FALSE;

    System_assertLinkMemAllocOutOfMem(
        &pObj->createArgs.memAllocInfo,
        "ISSCAPTURE"
        );

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("ISSCAPTURE:",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));
#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Create Done !!!\n");
#endif
    pObj->info.queInfo[0U].numCh = pPrm->numCh;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Link callback for putting empty buffers into link input queue
 *
 *
 * \param  pObj           [IN] Capture link global handle
 * \param  pBufList       [IN] List of buffers to be kept back into link queue
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvPutEmptyBuffers(IssCaptureLink_Obj *pObj,
                                        System_BufferList  *pBufList)
{
    UInt32 idx;
    FVID2_FrameList frameList;
    FVID2_Frame *pFrame;
    System_Buffer *pBuf;
    Int32 status= SYSTEM_LINK_STATUS_SOK;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    linkStatsInfo->linkStats.putEmptyBufCount++;

    if (pBufList->numBuf != 0)
    {
        /*
         * Iterate through list of buffers provided from previous link.
         * Get FVID2_Frame out of bufferlist. Queue empty frames to the
         * driver queue for capturing new data
         */
        for (idx = 0; idx < pBufList->numBuf; idx++)
        {
            pBuf   = pBufList->buffers[idx];
            UTILS_assert(pBuf != NULL);

            pFrame = pBuf->pCaptureOrgBufferPtr;

            UTILS_assert(pFrame != NULL);

            frameList.frames[0] = pFrame;
            frameList.numFrames  = 1;

            linkStatsInfo->linkStats.chStats[
                pBuf->chNum].inBufRecvCount++;
            linkStatsInfo->linkStats.chStats[
                pBuf->chNum].inBufProcessCount++;

            status = FVID2_queue(pObj->drvHandle,
                                &frameList, pBuf->chNum);

            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to start the link.
 *
 * This function calls the driver function to start the driver. As a part of
 * this call iss capture hardware is ready to receive frames.
 *
 * \param  pObj           [IN] Iss Capture link global handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvStart(IssCaptureLink_Obj *pObj)
{
    Int32         status;

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Start in progress !!!\n");
#endif

    status = Fvid2_start(pObj->drvHandle, NULL);
    if (FVID2_SOK != status)
    {
        Vps_printf(" ISSCAPTURE: ERROR: FVID2 Start Failed !!!\n");
        return -1;
    }

    pObj->statsStartTime = Utils_getCurGlobalTimeInMsec();

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Start Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to stop driver and link.
 *
 * Iss capture hardware stops receiving frames after this call.
 *
 * \param  pObj         [IN] ISS Capture link object
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvStop(IssCaptureLink_Obj *pObj)
{
    Int32         status;

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Stop in progress !!!\n");
#endif

    status = Fvid2_stop(pObj->drvHandle, NULL);
    if (FVID2_SOK != status)
    {
        Vps_printf(" ISSCAPTURE: ERROR: FVID2 Stop Failed !!!\n");
        return -1;
    }

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Stop Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete iss capture link and driver handle.
 *
 *
 * \param  pObj         [IN] Capture link object
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvDelete(IssCaptureLink_Obj *pObj)
{
    Int32         status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Delete in progress !!!\n");
#endif

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status==0);

    IssCaptureLink_drvDeleteDrv(pObj);
    IssCaptureLink_drvFreeFrames(pObj);

#ifdef SYSTEM_DEBUG_CAPTURE
    Vps_printf(" ISSCAPTURE: Delete Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Print statistics like FPS, callback time etc.
 *
 *  \param pObj         [IN] Capture link object
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvPrintStatus(IssCaptureLink_Obj *pObj)
{
    UTILS_assert(NULL != pObj->linkStatsInfo);
    Utils_printLinkStatistics(
        &pObj->linkStatsInfo->linkStats, "ISSCAPTURE", TRUE);

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Print iss capture link buffer statistics
 *
 *  \param pObj         [IN] Capture link object
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvPrintBufferStatus(IssCaptureLink_Obj *pObj)
{
    Uint8 str[32];

    sprintf((char *) str, "ISSCAPTURE");
    Utils_bufPrintStatus(str, &pObj->bufQue);
    return 0;
}

/**
 *******************************************************************************
 *
 * \brief This function save a raw frame into a fixed location
 *
 * \param   pObj     [IN] Capture Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvSaveFrame(IssCaptureLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    if (pObj->createArgs.allocBufferForRawDump)
    {
        pObj->saveFrame = TRUE;

        status = SYSTEM_LINK_STATUS_SOK;
    }

    return (status);
}

/**
 *******************************************************************************
 *
 * \brief This function returns information about the saved raw frame
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_drvGetSaveFrameStatus(IssCaptureLink_Obj *pObj,
                    IssCaptureLink_GetSaveFrameStatus *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    pPrm->isSaveFrameComplete = FALSE;
    pPrm->bufAddr = 0;
    pPrm->bufSize = 0;

    if (pObj->createArgs.allocBufferForRawDump)
    {
        if(pObj->saveFrame == FALSE)
        {
            pPrm->isSaveFrameComplete = TRUE;
            pPrm->bufAddr = (UInt32)pObj->saveFrameBufAddr;
            pPrm->bufSize = pObj->drvCalCfg.inFmt[0].pitch[0] *
                            pObj->createArgs.outParams[0U].height;

        }

        status = SYSTEM_LINK_STATUS_SOK;
    }

    return (status);
}

/**
 *******************************************************************************
 * \brief This function detemines the required pitch.
 *          Does not support all data types and multiple planes.
 *
 * \return  A Positive non-zero number
 *******************************************************************************
 */
static UInt32 getPitchMultiplier(IssCaptureLink_OutParams *pOutPrm,
                                    System_VideoIfMode ifMode,
                                    System_VideoIfWidth ifWidth)
{
    UInt32 mulVal = 1U;

    UTILS_assert(NULL != pOutPrm);

    if (SYSTEM_VIFM_SCH_CPI == ifMode)
    {
        if (SYSTEM_VIFW_8BIT < ifWidth)
        {
            /* Requires 2 bytes per pixel */
            mulVal++;

            /* ISS dosent support more than 16b Parallel interface */
            if (SYSTEM_VIFW_16BIT < ifWidth)
            {
                mulVal++;
            }
        }
    }
    else if (SYSTEM_VIFM_SCH_CSI2 == ifMode)
    {
        switch (pOutPrm->inCsi2DataFormat)
        {
            case SYSTEM_CSI2_RAW12:
            case SYSTEM_CSI2_RAW10:
            case SYSTEM_CSI2_RAW14:
                mulVal = 2U;
            break;

            case SYSTEM_CSI2_RAW6:
            case SYSTEM_CSI2_RAW7:
            case SYSTEM_CSI2_RAW8:
                mulVal = 1U;
            break;

            case SYSTEM_CSI2_YUV422_8B:
            case SYSTEM_CSI2_RGB444:
            case SYSTEM_CSI2_RGB555:
            case SYSTEM_CSI2_RGB565:
                mulVal = 2U;
            break;

            case SYSTEM_CSI2_RGB666:
            case SYSTEM_CSI2_RGB888:
            case SYSTEM_CSI2_ANY:
                mulVal = 3U;
            break;

            default :
                /* Illegal data type */
                UTILS_assert(0);
            break;
        }
    }
    else
    {
        /* Interface not supported by ISS. */
        UTILS_assert(0);
    }

    return (mulVal);
}

