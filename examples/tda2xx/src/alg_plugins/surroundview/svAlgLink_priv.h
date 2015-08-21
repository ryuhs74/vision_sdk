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
 * \ingroup ALGORITHM_LINK_API
 * \defgroup ALGORITHM_LINK_IMPL Algorithm Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file svAlgLink_priv.h Geometric Alignment Algorithm Link private
 *                            API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 *           Definitions common to all algorithm stages of surround view
 *
 * \version 0.0 (Oct 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _SVALG_LINK_PRIV_H_
#define _SVALG_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#ifndef PC_VERSION
	#include <include/link_api/system.h>
	#include <include/link_api/algorithmLink_algPluginSupport.h>
    #include "./include/svCommonDefs.h"
#else
	#include "../include/svCommonDefs.h"
#endif



/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Input Width
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_INPUT_FRAME_WIDTH (1280)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Input Height
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_INPUT_FRAME_HEIGHT (720)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Output Height
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_OUTPUT_FRAME_WIDTH (1000)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Output Height
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_OUTPUT_FRAME_HEIGHT (1080)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm pixel point width for 3D
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_3D_PIXEL_POINTS_WIDTH (220*2)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm pixel point height for 3D
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_3D_PIXEL_POINTS_HEIGHT (270*2)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Maximum number of views
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_MAX_NUM_VIEWS (4)

/**
 *******************************************************************************
 *
 *   \brief Size of Geometric Alignment LUT
 *
 *          Buffer allocation done considering following factors -
 *              - Output height
 *              - Output width
 *              - Based on the color format (1.25 for 420SP)-U and V use
 *                same table
 *              - 3 tables - Simple synthesis LUT, Blend LUT1, Blend LUT2
 *                TBD - Make 3 when blending / PA Stats is enabled
 *              - Per entry size of GAlignLUT_BitPerEntry
 *              - An additional 256 bytes for any round off etc during
 *                division etc..
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_GALUT_SIZE ((SV_ALGLINK_OUTPUT_FRAME_HEIGHT) * \
                               (SV_ALGLINK_OUTPUT_FRAME_WIDTH) *  \
                               (1.25) *                           \
                               2 *                                \
                               GAlignLUT_BitPerEntry              \
                               + 256)

//size of PixelsPerCm buffer in bytes
//1 float (4 bytes) for each view
#define SV_ALGLINK_GA_PIXELSPERCM_SIZE (SV_ALGLINK_MAX_NUM_VIEWS)*4

/********************************************************************************
Ultrasonic Extension
********************************************************************************/
//maximum number of ultrasonic sensors supported
#define SV_ALGLINK_INPUT_MAX_ULTRASONICS (16)

//size of ultrasonic overlay image in Bytes
#define SV_ALGLINK_UF_OVERLAYDATA_SIZE ((SV_ALGLINK_OUTPUT_FRAME_HEIGHT)/(SV_UF_OVERLAYDATA_SCALE) * \
        								(SV_ALGLINK_OUTPUT_FRAME_WIDTH)/(SV_UF_OVERLAYDATA_SCALE) * 2)

/**
 *******************************************************************************
 *
 *   \brief Size of Photometric Alignment LUT
 *
 *          Buffer allocation done considering following factors -
 *              - Number of views
 *              - Number of different pixel values (256 for 8-bit pixels)
 *              - Number of color planes - 3 (YUV or RGB)
 *              - An additional 256 bytes for any round off etc during
 *                division etc..
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_PALUT_SIZE ((SV_ALGLINK_MAX_NUM_VIEWS * 256 * 3) + 256)

/**
 *******************************************************************************
 *
 *   \brief Size of Synthesis blend LUT size. This is used only in 3D case.
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_SYNT_BLENDLUT_SIZE (SV_ALGLINK_OUTPUT_FRAME_HEIGHT * \
                                       SV_ALGLINK_OUTPUT_FRAME_WIDTH * 2)

/**
 *******************************************************************************
 *
 *   \brief Threshold size beyond which memory gets allocated in Shared area
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_ALGLINK_SRMEM_THRESHOLD (ALGORITHMLINK_SRMEM_THRESHOLD)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
