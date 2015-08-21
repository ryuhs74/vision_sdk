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
 * \file softIspLink_priv.h Soft Isp Algorithm Link
 *       private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.1 (Sep 2014) : [SR] First version
 *
 *******************************************************************************
 */

#ifndef _SOFTISP_LINK_PRIV_H_
#define _SOFTISP_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_softIsp.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <apps/soft_isp/algo/inc/isoft_isp_ti.h>
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
#define SOFTISP_ALGLINK_SRMEM_THRESHOLD (ALGORITHMLINK_SRMEM_THRESHOLD)

/**
 *******************************************************************************
 *
 *   \brief Max number of buffers
 *
 *   Soft Isp alg takes an input frame and generates an output
 *   frame. This macro defines the maximum number of such buffers this
 *   link can handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SOFTISP_LINK_MAX_NUM_OUTPUT    (6)

/**
 *******************************************************************************
 *
 *   \brief Max number of channels per output queue
 *
 *   Soft Isp alg works on Stereo sensors - so 2 channels data the link can
 *   handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SOFTISP_MAX_CH_PER_OUT_QUE    (2)


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
    System_LinkChInfo                   inputChInfo[SOFTISP_MAX_CH_PER_OUT_QUE];
    /**< channel info of input */

    UInt8 *                             pGbceToneCurve_4c;
    UInt8 *                             pGbceToneCurve_1c;
    /**<Buffers containing the tone curve used for Global Brightness and
     * Contrast Enhancement (GBCE).
     */

    Void *                              handle;
    /**< Handle to the algorithm */
    AlgorithmLink_SoftIspCreateParams   algLinkCreateParams;
    /**< Create params of soft isp algorithm link */
    SOFT_ISP_TI_CreateParams            algCreateParams;
    /**< Create parameters for the algorithm */
    SOFT_ISP_TI_InArgs                  inArgs;
    /**< inArgs for the algorithm */
    SOFT_ISP_TI_OutArgs                 outArgs;
    /**< outArgs for the algorithm */
    IVISION_InBufs                      inBufs;
    /**< input buffers for the algorithm */
    IVISION_OutBufs                     outBufs;
    /**< output buffers for the algorithm */
    IVISION_BufDesc                     inBufDesc;
    /**< input buffer descriptor */
    IVISION_BufDesc                     outBufDesc;
    /**< output buffer descriptor */
    IVISION_BufDesc             *inBufDescList[SOFT_ISP_TI_BUFDESC_IN_TOTAL];
    /**< list of input buffer descriptors */
    IVISION_BufDesc             *outBufDescList[SOFT_ISP_TI_BUFDESC_OUT_TOTAL];
    /**< list of input buffer descriptors */
    System_Buffer
            buffers[SOFTISP_MAX_CH_PER_OUT_QUE][SOFTISP_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */
    System_VideoFrameBuffer
        videoFrames[SOFTISP_MAX_CH_PER_OUT_QUE][SOFTISP_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */
    UInt32                              output_pitch;
    /**< Pitch of output video frames. This is kept same for all channels*/
    UInt32                              output_height;
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
} AlgorithmLink_SoftIspObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_softIspCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_softIspProcess(void * pObj);
Int32 AlgorithmLink_softIspControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_softIspStop(void * pObj);
Int32 AlgorithmLink_softIspDelete(void * pObj);
Int32 AlgorithmLink_softIspPrintStatistics
                        (void *pObj, AlgorithmLink_SoftIspObj *pSoftIspObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
