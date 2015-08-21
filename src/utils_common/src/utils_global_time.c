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
 * \file utils_global_time.c
 *
 * \brief  This file implements the global timer.
 *
 *          This file implements the periodic timer task for local M4. It
 *          increments the shared region with current M4 time stamp every 1ms
 *          Provides helper function to read this time stamp. This function can
 *          be called by all the CPUs. So that every CPU will get the
 *          same time reference for latency and debug print calculation.
 *
 *
 * \version 0.1 (Aug 2013) : [HS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>


/*******************************************************************************
 * Setup a low frequency clock periodic function
 * to call the CLK32KHZ counter function
 * in order to track overflow condition
 *
 * CLK32KHZ will roughly overflow after 1 day of operation
 * so calling this periodic function once in one hour is enough to
 * guard against overflow condition
 *******************************************************************************
 */
#define  COUNTER_32K_OVERFLOW_CHECK_TIMER_PERIOD_IN_MS          (1000*60*60)


#define COUNTER_32K_CR          *(volatile UInt32*)0x4AE04030
#define COUNTER_32K_CR_REF_CLK   (32*1024)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Global timer data structure.
 *
 *			This holds the handle to clock started.
 *
 *******************************************************************************
*/
typedef struct
{
    /**<  Periodic Clock object */
    BspOsal_ClockHandle clkHandle;
    /**<  Periodic Clock object handle */
    UInt32 oldClk32KhzValue;
    /**< Last value of CLK 32Khz timer to check overflow */
    UInt32 clk32KhzOverflow;
    /**< CLK 32Khz overflow count */
} Utils_GlobalTimerObj;

/**
 *******************************************************************************
 *
 *   \brief Global timer object
 *
 *          This holds the timer handle. This is for local processor.
 *
 *******************************************************************************
*/
Utils_GlobalTimerObj gUtils_GlobalTimerObj = { 0 };

/**
 *******************************************************************************
 *
 * \brief Periodic timer function
 *
 *        It get the current time and copies it into shared memory. For all
 *        cores this time is used as reference. This timer runs only for
 *        local core.
 *
 * \param   arg     [IN] Not used.
 *
 *******************************************************************************
 */
static Void Utils_globalTimerPrdFunc(UArg arg)
{
    Utils_getCurGlobalTimeInUsec();
}

/**
 *******************************************************************************
 *
 * \brief Initializes the global timer for 1ms period.
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
Int32 Utils_globalTimerInit()
{
    Utils_GlobalTimerObj *pClkObj;

    pClkObj = &gUtils_GlobalTimerObj;

    pClkObj->clkHandle = BspOsal_clockCreate(
                            (BspOsal_ClockFuncPtr)Utils_globalTimerPrdFunc,
                            COUNTER_32K_OVERFLOW_CHECK_TIMER_PERIOD_IN_MS,
                            FALSE,
                            pClkObj
                            );
    UTILS_assert(pClkObj->clkHandle!=NULL);

    BspOsal_clockStart(pClkObj->clkHandle);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get current Global time across all cores
 *        Its important to have a global time across all cores to identify
 *        certain things like latency/delay etc. All link should use this
 *        function to insert timestamp or calculate latency/delay etc.
 *
 * \return current Global time in units of micro sec's
 *
 *******************************************************************************
 */
UInt64 Utils_getCurGlobalTimeInMsec()
{
    return Utils_getCurGlobalTimeInUsec()/1000;
}

/**
 *******************************************************************************
 *
 * \brief Get current Global time across all cores
 *        Its important to have a global time across all cores to identify
 *        certain things like latency/delay etc. All link should use this
 *        function to insert timestamp or calculate latency/delay etc.
 *
 * \return current Global time in units of micro sec's
 *
 *******************************************************************************
 */
UInt64 Utils_getCurGlobalTimeInUsec()
{
    UInt64 curGblTime;
    UInt32 oldIntState;
    UInt32 clk32KhzValue;
    UInt64 clk32KhzValue64;

    oldIntState = Hwi_disable();

    clk32KhzValue = COUNTER_32K_CR;

    if(clk32KhzValue < gUtils_GlobalTimerObj.oldClk32KhzValue)
        gUtils_GlobalTimerObj.clk32KhzOverflow++;

    clk32KhzValue64 = ((UInt64)clk32KhzValue | ((UInt64)gUtils_GlobalTimerObj.clk32KhzOverflow << 32));

    curGblTime = (1000000*clk32KhzValue64)/COUNTER_32K_CR_REF_CLK;

    gUtils_GlobalTimerObj.oldClk32KhzValue = clk32KhzValue;

    Hwi_restore(oldIntState);

    return (curGblTime);
}

