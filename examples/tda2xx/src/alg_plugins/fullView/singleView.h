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

typedef struct {
	unsigned short xFraction:AVM_LUT_FRACTION_BITS;
	unsigned short xInteger:AVM_LUT_INTEGER_BITS;
	unsigned short yFraction:AVM_LUT_FRACTION_BITS;
	unsigned short yInteger:AVM_LUT_INTEGER_BITS;
} ViewLUT_Packed;

#define HD720P_WIDTH	1280
typedef yuyv yuvHD720P[HD720P_WIDTH];

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

static inline Int32 makeView(  UInt32            *RESTRICT inPtr,
                           	   UInt32            *RESTRICT outPtr,
							   UInt32             width,
							   UInt32             height,
							   UInt32             inPitch,
							   UInt32             outPitch,
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

    iPtr  = (yuvHD720P*)inPtr;
    oPtr = (yuvHD720P*)outPtr;

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




#endif /* EXAMPLES_TDA2XX_SRC_ALG_PLUGINS_FULLVIEW_SINGLEVIEW_H_ */
