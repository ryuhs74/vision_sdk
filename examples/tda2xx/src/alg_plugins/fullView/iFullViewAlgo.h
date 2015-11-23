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
 * \file iFullViewDSPAlgo.h
 *
 * \brief Interface file for Alg_FullView algorithm on DSP
 *
 *        This Alg_FullView algorithm is only for demonstrative purpose.
 *        It is NOT product quality.
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _IFULLVIEWDSPALGO_H_
#define _IFULLVIEWDSPALGO_H_

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

#ifdef BUILD_DSP
#define RESTRICT restrict
#else
#define RESTRICT
#endif
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
 *   \brief Structure for frame copy algoirthm object
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   maxHeight;
    /**< Max height of the frame */
    UInt32                   maxWidth;
    /**< max width of the frame */
} Alg_FullView_Obj;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the frame Copy create time parameters
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   maxHeight;
    /**< Max height of the frame */
    UInt32                   maxWidth;
    /**< max width of the frame */
} Alg_FullViewCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the frame Copy control parameters
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   xxx;
    /**< Any parameters which can be used to alter the algorithm behavior */
} Alg_FullViewControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Implementation of create for frame copy algo
 *
 * \param  pCreateParams    [IN] Creation parameters for frame copy Algorithm
 *
 * \return  Handle to algorithm
 *
 *******************************************************************************
 */
Alg_FullView_Obj * Alg_FullViewCreate(
                        Alg_FullViewCreateParams *pCreateParams);

/**
 *******************************************************************************
 *
 * \brief Implementation of Process for frame copy algo
 *
 *        Supported formats are SYSTEM_DF_YUV422I_YUYV, SYSTEM_DF_YUV420SP_UV 
 *        It is assumed that the width of the image will
 *        be multiple of 4 and buffer pointers are 32-bit aligned. 
 *        
 * \param  algHandle    [IN] Algorithm object handle
 * \param  inPtr[]      [IN] Array of input pointers
 *                           Index 0 - Pointer to Y data in case of YUV420SP, 
 *                                   - Single pointer for YUV422IL or RGB
 *                           Index 1 - Pointer to UV data in case of YUV420SP
 * \param  outPtr[]     [IN] Array of output pointers. Indexing similar to 
 *                           array of input pointers
 * \param  width        [IN] width of image
 * \param  height       [IN] height of image
 * \param  inPitch[]    [IN] Array of pitch of input image (Address offset 
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  outPitch[]   [IN] Array of pitch of output image (Address offset 
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  dataFormat   [IN] Different image data formats. Refer 
 *                           System_VideoDataFormat
 * \param  copyMode     [IN] 0 - copy by CPU, 1 - copy by DMA
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FullViewProcess(Alg_FullView_Obj *algHandle,
                           UInt32            *RESTRICT inPtr[RESTRICT],
                           UInt32            *RESTRICT outPtr[RESTRICT],
                           UInt32             width,
                           UInt32             height,
                           UInt32             inPitch[],
                           UInt32             outPitch[],
                           UInt32             startX,
						   UInt32             startY,
						   UInt32			  *RESTRICT viewLUT
                          );

/**
 *******************************************************************************
 *
 * \brief Implementation of Control for frame copy algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 * \param  pControlParams        [IN] Pointer to Control Params
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FullViewControl(Alg_FullView_Obj          *pAlgHandle,
                           Alg_FullViewControlParams *pControlParams);

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for frame copy algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FullViewDelete(Alg_FullView_Obj *pAlgHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* Nothing beyond this point */
