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
 *
 * \file decLink_jpeg_priv.h MJPEG codec private API/Data structures
 *
 * \brief  MJPEG codec private API/Data structures are defined here
 *         - MJPEG codec handle object
 *         - All the codec static/dynamic data structures
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _DEC_LINK_MJPEG_PRIV_H_
#define _DEC_LINK_MJPEG_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/decLink.h>
#include <ti/sdo/codecs/jpegvdec/ijpegvdec.h>
#include "decLink_algIf.h"

#define JPEG_APP_MARKER_SIZE_MAX    (256)
#define JPEG_APP_MARKER_TAG         (0xE2)

/**
 *******************************************************************************
 *
 *   \brief Structure containing the MJPEG Alg/Codec object
 *          This contains the MJPEG codec handle and other
 *          codec private parameters
 *
 *******************************************************************************
 */
typedef struct DecLink_JPEGObj {
    IJPEGVDEC_Handle algHandle;
    Int8 versionInfo[DEC_LINK_JPEG_VERSION_STRING_MAX_LEN];
    Int linkID;
    Int channelID;
    Int scratchID;
    UInt32 ivaChID;
    IJPEGVDEC_DynamicParams dynamicParams;
    IJPEGVDEC_Status status;
    IJPEGVDEC_Params staticParams;
    IJPEGVDEC_InArgs inArgs;
    IJPEGVDEC_OutArgs outArgs;
    XDM2_BufDesc inBufs;
    XDM2_BufDesc outBufs;
    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
} DecLink_JPEGObj;

/*******************************************************************************
 *  Decode Link - MJPEG Private Functions
 *******************************************************************************
 */
Int DecLinkJPEG_algCreate(DecLink_JPEGObj * hObj,
                          DecLink_AlgCreateParams * algCreateParams,
                          DecLink_AlgDynamicParams * algDynamicParams,
                          Int linkID, Int channelID, Int scratchGroupID,
                          FVID2_Format *pFormat, UInt32 numFrames,
                          IRES_ResourceDescriptor resDesc[]);
Void DecLinkJPEG_algDelete(DecLink_JPEGObj * hObj);

#endif

/* Nothing beyond this point */
