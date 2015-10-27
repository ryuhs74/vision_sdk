/*
 * TI816x Booting and Flashing Utilities
 *
 * This file provides low-level init functions.
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* --------------------------------------------------------------------------
 * AUTHOR      : Mansoor Ahamed <mansoor.ahamed@ti.com>
 * ---------------------------------------------------------------------------
 * */

/* Standard type include */

#include <stdint.h>
#include <examples/tda2xx/include/chains.h>
#include "../inc/device.h"

/* This module's header file */
#include "hw_types.h"
#include "../inc/util.h"

/************************************************************
 * Explicit External Declarations                            *
 ************************************************************/

/************************************************************
 * Local Macro Declarations                                  *
 ************************************************************/
#define MAX_DELAY   ((UInt32) 0x900)

/************************************************************
 * Local Typedef Declarations                                *
 ************************************************************/

/************************************************************
 * Local Function Declarations                               *
 ************************************************************/
void GPMC_Write(UInt32 offset, UInt32 val);
void GPMC_CSWrite(Int8 cs, UInt32 offset, UInt32 val);
UInt32 GPMC_Read(UInt32 offset);
UInt32 DEVICE_Delay(UInt32 delay);
void CM_Write(UInt32 module, UInt32 reg, UInt32 val);
void ti816x_nor_init(void);
void ti814x_nor_init(void);
Int32 evm_nor_init(void);

/************************************************************
 * Local Function Definitions                                *
 ************************************************************/
void GPMC_Write(UInt32 offset, UInt32 val)
{
    HW_WR_REG32(GPMC_BASE + offset, val);
}

void GPMC_CSWrite(Int8 cs, UInt32 offset, UInt32 val)
{
    UInt32 tmpAddr = GPMC_CONFIG_CS0_BASE +
                     (GPMC_CS_CONFIG_SIZE * (UInt32) cs) + offset;
    HW_WR_REG32(tmpAddr, val);
}

UInt32 GPMC_Read(UInt32 offset)
{
    UInt32 tmpVal;

    tmpVal = HW_RD_REG32(GPMC_BASE + offset);
    return (tmpVal);
}

/* make sure compiler doesnt optimize this function */
UInt32 DEVICE_Delay(UInt32 delay)
{
    UTIL_waitLoop(delay * 0x100U);
    return 0U;
}

void CM_Write(UInt32 module, UInt32 reg, UInt32 val)
{
    UInt32 tmpAddr = (CM_BASE + module + reg);
    HW_WR_REG32(tmpAddr, val);
}

/************************************************************
 * Local Variable Definitions                                *
 \***********************************************************/

/************************************************************
 * Global Function Definitions                               *
 ************************************************************/

/*********************************************************************
 *
 * GPMC_Init - Initialize GPMC based on CS and config values
 *
 *********************************************************************/
void GPMC_Init(const GPMC_Config_t *cfg, Int8 cs)
{
    UInt32 i;

#ifdef  ti814x
    /* enable gpmc */
    CM_Write(CM_ALWON_MOD_OFF, CM_ALWON_GPMC_CLKCTRL_OFF, MODULEMODE_SWCTRL);
    DEVICE_Delay(MAX_DELAY);
#endif

    /****** program global GPMC regs *****/
    GPMC_Write(GPMC_SYSCONFIG_OFF, cfg->SysConfig);
    GPMC_Write(GPMC_IRQENABLE_OFF, cfg->IrqEnable);
    GPMC_Write(GPMC_TIMEOUTCTRL_OFF, cfg->TimeOutControl);
    GPMC_Write(GPMC_CONFIG_OFF, cfg->Config);

    /****** program GPMC CS specific registers *****/
    /* disable cs */
    GPMC_CSWrite(cs, GPMC_CONFIG7_OFF, 0x00000000);
    DEVICE_Delay(MAX_DELAY);

    /* program new set of config values (1 to 7) */
    for (i = 0; i < GPMC_MAX_CS; i++) {
        GPMC_CSWrite(cs, (i * 4U), cfg->ChipSelectConfig[i]);
    }

    /* enable cs */
    GPMC_CSWrite(cs, GPMC_CONFIG7_OFF,
                 (cfg->ChipSelectConfig[6] | (UInt32) 0x40));
    DEVICE_Delay(MAX_DELAY);
}

/***********************************************************
 * End file                                                 *
 ***********************************************************/

/* --------------------------------------------------------------------------
 *  HISTORY
 *      v1.0 completion
 *          Mansoor Ahamed -  23-Apr-2010
 * -----------------------------------------------------------------------------
 * */
