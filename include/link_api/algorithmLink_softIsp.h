/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_SOFTISP Algorithm Plugin: Software ISP for RCCC to Y conversion API
 *
 * \brief  This module has the interface for using soft isp algorithm
 *
 *         Soft Isp algorithm link -
 *             1. This alg takes input from previous link and does CFA
 *              interpolation of RCCC data to generate luma components
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_softIsp.h
 *
 * \brief Algorithm Link API specific to feature plane computation algorithm
 *
 * \version 0.0 (Sep 2014) : [SR] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_SOFTISP_H_
#define _ALGORITHM_LINK_SOFTISP_H_

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
    UInt32                   imageWidth;
    /**< Width of the input frame */
    UInt32                   imageHeight;
    /**< Height of the input frame */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */
    UInt32                   numOutBuffers;
    /**< Number of output Buffers */
    UInt32                  enableExtractR;
    /**< enables extraction of Red pixels */
    UInt32                  enableStats;
    /**< enables statistics data generation */
} AlgorithmLink_SoftIspCreateParams;

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
} AlgorithmLink_SoftIspControlParams;

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
static inline void AlgorithmLink_SoftIsp_Init(
    AlgorithmLink_SoftIspCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_SOFTISP;

    pPrm->imageWidth = 1280;
    pPrm->imageHeight  = 664;
    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;

    pPrm->numOutBuffers             = 4;

    pPrm->enableExtractR            = 0;
    pPrm->enableStats               = 0;
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
Int32 AlgorithmLink_softIsp_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
