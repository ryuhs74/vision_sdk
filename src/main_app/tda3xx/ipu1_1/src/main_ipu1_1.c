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
 * \file main_ipu1_1.c
 *
 * \brief  This file implements the IPU1_1 main()
 *
 *         This file has the IPU1_1 main(), the entry point to the core.
 *         Set the IPU1_1 clock and call System_start() & BIOS_start()
 *
 * \version 0.0 (Aug 2013) : [SS] First version
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
Int32 System_start(Task_FuncPtr IPU1_1_main);
Int32 StartupEmulatorWaitFxn (void);

/**
 *******************************************************************************
 *
 * \brief This function implements the wait loop of IPU1_1.
 *
 *        This function would get called if the IPU1_1 doesn't run the demo
 *        or application. It is not used if IPU1_1 runs the app.  Do not
 *        remove this function, required when App is moved out of IPU1_1
 *
 * \param  arg0 [IN]
 * \param  arg1 [IN]
 *
 * \return  void
 *
 *******************************************************************************
 */
Void IPU1_1_main(UArg arg0, UArg arg1)
{
    UInt32 state;
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
 * \brief This is the main() implementation of IPU1_1.
 *
 *        This is the first function  and entry point to IPU1_1, does
 *         - Set the correct/required CPU frequency
 *         - Call the System_start with IPU1_1_main() and loops there
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
    Vps_printf(" ***** IPU1_1 Firmware build time %s %s\n",
               __TIME__,__DATE__);
    /* This is for debug purpose- see the description of function header */
    StartupEmulatorWaitFxn();

    {
        UInt32 clkHz;

        clkHz = Utils_getClkHz(UTILS_CLK_ID_IPU);

        if(clkHz==0)
            clkHz = SYSTEM_IPU_FREQ;

        Utils_setCpuFrequency(clkHz);
    }
    /* Timer i767 Silicon Issue workaround */
    Utils_TimerSetTsicrReadMode();

    System_start(IPU1_1_main);
    BIOS_start();

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function enables the IPU1_1 debug option
 *
 *        This function enables the IPU1_1 debug option from main() onwards
 *         - Set the volatile variable enableDebug = 1 to enable debug
 *         - rebuild the code with enableDebug = 1
 *         - Once enableDebug is set to 1, the control waits in this
 *           function even after the free-run
 *         - Can connect to core IPU1_1 via CCS and J-Tag to debug
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
 * \brief This function enables the IPU_CPU_0 Timer Ticks
 *
 * \param  arg [IN]
 *
 *******************************************************************************
 */
void mainIPU1_C1TimerTick(UArg arg)
{
    Clock_tick();
}

/* Nothing beyond this point */

