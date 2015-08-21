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
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_SYNTHESIS_API Algorithm Plugin: Synthesis API
 *
 * \brief  This module has the interface for using synthesis algorithm
 *
 *         Synthesis algorithm link -
 *           1. Will perform stiching of multiple views to generate surround
 *              view
 *               - Number of views can be 4 or 6
 *               - Synthesis mode can be simple (Using single view pixels) or
 *                 blending based (Using blend of pixels from two views)
 *           2. Generates certain Photometric statistics which will be used
 *              by Photometric alignment link
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_synthesis.h
 *
 * \brief Algorithm Link API specific to synthesis algorithm
 *
 * \version 0.0 (Oct 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_SYNTHESIS_H_
#define _ALGORITHM_LINK_SYNTHESIS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>

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
 *  \brief  Enumerations for the algorithms supported on DSP.
 *
 *          Method of copying frame
 *
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_ALG_SIMPLESYNTHESIS = 0,
    /**< Simple synthesis using pixels from single view */
    ALGORITHM_LINK_ALG_BLENDINGSYNTHESIS,
    /**< Synthesis by blending pixels from two views */
    ALGORITHM_LINK_ALG_SYNTHESIS_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_SynthesisMode;

/**
 *******************************************************************************
 * \brief Enum for the input Q IDs
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    ALGLINK_SYNTHESIS_IPQID_MULTIVIEW = 0x0,
    /**< QueueId for multiview images */

    ALGLINK_SYNTHESIS_IPQID_GALUT,
    /**< QueueId for GA LUTs */

    ALGLINK_SYNTHESIS_IPQID_PALUT,
    /**< 2D - QueueId for PA statistics */
    /**< 3D - Not used */

    ALGLINK_SYNTHESIS_IPQID_MAXIPQ,
    /**< Maximum number of input queues */

    ALGLINK_SYNTHESIS_IPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}AlgorithmLink_SynthesisInputQueId;

/**
 *******************************************************************************
 * \brief Enum for the output Q IDs
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    ALGLINK_SYNTHESIS_OPQID_OPFRAME = 0x0,
    /**< 2D - QueueId for output frame (Stiched frame) */
    /**< 3D - Not used */

    ALGLINK_SYNTHESIS_OPQID_PASTATS,
    /**< QueueId for PA statistics */

    ALGLINK_SYNTHESIS_OPQID_SGXLUT,
    /**< 2D - Not used */
    /**< 3D - QueueId for SGX mesh LUT */

    ALGLINK_SYNTHESIS_OPQID_MAXOPQ,
    /**< Maximum number of output queues */

    ALGLINK_SYNTHESIS_OPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}AlgorithmLink_SynthesisOutputQueId;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Synthesis algorithm
 *
 *          This structure is a replica of create time parameters of synthesis
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params. This structure should be first element */
    UInt32                   maxOutputHeight;
    /**< Max height of the output (stiched) frame */
    UInt32                   maxOutputWidth;
    /**< max width of the output (stiched) frame */
    UInt32                   maxInputHeight;
    /**< Max height of the input (captured) frame */
    UInt32                   maxInputWidth;
    /**< Max width of the input (captured) frame */
    UInt32                   numViews;
    /**< number of input views from which output will be synthesized */
    UInt32                   numOutputFrames;
    /**< Number of output frames (Stiched frames)to be created
     *   for this link per channel */
    UInt32                   carBoxWidth;
    /**< Width of the car box > */
    UInt32                   carBoxHeight;
    /**< Height of the car box > */
    UInt32                   numPhotometricStatisticsTables;
    /**< Number of output Photometric statistics tables */
    UInt32                   numSgxBlendLUTables;
    /**< Number of output Blend LUT tables, this is used only in 3D */
    System_LinkOutQueParams  outQueParams[ALGLINK_SYNTHESIS_OPQID_MAXOPQ];
    /**< Output queue information. Refer AlgorithmLink_SynthesisOutputQueId
     *   for indexing order.
     */
    System_LinkInQueParams   inQueParams[ALGLINK_SYNTHESIS_IPQID_MAXIPQ];
    /**< Input queue information. Refer AlgorithmLink_SynthesisInputQueId
     *   for indexing order.
     */
    AlgorithmLink_SynthesisMode  synthesisMode;
    /**< Simple or Blending based */
    AlgorithmLink_SrvOutputModes  svOutputMode;
    /**< Surround view Alg can support either 2D or 3D ouput modes */
    UInt32                   enableCarOverlayInAlg;
    /**< Set to 1, if DSP need to create the car image, apply only for 2D SRV */

} AlgorithmLink_SynthesisCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Synthesis algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
    AlgorithmLink_SynthesisMode    synthesisMode;
    /**< Simple or Blending based */
} AlgorithmLink_SynthesisControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of synthesis algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_Synthesis_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
