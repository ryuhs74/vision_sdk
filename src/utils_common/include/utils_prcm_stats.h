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
 * \defgroup UTILS_PRCM_STATS_API PRCM Statistics APIs
 *
 * \brief  APIs to execute CPU PRCM Stats
 *
 * @{
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file utils_prcm_stats.h
 *
 * \brief Prcm Print Stats
 *
 * \version 0.0 First version : [CM] First version
 *
 *******************************************************************************
*/

#ifndef _UTILS_PRCM_STATS_H_
#define _UTILS_PRCM_STATS_H_


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*< This is to enable get Voltage i2c read */
#undef PMIC_I2C_ENABLE

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>

/*******************************************************************************
 *  Functions
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \brief Print the temperature value for Available Temperature
 *        Sensor for given Voltage Id.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintTempValues(UInt32 voltId);

/**
 *******************************************************************************
 *
 * \brief Print the Voltage value for Available Voltage rails corrosponding to
 *         given voltage id
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintVoltageValues(UInt32 voltId);

/**
 *******************************************************************************
 *
 * \brief Print all the Voltage value for available Voltage Rails.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllVoltageValues();

/**
 *******************************************************************************
 *
 * \brief Print all the temperature value for Available Temperature Sensor.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllVDTempValues();

/**
 *******************************************************************************
 *
 * \brief Print all PRCM Register data and the current state .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmDumpRegisterData();

/**
 *******************************************************************************
 *
 * \brief Print all Dpll Register data and the current state .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllDpllValues();

/**
 *******************************************************************************
 *
 * \brief Init PMLIB ClockRate . Initialize during system init
 *
 * \return None
 *
 *******************************************************************************
 */
Int32 Utils_prcmClockRateInit();

/**
 *******************************************************************************
 *
 * \brief Print all CPU Frequency .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllCPUFrequency();

/**
 *******************************************************************************
 *
 * \brief Print all the Peripheral Frequency .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllPeripheralsFrequency();

/**
 *******************************************************************************
 *
 * \brief Print all Module current state .
 *        Print the Module SIDLE State,
 *                Clock Activity State,
 *                  Power Domain State.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllModuleState();

#endif

/* @} */
