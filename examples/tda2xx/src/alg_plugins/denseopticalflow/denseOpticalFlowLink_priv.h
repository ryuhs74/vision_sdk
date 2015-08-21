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
 * \file denseOpticalFlowLink_priv.h Dense optical flow Algorithm Link private
 *                               API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Nov 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _DENSEOPTICALFLOW_LINK_PRIV_H_
#define _DESEOPTICALFLOW_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <include/link_api/algorithmLink_denseOpticalFlow.h>
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_que.h>
#include "./include/ALG_lucasKanadePym.h"
#include <starterware/inc/edma_utils/edma_utils_context_size.h>
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
 *   \brief Max number of output motion tables
 *
 *   OF generates one table for one frame. This macro defines the number of
 *   output buffers to hold sunch tables.
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DENSE_OPTFLOW_LINK_MAX_NUM_OUTPUT      (8)

/**
 *******************************************************************************
 *
 *   \brief Max Height of the buffer that can be handled
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DENSE_OPTFLOW_LINK_MAX_HEIGHT         (1080)

/**
 *******************************************************************************
 *
 *   \brief Max Width of the buffer that can be handled
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DENSE_OPTFLOW_LINK_MAX_WIDTH          (1920)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing dense optical flow algorithm link parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef  struct
{
    AlgorithmLink_DenseOptFlowCreateParams algLinkCreateParams;
    /**< Create params of the dense optical flow algorithm Link*/

    Alg_LKParams algCreateParams;
    /**< Create params of the dense optical flow algorithm */

    AlgLink_MemRequests algMemRequests;
    /**< Algorithm memory requests */

    System_Buffer buffers[DENSE_OPTFLOW_LINK_MAX_NUM_OUTPUT];
    /**< System buffer data structure to exchange buffers between links */

    System_MetaDataBuffer   optFlowTbl[DENSE_OPTFLOW_LINK_MAX_NUM_OUTPUT];
    /**< Payload for System buffers */

    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Queues used */

    AlgorithmLink_InputQueueInfo  inputQInfo;
    /**< All the information about input Queues used */

    UInt32                        outBufSize;
    /**< Size of output buffer */

    UInt32                        outBufPitch;
    /**< Output pitch */

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
     *   to start stats counting */

    UInt32 frameCount;
    /**< This variable counts the number of frames received */

    System_Buffer *pPrevious;
    /**< Holds pointer to the previous system buffer */

    System_Buffer *pCurrent;
    /**< Holds pointer to the current system buffer */

    UInt32 *scratch;
    /**< Scratch buffer required for ALG */

    UInt32 scratchSize;
    /**< Size of the scratch buffer required for ALG */

} AlgorithmLink_DenseOptFlowObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_denseOptFlowCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_denseOptFlowProcess(void * pObj);
Int32 AlgorithmLink_denseOptFlowControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_denseOptFlowStop(void * pObj);
Int32 AlgorithmLink_denseOptFlowDelete(void * pObj);
Int32 AlgorithmLink_denseOptFlowPrintStatistics(void *pObj,
                    AlgorithmLink_DenseOptFlowObj *opticalFlowObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
