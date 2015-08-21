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
 * \defgroup ALGORITHM_LINK_LANE_DETECT_DRAW Algorithm Plugin: Lane Detect Draw Link API
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_LaneDetectDraw.h
 *
 * \brief Plugin that draws lanes
 *
 * \version 0.0 (Mar 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_LANEDETECTDRAW_H_
#define _ALGORITHM_LINK_LANEDETECTDRAW_H_

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
    /**< Start of actual input to algo relative to start of input buffer */
    UInt32                   imgFrameStartY;
    /**< Start of actual input to algo relative to start of input buffer */
    UInt32                   imgFrameWidth;
    /**< Width of frame given to the PD algo */
    UInt32                   imgFrameHeight;
    /**< Height of the frame given to the PD algo */
    UInt32                   laneThickness;
    /**< THickness in pixels of the lane */
    UInt32                   enableDrawLines;
    /**< Set TRUE for draw connected lines,
      *  Set as FALSE for draw individual Pixel points */
    UInt32                   numOutBuffers;
    /** Number of output Buffers, optional if copy of input is not needed */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */

} AlgorithmLink_LaneDetectDrawCreateParams;

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
} AlgorithmLink_LaneDetectDrawControlParams;

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
static inline void AlgorithmLink_LaneDetectDraw_Init(
    AlgorithmLink_LaneDetectDrawCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_LANE_DETECT_DRAW;

    pPrm->imgFrameStartX    = 0;
    pPrm->imgFrameStartY    = 0;
    pPrm->imgFrameWidth     = 640;
    pPrm->imgFrameHeight    = 360;
    pPrm->enableDrawLines   = TRUE;
    pPrm->numOutBuffers  = 3;
    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;
    pPrm->laneThickness = 1;
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
Int32 AlgorithmLink_laneDetectDraw_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
