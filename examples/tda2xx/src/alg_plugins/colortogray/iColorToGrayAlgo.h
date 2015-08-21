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
 * \file iColorToGrayAlgo.h
 *
 * \brief Interface file for Alg_ColorToGray algorithm on DSP
 *
 *        This Alg_ColorToGray algorithm is only for demonstrative purpose. 
 *        It is NOT product quality.
 *
 * \version 0.0 (Sept 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ICOLORTOGRAYALGO_H_
#define _ICOLORTOGRAYALGO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>

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
 *   \brief Structure for color to gray algoirthm object
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 xxx;
} Alg_ColorToGray_Obj;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the color to gray create time parameters
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 xxxx;
} Alg_ColorToGrayCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the color to gray control parameters
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   xxx;
    /**< Any parameters which can be used to alter the algorithm behavior */
} Alg_ColorToGrayControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Implementation of create for color to gray algo
 *
 * \param  pCreateParams    [IN] Creation parameters for color to gray Algorithm
 *
 * \return  Handle to algorithm
 *
 *******************************************************************************
 */
Alg_ColorToGray_Obj * Alg_ColorToGrayCreate(
                        Alg_ColorToGrayCreateParams *pCreateParams);

/**
 *******************************************************************************
 *
 * \brief Implementation of Process for color to gray algo
 *
 *        Supported formats are SYSTEM_DF_YUV422I_YUYV. It is assumed that 
 *        width of the image will be multiple of 4 and buffer pointers are 
 *        32-bit aligned. 
 *        
 * \param  algHandle    [IN] Algorithm object handle
 * \param  inPtr[]      [IN] Array of input pointers
 *                           Index 0 - Pointer to Y data in case of YUV420SP, 
 *                                   - Single pointer for YUV422IL or RGB
 *                           Index 1 - Pointer to UV data in case of YUV420SP
 * \param  width        [IN] width of image
 * \param  height       [IN] height of image
 * \param  inPitch[]    [IN] Array of pitch of input image (Address offset 
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  dataFormat   [IN] Different image data formats. Refer 
 *                           System_VideoDataFormat
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_ColorToGrayProcess(Alg_ColorToGray_Obj *algHandle,
                           UInt32            *inPtr[],
                           UInt32             width,
                           UInt32             height,
                           UInt32             inPitch[],
                           UInt32             dataFormat 
                          );

/**
 *******************************************************************************
 *
 * \brief Implementation of Control for color to gray algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 * \param  pControlParams        [IN] Pointer to Control Params
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_ColorToGrayControl(Alg_ColorToGray_Obj          *pAlgHandle,
                           Alg_ColorToGrayControlParams *pControlParams);

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for color to gray algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_ColorToGrayDelete(Alg_ColorToGray_Obj *pAlgHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* Nothing beyond this point */
