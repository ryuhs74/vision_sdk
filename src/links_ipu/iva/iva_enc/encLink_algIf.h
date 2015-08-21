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
 * \file encLink_algIf.h Encode Link codec private interface structures
 *
 * \brief  Defines Encode Link codec private interface structures
 *
 * \version 0.0 (April 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ENC_LINK_ALG_IF_H_
#define _ENC_LINK_ALG_IF_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/encLink.h>
#include <ti/sdo/codecs/jpegvenc/ijpegenc.h>
#include "encLink_priv.h"

#define ENC_LINK_ALG_VERSION_STRING_MAX_LEN                               (255)

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Encode link Alg/Codec private create
 *          time parameters
 *
 *******************************************************************************
 */
typedef struct EncLink_AlgCreateParams {
    System_IVideoFormat format;
    IVIDEO_VideoLayout dataLayout;
    Bool singleBuf;
    Int32 maxWidth;
    Int32 maxHeight;
    Int32 maxInterFrameInterval;
    Int32 inputContentType;
    Int32 inputChromaFormat;
    Int32 profile;
    Int32 level;
    Int32 enableAnalyticinfo;
    Int32 enableSVCExtensionFlag;
    Int32 enableWaterMarking;
    Int32 mvDataSize;
    Int32 maxBitRate;
    Int32 encodingPreset;
    Int32 rateControlPreset;
    Bool enableHighSpeed;
    Int32 numTemporalLayer;
} EncLink_AlgCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Encode link Alg/Codec ROI parameters
 *
 *******************************************************************************
 */
typedef struct EncLink_AlgROIDynamicParams
{
    Int32 roiNumOfRegion;
    /**< Number of ROI's */
    Int32 roiStartX[ENC_LINK_CURRENT_MAX_ROI];
    /**< starting location X coordinate of this region */
    Int32 roiStartY[ENC_LINK_CURRENT_MAX_ROI];
    /**< starting location Y coordinate of this region */
    Int32 roiWidth[ENC_LINK_CURRENT_MAX_ROI];
    /**< Width of this ROI */
    Int32 roiHeight[ENC_LINK_CURRENT_MAX_ROI];
    /**< Height of this ROI */
    Int32 roiType[ENC_LINK_CURRENT_MAX_ROI];
    /**< ROI type */
    Int32 roiPriority[ENC_LINK_CURRENT_MAX_ROI];
} EncLink_AlgROIDynamicParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Encode link Alg/Codec private run-time
 *          or dynamic parameters
 *
 *******************************************************************************
 */
typedef struct EncLink_AlgDynamicParams {
    Int32 startX;
    Int32 startY;
    Int32 inputWidth;
    Int32 inputHeight;
    Int32 inputPitch;
    Int32 targetBitRate;
    Int32 targetFrameRate;
    Int32 intraFrameInterval;
    Int32 interFrameInterval;
    Int32 rcAlg;
    Int32 qpMinI;
    Int32 qpMaxI;
    Int32 qpInitI;
    Int32 qpMinP;
    Int32 qpMaxP;
    Int32 qpInitP;
    Int32 forceFrame;
    Int32 vbrDuration;
    Int32 vbrSensitivity;
    Bool  forceFrameStatus;
    Int32 mvAccuracy;
    Int32 refFrameRate;

    EncLink_AlgROIDynamicParams roiParams;
} EncLink_AlgDynamicParams;

#endif

/* Nothing beyond this point */

