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
 * \defgroup ALGORITHM_LINK_DENSEOPTICALFLOW_API Algorithm Plugin: Dense\
 * Optical Flow API
 *
 * \brief  This module has the interface for using Dense Optical Flow algorithm
 *
 *         Dense Optical Flow algorithm link -
 *         - Takes the YUV video frame and process pixel by pixel.
 *         - Generate the horizontal and versical optical flow vectors.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_denseOpticalFlow.h
 *
 * \brief Algorithm Link API specific to Dense Optical Flow algorithm
 *
 * \version 0.0 (Nov 2013) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_DENSEOPTICALFLOW_H_
#define _ALGORITHM_LINK_DENSEOPTICALFLOW_H_

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

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief enum indication the number of levels of the image pyramid to use.
 *
 *
 *******************************************************************************
*/
typedef enum{
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_1 = 1,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_2,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_3,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_4,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_5,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_MAX = 5,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_DEFAULT = 3,
    ALGLINK_DENSEOPTFLOW_LKNUMPYR_FORCE32BITS = 0x7FFFFFFF
} AlgorithmLink_DenseOptFlowLKnumPyr;

/**
 *******************************************************************************
 *
 *   \brief enum to set the size of the smoothing kernel used to preprocess
 *
 *
 *******************************************************************************
*/
typedef enum{
    ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_3x3 = 3,
    ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_5x5 = 5,
    ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_7x7 = 7,
    ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_MAX = 7,
    ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_DEFAULT = 3,
    ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_FORCE32BITS = 0x7FFFFFFF
} AlgorithmLink_DenseOptFlowLKsmoothSize;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Dense Optical Flow
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params. This structure should be first element */
    AlgorithmLink_DenseOptFlowLKnumPyr      numPyramids;
    /**< Number of levels in the pyramid used in the algorithm */
    UInt32                   enableSmoothing;
    /**< Enables image smoothing before processing. 1:enable 0:disable */
    AlgorithmLink_DenseOptFlowLKsmoothSize  smoothingSize;
    /**< Sets the width and height of the smoothing kernel.
      *  Setting enableSmoothing to 0 will cause this value to be unused.*/
    UInt32                   maxVectorSizeX;
    /**< Clips u vectors (horizontal) to this magnitude */
    UInt32                   maxVectorSizeY;
    /**< Clips v vectors (vertical) to this magnitude */
    UInt32                   processPeriodicity;
    /**< Indicates the periodicity of processing input. If set as four, it
      *  means once every four input frames, actual algorithm processing
      *  will be done
      *
      *  ONLY valid when AlgorithmLink_DenseOptFlowCreateParams.roiEnable
      *  is FALSE
      */
    UInt32                   processStartFrame;
    /**< Indicates the first frame (Input) number which should be picked up
      *  for processing. Frame (Input) number begins from 0
      *
      *  ONLY valid when AlgorithmLink_DenseOptFlowCreateParams.roiEnable
      *  is FALSE
      */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */

    UInt32                   numOutBuf;
    /**< Number of output buffer to allocate */

    UInt32                   algEnable;
    /**< Debug flag to control algorithm enable/disable */

    UInt32                   roiEnable;
    /**< Enable ROI based processing */

    AlgorithmLink_RoiParams  roiParams;
    /**< ROI params */

} AlgorithmLink_DenseOptFlowCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Dense Optical Flow
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_DenseOptFlowControlParams;

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
static inline void AlgorithmLink_DenseOptFlowCreateParams_Init(
    AlgorithmLink_DenseOptFlowCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_DENSE_OPTICAL_FLOW;

    pPrm->numPyramids     = ALGLINK_DENSEOPTFLOW_LKNUMPYR_1;
    pPrm->enableSmoothing = TRUE;
    pPrm->smoothingSize   = ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_3x3;
    pPrm->maxVectorSizeX  = 16;
    pPrm->maxVectorSizeY  = 16;
    pPrm->processPeriodicity = 1;
    pPrm->processStartFrame = 1;

    pPrm->inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;

    pPrm->outQueParams.nextLink = SYSTEM_LINK_ID_INVALID;

    pPrm->numOutBuf = 4;

    pPrm->roiEnable = FALSE;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of Dense Optical Flow algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_DenseOptFlow_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */

