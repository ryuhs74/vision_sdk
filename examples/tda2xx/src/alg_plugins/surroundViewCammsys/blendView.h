/*
 * blendView.h
 *
 *  Created on: Nov 20, 2015
 *      Author: craven
 */

#ifndef EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_BLENDVIEW_H_
#define EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_BLENDVIEW_H_


#include "singleView.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>

static inline Int32 makeBlendView720P(  UInt32       *RESTRICT inPtr_main,
										UInt32       *RESTRICT inPtr_sub,
										UInt32       *RESTRICT buf1,
										UInt32       *RESTRICT buf2,
										UInt32       *RESTRICT outPtr,
										UInt32		 *RESTRICT viewLUTPtr_main,
										UInt32		 *RESTRICT viewLUTPtr_sub,
										UInt32		 *RESTRICT carMask,
										ViewInfo	 *RESTRICT viewInfo,
										ViewInfo	 *RESTRICT childViewInfoLUT
                          )
{
//	UInt32 mallocSize = childViewInfoLUT->height * (childViewInfoLUT->pitch>>1);

	yuvHD720P* mainBuf = (yuvHD720P*)buf1;
	yuvHD720P* subBuf =  (yuvHD720P*)buf2;

	yuvHD720P* oPtr = (yuvHD720P*)outPtr;
	UInt16 rowIdx;
    UInt16 colIdx;

	UInt16 startX = viewInfo->startX + childViewInfoLUT->startX;
	MaskLUT_Packed* mask = ((MaskLUT_Packed*)carMask) + (childViewInfoLUT->pitch * childViewInfoLUT->startY) + childViewInfoLUT->startX;


	makeSingleView720P(inPtr_main,(UInt32*)mainBuf,viewLUTPtr_main,viewInfo,childViewInfoLUT);

	makeSingleView720P(inPtr_sub,(UInt32*)subBuf,viewLUTPtr_sub,viewInfo,childViewInfoLUT);

	oPtr+= (viewInfo->startY + childViewInfoLUT->startY);
	mainBuf += (viewInfo->startY + childViewInfoLUT->startY);
	subBuf += (viewInfo->startY + childViewInfoLUT->startY);

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(100,720, 2);
#endif
	for(rowIdx = 0; rowIdx < childViewInfoLUT->height; rowIdx++)
	{
		MaskLUT_Packed *maskBak;
		mask += childViewInfoLUT->pitch;

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(100,720, 2);
#endif
		for(colIdx = 0,maskBak = mask; colIdx < childViewInfoLUT->width; colIdx++, maskBak++)
		{
			yuyv q1 = mainBuf[rowIdx][colIdx+startX];
			yuyv q2 = subBuf[rowIdx][colIdx+startX];
			UInt16 X = maskBak->cr_r_overlay;

			oPtr[rowIdx][colIdx+startX].y = LinearInterpolation(X,q2.y,q1.y,256,8);
			oPtr[rowIdx][colIdx+startX].uv = LinearInterpolation(X,q2.uv,q1.uv,256,8);
		}
	}

    return SYSTEM_LINK_STATUS_SOK;
}

static inline Int32 makeBlendView1080P(  UInt32       *RESTRICT inPtr_main,
											UInt32       *RESTRICT inPtr_sub,
											UInt32       *RESTRICT outPtr[],
											UInt32		 *RESTRICT viewLUTPtr_main,
											UInt32		 *RESTRICT viewLUTPtr_sub,
											UInt32		 *RESTRICT carMask,
											ViewInfo	 *RESTRICT viewInfo,
											ViewInfo	 *RESTRICT childViewInfoLUT
	                          )
{
	UInt32 mallocSize = childViewInfoLUT->height * (childViewInfoLUT->pitch>>1);

	yuvHD1080P* mainBuf = (yuvHD1080P*)Utils_memAlloc(
			UTILS_HEAPID_DDR_CACHED_SR,
			mallocSize,
			32
	);
	yuvHD1080P* subBuf = (yuvHD1080P*)Utils_memAlloc(
			UTILS_HEAPID_DDR_CACHED_SR,
			mallocSize,
			32
	);

	yuvHD1080P* oPtr = (yuvHD1080P*)outPtr;
	UInt16 rowIdx;
    UInt16 colIdx;

	UInt16 startX = viewInfo->startX;
	MaskLUT_Packed* mask = ((MaskLUT_Packed*)carMask) + (childViewInfoLUT->pitch * childViewInfoLUT->startY) + childViewInfoLUT->startX;

	oPtr+= viewInfo->startY;

	makeSingleView1080P(inPtr_main,(UInt32*)mainBuf,viewLUTPtr_main,viewInfo,childViewInfoLUT);

	makeSingleView1080P(inPtr_sub,(UInt32*)subBuf,viewLUTPtr_sub,viewInfo,childViewInfoLUT);

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(100,720, 2);
#endif
	for(rowIdx = 0; rowIdx < childViewInfoLUT->height; rowIdx++)
	{
		MaskLUT_Packed *maskBak;
		mask += childViewInfoLUT->pitch;

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(100,720, 2);
#endif
		for(colIdx = 0,maskBak = mask; colIdx < childViewInfoLUT->width; colIdx++, maskBak++)
		{
			yuyv q1 = mainBuf[rowIdx][colIdx];
			yuyv q2 = subBuf[rowIdx][colIdx];
			UInt16 X = maskBak->cr_r_overlay;

			oPtr[rowIdx][colIdx+startX].y = LinearInterpolation(X,q1.y,q2.y,256,8);
		}
	}


	Utils_memFree(
			UTILS_HEAPID_DDR_CACHED_SR,
			mainBuf,
			mallocSize
	);
	Utils_memFree(
			UTILS_HEAPID_DDR_CACHED_SR,
			subBuf,
			mallocSize
	);
    return SYSTEM_LINK_STATUS_SOK;
}



static inline Int32 makeBlendView(  UInt32       *RESTRICT inPtr_main,
									UInt32       *RESTRICT inPtr_sub,
									UInt32       *RESTRICT buf1,
									UInt32       *RESTRICT buf2,
									UInt32       *RESTRICT outPtr,
									UInt32		 *RESTRICT viewLUTPtr_main,
									UInt32		 *RESTRICT viewLUTPtr_sub,
									UInt32		 *RESTRICT carMask,
									ViewInfo	 *RESTRICT viewInfo,
									ViewInfo	 *RESTRICT childViewInfoLUT
                      )
{
	viewInfo->width = viewInfo->width < childViewInfoLUT->width + childViewInfoLUT->startX ? viewInfo->width : childViewInfoLUT->width + childViewInfoLUT->startX;
	viewInfo->height = viewInfo->height < childViewInfoLUT->height + childViewInfoLUT->startY ? viewInfo->height : childViewInfoLUT->height+ childViewInfoLUT->startY;

	if(viewInfo->pitch == HD720P_WIDTH)
	{
		makeBlendView720P(inPtr_main, inPtr_sub, buf1, buf2, outPtr, viewLUTPtr_main, viewLUTPtr_sub, carMask, viewInfo, childViewInfoLUT);
	}else if(viewInfo->pitch == HD1080P_WIDTH)
	{
		//makeBlendView1080P(inPtr_main, inPtr_sub, buf1, buf2, outPtr, viewLUTPtr_main, viewLUTPtr_sub, carMask, viewInfo, childViewInfoLUT);
	}else
	{
		return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
	}

    return SYSTEM_LINK_STATUS_SOK;
}




#endif /* EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_BLENDVIEW_H_ */
