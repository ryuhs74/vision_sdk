/*
 *  utils_lut.c
 *
 *  Created on: Nov 5, 2015
 *      Author: craven
 */

#include <stdint.h>
#include <include/link_api/system.h>
#include <fvid2/fvid2.h>
#include <vps/vps.h>
#include <src/utils_common/include/utils_lut.h>
#include "lutNor.h"


uint8_t* LUTAlloc(LUT_INDEX index )
{
	return GetLUT((LUT_NOR_INDEX)index);
}


void LUTFree(uint8_t* lutAddress)
{

}

