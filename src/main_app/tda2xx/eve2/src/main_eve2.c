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
 * \file main_eve2.c
 *
 * \brief  This file implements the EVE2 main()
 *
 *         This file has the EVE2 main(), the entry point to the core.
 *         Set the EVE2 clock and call System_start() & BIOS_start()
 *
 * \version 0.0 (Aug 2013) : [KRB] First version
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

/*******************************************************************************
 *  Function prototypes
 *******************************************************************************
 */
Int32 System_start(Task_FuncPtr Eve2_main);
Int32 StartupEmulatorWaitFxn (void);

/**
 *******************************************************************************
 *
 * \brief This function implements the wait loop of EVE2.
 *
 *        This function would get called if the EVE2 doesn't run the demo
 *        or application. It is not used if EVE2 runs the app.  Do not
 *        remove this function, required when App is moved out of EVE2
 *
 * \param  arg0 [IN]
 * \param  arg1 [IN]
 *
 * \return  void
 *
 *******************************************************************************
 */
Void Eve2_main(UArg arg0, UArg arg1)
{
    UInt32 state;
    UInt32 coreId = System_getSelfProcId();
    /* Doing this in two places because the BIOS_Start API is reconfiguring
     * SCTM Timers.
     */
    Utils_idlePrepare();
    while (1)
    {
        Task_sleep(100);
        Utils_getAppInitState(coreId, &state);
        if (state == CORE_APP_INITSTATUS_DO_EXIT)
            break;
    }
}

/**
 *******************************************************************************
 *
 * \brief This is the main() implementation of EVE2.
 *
 *        This is the first function  and entry point to EVE2, does
 *         - Set the correct/required CPU frequency
 *         - Call the System_start with Eve2_main() and loops there
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
    Vps_printf("***** EVE2 Firmware build time %s %s \n",
               __TIME__,__DATE__);


    /* This is for debug purpose- see the description of function header */
    StartupEmulatorWaitFxn();

    {
        UInt32 clkHz;

        clkHz = Utils_getClkHz(UTILS_CLK_ID_EVE)/2;

        if(clkHz==0)
            clkHz = SYSTEM_EVE_FREQ;

        Utils_setCpuFrequency(clkHz);
    }
    /* Calling this init here to make sure EVE does not hang when
     * put to auto clock gate.
     */
    Utils_idlePrepare();

    System_start(Eve2_main);
    BIOS_start();

    return (SYSTEM_LINK_STATUS_SOK);
}

/**
 *******************************************************************************
 *
 * \brief This function enables the EVE2 debug option
 *
 *        This function enables the EVE2 debug option from main() onwards
 *         - Set the volatile variable enableDebug = 1 to enable debug
 *         - rebuild the code with enableDebug = 1
 *         - Once enableDebug is set to 1, the control waits in this
 *           function even after the free-run
 *         - Can connect to core EVE via CCS and J-Tag to debug
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
 * \brief This function enables the ARP32_CPU_1 Timer Ticks
 *
 * \param  arg [IN]
 *
 *******************************************************************************
 */
void mainARP32_1_TimerTick(UArg arg)
{
    Clock_tick();
}

/* Nothing beyond this point */
