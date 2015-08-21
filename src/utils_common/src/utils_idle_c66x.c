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
 * \file utils_idle_c66x.c
 *
 * \brief  APIs from this file are used to interface the CPU Idle functions
 *
 *         The APIs allow a user to enable CPU idle on a given processor.
 *
 * \version 0.0 (Dec 2014) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <stdint.h>
#include <hw/hw_types.h>
#include <dsp_wugen.h>
#include <src/utils_common/include/utils_idle.h>
#include <pm/pmlib/pmlib_cpuidle.h>
#include <pm/pmhal/pmhal_pdm.h>
#include <pm/pmhal/pmhal_cm.h>
#include <pm/pmhal/pmhal_sd.h>
#include <src/links_dsp/system/system_priv_c6xdsp.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* None */

#ifdef A15_TARGET_OS_LINUX
/*
 * To enable DSP CPU Idle mode,
 * PMLIBCpuIdle() function needs to be loaded to L2SRAM.
 * However currently rproc loader on linux side does not
 * support loading sections from DSP binary to L2SRAM.
 *
 * Hence disabling CPU Idle feature when Linux runs on A15.
 */
#undef CPU_IDLE_ENABLED
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
    System_DspObj *pObj;
    pObj = &gSystem_objDsp;
    UInt32 key;

    key = Hwi_disable();
    DSP_WUGEN_IRQ_Interrupt_Lookup();
    Hwi_restore(key);

    if (0U != pObj->c66CpuIdleInitDone)
    {
        PMLIBCpuIdle(PMHAL_PRCM_PD_STATE_RETENTION);
    }
#endif
}

/**
 *******************************************************************************
 *
 * \brief Prepare the system configuration for CPU idle in IDLE Task.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idlePrepare(Void)
{
#ifdef CPU_IDLE_ENABLED
    System_DspObj *pObj;
    pObj = &gSystem_objDsp;
    UInt32 key;

    /* Enable IPU Clock domain to HW_AUTO and set next power state to ON as ipu
       is master is VISION SDK */
    if (SYSTEM_PROC_DSP1 == System_getSelfProcId())
    {
        PMHALStaticDepDisableToAllMaster((pmhalPrcmCdId_t) PMHAL_PRCM_CD_DSP1,
                                        NULL);
        PMHALStaticDepDisableAllSlaveDep((pmhalPrcmCdId_t) PMHAL_PRCM_CD_DSP1);
        PMHALPdmSetPDState(PMHAL_PRCM_PD_DSP1, PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                           PM_TIMEOUT_NOWAIT);
        PMHALCMSetCdClockMode((pmhalPrcmCdId_t) PMHAL_PRCM_CD_DSP1,
                              PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                              PM_TIMEOUT_NOWAIT);

    }
#ifndef TDA2EX_BUILD
    else
    {
        PMHALStaticDepDisableToAllMaster((pmhalPrcmCdId_t) PMHAL_PRCM_CD_DSP2,
                                        NULL);
        PMHALStaticDepDisableAllSlaveDep((pmhalPrcmCdId_t) PMHAL_PRCM_CD_DSP2);
        PMHALPdmSetPDState(PMHAL_PRCM_PD_DSP2, PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                           PM_TIMEOUT_NOWAIT);
        PMHALCMSetCdClockMode((pmhalPrcmCdId_t) PMHAL_PRCM_CD_DSP2,
                               PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                               PM_TIMEOUT_NOWAIT);
    }
#endif
    key = Hwi_disable();
    DSP_WUGEN_IRQ_Init();
    DSP_WUGEN_IRQ_Interrupt_Lookup();
    Hwi_restore(key);
    PMLIBSetCorepacPowerDown((uint32_t) 1U);
    pObj->c66CpuIdleInitDone = 1U;
#endif
}

/* Nothing beyond this point */
