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
 * \file main_a15_01.c
 *
 * \brief  This file implements the A15_0 main()
 *
 *         This file has the A15_0 main(), the entry point to the core.
 *         Set the A15 clock and call System_start() & BIOS_start()
 *
 * \version 0.0 (Jul 2013) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_idle.h>
#include <src/utils_common/include/utils_timer_reconfig.h>

/*******************************************************************************
 *  Function prototypes
 *******************************************************************************
 */
Int32 System_start(Task_FuncPtr A15_main);
Int32 StartupEmulatorWaitFxn (void);

/**
 *******************************************************************************
 *
 * \brief This function implements the wait loop of A15.
 *
 *        This function would get called if the A15_0 doesn't run the demo
 *        or application. It is not used if A15_0 runs the app.  Do not
 *        remove this function, required when App is moved out of A15
 *
 * \param  arg0 [IN]
 * \param  arg1 [IN]
 *
 * \return  void
 *
 *******************************************************************************
 */
Void A15_main(UArg arg0, UArg arg1)
{
    unsigned int state;
    UInt32 coreId = System_getSelfProcId();

    Utils_idlePrepare();

    while (1)
    {
        BspOsal_sleep(100);
        Utils_getAppInitState(coreId, &state);
        if (state == CORE_APP_INITSTATUS_DO_EXIT)
            break;
    }
}

/**
 *******************************************************************************
 *
 * \brief This is the main() implementation of A15.
 *
 *        This is the first function  and entry point to A15, does
 *         - Set the correct/required CPU frequency
 *         - Call the System_start with A15_main() and loops there
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
int main(void)
{
    /* This should the first call for initializing the remote debug server
     */
    RemoteLog_init();
    Vps_printf(" ***** A15_0 Firmware build time %s %s \n",
               __TIME__,__DATE__);
    /* This is for debug purpose- see the description of function header */
    StartupEmulatorWaitFxn();

    {
        UInt32 clkHz;

        clkHz = Utils_getClkHz(UTILS_CLK_ID_A15);

        if(clkHz==0)
            clkHz = SYSTEM_A15_FREQ;

        Utils_setCpuFrequency(clkHz);
    }
    /* Timer i767 Silicon Issue workaround */
    Utils_TimerSetTsicrReadMode();

    System_start(A15_main);
    BIOS_start();

    return (SYSTEM_LINK_STATUS_SOK);
}

/**
 *******************************************************************************
 *
 * \brief This function enables the A15 debug option
 *
 *        This function enables the A15 debug option from main() onwards
 *         - Set the volatile variable enableDebug = 1 to enable debug
 *         - rebuild the code with enableDebug = 1
 *         - Once enableDebug is set to 1, the control waits in this
 *           function even after the free-run
 *         - Can connect to core A15via CCS and J-Tag to debug
 *         - Once CCS is connected, reset enableDebug = 0 to come out of
 *           this function and proceeds with further debug
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 StartupEmulatorWaitFxn (void)
{
    volatile int enableDebug = 0;
    while (enableDebug);
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function enables the A15 Timer Ticks
 *
 * \param  arg [IN]
 *
 *******************************************************************************
 */
void mainA15TimerTick(UArg arg)
{
    Clock_tick();
}

/**
 *******************************************************************************
 *
 * \brief: The load module does not update the total elapsed time for the idle
 * function on Core1 if no task switches happen. So, If Core1 is lightly loaded
 * and there are no task switches during the sampling window, the idle task
 * load on Core 1 will get artificially inflated. This is a Bug in Bios, will
 * get fixed in 6.42.01.
 * In the mean time, use the below workaround, This function will cause
 * the load module to update the idle task's total elapsed time and should fix
 * the idle task's computed load.
 *
 * \param  arg [IN]
 *
 *******************************************************************************
 */
void mainA15_idle_func(UArg arg)
{
    Load_updateCurrentThreadTime();
}



/* Nothing beyond this point */

