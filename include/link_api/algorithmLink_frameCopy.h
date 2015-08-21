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
 * \defgroup ALGORITHM_LINK_FRAMECOPY_API Algorithm Plugin: Frame Copy API
 *
 * \brief  This module has the interface for using frame copy algorithm
 *
 *         Frame copy is a sample algorithm, which just copies a frame from
 *         input to output buffer.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_frameCopy.h
 *
 * \brief Algorithm Link API specific to frame copy algorithm on DSP
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_FRAMECOPY_H_
#define _ALGORITHM_LINK_FRAMECOPY_H_

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
    ALGORITHM_LINK_ALG_CPUFRAMECOPY = 0,
    /**< Frame copy by CPU */
    ALGORITHM_LINK_ALG_DMAFRAMECOPY,
    /**< Frame copy by DMA */
    ALGORITHM_LINK_ALG_FRAMECOPY_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_CopyMode;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Frame copy algorithm
 *
 *          This structure is a replica of create time parameters of frame copy
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params */
    UInt32                    maxHeight;
    /**< Max height of the frame */
    UInt32                    maxWidth;
    /**< max width of the frame */
    UInt32                    numOutputFrames;
    /**< Number of output frames to be created for this link per channel*/
    System_LinkOutQueParams   outQueParams;
    /**< Output queue information */
    System_LinkInQueParams    inQueParams;
    /**< Input queue information */
} AlgorithmLink_FrameCopyCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Frame copy algorithm
 *
 *          This structure is a replica of control parameters of frame copy
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_frameCopyControlParams;

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
 *        register plugins of frame copy algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_FrameCopy_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
