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

#define LUT_CHECK	0

typedef struct
{
	Ptr address;
	UInt32 size;
}lut_memInfo;

static lut_memInfo memInfo[MAX_LUT_INDEX] = {0};

void* LUTAlloc(LUT_INDEX index )
{

	typedef struct {
		unsigned short xFraction:5;
		unsigned short xInteger:11;
		unsigned short yFraction:5;
		unsigned short yInteger:11;
	} ViewLUT_Packed;


	UInt32 size = 0;

	if(memInfo[index].address!=NULL)
		return (void*)memInfo[index].address;

	Ptr lut = (Ptr)GetLUT((LUT_NOR_INDEX)index, (uint32_t*)&size);

	if (lut == NULL)
		return NULL;

	Ptr lut_ddr = Utils_memAlloc(
	            UTILS_HEAPID_DDR_CACHED_SR,
	            size,
	            32);
	memcpy(lut_ddr,lut,size);

#if LUT_CHECK
	{
		int k = 0;
		ViewLUT_Packed* lut = (ViewLUT_Packed*)lut_ddr;

		for (k=0; k<(size>>1); k++, lut++)
		{
			if((*lut).xInteger >1280)
				Vps_printf("\n %d LUT X Warning[%d]\n", k, (*lut).xInteger);
			if((*lut).yInteger >720)
				Vps_printf("\n %d LUT Y Warning[%d]\n", k, (*lut).yInteger);

			Vps_printf("(%d, %d)",(*lut).xInteger, (*lut).yInteger);

			if(k%32 == 0)
			{
				Vps_printf("\n");
			}
		}
	}
#endif

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

lut_Info* GetLutInfo(LUT_INFO_INDEX index)
{
	lut_Info* lutInfo = (lut_Info*)LUTAlloc(Basic_lutInfo);

	return &lutInfo[index];
}
