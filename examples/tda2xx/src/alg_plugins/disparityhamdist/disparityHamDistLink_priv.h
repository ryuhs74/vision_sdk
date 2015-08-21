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
 * \file disparityHamDistLink_priv.h Disparity Haming Distance Algorithm Link
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

#ifndef _DISPARITY_HAMDIST_LINK_PRIV_H_
#define _DISPARITY_HAMDIST_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_disparityHamDist.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <apps/disparity/algo/inc/iDisparity_ti.h>
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
#define DISPARITY_HAMDIST_ALGLINK_SRMEM_THRESHOLD (ALGORITHMLINK_SRMEM_THRESHOLD)

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
#define DISPARITY_HAMDIST_MAX_CH_PER_OUT_QUE    (2)

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
#define DISPARITY_HAMDIST_LINK_MAX_NUM_OUTPUT    (6)

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
    System_LinkChInfo       inputChInfo[DISPARITY_HAMDIST_MAX_CH_PER_OUT_QUE];
    /**< channel info of input */

    Void *               handle;
    Void *               rlHandle; /* to hold handle for the instance in charge of performing right to left execution */
    
    /**< Handle to the algorithm */
    AlgorithmLink_DisparityHamDistCreateParams    algLinkCreateParams;
    /**< Create params of soft isp algorithm link */
    DISPARITY_TI_CreateParams   algCreateParams;
    DISPARITY_TI_CreateParams   rlAlgCreateParams; /* for right to left execution */
    /**< Create parameters for the algorithm */
    IVISION_InArgs          inArgs;
    /**< inArgs for the algorithm */
    DISPARITY_TI_outArgs    outArgs;
    /**< outArgs for the algorithm */

    IVISION_InBufs          inBufs;
    IVISION_InBufs          rlInBufs;
    /**< input buffers for the algorithm */
    IVISION_OutBufs         outBufs;
    IVISION_OutBufs         rlOutBufs;
    /**< output buffers for the algorithm */
    IVISION_BufDesc     inBufDesc[DISPARITY_TI_BUFDESC_IN_TOTAL];
    IVISION_BufDesc     rlInBufDesc[DISPARITY_TI_BUFDESC_IN_TOTAL];
    /**< input buffer descriptor */
    IVISION_BufDesc     outBufDesc[DISPARITY_TI_BUFDESC_OUT_TOTAL];
    IVISION_BufDesc     rlOutBufDesc[DISPARITY_TI_BUFDESC_OUT_TOTAL];
    /**< output buffer descriptor */
    IVISION_BufDesc     *inBufDescList[DISPARITY_TI_BUFDESC_IN_TOTAL];
    IVISION_BufDesc     *rlInBufDescList[DISPARITY_TI_BUFDESC_IN_TOTAL];
    /**< list of input buffer descriptors */
    IVISION_BufDesc     *outBufDescList[DISPARITY_TI_BUFDESC_OUT_TOTAL];
    IVISION_BufDesc     *rlOutBufDescList[DISPARITY_TI_BUFDESC_OUT_TOTAL];
    /**< list of input buffer descriptors */
    System_Buffer       buffers[DISPARITY_HAMDIST_LINK_MAX_NUM_OUTPUT];
    System_Buffer       rlBuffers[DISPARITY_HAMDIST_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */
    System_MetaDataBuffer   disparityHamDistOpBuffers
                                [DISPARITY_HAMDIST_LINK_MAX_NUM_OUTPUT];
    System_MetaDataBuffer   rlDisparityHamDistOpBuffers
                                [DISPARITY_HAMDIST_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers System_MetaDataBuffer */
    UInt32                  outBufferSize;
    UInt32                  rlOutBufferSize;
    /**< Size of each output buffer */
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
} AlgorithmLink_DisparityHamDistObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_disparityHamDistCreate
                                        (void * pObj, void * pCreateParams);
Int32 AlgorithmLink_disparityHamDistProcess(void * pObj);
Int32 AlgorithmLink_disparityHamDistControl
                                        (void * pObj, void * pControlParams);
Int32 AlgorithmLink_disparityHamDistStop(void * pObj);
Int32 AlgorithmLink_disparityHamDistDelete(void * pObj);
Int32 AlgorithmLink_disparityHamDistPrintStatistics
        (void *pObj, AlgorithmLink_DisparityHamDistObj *pDisparityHamDistObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
