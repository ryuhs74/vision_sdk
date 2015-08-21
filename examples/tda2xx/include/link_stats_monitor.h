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
 * \ingroup EXAMPLES_API
 * \defgroup EXAMPLES_LINK_STATS_MONITOR_API APIs for statistics monitoring
 *
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file video_sensor.h
 *
 * \brief APIs for controlling external sensors.
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _LINK_STATS_MONITOR_H_
#define _LINK_STATS_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */

/* None */

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *  \brief Command to print Core Performance load
 */
#define SYSTEM_LINK_CMD_PRINT_CORE_PRF_LOAD         (0xA000)


/**
 *******************************************************************************
 *
 * \brief Initializes the Link Stats Server.
 *        Creates a task to print link statistics at every 60 seconds
 *        Creates a timer to give a callback at every 60seconds
 *        Initializes other parameters
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
Int32 Chains_linkStatsMonitorInit();

/**
 *******************************************************************************
 *
 * \brief DeInitializes the Link Stats Server.
 *        Deletes the task and timer
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
Void Chains_linkStatsMonitorDeInit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
