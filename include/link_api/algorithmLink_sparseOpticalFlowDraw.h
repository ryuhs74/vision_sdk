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
 * \defgroup ALGORITHM_LINK_sparseOpticalFlowDraw Algorithm Plugin: Sparse Optical Flow Draw Link API
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_sparseOpticalFlowDraw.h
 *
 * \brief Plugin that draws sparse optical flow vectors
 *
 * \version 0.0 (Mar 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_SPARSEOPTICALFLOWDRAW_H_
#define _ALGORITHM_LINK_SPARSEOPTICALFLOWDRAW_H_

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
    UInt32                   numOutBuffers;
    /** Number of output Buffers, optional if copy of input is not needed */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */

} AlgorithmLink_sparseOpticalFlowDrawCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_sparseOpticalFlowDrawControlParams;

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
static inline void AlgorithmLink_sparseOpticalFlowDraw_Init(
    AlgorithmLink_sparseOpticalFlowDrawCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_SOF_DRAW;

    pPrm->imgFrameStartX    = 0;
    pPrm->imgFrameStartY    = 0;
    pPrm->imgFrameWidth     = 640;
    pPrm->imgFrameHeight    = 360;
    pPrm->numOutBuffers  = 3;
    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_sparseOpticalFlowDraw_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
