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
	MAX_LUT_INDEX,
}LUT_INDEX;

uint8_t* LUTAlloc(LUT_INDEX index );
void LUTFree(uint8_t* lutAddress);

#endif /* EXAMPLES_TDA2XX_SRC_DEVICES_UTILS_LUT_H_ */
