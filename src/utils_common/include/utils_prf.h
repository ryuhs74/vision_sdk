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
  *  \ingroup UTILS_API
  *  \defgroup UTILS_PRF_API Profiling API
  *
  *   Utils_prfTsXxxx - APIs to measure and print elasped time
  *                     @ 64-bit precision
  *
  *   Utils_prfLoadXxxx - APIs to measure and print CPU load at task,
  *                       HWI, SWI, global level
  *  @{
  *
  ******************************************************************************
  */

#ifndef _UTILS_PRF_H_
#define _UTILS_PRF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Maximum number of Utils perf handles allowed
 *
 *******************************************************************************
*/
#define UTILS_PRF_MAX_HNDL     (64)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure containing load information.
 *
 *          This structure is used to store load of various components of BIOS
 *
 *******************************************************************************
*/
typedef struct
{
    int32_t cpuLoad;
    /**< cpu load */
    int32_t hwiLoad;
    /**< hardware interrupt load */
    int32_t swiLoad;
    /**< software interrupt load */
    int32_t tskLoad;
    /**< task load */
} Utils_PrfLoad;

/**
 *******************************************************************************
 *
 * \brief Typedef for the loadupdate function for the user
 *
 *******************************************************************************
 */
typedef Void(*Utils_loadUpdate) (Utils_PrfLoad *);

/**
 *******************************************************************************
 *
 *  \brief  Structure containing latency information for a task.
 *
 *******************************************************************************
 */
typedef struct
{
    uint32_t maxLatencyHi;
    /**< Upper 32 bits of Maximum latency for this link */
    uint32_t maxLatencyLo;
    /**< Lower 32 bits of Maximum latency for this link */
    uint32_t minLatencyHi;
    /**< Upper 32 bits of Minimum latency for this link */
    uint32_t minLatencyLo;
    /**< Lower 32 bits of Minimum latency for this link */
    uint32_t accumulatedLatencyHi;
    /**< Upper 32 bis of Accumulated latency added for every frame */
    uint32_t accumulatedLatencyLo;
    /**< Lower 32 bis of Accumulated latency added for every frame */
    uint32_t countHi;
    /**< Upper 32 bits of Number of times latency update is called */
    uint32_t countLo;
    /**< Lower 32 bits of Number of times latency update is called */
} Utils_LatencyStats;

/**
 *******************************************************************************
 *
 * \brief Common statistics maintained across all links for a given channel
 *
 *******************************************************************************
 */
typedef struct {

    uint32_t inBufRecvCount;
    /**< Number of input buffers received */

    uint32_t inBufDropCount;
    /**< Number of input buffers dropped by link auto-matically due to
     *   internal processing constraints
     */

    uint32_t inBufUserDropCount;
    /**< Number of input buffers dropped by link due to user specified
     *   parameter
     */

    uint32_t inBufProcessCount;
    /**< Number of input buffers actually processed by the link */

    uint32_t numOut;
    /**< Number of outputs from this channel */

    uint32_t outBufCount[SYSTEM_MAX_OUT_QUE];
    /**< Number of buffers output for this channel */

    uint32_t outBufDropCount[SYSTEM_MAX_OUT_QUE];
    /**< Number of output buffers dropped by link auto-matically due to
     *   internal processing constraints
     */

    uint32_t outBufUserDropCount[SYSTEM_MAX_OUT_QUE];
    /**< Number of output buffers dropped by link due to user specified
     *   parameter
     */

} Utils_LinkChStatistics;

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

    uint32_t numCh;
    /**< number of channels in the link */

    Utils_LinkChStatistics chStats[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Channel specific statistics */

    uint32_t inBufErrorCount;
    /**< Invalid buffers received at input */

    uint32_t outBufErrorCount;
    /**< Invalid buffers received at output */

    uint32_t statsStartTime;
    /**< Time at which stats were started */

    uint32_t newDataCmdCount;
    /**< Number of times 'NEW_DATA' command was received */

    uint32_t releaseDataCmdCount;
    /**< Number of times 'RELEASE_DATA' command was received */

    uint32_t getFullBufCount;
    /**< Number of times 'getFullBuf' callback was called by next link */

    uint32_t putEmptyBufCount;
    /**< Number of times 'putEmptyBuf' callback was called by next link */

    uint32_t notifyEventCount;
    /**< Number of times 'notify' callback was called by a link
     *   Notify could mean display ISR for display link
     *   Capture ISR for capture link
     *   IPC Notify ISR for IPC link
     */

} Utils_LinkStatistics;

typedef struct {

    struct TotalLoad{

        uint32_t integerValue;
        uint32_t fractionalValue;

    } totalLoadParams;

} Utils_SystemLoadStats;


int32_t Utils_prfInit();
int32_t Utils_prfDeInit();

uint64_t Utils_prfTsGet64();

Void   Utils_prfLoadUpdate();
int32_t  Utils_prfLoadRegister(BspOsal_TaskHandle pTsk, char *name);
int32_t  Utils_prfLoadUnRegister(BspOsal_TaskHandle pTsk);
int32_t  Utils_prfLoadPrintAll(Bool printTskLoad);
int32_t  Utils_prfGetLoad(Utils_SystemLoadStats * pPrm);
Void   Utils_prfLoadCalcStart();
Void   Utils_prfLoadCalcStop();
Void   Utils_prfLoadCalcReset();


Void Utils_resetLatency(Utils_LatencyStats *lStats);
Void Utils_updateLatency(Utils_LatencyStats *lStats,
                         uint64_t linkLocalTime);
Void Utils_printLatency(char *name,
                        Utils_LatencyStats *localLinkstats,
                        Utils_LatencyStats *srcToLinkstats,
                        Bool resetStats
                        );

Void Utils_resetLinkStatistics(Utils_LinkStatistics *pPrm,
                                uint32_t numCh,
                                uint32_t numOut);
Void Utils_printLinkStatistics(Utils_LinkStatistics *pPrm, char *name,
                                Bool resetStats);

uint32_t Utils_calcFps(uint32_t count, uint32_t divValue);

#if defined (BUILD_ARP32)
Void Utils_prfUpdateEveLoadPreAutoCg(UInt64 totalTime);
Void Utils_prfUpdateEveLoadPostAutoCg(UInt64 totalTimeIdle);
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
