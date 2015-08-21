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
 * \file objectDetectionLink_priv.h Object Detection Algorithm Link
 *       private API/Data structures
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

#ifndef _OBJECTDETECTION_LINK_PRIV_H_
#define _OBJECTDETECTION_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_objectDetection.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <examples/tda2xx/src/alg_plugins/common/include/alg_ivision.h>
#include <src/utils_common/include/utils_prf.h>
#include "iobjdet_ti.h"
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
 *   Feature Plane Classification alg takes an input frame from feature plane
 *   computation link and generates an output buffer which is a meta data
 *   buffer. This macro defines the maximum number of such buffers this
 *   link can handle
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define OBJECTDETECTION_LINK_MAX_NUM_OUTPUT    (8)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing feature plane Classification algorithm link
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef struct
{
    Void *handle;
    /**< Handle to the algorithm */
    AlgorithmLink_ObjectDetectionCreateParams algLinkCreateParams;
    /**< Create params of feature plane Classification algorithm link */
    TI_OD_CreateParams algCreateParams;
    /**< Create parameters for the algorithm */
    TI_OD_InArgs inArgs;
    /**< inArgs for the algorithm */
    TI_OD_OutArgs   outArgs;
    /**< outArgs for the algorithm */
    IVISION_InBufs    inBufs;
    /**< input buffers for the algorithm */
    IVISION_OutBufs   outBufs;
    /**< output buffers for the algorithm */
    IVISION_BufDesc   inBufDesc;
    /**< input buffer descriptor */
    IVISION_BufDesc   outBufDesc;
    /**< output buffer descriptor */
    IVISION_BufDesc   *inBufDescList[TI_OD_IN_BUFDESC_TOTAL];
    /**< list of input buffer descriptors */
    IVISION_BufDesc   *outBufDescList[TI_OD_OUT_BUFDESC_TOTAL];
    /**< list of input buffer descriptors */
    System_Buffer buffers[OBJECTDETECTION_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */
    System_MetaDataBuffer rectangles[OBJECTDETECTION_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */
    UInt32  outBufferSize;
    /**< Size of each output buffer */
    AlgorithmLink_InputQueueInfo  inputQInfo;
    /**< All the information about input Queue*/
    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Queue*/
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
} AlgorithmLink_ObjectDetectionObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_objectDetectionCreate(void * pObj,
                                              void * pCreateParams);
Int32 AlgorithmLink_objectDetectionProcess(void * pObj);
Int32 AlgorithmLink_objectDetectionControl(void * pObj,
                                               void * pControlParams);
Int32 AlgorithmLink_objectDetectionStop(void * pObj);
Int32 AlgorithmLink_objectDetectionDelete(void * pObj);
Int32 AlgorithmLink_objectDetectionPrintStatistics(void *pObj,
                AlgorithmLink_ObjectDetectionObj *pObjectDetectionObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
