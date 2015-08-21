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
 * \file utils_stat_collector.h
 *
 * \brief  Stat Collector related utilities
 *
 * \version 0.0 (Jan 2014) : [CM] First version
 *
 *
 *******************************************************************************
 */

#ifndef UTILS_STAT_COLLECTOR_H
#define UTILS_STAT_COLLECTOR_H

#ifdef __cplusplus
extern "C"
{
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

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */
/**
 *******************************************************************************
 *
 * \brief   Stat Collector Init Function
 *
 *          This functions creates on saperate task which
 *          Stat Collector is initialized and started.
 *
 *******************************************************************************
*/

Void Utils_statCollectorInit();
/**
 *******************************************************************************
 *
 * \brief   Prints the stat collector values
 *
 *          Function itrates through a loop of number of statcol usecase and
 *          prints the averaged values. These are been updated once in
 *          SAMPLING_WINDOW_WIDTH duration.
 *
 *
 *******************************************************************************
*/
Void Utils_statCollectorPrintCount();

/**
 *******************************************************************************
 *
 * \brief   Stat Collector deInit Function
 *
 *          DeInit And terminate the thread
 *
 *******************************************************************************
*/

Void Utils_statCollectorDeInit();

/**
 *******************************************************************************
 *
 * \brief   Reset the stat collector values
 *
 *          Function itrates through a loop of number of statcol usecase and
 *          resets the values.
 *
 *
 *******************************************************************************
*/

Void Utils_statCollectorReset();


#ifdef __cplusplus
}
#endif

#endif /* UTILS_STAT_COLLECTOR_H */

/* @} */
