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
 * \file sparseOpticalFlowLink_priv.h Sparse Optical Flow Link private API/Data
 *       structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Apr 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _SPARSEOPTICALFLOW_LINK_PRIV_H_
#define _SPARSEOPTICALFLOW_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_sparseOpticalFlow.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <algorithms/pyramid_lk_sof/algo/inc/isof_ti.h>
#include <starterware/inc/edma_utils/edma_utils_memcpy.h>
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
#define SOF_ALGLINK_PAD_PIXELS                  (16)

#define SOF_ALGLINK_TRACK_POINTS_MAX            (600)
#define SOF_ALGLINK_FLOW_TRACK_POINTS_BUF_SIZE  (SOF_ALGLINK_TRACK_POINTS_MAX*sizeof(strackInfo))


/**
 *******************************************************************************
 *
 *   \brief Max number of buffers
 *
 *   SOF generates flow vectors for the objects in the frame. Flow vectors
 *   give the direction of motion of the object. The SOF algorithm generates
 *   flow vectors for a group of pixels.
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SPARSEOPTICALFLOW_LINK_MAX_NUM_OUTPUT    (8)

/**
 *******************************************************************************
 *
 *   \brief Max number of channels
 *
 *******************************************************************************
 */
#define SPARSEOPTICALFLOW_LINK_MAX_CH             (5)


/**
 *******************************************************************************
 *
 *   \brief Size of meta data buffer for SFM input data
 *
 *******************************************************************************
 */
#define SPARSEOPTICALFLOW_SFM_META_DATA_MAX_SIZE      (256)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

typedef struct
{
    Void               *algSofHandle;
    SOF_TI_CreateParams algSofCreateParams;

    SOF_TI_InArgs       algSofInArgs;
    SOF_TI_OutArgs      algSofOutArgs;

    IVISION_InBufs      algSofInBufs;
    IVISION_OutBufs     algSofOutBufs;

    IVISION_BufDesc     algSofInBufDescImage;
    IVISION_BufDesc     algSofInBufDescKeyPoints;
    IVISION_BufDesc    *algSofInBufDesc[2];

    IVISION_BufDesc     algSofOutBufDescKeyPoints;
    IVISION_BufDesc     algSofOutBufDescErrEst;
    IVISION_BufDesc     algSofOutBufDescTrackedPoints;
    IVISION_BufDesc    *algSofOutBufDesc[3];

    Void               *pAlgSofOutBufKeyPoints;
    Void               *pAlgSofOutBufErrEst;
    Void               *pAlgSofOutBufTrackedPoints;

    UInt32             algSofOutBufSizeKeyPoints;
    UInt32             algSofOutBufSizeErrEst;
    UInt32             algSofOutBufSizeTrackedPoints;

    System_Buffer buffers[SPARSEOPTICALFLOW_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */

    System_MetaDataBuffer metaDataBuffers[SPARSEOPTICALFLOW_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */

} AlgorithmLink_SparseOpticalFlowChObj;


/**
 *******************************************************************************
 *
 *   \brief Structure containing sparse optical flow algorithm link
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_SparseOpticalFlowCreateParams algLinkCreateParams;

    AlgorithmLink_SparseOpticalFlowChObj
                chObj[SPARSEOPTICALFLOW_LINK_MAX_CH];

    AlgorithmLink_InputQueueInfo  inputQInfo;
    /**< All the information about input Queue*/

    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Queue*/

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

} AlgorithmLink_SparseOpticalFlowObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_sparseOpticalFlowCreate(void * pObj,
                                              void * pCreateParams);
Int32 AlgorithmLink_sparseOpticalFlowProcess(void * pObj);
Int32 AlgorithmLink_sparseOpticalFlowControl(void * pObj,
                                               void * pControlParams);
Int32 AlgorithmLink_sparseOpticalFlowStop(void * pObj);
Int32 AlgorithmLink_sparseOpticalFlowDelete(void * pObj);
Int32 AlgorithmLink_sparseOpticalFlowPrintStatistics(void *pObj,
                AlgorithmLink_SparseOpticalFlowObj *pSparseOpticalFlowObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
