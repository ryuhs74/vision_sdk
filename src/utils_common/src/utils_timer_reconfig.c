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
 *
 * \file utils_timer_reconfig.c
 *
 * \brief This is a temporary file which would allow setting the TSICR register
 *        till the BIOS allows configuring this.
 *
 *
 * \version 0.0 (May 2015) : [PG] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <stdint.h>
#include <hw/hw_types.h>
#include <ti/sysbios/timers/dmtimer/package/internal/Timer.xdc.h>
#include <ti/sysbios/timers/dmtimer/Timer.h>
#include <src/utils_common/include/utils_timer_reconfig.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
#ifndef TDA3XX_BUILD
#define UTILS_TIMER_RECONFIG_NUM_TIMERS (16U)
#else
#define UTILS_TIMER_RECONFIG_NUM_TIMERS (8U)
#endif

extern __T1_ti_sysbios_timers_dmtimer_Timer_Module_State__handles
                ti_sysbios_timers_dmtimer_Timer_Module_State_0_handles__A[
                                               UTILS_TIMER_RECONFIG_NUM_TIMERS];

/**
 *******************************************************************************
 *
 * \brief Timer Reconfigure Function. This sets the read_mode bit of the TSICR
 *        register for all timers set for a given core.
 *
 * \return None
 *
 *******************************************************************************
 */

Void Utils_TimerSetTsicrReadMode()
{
    /* The following logic has been commented to ensure that BIOS
     * structures are not accessed directly from code. Waiting on BIOS
     * support for the READ_MODE configuration for timer. When the support
     * is added this function would be deprecated.
     */
#if 0
    struct ti_sysbios_timers_dmtimer_Timer_Object * timerObj;
    xdc_Int id;
    UInt32 timerBase, regVal;
    UInt32 i = 0;
    for (i = 0; i < UTILS_TIMER_RECONFIG_NUM_TIMERS; i++)
    {
        if (ti_sysbios_timers_dmtimer_Timer_Module_State_0_handles__A[i] == NULL)
        {
            continue;
        }

        timerObj = (ti_sysbios_timers_dmtimer_Timer_Object *)
            ti_sysbios_timers_dmtimer_Timer_Module_State_0_handles__A[i];

        id = timerObj->id;
        Vps_printf(" TIMER RECONFIG: Timer ID %d\n", id);
        timerBase = (UInt32)Timer_module->device[id].baseAddr;
        regVal = HW_RD_REG32(timerBase + 0x54);
        regVal = 0x8 | regVal;
        HW_WR_REG32(timerBase + 0x54, regVal);
    }
#endif
}

/* Nothing beyond this point */
