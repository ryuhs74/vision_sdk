/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup UTILS_API
 * \defgroup LINK_STATS_COLLECT    Link Statistics Collector,
 *                                 Keeps all links statistics information in
 *                                 shared memory
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file utils_link_stats_collector.h Link Statistics information collector
 *
 * \brief  Internal header file for Link statistics, included by each link
 *
 * \version 0.1 (Mar 2015) : First version
 *
 *******************************************************************************
 */

#ifndef _LINK_STATS_COLLECTOR_
#define _LINK_STATS_COLLECTOR_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include <src/utils_common/include/utils_link_stats_if.h>


#define UTILS_LINK_STATS_IPU1_0_INST_START   (0U)
#define UTILS_LINK_STATS_IPU1_0_INST_NUM     (30U)
#define UTILS_LINK_STATS_IPU1_0_INST_END     (UTILS_LINK_STATS_IPU1_0_INST_START +\
                                              UTILS_LINK_STATS_IPU1_0_INST_NUM)

#define UTILS_LINK_STATS_IPU1_1_INST_START   (UTILS_LINK_STATS_IPU1_0_INST_END)
#define UTILS_LINK_STATS_IPU1_1_INST_NUM     (10)
#define UTILS_LINK_STATS_IPU1_1_INST_END     (UTILS_LINK_STATS_IPU1_1_INST_START +\
                                              UTILS_LINK_STATS_IPU1_1_INST_NUM)

#define UTILS_LINK_STATS_A15_0_INST_START    (UTILS_LINK_STATS_IPU1_1_INST_END)
#define UTILS_LINK_STATS_A15_0_INST_NUM      (20)
#define UTILS_LINK_STATS_A15_0_INST_END      (UTILS_LINK_STATS_A15_0_INST_START +\
                                              UTILS_LINK_STATS_A15_0_INST_NUM)

#define UTILS_LINK_STATS_DSP1_INST_START     (UTILS_LINK_STATS_A15_0_INST_END)
#define UTILS_LINK_STATS_DSP1_INST_NUM       (15)
#define UTILS_LINK_STATS_DSP1_INST_END       (UTILS_LINK_STATS_DSP1_INST_START +\
                                              UTILS_LINK_STATS_DSP1_INST_NUM)

#define UTILS_LINK_STATS_DSP2_INST_START     (UTILS_LINK_STATS_DSP1_INST_END)
#define UTILS_LINK_STATS_DSP2_INST_NUM       (15)
#define UTILS_LINK_STATS_DSP2_INST_END       (UTILS_LINK_STATS_DSP2_INST_START +\
                                              UTILS_LINK_STATS_DSP2_INST_NUM)

#define UTILS_LINK_STATS_EVE1_INST_START     (UTILS_LINK_STATS_DSP2_INST_END)
#define UTILS_LINK_STATS_EVE1_INST_NUM       (10)
#define UTILS_LINK_STATS_EVE1_INST_END       (UTILS_LINK_STATS_EVE1_INST_START +\
                                              UTILS_LINK_STATS_EVE1_INST_NUM)

#define UTILS_LINK_STATS_EVE2_INST_START     (UTILS_LINK_STATS_EVE1_INST_END)
#define UTILS_LINK_STATS_EVE2_INST_NUM       (10)
#define UTILS_LINK_STATS_EVE2_INST_END       (UTILS_LINK_STATS_EVE2_INST_START +\
                                              UTILS_LINK_STATS_EVE2_INST_NUM)

#define UTILS_LINK_STATS_EVE3_INST_START     (UTILS_LINK_STATS_EVE2_INST_END)
#define UTILS_LINK_STATS_EVE3_INST_NUM       (10)
#define UTILS_LINK_STATS_EVE3_INST_END       (UTILS_LINK_STATS_EVE3_INST_START +\
                                              UTILS_LINK_STATS_EVE3_INST_NUM)

#define UTILS_LINK_STATS_EVE4_INST_START     (UTILS_LINK_STATS_EVE3_INST_END)
#define UTILS_LINK_STATS_EVE4_INST_NUM       (10)
#define UTILS_LINK_STATS_EVE4_INST_END       (UTILS_LINK_STATS_EVE4_INST_START +\
                                              UTILS_LINK_STATS_EVE4_INST_NUM)

/** \brief Guard macro */
#if (UTILS_LINK_STATS_EVE4_INST_END > LINK_STATS_MAX_STATS_INST)
    #error "Increase LINK_STATS_MAX_STATS_INST in file utils_link_stats_if.h"
#endif

#define UTILS_LINK_STATS_PRF_IPU1_0_INST_START   (0U)
#define UTILS_LINK_STATS_PRF_IPU1_0_INST_NUM     (96U)
#define UTILS_LINK_STATS_PRF_IPU1_0_INST_END     (UTILS_LINK_STATS_PRF_IPU1_0_INST_START +\
                                                  UTILS_LINK_STATS_PRF_IPU1_0_INST_NUM)

#define UTILS_LINK_STATS_PRF_IPU1_1_INST_START   (UTILS_LINK_STATS_PRF_IPU1_0_INST_END)
#define UTILS_LINK_STATS_PRF_IPU1_1_INST_NUM     (50U)
#define UTILS_LINK_STATS_PRF_IPU1_1_INST_END     (UTILS_LINK_STATS_PRF_IPU1_1_INST_START +\
                                                  UTILS_LINK_STATS_PRF_IPU1_1_INST_NUM)

#define UTILS_LINK_STATS_PRF_A15_0_INST_START    (UTILS_LINK_STATS_PRF_IPU1_1_INST_END)
#define UTILS_LINK_STATS_PRF_A15_0_INST_NUM      (50U)
#define UTILS_LINK_STATS_PRF_A15_0_INST_END      (UTILS_LINK_STATS_PRF_A15_0_INST_START +\
                                                  UTILS_LINK_STATS_PRF_A15_0_INST_NUM)

#define UTILS_LINK_STATS_PRF_DSP1_INST_START     (UTILS_LINK_STATS_PRF_A15_0_INST_END)
#define UTILS_LINK_STATS_PRF_DSP1_INST_NUM       (16U)
#define UTILS_LINK_STATS_PRF_DSP1_INST_END       (UTILS_LINK_STATS_PRF_DSP1_INST_START +\
                                                  UTILS_LINK_STATS_PRF_DSP1_INST_NUM)

#define UTILS_LINK_STATS_PRF_DSP2_INST_START     (UTILS_LINK_STATS_PRF_DSP1_INST_END)
#define UTILS_LINK_STATS_PRF_DSP2_INST_NUM       (16U)
#define UTILS_LINK_STATS_PRF_DSP2_INST_END       (UTILS_LINK_STATS_PRF_DSP2_INST_START +\
                                                  UTILS_LINK_STATS_PRF_DSP2_INST_NUM)

#define UTILS_LINK_STATS_PRF_EVE1_INST_START     (UTILS_LINK_STATS_PRF_DSP2_INST_END)
#define UTILS_LINK_STATS_PRF_EVE1_INST_NUM       (16U)
#define UTILS_LINK_STATS_PRF_EVE1_INST_END       (UTILS_LINK_STATS_PRF_EVE1_INST_START +\
                                                  UTILS_LINK_STATS_PRF_EVE1_INST_NUM)

#define UTILS_LINK_STATS_PRF_EVE2_INST_START     (UTILS_LINK_STATS_PRF_EVE1_INST_END)
#define UTILS_LINK_STATS_PRF_EVE2_INST_NUM       (16U)
#define UTILS_LINK_STATS_PRF_EVE2_INST_END       (UTILS_LINK_STATS_PRF_EVE2_INST_START +\
                                                  UTILS_LINK_STATS_PRF_EVE2_INST_NUM)

#define UTILS_LINK_STATS_PRF_EVE3_INST_START     (UTILS_LINK_STATS_PRF_EVE2_INST_END)
#define UTILS_LINK_STATS_PRF_EVE3_INST_NUM       (16U)
#define UTILS_LINK_STATS_PRF_EVE3_INST_END       (UTILS_LINK_STATS_PRF_EVE3_INST_START +\
                                                  UTILS_LINK_STATS_PRF_EVE3_INST_NUM)

#define UTILS_LINK_STATS_PRF_EVE4_INST_START     (UTILS_LINK_STATS_PRF_EVE3_INST_END)
#define UTILS_LINK_STATS_PRF_EVE4_INST_NUM       (16U)
#define UTILS_LINK_STATS_PRF_EVE4_INST_END       (UTILS_LINK_STATS_PRF_EVE4_INST_START +\
                                                  UTILS_LINK_STATS_PRF_EVE4_INST_NUM)

/** \brief Guard macro */
#if (UTILS_LINK_STATS_PRF_EVE4_INST_END > LINK_STATS_PRF_MAX_TSK)
    #error "Increase LINK_STATS_PRF_MAX_TSK in file utils_link_stats_if.h"
#endif


/**
 *******************************************************************************
 *
 *  \brief  Function to initialize link stats collector.
 *
 *          This api initializes Link stats collector, it resets the flags,
 *          counter for each core.
 *          This API must be called only from one core
 *
 *  \returns    0: Collector is initialized and ready to be used
 *              any other number: collector is not initialized and
 *                                return the error
 *******************************************************************************
 */
Int32 Utils_linkStatsCollectorInit();

/**
 *******************************************************************************
 *
 *  \brief  Function to Deinitialize collector.
 *          checks if all link stats objects are de-allocated or not,
 *          Asserts if this is not the case.
 *          This API should be called after deinit of all links
 *          This API must be called only from one core
 *
 *******************************************************************************
 */
Void Utils_linkStatsCollectorDeInit();

#endif /* _LINK_STATS_COLLECTOR_ */

/*@}*/
