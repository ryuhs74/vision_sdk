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
 * \file stereoPostProcessLink_priv.h Stereo Post Processing Algorithm Link
 *       private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.1 (Oct 2014) : [VT] First version
 *
 *******************************************************************************
 */

#ifndef _STEREO_POST_PROCESS_LINK_PRIV_H_
#define _STEREO_POST_PROCESS_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_stereoPostProcess.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_que.h>
#include "istereovision_ti.h"
#include <examples/tda2xx/src/alg_plugins/common/include/alg_ivision.h>
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
 *   \brief Threshold size beyond which memory gets allocated in Shared area
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define STEREO_POST_PROCESS_ALGLINK_SRMEM_THRESHOLD (ALGORITHMLINK_SRMEM_THRESHOLD)

/**
 *******************************************************************************
 *
 *   \brief Max number of channels per output queue
 *
 *   Post Process output goes to display as a single channel
 *   handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define STEREO_POST_PROCESS_ALGLINK_MAX_CH_PER_OUT_QUE    (1)

/**
 *******************************************************************************
 *
 *   \brief Max number of channels per input queue
 *
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define STEREO_POST_PROCESS_ALGLINK_MAX_CH_PER_IN_QUE    (3)

/**
 *******************************************************************************
 *
 *   \brief Max number of buffers
 *
 *   This macro defines the maximum number of such buffers this
 *   link can handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define STEREO_POST_PROCESS_ALGLINK_MAX_NUM_OUTPUT    (6)

/**
 *******************************************************************************
 *
 *   \brief Max number of elements for local queues
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define PPROC_LINK_MAX_LOCALQUEUELENGTH               (20)

#define PPROC_LINK_MAX_NUM_INPUT_CHANNELS             (2)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Data Structure for the Que of system buffers.
 *
 *   Que handle and the associated memory for queue elements are grouped.
 *
 *******************************************************************************
*/
typedef struct {
    Utils_QueHandle    queHandle;
    /**< Handle to the queue for this channel */
    System_Buffer      *queMem[PPROC_LINK_MAX_LOCALQUEUELENGTH];
    /**< Queue memory */
} PProc_SysBufferQue;

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
    Void *                     handle;
    /**< Handle to the algorithm */
    STEREOVISION_TI_CreateParams algCreateParams;
    /**< Create parameters for the algorithm */
    STEREOVISION_TI_InArgs inArgs;
    /**< inArgs for the algorithm */
    STEREOVISION_TI_OutArgs   outArgs;
    /**< outArgs for the algorithm */
    IVISION_InBufs    inBufs;
    /**< input buffers for the algorithm */
    IVISION_OutBufs   outBufs;
    /**< output buffers for the algorithm */
    IVISION_BufDesc   inBufDesc[STEREOVISION_TI_BUFDESC_IN_TOTAL];
    /**< input buffer descriptor */
    IVISION_BufDesc   outBufDesc[STEREOVISION_TI_BUFDESC_OUT_TOTAL];
    /**< output buffer descriptor */
    IVISION_BufDesc   *inBufDescList[STEREOVISION_TI_BUFDESC_IN_TOTAL];
    /**< list of input buffer descriptors */
    IVISION_BufDesc   *outBufDescList[STEREOVISION_TI_BUFDESC_OUT_TOTAL];
    /**< list of input buffer descriptors */
    UInt32  outBufferSize;
    /**< Size of each output buffer */

    System_LinkOutQueParams         outQueParams;
    /**< Output queue information */
    System_LinkInQueParams          inQueParams;
    /**< Input queue information */
    AlgorithmLink_InputQueueInfo    inputQInfo;
    /**< All the information about input Queue*/
    AlgorithmLink_OutputQueueInfo   outputQInfo;
    /**< All the information about output Queue*/
    UInt32                          numInputChannels;
    /**< Number of input channels on input Q (Prev link output Q) */
    System_LinkChInfo inputChInfo[STEREO_POST_PROCESS_ALGLINK_MAX_CH_PER_IN_QUE];
    /**< channel info of input */

    UInt32                          circIdx;
    /* Circular index  for temporal median filter */

    AlgorithmLink_StereoPostProcessCreateParams    algLinkCreateParams;
    /**< Create params of soft isp algorithm link */

    System_Buffer       buffers[STEREO_POST_PROCESS_ALGLINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */
    System_VideoFrameBuffer videoFrames[STEREO_POST_PROCESS_ALGLINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers System_MetaDataBuffer */
    UInt32                  imagePitch[SYSTEM_MAX_PLANES];
    /**< Image Pitch */
    UInt32                  imageHeight[SYSTEM_MAX_PLANES];
    /**< Image Height */
    UInt32                  temporalFilterNumFrames;
    /**< For temporal filter, threshold is in number of frames a pixel must have non zero value before being displayed */
    System_MetaDataBuffer   postProcOutput[4];
    /**< Payload for the system buffers System_MetaDataBuffer. We have 3 of them in order to implement temporal 3-taps median filter and an extra buffer to store the result of the median */

    PProc_SysBufferQue      localInputQ[PPROC_LINK_MAX_NUM_INPUT_CHANNELS];
    /**< Local Qs to hold input */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool                    isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */
} AlgorithmLink_StereoPostProcessObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_StereoPostProcessCreate
                                        (void * pObj, void * pCreateParams);
Int32 AlgorithmLink_StereoPostProcessProcess(void * pObj);
Int32 AlgorithmLink_StereoPostProcessControl
                                        (void * pObj, void * pControlParams);
Int32 AlgorithmLink_StereoPostProcessStop(void * pObj);
Int32 AlgorithmLink_StereoPostProcessDelete(void * pObj);
Int32 AlgorithmLink_StereoPostProcessPrintStatistics
        (void *pObj, AlgorithmLink_StereoPostProcessObj *pDisparityHamDistObj);
Int32 AlgorithmLink_StereoPostProcessUpdateParams(AlgorithmLink_StereoPostProcessObj *pObj, AlgorithmLink_StereoPostProcessControlParams *pStereoCtlParams);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
