/*
 * utils_lut.h
 *
 *  Created on: Nov 5, 2015
 *      Author: craven
 */

#ifndef EXAMPLES_TDA2XX_SRC_DEVICES_UTILS_LUT_H_
#define EXAMPLES_TDA2XX_SRC_DEVICES_UTILS_LUT_H_

//ryuhs74@20151112 - Add View Mode ENUM & STRUCT
typedef enum
{
	TOP_VIEW,
	FULL_VIEW
}VIEW_MODE;

typedef enum
{
	FRONT_VIEW,
	REAR_VIEW,
	LEFT_VIEW,
	RIGHT_VIEW
}VIEW_NT;

typedef struct
{
	VIEW_MODE viewmode;
	VIEW_NT viewnt;
	VIEW_NT	prvVient;
}ViewMode;
// - END

typedef struct
{
	UInt16 startX;
	UInt16 startY;
	UInt16 width;
	UInt16 height;
}lut_Info;

typedef enum lut_Info_Index
{
	LUT_INFO_TOP_A00,
	LUT_INFO_TOP_A01,
	LUT_INFO_TOP_A02,
	LUT_INFO_TOP_A03,
	LUT_INFO_TOP_A04,
	LUT_INFO_TOP_A05,
	LUT_INFO_TOP_A06,
	LUT_INFO_TOP_A07,
	LUT_INFO_INDEX_MAX
}LUT_INFO_INDEX;

typedef enum lut_index
{
	Basic_frontView,
	Basic_rearView,
	Basic_leftSideView,
	Basic_rightSideView,
	Basic_frontNT,
	Basic_rearNT,
	Basic_leftNT,
	Basic_rightNT,
	cmaskNT,
	Basic_frontFullView,
	Basic_rearFullView,
	Basic_lutInfo,
	MAX_LUT_INDEX
}LUT_INDEX;

void* LUTAlloc(LUT_INDEX index );
void LUTFree(LUT_INDEX index);
void GetLutInfo(LUT_INFO_INDEX index,lut_Info* info);

#endif /* EXAMPLES_TDA2XX_SRC_DEVICES_UTILS_LUT_H_ */
