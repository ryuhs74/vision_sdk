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
 * \ingroup UTILS_API
 * \defgroup UTILS_EVELOADER_API APIs to load EVE binary from M4 when Linux run's on A15
 *
 * \brief This module define APIs used to load EVEs when a15 is running linux
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_eveloader.h
 *
 * \brief APIs to load EVE binary from M4 when Linux run's on A15
 *
 * \version 0.0 (Aug 2014) : [YM] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_EVELOADER_H_
#define _UTILS_EVELOADER_H_
#include <src/utils_common/include/utils.h>
#include "soc_defines.h"

/**
 * \brief       configure_dpll function program the ADPLLM & ADPLLJM for
 *              vayu SoC
 *
 * \return      error status.If error has occured it returns a non zero value.
 *                  If no error has occured then return status will be zero.
 *
 **/
Int32 configure_dpll(void);

/**
 * \brief           configure_clock_domains function wake-up the clock domains &
 *                  check for functional clock are gated.
 *                   If not log error message & returns
 *
 * \return      error status.If error has occured it returns a non zero value.
 *                  If no error has occured then return status will be zero.
 *
 **/
Int32 configure_clock_domains(void);

/**
 * \brief       DDR3BootRprc function parse the multi-core app image
 *              stored in the DDR.
 *              It Parses the AppImage & copies the section into CPU
 *              internal memory & external memory.
 *              CPUs entry loctions are stored into entry point
 *              global pointers.
 *
 *
 * \return         error status.If error has occured it returns a non zero
 *                 value.
 *                 If no error has occured then return status will be
 *                 zero.
 */
Int32 DDR3BootRprc(void);

/**
 * \brief      EVE1_BringUp function assert reset(CPU & EVE SS), set
 *             the entry point & release the CPU
 *             from reset.
 *
 *
 * \param   EntryPoint  CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE1_BringUp(UInt32 EntryPoint);

/**
 * \brief      EVE2_BringUp function assert reset(CPU & EVE SS), set
 *             the entry point & release the CPU
 *             from reset.
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE2_BringUp(UInt32 EntryPoint);

/**
 * \brief      EVE3_BringUp function assert reset(CPU & EVE SS), set
 *             the entry point & release the CPU
 *             from reset.
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE3_BringUp(UInt32 EntryPoint);

/**
 * \brief      EVE4_BringUp function assert reset(CPU & EVE SS), set
 *             the entry point & release the CPU
 *             from reset.
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE4_BringUp(UInt32 EntryPoint);


/**
 * \brief          slavecore_enable() - This function do the slave cores
 *                 PRCM module enable & clockdomain
 *                 force wake-up. Brings the slave cores system out of
 *                 reset.
 *
 *
 * \return      None.
 *
 **/
void slavecore_prcm_enable();


/**
 *******************************************************************************
 *
 * \brief Boots Eves with AppImage
 *
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_eveBoot();


#endif

/* @} */

