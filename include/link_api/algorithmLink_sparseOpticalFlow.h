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
 * \defgroup ALGORITHM_LINK_SPARSEOPTICALFLOW Algorithm Plugin: Sparse Optical Flow API
 *
 * \brief  This module has the interface for using SOF algorithm
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_sparseOpticalFlow.h
 *
 * \brief Algorithm Link API specific to feature plane computation algorithm
 *
 * \version 0.0 (Feb 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_SPARSEOPTICALFLOW_H_
#define _ALGORITHM_LINK_SPARSEOPTICALFLOW_H_

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
    UInt32                   imgFrameStartX;
    /**< Start of actual video relative to start of input buffer */
    UInt32                   imgFrameStartY;
    /**< Start of actual video relative to start of input buffer */
    UInt32                   imgFrameWidth;
    /**< Width of the input frame */
    UInt32                   imgFrameHeight;
    /**< Height of the input frame */

} AlgorithmLink_SparseOpticalFlowCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for SOF algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_SparseOpticalFlowControlParams;

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
static inline void AlgorithmLink_SparseOpticalFlow_Init(
    AlgorithmLink_SparseOpticalFlowCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_SPARSE_OPTICAL_FLOW;

    pPrm->inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->numOutBuffers = 3;
    pPrm->outQueParams.nextLink = SYSTEM_LINK_ID_INVALID;

    pPrm->imgFrameStartX    = 0;
    pPrm->imgFrameStartY    = 0;
    pPrm->imgFrameWidth     = 640;
    pPrm->imgFrameHeight    = 360;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of sparse optical flow algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_sparseOpticalFlow_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
