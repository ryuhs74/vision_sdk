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
 * \file utils_idle_a15.c
 *
 * \brief  APIs from this file are used to interface the CPU Idle functions
 *
 *         The APIs allow a user to enable CPU idle on a given processor.
 *
 * \version 0.0 (Sep 2014) : [SL] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <stdint.h>
#include <src/utils_common/include/utils_idle.h>
#include <hw/hw_types.h>
#include <armv7a/tda2xx/mpu_wugen.h>
#include <pm/pmlib/pmlib_cpuidle.h>
#include <pm/pmhal/pmhal_mpu_lprm.h>
#include <pm/pmhal/pmhal_pdm.h>
#include <pm/pmhal/pmhal_cm.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

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

    key = Hwi_disable();
    MPU_WUGEN_0_Interrupt_Lookup();
    Hwi_restore(key);
    PMLIBCpuIdle(PMHAL_PRCM_PD_STATE_RETENTION);

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
    pmhalMpuLprmHgRampParams_t hgRampParam = {1,0};
    pmhalMpuPowerStatusParams_t mpuPowerState;

    /* Enable Hg/FastRamp-up in Retention */
    PMHALMpuLprmSetHgRampParams(&hgRampParam);
    PMHALMpuLprmSetMercuryRetention();

    /* Enable MPU Clock domain to HW_AUTO and set next power state to RET */
    PMHALCMSetCdClockMode(PMHAL_PRCM_CD_MPU,
                          PMHAL_PRCM_CD_CLKTRNMODES_HW_AUTO,
                          PM_TIMEOUT_NOWAIT);
    PMHALPdmSetPDState(PMHAL_PRCM_PD_MPU,
                       PMHAL_PRCM_PD_STATE_RETENTION,
                       PM_TIMEOUT_NOWAIT);

    /* Make sure CPU1 is ON before forcing CPU1 off */
    PMHALMpuLprmGetPowerStatus(1, &mpuPowerState);
    if (PMHAL_PRCM_PD_STATE_ON_ACTIVE == mpuPowerState.currPowerStatus)
    {
        PMLIBCpu1ForcedOff();
    }
#endif
}

/* Nothing beyond this point */
