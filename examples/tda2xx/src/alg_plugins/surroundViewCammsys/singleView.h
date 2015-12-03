/*
 * singleView.h
 *
 *  Created on: Nov 18, 2015
 *      Author: craven
 *       * Copyright (C) 2015 Cammsys - http://www.cammsys.net/
 */

#ifndef EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_
#define EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_

#ifndef RESTRICT
#ifdef BUILD_DSP
#define RESTRICT restrict
#else
#define RESTRICT
#endif
#endif

#define AVM_LUT_MAX_BIT					16
#define AVM_LUT_INTEGER_BITS			11
#define AVM_LUT_FRACTION_BITS			(AVM_LUT_MAX_BIT - AVM_LUT_INTEGER_BITS)

typedef struct
{
	UInt8 y;
	UInt8 uv;
}yuyv;
typedef struct
{
	UInt8 y1;
	UInt8 u;
	UInt8 y2;
	UInt8 v;
}YUYV;
#if 0
typedef struct
{
	UInt16 width;
	UInt16 height;
	UInt16 startX;
	UInt16 startY;
	UInt16 pitch;
}ViewInfo;
#endif

typedef struct {
	unsigned short xFraction:AVM_LUT_FRACTION_BITS;
	unsigned short xInteger:AVM_LUT_INTEGER_BITS;
	unsigned short yFraction:AVM_LUT_FRACTION_BITS;
	unsigned short yInteger:AVM_LUT_INTEGER_BITS;
} ViewLUT_Packed;

typedef struct {
	unsigned char pos:8;
	unsigned char y_b:8;
	unsigned char cb_g:8;
	unsigned char cr_r_overlay:8;
} MaskLUT_Packed;

#define HD720P_WIDTH	1280
#define HD1080P_WIDTH	1920
typedef yuyv yuvHD720P[HD720P_WIDTH];
typedef yuyv yuvHD1080P[HD1080P_WIDTH];
typedef yuyv yuvHD260Pixel[260];


#define ONE_PER_AVM_LUT_FRACTION_BITS	(1<<AVM_LUT_FRACTION_BITS)

#define LinearInterpolation(x,q1,q2,perFraction,fractionBits)\
	((perFraction-x)*q1 + x*q2)>>fractionBits

#define _LinearInterpolation(x,q1,q2,perFraction)\
	((perFraction-x)*q1 + x*q2)
#define _X	lut->xFraction
#define _Y lut->yFraction
///https://en.wikipedia.org/wiki/Bilinear_interpolation

#if 1
#define BilinearInterpolation(q, lut, Q,  pitch)\
{\
	register Int32 R1,R2;\
	R1 = _LinearInterpolation(_X,q[0].y,q[1].y,ONE_PER_AVM_LUT_FRACTION_BITS);\
	R2 = _LinearInterpolation(_X,q[pitch].y,q[pitch+1].y,ONE_PER_AVM_LUT_FRACTION_BITS);\
	Q = _LinearInterpolation(_Y,R1,R2,ONE_PER_AVM_LUT_FRACTION_BITS)>>(AVM_LUT_FRACTION_BITS<<1);\
}
///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolationUV(q, lut, Q, pitch)\
{\
	register Int32 R1,R2;\
	R1 = _LinearInterpolation(_X,q[0].uv,q[2].uv,ONE_PER_AVM_LUT_FRACTION_BITS);\
	R2 = _LinearInterpolation(_X,q[pitch].uv,q[pitch+2].uv,ONE_PER_AVM_LUT_FRACTION_BITS);\
	Q = _LinearInterpolation(_Y,R1,R2,ONE_PER_AVM_LUT_FRACTION_BITS)>>(AVM_LUT_FRACTION_BITS<<1);\
}
#elif 0
#define BilinearInterpolation(q, lut, Q,  pitch)\
{\
	register UInt16 R1,R2;\
	R1 = LinearInterpolation(_X,q[0].y,q[1].y,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);\
	R2 = LinearInterpolation(_X,q[pitch].y,q[pitch+1].y,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);\
	Q = LinearInterpolation(_Y,R1,R2,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);\
}
///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolationUV(q, lut, Q, pitch)\
{\
	register UInt16 R1,R2;\
	R1 = LinearInterpolation(_X,q[0].uv,q[2].uv,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);\
	R2 = LinearInterpolation(_X,q[pitch].uv,q[pitch+2].uv,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);\
	Q = LinearInterpolation(_Y,R1,R2,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);\
}
#else
#define BilinearInterpolation(q, lut, Q,  pitch)\
{\
	Q = q[0].y;\
}
///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolationUV(q, lut, Q, pitch)\
{\
	Q = q[0].uv;\
}

#endif

static inline Int32 makeSingleView720P(  UInt32       *RESTRICT inPtr,
                           	   UInt32           *RESTRICT outPtr,
							   UInt32			*RESTRICT viewLUTPtr,
							   AlgorithmLink_SurroundViewLutInfo			*RESTRICT viewInfo,
							   AlgorithmLink_SurroundViewLutInfo			*RESTRICT childViewInfoLUT
                          )
{
	UInt16 rowIdx;
    UInt16 colIdx;

    yuvHD720P* iPtr;
    yuvHD720P* oPtr;

    UInt16 startX = viewInfo->startX + childViewInfoLUT->startX;
    UInt16 width = childViewInfoLUT->width + startX;
    UInt16 height = childViewInfoLUT->height;

    if(width > (viewInfo->startX+viewInfo->width))
    	width = viewInfo->startX+viewInfo->width;

    ViewLUT_Packed *lut = ((ViewLUT_Packed*)viewLUTPtr) + (childViewInfoLUT->pitch * childViewInfoLUT->startY);

    iPtr  = (yuvHD720P*)inPtr;
    oPtr = ((yuvHD720P*)outPtr) + viewInfo->startY + childViewInfoLUT->startY;

    for(rowIdx = 0; rowIdx < height ; rowIdx++)
    {
    	ViewLUT_Packed *lutbak;
        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx++, lutbak++)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q, lutbak, oPtr[rowIdx][colIdx].y, HD720P_WIDTH);
        }

        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	yuyv *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	yuyv *qv = qu+1;
        	///U
        	BilinearInterpolationUV(qu, lutbak, oPtr[rowIdx][colIdx].uv, HD720P_WIDTH);
        	///V
        	BilinearInterpolationUV(qv, lutbak, oPtr[rowIdx][colIdx+1].uv, HD720P_WIDTH);
        }

    	lut += childViewInfoLUT->pitch;
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 * @param brief  Top View Size 520x688 and than bland area must be smaller quad size(260x344)
 * @param inPtr
 * @param outPtr
 * @param viewLUTPtr
 * @param viewInfo
 * @param childViewInfoLUT
 * @return
 */
static inline Int32 makeSingleView720PBuff(  UInt32       *RESTRICT inPtr,
                           	   UInt32           *RESTRICT outPtr,
							   UInt32			*RESTRICT viewLUTPtr,
							   AlgorithmLink_SurroundViewLutInfo			*RESTRICT viewInfo,
							   AlgorithmLink_SurroundViewLutInfo			*RESTRICT childViewInfoLUT
                          )
{
	UInt16 rowIdx;
    UInt16 colIdx;

    yuvHD720P* iPtr;
    yuvHD260Pixel* oPtr;

    UInt16 width = childViewInfoLUT->width;
    UInt16 height = childViewInfoLUT->height;


    ViewLUT_Packed *lut = ((ViewLUT_Packed*)viewLUTPtr) + (childViewInfoLUT->pitch * childViewInfoLUT->startY);

    iPtr  = (yuvHD720P*)inPtr;
    oPtr = ((yuvHD260Pixel*)outPtr);

    for(rowIdx = 0; rowIdx < height ; rowIdx++)
    {
    	ViewLUT_Packed *lutbak;
        for(colIdx = 0, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx++, lutbak++)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q, lutbak, oPtr[rowIdx][colIdx].y, HD720P_WIDTH);
        }

        for(colIdx = 0, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	yuyv *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	yuyv *qv = qu+1;
        	///U
        	BilinearInterpolationUV(qu, lutbak, oPtr[rowIdx][colIdx].uv, HD720P_WIDTH);
        	///V
        	BilinearInterpolationUV(qv, lutbak, oPtr[rowIdx][colIdx+1].uv, HD720P_WIDTH);
        }

    	lut += childViewInfoLUT->pitch;
    }
    return SYSTEM_LINK_STATUS_SOK;
}

#define makeSingleView makeSingleView720P

#if 0
static inline Int32 makeSingleView(  UInt32       	*RESTRICT inPtr,
                           	   UInt32           *RESTRICT outPtr,
							   UInt32			*RESTRICT viewLUTPtr,
							   AlgorithmLink_SurroundViewLutInfo			*RESTRICT viewInfo,
							   AlgorithmLink_SurroundViewLutInfo			*RESTRICT childViewInfoLUT
                          )
{
/*
	viewInfo->width = viewInfo->width < childViewInfoLUT->width + childViewInfoLUT->startX ? viewInfo->width : childViewInfoLUT->width + childViewInfoLUT->startX;
	viewInfo->height = viewInfo->height < childViewInfoLUT->height + childViewInfoLUT->startY ? viewInfo->height : childViewInfoLUT->height+ childViewInfoLUT->startY;
*/
	makeSingleView720P(inPtr, outPtr, viewLUTPtr, viewInfo, childViewInfoLUT);

    return SYSTEM_LINK_STATUS_SOK;
}
#endif



#endif /* EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_ */
