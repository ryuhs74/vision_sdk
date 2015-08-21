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
 * \defgroup ALGORITHM_LINK_FEATUREPLANECOMPUTATION Algorithm Plugin: Feature Plane Computation API
 *
 * \brief  This module has the interface for using feature plane comp algorithm
 *
 *         Feature Plane Computation algorithm link -
 *             1. This alg takes input from previous link and computes
 *                feature planes for luma and chroma components
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_featurePlaneComputation.h
 *
 * \brief Algorithm Link API specific to feature plane computation algorithm
 *
 * \version 0.0 (Feb 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_FEATUREPLANECOMPUTATION_H_
#define _ALGORITHM_LINK_FEATUREPLANECOMPUTATION_H_

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
 *   \ingroup LINK_API_CMD
 *   \addtogroup ALGORITHM_LINK_FEATURE_PLANE_COMPUTE_CMD Algorithm Plugin: Feature Plane Compute Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Alg Link Config CMD: Set ROI parameters
 *
 *   \param AlgorithmLink_FeaturePlaneComputationSetROIParams [IN]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_FEATURE_PLANE_COMPUTE_CMD_SET_ROI      (0x1000)

/* @} */

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
 *   \brief Parameters to set specific ROI
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    UInt32                   imgFrameStartX;
    /**< Start of actual video relative to start of input buffer */
    UInt32                   imgFrameStartY;
    /**< Start of actual video relative to start of input buffer */
    UInt32                   imgFrameWidth;
    /**< Width of the input frame */
    UInt32                   imgFrameHeight;
    /**< Height of the input frame */
} AlgorithmLink_FeaturePlaneComputationSetROIParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for feature plane
 *          computation algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params. This structure should be first element */
    UInt32                   imgFrameStartX;
    /**< Start of actual video relative to start of input buffer */
    UInt32                   imgFrameStartY;
    /**< Start of actual video relative to start of input buffer */
    UInt32                   imgFrameWidth;
    /**< Width of the input frame */
    UInt32                   imgFrameHeight;
    /**< Height of the input frame */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */
    UInt32                   numOutBuffers;
    /**< Number of output buffers */
    UInt32                   roiEnable;
    /**< FALSE: Full ROI is assumed, TRUE: roiXxxx parameters are used */
    UInt32                   roiCenterX;
    /**< Center of ROI relative to imgFrameStartX, imgFrameStartY */
    UInt32                   roiCenterY;
    /**< Center of ROI relative to imgFrameStartX, imgFrameStartY */
    UInt32                   roiWidth;
    /**< Width of ROI relative to imgFrameStartX, imgFrameStartY */
    UInt32                   roiHeight;
    /**< Height of ROI relative to imgFrameStartX, imgFrameStartY */
    UInt32                   numScales;
    /**< Number of scales to use for image pyramid */
} AlgorithmLink_FeaturePlaneComputationCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Geometric Alignment
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_FeaturePlaneComputationControlParams;

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
static inline void AlgorithmLink_FeatureComputation_Init(
    AlgorithmLink_FeaturePlaneComputationCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_FEATUREPLANECOMPUTE;

    pPrm->imgFrameStartX = 0;
    pPrm->imgFrameStartY = 0;
    pPrm->imgFrameHeight = 768;
    pPrm->imgFrameWidth  = 488;
    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;
    pPrm->numOutBuffers             = 4;
    pPrm->roiEnable                 = FALSE;
    pPrm->numScales                 = 17;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of feature plane computation algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_featurePlaneComputation_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
