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
 * \file epLink.c
 *
 * \brief  This file has the implementation of Endpoint Link API
 *
 *         This file implements the software logic needed to exchange frames
 *         between processors
 *
 * \version 0.0 (May 2015) : [SM] First version of endpoint sink driver
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "epLink_priv.h"
#include <linux/src/osa/include/osa_mem.h>

/**
 *******************************************************************************
 *
 * \brief Create Endpoint(EP) sink driver
 *
 *        Following happens during create phase,
 *        - Call 'get link info' on previous link.
 *          When link from other processors asks link info it gives back
 *          this link info to the next link (Endpoint)
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 EpLink_drvSinkCreate(EpLink_obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

#ifdef SYSTEM_DEBUG_EP
    Vps_printf(" EP_%u   : Create in progress !!!\n",
               pObj->instId
               );
#endif

    /* get previous link info */
    status = System_linkGetInfo(
                    pObj->createArgs.inQueParams.prevLinkId,
                    &pObj->linkInfo);

    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* validate previous link que ID */
    OSA_assert(pObj->createArgs.inQueParams.prevLinkQueId <
                  pObj->linkInfo.numQue);

    /*
     * Setup current link que information
     * Current queue is considered to have one output queue
     * with que information same as selected previous link queue
     */
    pObj->linkInfo.numQue = 1;
    memcpy(&pObj->linkInfo.queInfo[0],
           &pObj->linkInfo.queInfo
                [pObj->createArgs.inQueParams.prevLinkQueId],
           sizeof(pObj->linkInfo.queInfo[0]));

#ifdef SYSTEM_DEBUG_EP
    Vps_printf(" EP_%u   : Create Done !!!\n",
           pObj->instId
          );
#endif
    OSA_resetLatency(&pObj->srcToEpSinkLatency);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Process buffer's
 *
 *        - Previous link will notify when buffers are available
 *            to be sent across processors
 *
 *        - Pick the buffers from the bufList and post it to the que
 *
 * \param  pObj     [IN]  Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 EpLink_drvSinkProcessBuffers(EpLink_obj *pObj)
{
    Int32 i, j;
    Int32 status    = SYSTEM_LINK_STATUS_EFAIL;
    System_Buffer     *pBuffer;
    System_BufferList bufList;

    System_BufferType buf_type;
    System_VideoFrameCompositeBuffer *compBuf;
    System_VideoFrameBuffer *vidBuf;
    System_LinkChInfo *chInfo;
    System_LinkChInfo *srcChInfo;


    bufList.numBuf = 0;

    status = System_getLinksFullBuffers(
                    pObj->createArgs.inQueParams.prevLinkId,
                    pObj->createArgs.inQueParams.prevLinkQueId,
                    &bufList);

    srcChInfo = &pObj->linkInfo.queInfo[0].chInfo[0];
    for (i=0; i < bufList.numBuf; i++) {

        pBuffer = bufList.buffers[i];
        OSA_assert(pBuffer != NULL);

        /* let's update the timestamp at which we received the buffer */
        pBuffer->linkLocalTimestamp = OSA_getCurGlobalTimeInUsec();

	buf_type = pBuffer->bufType;
	if (buf_type == SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER) {
            /* Composite video frame - multiple frames in the same buffer
             * interprete payload as System_VideoFrameCompositeBuffer */
            compBuf = pBuffer->payload;
            chInfo = &compBuf->chInfo;
            compBuf->flags = srcChInfo->flags;
            for (j=0; j<compBuf->numFrames; j++) {
                /* TODO Handle multi-planar buffers */
                compBuf->bufAddr[0][j] = (Void*) OSA_memVirt2Phys((UInt32)compBuf->bufAddr[0][j],
                                                              OSA_MEM_REGION_TYPE_AUTO);
            }
        } else if (buf_type == SYSTEM_BUFFER_TYPE_VIDEO_FRAME) {
            /* Single video frame - only one frame in the buffer
             * interprete payload as System_VideoFrameBuffer */
             vidBuf = pBuffer->payload;
             chInfo = &vidBuf->chInfo;
             vidBuf->flags = srcChInfo->flags;
             vidBuf->bufAddr[0] = (Void*) OSA_memVirt2Phys((UInt32)vidBuf->bufAddr[0],
                                                          OSA_MEM_REGION_TYPE_AUTO);
	} else {
            /* getting rid of compiler warning - [-Wmaybe-uninitialized] */
             vidBuf = pBuffer->payload;
             chInfo = &vidBuf->chInfo;
        }

        chInfo->flags = srcChInfo->flags;
        chInfo->startY = srcChInfo->startY;
        chInfo->startY = srcChInfo->startY;
        chInfo->width = srcChInfo->width;
        chInfo->height = srcChInfo->height;

        for(j=0; j<SYSTEM_MAX_PLANES; j++) {
            chInfo->pitch[j] = srcChInfo->pitch[j];
        }
        /* we are now ready to post the buffer into the que */
        status = (pObj->post_buf)(&pObj->ep_ctx, pBuffer);
        if (status < 0)
	    Vps_printf("VIVI: Endpoint: %x: Failed to queue buffer\n", pObj->linkId);

        /* let's find the latency now */
        OSA_updateLatency(&pObj->srcToEpSinkLatency, pBuffer->srcTimestamp);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to get empty buffers from next
 *        link.
 *
 *        We are going to call the 'putEmptyBuf' callback function of the
 *        preceding link.
 *
 * \param  pObj     [IN]  Link object
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 EpLink_drvSinkPutEmptyBuffers(EpLink_obj *pObj,
                                    System_BufferList *pBufList)
{

    Int32 j;
    Int32 status = SYSTEM_LINK_STATUS_EFAIL;

    System_Buffer     *pBuffer;
    System_BufferType buf_type;
    System_VideoFrameCompositeBuffer *compBuf;
    System_VideoFrameBuffer *vidBuf;

    OSA_assert(pBufList->numBuf == 1);

    pBuffer = pBufList->buffers[0];
    buf_type = pBuffer->bufType;

    if (buf_type == SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER) {
            /* Composite video frame - multiple frames in the same buffer
             * interprete payload as System_VideoFrameCompositeBuffer */
            compBuf = pBuffer->payload;
            for (j=0; j<compBuf->numFrames; j++) {
                /* TODO Handle multi-planar buffers */
                compBuf->bufAddr[0][j] = (Void*) OSA_memPhys2Virt((UInt32)compBuf->bufAddr[0][j],
                                                              OSA_MEM_REGION_TYPE_AUTO);
            }
    } else if (buf_type == SYSTEM_BUFFER_TYPE_VIDEO_FRAME) {
            /* Single video frame - only one frame in the buffer
             * interprete payload as System_VideoFrameBuffer */
             vidBuf = pBuffer->payload;
             vidBuf->bufAddr[0] = (Void*) OSA_memPhys2Virt((UInt32)vidBuf->bufAddr[0],
                                                          OSA_MEM_REGION_TYPE_AUTO);
    }

    status = System_putLinksEmptyBuffers(
            pObj->createArgs.inQueParams.prevLinkId,
            pObj->createArgs.inQueParams.prevLinkQueId,
            pBufList);

    return status;
}
