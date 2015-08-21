/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_SUBFRAME_COPY_API Algorithm Plugin: Sub Frame Copy API
 *
 * \brief  This module has the interface for using subframe copy alg plugin
 *
 *         Subframe copy is a sample algorithm, which just copies subframe from
 *         input to output buffer. The input buffer is in OCMC region and
 *         allocated by M4 capture driver. The Plugin registers an interrupt
 *         on VIP for subframe and frame complete events.
 *         The Link has a single output queue and single channel support
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_subframeCopy.h
 *
 * \brief Algorithm Link API specific to subframe copy algorithm on EVE
 *
 * \version 0.0 (Jul 2014) : [VT] First version
 *
 *******************************************************************************
 */

#ifndef _ALGLINK_SUBFRAME_COPY_H_
#define _ALGLINK_SUBFRAME_COPY_H_

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/captureLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 *
 * \brief Max number of output queue
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SUBFRAME_COPY_LINK_MAX_NUM_OUT_QUE                                  (1)

/**
 *******************************************************************************
 *
 * \brief Max Channels per output queue
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SUBFRAME_COPY_LINK_MAX_CH_PER_OUT_QUE                               (1)


/**
 *******************************************************************************
 *
 * \brief Indicates number of output buffers to be set to default
 *         value by the Subframe Process link
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SUBFRAME_COPY_LINK_NUM_BUFS_PER_CH_DEFAULT                          (4)


/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */


/**
 *******************************************************************************
 * \brief Subframe Process link configuration parameters.
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams      baseClassCreate;

    UInt32                          numBufs;
    /**< Number of buffers to be allocated for the Subframe Process link. Min
     *   number of buffers required is 4 for Subframe Process link to capture
     *   without frame drops
     */

    UInt32                          inChannelId;
    /**<This is the channel ID in input queue which this alg plugin processes */

    System_LinkOutQueParams         outQueParams;
    /**< Output queue information */

    System_LinkInQueParams          inQueParams;
    /**< link input queue information */

} AlgorithmLink_SubframeCopyCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for subframe copy algorithm
 *
 *          Ideally subframe copy should not have any control cmds in running
 *          state, as this will make the link process subframe slower leading
 *          to corruption.
 *          This data dstructure is added as Base class implementation of alg
 *          plugin expects it.
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_SubframeCopyControlParams;

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
 *        register plugins of sub frame copy algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_SubframeCopy_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
