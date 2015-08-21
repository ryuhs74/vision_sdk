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
 * \defgroup ALGORITHM_LINK_CENSUS Algorithm Plugin: Census API
 *
 * \brief  This module has the interface for census algorithm used in Stereo
 *         processing
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_census.h
 *
 * \brief Algorithm Link API specific to feature plane computation algorithm
 *
 * \version 0.0 (Sept 2014) : [SR] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_CENSUS_H_
#define _ALGORITHM_LINK_CENSUS_H_

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
    UInt32                   srcImageWidth;
    /**< Width of the input frame */
    UInt32                   srcImageHeight;
    /**< Height of the input frame */
    UInt32                   imageRoiWidth;
    /**< Width of the input frame */
    UInt32                   imageRoiHeight;
    /**< Height of the input frame */
    UInt32                   inputBitDepth;
    /**< Input frame bit depth */
    UInt32                   winWidth;
    /**< Window Width */
    UInt32                   winHeight;
    /**< Window Height */
    UInt32                   winHorzStep;
    /**< Window Horizontal Step size */
    UInt32                   winVertStep;
    /**< Window Vertical Step size */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */
    UInt32                   numOutBuffers;
    /**< Number of output Buffers */
} AlgorithmLink_CensusCreateParams;

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
} AlgorithmLink_CensusControlParams;

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
static inline void AlgorithmLink_Census_Init(
                                AlgorithmLink_CensusCreateParams *pPrm)
{
   // VC: If we can pass AlgorithmLink_DisparityHamDistCreateParams as a second argument then below values can be taken from it instead of being
   // hardcoded
   UInt32 finalRoiWidth= 640; // VC, ideally should be disparityHamDistCreateParams->imageRoiWidth
   UInt32 finalRoiHeight= 360; // VC, ideally should be disparityHamDistCreateParams->imageRoiHeight
   UInt32 numDisparities= 64; // VC, ideally should be disparityHamDistCreateParams->numDisparities
   Uint32 disparityWinWidth= 15; // VC, ideally should be disparityHamDistCreateParams->winWidth
   Uint32 disparityWinHeight= 15; // VC, ideally should be disparityHamDistCreateParams->winHeight

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_CENSUS;

    pPrm->imageRoiWidth     = ((finalRoiWidth + numDisparities - 1 + disparityWinWidth - 1 + 15)>>4)<<4; // VC
    pPrm->imageRoiHeight     = ((finalRoiHeight + disparityWinHeight - 1 + 7)>>3)<<3; // VC

    pPrm->inputBitDepth     = 8; /*YUV420 format*/
    pPrm->winWidth          = 9;
    pPrm->winHeight         = 9;
    pPrm->winHorzStep       = 2;
    pPrm->winVertStep       = 2;

    pPrm->srcImageWidth     = 800; // VC: must match ISP output's width and must be >= pPrm->imageRoiWidth + pPrm->winWidth - 1;
    pPrm->srcImageHeight    = 450; // VC: must match ISP output's height and must be >= pPrm->imageRoiHeight + pPrm->winHeight - 1;

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
Int32 AlgorithmLink_census_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
