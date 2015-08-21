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
 * \file CaptureLink_subframe__drv.c
 *
 * \brief  This file contains subframe related implementation for capture link
 *
 * \version 0.0 (Jul 2014) : [VT] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "captureLink_priv.h"
#include <src/utils_common/include/utils_cbuf_ocmc.h>


/**
 *******************************************************************************
 *
 * \brief   This functions fills the information of VIP Output frame  into
 *          the OCMC info buffer pointer passed to it.
            The subframe copy  link queries this information from capture link.
 * \param   pObj            [IN] subframe capture link global handle
 *          pCaptureLinkSubframeInfo [IN] ocmc buffer i.e. VIP output buffer info.
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 CaptureLink_subframe_drvGetVIPOutFrameInfo
                            (CaptureLink_Obj *pObj,
                            CaptureLink_Subframe_Info *pCaptureLinkSubframeInfo)
{
    UInt32  planeCnt, instId, streamId;
    Vps_CaptVipOutInfo *pOutInfo;
    Vps_CaptCreateParams *pVipCreateArgs;
    CaptureLink_InstObj * pDrvObj;

    /* Get the VIP instance id based on the qChannel id passed by subframe copy
    * alg plugin
    */
    instId = pObj->chToInstMap[pCaptureLinkSubframeInfo->inChannelId];
    pDrvObj = &pObj->instObj[instId];
    pVipCreateArgs = &pDrvObj->createArgs;

    /* Get the atream id based on the qChannel id passed by subframe copy
    * alg plugin
    */
    for (streamId = 0; streamId < pVipCreateArgs->numStream; streamId++)
    {
        if(pVipCreateArgs->chNumMap[streamId][0] == pCaptureLinkSubframeInfo->inChannelId)
            break;
    }
    pOutInfo = &pDrvObj->vipPrms.outStreamInfo[streamId];

    /*
     * Fill the OCMC buffer information
     */
    pCaptureLinkSubframeInfo->numLinesPerSubFrame
                                    = pOutInfo->subFrmPrms.numLinesPerSubFrame;

    for (planeCnt = 0; planeCnt < SYSTEM_MAX_PLANES; planeCnt++)
    {
        pCaptureLinkSubframeInfo->ocmcCBufVirtAddr[planeCnt]
                        = (UInt32)pDrvObj->frames[0].addr[0][planeCnt];
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Allocates OCMC frames.
 *
 *
 * \param   pFormat     [IN] data format
 *          pFrame      [IN] frame pointer to store allocated data
 *          numLinesPerSlice [IN] number of vertical lines per slice
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 CaptureLink_subframe_drvAllocOCMCFrame(
                    FVID2_Format * pFormat,
                  FVID2_Frame * pFrame,
                  UInt32 numLinesPerSlice)
{
    Ptr virtStartAddr = 0;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    //Initialise the OCMC Cbuf for allocating data from OCMC_RAM1 region
    Utils_cbufOcmcInit(UTILS_OCMC_RAM1);

    //Allocate frames from OCMC region as circular buffers
    switch (pFormat->dataFormat)
    {
        case FVID2_DF_YUV422I_YUYV:
            /* Actualy memory allocated in only for numLinesPerSlice but for
            * app we recieve a virtual address pointer and access data as if
            * a full frame is present there
            */
            virtStartAddr =  Utils_cbufOcmcAlloc
                                    (UTILS_OCMC_RAM1,
                                    pFormat->bpp,
                                    pFormat->width,
                                    VpsUtils_align(pFormat->height, 2),
                                    numLinesPerSlice,
                                    CAPTURE_LINK_NUM_SUBFRAME_PER_CBUF);
            UTILS_assert(virtStartAddr !=NULL);

            //Single plan as its is YUV422I format
            memset(pFrame, 0, sizeof(*pFrame));
            pFrame->chNum = pFormat->chNum;
            pFrame->addr[0][0] = virtStartAddr;
            break;

        case FVID2_DF_YUV420SP_UV:
            /* Since is it YUV420sp format two seperate circular buffers are
             * allocated for Y and UV planes
             */
            memset(pFrame, 0, sizeof(*pFrame));
            pFrame->chNum = pFormat->chNum;
            virtStartAddr =  Utils_cbufOcmcAlloc
                                    (UTILS_OCMC_RAM1,
                                    1,
                                    pFormat->width,
                                    VpsUtils_align(pFormat->height, 2),
                                    numLinesPerSlice,
                                    CAPTURE_LINK_NUM_SUBFRAME_PER_CBUF);
            UTILS_assert(virtStartAddr !=NULL);
            pFrame->addr[0][0] = virtStartAddr;

            virtStartAddr =  Utils_cbufOcmcAlloc
                                    (UTILS_OCMC_RAM1,
                                    1,
                                    pFormat->width,
                                    VpsUtils_align(pFormat->height/2, 2),
                                    numLinesPerSlice/2,
                                    CAPTURE_LINK_NUM_SUBFRAME_PER_CBUF);
            UTILS_assert(virtStartAddr !=NULL);
            /* assign pointer for C plane */
            pFrame->addr[0][1] = (UInt8 *) virtStartAddr;
            break;
        default:
            /* illegal data format */
            status = SYSTEM_LINK_STATUS_EFAIL;
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Allocate frames for capture. Queue allocated frames to capture driver
 *
 *
 *  \param pObj         [IN] Capture link object
 *  \param pDrvObj      [IN] Capture link instance object
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 CaptureLink_subframe_drvAllocAndQueueFrames
                        (CaptureLink_Obj * pObj,CaptureLink_InstObj * pDrvObj)
{
    Int32 status;
    UInt16 streamId;
    FVID2_Frame *frame;
    FVID2_FrameList frameList;
    FVID2_Format format;
    Vps_CaptVipOutInfo *pOutInfo;

    /*
     * for every stream and channel in a capture handle
     */
    for (streamId = 0; streamId < pDrvObj->createArgs.numStream; streamId++)
    {
        pOutInfo = &pDrvObj->vipPrms.outStreamInfo[streamId];

        /*
         * fill format with channel specific values
         */
        format.chNum =
            pDrvObj->createArgs.chNumMap[streamId][0];

        format.width = pDrvObj->bufferWidth;
        format.height = pDrvObj->bufferHeight[0];

        format.pitch[0] = pOutInfo->outFmt.pitch[0];
        format.pitch[1] = pOutInfo->outFmt.pitch[1];
        format.pitch[2] = pOutInfo->outFmt.pitch[2];
        format.fieldMerged[0] = FALSE;
        format.fieldMerged[1] = FALSE;
        format.fieldMerged[2] = FALSE;
        format.dataFormat = pOutInfo->outFmt.dataFormat;
        format.scanFormat = FVID2_SF_PROGRESSIVE;
        format.bpp = FVID2_BPP_BITS8;                  /* ignored */

        if (format.dataFormat == FVID2_DF_RAW_VBI)
        {
                format.height = CAPTURE_LINK_RAW_VBI_LINES;
        }

        frame = &pDrvObj->frames[0];

        /* Allocate frame in OCMC region as circular buffers based on format
        * and number of lines in each subframe.
        */
        status = CaptureLink_subframe_drvAllocOCMCFrame(&format,
                frame,
                pOutInfo->subFrmPrms.numLinesPerSubFrame);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        frame->perFrameCfg = NULL;
        frame->subFrameInfo = NULL;
        frame->appData = NULL;
        frame->reserved = NULL;

        /*
         * Set number of frame in frame list. Only a single frame is allocated
         * and queued.
         */
        frameList.numFrames = 1;
        frameList.frames[0] = frame;


        /*
         * queue the frames in frameList
         */
        status =
            FVID2_queue(pDrvObj->captureVipHandle, &frameList, streamId);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
    return SYSTEM_LINK_STATUS_SOK;
}



/**
 *******************************************************************************
 *
 * \brief Free the allocated frames
 *
 *
 *  \param pObj         [IN] Capture link object
 *  \param pDrvObj      [IN] Capture link instance object
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 CaptureLink_subframe_drvFreeFrames(CaptureLink_Obj * pObj,
                                CaptureLink_InstObj * pDrvObj)
{
    UInt16 streamId, planeCnt;
    FVID2_Frame *pFrame;

    UTILS_assert(pDrvObj->createArgs.numStream
                    <=CAPTURE_LINK_MAX_STREAMS_PER_HANDLE);

    /*
     * for every stream and channel in a capture handle
     */
    for (streamId = 0; streamId < pDrvObj->createArgs.numStream; streamId++)
    {
        pFrame = &pDrvObj->frames[0];

        for (planeCnt = 0; planeCnt < SYSTEM_MAX_PLANES; planeCnt++)
        {
            if((pFrame->addr[0][planeCnt]))
                Utils_cbufOcmcFree(UTILS_OCMC_RAM1, (pFrame->addr[0][planeCnt]));
        }

        Utils_cbufOcmcDeInit(UTILS_OCMC_RAM1);
    }
    return SYSTEM_LINK_STATUS_SOK;
}
