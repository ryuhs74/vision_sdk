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
 * \defgroup ALGORITHM_LINK_OBJECTDETECTION Algorithm Plugin: Object detection API
 *
 * \brief  This module has the interface for using feature plane classification
 *         algorithm
 *         Feature Plane classification algorithm link -
 *             1. This alg takes input from feature plane compute link
 *                and generates co-ordinates of the rectangles where
 *                objects are present in the frame
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_objectDetection.h
 *
 * \brief Algorithm Link API specific to feature plane classification algorithm
 *
 * \version 0.0 (Feb 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_OBJECTDETECTION_H_
#define _ALGORITHM_LINK_OBJECTDETECTION_H_

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
 *   \addtogroup ALGORITHM_LINK_OBJETC_DETECT_CMD Algorithm Plugin: Object Detect Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Alg Link Config CMD: Enable/Disable algorithm
 *
 *   \param AlgorithmLink_ObjectDetectEnableAlgParams [IN]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_OBJECT_DETECT_CMD_ENABLE_ALG     (0x2000)

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

    UInt32                   enablePD;
    /**< Enabled detection of Pedestrains */
    UInt32                   enableTSR;
    /**< Enabled detection of Traffic signs */
} AlgorithmLink_ObjectDetectEnableAlgParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for feature plane
 *          classification algorithm
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
    /**< Number of output buffers */
    UInt32                   enablePD;
    /**< Enabled detection of Pedestrains */
    UInt32                   enableTSR;
    /**< Enabled detection of Traffic signs */
} AlgorithmLink_ObjectDetectionCreateParams;

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
} AlgorithmLink_ObjectDetectionControlParams;

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
static inline void AlgorithmLink_ObjectDetection_Init(
    AlgorithmLink_ObjectDetectionCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size      = sizeof(*pPrm);
    pPrm->baseClassCreate.algId     = ALGORITHM_LINK_DSP_ALG_OBJECTDETECTION;
    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;
    pPrm->numOutBuffers             = 4;
    pPrm->enablePD                  = TRUE;
    pPrm->enableTSR                 = FALSE;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of feature plane classification algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_ObjectDetection_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
