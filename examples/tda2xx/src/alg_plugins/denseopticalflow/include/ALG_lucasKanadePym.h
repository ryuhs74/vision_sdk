/*
 *=======================================================================
 *
 * \file ALG_lukasKanadePym.h
 *
 * \brief Interface file for the Lucas-Kanade Optical Flow AlgLink
 *
 *
 *
 * \version 0.0 (NOV 1 2013) : BM
 *=======================================================================*/
#ifndef ALG_LUKASKANADEPYRM_H
#define ALG_LUKASKANADEPYRM_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	//TODO: The needed parameters of this structure are yet to be determined.
	unsigned int dummy;
} AlgLink_MemRequests;



/**
 *******************************************************************************
 *
 *   \brief enum indication the number of levels of the image pyramid to use.
 *
 *
 *******************************************************************************
*/
typedef enum{
	LKNUMPYR_1 = 1,
	LKNUMPYR_2,
	LKNUMPYR_3,
	LKNUMPYR_4,
	LKNUMPYR_5,
	LKNUMPYR_MAX = 5,
	LKNUMPYR_DEFAULT = 3,
    LKNUMPYR_FORCE32BITS = 0x7FFFFFFFUL
} Alg_LKnumPyr;

/**
 *******************************************************************************
 *
 *   \brief enum to set the size of the smoothing kernel used to preprocess
 *
 *
 *******************************************************************************
*/
typedef enum{
	LKSMOOTHSIZE_3x3 = 3,
	LKSMOOTHSIZE_5x5 = 5,
	LKSMOOTHSIZE_7x7 = 7,
	LKSMOOTHSIZE_MAX = 7,
	LKSMOOTHSIZE_DEFAULT = 3,
    LKSMOOTHSIZE_FORCE32BITS = 0x7FFFFFFFUL
} Alg_LKsmoothSize;

#define ALG_LK_NUM_STAGES LKNUMPYR_MAX + 1
#define ALG_LK_NUM_DMA_TRANSFERS_IN  2
#define ALG_LK_NUM_DMA_TRANSFERS_OUT 2
#define ALG_LK_PYR_NUM_DMA_TRANSFERS_IN  2
#define ALG_LK_PYR_NUM_DMA_TRANSFERS_OUT ((LKNUMPYR_MAX - 1) << 1)
#define ALG_LK_MULTI_NUM_DMA_TRANSFERS_IN 4
#define ALG_LK_MULTI_NUM_DMA_TRANSFERS_OUT 2

typedef struct {
	//TODO: The needed parameters of this structure are yet to be determined.
	void *pyrInPtr[ALG_LK_PYR_NUM_DMA_TRANSFERS_IN];
	void *pyrOutPtr[ALG_LK_PYR_NUM_DMA_TRANSFERS_OUT];
	void *blkInPtr[ALG_LK_NUM_DMA_TRANSFERS_IN];
	void *blkOutPtr[ALG_LK_NUM_DMA_TRANSFERS_OUT];
	void *multiInPtr[ALG_LK_MULTI_NUM_DMA_TRANSFERS_IN];
	void *multiOutPtr[ALG_LK_MULTI_NUM_DMA_TRANSFERS_OUT];

	int srcPyrPitch[ALG_LK_PYR_NUM_DMA_TRANSFERS_IN];
	int srcPitch[ALG_LK_NUM_DMA_TRANSFERS_IN];
	int srcMultiPitch[ALG_LK_MULTI_NUM_DMA_TRANSFERS_IN];

	int dstPyrPitch[ALG_LK_PYR_NUM_DMA_TRANSFERS_OUT];
	int dstPyrSize[ALG_LK_PYR_NUM_DMA_TRANSFERS_OUT];
	int dstPitch[ALG_LK_NUM_DMA_TRANSFERS_OUT];
	int dstMultiPitch[ALG_LK_MULTI_NUM_DMA_TRANSFERS_OUT];
	int dstVectorSize;
} Alg_LKdmaParams;

/**
 *******************************************************************************
 *
 *   \brief Structure of Lucas-Kanade optical flow algorithm paramters
 *
 *
 *******************************************************************************
*/
typedef struct {

	unsigned int imWidth;
	/**< Width of the valid image data in image1 and image2 in bytes*/
	unsigned int imHeight;
	/**< Height of the valid image data in image1 and image2 in rows*/
	unsigned int imPitch;
	/**< Number of bytes between the first pixel of two consecutive rows of an image*/
	unsigned int xRoiStart;
	/**< Horizontal position in bytes that the algorithm will start processing at*/
	unsigned int roiWidth;
	/**< Horizontal width of the ROI in bytes*/
	unsigned int yRoiStart;
	/**< Vertical position in rows that the algorithm will start processing at */
	unsigned int roiHeight;
	/**< Number of rows in the ROI*/
	Alg_LKnumPyr numPyr;
	/**< Number of levels in the pyramid used in the algorithm*/
	unsigned int enableSmoothing;
	/**< Enables image smoothing before processing. 1:enable 0:disable */
	Alg_LKsmoothSize smoothingKernSize;
	/**< Sets the width and height of the smoothing kernel. Setting enableSmoothing to 0 will cause this value to be unused.*/
	unsigned int maxVectorSizeX;
	/**< Clips u vectors (horizontal) to this magnitude*/
	unsigned int maxVectorSizeY;
	/**< Clips v vectors (vertical) to this magnitude*/
	void* scratch;
	/**< Scratch memory containing parameters and calculations stored between calls of the algorithm*/
	Alg_LKdmaParams dmaParams;
	/**< Parameters for setup and execution of DMA*/

    unsigned char *edmaAlgAutoIncrementContext1;
    /**< NEEDS TO BE ALLOCATED BY USER for size of EDMA_UTILS_AUTOINCREMENT_CONTEXT_SIZE */

    unsigned char *edmaAlgAutoIncrementContext2;
    /**< NEEDS TO BE ALLOCATED BY USER for size of EDMA_UTILS_AUTOINCREMENT_CONTEXT_SIZE */

    unsigned char *edmaAlgAutoIncrementContext3;
    /**< NEEDS TO BE ALLOCATED BY USER for size of EDMA_UTILS_AUTOINCREMENT_CONTEXT_SIZE */

} Alg_LKParams;

/**
 *******************************************************************************
 *
 * \brief   Mem estimate function for pyramidal Lucas-Kanade optical flow.
 *
 *          This function shall populate the structure AlgLink_MemRequests,
 *          to indicate the number of memory requests and attributes of each
 *          memory request.
 *
 * 	\param	params         [IN] A fully populated params structure except for the scratch member. The scratch member will be allocated by the initialization function
 * 	\param	memRequests    [IN] Pointer to strucutre to capture number of
 *                              memory requests and attributes of memory
 *                              requests.
 *
 * 	\return status			0 for success, negative for an error
 *
 *
 *******************************************************************************
*/
int Alg_LKPyrm_memEstimate(Alg_LKParams* params, AlgLink_MemRequests *memRequests);

/**
 *******************************************************************************
 *
 * \brief   Creation / Initialization function for pyramidal Lucas-Kanade optical flow.
 *
 *          This function performs argument sanity checking on the members of the params structure and allocates the necessary scratch memory
 *			based on the arguments to the scratch member. DMA engine initialization will also be performed.
 *          Framework shall call this function with pointer values in memRequests
 *          populated (For memory requests).
 *
 * 	\param	params         [IN] A fully populated params structure except for the scratch member. The scratch member will be allocated by the initialization function
 *
 * 	\return status			0 for success, negative for an error
 *
 *
 *******************************************************************************
*/
int Alg_LKPyrm_create(Alg_LKParams* params, AlgLink_MemRequests *memRequests);


/**
 *******************************************************************************
 *
 * \brief   Main Processing function for pyramidal Lucas-Kanade Optical Flow
 *
 *          This function implements pyramidal Lucas-Kanade optical flow on a pair of images
 *			and outputs the motion vectors (u and v) calculated by the algorithm.
 *
 * 	\param	image1         	[IN] An 8 bit monochrome image pointer. This image is the first (oldest) of the two
 *							images in the sequence. Must be the same size as image2.
 *
 *	\param  image2			[IN]An 8 bit monochrome image pointer. This is the second (newest) of the
 *							two images in the sequence. Must be the same size as image1.
 *
 *	\param	params			[IN]A pointer to a structure containing many algorithm parameters. See ALG_LKParams for definition
 *
 *	\param 	uVectors		[OUT] A pointer to an allocated buffer that the algorithm will output the horizontal vector values into.
 *							The size of the buffer should be equal to params.roiWidth * params.roiHeight bytes or larger
 *
 *	\param	vVectors		[OUT] A pointer to an allocated buffer that the algorithm will output the horizontal vector values into.
 *							The size of the buffer should be equal to params.roiWidth * params.roiHeight bytes or larger
 *
 * 	\return status			0 for success, negative for an error
 *
 *
 *******************************************************************************
*/
int Alg_LKPyrm_process(uint8_t* image1, uint8_t* image2, Alg_LKParams* params,int8_t* uVectors,int8_t* vVectors);


/**
 *******************************************************************************
 *
 * \brief   Delete / Deinitializtion function for pyramidal Lucas-Kanade optical flow.
 *
 *          This function performs the teardown of all resources allocated by ALG_LKPyrm_init, namely deallocating scratch memory DMA parameters.
 *          This function should populate memRequests as per what it received
 *          during creation, so that framework can free up those memories.
 *
 * 	\param	params         [IN] A fully populated params structure. The scratch and dmaParams will be deallocated by the function
 *
 * 	\return status			0 for success, negative for an error
 *
 *
 *******************************************************************************
*/
int Alg_LKPyrm_delete(Alg_LKParams* params, AlgLink_MemRequests *memRequests);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

