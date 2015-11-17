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
 * \file displayLink_drv.c
 *
 * \brief  This file has the implementation of Display Link
 *
 *         Display Link is used to feed video frames over a video/graphics
 *         pipe to a connected VENC. The connection of input pipe to a VENC
 *         is done by Display Controller. This link deals with actually
 *         displaying the video/graphic frames from a previous link onto the
 *         display device.
 *
 *         The display link receives the input frames, submitted/queued them
 *         into VPS display driver.  The driver displays the frames and also
 *         invoke a call back. On call back Display Link reclaim these frames
 *         which are already displayed and send back to the previous link.
 *         Also queued the new frame to the driver and which will be
 *         displayed in subsequent display interrupts
 *
 *         The display link can only take input for a single input queue.
 *         The single input queue can contain multiple channels but only
 *         one of the channel can be shown at a time.
 *
 *         By default CH0 is shown on the display.
 *
 *         Users can use the command DISPLAY_LINK_CMD_SWITCH_CH to switch
 *         the channel that is displayed on the display - This feature is
 *         NOT supported in this version.
 *
 *         Display link also supported an inline scalar. The scalar will be
 *         enabled automatically once the input image resolution is different
 *         than the target video window display resolution
 *
 *         Display link also supports the run time input resolution change
 *         or dynamic channel switch - This feature is
 *         NOT supported in this version.
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "displayLink_priv.h"

char *gDisplayLink_displayName[] = { "VID1", "VID2", "VID3", "GRPX1" };


/*
 *******************************************************************************
 *
 * This function is used to select the switch the current active channel
 * Active channel ID MUST be less than pObj->linkStatsInfo->linkStats.numCh
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvSwitchCh(DisplayLink_Obj *pObj,
                              DisplayLink_SwitchChannelParams *prm)
{
    if(prm->activeChId < pObj->inTskInfo.queInfo[0].numCh)
    {
        /* if valid CH ID change the current display channel number,
         * else leave it unchanged
         */
        pObj->curDisplayChannelNum = prm->activeChId;
    }
    else
    {
        Vps_printf("\n Invalid channel Id");
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/*
 *******************************************************************************
 *
 * This function is used to create queue's that will be used within display link
 *
 * Two queues are created,
 *
 * 1. A queue to hold FVID_Frame objects that will be exchanged with
 *    the FVID2 driver.
 *    When a System_Buffer is received from previous link, a FVID2_Frame is
 *    deuqued from this queue and then queued to the FIVD2 driver.
 *
 * 2. A queue to hold System_Buffer's that needs to be released to the
 *    previous link after the buffer has been displayed
 *
 *******************************************************************************
 */
static
Void DisplayLink_drvCreateQueues(DisplayLink_Obj *pObj)
{
    Int32 status;
    Int32 i;

    status = Utils_queCreate(&pObj->fvidFrameQueue,
                             DISPLAY_LINK_MAX_FRAMES_PER_HANDLE,
                             pObj->fvidFrameQueueMem,
                             UTILS_QUE_FLAG_NO_BLOCK_QUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_queCreate(&pObj->systemBufferQueue,
                             DISPLAY_LINK_MAX_FRAMES_PER_HANDLE,
                             pObj->systemBufferQueueMem,
                             UTILS_QUE_FLAG_NO_BLOCK_QUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* Queue uninitialized FVID2_Frame objects to
     * the FVID2 frame queue initially
     */
    for (i = 0; i < DISPLAY_LINK_MAX_FRAMES_PER_HANDLE; i++)
    {
        status = Utils_quePut(&pObj->fvidFrameQueue,
                              &pObj->frames[i],
                              BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
}

/*
 *******************************************************************************
 *
 * This function is the callback that is called by the driver at every
 * display Vsync
 *
 * In this callback, we do the below
 * - Dequeue frames from the driver
 * - Release the FVID frames to the FVID2 frame queue
 * - Queue the System_Buffer's to the System Buffer queue
 * - Send a command to display link to "release" the system buffer's to
 *   previous link
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvFvidCb(FVID2_Handle handle, Ptr appData, Ptr reserved)
{
    DisplayLink_Obj *pObj = (DisplayLink_Obj *) appData;
    FVID2_Frame *pFrame;
    FVID2_FrameList frameList;
    UInt32 i, sendCmd;
    Int32 status;

    pObj->linkStatsInfo->linkStats.notifyEventCount++;

    sendCmd = FALSE;

    do
    {
        frameList.numFrames = 0;

        status = FVID2_dequeue(
                    pObj->displayHndl,
                    &frameList,
                    0,
                    BSP_OSAL_NO_WAIT);

        for(i=0; i<frameList.numFrames; i++)
        {
            pFrame = frameList.frames[i];

            /* Put System_Buffer in System buffer queue
             * These will be released when
             * DISPLAY_LINK_CMD_RELEASE_FRAMES is called
             *
             * We need a queue here since we cannot free these buffers
             * from ISR context
             */
            status = Utils_quePut(&pObj->systemBufferQueue,
                                  pFrame->appData,
                                  BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            status = Utils_quePut(&pObj->fvidFrameQueue,
                                  pFrame,
                                  BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }

        if(frameList.numFrames)
        {
            sendCmd = TRUE;
        }
    } while(status==SYSTEM_LINK_STATUS_SOK);

    if(sendCmd)
    {
        Utils_tskSendCmd(&pObj->tsk, DISPLAY_LINK_CMD_RELEASE_FRAMES, NULL);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/*
 *******************************************************************************
 *
 * This function creates the FVID2 driver
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvDisplayCreate(DisplayLink_Obj *pObj)
{
    Int32 status;
    FVID2_CbParams cbParams;
    Vps_DispDssParams *dssPrms;
    System_LinkChInfo *pInChInfo;
    Vps_DispCreateStatus displayCreateStatus;

    pInChInfo = &pObj->inQueInfo.chInfo[pObj->curDisplayChannelNum];
    dssPrms = &pObj->dssPrms;

    switch (pObj->createArgs.displayId)
    {
        case DISPLAY_LINK_INST_DSS_VID1:
            pObj->displayInstId = VPS_DISP_INST_DSS_VID1;
            break;
        case DISPLAY_LINK_INST_DSS_VID2:
            pObj->displayInstId = VPS_DISP_INST_DSS_VID2;
            break;
        case DISPLAY_LINK_INST_DSS_VID3:
            pObj->displayInstId = VPS_DISP_INST_DSS_VID3;
            if(Bsp_platformIsTda3xxFamilyBuild())
            {
                Vps_printf(" DISPLAY: DSS_VID3_PIPE NOT supported on TDA3xx"
                       " !!!\n"
                       );
                UTILS_assert(0);
            }
            break;
        case DISPLAY_LINK_INST_DSS_GFX1:
            pObj->displayInstId = VPS_DISP_INST_DSS_GFX1;
            break;
        default:
            pObj->displayInstId = VPS_DISP_INST_DSS_VID1;
            break;
    }

    Fvid2CbParams_init(&cbParams);
    cbParams.cbFxn = DisplayLink_drvFvidCb;
    cbParams.appData = pObj;

    VpsDispCreateParams_init(&pObj->displayCreateArgs);
    pObj->displayCreateArgs.periodicCbEnable = TRUE;

    pObj->displayHndl = FVID2_create(FVID2_VPS_DISP_DRV,
                                     pObj->displayInstId,
                                     &pObj->displayCreateArgs,
                                     &displayCreateStatus, &cbParams);
    UTILS_assert(pObj->displayHndl != NULL);

    VpsDispDssParams_init(dssPrms);

    VpsDssDispcVidConfig_init(&pObj->vidCfg);
    VpsDssDispcGfxConfig_init(&pObj->gfxCfg);

    pObj->vidCfg.pipeCfg.scEnable = TRUE;
    if ( (pInChInfo->width == pObj->createArgs.rtParams.tarWidth)
         &&
         (pInChInfo->height == pObj->createArgs.rtParams.tarHeight)
       )
    {
        pObj->vidCfg.pipeCfg.scEnable = FALSE;
    }

    if (Vps_dispIsVidInst(pObj->displayInstId))
    {
        dssPrms->vidCfg = &pObj->vidCfg;
    }
    if (Vps_dispIsGfxInst(pObj->displayInstId))
    {
        dssPrms->gfxCfg = &pObj->gfxCfg;
    }

    dssPrms->inFmt.chNum           = 0;
    dssPrms->inFmt.width           = pInChInfo->width; //¿©±â¼­ ¹Ù²ñ
    dssPrms->inFmt.height          = pInChInfo->height;
    dssPrms->inFmt.pitch[0u]       = pInChInfo->pitch[0];
    dssPrms->inFmt.pitch[1u]       = pInChInfo->pitch[1];
    dssPrms->inFmt.pitch[2u]       = pInChInfo->pitch[2];
    dssPrms->inFmt.dataFormat      =
        SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pInChInfo->flags);
    if(dssPrms->inFmt.dataFormat == SYSTEM_DF_ABGR32_8888)
    {
        /* Convert to equivalent Display format */
        dssPrms->inFmt.dataFormat = SYSTEM_DF_BGRA32_8888;
    }
    dssPrms->inFmt.fieldMerged[0] = FALSE;
    dssPrms->inFmt.scanFormat = FVID2_SF_PROGRESSIVE;
    if (pObj->createArgs.displayScanFormat == SYSTEM_SF_INTERLACED)
    {
        dssPrms->inFmt.fieldMerged[0] = TRUE;
        dssPrms->inFmt.scanFormat = FVID2_SF_INTERLACED;
    }
    dssPrms->inFmt.fieldMerged[1] = dssPrms->inFmt.fieldMerged[0];
    dssPrms->inFmt.fieldMerged[2] = dssPrms->inFmt.fieldMerged[0];
    dssPrms->inFmt.bpp = FVID2_BPP_BITS16;
    dssPrms->inFmt.reserved = NULL;

    if (pObj->createArgs.rtParams.tarWidth == 0)
    {
       pObj->createArgs.rtParams.tarWidth = dssPrms->inFmt.width;
    }
    if (pObj->createArgs.rtParams.tarHeight == 0)
    {
       pObj->createArgs.rtParams.tarHeight = dssPrms->inFmt.height;
    }

    dssPrms->tarWidth  = pObj->createArgs.rtParams.tarWidth;
    dssPrms->tarHeight = pObj->createArgs.rtParams.tarHeight;
    dssPrms->posX      = pObj->createArgs.rtParams.posX;
    dssPrms->posY      = pObj->createArgs.rtParams.posY;
    dssPrms->memType   = VPS_VPDMA_MT_NONTILEDMEM;

    status = FVID2_control(
                 pObj->displayHndl,
                 IOCTL_VPS_DISP_SET_DSS_PARAMS,
                 &pObj->dssPrms,
                 NULL);
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Display link create function
 *
 *        This Set the Link and driver create time parameters.
 *        - Get the channel info from previous link
 *        - Set the internal data structures
 *        - Set the default display channel number
 *        - Call the driver create and control functions
 *
 * \param   pObj     [IN] Display Link Instance handle
 * \param   pPrm     [IN] Display link create parameters
 *                        This need to be configured by the application
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvCreate(DisplayLink_Obj *pObj,
                            DisplayLink_CreateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    DisplayLink_SwitchChannelParams switchParams;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Create in progress !!!\n");
#endif
    UTILS_MEMLOG_USED_START();

    pObj->isDisplayRunning = FALSE;
    pObj->queueCount = 0;

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    status = System_linkGetInfo(pPrm->inQueParams.prevLinkId,
                                &pObj->inTskInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    /*
     * In case of interlaced display for NTSC / PAL, height needs to be field height.
     * Assuming that height obtained from previous link
     * and create time height will be frame height,
     * it is halved here.
     * And also active channel Id is assumed to be 0
     */
    if (pObj->createArgs.displayScanFormat == SYSTEM_SF_INTERLACED)
    {
        pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId].chInfo[0].height =
            pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId].chInfo[0].height/2;
        pObj->createArgs.rtParams.tarHeight = pObj->createArgs.rtParams.tarHeight/2;
        pObj->createArgs.rtParams.posY = pObj->createArgs.rtParams.posY/2;
    }

    memcpy(&pObj->inQueInfo,
           &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId],
           sizeof(pObj->inQueInfo));

    /* Assume channel 0 will be display as default in the create time*/
    switchParams.activeChId = 0;
    DisplayLink_drvSwitchCh(pObj, &switchParams);
    DisplayLink_drvDisplayCreate(pObj);
    DisplayLink_drvCreateQueues(pObj);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->tskId, "DISPLAY");
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->isFirstFrameRecv = FALSE;

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("DISPLAY:",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Create Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function prints the Link status
 *
 *        prints the Link status, such as
 *        - FPS
 *        - Callback Intervals
 *        - Input DropCount
 *        - etc
 *
 * \param   pObj        [IN] Display Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvPrintStatistics(DisplayLink_Obj *pObj)
{
    Vps_DispStatus dispStaus;

    UTILS_assert(NULL != pObj->linkStatsInfo);

    Utils_printLinkStatistics(&pObj->linkStatsInfo->linkStats, "DISPLAY", TRUE);

    Utils_printLatency("DISPLAY",
                       &pObj->linkStatsInfo->linkLatency,
                       &pObj->linkStatsInfo->srcToLinkLatency,
                       TRUE);

    if (pObj->displayHndl)
    {
        Int32 status;

        status =
        FVID2_control(pObj->displayHndl,
                      IOCTL_VPS_DISP_GET_STATUS,
                      &dispStaus,NULL);
        if (SYSTEM_LINK_STATUS_SOK == status)
        {
            Vps_printf(" \n");
            Vps_printf(" [ DISPLAY %5s ] Additional Statistics,\n",
                                gDisplayLink_displayName[pObj->displayInstId]);
            Vps_printf(" ************************************\n");
            Vps_printf(" Driver Queued    = %6d frames \n",
                            dispStaus.queueCount);
            Vps_printf(" Driver De-Queued = %6d frames \n",
                            dispStaus.dequeueCount);
            Vps_printf(" Driver Displayed = %6d frames \n",
                            dispStaus.dispFrmCount);
            Vps_printf(" Driver Repeated  = %6d frames \n",
                            dispStaus.repeatFrmCount);
            Vps_printf(" \n");
            Vps_printf(" ##### DSS DISPC Underflow Count  = %6d #####\n",
                            dispStaus.underflowCount);
            Vps_printf(" \n");
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * This function releases System_Buffer's queued from ISR context to the
 * previous link
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvReleaseData(DisplayLink_Obj *pObj)
{
    System_BufferList bufList;
    System_LinkInQueParams *pInQueParams;
    Int32 status;
    System_Buffer *pBuffer;

    pInQueParams = &pObj->createArgs.inQueParams;

    bufList.numBuf = 0;

    do
    {
        status = Utils_queGet(&pObj->systemBufferQueue,
                     (Ptr*)&pBuffer,
                     1,
                     BSP_OSAL_NO_WAIT);
        if(status==SYSTEM_LINK_STATUS_SOK && pBuffer)
        {
            bufList.buffers[bufList.numBuf]
                = pBuffer;

            bufList.numBuf++;
        }
    } while(status==SYSTEM_LINK_STATUS_SOK);

    /* release buffers to previous link */
    if (bufList.numBuf)
    {
        System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                    pInQueParams->prevLinkQueId,
                                    &bufList);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * This function get buffer's from previous link and display's the buffer
 * associated with current active channel
 *
 * All other CH buffer's are released as
 *
 *******************************************************************************
 */
Int32 DisplayLink_drvProcessData(DisplayLink_Obj *pObj)
{
    System_BufferList bufList;
    System_BufferList freeBufList;
    FVID2_FrameList displayFrameList;
    System_LinkInQueParams *pInQueParams;
    FVID2_Frame *pFrame;
    System_Buffer *pBuffer = NULL;
    Int32 status;
    UInt32 frameIdx;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    pInQueParams = &pObj->createArgs.inQueParams;

    /* get frames from previous link */
    bufList.numBuf = 0;
    System_getLinksFullBuffers(pInQueParams->prevLinkId,
                               pInQueParams->prevLinkQueId,
                               &bufList);

    if(pObj->isDisplayRunning)
    {
        /* init number of frames to be released to 0 */
        freeBufList.numBuf = 0;

        /* handle each received frame */
        for( frameIdx = 0;  frameIdx < bufList.numBuf; frameIdx++)
        {
            /* reset stats counter if this is first frame that is recived */
            if(pObj->isFirstFrameRecv==FALSE)
            {
                pObj->isFirstFrameRecv = TRUE;

                Utils_resetLinkStatistics(
                        &linkStatsInfo->linkStats,
                        pObj->inQueInfo.numCh,
                        0);

                Utils_resetLatency(&linkStatsInfo->linkLatency);
                Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
            }

            pBuffer = bufList.buffers[frameIdx];

            UTILS_assert(pBuffer->chNum < pObj->inQueInfo.numCh);

            linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufRecvCount++;

            pBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

            if (pBuffer->chNum != pObj->curDisplayChannelNum)
            {
                /* if this is not the current channel to be displayed
                 * put buffer in buffer list to be released
                 */
                freeBufList.buffers[freeBufList.numBuf]
                    = pBuffer;

                freeBufList.numBuf++;

                linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
            }
            else
            {
                /* display this frame */

                /* get FVID2 frame to queue to the driver
                 * This should not fail.
                 */
                status  = Utils_queGet(&pObj->fvidFrameQueue,
                                       (Ptr *)&pFrame,
                                       1,
                                       BSP_OSAL_NO_WAIT);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                UTILS_assert(pFrame != NULL);

                /* copy info from System buffer to FVID2 frame */
                status = Utils_bufInitFrame(pFrame, pBuffer);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                /* Set FVID2 plane address for odd field's */
                pFrame->addr[1][0] =
                    (UInt8 *) pFrame->addr[0][0] + pObj->dssPrms.inFmt.pitch[0];
                pFrame->addr[1][1] =
                    (UInt8 *) pFrame->addr[0][1] + pObj->dssPrms.inFmt.pitch[1];

                /* queue frame for display */
                displayFrameList.numFrames = 1;
                displayFrameList.frames[0] = pFrame;
                displayFrameList.perListCfg = NULL;

                status = FVID2_queue(pObj->displayHndl,
                                     &displayFrameList,
                                     0);

                if(status!=SYSTEM_LINK_STATUS_SOK)
                {
                    /* if frame could not be queued to driver
                     *   then release the frame
                     */
                    Vps_printf(" DISPLAY: %s: Queue to driver failed !!!\n",
                        gDisplayLink_displayName[pObj->displayInstId]
                        );

                    freeBufList.buffers[freeBufList.numBuf]
                        = pBuffer;

                    freeBufList.numBuf++;

                    Utils_quePut(&pObj->fvidFrameQueue,
                                 (Ptr)pFrame,
                                 BSP_OSAL_NO_WAIT);

                    linkStatsInfo->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
                }
                else
                {
                    linkStatsInfo->linkStats.chStats[pBuffer->chNum].
                            inBufProcessCount++;

                    Utils_updateLatency(&linkStatsInfo->linkLatency,
                                            pBuffer->linkLocalTimestamp);
                    Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                            pBuffer->srcTimestamp);

                    /* start FVID2 driver after 1 frame is queued */
                    if(pObj->queueCount<1)
                    {
                        pObj->queueCount++;

                        if(pObj->queueCount==1)
                        {
                            status = FVID2_start(pObj->displayHndl, NULL);
                            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                        }
                    }
                }
            }
        }

        if (freeBufList.numBuf)
        {
            System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                        pInQueParams->prevLinkQueId,
                                        &freeBufList);
        }
    }
    else
    {
        /* display is not running, release all frames to previous link */
        if (bufList.numBuf)
        {
            System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                        pInQueParams->prevLinkQueId,
                                        &bufList);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 DisplayLink_drvDelete(DisplayLink_Obj *pObj)
{
    FVID2_FrameList frameList;
    Int32 status;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Delete in progress !!!\n");
#endif

    do
    {
        // de-queue queued buffer's
        status = FVID2_dequeue(pObj->displayHndl, &frameList, 0, BSP_OSAL_NO_WAIT);
    } while (status == SYSTEM_LINK_STATUS_SOK);

    status = FVID2_delete(pObj->displayHndl, NULL);
    if (SYSTEM_LINK_STATUS_SOK != status)
    {
        Vps_printf(" DISPLAY: Delete ERROR !!!\n");
        return (status);
    }

    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_queDelete(&pObj->fvidFrameQueue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_queDelete(&pObj->systemBufferQueue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Delete Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 DisplayLink_drvStart(DisplayLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Start in progress !!!\n");
#endif

    pObj->isDisplayRunning = TRUE;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Start Done !!!\n");
#endif

    return status;
}

Int32 DisplayLink_drvStop(DisplayLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Stop in progress !!!\n");
#endif

    status = FVID2_stop(pObj->displayHndl, NULL);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pObj->isDisplayRunning = FALSE;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" DISPLAY: Stop Done !!!\n");
#endif

    return status;
}

Int32 DisplayLink_drvGetStatistics(DisplayLink_Obj *pObj,
            DisplayLink_Statistics *pPrm)
{
    UInt16 chNum = pObj->curDisplayChannelNum;
    System_LinkStatistics *linkStatsInfo;

    linkStatsInfo = pObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    pPrm->elaspedTimeInMsec =
        Utils_getCurGlobalTimeInMsec()
            - linkStatsInfo->linkStats.statsStartTime;

    pPrm->displayIsrCount =
        linkStatsInfo->linkStats.notifyEventCount;

    if(chNum < linkStatsInfo->linkStats.numCh)
    {
        pPrm->inBufRecvCount =
            linkStatsInfo->linkStats.chStats[chNum].inBufRecvCount;

        pPrm->inBufDropCount =
            linkStatsInfo->linkStats.chStats[chNum].inBufDropCount;

        pPrm->inBufDisplayCount =
            linkStatsInfo->linkStats.chStats[chNum].inBufProcessCount;
    }
    else
    {
        pPrm->inBufRecvCount = 0;
        pPrm->inBufDropCount = 0;
        pPrm->inBufDisplayCount = 0;
    }

    if(pPrm->resetStatistics==TRUE)
    {
        Utils_resetLinkStatistics(
            &linkStatsInfo->linkStats,
            pObj->inQueInfo.numCh,
            0);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
