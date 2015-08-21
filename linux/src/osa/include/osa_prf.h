/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

 /**
  ******************************************************************************
  *
  *  \ingroup OSA_API
  *  \defgroup OSA_PRF_API Profiling API
  *
  *   OSA_prfTsXxxx - APIs to measure and print elasped time
  *                     @ 64-bit precision
  *
  *   OSA_prfLoadXxxx - APIs to measure and print CPU load at task,
  *                       HWI, SWI, global level
  *  @{
  *
  ******************************************************************************
  */

#ifndef _OSA_PRF_H_
#define _OSA_PRF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */


#include <osa.h>
#include <osa_types.h>

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure containing latency information for a task.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt64 maxLatency;
    /**< Maximum latency for this link */
    UInt64 minLatency;
    /**< Minimum latency for this link */
    UInt64 accumulatedLatency;
    /**< Accumulated latency added for every frame */
    UInt64 count;
    /**< Number of times latency update is called */
} OSA_LatencyStats;

/**
 *******************************************************************************
 *
 * \brief Common statistics maintained across all links for a given channel
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 inBufRecvCount;
    /**< Number of input buffers received */

    UInt32 inBufDropCount;
    /**< Number of input buffers dropped by link auto-matically due to
     *   internal processing constraints
     */

    UInt32 inBufUserDropCount;
    /**< Number of input buffers dropped by link due to user specified
     *   parameter
     */

    UInt32 inBufProcessCount;
    /**< Number of input buffers actually processed by the link */

    UInt32 numOut;
    /**< Number of outputs from this channel */

    UInt32 outBufCount[SYSTEM_MAX_OUT_QUE];
    /**< Number of buffers output for this channel */

    UInt32 outBufDropCount[SYSTEM_MAX_OUT_QUE];
    /**< Number of output buffers dropped by link auto-matically due to
     *   internal processing constraints
     */

    UInt32 outBufUserDropCount[SYSTEM_MAX_OUT_QUE];
    /**< Number of output buffers dropped by link due to user specified
     *   parameter
     */

} OSA_LinkChStatistics;

/**
 *******************************************************************************
 *
 * \brief Common statistics maintained across all links
 *
 *        A link implementation is recommended to use maintain these statistics.
 *
 *        Some link may need to maintain additional link specific statistics.
 *        Such link specific statistics can be maintain inside the link itself.
 *
 *        Common Utility APIs to print the stats can be used by a link to print
 *        this information.
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 numCh;
    /**< number of channels in the link */

    OSA_LinkChStatistics chStats[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Channel specific statistics */

    UInt32 inBufErrorCount;
    /**< Invalid buffers received at input */

    UInt32 outBufErrorCount;
    /**< Invalid buffers received at output */

    UInt32 statsStartTime;
    /**< Time at which stats were started */

    UInt32 newDataCmdCount;
    /**< Number of times 'NEW_DATA' command was received */

    UInt32 releaseDataCmdCount;
    /**< Number of times 'RELEASE_DATA' command was received */

    UInt32 getFullBufCount;
    /**< Number of times 'getFullBuf' callback was called by next link */

    UInt32 putEmptyBufCount;
    /**< Number of times 'putEmptyBuf' callback was called by next link */

    UInt32 notifyEventCount;
    /**< Number of times 'notify' callback was called by a link
     *   Notify could mean display ISR for display link
     *   Capture ISR for capture link
     *   IPC Notify ISR for IPC link
     */

} OSA_LinkStatistics;


Void OSA_resetLatency(OSA_LatencyStats *lStats);
Void OSA_updateLatency(OSA_LatencyStats *lStats,
                         UInt64 linkLocalTime);
Void OSA_printLatency(char *name,
                        OSA_LatencyStats *localLinkstats,
                        OSA_LatencyStats *srcToLinkstats,
                        Bool resetStats
                        );

Void OSA_resetLinkStatistics(OSA_LinkStatistics *pPrm,
                                UInt32 numCh,
                                UInt32 numOut);
Void OSA_printLinkStatistics(OSA_LinkStatistics *pPrm, char *name,
                                Bool resetStats);
UInt32 OSA_calcFps(UInt32 count, UInt32 divValue);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
