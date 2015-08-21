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
 * \defgroup UTILS_TEMP_API APIs for junction temperature alerts
 *
 * \brief  APIs to execute CPU Junction temperature and alerts an interrupt on
 *         core
 *
 * @{
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file utils_temperature.h
 *
 * \brief Junction Temperatue
 *
 * \version 0.0 First version : [CM] First version
 *
 *******************************************************************************
*/

#ifndef _UTILS_TEMPERATURE_H_
#define _UTILS_TEMPERATURE_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include "pmhal_prcm.h"
#include "pm/pmhal/pmhal_bgap.h"
#include "pm/pmhal/hw_pmhal_data_names.h"

/* \brief Macro to indicate that a hot event has occurred
 *
 * \param pmhalPrcmVdId_t * [IN] Voltage Id which caused the thermal event
 *
 */
#define UTILS_TEMP_CMD_EVENT_HOT            (0xD001)

/* \brief Macro to indicate that a cold event has occurred
 *
 * \param pmhalPrcmVdId_t * [IN] Voltage Id which caused the thermal event
 *
 */
#define UTILS_TEMP_CMD_EVENT_COLD           (0xD002)

#define UTILS_TEMP_INVALID                  (0x0EADBEAF)
/* \brief Macro to indicate temperature is not valid */

#define UTILS_TEMP_LIMP_HOME_ACTIVE         (0x1U)
/* \brief Macro to indicate that Limp Home Mode is active */
#define UTILS_TEMP_LIMP_HOME_INACTIVE       (0x0U)
/* \brief Macro to indicate that Limp Home Mode is in-active */


/*******************************************************************************
 *  Functions
 *******************************************************************************
*/

Void Utils_tempConfigInit();

Void Utils_tempConfigDeInit();

Void Utils_tempChangeHotThreshold(Int32 Value);

Void Utils_tempChangeColdThreshold(Int32 Value);

Void Utils_tempChangeStepSize(UInt32 Value);

Int32 Utils_tempGetHotThreshold(UInt32 voltId);

Int32 Utils_tempGetColdThreshold(UInt32 voltId);

Int32 Utils_tempGetStepSize(UInt32 voltId);

Int32 Utils_tempGetCurrTemperature(UInt32 voltId);

Void Utils_tempReadAllHotThreshold(Void);

Void Utils_tempReadAllColdThreshold(Void);

Void Utils_tempUpdateLimpHomeState(UInt32 voltId, UInt32 state);

Void Utils_tempUpdateAllVoltLimpHomeState(UInt32 state);

UInt32 Utils_tempGetLimpHomeState(Void);

#endif

/* @} */
