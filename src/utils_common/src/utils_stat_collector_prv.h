/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup UTILS_API
 * \defgroup UTILS_STATCOLLECTOR_API Stat Collector related utilities
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_stat_collector_prv.h
 *
 * \brief  Stat Collector related utilities
 *
 * \version 0.0 (Jan 2014) : [FM] First version
 *
 *
 *******************************************************************************
 */

#ifndef UTILS_STAT_COLLECTOR_PRV_H
#define UTILS_STAT_COLLECTOR_PRV_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include <src/utils_common/include/utils_stat_collector.h>
#include <src/utils_common/src/stat_collector_utils/sci.h>
#ifdef TDA3XX_FAMILY_BUILD
#include <src/utils_common/src/stat_collector_utils/sci_tda3x.h>
#else
#include <src/utils_common/src/stat_collector_utils/sci_dra7xx.h>
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#define UTILS_STAT_COLL_TSK_STACK_SIZE    (8*1024)
#define UTILS_STAT_COLL_TSK_PRI           5

#define MAX_SDRAM_USECASES          16
#define MAX_SDRAM_CFGS              1

#define MAX_MSTR_USECASES           36
#define MAX_MSTR_CFGS               2

#define CNTR_DUMP_INTRVL            5
#define SAMPLING_WINDOW_WIDTH       (10*1000)
#define MAX_CNTRS_REQD              2

#define SDRAM                       0
#define MSTR                        1

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  system link object handle
 *          Contains LinkObject Handle and TaskId
 *
 *******************************************************************************
*/

typedef struct {

    char *descr;
    struct sci_config_sdram cfg;

} Utils_StatCollSdramUsecase;


typedef struct {

    char *descr;
    struct sci_config_mstr cfg;

}Utils_StatCollMstrUsecase;


typedef struct {

    UInt64 avg[MAX_CNTRS_REQD];
    /**< Average Stat Collector in Time Window */

    UInt64 peakRate[MAX_CNTRS_REQD];
    /**< Peak MB/s Stat Collector in Time Window */

    UInt64 peak[MAX_CNTRS_REQD];
    /**< Peak bytes Stat Collector in Time Window */

    UInt32 numCounters;
    /**< number of counter for stat collector */

} Utils_StatCollData;

typedef struct {

    UInt32                     stat_buf[MAX_MSTR_USECASES];
    psci_usecase_key           uc_key[MAX_MSTR_USECASES];
    Utils_StatCollData         data[MAX_MSTR_USECASES];
    UInt64                     total[MAX_MSTR_USECASES];
    UInt64                     count[MAX_MSTR_USECASES];
    Utils_StatCollMstrUsecase *cfg;
    UInt32                     num_uc;

} Utils_StatCollMstrCfg;

typedef struct {

    UInt32                      stat_buf[MAX_SDRAM_USECASES];
    psci_usecase_key            uc_key[MAX_SDRAM_USECASES];
    Utils_StatCollData          data[MAX_SDRAM_USECASES];
    UInt64                      total[MAX_SDRAM_USECASES];
    Utils_StatCollSdramUsecase *cfg;
    UInt32                      num_uc;

} Utils_StatCollSdramCfg;

typedef struct {

    Utils_StatCollMstrCfg    mstrCfg[MAX_MSTR_CFGS];
    Utils_StatCollSdramCfg   sdramCfg[MAX_SDRAM_CFGS];
    volatile Bool            doStop;
    volatile Bool            doReset;
    psci_handle              psci_hdl;
    UInt64                   startTimeAvg;
    UInt64                   startTimePeak;
    BspOsal_TaskHandle       tskHndl;

} Utils_StatCollCtrl;

#ifdef __cplusplus
}
#endif

#endif /* UTILS_STAT_COLLECTOR_H */

/* @} */
