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
 * \ingroup ALGORITHM_LINK_DMA_SW_MS_API
 * \defgroup ALGORITHM_LINK_DMA_SW_MS_IMPL Algorithm Link Plugin : DMA SW Mosaic
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file dmaSwMsLink_priv.h Algorithm Link Plugin : DMA SW Mosaic private
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

#ifndef _DMA_SWMS_LINK_PRIV_H_
#define _DMA_SWMS_LINK_PRIV_H_

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
#include <include/link_api/algorithmLink_dmaSwMs.h>
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
#define DMA_SWMS_LINK_MAX_OUT_BUF (8)

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

    Utils_DmaCopyFill2D dmaCopyPrms[ALGORITHM_LINK_DMA_SW_MS_MAX_WINDOWS];
    /**< DMA copy params */

    Utils_DmaCopyFill2D dmaFillPrms;
    /**< DMA fill params    */

    AlgorithmLink_DmaSwMsCreateParams   createArgs;
    /**< Create time arguments */

    System_LinkQueInfo          prevLinkQueInfo;
    /**< channel info of input queue */

    UInt32                      dataFormat;
    /**< Data format, YUV422 or YUV420 */

    UInt32                      outPitch[2];
    /**< Pitch of output buffer */

    System_Buffer buffers[DMA_SWMS_LINK_MAX_OUT_BUF];
    /**< System buffer data structure to exchange buffers between links */

    System_VideoFrameBuffer videoFrames[DMA_SWMS_LINK_MAX_OUT_BUF];
    /**< Payload for System buffers */

    UInt32 outBufSize;
    /**< Size of output buffer in bytes */

    Ptr *dmaFillLineAddr[UTILS_DMA_MAX_PLANES];
    /**< Address of fill data line */

    UInt32 dmaFillLineSize;
    /**< Size of fill data line in bytes */

    AlgorithmLink_DmaSwMsLayoutParams   curLayoutPrm;
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
} AlgorithmLink_DmaSwMsObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_dmaSwMsCreate(void * pObj, void * pCreateParams);
Int32 AlgorithmLink_dmaSwMsProcess(void * pObj);
Int32 AlgorithmLink_dmaSwMsControl(void * pObj, void * pControlParams);
Int32 AlgorithmLink_dmaSwMsStop(void * pObj);
Int32 AlgorithmLink_dmaSwMsDelete(void * pObj);

Int32 AlgorithmLink_dmaSwMsPrintStatistics(void * pObj,
                    AlgorithmLink_DmaSwMsObj *pDmaSwMsObj);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
