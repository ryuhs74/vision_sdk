/*
 * singleView.h
 *
 *  Created on: Nov 18, 2015
 *      Author: craven
 */

#ifndef EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_
#define EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_

#define AVM_LUT_INTEGER_BITS			10
#define AVM_LUT_FRACTION_BITS			6

typedef struct
{
	UInt8 y;
	UInt8 uv;
}yuyv;

typedef struct
{
	UInt16 width;
	UInt16 height;
	UInt16 startX;
	UInt16 startY;
	UInt16 pitch;
}ViewInfo;

typedef struct {
	unsigned short xFraction:AVM_LUT_FRACTI0ON_BITS;
	unsigned short xInteger:AVM_LUT_INTEGER_BITS;
	unsigned short yFraction:AVM_LUT_FRACTION_BITS;
	unsigned short yInteger:AVM_LUT_INTEGER_BITS;
} ViewLUT_Packed;

#define HD720P_WIDTH	1280
#define HD1080P_WIDTH	1920
typedef yuyv yuvHD720P[HD720P_WIDTH];
typedef yuyv yuvHD1080P[HD1080P_WIDTH];

///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolation(q, lut, Q,  pitch)\
{\
	UInt16 X =	lut->xFraction;\
	UInt16 Y =	lut->yFraction;\
	UInt16 A,B;\
	A = 64 - X;\
	B = 64 - Y;\
	Q = (UInt8)((B*(q[0].y*A + q[1].y*X) + Y*(q[pitch].y*A + q[pitch+1].y*X))>>12);\
}
///https://en.wikipedia.org/wiki/Bilinear_interpolation
#define BilinearInterpolationUV(q, lut, Q, pitch)\
{\
	UInt16 X =	lut->xFraction;\
	UInt16 Y =	lut->yFraction;\
	UInt16 A,B;\
	A = 64 - X;\
	B = 64 - Y;\
	Q = (UInt8)((B*(q[0].uv*A + q[2].uv*X) + Y*(q[pitch].uv*A + q[pitch+2].uv*X))>>12);\
}


static inline Int32 makeView720P(  UInt32       *RESTRICT inPtr,
                           	   UInt32           *RESTRICT outPtr,
							   UInt32			*RESTRICT viewLUTPtr,
							   ViewInfo			*RESTRICT viewInfo,
							   ViewInfo			*RESTRICT viewLUTInfo
                          )
{
	UInt16 rowIdx;
    UInt16 colIdx;

    yuvHD720P* iPtr;
    yuvHD720P* oPtr;

    UInt16 width = viewInfo->width + viewInfo->startX;

    ViewLUT_Packed *lut = ((ViewLUT_Packed*)viewLUTPtr) + (viewLUTInfo->pitch * viewLUTInfo->startY);

    iPtr  = (yuvHD720P*)inPtr;
    oPtr = ((yuvHD720P*)outPtr) + viewInfo->startY;

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(500,720, 2);
#endif
    for(rowIdx = 0; rowIdx < viewInfo->height ; rowIdx++)
    {
    	lut += viewLUTInfo->pitch;
    	ViewLUT_Packed *lutbak;
#ifdef BUILD_DSP
#pragma UNROLL(4);
#pragma MUST_ITERATE(500,1280, 4);
#endif
        for(colIdx = viewInfo->startX, lutbak = lut + viewLUTInfo->startX; colIdx < width ; colIdx++, lutbak++)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q, lutbak, oPtr[rowIdx][colIdx].y, HD720P_WIDTH);
        }
#if 1
#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(350,640, 2);
#endif
        for(colIdx = viewInfo->startX, lutbak = lut + viewLUTInfo->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	BilinearInterpolationUV(q, lutbak, oPtr[rowIdx][colIdx].uv, HD720P_WIDTH);
        	///V
        	q ++;
        	BilinearInterpolationUV(q, lutbak, oPtr[rowIdx][colIdx+1].uv, HD720P_WIDTH);
        }
#endif
    }
    return SYSTEM_LINK_STATUS_SOK;
}

static inline Int32 makeView1080P(  UInt32       *RESTRICT inPtr,
                           	   UInt32           *RESTRICT outPtr,
							   UInt32			*RESTRICT viewLUTPtr,
							   ViewInfo			*RESTRICT viewInfo,
							   ViewInfo			*RESTRICT viewLUTInfo
                          )
{
	UInt16 rowIdx;
    UInt16 colIdx;

    yuvHD1080P* iPtr;
    yuvHD1080P* oPtr;

    ViewLUT_Packed *lut = ((ViewLUT_Packed*)viewLUTPtr) + (viewLUTInfo->pitch * viewLUTInfo->startY);
    UInt16 width = viewInfo->width + viewInfo->startX;

    iPtr  = (yuvHD1080P*)inPtr;
    oPtr = ((yuvHD1080P*)outPtr) + viewInfo->startY;

#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(500,720, 2);
#endif
    for(rowIdx =0 ; rowIdx < viewInfo->height ; rowIdx++)
    {
    	lut += viewLUTInfo->pitch;
    	ViewLUT_Packed *lutbak;
#ifdef BUILD_DSP
#pragma UNROLL(4);
#pragma MUST_ITERATE(500,1280, 4);
#endif
        for(colIdx = viewInfo->startX, lutbak = lut + viewLUTInfo->startX; colIdx < width ; colIdx++, lutbak++)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger];

        	BilinearInterpolation(q, lutbak, oPtr[rowIdx][colIdx].y, HD1080P_WIDTH);
        }
#if 1
#ifdef BUILD_DSP
#pragma UNROLL(2);
#pragma MUST_ITERATE(350,640, 2);
#endif
        for(colIdx = viewInfo->startX, lutbak = lut + viewLUTInfo->startX; colIdx < width ; colIdx+=2, lutbak+=2)
        {
        	yuyv *q = &iPtr[lutbak->yInteger][lutbak->xInteger & 0xfffe];
        	///U
        	BilinearInterpolationUV(q, lutbak, oPtr[rowIdx][colIdx].uv, HD1080P_WIDTH);
        	///V
        	q ++;
        	BilinearInterpolationUV(q, lutbak, oPtr[rowIdx][colIdx+1].uv, HD1080P_WIDTH);
        }
#endif
    }
    return SYSTEM_LINK_STATUS_SOK;
}



static inline Int32 makeView(  UInt32       	*RESTRICT inPtr,
                           	   UInt32           *RESTRICT outPtr,
							   UInt32			*RESTRICT viewLUTPtr,
							   ViewInfo			*RESTRICT viewInfo,
							   ViewInfo			*RESTRICT viewLUTInfo
                          )
{
	viewInfo->width = viewInfo->width < viewLUTInfo->width + viewLUTInfo->startX ? viewInfo->width : viewLUTInfo->width + viewLUTInfo->startX;
	viewInfo->height = viewInfo->height < viewLUTInfo->height + viewLUTInfo->startY ? viewInfo->height : viewLUTInfo->height+ viewLUTInfo->startY;

	if(viewInfo->pitch == HD720P_WIDTH)
	{
		makeView720P(inPtr, outPtr, viewLUTPtr, viewInfo, viewLUTInfo);
	}else if(viewInfo->pitch == HD1080P_WIDTH)
	{
		makeView1080P(inPtr, outPtr, viewLUTPtr, viewInfo, viewLUTInfo);
	}else
	{
		return SYSTEM_LINK_STATUS_EINVALID_PARAMS;
	}

    return SYSTEM_LINK_STATUS_SOK;
}




#endif /* EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_ */
