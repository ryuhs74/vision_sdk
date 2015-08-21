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
 * \ingroup ALGORITHM_LINK_API
 * \defgroup ALGORITHM_LINK_IMPL Algorithm Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file sparseOpticalFlowDrawLink_priv.h Private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Feb 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _SPARSEOPTICALFLOWDRAW_LINK_PRIV_H_
#define _SPARSEOPTICALFLOWDRAW_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_sparseOpticalFlowDraw.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_dma.h>
#include "examples/tda2xx/include/draw2d.h"
#include "sparseOpticalFlowLink_priv.h"
#include <src/utils_common/include/utils_link_stats_if.h>

/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Max number of buffers
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SPARSEOPTICALFLOWDRAW_LINK_MAX_NUM_OUTPUT    (8)


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing feature plane computation algorithm link
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_sparseOpticalFlowDrawCreateParams algLinkCreateParams;
    /**< Create params of algorithm */

    System_Buffer buffers[SPARSEOPTICALFLOWDRAW_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */

    System_VideoFrameBuffer videoBuffers[SPARSEOPTICALFLOWDRAW_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */

    AlgorithmLink_InputQueueInfo  inputQInfo;
    /**< All the information about input Queue*/

    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Queue*/

    Utils_DmaChObj  copyFramesDmaObj;
    /**< DMA object to use when copying input frame to output frame */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */

    Draw2D_Handle draw2DHndl;
    /**< Handle to the draw object */

} AlgorithmLink_SparseOpticalFlowDrawObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_sparseOpticalFlowDrawCreate(void * pObj,
                                              void * pCreateParams);
Int32 AlgorithmLink_sparseOpticalFlowDrawProcess(void * pObj);
Int32 AlgorithmLink_sparseOpticalFlowDrawControl(void * pObj,
                                               void * pControlParams);
Int32 AlgorithmLink_sparseOpticalFlowDrawStop(void * pObj);
Int32 AlgorithmLink_sparseOpticalFlowDrawDelete(void * pObj);
Int32 AlgorithmLink_sparseOpticalFlowDrawPrintStatistics(void *pObj,
                AlgorithmLink_SparseOpticalFlowDrawObj *psparseOpticalFlowDrawObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
