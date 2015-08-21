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
 * \file sgxDisplayLink_drv.c
 *
 * \brief  This file has the implementation of SgxDisplay Link
 *
 *         SgxDisplay Link is used to feed video frames to SGX for
 *         rendering.
 *         The rendered output will be pushed to display via DRM.
 *
 * \version 0.0 (Jun 2014) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "sgxDisplayLink_priv.h"

/**
 *******************************************************************************
 *
 * \brief SgxDisplay link create function
 *
 *        This Set the Link and driver create time parameters.
 *        - Get the channel info from previous link
 *        - create the semaphore required for simply link
 *        - Set the internal data structures
 *        - Set the default sgxDisplay channel number
 *        - Call the driver create and control functions
 *        - Invoke Priming and start the driver
 *        - Create the link periodic object
 *
 * \param   pObj     [IN] SgxDisplay Link Instance handle
 * \param   pPrm     [IN] SgxDisplay link create parameters
 *                        This need to be configured by the application
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvCreate(SgxDisplayLink_Obj *pObj,
                               SgxDisplayLink_CreateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_DrmOpenPrms drmOpenPrm;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Create in progress for Display resolution: %dx%d !!!\n",
                 pPrm->displayWidth, pPrm->displayHeight);

#endif

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    status = System_linkGetInfo(pPrm->inQueParams.prevLinkId,
                                &pObj->inTskInfo);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
    OSA_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    memcpy(&pObj->inQueInfo,
           &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId],
           sizeof(pObj->inQueInfo));

    drmOpenPrm.connector_id = -1;
    drmOpenPrm.displayWidth = pPrm->displayWidth;
    drmOpenPrm.displayHeight = pPrm->displayHeight;
    status = System_drmOpen(&pObj->drmObj, &drmOpenPrm);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    status = System_eglOpen(&pObj->eglObj, &pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    status = System_drmSetMode(&pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    switch(pPrm->renderType)
    {
        default:
        case SGXDISPLAY_RENDER_TYPE_1x1:
            status = SgxRender1x1_setup(&pObj->render1x1Obj);
            break;
        case SGXDISPLAY_RENDER_TYPE_3D_CUBE:
            status = SgxRenderKmsCube_setup(&pObj->renderKmsCubeObj);
            break;
        case SGXDISPLAY_RENDER_TYPE_2x2:
            status = SgxRender2x2_setup(&pObj->render2x2Obj);
            break;
    }

    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    OSA_resetLatency(&pObj->linkLatency);
    OSA_resetLatency(&pObj->srcToLinkLatency);

    OSA_resetLinkStatistics(&pObj->linkStats, pObj->inQueInfo.numCh, 0);

    pObj->isFirstFrameRecv = FALSE;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Create Done for Display resolution: %dx%d !!!\n",
                 pPrm->displayWidth, pPrm->displayHeight);
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
 * \param   pObj        [IN] SgxDisplay Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvPrintStatistics(SgxDisplayLink_Obj *pObj)
{
    OSA_printLinkStatistics(&pObj->linkStats, "SGXDISPLAY", TRUE);

    OSA_printLatency("SGXDISPLAY",
                       &pObj->linkLatency,
                       &pObj->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief This function de-queue and process/sgxDisplay the input frames
 *
 *        Function perform the following operations
 *        - De-queue the frames from the Driver which are already sgxDisplayed
 *        - Free-up these frames by send back to previous link
 *        - Get/De-queue the valid frames from the input queue
 *        - Populate/Map Drm frame structure from system buffer
 *        - Submit these full frames to sgxDisplay driver
 *        - Immediately Free-up the frames which are not part of the sgxDisplay
 *
 * \param   pObj     [IN] SgxDisplay Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvDoProcessFrames(SgxDisplayLink_Obj *pObj)
{
    System_BufferList bufList;
    System_BufferList freeBufList;
    UInt32 freeBufNum, frameIdx;
    System_LinkInQueParams *pInQueParams;
    System_VideoFrameBuffer *pVideoFrame;
    System_VideoFrameCompositeBuffer *pVideoCompositeFrame;
    System_Buffer *pBuffer = NULL;
    Int32 status;
    GLuint texYuv[4] = {0};

    System_EglTexProperty texProp;

    pObj->linkStats.newDataCmdCount++;

    pInQueParams = &pObj->createArgs.inQueParams;
    /* queue frames if any */

    System_getLinksFullBuffers(pInQueParams->prevLinkId,
                               pInQueParams->prevLinkQueId,
                               &bufList);
    freeBufNum = 0;

    for(frameIdx=0; frameIdx<bufList.numBuf; frameIdx++)
    {
        if(pObj->isFirstFrameRecv==FALSE)
        {
            pObj->isFirstFrameRecv = TRUE;

            OSA_resetLinkStatistics(
                    &pObj->linkStats,
                    pObj->inQueInfo.numCh,
                    0);

            OSA_resetLatency(&pObj->linkLatency);
            OSA_resetLatency(&pObj->srcToLinkLatency);
        }

        pBuffer = bufList.buffers[frameIdx];

        if(pBuffer==NULL)
            continue;

        if(pBuffer->chNum < pObj->linkStats.numCh)
        {
            pObj->linkStats.chStats[pBuffer->chNum].inBufRecvCount++;
        }

        if(pBuffer->chNum == 0)
        {
            pBuffer->linkLocalTimestamp = OSA_getCurGlobalTimeInUsec();

            pObj->linkStats.chStats[pBuffer->chNum].inBufProcessCount++;

            texProp.width      = pObj->inQueInfo.chInfo[pBuffer->chNum].width;
            texProp.height     = pObj->inQueInfo.chInfo[pBuffer->chNum].height;
            texProp.pitch[0]   = pObj->inQueInfo.chInfo[pBuffer->chNum].pitch[0];
            texProp.pitch[1]   = pObj->inQueInfo.chInfo[pBuffer->chNum].pitch[1];
            texProp.dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pObj->inQueInfo.chInfo[pBuffer->chNum].flags);

            if(pBuffer->bufType==SYSTEM_BUFFER_TYPE_VIDEO_FRAME)
            {
                pVideoFrame = pBuffer->payload;
                OSA_assert(pVideoFrame != NULL);

                texYuv[0] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoFrame->bufAddr[0]);
                if(pObj->createArgs.renderType==SGXDISPLAY_RENDER_TYPE_2x2)
                {
                    texYuv[1] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoFrame->bufAddr[0]);
                    texYuv[2] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoFrame->bufAddr[0]);
                    texYuv[3] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoFrame->bufAddr[0]);
                }
            }
            else
            if(pBuffer->bufType==SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER)
            {
                pVideoCompositeFrame = pBuffer->payload;
                OSA_assert(pVideoCompositeFrame != NULL);

                /* pick CH0 by default */
                texYuv[0] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][0]);
                if(pObj->createArgs.renderType==SGXDISPLAY_RENDER_TYPE_2x2)
                {
                    texYuv[1] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][1]);
                    texYuv[2] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][2]);
                    texYuv[3] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][3]);
                }
            }
            else
            {
                Vps_printf(" SGXDISPLAY: ERROR: Recevied invalid buffer type !!!\n");
                OSA_assert(0);
            }

            switch(pObj->createArgs.renderType)
            {
                default:
                case SGXDISPLAY_RENDER_TYPE_1x1:
                    SgxRender1x1_renderFrame(
                            &pObj->render1x1Obj,
                            &pObj->eglObj,
                            texYuv[0]
                            );
                    break;
                case SGXDISPLAY_RENDER_TYPE_3D_CUBE:
                    SgxRenderKmsCube_renderFrame(
                            &pObj->renderKmsCubeObj,
                            &pObj->eglObj,
                            texYuv[0]
                            );
                    break;
                case SGXDISPLAY_RENDER_TYPE_2x2:
                    SgxRender2x2_renderFrame(
                            &pObj->render2x2Obj,
                            &pObj->eglObj,
                            texYuv,
                            4
                            );
                    break;
            }

            status = System_drmEglSwapBuffers(&pObj->drmObj, &pObj->eglObj);

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                Vps_printf(" SGXDISPLAY: System_drmEglSwapBuffers() failed !!!\n");
            }
            else
            {
                OSA_updateLatency(&pObj->linkLatency,
                                    pBuffer->linkLocalTimestamp);
                OSA_updateLatency(&pObj->srcToLinkLatency,
                                    pBuffer->srcTimestamp);
            }
        }
        else
        {
            if(pBuffer->chNum < pObj->linkStats.numCh)
            {
                pObj->linkStats.chStats[pBuffer->chNum].inBufDropCount++;
            }
        }
        OSA_assert(freeBufNum <
                   OSA_ARRAYSIZE(freeBufList.buffers));
        /* error in queuing to sgxDisplay, instead of asserting
            release the frame and continue
        */
        OSA_assert(pBuffer != NULL);
        freeBufList.buffers[freeBufNum] = pBuffer;
        freeBufNum++;
    }

    if (freeBufNum)
    {
        freeBufList.numBuf = freeBufNum;
        System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                    pInQueParams->prevLinkQueId,
                                    &freeBufList);
    }


    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function delete the sgxDisplay link & driver
 *
 *        De-queue any frames which are held inside the driver, then
 *        - Delete the simply driver
 *        - delete the semaphore and other link data structures
 *        - delete the link periodic object
 *
 * \param   pObj     [IN] SgxDisplay Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvDelete(SgxDisplayLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Delete in progress !!!\n");
#endif

    status = System_drmClose(&pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Delete Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function start the simply driver
 *
 *        Primming of a few frames are required to start the SgxDisplay driver.
 *        Use blank buffers to prime and start the simply driver even
 *        before the actual frames are received by the sgxDisplay link. This
 *        primming is done while sgxDisplay link create. Start shall be called
 *        only after the link create function
 *
 * \param   pObj     [IN] SgxDisplay Link Instance handle
 *
 * \return  status
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvStart(SgxDisplayLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Start in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Start Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function stop the simply driver
 *
 *        When ever the driver is stopped, enable the sgxDisplay link periodic
 *        call back function. This will initiate to free-up the input frames
 *        in STOP state. The driver call back will be stopped when sgxDisplay
 *        driver stop is done
 *
 * \param   pObj     [IN] SgxDisplay Link Instance handle
 *
 * \return  status
 *
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvStop(SgxDisplayLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Stop in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGXDISPLAY: Stop Done !!!\n");
#endif

    return status;
}

