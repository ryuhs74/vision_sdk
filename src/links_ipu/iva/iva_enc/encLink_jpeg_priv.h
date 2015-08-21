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
 * \file encLink_jpeg_priv.h MJPEG codec private API/Data structures
 *
 * \brief  MJPEG codec private API/Data structures are defined here
 *         - MJPEG codec handle object
 *         - All the codec static/dynamic data structures
 *
 * \version 0.0 (April 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ENC_LINK_JPEG_PRIV_H_
#define _ENC_LINK_JPEG_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/encLink.h>
#include <ti/sdo/codecs/jpegvenc/ijpegenc.h>

#include "encLink_algIf.h"

/**
 *******************************************************************************
 *  Analytic info output buffer size, this buffer is used to place MV & SAD of
 *  encoded frame, should be big enough to hold the size of  typical HD sequence
 *******************************************************************************
 */
#define ANALYTICINFO_OUTPUT_BUFF_SIZE      (0x00028000)
typedef struct IJPEGVENC_Obj *JPEGVENC_Handle;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the MJPEG Alg/Codec object
 *          This contains the MJPEG codec handle and other
 *          codec private parameters
 *
 *******************************************************************************
 */
typedef struct EncLink_JPEGObj {
    JPEGVENC_Handle algHandle;
    Int8 versionInfo[ENC_LINK_ALG_VERSION_STRING_MAX_LEN];
    Int linkID;
    Int channelID;
    Int scratchID;
    UInt32 ivaChID;
    IJPEGVENC_DynamicParams dynamicParams;
    IJPEGVENC_Status status;
    IJPEGVENC_Params staticParams;
    IJPEGVENC_InArgs inArgs;
    IJPEGVENC_OutArgs outArgs;
    IVIDEO2_BufDesc inBufs;
    XDM2_BufDesc outBufs;
    System_IVideoFormat format;
    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
} EncLink_JPEGObj;

/*******************************************************************************
 *  Encode Link - MJPEG Private Functions
 *******************************************************************************
 */
Int EncLinkJPEG_algCreate(EncLink_JPEGObj * hObj,
                          EncLink_AlgCreateParams * algCreateParams,
                          EncLink_AlgDynamicParams * algDynamicParams,
                          Int linkID, Int channelID, Int scratchGroupID);
Void EncLinkJPEG_algDelete(EncLink_JPEGObj * hObj);

#endif

/* Nothing beyond this point */
