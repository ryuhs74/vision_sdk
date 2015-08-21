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
 * \file main_ipu1_0.c
 *
 * \brief  This file implements the IPU1_0 main()
 *
 *         This file has the IPU1_0 main(), the entry point to the core.
 *         Set the IPU1_0 clock and call System_start() & BIOS_start()
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 * \version 0.2  (Jul 2013) : [HS] Added support for remote debug
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
#include <src/utils_common/include/utils.h>
#include <include/link_api/system_common.h>
#include <src/links_common/system/system_priv_common.h>
#include <src/utils_common/include/utils_idle.h>
#include <src/utils_common/include/utils_timer_reconfig.h>

#include <xdc/cfg/global.h>

#ifdef A15_TARGET_OS_LINUX
/* xdctools header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Registry.h>

#include <src/utils_common/include/utils_prcm.h>
#include <src/utils_common/include/utils_eveloader.h>
#include <src/links_common/system/system_rsc_table_ipu.h>
#endif

#include "soc.h"
#include "wd_timer.h"

#ifdef A15_TARGET_OS_LINUX
/* private data */
Registry_Desc               Registry_CURDESC;
#endif

/*******************************************************************************
 *  Function prototypes
 *******************************************************************************
 */
Int32 System_start(Task_FuncPtr chainsMainFunc);
Int32 StartupEmulatorWaitFxn (void);

/*******************************************************************************
 *  external reference
 *******************************************************************************
 */
extern Void Chains_main(UArg arg0, UArg arg1);


/*******************************************************************************
 *  Return Physical address for VPDMA descriptor memory
 *******************************************************************************
 */
UInt32 System_getVpdmaPhysAddr()
{
    return XDC_CFG_VPDMA_DESC_PHYS_ADDR;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the wait loop of IPU1_0.
 *
 *        This function would get called if the IPU1_0 doesn't run the demo
 *        or application. It is not used if IPU1_0 runs the app.  Do not
 *        remove this function, required when App is moved out of IPU1_0
 *
 * \param  arg0 [IN]
 * \param  arg1 [IN]
 *
 * \return  void
 *
 *******************************************************************************
 */
Void IPU1_0_main(UArg arg0, UArg arg1)
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
 * \brief This is the main() implementation of IPU1_0.
 *
 *        This is the first function  and entry point to IPU1_0, does
 *         - Set the correct/required CPU frequency
 *         - Call the System_start with either Chain_main() or IPU1_0_main()
 *         - Call Chain_main() if App/Chain App is runs on local core
 *         - Call IPU1_0_main() instead of Chain_main if App/Chain is
 *           implemented on any other remote code
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
int main(void)
{


    /* This is for debug purpose- see the description of function header */
    StartupEmulatorWaitFxn();


#ifdef A15_TARGET_OS_LINUX
    Registry_Result result;

    Utils_prcmInit();

    /* register with xdc.runtime to get a diags mask */
    result = Registry_addModule(&Registry_CURDESC, MODULE_NAME);
    Assert_isTrue(result == Registry_SUCCESS, (Assert_Id)NULL);

    /* enable some log events */
    Diags_setMask(MODULE_NAME"+EXF");

    /* zero initialize NDK_MEM section */
    memset((Ptr)XDC_CFG_NDK_MEM, 0x0, XDC_CFG_NDK_MEM_SIZE);

#ifdef IPU1_LOAD_EVES
    /* Load eves if needed */
    Utils_eveBoot();
#endif
#endif

    /* Disable the watchdog timer. The policy for handling WD Timer
     * needs to be defined and implemented appropriately */
    WDTIMERDisable(SOC_WD_TIMER2_BASE);

    /* This should the first call for initializing the remote debug server
     */
    RemoteLog_init();
#ifdef A15_TARGET_OS_BIOS
    Utils_remoteLogClientInit();
#endif

    Vps_printf(" ***** IPU1_0 Firmware build time %s %s \n",
               __TIME__,__DATE__);


    {
        UInt32 clkHz;

        clkHz = Utils_getClkHz(UTILS_CLK_ID_IPU);

        if(clkHz==0)
            clkHz = SYSTEM_IPU_FREQ;

        Utils_setCpuFrequency(clkHz);
    }

    /* Timer i767 Silicon Issue workaround */
    Utils_TimerSetTsicrReadMode();

    System_start(Chains_main);
    BIOS_start();

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function enables the IPU1_0 debug option
 *
 *        This function enables the IPU1_0 debug option from main() onwards
 *         - Set the volatile variable enableDebug = 1 to enable debug
 *         - rebuild the code with enableDebug = 1
 *         - Once enableDebug is set to 1, the control waits in this
 *           function even after the free-run
 *         - Can connect to core IPU1_0 via CCS and J-Tag to debug
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
void mainIPU1_C0TimerTick(UArg arg)
{
    Clock_tick();
}

/* Nothing beyond this point */



