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
#include "grpxSrcLink_priv.h"

#define ULTRASONIC_LAYOUT_CLIP_TOP       (100)
#define ULTRASONIC_LAYOUT_CLIP_BOTTOM    ( 75)
/* below co-ordinates are relative to surround view window*/
#define ULTRASONIC_LAYOUT_CLIP_CENTER_X  (326)
#define ULTRASONIC_LAYOUT_CLIP_CENTER_Y  (329)
#define ULTRASONIC_LAYOUT_CLIP_CENTER_W  (228) /* this MUST match the jeep width */
#define ULTRASONIC_LAYOUT_CLIP_CENTER_H  (432) /* this MUST match the jeep height */

Int32 GrpxSrcLink_drawUltrasonicResultsCreate(GrpxSrcLink_Obj *pObj)
{
    Int32 status;

    Utils_DmaChCreateParams dmaPrm;

    memset(&pObj->ultrasonicDrawObj, 0, sizeof(pObj->ultrasonicDrawObj));
    pObj->ultrasonicDrawObj.isFirstTime = TRUE;
    pObj->ultrasonicDrawObj.startTime = 0;
    pObj->ultrasonicDrawObj.refreshInterval = 34; /* in msecs */

    Utils_DmaChCreateParams_Init(&dmaPrm);

    status = Utils_dmaCreateCh(&pObj->ultrasonicDrawObj.dmaObj, &dmaPrm);
    UTILS_assert(status==0);

    return status;
}

Int32 GrpxSrcLink_drawUltrasonicResultsDelete(GrpxSrcLink_Obj *pObj)
{
    Int32 status;

    status = Utils_dmaDeleteCh(&pObj->ultrasonicDrawObj.dmaObj);

    UTILS_assert(status==0);

    return status;
}

Int32 GrpxSrcLink_drawUltrasonicResultsRun(GrpxSrcLink_Obj *pObj)
{
    GrpxSrcLink_UltrasonicDrawObj *pUltrasonicDrawObj;
    UInt32 elaspedTime;
    System_BufferList bufferList;
    System_Buffer *pBuffer;
    System_MetaDataBuffer *pMetaBuffer;
    Utils_DmaCopyFill2D dmaPrm;
    Int32 status;

    pUltrasonicDrawObj = &pObj->ultrasonicDrawObj;

    if(pUltrasonicDrawObj->isFirstTime)
    {
        pUltrasonicDrawObj->isFirstTime = FALSE;
        pUltrasonicDrawObj->startTime = Utils_getCurTimeInMsec();
        return 0;
    }

    elaspedTime = Utils_getCurTimeInMsec() - pUltrasonicDrawObj->startTime;

    if(elaspedTime < pUltrasonicDrawObj->refreshInterval)
    {
        /* Not yet time to draw */
        return 0;
    }

    /* now draw */
    pUltrasonicDrawObj->startTime = Utils_getCurTimeInMsec();

    System_getLinksFullBuffers(
        pObj->createArgs.inQueParams.prevLinkId,
        pObj->createArgs.inQueParams.prevLinkQueId,
        &bufferList
        );

    if(bufferList.numBuf)
    {
        /* pick last buffer or latest buffer to draw */
        pBuffer = bufferList.buffers[bufferList.numBuf-1];

        if(pBuffer
            && pBuffer->bufType == SYSTEM_BUFFER_TYPE_METADATA
            )
        {
            pMetaBuffer = (System_MetaDataBuffer*)pBuffer->payload;

            if(pMetaBuffer && pMetaBuffer->numMetaDataPlanes==1)
            {
                UInt16 startX, startY, winWidth, winHeight, displayWidth, displayHeight;
                /* there can be only one plane  for overlay data */

                /* check if drawing area is within display are, else dont draw */

                startX    = pObj->createArgs.ultrasonicParams.windowStartX;
                startY    = pObj->createArgs.ultrasonicParams.windowStartY;
                winWidth  = pObj->createArgs.ultrasonicParams.windowWidth;
                winHeight = pObj->createArgs.ultrasonicParams.windowHeight;
                displayWidth  = pObj->info.queInfo[0].chInfo[0].width;
                displayHeight = pObj->info.queInfo[0].chInfo[0].height;

                if(startX + winWidth <= displayWidth
                       &&
                   startY + winHeight <= displayHeight
                       &&
                    pObj->createArgs.grpxBufInfo.dataFormat ==
                        SYSTEM_DF_BGRA16_4444
                       &&
                    ULTRASONIC_LAYOUT_CLIP_CENTER_X + ULTRASONIC_LAYOUT_CLIP_CENTER_W <= winWidth
                       &&
                    ULTRASONIC_LAYOUT_CLIP_CENTER_Y + ULTRASONIC_LAYOUT_CLIP_CENTER_H <= winHeight
                       &&
                    ULTRASONIC_LAYOUT_CLIP_TOP <= ULTRASONIC_LAYOUT_CLIP_CENTER_Y
                       &&
                    (Int32)(displayHeight - ULTRASONIC_LAYOUT_CLIP_BOTTOM) >= (ULTRASONIC_LAYOUT_CLIP_CENTER_Y+ULTRASONIC_LAYOUT_CLIP_CENTER_H)
                    )
                {
                    /* Now DMA from this buffer to Grpx Buffer */

                    /* DMA in 4 blocks such that it does not overlap
                     * the surround view logo, jeep image and text
                     */

                    /* Block 0 - top */
                    dmaPrm.dataFormat   = SYSTEM_DF_RAW16;
                    dmaPrm.destAddr[0]  = (Ptr) pObj->outObj.videoFrames[0].bufAddr[0];
                    dmaPrm.destPitch[0] = pObj->info.queInfo[0].chInfo[0].pitch[0];
                    dmaPrm.destAddr[1]  = NULL;
                    dmaPrm.destPitch[1] = 0;
                    dmaPrm.srcStartX    = 0;
                    dmaPrm.srcStartY    = ULTRASONIC_LAYOUT_CLIP_TOP;
                    dmaPrm.destStartX   = startX + dmaPrm.srcStartX;
                    dmaPrm.destStartY   = startY + dmaPrm.srcStartY;
                    dmaPrm.width        = winWidth;
                    dmaPrm.height       = ULTRASONIC_LAYOUT_CLIP_CENTER_Y - dmaPrm.srcStartY;
                    dmaPrm.srcAddr[0]   = (Ptr) pMetaBuffer->bufAddr[0];
                    dmaPrm.srcPitch[0]  = dmaPrm.width*2; /* assuming 2 bytes per pixel */
                    dmaPrm.srcAddr[1]   = NULL;
                    dmaPrm.srcPitch[1]  = 0;

                    status = Utils_dmaCopy2D(&pUltrasonicDrawObj->dmaObj,
                                             &dmaPrm,
                                             1);

                    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                    /* Block 1 - left */
                    dmaPrm.dataFormat   = SYSTEM_DF_RAW16;
                    dmaPrm.destAddr[0]  = (Ptr) pObj->outObj.videoFrames[0].bufAddr[0];
                    dmaPrm.destPitch[0] = pObj->info.queInfo[0].chInfo[0].pitch[0];
                    dmaPrm.destAddr[1]  = NULL;
                    dmaPrm.destPitch[1] = 0;
                    dmaPrm.srcStartX    = 0;
                    dmaPrm.srcStartY    = ULTRASONIC_LAYOUT_CLIP_CENTER_Y;
                    dmaPrm.destStartX   = startX + dmaPrm.srcStartX;
                    dmaPrm.destStartY   = startY + dmaPrm.srcStartY;
                    dmaPrm.width        = ULTRASONIC_LAYOUT_CLIP_CENTER_X;
                    dmaPrm.height       = ULTRASONIC_LAYOUT_CLIP_CENTER_H;
                    dmaPrm.srcAddr[0]   = (Ptr) pMetaBuffer->bufAddr[0];
                    dmaPrm.srcPitch[0]  = dmaPrm.width*2; /* assuming 2 bytes per pixel */
                    dmaPrm.srcAddr[1]   = NULL;
                    dmaPrm.srcPitch[1]  = 0;

                    status = Utils_dmaCopy2D(&pUltrasonicDrawObj->dmaObj,
                                             &dmaPrm,
                                             1);

                    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                    /* Block 2 - right */
                    dmaPrm.dataFormat   = SYSTEM_DF_RAW16;
                    dmaPrm.destAddr[0]  = (Ptr) pObj->outObj.videoFrames[0].bufAddr[0];
                    dmaPrm.destPitch[0] = pObj->info.queInfo[0].chInfo[0].pitch[0];
                    dmaPrm.destAddr[1]  = NULL;
                    dmaPrm.destPitch[1] = 0;
                    dmaPrm.srcStartX    = ULTRASONIC_LAYOUT_CLIP_CENTER_X + ULTRASONIC_LAYOUT_CLIP_CENTER_W;
                    dmaPrm.srcStartY    = ULTRASONIC_LAYOUT_CLIP_CENTER_Y;
                    dmaPrm.destStartX   = startX + dmaPrm.srcStartX;
                    dmaPrm.destStartY   = startY + dmaPrm.srcStartY;
                    dmaPrm.width        = winWidth - dmaPrm.srcStartX;
                    dmaPrm.height       = ULTRASONIC_LAYOUT_CLIP_CENTER_H;
                    dmaPrm.srcAddr[0]   = (Ptr) pMetaBuffer->bufAddr[0];
                    dmaPrm.srcPitch[0]  = dmaPrm.width*2; /* assuming 2 bytes per pixel */
                    dmaPrm.srcAddr[1]   = NULL;
                    dmaPrm.srcPitch[1]  = 0;

                    status = Utils_dmaCopy2D(&pUltrasonicDrawObj->dmaObj,
                                             &dmaPrm,
                                             1);

                    /* Block 3 - bottom */
                    dmaPrm.dataFormat   = SYSTEM_DF_RAW16;
                    dmaPrm.destAddr[0]  = (Ptr) pObj->outObj.videoFrames[0].bufAddr[0];
                    dmaPrm.destPitch[0] = pObj->info.queInfo[0].chInfo[0].pitch[0];
                    dmaPrm.destAddr[1]  = NULL;
                    dmaPrm.destPitch[1] = 0;
                    dmaPrm.srcStartX    = 0;
                    dmaPrm.srcStartY    = ULTRASONIC_LAYOUT_CLIP_CENTER_Y + ULTRASONIC_LAYOUT_CLIP_CENTER_H;
                    dmaPrm.destStartX   = startX + dmaPrm.srcStartX;
                    dmaPrm.destStartY   = startY + dmaPrm.srcStartY;
                    dmaPrm.width        = winWidth;
                    dmaPrm.height       = winHeight - dmaPrm.srcStartY - ULTRASONIC_LAYOUT_CLIP_BOTTOM;
                    dmaPrm.srcAddr[0]   = (Ptr) pMetaBuffer->bufAddr[0];
                    dmaPrm.srcPitch[0]  = dmaPrm.width*2; /* assuming 2 bytes per pixel */
                    dmaPrm.srcAddr[1]   = NULL;
                    dmaPrm.srcPitch[1]  = 0;

                    status = Utils_dmaCopy2D(&pUltrasonicDrawObj->dmaObj,
                                             &dmaPrm,
                                             1);

                    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                }
            }
        }

        System_putLinksEmptyBuffers(
            pObj->createArgs.inQueParams.prevLinkId,
            pObj->createArgs.inQueParams.prevLinkQueId,
            &bufferList
            );
    }

    return 0;
}

