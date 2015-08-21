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
 * \file osa_global_time.c
 *
 * \brief  This file implements the global timer.
 *
 * \version 0.1 (Aug 2013) : [HS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <osa.h>
#include <osa_mem.h>
#include <osa_mutex.h>

#define COUNTER_32K_CR_REG_PHYS_ADDR        (0x4AE04030)
#define COUNTER_32K_CR_REF_CLK              (32*1024)

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
    OSA_MutexHndl lock;
    /**< lock to serialize access to counter */

    unsigned int COUNTER_32K_CR_REG_VIRT_ADDR;
    /**< CLK32KHZ virtual address */

    UInt32 oldClk32KhzValue;
    /**< Last value of CLK 32Khz timer to check overflow */

    UInt32 clk32KhzOverflow;
    /**< CLK 32Khz overflow count */

} OSA_GlobalTimerObj;

/**
 *******************************************************************************
 *
 *   \brief Global timer object
 *
 *          This holds the timer handle. This is for local processor.
 *
 *******************************************************************************
*/
OSA_GlobalTimerObj gOSA_GlobalTimerObj;

/**
 *******************************************************************************
 *
 * \brief Initializes the global timer for 1ms period.
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
int OSA_globalTimerInit()
{
    Int32 status;

    memset(&gOSA_GlobalTimerObj, 0, sizeof(gOSA_GlobalTimerObj));

    status = OSA_mutexCreate(&gOSA_GlobalTimerObj.lock);
    OSA_assertSuccess(status);

    gOSA_GlobalTimerObj.COUNTER_32K_CR_REG_VIRT_ADDR
        = OSA_memMap(COUNTER_32K_CR_REG_PHYS_ADDR, sizeof(UInt32));

    return 0;
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
int OSA_globalTimerDeInit()
{
    Int32 status;

    status = OSA_mutexDelete(&gOSA_GlobalTimerObj.lock);
    OSA_assertSuccess(status);

    OSA_memUnMap(gOSA_GlobalTimerObj.COUNTER_32K_CR_REG_VIRT_ADDR, sizeof(UInt32));

    return 0;
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
UInt64 OSA_getCurGlobalTimeInMsec()
{
    return OSA_getCurGlobalTimeInUsec()/1000;
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
UInt64 OSA_getCurGlobalTimeInUsec()
{
    UInt64 curGblTime;
    UInt32 clk32KhzValue;
    UInt64 clk32KhzValue64;

    OSA_mutexLock(&gOSA_GlobalTimerObj.lock);

    clk32KhzValue = *(volatile UInt32*)gOSA_GlobalTimerObj.COUNTER_32K_CR_REG_VIRT_ADDR;

    if(clk32KhzValue < gOSA_GlobalTimerObj.oldClk32KhzValue)
        gOSA_GlobalTimerObj.clk32KhzOverflow++;

    clk32KhzValue64 = ((UInt64)clk32KhzValue | ((UInt64)gOSA_GlobalTimerObj.clk32KhzOverflow << 32));

    curGblTime = (1000000*clk32KhzValue64)/COUNTER_32K_CR_REF_CLK;

    gOSA_GlobalTimerObj.oldClk32KhzValue = clk32KhzValue;

    OSA_mutexUnlock(&gOSA_GlobalTimerObj.lock);

    return (curGblTime);
}

