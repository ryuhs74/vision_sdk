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
 * \file utils_idle_arp32.c
 *
 * \brief  APIs from this file are used to interface the CPU Idle functions
 *
 *         The APIs allow a user to enable CPU idle on a given processor.
 *
 * \version 0.0 (Jan 2015) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <stdint.h>
#include <hw/hw_types.h>
#include <arp32_wugen.h>
#include "soc_defines.h"
#include "platform.h"
#include <src/utils_common/include/utils.h>
#include <src/utils_common/include/utils_idle.h>
#include <src/utils_common/include/utils_prf.h>
#include <src/links_eve/system/system_priv_eve.h>
#include <include/link_api/system_trace.h>
#include <pmhal_prcm.h>
#include <pm/pmlib/pmlib_cpuidle.h>
#include <pm/pmhal/pmhal_pdm.h>
#include <pm/pmhal/pmhal_cm.h>
#include <ti/sysbios/knl/Swi.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#ifdef CPU_IDLE_ENABLED
static volatile UInt64 startTime = 0;
static volatile UInt64 endTime = 0;
#endif

/**
 *******************************************************************************
 *
 * \brief Idle function to be plugged into Idle task.
 *        This function executes idle instruction and puts CPU into low power
 *        state. The targeted low power state is Retention.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idleFxn(Void)
{
#ifdef CPU_IDLE_ENABLED
    System_EveObj *pObj;
    pObj = &gSystem_objEve;
    Uint32 key;
    key = Hwi_disable();
    ARP32_WUGEN_IRQ_Interrupt_Lookup();
    Hwi_restore(key);

    if (0U != pObj->eveIdleInitDone)
    {
        PMLIBCpuIdle(PMHAL_PRCM_PD_STATE_RETENTION);
    }
#endif
}

/**
 *******************************************************************************
 *
 * \brief Prepare the system configuration for CPU idle in IDLE Task.
 *        In case of A15, this function enables SR3-APG (HG / Fast Ramp-up).
 *        It also forces A15_1 to OFF.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idlePrepare(Void)
{
#ifdef CPU_IDLE_ENABLED
    System_EveObj *pObj;
    pObj = &gSystem_objEve;
    Int32 status = (Int32) PM_SUCCESS;
    pObj->evePreIdleTimeStampTaken = 0;
    if (SYSTEM_PROC_EVE1 == System_getSelfProcId())
    {
        status = PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE1,
                           PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                           PM_TIMEOUT_NOWAIT);

        status |=  PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE1,
                           PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                           PM_TIMEOUT_NOWAIT);
        pObj->eveIdleInitDone = 1U;
    }
#if defined (TDA2XX_FAMILY_BUILD)
    if (SYSTEM_PROC_EVE2 == System_getSelfProcId())
    {

        status = PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE2,
                           PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                           PM_TIMEOUT_NOWAIT);

        status |=  PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE2,
                           PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                           PM_TIMEOUT_NOWAIT);
        pObj->eveIdleInitDone = 1U;
    }
    if (SYSTEM_PROC_EVE3 == System_getSelfProcId())
    {
        status = PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE3,
                           PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                           PM_TIMEOUT_NOWAIT);

        status |=  PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE3,
                           PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                           PM_TIMEOUT_NOWAIT);
        pObj->eveIdleInitDone = 1U;
    }
    if (SYSTEM_PROC_EVE4 == System_getSelfProcId())
    {
        status = PMHALPdmSetPDState(PMHAL_PRCM_PD_EVE4,
                           PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                           PM_TIMEOUT_NOWAIT);

        status |=  PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE4,
                           PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                           PM_TIMEOUT_NOWAIT);
        pObj->eveIdleInitDone = 1U;
    }
#endif
    /* If the DMA is freed then disable the DMA by setting
     * Force standby and Force Idle.
     */
    {
        UInt32 regVal  = HW_RD_REG32(0x400A5640);
        if (0U == regVal)
        {
            HW_WR_REG32(0x40086000 + 0x10, 0);
            HW_WR_REG32(0x40087000 + 0x10, 0);
        }
    }

    Utils_idleSetEveMode(UTILS_IDLE_EVE_IDLE);

#endif
}
/**
 *******************************************************************************
 *
 * \brief Function which sets the EVEs corresponding Power Mode. EVE Idle only
 *        or Auto clock gate. This function gets called from the system settings
 *        call.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idleSetEveMode(UInt32 eveMode)
{
#ifdef CPU_IDLE_ENABLED
    System_EveObj *pObj;
    UInt32 regVal;
    pmhalPrcmCdClkTrnModes_t cdMode;
    pObj = &gSystem_objEve;

    if (UTILS_IDLE_EVE_AUTOCG == eveMode)
    {
        Vps_printf(" UTILS_IDLE: Setting EVE %d to Auto clock Gate.\n",
                    (System_getSelfProcId() - SYSTEM_PROC_EVE1 + 1));
        Vps_printf(
        "             Note: CPU Load calculations not accurate in this mode.\n");

        cdMode =  PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO;
        pObj->evePowerMode = UTILS_IDLE_EVE_AUTOCG;
        /* Configure the SCTM counters to idle when the
         * ARP32 is IDLE.
         */
        regVal  = HW_RD_REG32(0x40085000);
        regVal &= 0xFFFFFFF9U;
        regVal |= 0x04U;
        HW_WR_REG32(0x40085000, regVal);
        regVal  = HW_RD_REG32(0x40085100);
        regVal &= ~(0x20U);
        HW_WR_REG32(0x40085100, regVal);

    }
    else
    {
        Vps_printf(" UTILS_IDLE: Setting EVE %d to ARP32 Idle Only Mode.\n",
                    (System_getSelfProcId() - SYSTEM_PROC_EVE1 + 1));
        Vps_printf(
        "             Note: CPU Load calculations accurate in this mode.\n");
        cdMode =  PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP;
        pObj->evePowerMode = UTILS_IDLE_EVE_IDLE;

        /* Configure the SCTM counters to continue counting even when the
         * ARP32 is IDLE.
         */
        regVal  = HW_RD_REG32(0x40085000);
        regVal &= 0xFFFFFFF9U;
        regVal |= 0x02U;
        HW_WR_REG32(0x40085000, regVal);
        regVal  = HW_RD_REG32(0x40085100);
        regVal |= 0x20U;
        HW_WR_REG32(0x40085100, regVal);
    }

    if (SYSTEM_PROC_EVE1 == System_getSelfProcId())
    {
        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE1,
                       cdMode,
                       PM_TIMEOUT_NOWAIT);
    }
#if defined (TDA2XX_FAMILY_BUILD)
    if (SYSTEM_PROC_EVE2 == System_getSelfProcId())
    {

        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE2,
                       cdMode,
                       PM_TIMEOUT_NOWAIT);
    }
    if (SYSTEM_PROC_EVE3 == System_getSelfProcId())
    {
        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE3,
                       cdMode,
                       PM_TIMEOUT_NOWAIT);
    }
    if (SYSTEM_PROC_EVE4 == System_getSelfProcId())
    {

        PMHALCMSetCdClockMode(PMHAL_PRCM_CD_EVE4,
                       cdMode,
                       PM_TIMEOUT_NOWAIT);
     }
#endif
#endif
}

/**
 *******************************************************************************
 *
 * \brief This Function Enables the TPTC by setting it to smart IDLE and smart
 *        standby. The EDMA is operational after this setting.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idleEnableEveDMA()
{
#ifdef CPU_IDLE_ENABLED
    /* Enable DMA before ALG Process. Put the TPTC to
     * Smart Standby and Smart Idle
     */
    HW_WR_REG32(0x40086000 + 0x10, 0x28);
    HW_WR_REG32(0x40087000 + 0x10, 0x28);
#endif
}

/**
 *******************************************************************************
 *
 * \brief This Function Disables the TPTC by setting it to force IDLE and force
 *        standby. The EDMA is in low power mode after this setting.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idleDisableEveDMA()
{
#ifdef CPU_IDLE_ENABLED
    /* If the DMA is freed then disable the DMA by setting
    * Force standby and Force Idle.
    */
    UInt32 regVal  = HW_RD_REG32(0x400A5640);
    if (0U == regVal)
    {
        HW_WR_REG32(0x40086000 + 0x10, 0);
        HW_WR_REG32(0x40087000 + 0x10, 0);
    }
#endif
}

/**
 *******************************************************************************
 *
 * \brief This function calculates the time stamp when the EVE comes out of
 *        Auto clock gated state. This is used for CPU load calculations.
 *
 * \return None
 *
 *******************************************************************************
 */

Void Utils_idleGetPreEveAutoCgTime()
{
#ifdef CPU_IDLE_ENABLED
    UInt64 totalTime;
    System_EveObj *pObj;

    pObj = &gSystem_objEve;
    if (pObj->evePowerMode == UTILS_IDLE_EVE_AUTOCG)
    {
        startTime = Utils_getCurGlobalTimeInUsec();
        totalTime = startTime - endTime;
        Utils_prfUpdateEveLoadPreAutoCg(totalTime);
    }
    pObj->evePreIdleTimeStampTaken = 1;
#endif
}

/**
 *******************************************************************************
 *
 * \brief This function calculates the time stamp when the EVE comes out of
 *        Auto clock gated state. This is used for CPU load calculations.
 *
 * \return None
 *
 *******************************************************************************
 */

Void Utils_idleGetPostEveAutoCgTime()
{
#ifdef CPU_IDLE_ENABLED
    System_EveObj *pObj;

    pObj = &gSystem_objEve;
    /* When the EVE power mode is Auto clock gate do not calculate
     * CPU load as the task loads would not be accurate.
     */
    if (pObj->evePreIdleTimeStampTaken == 0)
        return;
    if (pObj->evePowerMode == UTILS_IDLE_EVE_AUTOCG)
    {
        UInt64 totalTimeIdle;
        endTime = Utils_getCurGlobalTimeInUsec();
        totalTimeIdle = endTime - startTime;
        Utils_prfUpdateEveLoadPostAutoCg(totalTimeIdle);
    }
    pObj->evePreIdleTimeStampTaken = 0;
#endif
}

/* Nothing beyond this point */
