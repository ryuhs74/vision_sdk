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
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_STEREO_POST_PROCESS Algorithm Plugin: Stereo Stereo Post Process API
 *
 * \brief  This module has the interface for post-processing algorithm
 *         used in Stereo processing
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_stereoPostProcess.h
 *
 * \brief Algorithm Link API specific to stereo post process algorithm and visualisation
 *
 * \version 0.0 (Oct 2014) : [VT] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_STEREO_POST_PROCESS_H_
#define _ALGORITHM_LINK_STEREO_POST_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To update post processing's dynamic parameters
 *
 *
 *******************************************************************************
 */
#define STEREO_POSTPROCESS_LINK_CMD_SET_DYNAMIC_PARAMS             (0x5000)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for stereo post process and visualisation
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params. This structure should be first element */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */
    UInt32                   numOutBuffers;
    /**< Number of output Buffers */

    UInt32 maxImageRoiWidth;
    UInt32 maxImageRoiHeight;
    UInt32 inputBitDepth;
    UInt32 censusWinWidth;
    UInt32 censusWinHeight;
    UInt32 disparityWinWidth;
    UInt32 disparityWinHeight;
    UInt32 numDisparities;
    UInt32 disparityStep;
    UInt32 costMaxThreshold;
    UInt32 minConfidenceThreshold;
    UInt32 holeFillingStrength;
    UInt32 textureLumaHiThresh;  /* 0 - 100 */
    UInt32 textureLumaLoThresh;  /* 0 - 100 */
    UInt32 textureThreshold;
    UInt32 lrMaxDiffThreshold;
    UInt32 maxDispDissimilarity;
    UInt32 minConfidentNSegment;
    UInt32 censusSrcImageWidth;
    UInt32 censusSrcImageHeight;
    UInt32 temporalFilterNumFrames;
    UInt32 minDisparityToDisplay;
    UInt32 colorMapIndex; /*0: multi-color, 1: blue */
    UInt32 disparityExtraRightLeft;
    UInt32 disparitySearchDir;
    UInt32 imageStartX;
    UInt32 imageStartY;

#if 0
    UInt32                   censusSrcImageWidth;
    /**< Width of the input frame */
    UInt32                   censusSrcImageHeight;
    /**< Height of the input frame */
    UInt32                   censusImageRoiWidth;
    /**< Width of the input frame */
    UInt32                   censusImageRoiHeight;
    /**< Height of the input frame */
#endif
} AlgorithmLink_StereoPostProcessCreateParams;

/*NOTE: This struct is also defined in netwrork_ctrl tools in network_ctrl_handle_stereo_set_dynamic_params.c
Please make sure the two are matching, if any changes are made.
*/
typedef struct
{
    UInt32 postproc_cost_max_threshold;
    UInt32 postproc_conf_min_thrseshold;
    UInt32 postproc_texture_lumalothresh;
    UInt32 postproc_texture_lumahithresh;
    UInt32 postproc_texture_threshold;
    UInt32 postproc_lrmaxdiff_threshold;
    Uint32 postproc_maxdisp_dissimilarity;
    UInt32 postproc_minconf_nseg_threshold;
} AlgorithmLink_StereoPostProcessDynamicParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for  stereo post process algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
    AlgorithmLink_StereoPostProcessDynamicParams stereoParams;
    /**< Dynamic stereo params */
} AlgorithmLink_StereoPostProcessControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Set defaults for plugin create parameters
 *
 * \param pPrm  [OUT] plugin create parameters
 *
 *******************************************************************************
 */
static inline void AlgorithmLink_StereoPostProcess_Init(
    AlgorithmLink_StereoPostProcessCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_STEREO_POST_PROCESS;

    pPrm->numDisparities    = 64;

    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;
    pPrm->numOutBuffers             = 4;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of soft isp algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_StereoPostProcess_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
