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
 * \file algorithmLink_cfg.c
 *
 * \brief  This file has some configuration of algorithm link
 *
 *         Functions in this file will be called by use case or application
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "algorithmLink_priv.h"
#include "algorithmLink_cfg.h"
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_edgeDetection.h>
#include <include/link_api/algorithmLink_frameCopy.h>
#include <include/link_api/algorithmLink_fullView.h>
#include <include/link_api/algorithmLink_dmaSwMs.h>
#include <include/link_api/algorithmLink_colorToGray.h>
#include <include/link_api/algorithmLink_synthesis.h>
#include <include/link_api/algorithmLink_geometricAlignment.h>
#include <include/link_api/algorithmLink_photoAlignment.h>
#include <include/link_api/algorithmLink_ultrasonicFusion.h>
#include <include/link_api/algorithmLink_denseOpticalFlow.h>
#include <include/link_api/algorithmLink_vectorToImage.h>
#include <include/link_api/algorithmLink_featurePlaneComputation.h>
#include <include/link_api/algorithmLink_objectDetection.h>
#include <include/link_api/algorithmLink_objectDraw.h>
#include <include/link_api/algorithmLink_sparseOpticalFlow.h>
#include <include/link_api/algorithmLink_sparseOpticalFlowDraw.h>
#include <include/link_api/algorithmLink_laneDetect.h>
#include <include/link_api/algorithmLink_laneDetectDraw.h>
#include <include/link_api/algorithmLink_subframeCopy.h>
#include <include/link_api/algorithmLink_softIsp.h>
#include <include/link_api/algorithmLink_remapMerge.h>
#include <include/link_api/algorithmLink_issAewb.h>
#include <include/link_api/algorithmLink_census.h>
#include <include/link_api/algorithmLink_disparityHamDist.h>
#include <include/link_api/algorithmLink_stereoPostProcess.h>
#include <include/link_api/algorithmLink_crc.h>

/*******************************************************************************
 *  Declaring gAlgorithmLinkFuncTable for the current core
 *******************************************************************************
 */
AlgorithmLink_FuncTable gAlgorithmLinkFuncTable[ALGORITHM_LINK_ALG_MAXNUM];

/**
 *******************************************************************************
 * \brief Display Link object, stores all link related information
 *******************************************************************************
 */
AlgorithmLink_Obj gAlgorithmLink_obj[ALGORITHM_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief Initializing alg plugins
 *
 *        This function needs to be called by the use case / application.
 *        This function will inturn call the algorithm specific init functions
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_initAlgPlugins()
{
    memset(gAlgorithmLinkFuncTable, 0, sizeof(gAlgorithmLinkFuncTable));

#ifdef BUILD_DSP
    /** For all alorithms on Dsp */
    AlgorithmLink_FrameCopy_initPlugin();
    AlgorithmLink_FullView_initPlugin();
    AlgorithmLink_ColorToGray_initPlugin();
    AlgorithmLink_DmaSwMs_initPlugin();
    AlgorithmLink_ObjectDetection_initPlugin();
    AlgorithmLink_vectorToImage_initPlugin();
    AlgorithmLink_sparseOpticalFlowDraw_initPlugin();
    AlgorithmLink_laneDetect_initPlugin();
    AlgorithmLink_laneDetectDraw_initPlugin();
    /** For 64MB DDR configuration on TDA3xx, surround view is not included, since surround view
      * significant DDR memory needs */
#ifndef TDA3XX_64MB_DDR
    AlgorithmLink_Synthesis_initPlugin();
    AlgorithmLink_pAlign_initPlugin();
    AlgorithmLink_gAlign_initPlugin();
    AlgorithmLink_UltrasonicFusion_initPlugin();
#endif

    #ifdef TDA2XX_FAMILY_BUILD
    AlgorithmLink_StereoPostProcess_initPlugin();
    #endif
#endif

#ifdef BUILD_ARP32
    /** For all alorithms on Eve */
    AlgorithmLink_FrameCopy_initPlugin();
    AlgorithmLink_FullView_initPlugin();
    AlgorithmLink_softIsp_initPlugin();
    #ifdef TDA2XX_FAMILY_BUILD
    AlgorithmLink_census_initPlugin();
    AlgorithmLink_disparityHamDist_initPlugin();
    #endif
    AlgorithmLink_EdgeDetection_initPlugin();
    AlgorithmLink_DenseOptFlow_initPlugin();
    AlgorithmLink_featurePlaneComputation_initPlugin();
    AlgorithmLink_sparseOpticalFlow_initPlugin();
    AlgorithmLink_SubframeCopy_initPlugin();
    #ifdef TDA2XX_FAMILY_BUILD
        #ifdef BUILD_ARP32_2
        AlgorithmLink_RemapMerge_initPlugin();
        #endif
    #endif
#endif

#ifdef BUILD_M4
    /** For all alorithms on IPU (M4) */
    AlgorithmLink_DmaSwMs_initPlugin();
#endif

#ifdef BUILD_M4_0
    /** For all alorithms on IPU (M4) */
    AlgorithmLink_objectDraw_initPlugin();

    #ifdef ISS_INCLUDE
    AlgorithmLink_issAewb1_initPlugin();
    #endif

    #ifdef CRC_INCLUDE
    AlgorithmLink_Crc_initPlugin();
    #endif
#endif


#ifdef BUILD_A15
    /** For all alorithms on A15 */
    AlgorithmLink_FrameCopy_initPlugin();
    AlgorithmLink_FullView_initPlugin();
    AlgorithmLink_DmaSwMs_initPlugin();
    AlgorithmLink_sparseOpticalFlowDraw_initPlugin();
    AlgorithmLink_laneDetectDraw_initPlugin();
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
