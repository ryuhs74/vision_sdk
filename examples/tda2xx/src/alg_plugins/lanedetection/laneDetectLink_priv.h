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
 * \file laneDetectLink_priv.h Lane Detect Link private API/Data
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

#ifndef _LANEDETECT_LINK_PRIV_H_
#define _LANEDETECT_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_laneDetect.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include "ild_ti.h"
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_dma.h>
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
#define LANEDETECT_LINK_LANE_POINTS_BUF_SIZE        (SystemUtils_align(sizeof(AlgorithmLink_LaneDetectOutput), 128))




/**
 *******************************************************************************
 *
 *   \brief Max number of buffers
 *
 *******************************************************************************
 */
#define LANEDETECT_LINK_MAX_NUM_OUTPUT    (8)


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */



/**
 *******************************************************************************
 *
 *   \brief Structure containing algorithm link
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_LaneDetectCreateParams algLinkCreateParams;

    Void               *algLdHandle;
    LD_TI_CreateParams  algLdCreateParams;

    LD_TI_InArgs        algLdInArgs;
    LD_TI_OutArgs       algLdOutArgs;

    IVISION_InBufs      algLdInBufs;
    IVISION_OutBufs     algLdOutBufs;

    IVISION_BufDesc     algLdInBufDescImage;
    IVISION_BufDesc    *algLdInBufDesc[1];

    IVISION_BufDesc     algLdOutBufDescLanePoints;
    IVISION_BufDesc    *algLdOutBufDesc[1];

    System_LinkChInfo inChInfo;

    System_Buffer buffers[LANEDETECT_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */

    System_MetaDataBuffer metaDataBuffers[LANEDETECT_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */

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

    UInt8 *tmpBuf;
    UInt32 tmpBufSize;

} AlgorithmLink_LaneDetectObj;

/**
 *******************************************************************************
 *
 *   \brief Structure containing algorithm output
 *
 *******************************************************************************
*/
typedef struct {

  UInt32 numLeftLanePoints ;
  UInt32 numRightLanePoints ;
  UInt32 laneCrossInfo;

  LD_TI_output  laneInfo[LD_TI_MAXLANEPOINTS];

} AlgorithmLink_LaneDetectOutput;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_laneDetectCreate(void * pObj,
                                              void * pCreateParams);
Int32 AlgorithmLink_laneDetectProcess(void * pObj);
Int32 AlgorithmLink_laneDetectControl(void * pObj,
                                               void * pControlParams);
Int32 AlgorithmLink_laneDetectStop(void * pObj);
Int32 AlgorithmLink_laneDetectDelete(void * pObj);
Int32 AlgorithmLink_laneDetectPrintStatistics(void *pObj,
                AlgorithmLink_LaneDetectObj *pLaneDetectObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
