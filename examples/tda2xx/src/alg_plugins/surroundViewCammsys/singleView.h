/*
 * singleView.h
 *
 *  Created on: Nov 18, 2015
 *      Author: craven
 *       * Copyright (C) 2015 Cammsys - http://www.cammsys.net/
 */

#ifndef EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_
#define EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_

#define SUPPORT_SHARPEN_FILTER	1


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
}YUYV;

typedef struct
{
	UInt8 u;
	UInt8 v;
}UV;

typedef struct
{
	UInt8 y;
}Y;

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

#define TEMP_BUF_WIDTH	208
#define HD720P_WIDTH	1280
#define HD1080P_WIDTH	1920
typedef YUYV yuvHD720P[HD720P_WIDTH];
typedef YUYV yuvHD1080P[HD1080P_WIDTH];
typedef YUYV yuvHD260Pixel[TEMP_BUF_WIDTH];
typedef UInt8 yHD260Pixel[TEMP_BUF_WIDTH];


#define ONE_PER_AVM_LUT_FRACTION_BITS	(1<<AVM_LUT_FRACTION_BITS)

#define LinearInterpolation(x,q1,q2,perFraction,fractionBits)\
	((perFraction-x)*q1 + x*q2)>>fractionBits

#define _LinearInterpolation(x,q1,q2,perFraction)\
	((perFraction-x)*q1 + x*q2)



///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolation(q1,q2,q3,q4, lut, Q)\
{\
	register Int32 R1,R2;\
	R1 = _LinearInterpolation((lut->xFraction),q1,q2,ONE_PER_AVM_LUT_FRACTION_BITS);\
	R2 = _LinearInterpolation((lut->xFraction),q3,q4,ONE_PER_AVM_LUT_FRACTION_BITS);\
	Q = _LinearInterpolation((lut->yFraction),R1,R2,ONE_PER_AVM_LUT_FRACTION_BITS)>>(AVM_LUT_FRACTION_BITS<<1);\
}
#define BilinearInterpolation_filter(q1,q2,q3,q4, lut, Q, _Q)\
{\
	register Int32 R1,R2;\
	R1 = _LinearInterpolation((lut->xFraction),q1,q2,ONE_PER_AVM_LUT_FRACTION_BITS);\
	R2 = _LinearInterpolation((lut->xFraction),q3,q4,ONE_PER_AVM_LUT_FRACTION_BITS);\
	Q = _LinearInterpolation((lut->yFraction),R1,R2,ONE_PER_AVM_LUT_FRACTION_BITS)>>(AVM_LUT_FRACTION_BITS<<1);\
	_Q = Q;\
}
#define NoInterpolation(q1,q2,q3,q4, lut, Q)\
{\
	Q = q1;\
}

static inline Int32 makeSingleView720P(	UInt32 *RESTRICT inPtr,
										UInt32 *RESTRICT outPtr,
										UInt8 *RESTRICT buf1,
										UInt8 *RESTRICT buf2,
										UInt32 *RESTRICT viewLUTPtr,
										AlgorithmLink_SurroundViewLutInfo *RESTRICT viewInfo,
										AlgorithmLink_SurroundViewLutInfo *RESTRICT childViewInfoLUT)
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
        	YUYV *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q[0].y, q[1].y, q[HD720P_WIDTH].y, q[HD720P_WIDTH+1].y, lutbak, oPtr[rowIdx][colIdx].y);
        }

        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	YUYV *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	BilinearInterpolation(qu[0].uv, qu[2].uv, qu[HD720P_WIDTH].uv, qu[HD720P_WIDTH+2].uv, lutbak, oPtr[rowIdx][colIdx].uv);
        	///V
        	BilinearInterpolation(qu[1].uv, qu[3].uv, qu[HD720P_WIDTH+1].uv, qu[HD720P_WIDTH+3].uv, lutbak, oPtr[rowIdx][colIdx+1].uv);
        }

    	lut += childViewInfoLUT->pitch;
    }
    return SYSTEM_LINK_STATUS_SOK;
}

static inline Int32 makeSingleView720PNoInter(	UInt32 *RESTRICT inPtr,
												UInt32 *RESTRICT outPtr,
												UInt8       *RESTRICT buf1,
												UInt8       *RESTRICT buf2,
												UInt32 *RESTRICT viewLUTPtr,
												AlgorithmLink_SurroundViewLutInfo *RESTRICT viewInfo,
												AlgorithmLink_SurroundViewLutInfo *RESTRICT childViewInfoLUT)
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
        	YUYV *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	NoInterpolation(q[0].y, q[1].y, q[HD720P_WIDTH].y, q[HD720P_WIDTH+1].y, lutbak, oPtr[rowIdx][colIdx].y);
        }

        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	YUYV *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	NoInterpolation(qu[0].uv, qu[2].uv, qu[HD720P_WIDTH].uv, qu[HD720P_WIDTH+2].uv, lutbak, oPtr[rowIdx][colIdx].uv);
        	///V
        	NoInterpolation(qu[1].uv, qu[3].uv, qu[HD720P_WIDTH+1].uv, qu[HD720P_WIDTH+3].uv, lutbak, oPtr[rowIdx][colIdx+1].uv);
        }

    	lut += childViewInfoLUT->pitch;
    }
    return SYSTEM_LINK_STATUS_SOK;
}

static inline Int32 makeSingleView720PWithFilter(	UInt32 *RESTRICT inPtr,
													UInt32 *RESTRICT outPtr,
													UInt8       *RESTRICT buf1,
													UInt8       *RESTRICT buf2,
													UInt32 *RESTRICT viewLUTPtr,
													AlgorithmLink_SurroundViewLutInfo *RESTRICT viewInfo,
													AlgorithmLink_SurroundViewLutInfo *RESTRICT childViewInfoLUT)
{
	UInt16 rowIdx;
    UInt16 colIdx;

    yHD260Pixel* FilterBuffIn  = (yHD260Pixel*)buf1;
    yHD260Pixel* FilterBuffOut = (yHD260Pixel*)buf2;
/*
    UInt8* FilterBuffIn = buf1;
    UInt8* FilterBuffOut = buf2;
*/

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
        for(colIdx = 0, lutbak = lut + childViewInfoLUT->startX; colIdx < (width - startX) ; colIdx++, lutbak++)
        {
        	YUYV *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation_filter(q[0].y, q[1].y, q[HD720P_WIDTH].y, q[HD720P_WIDTH+1].y, lutbak, FilterBuffIn[rowIdx][colIdx], FilterBuffOut[rowIdx][colIdx+3]);
        }

        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	YUYV *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	BilinearInterpolation(qu[0].uv, qu[2].uv, qu[HD720P_WIDTH].uv, qu[HD720P_WIDTH+2].uv, lutbak, oPtr[rowIdx][colIdx].uv);
        	///V
        	BilinearInterpolation(qu[1].uv, qu[3].uv, qu[HD720P_WIDTH+1].uv, qu[HD720P_WIDTH+3].uv, lutbak, oPtr[rowIdx][colIdx+1].uv);
        }

    	lut += childViewInfoLUT->pitch;
    }
    ///Filter
#ifdef BUILD_DSP
	{
#include "IMG_conv_3x3_i8_c8s.h"
		char sharpen_mask[3][3] =
		{
		{ -9, 1, -9 },
		{ 1, 96, 1 },
		{ -9, 1, -9 } };
		char sharpen_shift = 6;
		int i = 0;
		int _width = ((width - startX) & 0xfffc)-4;
		for (i = 0; i < height - 2; i++)
		{
			IMG_conv_3x3_i8_c8s(FilterBuffIn[i],
								FilterBuffOut[i+1]+4,
								_width,
								TEMP_BUF_WIDTH,
								(char*) sharpen_mask,
								sharpen_shift);
		}
	}
#endif
    for(rowIdx = 0; rowIdx < height ; rowIdx++)
    {
    	int filtercolIdx = 3;
        for(colIdx = startX, filtercolIdx = 3; colIdx < width ; colIdx++, filtercolIdx++)
        {
        	oPtr[rowIdx][colIdx].y = FilterBuffOut[rowIdx][filtercolIdx];
        }
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
        	YUYV *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q[0].y, q[1].y, q[HD720P_WIDTH].y, q[HD720P_WIDTH+1].y, lutbak, oPtr[rowIdx][colIdx].y);
        }

        for(colIdx = 0, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	YUYV *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	BilinearInterpolation(qu[0].uv, qu[2].uv, qu[HD720P_WIDTH].uv, qu[HD720P_WIDTH+2].uv, lutbak, oPtr[rowIdx][colIdx].uv);
        	///V
        	BilinearInterpolation(qu[1].uv, qu[3].uv, qu[HD720P_WIDTH+1].uv, qu[HD720P_WIDTH+3].uv, lutbak, oPtr[rowIdx][colIdx+1].uv);
        }

    	lut += childViewInfoLUT->pitch;
    }
    return SYSTEM_LINK_STATUS_SOK;
}


static inline Int32 makeSingleView720P_TEST(	UInt32 *RESTRICT inPtr,
										UInt32 *RESTRICT outPtr,
										UInt8 *RESTRICT buf1,
										UInt8 *RESTRICT buf2,
										UInt32 *RESTRICT viewLUTPtr,
										AlgorithmLink_SurroundViewLutInfo *RESTRICT viewInfo,
										AlgorithmLink_SurroundViewLutInfo *RESTRICT childViewInfoLUT)
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

    for(colIdx = 0; colIdx < HD720P_WIDTH ; colIdx++)
    {
    	iPtr[100][colIdx].y = 0;
    	iPtr[100][colIdx].uv = 0;

    	iPtr[200][colIdx].y = 0;
    	iPtr[200][colIdx].uv = 0;

    	iPtr[300][colIdx].y = 0;
    	iPtr[300][colIdx].uv = 0;


    	iPtr[400][colIdx].y = 0;
    	iPtr[400][colIdx].uv = 0;


    	iPtr[600][colIdx].y = 0;
    	iPtr[600][colIdx].uv = 0;


    	iPtr[700][colIdx].y = 0;
    	iPtr[700][colIdx].uv = 0;

    }
    for(rowIdx = 0; rowIdx < 720 ; rowIdx++)
    {
    	iPtr[rowIdx][100].y = 0;
    	iPtr[rowIdx][100].uv = 0;
    	iPtr[rowIdx][101].y = 0;
    	iPtr[rowIdx][101].uv = 0;

    	iPtr[rowIdx][200].y = 0;
    	iPtr[rowIdx][200].uv = 0;
    	iPtr[rowIdx][201].y = 0;
    	iPtr[rowIdx][201].uv = 0;

    	iPtr[rowIdx][300].y = 0;
    	iPtr[rowIdx][300].uv = 0;
    	iPtr[rowIdx][301].y = 0;
    	iPtr[rowIdx][301].uv = 0;

    	iPtr[rowIdx][400].y = 0;
    	iPtr[rowIdx][400].uv = 0;
    	iPtr[rowIdx][401].y = 0;
    	iPtr[rowIdx][401].uv = 0;

    	iPtr[rowIdx][500].y = 0;
    	iPtr[rowIdx][500].uv = 0;
    	iPtr[rowIdx][501].y = 0;
    	iPtr[rowIdx][501].uv = 0;

    	iPtr[rowIdx][600].y = 0;
    	iPtr[rowIdx][600].uv = 0;
    	iPtr[rowIdx][601].y = 0;
    	iPtr[rowIdx][601].uv = 0;

    	iPtr[rowIdx][700].y = 0;
    	iPtr[rowIdx][700].uv = 0;
    	iPtr[rowIdx][701].y = 0;
    	iPtr[rowIdx][701].uv = 0;

    	iPtr[rowIdx][800].y = 0;
    	iPtr[rowIdx][800].uv = 0;
    	iPtr[rowIdx][801].y = 0;
    	iPtr[rowIdx][801].uv = 0;

    	iPtr[rowIdx][900].y = 0;
    	iPtr[rowIdx][900].uv = 0;
    	iPtr[rowIdx][901].y = 0;
    	iPtr[rowIdx][901].uv = 0;

    	iPtr[rowIdx][1000].y = 0;
    	iPtr[rowIdx][1000].uv = 0;
    	iPtr[rowIdx][1001].y = 0;
    	iPtr[rowIdx][1001].uv = 0;


    	iPtr[rowIdx][1100].y = 0;
    	iPtr[rowIdx][1100].uv = 0;
    	iPtr[rowIdx][1101].y = 0;
    	iPtr[rowIdx][1101].uv = 0;

    	iPtr[rowIdx][1200].y = 0;
    	iPtr[rowIdx][1200].uv = 0;
    	iPtr[rowIdx][1201].y = 0;
    	iPtr[rowIdx][1201].uv = 0;

    }

    for(rowIdx = 0; rowIdx < height ; rowIdx++)
    {
    	ViewLUT_Packed *lutbak;
        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx++, lutbak++)
        {
        	YUYV *q = &iPtr[lutbak->yInteger][lutbak->xInteger];
        	//register Int32 _Q1, _Q2;\

        	BilinearInterpolation(q[0].y, q[1].y, q[HD720P_WIDTH].y, q[HD720P_WIDTH+1].y, lutbak, oPtr[rowIdx][colIdx].y);
        	//_Q1 = LinearInterpolation(lutbak->xFraction,q[0].y,(q + 1)->y,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);
        	//_Q2 = LinearInterpolation(lutbak->yFraction,q[0].y,(q + HD720P_WIDTH)->y,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);
        	//oPtr[rowIdx][colIdx].y = (_Q1 + _Q2)>>1;
        }

        for(colIdx = startX, lutbak = lut + childViewInfoLUT->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	YUYV *qu = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	//register Int32 _Q1, _Q2;\
        	///U
        	BilinearInterpolation(qu[0].uv, qu[2].uv, qu[HD720P_WIDTH].uv, qu[HD720P_WIDTH+2].uv, lutbak, oPtr[rowIdx][colIdx].uv);
        	//_Q1 = LinearInterpolation(lutbak->xFraction,qu[0].uv,(qu + 2)->uv,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);
        	//_Q2 = LinearInterpolation(lutbak->yFraction,qu[0].uv,(qu + HD720P_WIDTH)->uv,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);
        	//oPtr[rowIdx][colIdx].uv = (_Q1 + _Q2)>>1;
        	///V
        	BilinearInterpolation(qu[1].uv, qu[3].uv, qu[HD720P_WIDTH+1].uv, qu[HD720P_WIDTH+3].uv, lutbak, oPtr[rowIdx][colIdx+1].uv);
        	//_Q1 = LinearInterpolation(lutbak->xFraction,qu[1].uv,(qu + 3)->uv,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);
        	//_Q2 = LinearInterpolation(lutbak->yFraction,qu[1].uv,(qu + HD720P_WIDTH + 1)->uv,ONE_PER_AVM_LUT_FRACTION_BITS,AVM_LUT_FRACTION_BITS);
        	//oPtr[rowIdx][colIdx+1].uv = (_Q1 + _Q2)>>1;
        }

    	lut += childViewInfoLUT->pitch;
    }

    for(colIdx = 0; colIdx < HD720P_WIDTH ; colIdx++)
    {

    	YUYV *q = &oPtr[200][0];
    	q[colIdx].y = 0;
    	q[colIdx].uv = 0;
    	q[colIdx + HD720P_WIDTH + HD720P_WIDTH].y = 0;
    	q[colIdx + HD720P_WIDTH + HD720P_WIDTH].uv = 0;

    }


    return SYSTEM_LINK_STATUS_SOK;
}


#if SUPPORT_SHARPEN_FILTER
#define makeSingleView makeSingleView720PWithFilter
#else
#define makeSingleView makeSingleView720P
#endif
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
