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
 * \defgroup ALGORITHM_LINK_EDGEDETECTION_API Algorithm Plugin: Edge Detect API
 *
 * \brief  This module has the interface for using edge detection algorithm
 *
 *         Frame copy is a sample algorithm, which just copies a frame from
 *         input to output buffer.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_edgeDetection.h
 *
 * \brief Algorithm Link API specific to edge detection algorithm on DSP
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_EDGEDETECTION_H_
#define _ALGORITHM_LINK_EDGEDETECTION_H_

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

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Frame copy algorithm
 *
 *          This structure is a replica of create time parameters of edge detection
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params */
    UInt32                    maxHeight;
    /**< Max height of the frame */
    UInt32                    maxWidth;
    /**< max width of the frame */
    UInt32                    numOutputFrames;
    /**< Number of output frames to be created for this link per channel*/
    System_LinkOutQueParams   outQueParams;
    /**< Output queue information */
    System_LinkInQueParams    inQueParams;
    /**< Input queue information */
} AlgorithmLink_EdgeDetectionCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Frame copy algorithm
 *
 *          This structure is a replica of control parameters of edge detection
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_EdgeDetectionControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of edge detection algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_EdgeDetection_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
