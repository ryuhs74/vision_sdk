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
 * \file decLink_h264_priv.h H264 dec codec private API/Data structures
 *
 * \brief  H264 DEC codec private API/Data structures are defined here
 *         - H264 codec handle object
 *         - All the codec static/dynamic data structures
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _DEC_LINK_H264_PRIV_H_
#define _DEC_LINK_H264_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/decLink.h>
#include <ti/sdo/codecs/h264vdec/ih264vdec.h>
#include "decLink_algIf.h"

/**
 *******************************************************************************
 *
 *   \brief Structure containing the H264 Alg/Codec object
 *          This contains the H264 codec handle and other
 *          codec private parameters
 *
 *******************************************************************************
 */
typedef struct DecLink_H264Obj {
    IH264VDEC_Handle algHandle;
    Int8 versionInfo[DEC_LINK_H264_VERSION_STRING_MAX_LEN];
    Int linkID;
    Int channelID;
    Int scratchID;
    UInt32 ivaChID;
    IH264VDEC_DynamicParams dynamicParams;
    IH264VDEC_Status status;
    IH264VDEC_Params staticParams;
    IH264VDEC_InArgs inArgs;
    IH264VDEC_OutArgs outArgs;
    XDM2_BufDesc inBufs;
    XDM2_BufDesc outBufs;
    UInt32 numProcessCalls;
    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
} DecLink_H264Obj;

/*******************************************************************************
 *  Decode Link - H264 Private Functions
 *******************************************************************************
 */
Int DecLinkH264_algCreate(DecLink_H264Obj * hObj,
                          DecLink_AlgCreateParams * algCreateParams,
                          DecLink_AlgDynamicParams * algDynamicParams,
                          Int linkID, Int channelID, Int scratchGroupID,
                          FVID2_Format *pFormat, UInt32 numFrames,
                          IRES_ResourceDescriptor resDesc[]);
Void DecLinkH264_algDelete(DecLink_H264Obj * hObj);

#endif

/* Nothing beyond this point */

