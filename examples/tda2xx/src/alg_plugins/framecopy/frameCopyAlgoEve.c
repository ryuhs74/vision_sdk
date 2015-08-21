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
 * \file frameCopyDSPAlgo.c
 *
 * \brief Algorithm for Alg_FrameCopy on DSP
 *
 *        This Alg_FrameCopy algorithm is only for demonstrative purpose.
 *        It is NOT product quality.
 *        This algorithm does a frame copy. Height and width gets decided during
 *        Create. If height / width needs to be altered, then control call
 *        needs to be done.
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "iFrameCopyAlgo.h"
#include <evelib_memcopy_dma_2d.h>

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
Alg_FrameCopy_Obj * Alg_FrameCopyCreate(
                        Alg_FrameCopyCreateParams *pCreateParams)
{

    Alg_FrameCopy_Obj * pAlgHandle;

    pAlgHandle = (Alg_FrameCopy_Obj *) malloc(sizeof(Alg_FrameCopy_Obj));

    UTILS_assert(pAlgHandle != NULL);

    pAlgHandle->maxHeight   = pCreateParams->maxHeight;
    pAlgHandle->maxWidth    = pCreateParams->maxWidth;

    return pAlgHandle;
}

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
Int32 Alg_FrameCopyProcess(Alg_FrameCopy_Obj *algHandle,
                           UInt32            *inPtr[],
                           UInt32            *outPtr[],
                           UInt32             width,
                           UInt32             height,
                           UInt32             inPitch[],
                           UInt32             outPitch[],
                           UInt32             dataFormat,
                           Uint32             copyMode
                          )
{
    Int32 rowIdx;
    Int32 colIdx;

    UInt32 wordWidth;
    UInt32 numPlanes;

    UInt32 *inputPtr;
    UInt32 *outputPtr;

    if((width > algHandle->maxWidth) ||
       (height > algHandle->maxHeight))
    {
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
    {
        numPlanes = 1;
        wordWidth = (width*2)>>2;
    }
    else if(dataFormat == SYSTEM_DF_YUV420SP_UV)
    {
        numPlanes = 2;
        wordWidth = (width)>>2;
    }
    else
    {
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    /*
     * For Luma plane of 420SP OR RGB OR 422IL
     */
    inputPtr  = inPtr[0];
    outputPtr = outPtr[0];

    if(copyMode == 0)
    {
        for(rowIdx = 0; rowIdx < height ; rowIdx++)
        {
            for(colIdx = 0; colIdx < wordWidth ; colIdx++)
            {
                *(outputPtr+colIdx) = *(inputPtr+colIdx);
            }
            inputPtr += (inPitch[0] >> 2);
            outputPtr += (outPitch[0] >> 2);
        }
        /*
         * For chroma plane of 420SP
         */
        if(numPlanes == 2)
        {
            inputPtr  = inPtr[1];
            outputPtr = outPtr[1];
            for(rowIdx = 0; rowIdx < (height >> 1) ; rowIdx++)
            {
                for(colIdx = 0; colIdx < wordWidth ; colIdx++)
                {
                    *(outputPtr+colIdx) = *(inputPtr+colIdx);
                }
                inputPtr += (inPitch[1] >> 2);
                outputPtr += (outPitch[1] >> 2);
            }
        }
    }
    else
    {
        EVELIB_memcopyDMA2D((UInt8 *)inputPtr,
                            (UInt8 *)outputPtr,
                            (wordWidth<<2),
                            height,
                            inPitch[0],
                            outPitch[0]);
        /*
         * For chroma plane of 420SP
         */
        if(numPlanes == 2)
        {
            inputPtr  = inPtr[1];
            outputPtr = outPtr[1];
            EVELIB_memcopyDMA2D((UInt8 *)inputPtr,
                                (UInt8 *)outputPtr,
                                (wordWidth<<2),
                                height/2,
                                inPitch[1],
                                outPitch[1]);
        }

    }

    return SYSTEM_LINK_STATUS_SOK;
}

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
Int32 Alg_FrameCopyControl(Alg_FrameCopy_Obj          *pAlgHandle,
                           Alg_FrameCopyControlParams *pControlParams)
{
    /*
     * Any alteration of algorithm behavior
     */
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop for frame copy algo
 *
 * \param  algHandle    [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FrameCopyStop(Alg_FrameCopy_Obj *algHandle)
{
      return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for frame copy algo
 *
 * \param  algHandle    [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FrameCopyDelete(Alg_FrameCopy_Obj *algHandle)
{
    free(algHandle);
    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
