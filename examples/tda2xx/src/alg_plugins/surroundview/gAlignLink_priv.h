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
 * \file gAlignLink_priv.h Geometric Alignment Algorithm Link private
 *                            API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Oct 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _GALIGN_LINK_PRIV_H_
#define _GALIGN_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_geometricAlignment.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_que.h>
#include "./include/iGeometricAlignmentAlgo.h"
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
 *   \brief Max number of output LUT tables
 *
 *   GA generates three tables - Simple stitch LUT, Blend LUT1, Blend LUT2.
 *   All the three are considered together as one unit. This macro defines
 *   number of such units.
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GALIGN_LINK_MAX_NUM_OUTPUT (8)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing geometric alignment algorithm link parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef  struct
{
    void                      * algHandle;
    /**< Handle of the algorithm */
    UInt32                      dataFormat;
    /**< Data format of the video to operate on */
    UInt32                      inPitch[SYSTEM_MAX_PLANES];
    /**< Pitch of the input video buffer, support only YUV420 Data format */
    UInt32                      numInputChannels;
    /**< Number of input channels on input Q (Prev link output Q) */
    System_LinkChInfo           inputChInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< channel info of input */
    AlgorithmLink_GAlignCreateParams             algLinkCreateParams;
    /**< Create params of the geometric alignment algorithm Link*/
    SV_GAlign_CreationParamsStruct               algCreateParams;
    /**< Create params of the geometric alignment algorithm */
    SV_GAlign_ControlParams                      controlParams;
    /**< Control params of the geometric alignment algorithm */
    System_Buffer buffers[ALGLINK_GALIGN_OPQID_MAXOPQ]
                         [GALIGN_LINK_MAX_NUM_OUTPUT];
    /**< System buffer data structure to exchange buffers between links */
    System_MetaDataBuffer   gAlignLUT[ALGLINK_GALIGN_OPQID_MAXOPQ]
                                     [GALIGN_LINK_MAX_NUM_OUTPUT];
    /**< Payload for System buffers */
    AlgorithmLink_OutputQueueInfo outputQInfo[ALGLINK_GALIGN_OPQID_MAXOPQ];
    /**< All the information about output Queues used */
    AlgorithmLink_InputQueueInfo  inputQInfo[ALGLINK_GALIGN_IPQID_MAXIPQ];
    /**< All the information about input Queues used */
    UInt32                        frameDropCounter;
    /**< Counter to keep track of number of frame drops */
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
    Bool isFirstOPGenerated;
    /**< Flag to indicate if first output is generated. This flag
     *   can be used to control any special processing for first time
     */
} AlgorithmLink_GAlignObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_gAlignCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_gAlignProcess(void * pObj);
Int32 AlgorithmLink_gAlignControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_gAlignStop(void * pObj);
Int32 AlgorithmLink_gAlignDelete(void * pObj);
Int32 AlgorithmLink_gAlignPrintStatistics(void *pObj,
                       AlgorithmLink_GAlignObj *pGeometricAlignmentObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
