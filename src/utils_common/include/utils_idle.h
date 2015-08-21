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
 * \defgroup UTILS_IDLE_API CPU IDLE API
 *
 * \brief  APIs to execute CPU Idle instruction and put CPU into IDLE
 *
 * @{
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file utils_idle.h
 *
 * \brief CPU Idle
 *
 * \version 0.0 First version
 *
 *******************************************************************************
*/

#ifndef _UTILS_IDLE_H_
#define _UTILS_IDLE_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>


/*******************************************************************************
 * Defines
 *******************************************************************************
*/
#if defined (BUILD_ARP32)
/**
 *******************************************************************************
 *
 * \brief EVE Power ID for Auto clock gate
 *
 *******************************************************************************
*/
#define UTILS_IDLE_EVE_AUTOCG (0)

/**
 *******************************************************************************
 *
 * \brief EVE Power ID for ARP32 Idle
 *
 *******************************************************************************
*/
#define UTILS_IDLE_EVE_IDLE   (1)
#endif

/*******************************************************************************
 *  Functions
 *******************************************************************************
*/
Void Utils_idleFxn();

Void Utils_idlePrepare();

#if defined (BUILD_ARP32)
Void Utils_idleSetEveMode(UInt32 eveMode);
Void Utils_idleEnableEveDMA();
Void Utils_idleDisableEveDMA();
Void Utils_idleGetPreEveAutoCgTime();
Void Utils_idleGetPostEveAutoCgTime();
#endif

#endif

/* @} */
