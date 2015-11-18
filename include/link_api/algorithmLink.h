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
 * \ingroup FRAMEWORK_MODULE_API
 * \defgroup ALGORITHM_LINK_API Algorithm Link API
 *
 * \brief  This module has the interface for using Algorithm Link
 *
 *         Algorithm Link is used to perform processing operations, which
 *         are executedon DSP / EVE.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink.h
 *
 * \brief Algorithm Link API common to all algorithm links
 *
 * \version 0.0 (Jul 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_H_
#define _ALGORITHM_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup ALGORITHM_LINK_API_CMD Algorithm Link Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Set / get the control / status of algorithm
 *
 *   SUPPORTED in ALL platforms
 *
 *   \param AlgorithmLink_ControlParams *pPrm [IN]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_CMD_CONFIG                      (0x7000)

/* @} */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the algorithms supported on DSP.
 *
 *          List of the supported algorithms. Alg ID to be configured by
 *          user/App while creating a specific algorithm link instance.
 *          Note that it is fine for more than one algorithm link instance
 *          to have the same algorithm Id.
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_DSP_ALG_FRAMECOPY = 0,
    /**< Alg to copy i/p frame to o/p frame */

    ALGORITHM_LINK_DSP_ALG_FULLVIEW = 0,
    /**< Alg to full view */

    ALGORITHM_LINK_DSP_ALG_COLORTOGRAY,
    /**< Alg to convert color image to gray image */

    ALGORITHM_LINK_DSP_ALG_DMA_SWMS,
    /**< Alg to DMA based SW Mosaic */

    ALGORITHM_LINK_DSP_ALG_SYNTHESIS,
    /**< Alg to perform synthesis for surround view */

    ALGORITHM_LINK_DSP_ALG_GALIGNMENT,
    /**< Alg to generate Geometric alignment LUTs for surroundview synthesis */

    ALGORITHM_LINK_DSP_ALG_ULTRASONICFUSION,
    /**< Alg to ultrasonic detections with surroundview */

    ALGORITHM_LINK_DSP_ALG_PALIGNMENT,
    /**< Alg to generate Photometric alignment LUTs for surroundview synthesis*/

    ALGORITHM_LINK_DSP_ALG_VECTORTOIMAGE,
    /**< Alg to generate Color image from optical flow vectors*/

    ALGORITHM_LINK_DSP_ALG_OBJECTDETECTION,
    /**< Alg to compute feature planes for Object detection */

    ALGORITHM_LINK_DSP_ALG_SOF_DRAW,
    /**< Alg to draw arrows on the video (Needed by SOF)*/

    ALGORITHM_LINK_DSP_ALG_LANE_DETECT,
    /**< Alg to detect lanes in a video */

    ALGORITHM_LINK_DSP_ALG_LANE_DETECT_DRAW,
    /**< Alg to draw detected lanes in a video */

    ALGORITHM_LINK_DSP_ALG_STEREO_POST_PROCESS,
    /**< Alg to do post processing on disparity output*/

    ALGORITHM_LINK_DSP_ALG_MAXNUM,
    /**< Should be the last value of this enumeration.
     *   Will be used by Link/driver for validating the input parameters. */
    ALGORITHM_LINK_DSP_ALG_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_DspAlgorithmId;

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the algorithms supported on EVE
 *
 *          List of the supported algorithms. Alg ID to be configured by
 *          user/App while creating a specific algorithm link instance.
 *          Note that it is fine for more than one algorithm link instance
 *          to have the same algorithm Id.
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_EVE_ALG_FRAMECOPY = 0,
    /**< Alg to copy i/p frame to o/p frame */

    ALGORITHM_LINK_EVE_ALG_FULLVIEW = 0,
    /**< Alg to full view */

    ALGORITHM_LINK_EVE_ALG_DENSE_OPTICAL_FLOW,
    /**< Alg to generate LUT to show optical Flow */

    ALGORITHM_LINK_EVE_ALG_EDGEDETECTION,
    /**< Alg to detect edge in image */

    ALGORITHM_LINK_EVE_ALG_FEATUREPLANECOMPUTE,
    /**< Alg to compute feature planes for Object detection */

    ALGORITHM_LINK_EVE_ALG_SPARSE_OPTICAL_FLOW,
    /**< Alg to get flow vectors(Sparse Optical Flow) */

    ALGORITHM_LINK_EVE_ALG_SOFTISP,
    /**< Alg to process RCCC->CFA->Y(RCCC Soft ISP) */

    ALGORITHM_LINK_EVE_ALG_CENSUS,
    /**< Alg to process RCCC->CFA->Y(RCCC Soft ISP) */

    ALGORITHM_LINK_EVE_ALG_DISPARITY_HAMDIST,
    /**< Alg to process RCCC->CFA->Y(RCCC Soft ISP) */

    ALGORITHM_LINK_EVE_ALG_SUBFRAMECOPY,
    /**< Alg to copy subframe*/

    ALGORITHM_LINK_EVE_ALG_REMAPMERGE,
    /**< Alg to copy subframe*/

    ALGORITHM_LINK_EVE_ALG_MAXNUM,
    /**< Should be the last value of this enumeration.
     *   Will be used by Link/driver for validating the input parameters. */

    ALGORITHM_LINK_EVE_ALG_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_EveAlgorithmId;

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the algorithms supported on A15
 *
 *          List of the supported algorithms. Alg ID to be configured by
 *          user/App while creating a specific algorithm link instance.
 *          Note that it is fine for more than one algorithm link instance
 *          to have the same algorithm Id.
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_A15_ALG_FRAMECOPY = 0,
    /**< Alg to copy i/p frame to o/p frame */

    ALGORITHM_LINK_A15_ALG_FULLVIEW = 0,
    /**< Alg to full view */

    ALGORITHM_LINK_A15_ALG_COLORTOGRAY,
    /**< Alg to convert color image to gray image */

    ALGORITHM_LINK_A15_ALG_DMA_SWMS,
    /**< Alg to DMA based SW Mosaic */

    ALGORITHM_LINK_A15_ALG_SOF_DRAW,
    /**< Alg to draw arrows on the video (Needed by SOF)*/

    ALGORITHM_LINK_A15_ALG_LANE_DETECT_DRAW,
    /**< Alg to draw detected lanes in a video */

    ALGORITHM_LINK_A15_ALG_MAXNUM,
    /**< Should be the last value of this enumeration.
     *   Will be used by Link/driver for validating the input parameters. */
    ALGORITHM_LINK_A15_ALG_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_A15AlgorithmId;

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the algorithms supported on M4
 *
 *          List of the supported algorithms. Alg ID to be configured by
 *          user/App while creating a specific algorithm link instance.
 *          Note that it is fine for more than one algorithm link instance
 *          to have the same algorithm Id.
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_IPU_ALG_DMA_SWMS = 0,
    /**< Alg to DMA based SW Mosaic */

    ALGORITHM_LINK_IPU_ALG_PD_DRAW,
    /**< Alg to draw rectangles on the image (Needed by PD) */

    ALGORITHM_LINK_IPU_ALG_ISS_AEWB,
    /**< AEWB for ISS running on IPU1-0. Only valid for TDA3x */

    ALGORITHM_LINK_IPU_ALG_ISS_AEWB1,
    /**< AEWB for ISS running on IPU1-0. Only valid for TDA3x */

    ALGORITHM_LINK_IPU_ALG_HW_CRC,
    /**< CRC for checking Frame Freeze Detect on IPU1-0. Only valid for TDA3x */

    ALGORITHM_LINK_IPU_ALG_MAXNUM,
    /**< Should be the last value of this enumeration.
     *   Will be used by Link/driver for validating the input parameters. */

    ALGORITHM_LINK_IPU_ALG_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_IpuAlgorithmId;

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the SRV algorithms ouput modes supported
 *
 *          Surround view Alg can support either 2D or 3D ouput modes.
 *          user/App while creating need to specify one of this modes.
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_SRV_OUTPUT_2D = 0,
    /**< 2D surround view stitching/creation on DSP */

    ALGORITHM_LINK_SRV_OUTPUT_3D,
    /**< 3D surround view stitching/creation on A15-SGX */

    ALGORITHM_LINK_SRV_OUTPUT_MAXNUM,
    /**< Should be the last value of this enumeration.
     *   Will be used by Link/driver for validating the input parameters. */

    ALGORITHM_LINK_SRV_OUTPUT_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_SrvOutputModes;
/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Algorithm link create time parameters
 *
 *          Create parameters for an algorithm will be defined based on the
 *          algorithm. This structure is the base class and algorithm
 *          specific parameters will be extended class elements. First element
 *          of extended structure should be base class structure and first
 *          element in base class has to be size element, which indicates
 *          size of extended or base structure, depending on what is used.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   size;
    /**< Size of the structure */
    UInt32                   algId;
    /**< Used to select the algorithm to be used in this link instance.
     *   See AlgorithmLink_algorithmID for the supported values */
} AlgorithmLink_CreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Algorithm link control parameters
 *
 *          Control parameters for an algorithm will be defined based on the
 *          algorithm. This structure is the base class and algorithm
 *          specific parameters will be extended class elements. First element
 *          of extended structure should be base class structure and first
 *          element in base class has to be size element, which indicates
 *          size of extended or base structure, depending on what is used.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   size;
    /**< Size of the structure */

    UInt32                   controlCmd;
    /**< Plugin defined control command */

} AlgorithmLink_ControlParams;

/**
 *******************************************************************************
 *
 *   \brief ROI Parameters
 *
 *******************************************************************************
*/
typedef struct {

    UInt32 startX;
    /**< ROI start X */

    UInt32 startY;
    /**< ROI start Y */

    UInt32 width;
    /**< ROI width */

    UInt32 height;
    /**< ROI height  */

} AlgorithmLink_RoiParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Algorithm link register and init function
 *
 *          For each algorithm link instance
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_init();

/**
 *******************************************************************************
 *
 *   \brief Algorithm link de-register and de-init function
 *
 *          For each algorithm link instance
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_deInit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
