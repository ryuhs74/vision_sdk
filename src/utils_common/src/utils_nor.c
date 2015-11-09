/*
 * TI Booting and Flashing Utilities
 *
 * Main function for flashing the NOR device on the DM644x EVM.
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
 * AUTHOR      : Daniel Allred
 * ---------------------------------------------------------------------------
 * */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "soc_defines.h"
#include "platform.h"

#include "device.h"
//#include "norwriter.h"
#include "nor.h"
#ifdef TDA2XX_FAMILY_BUILD
#include "wd_timer.h"
#endif
//#include "util.h"
#include "debug.h"

/************************************************************
 * Explicit External Declarations                            *
 ************************************************************/

#define BUF_SIZE (32 * 1024)

/************************************************************
 * Local Macro Declarations                                  *
 ************************************************************/
/* NOR Base address */
#define NOR_BASE    ((UInt32) 0x08000000U)

#define READ_CHUNK      (16 * 1024)

#ifndef FILE_SEEK_SET
#define FILE_SEEK_SET    0   /* set file offset to offset */
#endif
#ifndef FILE_SEEK_CUR
#define FILE_SEEK_CUR    1   /* set file offset to current plus offset */
#endif
#ifndef FILE_SEEK_END
#define FILE_SEEK_END    2   /* set file offset to EOF plus offset */
#endif

/* GPMC CFG values for Spansion S29GL512P11TFI010 & S29GL512N11TFI010
 * This should work for most NOR, else we might have to move
 * these defines to evm.h
 * Values used here are for nominal speed, tweak it to improve performance
 */
#define SPNOR_GPMC_CONFIG1  0x00001010
#define SPNOR_GPMC_CONFIG2  0x00101080
#define SPNOR_GPMC_CONFIG3  0x00020201
#define SPNOR_GPMC_CONFIG4  0x0f031003
#define SPNOR_GPMC_CONFIG5  0x000f1111
#define SPNOR_GPMC_CONFIG6  0x0f030080
#define SPNOR_GPMC_CONFIG7  0x00000C00

#define GPMC_CS0 0

/************************************************************
 * Local Typedef Declarations                                *
 ************************************************************/

/************************************************************
 * Local Function Declarations                               *
 ************************************************************/

static int32_t local_DEBUGprintString(const char *s);

/************************************************************
 * Local Function Deefinitions                              *
 ************************************************************/
static int32_t local_DEBUGprintString(const char *s)
{
#ifndef BUILD_A15
    printf(s);
#else
    UARTPuts(s, -1);
#endif
    return (int32_t) SUCCESS;
}

/************************************************************
 * Local Variable Definitions                                *
 \***********************************************************/

UInt32        NORStartAddr = NOR_BASE;

/* Defines the ROM Code default parameters for NOR/superAND
 * Initializes as 16 bits multiplexed NOR interface without Wait monitoring
 * Timings are set to:
 *  TBD
 */
GPMC_Config_t GPMC_ConfigNorDefault = {
    /* SysConfig - ROM Code defaults */
    0x0,            /* 0x0008, */
    /* IRQEnable - ROM Code defaults */
    0x0000,
    /* TimeOutControl - ROM Code defaults */
    0xf01f0000U,
    /* Config - ROM Code defaults */
    0x000a0000,
    {
        0x41041010,
        0x001E1C01,
        0x00000000,
        0x0F071C03,
        0x041B1F1F,
        0x8F070000U,
        0x00000C08,
    },
    /* PrefetchConfig1 */
    0x0,
    /* PrefetchConfig2 */
    0x0,
    /* PrefetchConfig3 */
    0x0,
};

/************************************************************
 * Global Variable Definitions
 ************************************************************/

/************************************************************
 * Global Function Definitions                               *
 ************************************************************/

int System_norInit(void)
{
    Nor_InitPrms_t Nor_InitPrms;
    uint32_t       local_DEBUGprintStringFuncAddr =
        (uint32_t) (&local_DEBUGprintString);


    GPMC_Init(&GPMC_ConfigNorDefault, GPMC_CS0);

    PlatformGPMCSetPinMux();

    /* Initialize function pointer Default */
    NOR_InitParmsDefault(&Nor_InitPrms);

    Nor_InitPrms.norFlashInitPrintFxnPtr =
        (NOR_FlashInitPrintFxnPtr) local_DEBUGprintStringFuncAddr;
    /* Initialize function pointer Default */
    NOR_Init(&Nor_InitPrms);

    return 0;
}
/***********************************************************
 * End file                                                 *
 ***********************************************************/

/* --------------------------------------------------------------------------
 *  HISTORY
 *      v1.00  -  DJA  -  06-Nov-2007
 *          Completion
 * -----------------------------------------------------------------------------
 * */

