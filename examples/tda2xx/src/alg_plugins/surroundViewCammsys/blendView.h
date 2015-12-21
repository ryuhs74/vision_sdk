/*
 * blendView.h
 *
 *  Created on: Nov 20, 2015
 *      Author: craven
 *       * Copyright (C) 2015 Cammsys - http://www.cammsys.net/
 */

#ifndef EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_BLENDVIEW_H_
#define EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_BLENDVIEW_H_


#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>
#include "singleView.h"

#define BLEND_VIEW_TEMP_BUF_SIZE	260*500

#define ONE_PER_AVM_BLEND_FRACTION_BITS	256

static inline Int32 makeBlendView720P(  UInt32       *RESTRICT inPtr_main,
										UInt32       *RESTRICT inPtr_sub,
										UInt8       *RESTRICT buf1,
										UInt8       *RESTRICT buf2,
										UInt32       *RESTRICT outPtr,
										UInt32		 *RESTRICT viewLUTPtr_main,
										UInt32		 *RESTRICT viewLUTPtr_sub,
										UInt32		 *RESTRICT carMask,
										AlgorithmLink_SurroundViewLutInfo	 *RESTRICT viewInfo,
										AlgorithmLink_SurroundViewLutInfo	 *RESTRICT childViewInfoLUT
                          )
{
//	UInt32 mallocSize = childViewInfoLUT->height * (childViewInfoLUT->pitch>>1);

	yuvHD260Pixel* mainBuf = (yuvHD260Pixel*)buf1;
	yuvHD260Pixel* subBuf =  (yuvHD260Pixel*)buf2;

	yuvHD720P* oPtr = (yuvHD720P*)outPtr;
	UInt16 rowIdx;
    UInt16 colIdx;

	UInt16 startX = viewInfo->startX + childViewInfoLUT->startX;
	MaskLUT_Packed* mask = ((MaskLUT_Packed*)carMask) + (childViewInfoLUT->pitch * childViewInfoLUT->startY) + childViewInfoLUT->startX;


	makeSingleView720PBuff(inPtr_main,(UInt32*)mainBuf,viewLUTPtr_main,viewInfo,childViewInfoLUT);

	makeSingleView720PBuff(inPtr_sub,(UInt32*)subBuf,viewLUTPtr_sub,viewInfo,childViewInfoLUT);

	oPtr+= (viewInfo->startY + childViewInfoLUT->startY);
//	mainBuf += (viewInfo->startY + childViewInfoLUT->startY);
//	subBuf += (viewInfo->startY + childViewInfoLUT->startY);


	for(rowIdx = 0; rowIdx < childViewInfoLUT->height; rowIdx++)
	{
		MaskLUT_Packed *maskBak;
		for(colIdx = 0,maskBak = mask; colIdx < childViewInfoLUT->width; colIdx++, maskBak++)
		{
			YUYV q1 = mainBuf[rowIdx][colIdx];
			YUYV q2 = subBuf[rowIdx][colIdx];
			UInt16 X = maskBak->cr_r_overlay;

			oPtr[rowIdx][colIdx+startX].y = LinearInterpolation(X,q2.y,q1.y,ONE_PER_AVM_BLEND_FRACTION_BITS,8);
			oPtr[rowIdx][colIdx+startX].uv = LinearInterpolation(X,q2.uv,q1.uv,ONE_PER_AVM_BLEND_FRACTION_BITS,8);
		}
		mask += childViewInfoLUT->pitch;
	}

    return SYSTEM_LINK_STATUS_SOK;
}
static inline Int32 makeBlendView720PWidthSharpen(	UInt32 *RESTRICT inPtr_main,
													UInt32 *RESTRICT inPtr_sub,
													UInt8 *RESTRICT buf1,
													UInt8 *RESTRICT buf2,
													UInt32 *RESTRICT outPtr,
													UInt32 *RESTRICT viewLUTPtr_main,
													UInt32 *RESTRICT viewLUTPtr_sub,
													UInt32 *RESTRICT carMask,
													AlgorithmLink_SurroundViewLutInfo *RESTRICT viewInfo,
													AlgorithmLink_SurroundViewLutInfo *RESTRICT childViewInfoLUT)
{
//	UInt32 mallocSize = childViewInfoLUT->height * (childViewInfoLUT->pitch>>1);

	uvHD260Pixel* mainBufUV = (uvHD260Pixel*)buf1;
	yHD260Pixel* mainBufY = (yHD260Pixel*)(buf1 + (BLEND_VIEW_TEMP_BUF_SIZE >> 1));

	uvHD260Pixel* subBufUV = (uvHD260Pixel*)buf2;
	yHD260Pixel* subBufY = (yHD260Pixel*)(buf2 + (BLEND_VIEW_TEMP_BUF_SIZE >> 1));

	yHD260Pixel* FilterBuffIn  = (yHD260Pixel*)mainBufUV;
	yHD260Pixel* FilterBuffOut = (yHD260Pixel*)subBufUV;
	yuvHD720P* oPtr = (yuvHD720P*)outPtr;
	UInt16 rowIdx;
    UInt16 colIdx;

    UInt16 width = childViewInfoLUT->width;
    UInt16 height = childViewInfoLUT->height;

	UInt16 startX = viewInfo->startX + childViewInfoLUT->startX;
	MaskLUT_Packed* maskOry = ((MaskLUT_Packed*)carMask) + (childViewInfoLUT->pitch * childViewInfoLUT->startY) + childViewInfoLUT->startX;
	MaskLUT_Packed* mask = maskOry;

	makeSingleView720PBuffConv422SP(inPtr_main,(UInt32*)mainBufY,(UInt32*)mainBufUV,viewLUTPtr_main,viewInfo,childViewInfoLUT);

	makeSingleView720PBuffConv422SP(inPtr_sub,(UInt32*)subBufY,(UInt32*)subBufUV,viewLUTPtr_sub,viewInfo,childViewInfoLUT);

	oPtr+= (viewInfo->startY + childViewInfoLUT->startY);

	for(rowIdx = 0; rowIdx < height; rowIdx++)
	{
		MaskLUT_Packed *maskBak;
		for(colIdx = 0,maskBak = mask; colIdx < width; colIdx++, maskBak++)
		{
			UV q1 = mainBufUV[rowIdx][colIdx];
			UV q2 = subBufUV[rowIdx][colIdx];
			UInt16 X = maskBak->cr_r_overlay;

			oPtr[rowIdx][colIdx+startX].uv = LinearInterpolation(X,q2.uv,q1.uv,ONE_PER_AVM_BLEND_FRACTION_BITS,8);
		}
		mask += childViewInfoLUT->pitch;
	}
	mask = maskOry;
	for(rowIdx = 0; rowIdx < height; rowIdx++)
	{
		MaskLUT_Packed *maskBak;
		for(colIdx = 0,maskBak = mask; colIdx < width; colIdx++, maskBak++)
		{
			UInt8 q1 = mainBufY[rowIdx][colIdx];
			UInt8 q2 = subBufY[rowIdx][colIdx];
			UInt16 X = maskBak->cr_r_overlay;

			FilterBuffIn[rowIdx][colIdx] = LinearInterpolation(X,q2,q1,ONE_PER_AVM_BLEND_FRACTION_BITS,8);
			FilterBuffOut[rowIdx][colIdx+3] = FilterBuffIn[rowIdx][colIdx];
		}
		mask += childViewInfoLUT->pitch;
	}



#ifdef BUILD_DSP
	{
		char sharpen_mask[3][3] =
		{
		{ -9, 1, -9 },
		{ 1, 96, 1 },
		{ -9, 1, -9 } };
		char sharpen_shift = 6;
		int _width = ((width) & 0xfffc)-4;
		for (rowIdx = 0; rowIdx < (height - 2); rowIdx++)
		{
			IMG_conv_3x3_i8_c8s(FilterBuffIn[rowIdx],
								FilterBuffOut[rowIdx+1]+4,
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
        for(colIdx = startX; colIdx < width + startX ; colIdx++, filtercolIdx++)
        {
        	oPtr[rowIdx][colIdx].y = FilterBuffOut[rowIdx][filtercolIdx];
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}
#if SUPPORT_SHARPEN_FILTER
#define makeBlendView makeBlendView720PWidthSharpen
#else
#define	makeBlendView makeBlendView720P
#endif

#if 0
static inline Int32 makeBlendView(  UInt32       *RESTRICT inPtr_main,
									UInt32       *RESTRICT inPtr_sub,
									UInt32       *RESTRICT buf1,
									UInt32       *RESTRICT buf2,
									UInt32       *RESTRICT outPtr,
									UInt32		 *RESTRICT viewLUTPtr_main,
									UInt32		 *RESTRICT viewLUTPtr_sub,
									UInt32		 *RESTRICT carMask,
									AlgorithmLink_SurroundViewLutInfo	 *RESTRICT viewInfo,
									AlgorithmLink_SurroundViewLutInfo	 *RESTRICT childViewInfoLUT
                      )
{
	viewInfo->width = viewInfo->width < childViewInfoLUT->width + childViewInfoLUT->startX ? viewInfo->width : childViewInfoLUT->width + childViewInfoLUT->startX;
	viewInfo->height = viewInfo->height < childViewInfoLUT->height + childViewInfoLUT->startY ? viewInfo->height : childViewInfoLUT->height+ childViewInfoLUT->startY;
	makeBlendView720P(inPtr_main, inPtr_sub, buf1, buf2, outPtr, viewLUTPtr_main, viewLUTPtr_sub, carMask, viewInfo, childViewInfoLUT);


    return SYSTEM_LINK_STATUS_SOK;
}
#endif



#endif /* EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_BLENDVIEW_H_ */
