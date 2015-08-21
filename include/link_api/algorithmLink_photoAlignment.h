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
 * \defgroup ALGORITHM_LINK_PHOTOALIGNMENT_API Algorithm Plugin: Photo\
 * Alignment API
 *
 * \brief  This module has the interface for using photo alignment algorithm
 *
 *         Photo Alignment algorithm link -
 *           1. Will take in statistics (Sub sampled image) from Synthesis
 *              stage and will generate Photo alignment tables.
 *              Photo alignment tables will be used to transform pixel values
 *              into new values, so as to compensate for photometric changes.
 *              Photo tables will get used during syntheis.
 *           2. Will generate two types of LUTs:
 *               - Simple / Default LUT which is just one is to one mapping
 *                 (Equal mapping)
 *               - Normal LUT which provides photometric alignment
 *           3. Will call the algorithm process function every frame
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_photoAlignment.h
 *
 * \brief Algorithm Link API specific to photo alignment algorithm
 *
 * \version 0.0 (Oct 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_PHOTOALIGNMENT_H_
#define _ALGORITHM_LINK_PHOTOALIGNMENT_H_

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
    ALGLINK_PALIGN_IPQID_PASTATS = 0x0,
    /**< QueueId for multiview images */

    ALGLINK_PALIGN_IPQID_MAXIPQ,
    /**< Maximum number of input queues */

    ALGLINK_PALIGN_IPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}AlgorithmLink_PAlignInputQueId;

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
    ALGLINK_PALIGN_OPQID_PALUT = 0x0,
    /**< QueueId for GA LUT output */
    /**< Connect to SYNT link incase of 2D */
    /**< Connect to SGX  link incase of 3D */

    ALGLINK_PALIGN_OPQID_MAXOPQ,
    /**< Maximum number of output queues */

    ALGLINK_PALIGN_OPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}AlgorithmLink_PAlignOutputQueId;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Photo Alignment
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
    UInt32                   numOutputTables;
    /**< Number of output GA LUT tables. All the 3 LUT tables are considered
     * as one unit. numOutputTables defines number of such units.
     */
    Int16 					 carBoxWidth;
    /**< Width of the car box > */
    Int16				     carBoxHeight;
    /**< Height of the car box > */
    UInt32                   dataFormat;
    /**< Data format */
    System_LinkOutQueParams  outQueParams[ALGLINK_PALIGN_OPQID_MAXOPQ];
    /**< Output queue information */
    System_LinkInQueParams   inQueParams[ALGLINK_PALIGN_IPQID_MAXIPQ];
    /**< Input queue information */
    AlgorithmLink_SrvOutputModes  svOutputMode;
    /**< Surround view Alg can support either 2D or 3D ouput modes */
} AlgorithmLink_PAlignCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Photo Alignment
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_PAlignControlParams;

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
 *        register plugins of photo alignment algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_pAlign_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
