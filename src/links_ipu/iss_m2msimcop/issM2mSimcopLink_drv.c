/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "issM2mSimcopLink_priv.h"


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *******************************************************************************
 *
 * \brief This function is the call back function, which gets called when
 *        ISS completes processing and interrupts driver
 *
 * \param  handle            [IN] FVID handle of buffer which was operated upon
 * \param  appData           [IN] App object
 * \param  reserved          [IN]
 *
 * \return  FVID2_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvCallBack(Fvid2_Handle handle,
                                       Ptr appData,
                                       Ptr reserved)
{
    IssM2mSimcopLink_Obj *pObj;

    pObj = (IssM2mSimcopLink_Obj *)appData;

    if (NULL == pObj)
    {
        return (FVID2_EFAIL);
    }

    BspOsal_semPost(pObj->drvSemProcessComplete);

    return (FVID2_SOK);

}

Int32 IssM2mSimcopLink_drvCreateDrv(IssM2mSimcopLink_Obj *pObj,
                                    UInt32             chId)
{
    UInt32 status = SYSTEM_LINK_STATUS_SOK;
    vpsissSimcopOpenParams_t    drvSimcopOpenParams;
    Vps_M2mIntfCreateParams     drvCreatePrms;
    Fvid2_CbParams              drvCbPrms;
    Vps_M2mIntfCreateStatus     drvCreateStatusPrms;
    IssM2mSimcopLink_ConfigParams drvConfig;
    vpsissldcConfig_t           ldcCfg;
    vpsissvtnfConfig_t          vtnfCfg;

    IssM2mSimcopLink_ChObj *pChObj;

    pChObj = &pObj->chObj[chId];

    VpsM2mIntfCreateParams_init(&drvCreatePrms);
    VpsM2mIntfCreateStatus_init(&drvCreateStatusPrms);
    Fvid2CbParams_init(&drvCbPrms);

    /* set default driver config */
    IssM2mSimcopLink_ConfigParams_Init(&drvConfig);

    vpsissVtnfCfg_init(&vtnfCfg);
    vpsissLdcCfg_init(&ldcCfg);

    drvConfig.ldcConfig = &ldcCfg;
    drvConfig.vtnfConfig = &vtnfCfg;

    drvConfig.chNum = chId;
    IssM2mSimcopLink_drvSetSimcopConfig(pObj, &drvConfig);

    drvCbPrms.cbFxn   = IssM2mSimcopLink_drvCallBack;
    drvCbPrms.appData = pObj;

    drvCreatePrms.numCh           = 1U;
    drvCreatePrms.chInQueueLength = 1U;
    drvCreatePrms.pAdditionalArgs = (Ptr)&drvSimcopOpenParams;

    if(pObj->createArgs.channelParams[chId].operatingMode
        ==
       ISSM2MSIMCOP_LINK_OPMODE_LDC
    )
    {
        drvSimcopOpenParams.mode = VPS_ISS_SIMCOP_LDC;
    }
    else
    if(pObj->createArgs.channelParams[chId].operatingMode
        ==
       ISSM2MSIMCOP_LINK_OPMODE_VTNF
    )
    {
        drvSimcopOpenParams.mode = VPS_ISS_SIMCOP_VTNF;
    }
    else
    if(pObj->createArgs.channelParams[chId].operatingMode
        ==
       ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF
    )
    {
        drvSimcopOpenParams.mode = VPS_ISS_SIMCOP_LDC_VTNF;
    }
    else
    {
        drvSimcopOpenParams.mode = VPS_ISS_SIMCOP_LDC_VTNF;
    }

    drvSimcopOpenParams.arg = NULL;

    pChObj->drvHandle = Fvid2_create(
                            FVID2_VPS_COMMON_M2M_INTF_DRV,
                            VPS_M2M_ISS_INST_SIMCOP,
                            &drvCreatePrms,
                            &drvCreateStatusPrms,
                            &drvCbPrms);

    if(NULL == pChObj->drvHandle)
    {
        Vps_printf(" ISSM2MSIMCOP: CH%d: FVID2 Create failed !!!",
                    chId);
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Allocate frame buffers and do the necessary initializations
 *
 *  \param pObj   [IN] link obj
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvAllocBuffer(IssM2mSimcopLink_Obj * pObj, UInt32 chId)
{
    UInt32 numFrames;
    UInt32 frameIdx, outHeight, outPitch[2];
    Int32 status;
    System_Buffer *pSystemBuffer;
    System_VideoFrameBuffer *pSystemVideoFrameBuffer;
    IssM2mSimcopLink_CreateParams * pPrm;
    System_LinkChInfo *pChInfo;
    IssM2mSimcopLink_ChObj *pChObj;
    Utils_DmaChCreateParams dmaParams;

    pChObj = &pObj->chObj[chId];
    pChInfo = &pObj->linkInfo.queInfo[0].chInfo[chId];

    pPrm = &pObj->createArgs;

    pChObj->pPrevOutBuffer = NULL;

    if(pPrm->channelParams[chId].numBuffersPerCh
        >
        ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH
        )
    {
        pPrm->channelParams[chId].numBuffersPerCh
            =
            ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH;
    }

    status = Utils_queCreate(
                    &pChObj->emptyBufQue,
                    ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH,
                    pChObj->emptyBufsMem,
                    UTILS_QUE_FLAG_NO_BLOCK_QUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    numFrames  = pPrm->channelParams[chId].numBuffersPerCh;

    outHeight   = pChInfo->height;
    outPitch[0] = pChInfo->pitch[0];
    outPitch[1] = pChInfo->pitch[1];

    /* driver assumes outPitch[0] == outPitch[1] */
    UTILS_assert(outPitch[0]==outPitch[1]);

    pChObj->outBufSize  = outHeight*outPitch[0]+outHeight/2*outPitch[1];

    for(frameIdx = 0; frameIdx < numFrames; frameIdx++)
    {
        pSystemBuffer           = &pChObj->buffers[frameIdx];
        pSystemVideoFrameBuffer = &pChObj->videoFrames[frameIdx];

        /*
        * Properties of pSystemBuffer
        */
        pSystemBuffer->payload     = pSystemVideoFrameBuffer;
        pSystemBuffer->payloadSize = sizeof(System_VideoFrameBuffer);
        pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
        pSystemBuffer->chNum       = chId;

        if(SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(
                    pChInfo->flags
                )
                != SYSTEM_DF_YUV420SP_UV
           )
        {
            /* only YUV420 input format suported as of now */
            UTILS_assert(NULL);
        }

        pSystemVideoFrameBuffer->chInfo
            =
                pObj->linkInfo.queInfo[0].chInfo[chId];

        if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
        {
            pSystemVideoFrameBuffer->bufAddr[0] = Utils_memAlloc(
                                            UTILS_HEAPID_DDR_CACHED_SR,
                                            pChObj->outBufSize,
                                            SYSTEM_BUFFER_ALIGNMENT);
        }
        else
        {
            pSystemVideoFrameBuffer->bufAddr[0] =
                                     System_allocLinkMemAllocInfo(
                                            &pObj->createArgs.memAllocInfo,
                                            pChObj->outBufSize,
                                            SYSTEM_BUFFER_ALIGNMENT);
        }
        UTILS_assert(pSystemVideoFrameBuffer->bufAddr[0] != NULL);

        /*
         * Carving out memory pointer for chroma which will get used in case of
         * SYSTEM_DF_YUV422SP_UV
         */
        pSystemVideoFrameBuffer->bufAddr[1] = (void*)(
             (UInt32) pSystemVideoFrameBuffer->bufAddr[0] +
                        outHeight*outPitch[0]
             );

        status = Utils_quePut(&pChObj->emptyBufQue,
                                pSystemBuffer,
                                BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    /* Allocate Extra frame for saving captured frame */
    if (pObj->createArgs.allocBufferForDump)
    {
        if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
        {
            pChObj->saveFrameBufAddr =
                Utils_memAlloc(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    pChObj->outBufSize,
                    SYSTEM_BUFFER_ALIGNMENT);
        }
        else
        {
            pChObj->saveFrameBufAddr =
                    System_allocLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo,
                            pChObj->outBufSize,
                            SYSTEM_BUFFER_ALIGNMENT);
        }
        UTILS_assert(pChObj->saveFrameBufAddr != NULL);

        /* Initialize DMA parameters and create object for Frame Dumping */
        Utils_DmaChCreateParams_Init(&dmaParams);
        status = Utils_dmaCreateCh(
                        &pChObj->dumpFramesDmaObj,
                        &dmaParams);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        pChObj->saveFrame = 0xFF;
    }

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Create API for link. Link gets created using this function.
 *
 *      Handles all link creation time functionality.
 *
 * \param  pObj     [IN] Link global handle
 * \param  pPrm     [IN] Link create parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvCreate(IssM2mSimcopLink_Obj          * pObj,
                              IssM2mSimcopLink_CreateParams * pPrm)
{
    Int32 status;
    UInt32 chId, numChannels;

#ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MSIMCOP: Create in progress !!!\n");
#endif

    UTILS_MEMLOG_USED_START();

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    System_resetLinkMemAllocInfo(&pObj->createArgs.memAllocInfo);

    status = System_linkGetInfo(pObj->createArgs.inQueParams.prevLinkId,
                                &pObj->prevLinkInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(pObj->createArgs.inQueParams.prevLinkQueId <
                 pObj->prevLinkInfo.numQue);

    numChannels = pObj->prevLinkInfo.queInfo
                        [pObj->createArgs.inQueParams.prevLinkQueId].numCh;

    UTILS_assert(numChannels<=ISSM2MSIMCOP_LINK_MAX_CH);

    status = Utils_queCreate(&pObj->fullBufQue,
                              ISSM2MSIMCOP_LINK_MAX_FRAMES,
                              pObj->fullBufsMem,
                              UTILS_QUE_FLAG_NO_BLOCK_QUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /*
     * Population of link info, which will be queried by successor link
     */
    pObj->linkInfo.numQue = 1;
    pObj->linkInfo.queInfo[0]
        =
        pObj->prevLinkInfo.queInfo[pObj->createArgs.inQueParams.prevLinkQueId];

    for (chId=0; chId < numChannels; chId++)
    {
        IssM2mSimcopLink_drvAllocBuffer(pObj, chId);
        status = IssM2mSimcopLink_drvCreateDrv(pObj, chId);
        UTILS_assert(status==0);
    }

    pObj->drvSemProcessComplete = BspOsal_semCreate(0U, TRUE);
    UTILS_assert(pObj->drvSemProcessComplete != NULL);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->linkId, "ISSM2MSIMCOP");
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->isFirstFrameRecv = FALSE;

    System_assertLinkMemAllocOutOfMem(
        &pObj->createArgs.memAllocInfo,
        "ISSM2MSIMCOP"
        );

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("ISSM2MSIMCOP:",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));
#ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MSIMCOP: Create Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete ISS M2M SIMCOP Link and driver handle.
 *
 *
 * \param  pObj         [IN] Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvDelete(IssM2mSimcopLink_Obj * pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_VideoFrameBuffer *pSystemVideoFrameBuffer;
    UInt32 chId, frameIdx;
    IssM2mSimcopLink_ChObj * pChObj;

#ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MSIMCOP: Delete in progress !!!\n");
#endif

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status==0);

    for(chId=0; chId<pObj->linkInfo.queInfo[0].numCh; chId++)
    {
        pChObj = &pObj->chObj[chId];

        status = Utils_queDelete(&pChObj->emptyBufQue);
        UTILS_assert(status==0);

        for(frameIdx = 0;
            frameIdx < pObj->createArgs.channelParams[chId].numBuffersPerCh;
            frameIdx++)
        {
            pSystemVideoFrameBuffer = &pChObj->videoFrames[frameIdx];

            if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
            {
                status = Utils_memFree(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        pSystemVideoFrameBuffer->bufAddr[0],
                        pChObj->outBufSize
                     );
                UTILS_assert(status==0);
            }
        }

        status = Fvid2_delete(pChObj->drvHandle, NULL);
        UTILS_assert(status==0);

        /* Free up Extra frame for saving captured frame */
        if (pObj->createArgs.allocBufferForDump)
        {
            /* Initialize this flag to 0 so that it can't be used */
            pChObj->saveFrame = 0xFF;

            if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
            {
                /* Free up the extra buffer memory space */
                status = Utils_memFree(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        pChObj->saveFrameBufAddr,
                        pChObj->outBufSize);

                UTILS_assert(status==0);
            }
            pChObj->saveFrameBufAddr = NULL;

            /* Free up the DMA channel object */
            Utils_dmaDeleteCh(&pChObj->dumpFramesDmaObj);
        }
    }

    BspOsal_semDelete(&pObj->drvSemProcessComplete);

#ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MSIMCOP: Delete Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief API for setting SIMCOP parameters for the link.
 *
 *  In this function, configuration provided by the use case is just recorded
 *  inside the link. Providing this to driver happens before process call.
 *
 * \param  pObj        [IN] Link global handle
 * \param  pCfgPrm     [IN] SIMCOP configuration parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvSetSimcopConfig(
        IssM2mSimcopLink_Obj             *pObj,
        IssM2mSimcopLink_ConfigParams *pCfg)
{
    IssM2mSimcopLink_ChObj *pChObj;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_LinkChInfo *pChInfo;
    UInt32 chId, i;

    chId = pCfg->chNum;

    if(chId < pObj->linkInfo.queInfo[0].numCh)
    {
        pChObj = &pObj->chObj[chId];
        pChInfo = &pObj->linkInfo.queInfo[0].chInfo[chId];

        if (NULL != pCfg->ldcConfig)
        {
            memcpy(&pChObj->drvSimcopCfg.ldcCfg,
                         pCfg->ldcConfig,
                         sizeof(vpsissldcConfig_t));
        }
        if (NULL != pCfg->vtnfConfig)
        {
            memcpy(&pChObj->drvSimcopCfg.vtnfCfg,
                         pCfg->vtnfConfig,
                         sizeof(vpsissvtnfConfig_t));
        }

        /* over-ride user set values with values that can be
         * derived by the link
         */
        pChObj->drvSimcopCfg.inpFrmPitch        = pChInfo->pitch[0];
        pChObj->drvSimcopCfg.prevFrmPitch       = pChInfo->pitch[0];
        pChObj->drvSimcopCfg.outFrmPitch        = pChInfo->pitch[0];
        pChObj->drvSimcopCfg.inFrameWidth       = pChInfo->width;
        pChObj->drvSimcopCfg.inFrameHeight      = pChInfo->height;
        pChObj->drvSimcopCfg.cropCfg.cropStartX = 0;
        pChObj->drvSimcopCfg.cropCfg.cropStartY = 0;
        pChObj->drvSimcopCfg.cropCfg.cropWidth  = pChInfo->width;
        pChObj->drvSimcopCfg.cropCfg.cropHeight = pChInfo->height;
        pChObj->drvSimcopCfg.arg                = NULL;

        if (NULL != pCfg->ldcConfig)
        {
            if (FALSE == pCfg->ldcConfig->isAdvCfgValid)
            {
                /* auto-calc block WxH */

                /* by default assume 16x4, increase upto 32x36 depending on input WxH */
                pChObj->drvSimcopCfg.blkWidth      = 16;
                pChObj->drvSimcopCfg.blkHeight     = 4;

                for(i=32; i>=16; i-=16)
                {
                    if((pChInfo->width%i)==0)
                    {
                        pChObj->drvSimcopCfg.blkWidth = i;
                        break;
                    }
                }
                for(i=36; i>=4; i-=4)
                {
                    if((pChInfo->height%i)==0)
                    {
                        pChObj->drvSimcopCfg.blkHeight = i;
                        break;
                    }
                }
            }
            else
            {
                pChObj->drvSimcopCfg.blkWidth =
                    pCfg->ldcConfig->advCfg.outputBlockWidth;
                pChObj->drvSimcopCfg.blkHeight =
                    pCfg->ldcConfig->advCfg.outputBlockHeight;
            }
        }

        pChObj->drvSimcopCfg.ldcCfg.inputFrameWidth  = pChInfo->width;
        pChObj->drvSimcopCfg.ldcCfg.inputFrameHeight = pChInfo->height;
        pChObj->drvSimcopCfg.ldcCfg.isAdvCfgValid    = TRUE;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.outputBlockWidth
                = pChObj->drvSimcopCfg.blkWidth;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.outputBlockHeight
                = pChObj->drvSimcopCfg.blkHeight;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.outputFrameWidth = pChInfo->width;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.outputFrameHeight = pChInfo->height;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.outputStartX = 0;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.outputStartY = 0;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.enableCircAddrMode = FALSE;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.circBuffSize = 0;
        pChObj->drvSimcopCfg.ldcCfg.advCfg.enableConstOutAddr = FALSE;

        pChObj->drvSimcopCfg.vtnfCfg.outDataFormat = SYSTEM_DF_YUV420SP_UV;
        pChObj->drvSimcopCfg.vtnfCfg.isAdvCfgValid = TRUE;
        pChObj->drvSimcopCfg.vtnfCfg.advCfg.blockWidth
            = pChObj->drvSimcopCfg.blkWidth;
        pChObj->drvSimcopCfg.vtnfCfg.advCfg.blockHeight
            = pChObj->drvSimcopCfg.blkHeight;
        pChObj->drvSimcopCfg.vtnfCfg.advCfg.triggerSource
            = ISSHAL_VTNF_TRG_SRC_HWSEQ;
        pChObj->drvSimcopCfg.vtnfCfg.advCfg.intrEnable      = TRUE;


    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Process the given frame
 *
 * This function performs the actual driver call to process a given frame
 *
 * \param  pObj                 [IN]
 * \param  pInputBuffer         [IN]
 * \param  pOutputImageBuffer   [IN]
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvProcessFrame(IssM2mSimcopLink_Obj * pObj,
                                    UInt32 chId,
                                    System_Buffer     * pInputBuffer,
                                    System_Buffer     * pOutputBuffer)

{
    Fvid2_Frame                 inputMeshTableFrame;
    Fvid2_Frame                 inputFrame;
    Fvid2_Frame                 prevOutputFrame;
    Fvid2_Frame                 outputFrame;
    Fvid2_FrameList             inFrmList;
    Fvid2_FrameList             outFrmList;

    System_VideoFrameBuffer *pVideoInFrame;
    System_VideoFrameBuffer *pVideoOutFrame;
    System_VideoFrameBuffer *pVideoPrevOutFrame;
    Int32 status;
    IssM2mSimcopLink_ChObj *pChObj;

    chId = pInputBuffer->chNum;
    pChObj = &pObj->chObj[chId];

    pVideoInFrame  = (System_VideoFrameBuffer*)pInputBuffer->payload;
    pVideoOutFrame = (System_VideoFrameBuffer*)pOutputBuffer->payload;

    if(pChObj->pPrevOutBuffer!=NULL)
    {
        pVideoPrevOutFrame = (System_VideoFrameBuffer*)
                                pChObj->pPrevOutBuffer->payload;
    }
    else
    {
        pVideoPrevOutFrame = (System_VideoFrameBuffer*)pInputBuffer->payload;
    }

    inputMeshTableFrame.chNum      = 0;
    inputFrame.chNum               = 0;
    outputFrame.chNum              = 0;
    prevOutputFrame.chNum          = 0;

    inputMeshTableFrame.addr[0][0] = (Ptr)pChObj->drvSimcopCfg.ldcCfg.lutCfg.address;

    inputFrame.addr[0][0] = pVideoInFrame->bufAddr[0];
    inputFrame.addr[0][1] = pVideoInFrame->bufAddr[1];

    outputFrame.addr[0][0] = pVideoOutFrame->bufAddr[0];
    outputFrame.addr[0][1] = pVideoOutFrame->bufAddr[1];

    pVideoOutFrame->metaBufAddr = pVideoInFrame->metaBufAddr;
    pVideoOutFrame->metaBufSize = pVideoInFrame->metaBufSize;
    pVideoOutFrame->metaFillLength = pVideoInFrame->metaFillLength;

    prevOutputFrame.addr[0][0] = pVideoPrevOutFrame->bufAddr[0];
    prevOutputFrame.addr[0][1] = pVideoPrevOutFrame->bufAddr[1];

    pChObj->pPrevOutBuffer = pOutputBuffer;

    inFrmList.frames[VPS_SIMCOP_STREAM_ID_CUR_FRAME]  = &inputFrame;
    inFrmList.frames[VPS_SIMCOP_STREAM_ID_PREV_FRAME] = &prevOutputFrame;
    inFrmList.frames[VPS_SIMCOP_STREAM_ID_MESH_TABLE] = &inputMeshTableFrame;
    outFrmList.frames[VPS_SIMCOP_STREAM_ID_OUT_FRAME] = &outputFrame;

    inFrmList.numFrames = VPS_SIMCOP_STREAM_ID_MAX + 1;
    outFrmList.numFrames = VPS_SIMCOP_STREAM_ID_MAX + 1;

    Utils_pendIspLock();

    status = Fvid2_control(
                pChObj->drvHandle,
                IOCTL_VPS_SIMCOP_M2M_SET_PARAMS,
                &pChObj->drvSimcopCfg,
                NULL);

    UTILS_assert(status==0);

    /*
     * Submit Processing Request to the driver, wait on a semaphore and
     * get processed frame
     */
    status = Fvid2_processRequest(
                pChObj->drvHandle,
                &inFrmList,
                &outFrmList);

    UTILS_assert(status==0);

    BspOsal_semWait(pObj->drvSemProcessComplete, BSP_OSAL_WAIT_FOREVER);

    status = Fvid2_getProcessedRequest(
                pChObj->drvHandle,
                &inFrmList,
                &outFrmList,
                FVID2_TIMEOUT_NONE);

    UTILS_assert(status==0);

    Utils_postIspLock();

    if ((TRUE == pObj->createArgs.allocBufferForDump) &&
        (TRUE == pChObj->saveFrame))
    {
        Utils_DmaCopyFill2D dmaPrm;
        System_LinkChInfo *pChInfo;

        pChInfo = &pObj->linkInfo.queInfo[0].chInfo[chId];

        if (pObj->createArgs.channelParams[chId].operatingMode !=
            ISSM2MSIMCOP_LINK_OPMODE_LDC)
        {
            /* VTNF supports only YUV420 output format*/
            dmaPrm.dataFormat = SYSTEM_DF_YUV420SP_UV;
        }
        else
        {
            if (pChObj->drvSimcopCfg.ldcCfg.mode == VPS_ISS_LDC_MODE_YUV420_LDC)
            {
                dmaPrm.dataFormat = SYSTEM_DF_YUV420SP_UV;
            }
            else
            {
                dmaPrm.dataFormat = SYSTEM_DF_RAW16;
            }
        }

        dmaPrm.destAddr[0]  = pChObj->saveFrameBufAddr;
        dmaPrm.destAddr[1]  = pChObj->saveFrameBufAddr +
                                pChInfo->pitch[0] * pChInfo->height;
        dmaPrm.destPitch[0] = pChInfo->pitch[0];
        dmaPrm.destPitch[1] = pChInfo->pitch[1];
        dmaPrm.destStartX   = 0;
        dmaPrm.destStartY   = 0;
        dmaPrm.width        = pChInfo->width;
        dmaPrm.height       = pChInfo->height;
        dmaPrm.srcAddr[0]   = pVideoOutFrame->bufAddr[0];
        dmaPrm.srcAddr[1]   = pVideoOutFrame->bufAddr[1];
        dmaPrm.srcPitch[0]  = pChInfo->pitch[0];
        dmaPrm.srcPitch[1]  = pChInfo->pitch[1];
        dmaPrm.srcStartX    = 0;
        dmaPrm.srcStartY    = 0;

        status = Utils_dmaCopy2D(&pChObj->dumpFramesDmaObj, &dmaPrm, 1);
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

        /* Reset the flag */
        pChObj->saveFrame = FALSE;

        Vps_printf(" **************************************************** \n");
        Vps_printf(" ####### Save Frame from location 0x%x ####### \n",
            pChObj->saveFrameBufAddr);
        if (SYSTEM_DF_YUV420SP_UV == dmaPrm.dataFormat)
        {
            Vps_printf(" saveRaw(0, 0x%x, filename, %d, 32, false); ",
                pChObj->saveFrameBufAddr,
                pChInfo->pitch[0] *
                pChInfo->height * 3 / 8);
        }
        else
        {
            Vps_printf(" saveRaw(0, 0x%x, filename, %d, 32, false); ",
                pChObj->saveFrameBufAddr,
                pChInfo->pitch[0] *
                pChInfo->height / 4);
        }
        Vps_printf(" **************************************************** \n");
    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Process the new input data
 *
 * This function gets called in response to SYSTEM_CMD_NEW_DATA command from
 * previous link.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_drvProcessData(IssM2mSimcopLink_Obj * pObj)
{
    UInt32 bufId;
    System_LinkInQueParams *pInQueParams;
    System_BufferList bufList;
    Int32 status;
    UInt32 chId;
    System_Buffer *pInputBuffer;
    System_Buffer *pOutputBuffer;
    IssM2mSimcopLink_ChObj *pChObj;
    Bool sendCmdToNextLink = FALSE;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    if(pObj->isFirstFrameRecv==FALSE)
    {
        pObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(
                &linkStatsInfo->linkStats,
                pObj->linkInfo.queInfo[0].numCh,
                1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    pInQueParams = &pObj->createArgs.inQueParams;

    System_getLinksFullBuffers(pInQueParams->prevLinkId,
                               pInQueParams->prevLinkQueId, &bufList);

    if (bufList.numBuf)
    {
        for (bufId = 0; bufId < bufList.numBuf; bufId++)
        {
            pInputBuffer = bufList.buffers[bufId];

            chId = pInputBuffer->chNum;

            /*
             * Checks on sanity of input buffer
             */
            if (chId >= pObj->linkInfo.queInfo[0].numCh)
            {
                linkStatsInfo->linkStats.inBufErrorCount++;
                continue;
            }

            linkStatsInfo->linkStats.chStats[chId].inBufRecvCount++;

            pChObj = &pObj->chObj[chId];

            status = Utils_queGet(&pChObj->emptyBufQue,
                                  (Ptr *) &pOutputBuffer,
                                  1,
                                  BSP_OSAL_NO_WAIT);

            if (status != SYSTEM_LINK_STATUS_SOK)
            {
                linkStatsInfo->linkStats.chStats[chId].inBufDropCount++;
                continue;
            }

            pOutputBuffer->srcTimestamp = pInputBuffer->srcTimestamp;
            pOutputBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

            /*
             * Reaching here means an error free input and empty output buffers
             * are available
             */
            IssM2mSimcopLink_drvProcessFrame(
                                    pObj,
                                    chId,
                                    pInputBuffer,
                                    pOutputBuffer
                                  );

            Utils_updateLatency(&linkStatsInfo->linkLatency,
                              pOutputBuffer->linkLocalTimestamp);
            Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                              pOutputBuffer->srcTimestamp);

            linkStatsInfo->linkStats.chStats[chId].inBufProcessCount++;
            linkStatsInfo->linkStats.chStats[chId].outBufCount[0]++;

            status = Utils_quePut(
                          &pObj->fullBufQue,
                          pOutputBuffer,
                          BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            sendCmdToNextLink = TRUE;
        }

        System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                    pInQueParams->prevLinkQueId,
                                    &bufList);

        if(sendCmdToNextLink)
        {
            /*
             * Send command to next link for putting the buffer in output
             * queue of the buffer
             */
            System_sendLinkCmd(pObj->createArgs.outQueParams.nextLink,
                               SYSTEM_CMD_NEW_DATA,
                               NULL);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
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
Int32 IssM2mSimcopLink_drvPrintStatus(IssM2mSimcopLink_Obj * pObj)
{
    UTILS_assert(NULL != pObj->linkStatsInfo);

    Utils_printLinkStatistics(&pObj->linkStatsInfo->linkStats, "ISSM2MSIMCOP", TRUE);

    Utils_printLatency("ISSM2MSIMCOP",
                       &pObj->linkStatsInfo->linkLatency,
                       &pObj->linkStatsInfo->srcToLinkLatency,
                        TRUE);

    return 0;
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
Int32 IssM2mSimcopLink_drvGetSaveFrameStatus(IssM2mSimcopLink_Obj *pObj,
                    IssM2mSimcopLink_GetSaveFrameStatus *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;
    IssM2mSimcopLink_ChObj *pChObj;
    UInt32 isYuv420Fmt;
    UInt32 chId;

    UTILS_assert(NULL != pObj);
    UTILS_assert(NULL != pPrm);

    chId = pPrm->chId;
    UTILS_assert(chId < pObj->prevLinkInfo.queInfo
                        [pObj->createArgs.inQueParams.prevLinkQueId].numCh);

    pChObj = &pObj->chObj[chId];

    pPrm->isSaveFrameComplete = FALSE;
    pPrm->bufAddr = 0;
    pPrm->bufSize = 0;

    isYuv420Fmt = FALSE;
    if (pObj->createArgs.channelParams[chId].operatingMode !=
        ISSM2MSIMCOP_LINK_OPMODE_LDC)
    {
        isYuv420Fmt = TRUE;
    }
    else
    {
        if (pChObj->drvSimcopCfg.ldcCfg.mode == VPS_ISS_LDC_MODE_YUV420_LDC)
        {
            isYuv420Fmt = TRUE;
        }
        else
        {
            isYuv420Fmt = FALSE;
        }
    }

    if (pObj->createArgs.allocBufferForDump)
    {
        if(pChObj->saveFrame==FALSE)
        {
            pPrm->isSaveFrameComplete = TRUE;
            pPrm->bufAddr = (UInt32)pChObj->saveFrameBufAddr;

            if (TRUE == isYuv420Fmt)
            {
                pPrm->bufSize =
                    pObj->linkInfo.queInfo[0].chInfo[chId].pitch[0] *
                    pObj->linkInfo.queInfo[0].chInfo[chId].height * 3 / 2;
            }
            else
            {
                pPrm->bufSize =
                    pObj->linkInfo.queInfo[0].chInfo[chId].pitch[0] *
                    pObj->linkInfo.queInfo[0].chInfo[chId].height;
            }
        }

        status = SYSTEM_LINK_STATUS_SOK;
    }

    return (status);
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
Int32 IssM2mSimcopLink_drvSaveFrame(IssM2mSimcopLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    if (pObj->createArgs.allocBufferForDump)
    {
        pObj->chObj[0].saveFrame = TRUE;

        status = SYSTEM_LINK_STATUS_SOK;
    }

    return (status);
}

