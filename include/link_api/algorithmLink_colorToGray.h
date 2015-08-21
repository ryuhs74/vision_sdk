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
 * \defgroup ALGORITHM_LINK_COLORTOGRAY_API Algorithm Plugin: Color To Gray API
 *
 * \brief  This module has the interface for using color to gray algorithm
 *
 *         Color to gray is a sample algorithm, which just takes a colored video
 *         frame and zeros out chroma component to make it look gray. The
 *         purpose of this sample algorithm is to demonstrate inplace
 *         computation in Links and Chains framework.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_colorToGray.h
 *
 * \brief Algorithm Link API specific to color to gray algorithm on DSP
 *
 * \version 0.0 (Sept 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_COLORTOGRAY_H_
#define _ALGORITHM_LINK_COLORTOGRAY_H_

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
 *   \brief Structure containing create time parameters for color to gray
 *          algorithm
 *
 *          This structure is a replica of create time parameters of color to
 *          gray algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params */
    System_LinkOutQueParams   outQueParams;
    /**< Output queue information */
    System_LinkInQueParams    inQueParams;
    /**< Input queue information */
} AlgorithmLink_ColorToGrayCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for color to gray algorithm
 *
 *          This structure is a replica of control parameters of color to gray
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_ColorToGrayControlParams;

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
 *        to register plugins of color to gray algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_ColorToGray_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
