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
 * \brief Algorithm for Alg_FullView on DSP
 *
 *        This Alg_FullView algorithm is only for demonstrative purpose.
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
#include "iFullViewAlgo.h"

#define AVM_LUT_INTEGER_BITS			10
#define AVM_LUT_FRACTION_BITS			6

typedef struct
{
	UInt8 y;
	UInt8 uv;
}yuyv;

#define HD720P_WIDTH	1280
typedef yuyv yuvHD720P[HD720P_WIDTH];


typedef struct {
	unsigned short xFraction:AVM_LUT_FRACTION_BITS;
	unsigned short xInteger:AVM_LUT_INTEGER_BITS;
	unsigned short yFraction:AVM_LUT_FRACTION_BITS;
	unsigned short yInteger:AVM_LUT_INTEGER_BITS;
} ViewLUT_Packed;


typedef struct {
	unsigned short x;
	unsigned short y;
} ViewLUT_Pos;

#if 0
#define LinearInterpolation(x,q1,q2)\
	((64-x)*q1 + x*q2)>>AVM_LUT_FRACTION_BITS

inline UInt8 BilinearInterpolation(yuyv* q, ViewLUT_Packed *lut)
{
	UInt16 X =	lut->xFraction;
	UInt16 Y =	lut->yFraction;

	UInt16 R1,R2,Q;
	R1 = LinearInterpolation(X,q[0].y,q[1].y);
	R2 = LinearInterpolation(X,q[HD720P_WIDTH].y,q[HD720P_WIDTH+1].y);
	Q = LinearInterpolation(Y,R1,R2);

	return (UInt8)(Q);
}

inline UInt8 BilinearInterpolationUV(yuyv* q, ViewLUT_Packed *lut)
{
	UInt16 X =	lut->xFraction;
	UInt16 Y =	lut->yFraction;

	UInt16 R1,R2,Q;
	R1 = LinearInterpolation(X,q[0].uv,q[2].uv);
	R2 = LinearInterpolation(X,q[HD720P_WIDTH].uv,q[HD720P_WIDTH+2].uv);
	Q = LinearInterpolation(Y,R1,R2);

	return (UInt8)(Q);
}
#else
///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolation(q, lut, Q)\
{\
	UInt16 X =	lut->xFraction;\
	UInt16 Y =	lut->yFraction;\
	UInt16 A,B;\
	A = 64 - X;\
	B = 64 - Y;\
	Q = (UInt8)((q[0].y*A*B + q[1].y*X*B + q[HD720P_WIDTH].y*A*Y + q[HD720P_WIDTH+1].y*X*Y)>>12);\
}
///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolationUV(q, lut, Q)\
{\
	UInt16 X =	lut->xFraction;\
	UInt16 Y =	lut->yFraction;\
	UInt16 A,B;\
	A = 64 - X;\
	B = 64 - Y;\
	Q = (UInt8)((q[0].uv*A*B + q[2].uv*X*B + q[HD720P_WIDTH].uv*A*Y + q[HD720P_WIDTH+2].uv*X*Y)>>12);\
}

#endif

/**
 *******************************************************************************
 *float
 * \brief Implementation of create for frame copy algo
 *BilinearInterpolation
 * \param  pCreateParams    [IN] Creation parameters for frame copy Algorithm
 *
 * \return  Handle to algorithm
 *
 *******************************************************************************
 */
Alg_FullView_Obj * Alg_FullViewCreate(
                        Alg_FullViewCreateParams *pCreateParams)
{

    Alg_FullView_Obj * pAlgHandle;

    pAlgHandle = (Alg_FullView_Obj *) malloc(sizeof(Alg_FullView_Obj));

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
                          )
{
    Int16 rowIdx;
    Int16 colIdx;

    yuvHD720P* iPtr;
    yuvHD720P* oPtr;

    ViewLUT_Packed *lut = (ViewLUT_Packed*)viewLUT;

    iPtr  = (yuvHD720P*)inPtr[0];
    oPtr = (yuvHD720P*)outPtr[0];

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(500,720, 2);
#endif
    for(rowIdx = startY; rowIdx < height ; rowIdx++)
    {
    	lut = (ViewLUT_Packed*)(viewLUT) + rowIdx*width;
    	ViewLUT_Packed *lutbak;
#ifdef BUILD_DSP
#pragma UNROLL(4);
#pragma MUST_ITERATE(500,1280, 4);
#endif
        for(colIdx = startX, lutbak = lut; colIdx < width ; colIdx++, lutbak++)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q, lutbak, oPtr[rowIdx][colIdx].y);
        }
#if 1
#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(350,640, 2);
#endif
        for(colIdx = startX, lutbak = lut; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	BilinearInterpolationUV(q, lutbak, oPtr[rowIdx][colIdx].uv);
        	///V
        	q ++;
        	BilinearInterpolationUV(q, lutbak, oPtr[rowIdx][colIdx+1].uv);
        }
#endif
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
Int32 Alg_FullViewControl(Alg_FullView_Obj          *pAlgHandle,
                           Alg_FullViewControlParams *pControlParams)
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
Int32 Alg_FullViewStop(Alg_FullView_Obj *algHandle)
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
Int32 Alg_FullViewDelete(Alg_FullView_Obj *algHandle)
{
    free(algHandle);
    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
