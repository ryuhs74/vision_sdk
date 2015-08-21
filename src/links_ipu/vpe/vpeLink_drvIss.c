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
 * \file vpeLink_drvIss.c
 *
 * \brief  This file has the implementation of VPE Link which uses ISS
 *         sub system as the processing core.
 *
 *   VPE Link can be used to do processing on video input frames. These
 *   frames may be from capture or decoded video frames coming over network.
 *
 *   VPE Link (Using ISS) can do
 *   - Color space conversion on input frames (422I -> 420SP)
 *   - Resizing of input frames
 *
 *     The VPE link receives the input frames, submits them into
 *     ISS driver along with a output frame to which the ISS driver
 *     writes the output. once the processing is over
 *     the driver invoke a call back. On call back VPE Link reclaim these
 *     frames which are already processed and send back to the previous link.
 *     Also send out the output frames to the next link
 *
 *
 *     VPE link also supports the run time input and output resolution
 *     change - This feature is NOT verified in this version.
 *
 * \version 0.0 (Sept 2013) : [SS] First version
 * \version 0.1 (Dec  2014) : [PS] Modified first version for ISS
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
 * \brief Function to create the isp params for given channel
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   chId     [IN] channel number
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreateIspPrms(VpeLink_Obj * pObj, UInt32 chId)
{
    VpeLink_ChObj *pChObj;
    System_LinkChInfo *pInChInfo;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    UTILS_assert(chId < pObj->inQueInfo.numCh);
    pChObj = &pObj->chObj[chId];

    pInChInfo = &pObj->inQueInfo.chInfo[chId];

    /* Set the ISP Params */
    pChObj->drvIspPrms.inFmt.width         = pInChInfo->width;
    pChObj->drvIspPrms.inFmt.height        = pInChInfo->height;
    pChObj->drvIspPrms.inFmt.pitch[0U]     = pInChInfo->pitch[0];
    pChObj->drvIspPrms.inFmt.pitch[1U]     = pInChInfo->pitch[1];
    pChObj->drvIspPrms.inFmt.pitch[2U]     = pInChInfo->pitch[2];
    pChObj->drvIspPrms.inFmt.bpp           = SYSTEM_BPP_BITS16; //For both 420SP and 422I
    pChObj->drvIspPrms.inFmt.dataFormat    =
        SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pInChInfo->flags);
    /* N_1 frame settings are redundant */
    pChObj->drvIspPrms.inFmtN_1.width      = pInChInfo->width;
    pChObj->drvIspPrms.inFmtN_1.height     = pInChInfo->height;
    pChObj->drvIspPrms.inFmtN_1.pitch[0U]  = pInChInfo->pitch[0];
    pChObj->drvIspPrms.inFmtN_1.bpp        = SYSTEM_BPP_BITS16; //For both 420SP and 422I
    pChObj->drvIspPrms.inFmtN_1.dataFormat =
        SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pInChInfo->flags);

    pChObj->drvIspPrms.enableWdrMerge            = FALSE;
    pChObj->drvIspPrms.enableVportCompInput      = FALSE;
    pChObj->drvIspPrms.enableDfs                 = FALSE;
    pChObj->drvIspPrms.glbcePath                 = VPS_ISS_GLBCE_PATH_DISABLED;
    pChObj->drvIspPrms.nsf3Path                  = VPS_ISS_NSF3_PATH_DISABLED;
    pChObj->drvIspPrms.enableDpcPreNsf3          = FALSE;
    pChObj->drvIspPrms.enableCnf                 = FALSE;
    pChObj->drvIspPrms.enableRszInputFromIpipeif = FALSE;

    pChObj->drvIspPrms.enableStreams[VPS_ISS_STREAM_CAL_RD_INPUT_0] = TRUE;
    pChObj->drvIspPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A]       = TRUE;
    pChObj->drvIspPrms.enableStreams[VPS_ISS_STREAM_ID_INPUT_N1]    = FALSE;
    pChObj->drvIspPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_B]       = FALSE;
    pChObj->drvIspPrms.enableStreams[VPS_ISS_STREAM_ID_AF]          = FALSE;
    pChObj->drvIspPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB]        = FALSE;

    pChObj->drvIspPrms.useWen = FALSE;
    pChObj->drvIspPrms.hdPol  = FVID2_POL_HIGH;
    pChObj->drvIspPrms.vdPol  = FVID2_POL_HIGH;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Function to create the resizer configuration parameters for given channel
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   chId     [IN] channel number
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreateRszCfgPrms(VpeLink_Obj * pObj, UInt32 chId)
{
    VpeLink_ChObj *pChObj;
    System_LinkChInfo *pInChInfo;
    System_LinkChInfo *pOutChInfo;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    vpsissRszCtrl_t rszCtrl;

    UTILS_assert(chId < pObj->inQueInfo.numCh);
    pChObj = &pObj->chObj[chId];

    pInChInfo  = &pObj->inQueInfo.chInfo[chId];
    pOutChInfo = &pObj->info.queInfo[VPE_LINK_OUT_QUE_ID_0].chInfo[chId];

    memset(&rszCtrl, 0x0, sizeof (vpsissRszCtrl_t));
    memset(&pChObj->drvRszCfg,
                  0x0,
                  sizeof (vpsissRszCfg_t));

    rszCtrl.module = VPS_ISS_RSZ_MODULE_RSZCFG;
    rszCtrl.rszCfg = &pChObj->drvRszCfg;

    status = Fvid2_control(
        pObj->fvidHandle,
        VPS_ISS_RSZ_IOCTL_GET_CONFIG,
        &rszCtrl,
        NULL);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pChObj->drvRszCfg.inCfg.opMode = VPS_ISS_RSZ_OP_MODE_RESIZING;
    pChObj->drvRszCfg.inCfg.procWin.cropStartX = 0U;
    pChObj->drvRszCfg.inCfg.procWin.cropStartY = 0U;
    pChObj->drvRszCfg.inCfg.procWin.cropWidth  = pInChInfo->width;
    pChObj->drvRszCfg.inCfg.procWin.cropHeight = pInChInfo->height;

    pChObj->drvRszCfg.instCfg[0U].enable = TRUE;
    pChObj->drvRszCfg.instCfg[0U].outFmt.dataFormat =
        SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pOutChInfo->flags);
    pChObj->drvRszCfg.instCfg[0U].outFmt.width      = pOutChInfo->width;
    pChObj->drvRszCfg.instCfg[0U].outFmt.height     = pOutChInfo->height;
    pChObj->drvRszCfg.instCfg[0U].outFmt.pitch[0U]  = pOutChInfo->pitch[0];
    pChObj->drvRszCfg.instCfg[0U].outFmt.pitch[1U]  = pOutChInfo->pitch[1];
    pChObj->drvRszCfg.instCfg[0U].outFmt.pitch[2U]  = pOutChInfo->pitch[2];

    pChObj->drvRszCfg.instCfg[0U].flipCtrl =
        VPS_ISS_RSZ_STR_MODE_NORMAL;
    pChObj->drvRszCfg.instCfg[0U].startPos.startX = 0U;
    pChObj->drvRszCfg.instCfg[0U].startPos.startY = 0U;
    pChObj->drvRszCfg.instCfg[0U].scaleMode       =
        VPS_ISS_RSZ_SCALE_MODE_NORMAL;
    pChObj->drvRszCfg.instCfg[0U].filtCfg.horzLumaFilter =
        VPS_ISS_RSZ_FILTER_4TAP_CUBIC;
    pChObj->drvRszCfg.instCfg[0U].filtCfg.vertLumaFilter =
        VPS_ISS_RSZ_FILTER_4TAP_CUBIC;
    pChObj->drvRszCfg.instCfg[0U].filtCfg.horzChromaFilter =
        VPS_ISS_RSZ_FILTER_4TAP_CUBIC;
    pChObj->drvRszCfg.instCfg[0U].filtCfg.vertChromaFilter =
        VPS_ISS_RSZ_FILTER_4TAP_CUBIC;

    pChObj->drvRszCfg.instCfg[0U].intensityCfg.horzLumaIntensity   = 21u;
    pChObj->drvRszCfg.instCfg[0U].intensityCfg.horzChromaIntensity = 22u;
    pChObj->drvRszCfg.instCfg[0U].intensityCfg.vertLumaIntensity   = 14u;
    pChObj->drvRszCfg.instCfg[0U].intensityCfg.vertChromaIntensity = 15u;

    pChObj->drvRszCfg.instCfg[1U].enable = FALSE;

    return status;

}

/**
 *******************************************************************************
 *
 * \brief Function to create the IPIPE Input parameters for given channel
 *
 * \param   pObj     [IN] VPE Link Instance handle
 * \param   chId     [IN] channel number
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvCreateIpipeInputPrms(VpeLink_Obj * pObj, UInt32 chId)
{
    vpsissIpipeCtrl_t ipipeCtrl;
    VpeLink_ChObj *pChObj;
    System_LinkChInfo *pInChInfo;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    UTILS_assert(chId < pObj->inQueInfo.numCh);
    pChObj = &pObj->chObj[chId];

    pInChInfo  = &pObj->inQueInfo.chInfo[chId];

    memset(&pChObj->drvIpipeInputCfg,
                    0x0,
                    sizeof (vpsissIpipeInConfig_t));
    memset(&ipipeCtrl, 0x0, sizeof (vpsissIpipeCtrl_t));

    ipipeCtrl.module = VPS_ISS_IPIPE_MODULE_INPUT;
    ipipeCtrl.inCfg  = &pChObj->drvIpipeInputCfg;

    status = Fvid2_control(
        pObj->fvidHandle,
        VPS_ISS_IPIPE_IOCTL_GET_CONFIG,
        &ipipeCtrl,
        NULL);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pChObj->drvIpipeInputCfg.dataPath = VPS_ISS_IPIPE_DATA_PATH_YUV422_YUV422; //TODO - Chk this

    pChObj->drvIpipeInputCfg.procWin.cropStartX = 0U;
    pChObj->drvIpipeInputCfg.procWin.cropStartY = 0U;
    pChObj->drvIpipeInputCfg.procWin.cropWidth  = pInChInfo->width;
    pChObj->drvIpipeInputCfg.procWin.cropHeight = pInChInfo->height;

    return status;

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

    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Function to create the VPE driver handle
 *
 *        Create the VPE link driver object/instance.
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
    FVID2_CbParams cbParams;
    Uint32 chId;

    Fvid2CbParams_init(&cbParams);
    cbParams.cbFxn    = VpeLink_drvFvidCb;
    cbParams.errCbFxn = VpeLink_drvFvidErrCb;
    cbParams.errList  = &pObj->errProcessList;
    cbParams.appData  = pObj;

    VpsM2mIntfCreateParams_init(&pObj->drvCreatePrms);
    pObj->drvCreatePrms.numCh           = 1U; // TODO check
    pObj->drvCreatePrms.chInQueueLength = 4; // TODO check - UTILS_ARRAYSIZE(pChInObj->frames);
    pObj->drvCreatePrms.maxStatsInst    = 0U;
    pObj->drvCreatePrms.pAdditionalArgs = (Ptr) & pObj->drvOpenPrms;
    pObj->isStopSupported = FALSE;

    VpsM2mIntfCreateStatus_init(&pObj->drvCreateStatusPrms);
    pObj->drvCreateStatusPrms.pAdditionalStatus = (Ptr) & pObj->drvRetPrms;

    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF] = TRUE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]    = FALSE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE]   = FALSE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]    = TRUE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE]   = TRUE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]     = TRUE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_CNF]     = FALSE;
    pObj->drvOpenPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]     = FALSE;
    pObj->drvOpenPrms.arg = NULL;

    pObj->fvidHandle = Fvid2_create(
        FVID2_VPS_COMMON_M2M_INTF_DRV,
        VPS_M2M_ISS_INST_CAL_ISP,
        &pObj->drvCreatePrms,
        &pObj->drvCreateStatusPrms,
        &cbParams);

    UTILS_assert(NULL != pObj->fvidHandle);

    /* To capture ISS Driver parameters at create time */
    for (chId = 0u; chId <  pObj->inQueInfo.numCh; chId++)
    {
        VpeLink_drvCreateIspPrms(pObj, chId);
        VpeLink_drvCreateRszCfgPrms(pObj, chId);
        VpeLink_drvCreateIpipeInputPrms(pObj, chId);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function frees up input buffers
 *
 *        If an input buffer (from previous link) and fvid input frame
 *        has been acquired and if processing cannot be done, then this
 *        function is invoked to free up input buffers.
 *
 *        This can happen in two scenarios -
 *        - Lack of output buffers
 *        - User has configured link to skip frames
 *
 * \param   pObj           [IN] VPE Link Instance handle
 * \param   pInFrame       [IN] FVID frame to be freed
 * \param   pInBuffer      [IN] Input buffer to be freed
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 VpeLink_drvFreeInputBuffer(VpeLink_Obj   *pObj,
                                  FVID2_Frame   *pInFrame,
                                  System_Buffer *pInBuffer)
{
    System_BufferList freeBufList;
    FVID2_FrameList freeFrameList;
    System_LinkInQueParams *pInQueParams;
    Int32 status;

    pInQueParams = &pObj->createArgs.inQueParams;

    freeFrameList.numFrames = 1;
    freeFrameList.frames[0] = pInFrame;
    VpeLink_drvFreeFvidFrames(pObj, &freeFrameList);

    freeBufList.numBuf = 1;
    freeBufList.buffers[0] = pInBuffer;
    status = System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                         pInQueParams->prevLinkQueId,
                                         &freeBufList);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

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
 * \param   chId           [IN] Channel Id to be processed
 * \param   inFrameList    [IN] input frame List
 * \param   outFrameList   [IN] output frame List
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
UInt32 VpeLink_drvMakeFrameLists(VpeLink_Obj * pObj,
                                UInt32 chId,
                                FVID2_FrameList * inFrameList,
                                FVID2_FrameList * outFrameList
                                )
{
    VpeLink_ChObj *pChObj;
    UInt32 outId, frameId;
    FVID2_Frame *pInFrame, *pOutFrame;
    System_Buffer *pInBuffer;
    System_Buffer *pOutBuf;
    Int32 status;
    Bool doFrameDrop;
    frameId = 0;
    System_VideoFrameBuffer *videoFrame;
    UInt32 flags;
    UInt32 *outQueIdArray;
    System_LinkStatistics *linkStatsInfo;

    pChObj = &pObj->chObj[chId];

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_bufGetFullBuffer(&pChObj->inQue, &pInBuffer, BSP_OSAL_NO_WAIT);

    if (pInBuffer != NULL)
    {
        status = Utils_queGet(&pChObj->inObj.fvidFrameQueue,
                              (Ptr *)&pInFrame,
                              1,
                              BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        status = Utils_bufInitFrame(pInFrame, pInBuffer);

        inFrameList->frames[VPS_ISS_STREAM_CAL_RD_INPUT_0] = pInFrame;

        VpeLink_drvUpdateInputRtPrm(pObj, pInFrame, chId);

        outId = VPE_LINK_OUT_QUE_ID_0;
        pOutBuf = NULL;
        pOutFrame = NULL;

        if ((pObj->createArgs.enableOut[outId]) &&
            (pChObj->enableOut[outId]) &&
            (Utils_queGetQueuedCount (&pObj->outObj[outId].emptyBufQue[chId])))
        {
            doFrameDrop = Utils_doSkipBuf(&(pChObj->frameSkipCtx[outId]));

            if(doFrameDrop == TRUE)
            {
                pOutFrame = &pObj->outFrameDrop;

                linkStatsInfo->linkStats.chStats
                            [pInFrame->chNum].outBufUserDropCount[outId]++;

                VpeLink_drvFreeInputBuffer(pObj, pInFrame, pInBuffer);
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
                    pChObj->chRtOutInfoUpdate[outId] = FALSE;
                    pChObj->chRtOutInfoUpdateWhileDrop = outId;
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

                pOutFrame->chNum = pInFrame->chNum;
                pOutFrame->timeStamp  = pInFrame->timeStamp;
                pOutFrame->fid = pInFrame->fid;

                outFrameList->frames[VPS_ISS_STREAM_ID_RSZ_A] = pOutFrame;
                outQueIdArray = outFrameList[0].appData;
                UTILS_assert(outQueIdArray != NULL);
                outQueIdArray[frameId] = outId;
                frameId++;

            }
        }
        else
        {
            linkStatsInfo->linkStats.chStats[
                pInFrame->chNum].outBufDropCount[outId]++;

            VpeLink_drvFreeInputBuffer(pObj, pInFrame, pInBuffer);
        }
    }

    inFrameList->numFrames = VPS_ISS_STREAM_ID_MAX;
    outFrameList->numFrames = VPS_ISS_STREAM_ID_MAX;

    return frameId;
}

/**
 *******************************************************************************
 *
 * \brief This function sets the ISS configuration for the given channel
 *
 * \param   pObj   [IN]  VPE Link Instance handle
 * \param   chId   [IN]  Channel Id to be configured
 *
 * \param   status [OUT] Return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvSetChannelIssConfig(VpeLink_Obj * pObj, UInt32 chId)
{
    Int32 status;
    VpeLink_ChObj *pChObj;
    vpsissRszCtrl_t rszCtrl;
    vpsissIpipeCtrl_t ipipeCtrl;
    vpsissIsifCtrl_t isifCtrl;

    pChObj = &pObj->chObj[chId];

    status = Fvid2_control(
        pObj->fvidHandle,
        IOCTL_VPS_ISS_M2M_SET_ISP_PARAMS,
        &pChObj->drvIspPrms,
        NULL);

    UTILS_assert(status == FVID2_SOK);

    rszCtrl.module = VPS_ISS_RSZ_MODULE_RSZCFG;
    rszCtrl.rszCfg = &pChObj->drvRszCfg;

    status = Fvid2_control(
        pObj->fvidHandle,
        VPS_ISS_RSZ_IOCTL_SET_CONFIG,
        &rszCtrl,
        NULL);

    UTILS_assert(status == FVID2_SOK);

    ipipeCtrl.module = VPS_ISS_IPIPE_MODULE_INPUT;
    ipipeCtrl.inCfg  = &pChObj->drvIpipeInputCfg;

    status = Fvid2_control(
        pObj->fvidHandle,
        VPS_ISS_IPIPE_IOCTL_SET_CONFIG,
        &ipipeCtrl,
        NULL);

    UTILS_assert(status == FVID2_SOK);

    pChObj->isifBlkClampCfg.dcOffset = 0x0;

    isifCtrl.module = VPS_ISS_ISIF_MODULE_BLACK_CLAMP;
    isifCtrl.blkClampCfg  = &pChObj->isifBlkClampCfg;
    status = Fvid2_control(
        pObj->fvidHandle,
        VPS_ISS_ISIF_IOCTL_SET_CONFIG,
        &isifCtrl,
        NULL);

    UTILS_assert(status == FVID2_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function process all input frames for the given channel
 *
 * \param   pObj   [IN]  VPE Link Instance handle
 *
 * \param   status [OUT] Return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 VpeLink_drvProcessChannelData(VpeLink_Obj * pObj, UInt32 chId)
{
    VpeLink_ReqObj *pReqObj;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    pReqObj = &pObj->reqObj;

    pReqObj->outFrameList.appData = &pReqObj->outListQueIdMap[0];
    pReqObj->processList.inFrameList[0] = &pReqObj->inFrameList;
    pReqObj->processList.outFrameList[0] = &pReqObj->outFrameList;
    pReqObj->processList.numInLists = 1;
    pReqObj->processList.numOutLists = 1;

    pReqObj->inFrameList.appData = pReqObj;


    /*
     * This loop iterates until input buffers, to be processed are present.
     * Each iteration handles one input frame.
     */
    while(1)
    {
        UInt32 numFrames;


        /* Create frame list to be submitted to ISS driver*/
        numFrames = VpeLink_drvMakeFrameLists(pObj,
                                  chId,
                                  &pReqObj->inFrameList,
                                  &pReqObj->outFrameList
                                 );

        if (numFrames == 0)
            break;

        pObj->givenInFrames += pReqObj->inFrameList.numFrames;

        Utils_pendIspLock();

        VpeLink_drvSetChannelIssConfig(pObj, chId);

        status = Fvid2_processRequest(
            pObj->fvidHandle,
            &pReqObj->inFrameList,
            &pReqObj->outFrameList);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        BspOsal_semWait(pObj->complete, BSP_OSAL_WAIT_FOREVER);

        status = Fvid2_getProcessedRequest(
            pObj->fvidHandle,
            &pReqObj->inFrameList,
            &pReqObj->outFrameList,
            BSP_OSAL_NO_WAIT);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        Utils_postIspLock();

        VpeLink_drvReleaseFrames(pObj, &pReqObj->inFrameList,
                             &pReqObj->outFrameList);

    }

    return status;
}



/**
 *******************************************************************************
 *
 * \brief This function submit the job to the ISS driver to perform resizing
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

    UInt32 chId;
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

    for (chId = 0; chId < pObj->inQueInfo.numCh; chId++)
    {
        VpeLink_drvProcessChannelData(pObj, chId);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

