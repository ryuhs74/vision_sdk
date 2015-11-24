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
 * \ingroup ALGORITHM_LINK_SURROUND_VIEW_API
 * \defgroup ALGORITHM_LINK_SURROUND_VIEW_IMPL Algorithm Link Plugin : Surround View
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file surroundViewLink_priv.h Algorithm Link Plugin : Surround View private
 *                            API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Sept 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _SURROUND_VIEW_LINK_PRIV_H_
#define _SURROUND_VIEW_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/system_common.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_surroundView.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_mem.h>
#include <src/utils_common/include/utils_dma.h>
#include <ti/sysbios/hal/Cache.h>
#include <src/utils_common/include/utils_link_stats_if.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Max number of video frames needed for DMA SW MS algorithm
 *
 *******************************************************************************
 */
#define SURROUND_VIEW_LINK_MAX_OUT_BUF (8)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing frame copy algorithm link specific parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef  struct
{
    Utils_DmaChObj   dmaChObj;
    /**< Handle to DMA channel */

    Utils_DmaChCreateParams dmaCreateArgs;
    /**< DMA create args */

    Utils_DmaCopyFill2D dmaCopyPrms[ALGORITHM_LINK_SURROUND_VIEW_MAX_WINDOWS];
    /**< DMA copy params */

    Utils_DmaCopyFill2D dmaFillPrms;
    /**< DMA fill params    */

    AlgorithmLink_SurroundViewCreateParams   createArgs;
    /**< Create time arguments */

    System_LinkQueInfo          prevLinkQueInfo;
    /**< channel info of input queue */

    UInt32                      dataFormat;
    /**< Data format, YUV422 or YUV420 */

    UInt32                      outPitch[2];
    /**< Pitch of output buffer */

    System_Buffer buffers[SURROUND_VIEW_LINK_MAX_OUT_BUF];
    /**< System buffer data structure to exchange buffers between links */

    System_VideoFrameBuffer videoFrames[SURROUND_VIEW_LINK_MAX_OUT_BUF];
    /**< Payload for System buffers */

    UInt32 outBufSize;
    /**< Size of output buffer in bytes */

    Ptr *dmaFillLineAddr[UTILS_DMA_MAX_PLANES];
    /**< Address of fill data line */

    UInt32 dmaFillLineSize;
    /**< Size of fill data line in bytes */

    AlgorithmLink_SurroundViewLayoutParams   curLayoutPrm;
    /**< Current layout parameters */

    Bool                         isLayoutSwitch;
    /**< Layout is switched, may need to fill output buffer */

    UInt32                       doFillBuf;
    /**< if > 0 then fill the output buffer with blank color */

    Utils_LatencyStats  linkLatency;
    /**< Structure to find out min, max and average latency of the link */

    Utils_LatencyStats  srcToLinkLatency;
    /**< Structure to find out min, max and average latency from
     *   source to this link
     */

    System_LinkStatistics      * linkStatsInfo;
    /* link specific statistics */

    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */
    UInt32* buf1;
    UInt32* buf2;
} AlgorithmLink_SurroundViewObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_surroundViewCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_surroundViewProcess(void * pObj);
Int32 AlgorithmLink_surroundViewControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_surroundViewStop(void * pObj);
Int32 AlgorithmLink_surroundViewDelete(void * pObj);

Int32 AlgorithmLink_surroundViewPrintStatistics(void * pObj,
                    AlgorithmLink_SurroundViewObj *pSurroundViewObj);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
