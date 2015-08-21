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
 * \file colorToGrayLink_priv.h Color to gray Algorithm Link private
 *                            API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Sept 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _COLORTOGRAY_LINK_PRIV_H_
#define _COLORTOGRAY_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_colorToGray.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include "iColorToGrayAlgo.h"
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
 *   \brief Structure containing color to gray algorithm link specific 
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef  struct
{
    Alg_ColorToGray_Obj         * algHandle;
    /**< Handle of the algorithm */
    System_LinkOutQueParams     outQueParams;
    /**< Output queue information */
    System_LinkInQueParams      inQueParams;
    /**< Input queue information */
    System_LinkChInfo           inputChInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Input channel information */
    UInt32                      numInputChannels;
    /**< Number of input channels */    
    Alg_ColorToGrayCreateParams   createParams;
    /**< Create params of the color to gray algorithm */
    Alg_ColorToGrayControlParams  controlParams;
    /**< Control params of the color to gray algorithm */
    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Q */
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
} AlgorithmLink_ColorToGrayObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_ColorToGrayCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_ColorToGrayProcess(void * pObj);
Int32 AlgorithmLink_ColorToGrayControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_ColorToGrayStop(void * pObj);
Int32 AlgorithmLink_ColorToGrayDelete(void * pObj);
Int32 AlgorithmLink_ColorToGrayPrintStatistics(void *pObj,
                     AlgorithmLink_ColorToGrayObj *pColorToGrayObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
