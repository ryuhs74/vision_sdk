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
 * \file utils_idle_M4.c
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
#include <src/utils_common/include/utils_idle.h>
#include <src/utils_common/include/utils.h>
#include <hw/hw_types.h>
#include <armv7m/ipu_wugen.h>
#include <pm/pmlib/pmlib_cpuidle.h>
#include <pm/pmhal/pmhal_pdm.h>
#include <pm/pmhal/pmhal_cm.h>
#include <pm/pmhal/pmhal_mm.h>
#include <pm/pmhal/pmhal_rm.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 typedef struct {
    UInt32  ipuIdleInitDone;
} Utils_IpuCpuIdleObj;
Utils_IpuCpuIdleObj gUtils_IpuCpuIdleObj = {0};

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
    UInt32 key;
    Utils_IpuCpuIdleObj *pObj;
    pObj = &gUtils_IpuCpuIdleObj;

    key = Hwi_disable();
    IPU_WUGEN_Interrupt_Lookup();
    Hwi_restore(key);

    if(0U != pObj->ipuIdleInitDone)
    {
        PMLIBCpuIdle(PMHAL_PRCM_PD_STATE_RETENTION);
    }
#endif

}

/**
 *******************************************************************************
 *
 * \brief Prepare the system configuration for CPU idle in IDLE Task.
 *        In case of M4, this function sets the desired clock settings
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_idlePrepare(Void)
{
#ifdef CPU_IDLE_ENABLED
    Utils_IpuCpuIdleObj *pObj;
    pObj = &gUtils_IpuCpuIdleObj;
    /* Enable IPU Clock domain to HW_AUTO/SW_WAKEUP and set next power state to
        ON as ipu is master is VISION SDK */

    PMHALPdmSetPDState(PMHAL_PRCM_PD_IPU,
                        PMHAL_PRCM_PD_STATE_ON_ACTIVE,
                        PM_TIMEOUT_NOWAIT);

    PMHALCMSetCdClockMode( PMHAL_PRCM_CD_IPU,
                                PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
                                PM_TIMEOUT_NOWAIT);

    PMHALCMSetCdClockMode(PMHAL_PRCM_CD_IPU1,
                                PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                                PM_TIMEOUT_NOWAIT);

#ifdef TDA3XX_FAMILY_BUILD
    /*
     * This is required as the force override bit CTRL_CORE_SEC_IPU_WAKEUP
     * does not set the right values for the PRCM registers and when the
     * override is lifted then cores are left in a bad power and reset state.
     */
    PMHALResetRelease(PMHAL_PRCM_RG_IPU1_CPU0_RST, PM_TIMEOUT_NOWAIT);
    PMHALResetRelease(PMHAL_PRCM_RG_IPU1_RST, PM_TIMEOUT_NOWAIT);
    PMHALModuleModeSet(PMHAL_PRCM_MOD_IPU1,
                                 PMHAL_PRCM_MODULE_MODE_AUTO,
                                 PM_TIMEOUT_NOWAIT);
#endif
    pObj->ipuIdleInitDone = 1U;
#endif

}

/* Nothing beyond this point */
