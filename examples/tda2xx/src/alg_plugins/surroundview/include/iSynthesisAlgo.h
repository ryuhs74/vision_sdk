/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file iSynthesisAlgo.h
 *
 * \brief Interface file for Alg_Synthesis algorithm on DSP
 *
 *        This Alg_Synthesis algorithm is only for demonstrative purpose. 
 *        It is NOT product quality.
 *
 * \version 0.0 (Oct 2013) : [PS, VA, IP] First version
 *
 *******************************************************************************
 */

#ifndef _ISYNTHESISALGO_H_
#define _ISYNTHESISALGO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#ifndef PC_VERSION
	#include <include/link_api/system.h>
	
#endif

#include "memRequestAlgo.h"
#include "svCommonDefs.h"

#ifndef PC_VERSION
	#include <include/link_api/system_common.h>
	#include <src/utils_common/include/utils_mem.h>
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Structure containing the synthesis create time parameters
 *
 *******************************************************************************
*/
typedef struct
{		
	//Input frame size
	Word16		SVInCamFrmHeight; 
	Word16		SVInCamFrmWidth;  
	
	//output frame size
	Word16		SVOutDisplayHeight;
	Word16		SVOutDisplayWidth; 
	
	//number of color channels
	Byte		numColorChannels;
	
	//number of cameras
	Byte        numCameras;
	
	//car box dimensions and center location
	SV_CarBox_ParamsStruct svCarBoxParams;

	// Block Size for Statitsics collection (Should be 8,16,32,64...)
	Word16 blockSizeV;
	Word16 blockSizeH;

	// Blending Length (add default value)
	Byte	blendlen;
	// Offset location for seam
	Word32  seam_offset;

	Byte        outputMode; // 2D or 3D SRV

	// Subsample ratio for 3D SRV
	Byte         subsampleratio;

 // Set to 1, if DSP need to create the car image, apply only for 2D SRV */ 
 Bool  enableCarOverlayInAlg;

} SV_Synthesis_CreationParamsStruct;



/**
 *******************************************************************************
 *
 *   \brief Structure containing the synthesis control parameters
 *
 *******************************************************************************
*/
typedef struct
{
	Byte dummy; //to be extended later
} SV_Synthesis_ControlParams;



/*******************************************************************************
 *  Functions
 *******************************************************************************
 */


 /**
 *******************************************************************************
 *
 * \brief Implementation of memory query for synthesis algo
 *
 * \return  void
 *
 *******************************************************************************
 */
void Alg_SynthesisMemQuery(
    SV_Synthesis_CreationParamsStruct *Synthesis_createParams, 
    AlgLink_MemRequests *memPtr,
    Byte FirstTimeQuery);


 /**
 *******************************************************************************
 *
 * \brief Implementation of create for synthesis algo
 *
 * \param  pCreateParams    [IN] Creation parameters for synthesis Algorithm
 *								 and memory pointer
 *
 * \return  Handle to algorithm if creation is successful else return NULL
 *
 *******************************************************************************
 */
void *Alg_SynthesisCreate(SV_Synthesis_CreationParamsStruct *createParams, AlgLink_MemRequests *memPtr, uWord32 *outBlendLUTPtr);

/**
 *******************************************************************************
 *
 * \brief Implementation of Process for synthesis algo
 *
 *        Supported formats are SYSTEM_DF_YUV422I_YUYV, SYSTEM_DF_YUV420SP_UV 
 *        It is assumed that the width of the image will
 *        be multiple of 4 and buffer pointers are 32-bit aligned. 
 *        
 * \param  algHandle    [IN] Algorithm object handle
 * \param  inPtr[]      [IN] Matrix of input pointers
 *                           First Index: Plane Index
 *                              Index 0 - Pointer to Y data in case of YUV420SP, 
 *                                      - Single pointer for YUV422IL or RGB
 *                              Index 1 - Pointer to UV data in case of YUV420SP
 *                           Second Index: View ID Index
 *                              Index 0 - view ID 0
 *                              Index 1 - view ID 1 and so on.
 * \param  outPtr[]     [IN] Array of output pointers. 
 *                              Index 0 - Pointer to Y data in case of YUV420SP, 
 *                                      - Single pointer for YUV422IL or RGB
 *                              Index 1 - Pointer to UV data in case of YUV420SP
 * \param  outPitch[]   [IN] Array of pitch of output image (Address offset 
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  inGALUTPtr   [IN] Pointers for GA LUT table. All the three tables
 *                           given by GA will be considered as one single buffer 
 * \param  inPAlignLUTPtr [IN] Pointer for Photometric Alignment LUT
 * \param  outStatLUTPtr [IN] Pointer for statistics generated
 * \param  dataFormat    [IN] Different image data formats. Refer 
 *                           System_VideoDataFormat. Assumption that input
 *                           image and output image will have same data format.
 * \param  synthesisMode [IN] 0 - Simple synthesis from one view
 *                            1 - Blending synthesis from two views
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success 
 *
 *******************************************************************************
 */

Int32 Alg_SynthesisProcess(void               *svHandle,
						   System_VideoFrameCompositeBuffer  *pCompositeBuffer,
                           System_VideoFrameBuffer  *pVideoOutputBuffer,
						   uWord32             inPitch[],
                           //void               *inPtr[][MAX_NUM_VIEWS],
                           //void               *outPtr[],
                           uWord32             outPitch[], // --> outPitch needed only by Synthesis
                           void		           *inGALUTPtr,
						   uWord32              *inPAlignLUTPtr,
                           void            *outStatLUTPtr,
						   void               *outBlendLUTPtr,
                           uWord32             dataFormat,
						   uWord32             synthesisMode
                          );

/**
 *******************************************************************************
 *
 * \brief Implementation of Control for synthesis algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 * \param  pControlParams        [IN] Pointer to Control Params
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_SynthesisControl(void                       *pAlgHandle,
                           SV_Synthesis_ControlParams *pControlParams);

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for synthesis algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 * \param  memPtr                [IN] pointer to memory used by algo
 *
 * \return  
 *
 * \notes: 
 Framework would call this and the algorithm needs to populate AlgLink_MemRequests, like Memquery. But this time algorithm will give the pointers also, so that framework can free it up. 
 That means algorithm need to keep an internal copy of AlgLink_MemRequests, during create, so that you can send it back to framework during delete. 
 *******************************************************************************
 */
Int32 Alg_SynthesisDelete(void *pAlgHandle, 
                          AlgLink_MemRequests *memPtr);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* Nothing beyond this point */
