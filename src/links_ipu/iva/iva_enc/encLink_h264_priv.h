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
 * \file encLink_h264_priv.h H264 codec private API/Data structures
 *
 * \brief  H264 codec private API/Data structures are defined here
 *         - H264 codec handle object
 *         - All the codec static/dynamic data structures
 *
 * \version 0.0 (Aug 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ENC_LINK_H264_PRIV_H_
#define _ENC_LINK_H264_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/encLink.h>
#include <ti/sdo/codecs/h264enc/ih264enc.h>
#include "encLink_algIf.h"

/**
 *******************************************************************************
 *
 *   \brief Structure containing the H264 Alg/Codec object
 *          This contains the H264 codec handle and other
 *          codec private parameters
 *
 *******************************************************************************
 */
typedef struct EncLink_H264Obj {
    IH264ENC_Handle algHandle;
    Int8 versionInfo[ENC_LINK_ALG_VERSION_STRING_MAX_LEN];
    Int linkID;
    Int channelID;
    Int scratchID;
    UInt32 ivaChID;
    IH264ENC_DynamicParams dynamicParams;
    IH264ENC_Status status;
    IH264ENC_Params staticParams;
    IH264ENC_InArgs inArgs;
    IH264ENC_OutArgs outArgs;
    IVIDEO2_BufDesc inBufs;
    XDM2_BufDesc outBufs;
    IVIDEO_Format format;
    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
} EncLink_H264Obj;

/*******************************************************************************
 *  Encode Link - H264 Private Functions
 *******************************************************************************
 */
Int EncLinkH264_algCreate(EncLink_H264Obj * hObj,
                          EncLink_AlgCreateParams * algCreateParams,
                          EncLink_AlgDynamicParams * algDynamicParams,
                          Int linkID, Int channelID, Int scratchGroupID);
Void EncLinkH264_algDelete(EncLink_H264Obj * hObj);

#endif

/* Nothing beyond this point */
