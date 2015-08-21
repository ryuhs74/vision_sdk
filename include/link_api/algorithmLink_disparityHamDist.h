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
 * \defgroup ALGORITHM_LINK_DISPARITY_HAMDIST Algorithm Plugin: Disparity Haming Distance Computation API
 *
 * \brief  This module has the interface for Disparity Haming distance algorithm
 *         used in Stereo processing
 *         This alg takes input from Census transform Link and does the disparity
 *         computation
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_disparityHamDist.h
 *
 * \brief Algorithm Link API specific to feature plane computation algorithm
 *
 * \version 0.0 (Sept 2014) : [SR] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_DISPARITY_HAMDIST_H_
#define _ALGORITHM_LINK_DISPARITY_HAMDIST_H_

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
    UInt32                   srcImageWidth; // VC
    /**< Width of the input frame's ROI */
    UInt32                   srcImageHeight; // VC
    /**< Height of the input frame's ROI */
    UInt32                   imageRoiWidth; // VC
    /**< Width of the input frame */
    UInt32                   imageRoiHeight; // VC
    /**< Height of the input frame */
    UInt32                  inputBitDepth;
    /**< Input bit depth length */
    UInt32                  winWidth;
    /**< Window width size */
    UInt32                  winHeight;
    /**< Window height size */
    UInt32                  numDisparities;
    /**< Number of disparities */
    UInt32                  disparityStep;
    /**< Disparity step size */
    UInt32                  searchDir;
    /**< Search direction */
    UInt32                  extraRightLeftMap;
    /**< Extra Right Left map for left-right check */
    UInt32                  inputLeftImageStartX;
    /**< X offset for left frame */
    UInt32                  inputLeftImageStartY;
    /**< Y offset for left frame */
    UInt32                  inputRightImageStartX;
    /**< X offset for right frame */
    UInt32                  inputRightImageStartY;
    /**< Y offset for right frame */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */
    UInt32                   numOutBuffers;
    /**< Number of output Buffers */
} AlgorithmLink_DisparityHamDistCreateParams;

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
} AlgorithmLink_DisparityHamDistControlParams;

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
static inline void AlgorithmLink_DisparityHamDist_Init(
        AlgorithmLink_DisparityHamDistCreateParams *pPrm)
{
    UInt32 finalRoiWidth= 640; // VC
    UInt32 finalRoiHeight= 360; // VC

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_DISPARITY_HAMDIST;

    pPrm->imageRoiWidth       = finalRoiWidth; // VC
    pPrm->imageRoiHeight      = finalRoiHeight; // VC
    pPrm->inputBitDepth     = 32;
    pPrm->winWidth          = 15;
    pPrm->winHeight         = 15;
    pPrm->numDisparities    = 64;
    pPrm->disparityStep     = 2;

    pPrm->srcImageWidth
    = ((finalRoiWidth+pPrm->numDisparities-1 + pPrm->winWidth - 1 + 15)>>4)<<4; // VC
    pPrm->srcImageHeight  = ((finalRoiHeight + pPrm->winHeight - 1 + 7)>>3)<<3; // VC

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
Int32 AlgorithmLink_disparityHamDist_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
