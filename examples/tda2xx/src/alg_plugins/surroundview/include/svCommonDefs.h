//To include system type def header
#ifndef _SV_COMMONDEFS_H_
#define _SV_COMMONDEFS_H_

#define YUV_offset  128

#define GAlignLUT_BitPerEntry (2+2) //The number of bits for each entry in GAlign LUT
#define BlendLUT_BitPerEntry (1+1) //The number of bits for each entry in Blend LUT
#define PAlignStat_BitPerEntry (2) //The number of bytes for each Statistic entry
#define MAX_NUM_VIEWS (4) //maximum number of views 
#define NUM_MAX_COLORPLANES (3)

#define ALGORITHM_PROCESS_OK 0
#define ALGORITHM_PROCESS_FAIL -1
#define ALGORITHM_PROCESS_DISABLED 2

#define PHOTOMETRIC_ALIGNMENT_ON  1 

#define NUM_FEAT_POINTS_PER_CHART 4

/*-----------
UFusion
------------*/
//number of points to approximate arc
#define NUM_ARC_POINTS (9)
//scale of ARGB4444 overlay image size compared to SV image size
#define SV_UF_OVERLAYDATA_SCALE (1)

#ifndef PC_VERSION
	#include <include/link_api/system.h>
#endif
#ifdef PC_VERSION
	#include "../../system/system_simulator.h"
	#include <stdio.h> 
	#include <math.h>
	#include <stdlib.h>
	#include <string.h>
#endif

//DATA TYPE DEFINITIONS-----------------------------------------------------------------


typedef UInt32  uWord32;
typedef Int32   Word32;
typedef UInt16  uWord16;  
typedef Int16   Word16;
typedef UInt8   Byte;

typedef struct{
	Word16 CarBoxCenter_x;  //x coordinate of car box in output frame
	Word16 CarBoxCenter_y;  //y coordinate of car box in output frame
	Word16 CarBox_height;   //height of car box
	Word16 CarBox_width;    //width of car box
} SV_CarBox_ParamsStruct;  //structure to hold car box parameters

typedef struct{
	Word32 PAlignOut_Gain[MAX_NUM_VIEWS*NUM_MAX_COLORPLANES]; //Gains for each channel
	Byte PAlignOut_LUT[MAX_NUM_VIEWS*NUM_MAX_COLORPLANES * 256]; //LUT for each channel
} PAlignOutStruct; //structure for PAlign output


#endif
