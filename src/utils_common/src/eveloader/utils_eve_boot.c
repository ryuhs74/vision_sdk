/* ======================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ==================================================================== */
/**
 *   Component:         SBL
 *
 *   Filename:          sbl_tda2xx_slavecore_boot.c
 *
 *   Description:       This file contain functions to wake-up clock domain,
 *          module enable & booting up the slave cores
 */

/****************************************************************
 *  INCLUDE FILES
 ****************************************************************/

#include <stdint.h>
#include <src/utils_common/include/utils_eveloader.h>
#include "hw_types.h"
#include "soc.h"
#include "pmhal_prcm.h"
#include "pm/pmhal/pmhal_mm.h"
#include "pm/pmhal/pmhal_rm.h"
#include "pm/pmhal/pmhal_pdm.h"
#include "pm/pmhal/pmhal_cm.h"
#include "hw_ipu_prm.h"
#include "hw_ipu_cm_core_aon.h"
#include "hw_core_cm_core.h"
#include "hw_core_prm.h"
#include "hw_dsp1_prm.h"
#include "hw_dsp1_cm_core_aon.h"
#include "hw_dsp2_prm.h"
#include "hw_dsp2_cm_core_aon.h"
#include "hw_eve1_prm.h"
#include "hw_eve1_cm_core_aon.h"
#include "hw_eve2_prm.h"
#include "hw_eve2_cm_core_aon.h"
#include "hw_eve4_prm.h"
#include "hw_iva_prm.h"
#include "hw_iva_cm_core.h"
#include "hw_ctrl_core.h"
#include "soc.h"
#include "utils_tda2xx_platform.h"
#include "mmu.h"
#include "uartConsole.h"

/* ============================================================================
 * GLOBAL VARIABLES DECLARATIONS
 * =============================================================================
 */

#ifdef SBL_DEV_BUILD
const UInt32 gSblDevBuildFlag = 1;
#else
const UInt32 gSblDevBuildFlag = 0;
#endif

/* ============================================================================
 * LOCAL VARIABLES DECLARATIONS
 * =============================================================================
 */
#define DSP1BOOTADDR        (SOC_CTRL_MODULE_CORE_BASE + 0x55C)
#define DSP1BOOTADDRVALUE       (0x00800000U)

#define DSP2BOOTADDR        (SOC_CTRL_MODULE_CORE_BASE + 0x560)
#define DSP2BOOTADDRVALUE       (0x00800000U)

#define ICONT1_ITCM              (SOC_IVA_CONFIG_BASE + 0x08000)
#define ICONT2_ITCM              (SOC_IVA_CONFIG_BASE + 0x18000)

/* ============================================================================
 * LOCAL FUNCTIONS PROTOTYPES
 * =============================================================================
 */

/* ============================================================================
 * FUNCTIONS
 * =============================================================================
 */

/**
 * \brief               DSP_SystemReset
 *
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Int32 CPU_SystemReset(cpu_core_id_t cpu)
{
    pmhalPrcmResetGroupId_t SYSTEM_RESET, LOCAL_RESET;
    Int32  retVal  = 0;
    UInt32  retStat = 0;

    switch (cpu)
    {

        case EVE1_ID:
            SYSTEM_RESET = PMHAL_PRCM_RG_EVE1_RST;
            LOCAL_RESET  = PMHAL_PRCM_RG_EVE1_CPU_RST;
            break;

        case EVE2_ID:
            SYSTEM_RESET = PMHAL_PRCM_RG_EVE2_RST;
            LOCAL_RESET  = PMHAL_PRCM_RG_EVE2_CPU_RST;
            break;

        case EVE3_ID:
            SYSTEM_RESET = PMHAL_PRCM_RG_EVE3_RST;
            LOCAL_RESET  = PMHAL_PRCM_RG_EVE3_CPU_RST;
            break;

        case EVE4_ID:
            SYSTEM_RESET = PMHAL_PRCM_RG_EVE4_RST;
            LOCAL_RESET  = PMHAL_PRCM_RG_EVE4_CPU_RST;
            break;
        default:
            Vps_printf("UTILS : EVELOADER : CPU SystemReset - CPU ID error \n");
            return -1;
    }

    /*Assert the Reset for given CPU ID*/
    retVal = PMHALResetAssert(SYSTEM_RESET);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : SYSTEM_RESET Assert Failed \n");
    }
    retVal = PMHALResetAssert(LOCAL_RESET);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : LOCAL_RESET Assert Failed \n");
    }

    /*Check the Reset status & clear*/
    retVal = PMHALResetGetStatus(LOCAL_RESET, &retStat);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : LOCAL_RESET get status Failed \n");
    }
    if (0x1 == retStat)
    {
        retVal = PMHALResetClearStatus(LOCAL_RESET);
        if (0U != retVal)
        {
            Vps_printf("UTILS : EVELOADER : LOCAL_RESET clear status Failed \n");
        }
        retStat = 0;
    }
    retVal = PMHALResetGetStatus(SYSTEM_RESET, &retStat);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : SYSTEM_RESET get status Failed \n");
    }
    if (0x1 == retStat)
    {
        retVal = PMHALResetClearStatus(SYSTEM_RESET);
        if (0U != retVal)
        {
            Vps_printf("UTILS : EVELOADER : SYSTEM_RESET clear status Failed \n");
        }
        retStat = 0;
    }

    /*Clear reset for MMU, Cache & Slave interface OCP port*/
    retVal = PMHALResetRelease(SYSTEM_RESET, PM_TIMEOUT_INFINITE);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : SYSTEM_RESET Release Failed \n");
    }

    return 0;
}


/**
 * \brief    EVE1_Clk_Enable is PRCM function for EVE1. Configure the
 *            module to auto & force-wake-up the clock domain
 *
 *
 * \param     None.
 *
 * \return   error status.If error has occured it returns a non zero value.
 *                      If no error has occured then return status will be zero.
 *
 **/
Int32 EVE1_Clk_Enable(Void)
{
    Int32 retval = 0;

    // Enable the module - eve1
    retval = PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE1,
                                PMHAL_PRCM_MODULE_MODE_AUTO,
                                PM_TIMEOUT_INFINITE);

    if (retval != 0)
    {
        Vps_printf("UTILS : EVELOADER : Enable EVE1 Module - Error \n");
        return retval;
    }

    // Force Wake-up clock domain dsp1
    retval = PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE1,
                                   PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
                                   PM_TIMEOUT_INFINITE);

    Vps_printf("UTILS : EVELOADER : Enable EVE1 CLK Enable - completed \n");

    return retval;
}

/**
 * \brief        EVE_SYSMMU_Config function configure the EVE MMU0. Add
 *               entires to access entry location page, L4Per1,L4Per2 & L4Per3,
 *               & Internal memory buffers. Remaining DDR location should
 *               be taken care in the application layer.
 *
 * \param[in]           eve_mmu_base_address - Base address of EVEx MMU0.
 * \param[in]           eve_entryPoint - EVE start-up entry location
 *
 *
 * \return      None.
 **/

Void EVE_SYSMMU_Config(UInt32 eve_mmu_base_address,
                       UInt32 eve_entryPoint)
{
    MMU_TlbEntry_t mmu_entry;
    UInt32 entrypoint_page;


    /* Derive Entry point page for the given entrypoint*/
    entrypoint_page = eve_entryPoint & (0xFF000000);

    /* Configure MMU Tlb Entry for Vector table  as 1MB section

     * EVE starts executing from virtual address 0x0 when brought out of reset.
     * When SBL maps the 0x0 to entryPointPage, it assumes that the vector
     * table is present at entryPointPage.
     * E.g. If entry point is 0x81F62648, entryPointPage is calculated as
     * 0x81000000 and application image should be built in a way that vector
     * table is present at this address.
     */

    mmu_entry.phyAddr        = entrypoint_page;
    mmu_entry.virtAddr       = 0x00000000;
    mmu_entry.valid          = TRUE;
    mmu_entry.pageSize       = MMU_Section_Size;
    mmu_entry.endianness     = MMU_Little_Endian;
    mmu_entry.elementSize    = MMU_NoTranslation_ElementSize;
    mmu_entry.tlbElementSize = MMU_CPU_ElementSize;
    mmu_entry.preserve       = TRUE;
    MMUTlbEntrySet(eve_mmu_base_address, 1, &mmu_entry);

    /* Configure MMU Tlb Entry for code & data section  as 16MB
     *supersection*/
    mmu_entry.phyAddr        = entrypoint_page;
    mmu_entry.virtAddr       = entrypoint_page;
    mmu_entry.valid          = TRUE;
    mmu_entry.pageSize       = MMU_SuperSection_Size;
    mmu_entry.endianness     = MMU_Little_Endian;
    mmu_entry.elementSize    = MMU_NoTranslation_ElementSize;
    mmu_entry.tlbElementSize = MMU_CPU_ElementSize;
    mmu_entry.preserve       = TRUE;
    MMUTlbEntrySet(eve_mmu_base_address, 2, &mmu_entry);

    /* Configure MMU Tlb Entry for EVE internal memory  as 16MB
     *supersection*/
    mmu_entry.phyAddr        = SOC_EVE_DMEM_BASE;
    mmu_entry.virtAddr       = SOC_EVE_DMEM_BASE;
    mmu_entry.valid          = TRUE;
    mmu_entry.pageSize       = MMU_SuperSection_Size;
    mmu_entry.endianness     = MMU_Little_Endian;
    mmu_entry.elementSize    = MMU_NoTranslation_ElementSize;
    mmu_entry.tlbElementSize = MMU_CPU_ElementSize;
    mmu_entry.preserve       = TRUE;
    MMUTlbEntrySet(eve_mmu_base_address, 3, &mmu_entry);

    /* Configure MMU Tlb Entry for L4_Per1,L4_Per2,L4_Per3  as 16MB
     *supersection*/
    mmu_entry.phyAddr        = SOC_L4_PER_AP_BASE;
    mmu_entry.virtAddr       = SOC_L4_PER_AP_BASE;
    mmu_entry.valid          = TRUE;
    mmu_entry.pageSize       = MMU_SuperSection_Size;
    mmu_entry.endianness     = MMU_Little_Endian;
    mmu_entry.elementSize    = MMU_NoTranslation_ElementSize;
    mmu_entry.tlbElementSize = MMU_CPU_ElementSize;
    mmu_entry.preserve       = TRUE;
    MMUTlbEntrySet(eve_mmu_base_address, 4, &mmu_entry);

    /* Enable MMU */
    MMUEnable(eve_mmu_base_address);

    Vps_printf("UTILS : EVELOADER : EVE MMU configuration completed \n");
    Vps_printf("UTILS : EVELOADER : EVE eve_entryPoint  = [0x%x] \n", eve_entryPoint);
    Vps_printf("UTILS : EVELOADER : EVE entrypoint_page = [0x%x] \n", entrypoint_page);
    Vps_printf("\n");

}

/**
 * \brief      EVE1_BringUp function assert reset(CPU & EVE SS), set
 *             the entry point & release the CPU
 *             from reset.
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE1_BringUp(UInt32 EntryPoint)
{
    UInt32 retVal = 0;
    pmhalPrcmModuleSIdleState_t sIdleState;

    //SBL_PRINTF(TRACE_LOW, __func__);
    Vps_printf("UTILS : EVELOADER :Set Entry Point\n");

    if (EntryPoint != 0)
    {
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE1_MMU0_BASE, EntryPoint);
    }
    else
    {
        /*Assuming 0x80000000 not in use*/
        HW_WR_REG32(0x80000000, 0x80000100); /* Reset vector points to
                                         *0x8000_0100*/
        HW_WR_REG32(0x80000100, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000104, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000108, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x8000010C, 0x0000037F); /* IDLE; opcode for ARP32 */
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE1_MMU0_BASE, 0x80000000);
    }

    Vps_printf("UTILS : EVELOADER :Bringout of reset \n");

    /* Bring-out of Reset*/
    retVal = PMHALResetRelease(PMHAL_PRCM_RG_EVE1_CPU_RST,
                               PM_TIMEOUT_INFINITE);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : PMHAL_PRCM_RG_EVE1_CPU_RST Release Failed \n");
    }

    if ((EntryPoint == 0) && (0U == gSblDevBuildFlag))
    {
        /* Power Down the EVE power domain */

        /* Force Idle the EVE EDMA TPTC0 and TPTC1 */
        HW_WR_REG32(SOC_EVE1_TPTC0_BASE + 0x10, 0);
        HW_WR_REG32(SOC_EVE1_TPTC1_BASE + 0x10, 0);

        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE1,
                              PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                              PM_TIMEOUT_INFINITE);

        PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE1,
                           PMHAL_PRCM_PD_STATE_OFF,
                           PM_TIMEOUT_INFINITE);

        PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE1,
                           PMHAL_PRCM_MODULE_MODE_DISABLED,
                           PM_TIMEOUT_INFINITE);
    }
    else
    {
        /*Check the Status of EVE Module mode*/
        do
        {
            PMHALModuleSIdleStatusGet(PMHAL_PRCM_MOD_EVE1, &sIdleState);
        }
        while (sIdleState != PMHAL_PRCM_MODULE_SIDLESTATE_FUNCTIONAL);
    }
}

/**
 * \brief         EVE2_Clk_Enable is PRCM function for EVE1. Configure the
 *                module to auto & force-wake-up the clock domain
 *
 *
 * \param         None.
 *
 * \return        error status.If error has occured it returns a non zero value.
 *                If no error has occured then return status will be zero.
 *
 **/
Int32 EVE2_Clk_Enable(Void)
{
    Int32 retval = 0;

    /*Enable the module - EVE2*/
    retval = PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE2,
                                PMHAL_PRCM_MODULE_MODE_AUTO,
                                PM_TIMEOUT_INFINITE);

    if (retval != 0)
    {
        Vps_printf("UTILS : EVELOADER : Enable EVE2 Module - Error \n");
        return retval;
    }

    /*Force Wake-up clock domain dsp1*/
    retval = PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE2,
                                   PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
                                   PM_TIMEOUT_INFINITE);

    Vps_printf("UTILS : EVELOADER : Enable EVE2 CLK Enable - completed \n");

    return retval;
}

/**
 * \brief       EVE2_BringUp function assert reset(CPU & EVE SS), set
 *              the entry point & release the CPU
 *              from reset.
 *
 *
 * \param[in]   EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE2_BringUp(UInt32 EntryPoint)
{
    UInt32 retVal = 0;
    pmhalPrcmModuleSIdleState_t sIdleState;

    Vps_printf("UTILS : EVELOADER :Set Entry Point\n");

    if (EntryPoint != 0)
    {
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE2_MMU0_BASE, EntryPoint);
    }
    else
    {
        /*Assuming 0x80000000 not in use*/
        HW_WR_REG32(0x80000000, 0x80000100); /* Reset vector points to
                                         *0x8000_0100*/
        HW_WR_REG32(0x80000100, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000104, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000108, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x8000010C, 0x0000037F); /* IDLE; opcode for ARP32 */
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE2_MMU0_BASE, 0x80000000);
    }

    Vps_printf("UTILS : EVELOADER :Bringout of reset \n");

    /* Bring-out of Reset*/
    retVal = PMHALResetRelease(PMHAL_PRCM_RG_EVE2_CPU_RST,
                               PM_TIMEOUT_INFINITE);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER : PMHAL_PRCM_RG_EVE2_CPU_RST Release Failed \n");
    }

    if ((EntryPoint == 0) && (0U == gSblDevBuildFlag))
    {
        /* Power Down the EVE power domain */

        /* Force Idle the EVE EDMA TPTC0 and TPTC1 */
        HW_WR_REG32(SOC_EVE2_TPTC0_BASE + 0x10, 0);
        HW_WR_REG32(SOC_EVE2_TPTC1_BASE + 0x10, 0);

        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE2,
                              PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                              PM_TIMEOUT_INFINITE);

        PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE2,
                           PMHAL_PRCM_PD_STATE_OFF,
                           PM_TIMEOUT_INFINITE);

        PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE2,
                           PMHAL_PRCM_MODULE_MODE_DISABLED,
                           PM_TIMEOUT_INFINITE);
    }
    else
    {
        /* Check the Status of EVE Module mode*/
        do
        {
            PMHALModuleSIdleStatusGet(PMHAL_PRCM_MOD_EVE2, &sIdleState);
        }
        while (sIdleState != PMHAL_PRCM_MODULE_SIDLESTATE_FUNCTIONAL);
    }
}

/**
 * \brief    EVE3_Clk_Enable is PRCM function for EVE1. Configure the
 *           module to auto & force-wake-up the clock domain
 *
 *
 * \param    None.
 *
 * \return   error status.If error has occured it returns a non zero value.
 *           If no error has occured then return status will be zero.
 *
 **/
Int32 EVE3_Clk_Enable(Void)
{
    Int32 retval = 0;


    /*Enable the module - EVE3*/
    retval = PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE3,
                                PMHAL_PRCM_MODULE_MODE_AUTO,
                                PM_TIMEOUT_INFINITE);

    if (retval != 0)
    {
        Vps_printf("UTILS : EVELOADER :Enable EVE3 Module - Error \n");
        return retval;
    }

    // Force Wake-up clock domain EVE3
    retval = PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE3,
                                   PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
                                   PM_TIMEOUT_INFINITE);

    Vps_printf("UTILS : EVELOADER :Enable EVE3 CLK Enable - completed \n");

    return retval;
}

/**
 * \brief         EVE3_BringUp function assert reset(CPU & EVE SS), set
 *    the entry point & release the CPU
 *    from reset.
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE3_BringUp(UInt32 EntryPoint)
{
    UInt32 retVal = 0;
    pmhalPrcmModuleSIdleState_t sIdleState;

    //SBL_PRINTF(TRACE_LOW, __func__);
    Vps_printf("UTILS : EVELOADER :Set Entry Point\n");

    if (EntryPoint != 0)
    {
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE3_MMU0_BASE, EntryPoint);
    }
    else
    {
        /*Assuming 0x80000000 not in use*/
        HW_WR_REG32(0x80000000, 0x80000100); /* Reset vector points to
                                         *0x8000_0100*/
        HW_WR_REG32(0x80000100, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000104, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000108, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x8000010C, 0x0000037F); /* IDLE; opcode for ARP32 */
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE3_MMU0_BASE, 0x80000000);
    }

    Vps_printf("UTILS : EVELOADER :Bringout of reset \n");

    /*Bring-out of Reset*/
    retVal = PMHALResetRelease(PMHAL_PRCM_RG_EVE3_CPU_RST,
                               PM_TIMEOUT_INFINITE);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER :PMHAL_PRCM_RG_EVE3_CPU_RST Release Failed \n");
    }

    if ((EntryPoint == 0) && (0U == gSblDevBuildFlag))
    {
        /* Power Down the EVE power domain */

        /* Force Idle the EVE EDMA TPTC0 and TPTC1 */
        HW_WR_REG32(SOC_EVE3_TPTC0_BASE + 0x10, 0);
        HW_WR_REG32(SOC_EVE3_TPTC1_BASE + 0x10, 0);

        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE3,
                              PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                              PM_TIMEOUT_INFINITE);

        PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE3,
                           PMHAL_PRCM_PD_STATE_OFF,
                           PM_TIMEOUT_INFINITE);

        PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE3,
                           PMHAL_PRCM_MODULE_MODE_DISABLED,
                           PM_TIMEOUT_INFINITE);
    }
    else
    {
        /*Check the Status of EVE Module mode*/
        do
        {
            PMHALModuleSIdleStatusGet(PMHAL_PRCM_MOD_EVE3, &sIdleState);
        }
        while (sIdleState != PMHAL_PRCM_MODULE_SIDLESTATE_FUNCTIONAL);
    }
}

/**
 * \brief    EVE4_Clk_Enable is PRCM function for EVE4. Configure the
 *           module to auto & force-wake-up the clock domain
 *
 * \param    None.
 *
 * \return   error status.If error has occured it returns a non zero value.
 *           If no error has occured then return status will be zero.
 *
 **/
Int32 EVE4_Clk_Enable(Void)
{
    Int32 retval = 0;

    // Enable the module - eve4
    retval = PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE4,
                                PMHAL_PRCM_MODULE_MODE_AUTO,
                                PM_TIMEOUT_INFINITE);

    if (retval != 0)
    {
        Vps_printf("UTILS : EVELOADER :Enable EVE4 Module - Error \n");
        return retval;
    }

    // Force Wake-up clock domain dsp1
    retval = PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE4,
                                   PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
                                   PM_TIMEOUT_INFINITE);

    Vps_printf("UTILS : EVELOADER :Enable EVE4 CLK Enable - completed \n");

    return retval;
}

/**
 * \brief   EVE4_BringUp function assert reset(CPU & EVE SS), set
 *          the entry point & release the CPU
 *          from reset.
 *
 *
 * \param[in]  EntryPoint - CPU entry location on reset
 *
 * \return      None.
 *
 **/
Void EVE4_BringUp(UInt32 EntryPoint)
{
    UInt32 retVal = 0;
    pmhalPrcmModuleSIdleState_t sIdleState;

    Vps_printf("UTILS : EVELOADER :Set Entry Point\n");

    if (EntryPoint != 0)
    {
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE4_MMU0_BASE, EntryPoint);
    }
    else
    {
        /*Assuming 0x80000000 not in use*/
        HW_WR_REG32(0x80000000, 0x80000100); /* Reset vector points to
                                         *0x8000_0100*/
        HW_WR_REG32(0x80000100, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000104, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x80000108, 0x0000037F); /* IDLE; opcode for ARP32 */
        HW_WR_REG32(0x8000010C, 0x0000037F); /* IDLE; opcode for ARP32 */
        /*EVE_MMU0 config & set the entry point*/
        EVE_SYSMMU_Config(SOC_EVE4_MMU0_BASE, 0x80000000);
    }

    Vps_printf("UTILS : EVELOADER :Bringout of reset \n");

    /* Bring-out of Reset*/
    retVal = PMHALResetRelease(PMHAL_PRCM_RG_EVE4_CPU_RST,
                               PM_TIMEOUT_INFINITE);
    if (0U != retVal)
    {
        Vps_printf("UTILS : EVELOADER :PMHAL_PRCM_RG_EVE4_CPU_RST Release Failed \n");
    }

    if ((EntryPoint == 0) && (0U == gSblDevBuildFlag))
    {
        /* Power Down the EVE power domain */

        /* Force Idle the EVE EDMA TPTC0 and TPTC1 */
        HW_WR_REG32(SOC_EVE4_TPTC0_BASE + 0x10, 0);
        HW_WR_REG32(SOC_EVE4_TPTC1_BASE + 0x10, 0);

        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE4,
                              PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                              PM_TIMEOUT_INFINITE);

        PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE4,
                           PMHAL_PRCM_PD_STATE_OFF,
                           PM_TIMEOUT_INFINITE);

        PMHALModuleModeSet(PMHAL_PRCM_MOD_EVE4,
                           PMHAL_PRCM_MODULE_MODE_DISABLED,
                           PM_TIMEOUT_INFINITE);
    }
    else
    {
        /*Check the Status of EVE Module mode*/
        do
        {
            PMHALModuleSIdleStatusGet(PMHAL_PRCM_MOD_EVE4, &sIdleState);
        }
        while (sIdleState != PMHAL_PRCM_MODULE_SIDLESTATE_FUNCTIONAL);
    }
}


/**
 * \brief   slavecore_enable() - This function do the slave cores
 *          PRCM module enable & clockdomain
 *          force wake-up. Brings the slave cores system out of
 *          reset.
 *
 * \param[in]  EntryPoint -Dummy parameter
 *
 * \return      None.
 *
 **/
Void slavecore_prcm_enable()
{

    EVE1_Clk_Enable();
    CPU_SystemReset(EVE1_ID);
    EVE2_Clk_Enable();
    CPU_SystemReset(EVE2_ID);
    EVE3_Clk_Enable();
    CPU_SystemReset(EVE3_ID);
    EVE4_Clk_Enable();
    CPU_SystemReset(EVE4_ID);

}

