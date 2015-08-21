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
 * \file decLink_algIf.h Decode Link codec private interface structures
 *
 * \brief  Defines Decode Link codec private interface structures
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _DEC_LINK_ALG_IF_H_
#define _DEC_LINK_ALG_IF_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/decLink.h>
#include <ti/sdo/codecs/jpegvdec/ijpegvdec.h>
#include <ti/sdo/codecs/h264vdec/ih264vdec.h>

#define DEC_LINK_H264_VERSION_STRING_MAX_LEN                              (255)
#define DEC_LINK_MPEG4_VERSION_STRING_MAX_LEN                             (255)
#define DEC_LINK_JPEG_VERSION_STRING_MAX_LEN                              (255)
#define DEC_LINK_MAX_NUM_RESOURCE_DESCRIPTOR                              (25)

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Decode link Alg/Codec private create
 *          time parameters
 *
 *******************************************************************************
 */
typedef struct DecLink_AlgCreateParams {
    System_IVideoFormat format;
    Bool  fieldMergeDecodeEnable;
    Int32 maxWidth;
    Int32 maxHeight;
    Int32 maxFrameRate;
    Int32 maxBitRate;
    Int32 presetProfile;
    Int32 presetLevel;
    Int32 displayDelay;
    Int32 processCallLevel;
    Int32 dpbBufSizeInFrames;
    UInt32 decodeFrameType;
} DecLink_AlgCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Decode link Alg/Codec private run-time
 *          or dynamic parameters
 *
 *******************************************************************************
 */
typedef struct DecLink_AlgDynamicParams {
    XDM_DecMode decodeHeader;
    Int32 displayWidth;
    Int32 frameSkipMode;
    Int32 newFrameFlag;
} DecLink_AlgDynamicParams;

#endif

/* Nothing beyond this point */

