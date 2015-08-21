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
 * \file synthesisLink_priv.h Synthesis Algorithm Link private
 *                            API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _SYNTHESIS_LINK_PRIV_H_
#define _SYNTHESIS_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_synthesis.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_que.h>
#include "./include/iSynthesisAlgo.h"
#include "svAlgLink_priv.h"
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
 *   \brief Max number of video frames (Stiched output) / PA Stats needed for
 *          synthesis algorithm
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SYNTHESIS_LINK_MAX_NUM_OUTPUT (8)

/**
 *******************************************************************************
 *
 *   \brief Max number of elements for local queues
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SYNTHESIS_LINK_MAX_LOCALQUEUELENGTH (20)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Data Structure for the Que of system buffers.
 *          TBD: Can be made available to all alg links?
 *
 *   Que handle and the associated memory for queue elements are grouped.
 *
 *******************************************************************************
*/
typedef struct {
    Utils_QueHandle    queHandle;
    /**< Handle to the queue for this channel */
    System_Buffer      *queMem[SYNTHESIS_LINK_MAX_LOCALQUEUELENGTH];
    /**< Queue memory */
} SynthesisLink_SysBufferQue;

/**
 *******************************************************************************
 *
 *   \brief Structure containing synthesis algorithm link specific parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef  struct
{
    void                      * algHandle;
    /**< Handle of the algorithm */
    UInt32                      inPitch[SYSTEM_MAX_PLANES];
    /**< Pitch of the input video buffer, This is kept same for all channels */
    UInt32                      outPitch[SYSTEM_MAX_PLANES];
    /**< Pitch of output video frames */
    UInt32                      dataFormat;
    /**< Data format of the video to operate on */
    UInt32                      numInputChannels;
    /**< Number of input channels on input Q (Prev link output Q) */
    System_LinkChInfo           inputChInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< channel info of input */
    AlgorithmLink_SynthesisCreateParams algLinkCreateParams;
    /**< Create params of the synthesis algorithm Link*/
    SV_Synthesis_CreationParamsStruct   algCreateParams;
    /**< Create params of the synthesis algorithm */
    SV_Synthesis_ControlParams  controlParams;
    /**< Control params of the synthesis algorithm */
    System_Buffer buffers[ALGLINK_SYNTHESIS_OPQID_MAXOPQ]
                         [SYNTHESIS_LINK_MAX_NUM_OUTPUT];
    /**< System buffer data structure to exchange buffers between links */
    System_VideoFrameBuffer videoFrames[SYNTHESIS_LINK_MAX_NUM_OUTPUT];
    /**< Payload for System buffers */
    System_MetaDataBuffer   photoAlignStats[SYNTHESIS_LINK_MAX_NUM_OUTPUT];
    /**< Payload for System buffers */
    System_MetaDataBuffer   opSgxBlendLUT[SYNTHESIS_LINK_MAX_NUM_OUTPUT];
    /**< Payload for System buffers */
    AlgorithmLink_OutputQueueInfo outputQInfo[ALGLINK_SYNTHESIS_OPQID_MAXOPQ];
    /**< All the information about output Queues used */
    AlgorithmLink_InputQueueInfo  inputQInfo[ALGLINK_SYNTHESIS_IPQID_MAXIPQ];
    /**< All the information about input Queues used */
    SynthesisLink_SysBufferQue localInputQ[ALGLINK_SYNTHESIS_IPQID_MAXIPQ];
    /**< Local Qs to hold input */
    UInt32                        frameDropCounter;
    /**< Counter to keep track of number of frame drops */
    System_Buffer                *sysBufferGALUT;
    /**< Place holder for the GA LUT sysBuffer. Only one will be held
     * inside Synthesis link at any point in time.
     */
    System_Buffer                *sysBufferBlendLUT;
    /**< Place holder for the Blend LUT sysBuffer. Only one will be held
     * inside Synthesis link at any point in time.
     */
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
    Bool receivedGALUTFlag;
    /**< Flag to indicate if GA LUT has been received. At any point in time
     *   Synthesis link keeps only one copy of GA LUT. So no queue is used.
     *   Just a bool is used for tracking.
     */
    Bool isFirstOPGenerated;
    /**< Flag to indicate if first synthesized frame is generated. This flag
     *   can be used to control any special processing for first output
     *   frame.
     */
    UInt8 *pLinkStaticPALUT;
    /**< Ptr to Static array for first frame PA LUT input.
     */
    Bool isSGXBlendLUTOPGenerated;
    /**< Flag to indicate if first blend LUT is generated. This output
     *   need to be generated only once, same will be reused later
     */

} AlgorithmLink_SynthesisObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_synthesisCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_synthesisProcess(void * pObj);
Int32 AlgorithmLink_synthesisControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_synthesisStop(void * pObj);
Int32 AlgorithmLink_synthesisDelete(void * pObj);
Int32 AlgorithmLink_synthesisPrintStatistics(void *pObj,
                       AlgorithmLink_SynthesisObj *pSynthesisObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
