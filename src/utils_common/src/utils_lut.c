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
#include <src/utils_common/include/utils_mem.h>
#include "lutNor.h"

typedef struct
{
	Ptr address;
	UInt32 size;
}lut_memInfo;

lut_memInfo memInfo[MAX_LUT_INDEX];

void* LUTAlloc(LUT_INDEX index )
{
	UInt32 size = 0;
	Ptr lut = (Ptr)GetLUT((LUT_NOR_INDEX)index, (uint32_t*)&size);

	Ptr lut_ddr = Utils_memAlloc(
	            UTILS_HEAPID_DDR_CACHED_SR,
	            size,
	            32);
	memcpy(lut_ddr,lut,size);

	memInfo[index].address = lut_ddr;
	memInfo[index].size = size;

	return (void*)lut_ddr;
}


void LUTFree(LUT_INDEX index)
{
	if(memInfo[index].address == NULL)
		return;
	Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,memInfo[index].address,memInfo[index].size);
	memInfo[index].address = NULL;
	memInfo[index].size = 0;
}

