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
 * \file utils_slave_boot.c
 *
 * \brief  This file has the implementataion for booting slave cores using
 *         SBL LIB
 *
 * \version 0.0 (Jun 2015) : [YM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <xdc/std.h>
#include <string.h>
#include "qspi.h"
#include "qspi_flash.h"
#include "hw_qspi.h"
#include "hw_types.h"
#include "soc_defines.h"
#include "platform.h"
#include "sbl_lib/sbl_lib.h"
#include "sbl_lib/sbl_lib_tda3xx.h"
#include "pmlib_sysconfig.h"
#include "pmhal_rm.h"

#define UTILS_BOOT_SLAVES_DEBUG 0

#include <src/utils_common/include/utils_qspi.h>
#include <src/utils_common/include/utils_dma.h>
#include <src/utils_common/include/utils_boot_slaves.h>
#include <src/links_common/system/system_priv_ipc.h>
#include <ti/sysbios/hal/Cache.h>

sbllibAppImageHeader_t         sblAppImageHeader;
Utils_DmaChObj                 gDumpSectionsDmaObj;

#define MIN_SIZE_DMA_TRANSFER 1024

static Int32 Utils_qspiReadSectorsEdma(Void       *dstAddr,
                                       const Void *srcOffsetAddr,
                                       UInt32      length)
{
    Int32 status = EDMA3_DRV_SOK;
    Utils_DmaCopy1D dmaPrm;

    if(length < MIN_SIZE_DMA_TRANSFER)
    {
        QSPI_ReadSectors(dstAddr, (const void *) srcOffsetAddr, length);
    }
    else
    {
        dmaPrm.destAddr = dstAddr;
        dmaPrm.srcAddr  = (Ptr)(SOC_QSPI_ADDRSP1_BASE + *((const UInt32 *)srcOffsetAddr));
        dmaPrm.length   = length;

        status = Utils_dmaCopy1D(&gDumpSectionsDmaObj, &dmaPrm);
        UTILS_assert(status == EDMA3_DRV_SOK);

        /* This function is expected to increment the srcOffsetAddr */
        *((UInt32 *) srcOffsetAddr) += length;
    }

    #if UTILS_BOOT_SLAVES_DEBUG
    Vps_printf(" UTILS: BOOT SLAVES: section dst [0x%x] src [0x%x] length [0x%x]",
                 dstAddr, (SOC_QSPI_ADDRSP1_BASE + *((const UInt32 *)srcOffsetAddr)), length);
    #endif

    return status;
}

static Int32 Utils_qspiReadSectorsMemCpy(Void       *dstAddr,
                                         const Void *srcOffsetAddr,
                                         UInt32      length)
{
    /* Read from QSPI */
    QSPI_ReadSectors(dstAddr, (const void *) srcOffsetAddr, length);

    #if UTILS_BOOT_SLAVES_DEBUG
    Vps_printf(" UTILS: BOOT SLAVES: section dst [0x%x] src [0x%x] length [0x%x]",
                 dstAddr, (SOC_QSPI_ADDRSP1_BASE + *((const UInt32 *)srcOffsetAddr)), length);
    #endif

    return 0;
}

static void Utils_printFxn(const char* msg)
{
    /* Read from QSPI */
    Vps_printf(msg);
    return;
}

Int32 Utils_bootSlaves(Utils_BootSlaves_Params *params)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 offset = params->offset;
    Utils_DmaChCreateParams dmaParams;
    sbllibEntryPoints_t sblLibEntryPoints;
    sbllibInitParams_t sblInitPrms;

    pmlibSysConfigPowerStateParams_t inputTableDsp1[] =
    {{PMHAL_PRCM_MOD_DSP1,                     PMLIB_SYS_CONFIG_ALWAYS_ENABLED}};
    pmlibSysConfigPowerStateParams_t inputTableDsp2[] =
    {{PMHAL_PRCM_MOD_DSP2,                     PMLIB_SYS_CONFIG_ALWAYS_ENABLED}};
    pmlibSysConfigPowerStateParams_t inputTableEve1[] =
    {{PMHAL_PRCM_MOD_EVE1,                     PMLIB_SYS_CONFIG_ALWAYS_ENABLED}};

    /* Deault initialization of SBL Lib Params */
    SBLLibInitParamsInit(&sblInitPrms);

    /* Deault initialization of dma Params */
    Utils_DmaChCreateParams_Init(&dmaParams);
    /* Create DMA channel for transfer */
    status = Utils_dmaCreateCh(
                    &gDumpSectionsDmaObj,
                    &dmaParams
                    );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    if(System_isProcEnabled(SYSTEM_PROC_DSP1))
    {
        PMHALResetAssert(PMHAL_PRCM_RG_DSP1_RST);
        PMHALResetAssert(PMHAL_PRCM_RG_DSP1_SYS_RST);
        status = PMLIBSysConfigSetPowerState(inputTableDsp1, (UInt32) 1,
                                             PM_TIMEOUT_INFINITE,
                                             NULL);
        #if UTILS_BOOT_SLAVES_DEBUG
        Vps_printf(" UTILS: BOOT SLAVES: Resetting DSP1\n");
        #endif
    }
    if(System_isProcEnabled(SYSTEM_PROC_DSP2))
    {
        PMHALResetAssert(PMHAL_PRCM_RG_DSP2_RST);
        PMHALResetAssert(PMHAL_PRCM_RG_DSP2_SYS_RST);
        status = PMLIBSysConfigSetPowerState(inputTableDsp2, (UInt32) 1,
                                             PM_TIMEOUT_INFINITE,
                                             NULL);
        #if UTILS_BOOT_SLAVES_DEBUG
        Vps_printf(" UTILS: BOOT SLAVES: Resetting DSP2\n");
        #endif

    }
    if(System_isProcEnabled(SYSTEM_PROC_EVE1))
    {
        PMHALResetAssert(PMHAL_PRCM_RG_EVE1_RST);
        PMHALResetAssert(PMHAL_PRCM_RG_EVE1_CPU_RST);
        status = PMLIBSysConfigSetPowerState(inputTableEve1, (UInt32) 1,
                                             PM_TIMEOUT_INFINITE,
                                             NULL);
        #if UTILS_BOOT_SLAVES_DEBUG
        Vps_printf(" UTILS: BOOT SLAVES: Resetting EVE1\n");
        #endif
    }

    QSPI_Initialize(DEVICE_TYPE_QSPI4);

    QSPISetMAddrSpace(SOC_QSPI_ADDRSP0_BASE,
                      QSPI__SPI_SWITCH_REG__MMPT_S__SEL_MM_PORT,
                      QSPI_CS0);

    #if UTILS_BOOT_SLAVES_DEBUG
    Vps_printf(" UTILS: BOOT SLAVES: QSPI initialization done\n");
    #endif


    /* Assign SBL Params */
    sblInitPrms.printFxn     = Utils_printFxn;
    sblInitPrms.appImgHeader = &sblAppImageHeader;
    SBLLibInit(&sblInitPrms);

    if(params->useEdma)
    {
        SBLLibRegisterImageCopyCallback(&Utils_qspiReadSectorsEdma, &QSPI_seek);
    }
    else
    {
        SBLLibRegisterImageCopyCallback(&Utils_qspiReadSectorsMemCpy, &QSPI_seek);
    }

    status = SBLLibMultiCoreImageParse((const void *) &offset,
                                      params->offset,
                                      &sblLibEntryPoints);
    #if UTILS_BOOT_SLAVES_DEBUG
    Vps_printf(" UTILS: BOOT SLAVES: AppImage parsing done\n");
    #endif

    if(status >= 0)
    {

        if(System_isProcEnabled(SYSTEM_PROC_DSP1))
        {
            SBLLibDSP1BringUp(sblLibEntryPoints.entryPoint[SBLLIB_CORE_ID_DSP1],
                              SBLLIB_SBL_BUILD_MODE_DEV);
            #if UTILS_BOOT_SLAVES_DEBUG
            Vps_printf(" UTILS: BOOT SLAVES: DSP1 bringup done \n");
            #endif
        }
        if(System_isProcEnabled(SYSTEM_PROC_DSP2))
        {
            SBLLibDSP2BringUp(sblLibEntryPoints.entryPoint[SBLLIB_CORE_ID_DSP2],
                              SBLLIB_SBL_BUILD_MODE_DEV);
            #if UTILS_BOOT_SLAVES_DEBUG
            Vps_printf(" UTILS: BOOT SLAVES: DSP2 bringup done \n");
            #endif
        }
        if(System_isProcEnabled(SYSTEM_PROC_EVE1))
        {
            SBLLibEVE1BringUp(sblLibEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE1],
                              SBLLIB_SBL_BUILD_MODE_DEV);
            #if UTILS_BOOT_SLAVES_DEBUG
            Vps_printf(" UTILS: BOOT SLAVES: EVE1 bringup done \n");
            #endif
        }
    }

    Utils_dmaDeleteCh(&gDumpSectionsDmaObj);


    return status;
}


Void Utils_syncSlaves()
{
    System_ipcStart();

    System_triggerAppInit();
    System_waitAppInitComplete();
    System_triggerAppInitComplete();
}


/* Nothing past this point */
