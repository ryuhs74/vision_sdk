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
 * \file colorToGrayAlgo.c
 *
 * \brief Algorithm for Alg_ColorToGray on DSP
 *
 *        This Alg_ColorToGray algorithm is only for demonstrative purpose. 
 *        It is NOT product quality.
 *        Color to gray is a sample algorithm, which just takes a colored video
 *        frame and zeros out chroma component to make it look gray. The 
 *        purpose of this sample algorithm is to demonstrate inplace 
 *        computation in Links and Chains framework.
 *
 * \version 0.0 (Sept 2013) : [NN] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "iColorToGrayAlgo.h"

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
                        Alg_ColorToGrayCreateParams *pCreateParams)
{

    Alg_ColorToGray_Obj * pAlgHandle;
    
    pAlgHandle = (Alg_ColorToGray_Obj *) malloc(sizeof(Alg_ColorToGray_Obj));

    UTILS_assert(pAlgHandle != NULL);

    return pAlgHandle;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process for color to gray algo
 *        
 *        Supported formats are SYSTEM_DF_YUV422I_YUYV, SYSTEM_DF_YUV420SP_UV, 
 *        SYSTEM_DF_RGB24_888. It is assumed that the width of the image will
 *        be multiple of 4 and buffer pointers are 32-bit aligned. 
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
                          )
{
    Int32 rowIdx; 
    Int32 colIdx;

    UInt32 wordWidth;
    
    UInt32 *inputPtr;
    UInt32 mask = 0x00FF00FF;
    UInt32 mask1 = 0x80008000;
             
    if(dataFormat == SYSTEM_DF_YUV422I_YUYV) 
    {
        wordWidth = (width*2)>>2;
    } 
    else
    {
        return SYSTEM_LINK_STATUS_EFAIL;
    }
        
    inputPtr  = inPtr[0];
    
    for(rowIdx = 0; rowIdx < height ; rowIdx++)
    {
        for(colIdx = 0; colIdx < wordWidth ; colIdx++)
        {
             /* Masking chroma component */
             
            *(inputPtr+colIdx) &= mask;
            *(inputPtr+colIdx) |= mask1;
            
        }
        inputPtr += (inPitch[0] >> 2);
    }
    return SYSTEM_LINK_STATUS_SOK;
}

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
                           Alg_ColorToGrayControlParams *pControlParams)
{
    /* 
     * Any alteration of algorithm behavior
     */
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop for color to gray algo
 *
 * \param  algHandle    [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_ColorToGrayStop(Alg_ColorToGray_Obj *algHandle)
{
      return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for color to gray algo
 *
 * \param  algHandle    [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_ColorToGrayDelete(Alg_ColorToGray_Obj *algHandle)
{
    free(algHandle);
    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
