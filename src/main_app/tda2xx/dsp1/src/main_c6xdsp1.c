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
 * \file main_c6xdsp1.c
 *
 * \brief  This file implements the DSP1 main()
 *
 *         This file has the DSP1 main(), the entry point to the core.
 *         Set the DSP1 clock and call System_start() & BIOS_start()
 *
 * \version 0.0 (Jul 2013) : [SS] First version
 *
 *******************************************************************************
*/

#ifdef A15_TARGET_OS_LINUX
/* This define must precede inclusion of any xdc header file */
#define Registry_CURDESC Test__Desc
#define MODULE_NAME "Server"
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#ifdef A15_TARGET_OS_LINUX
/* xdctools header files */
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Registry.h>
#endif
#include <src/utils_common/include/utils_idle.h>
#include <src/utils_common/include/utils.h>
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_timer_reconfig.h>

#ifdef A15_TARGET_OS_LINUX
#include <src/links_common/system/system_priv_common.h>
#include <src/links_common/system/system_rsc_table_dsp.h>

/* private data */
Registry_Desc               Registry_CURDESC;
#endif


/*******************************************************************************
 *  Function prototypes
 *******************************************************************************
 */
Int32 System_start(Task_FuncPtr C6XDSP_main);
Int32 StartupEmulatorWaitFxn (void);
Void Utils_dspCacheInit();

/**
 *******************************************************************************
 *
 * \brief This function implements the wait loop of DSP1.
 *
 *        This function would get called if the DSP1 doesn't run the demo
 *        or application. It is not used if DSP1 runs the app.  Do not
 *        remove this function, required when App is moved out of DSP1
 *
 * \param  arg0 [IN]
 * \param  arg1 [IN]
 *
 * \return  void
 *
 *******************************************************************************
 */
Void C6XDSP_main(UArg arg0, UArg arg1)
{
    UInt32 state;
    UInt32 coreId = System_getSelfProcId();

    Utils_dspCacheInit();

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
 * \brief This is the main() implementation of DSP1.
 *
 *        This is the first function  and entry point to DSP1, does
 *         - Set the correct/required CPU frequency
 *         - Call the System_start with C6XDSP_main() and loops there
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
int main(void)
{
#ifdef A15_TARGET_OS_LINUX
    Registry_Result result;

    /* register with xdc.runtime to get a diags mask */
    result = Registry_addModule(&Registry_CURDESC, MODULE_NAME);
    Assert_isTrue(result == Registry_SUCCESS, (Assert_Id)NULL);

    /* enable some log events */
    Diags_setMask(MODULE_NAME"+EXF");
#endif

    /* This should the first call for initializing the remote debug server
     */
    RemoteLog_init();
    Vps_printf(" ***** DSP1 Firmware build time %s %s\n",
               __TIME__,__DATE__);
    /* This is for debug purpose- see the description of function header */
    StartupEmulatorWaitFxn();

    {
        UInt32 clkHz;

        clkHz = Utils_getClkHz(UTILS_CLK_ID_DSP);

        if(clkHz==0)
            clkHz = SYSTEM_DSP_FREQ;

        Utils_setCpuFrequency(clkHz);
    }
    /* Timer i767 Silicon Issue workaround */
    Utils_TimerSetTsicrReadMode();

    Utils_idlePrepare();

    System_start(C6XDSP_main);
    BIOS_start();

    return (SYSTEM_LINK_STATUS_SOK);
}

/**
 *******************************************************************************
 *
 * \brief This function enables the DSP1 debug option
 *
 *        This function enables the DSP1 debug option from main() onwards
 *         - Set the volatile variable enableDebug = 1 to enable debug
 *         - rebuild the code with enableDebug = 1
 *         - Once enableDebug is set to 1, the control waits in this
 *           function even after the free-run
 *         - Can connect to core DSP via CCS and J-Tag to debug
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
 * \brief This function enables the DSP Timer Ticks
 *
 *        Enables the DSP Timer Ticks is required for C66 DSP, remove this
 *        once BIOS support auto enable of this
 *
 * \param  arg [IN]
 *
 *******************************************************************************
 */
void mainDsp1TimerTick(UArg arg)
{
    Clock_tick();
}

/* Nothing beyond this point */

