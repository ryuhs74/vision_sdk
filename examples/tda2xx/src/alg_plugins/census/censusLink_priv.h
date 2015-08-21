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
 * \file censusLink_priv.h Census Algorithm Link
 *       private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.1 (Sept 2014) : [SR] First version
 *
 *******************************************************************************
 */

#ifndef _CENSUS_LINK_PRIV_H_
#define _CENSUS_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_census.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <apps/census/algo/inc/iCensus_ti.h>
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
 *   \brief Census window size
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */

#define _CENSUS_WIN_SIZE(winWidth, winHeight, hStep, vStep) \
    ((((winWidth) + (hStep)-1)/(hStep)) * (((winHeight) + (vStep)-1)/(vStep)))

/**
 *******************************************************************************
 *
 *   \brief Threshold size beyond which memory gets allocated in Shared area
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define CENSUS_ALGLINK_SRMEM_THRESHOLD (ALGORITHMLINK_SRMEM_THRESHOLD)

/**
 *******************************************************************************
 *
 *   \brief Max number of channels per output queue
 *
 *   Census alg works on Stereo sensors - so 2 channels data the link can
 *   handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define CENSUS_MAX_CH_PER_OUT_QUE    (2)

/**
 *******************************************************************************
 *
 *   \brief Max number of buffers
 *
 *   Soft Isp alg takes an input frame and generates an output
 *   frame. The format of output is not known. It can be considered as a meta
 *   data buffer. This macro defines the maximum number of such buffers this
 *   link can handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define CENSUS_LINK_MAX_NUM_OUTPUT    (6)

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
    System_LinkOutQueParams             outQueParams;
    /**< Output queue information */
    System_LinkInQueParams              inQueParams;
    /**< Input queue information */
    AlgorithmLink_InputQueueInfo        inputQInfo;
    /**< All the information about input Queue*/
    AlgorithmLink_OutputQueueInfo       outputQInfo;
    /**< All the information about output Queue*/
    UInt32                              numInputChannels;
    /**< Number of input channels on input Q (Prev link output Q) */
    System_LinkChInfo                   inputChInfo[CENSUS_MAX_CH_PER_OUT_QUE];
    /**< channel info of input */

    Void *                           handle;
    /**< Handle to the algorithm */
    AlgorithmLink_CensusCreateParams    algLinkCreateParams;
    /**< Create params of soft isp algorithm link */
    CENSUS_TI_CreateParams              algCreateParams;
    /**< Create parameters for the algorithm */
    IVISION_InArgs                      inArgs;
    /**< inArgs for the algorithm */
    CENSUS_TI_outArgs                   outArgs;
    /**< outArgs for the algorithm */

    IVISION_InBufs                      inBufs;
    /**< input buffers for the algorithm */
    IVISION_OutBufs                     outBufs;
    /**< output buffers for the algorithm */
    IVISION_BufDesc                     inBufDesc;
    /**< input buffer descriptor */
    IVISION_BufDesc                     outBufDesc;
    /**< output buffer descriptor */
    IVISION_BufDesc                     *inBufDescList[CENSUS_BUFDESC_IN_TOTAL];
    /**< list of input buffer descriptors */
    IVISION_BufDesc                 *outBufDescList[CENSUS_BUFDESC_OUT_TOTAL];
    /**< list of input buffer descriptors */
    System_Buffer                   buffers[CENSUS_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */
    System_VideoFrameCompositeBuffer censusOpBuffers[CENSUS_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */
    UInt32                              output_pitch;
    /**< Pitch of output video frames. This is kept same for all channels*/
    UInt32                              outBufferSize;
    /**< Size of each output buffer */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool                                isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */
} AlgorithmLink_CensusObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_censusCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_censusProcess(void * pObj);
Int32 AlgorithmLink_censusControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_censusStop(void * pObj);
Int32 AlgorithmLink_censusDelete(void * pObj);
Int32 AlgorithmLink_censusPrintStatistics(void *pObj, AlgorithmLink_CensusObj *pCensusObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
