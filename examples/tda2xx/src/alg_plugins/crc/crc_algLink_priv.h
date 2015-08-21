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
 * \file crc_algLink_priv.h CRC Algorithm Link private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (May 2015) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _CRC_ALGLINK_PRIV_H_
#define _CRC_ALGLINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_crc.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include "icrc_algo.h"
#include <src/utils_common/include/utils_link_stats_if.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing CRC algorithm link specific parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef  struct
{
    Alg_CrcDma_Obj algHandle;
    /**< Handle of the algorithm */
    System_LinkInQueParams inQueParams;
    /**< Input queue information */
    UInt32 numInputChannels;
    /**< Number of input channels on input Q (Prev link output Q) */
    System_LinkChInfo inputChInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< channel info of input */
    AlgorithmLink_CrcCreateParams algLinkCreateParams;
    /**< Create params of the CRC algorithm Link*/
    Alg_CrcCreateParams createParams;
    /**< Create params of the CRC algorithm */
    AlgorithmLink_CrcSig_Obj crcSig;
    /**< data structure to store the CRC signature */
    Alg_CrcControlParams controlParams;
    /**< Control params of the CRC algorithm */
    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Q */
    AlgorithmLink_InputQueueInfo inputQInfo;
    /**< All the information about input Q */
    System_LinkStatistics *linkStatsInfo;
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
} AlgorithmLink_CrcObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_crcCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_crcProcess(void * pObj);
Int32 AlgorithmLink_crcControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_crcStop(void * pObj);
Int32 AlgorithmLink_crcDelete(void * pObj);
Int32 AlgorithmLink_crcPrintStatistics(void *pObj, AlgorithmLink_CrcObj *pCrcObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
