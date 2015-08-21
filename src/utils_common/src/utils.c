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
 * \file utils.c
 *
 * \brief Utility functions implementation
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <src/utils_common/include/utils.h>


/**
 *******************************************************************************
 * \brief Global variable to break out of a assert loop when debugging
 *******************************************************************************
*/
volatile int g_AssertFailLoop = TRUE;

int RemoteLog_setAppInitState(int coreId, unsigned int state);
int RemoteLog_getAppInitState(int coreId, unsigned int *pState);

/**
 *******************************************************************************
 *
 * \brief Convert Hex char to interger value
 *
 * \param c        [IN] Character
 *
 * \return interger value
 *
 *******************************************************************************
 */
static char xtod(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return c = 0;                                          /* not Hex digit */
}

/**
 *******************************************************************************
 *
 * \brief Recrusively convert Hex string to interger value
 *
 * \param hex        [IN] Hex string
 * \param l          [IN] Current value of integer so far during the conversion
 *
 *
 * \return integer value
 *
 *******************************************************************************
 */
static int HextoDec(char *hex, int l)
{
    if (*hex == 0)
        return (l);

    return HextoDec(hex + 1, l * 16 + xtod(*hex));         /* hex+1 */
}

/**
 *******************************************************************************
 *
 * \brief Convert hex string to integer value
 *
 * \param hex [IN] Hex string
 *
 * \return integer value after conversion
 *
 *******************************************************************************
 */
int xstrtoi(char *hex)

{
    return HextoDec(hex, 0);
}

/**
 *******************************************************************************
 *
 * \brief Function to set CPU Mhz with the OS
 *
 *        OS uses this to convert its timer tick to clock time
 *
 * \param freq [IN] Frequency in Hz
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_setCpuFrequency (UInt32 freq)
{
    UInt cookie;
    Types_FreqHz cpuHz;
    Types_FreqHz oldCpuHz;

    BIOS_getCpuFreq(&oldCpuHz);

    cookie = Hwi_disable();
    cpuHz.lo = freq;
    cpuHz.hi = 0;
    ti_sysbios_BIOS_setCpuFreq(&cpuHz);
    Clock_tickStop();
    Clock_tickReconfig();
    Clock_tickStart();
    Hwi_restore(cookie);

    BIOS_getCpuFreq(&cpuHz);

    Vps_printf(" *** SYSTEM: CPU Frequency <ORG = %d Hz>, <NEW = %d Hz>\n",
                oldCpuHz.lo,
                cpuHz.lo);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get current time in units of msec's
 *
 * \return Current time in units of msec's
 *
 *******************************************************************************
 */
UInt32 Utils_getCurTimeInMsec()
{
    static UInt32 cpuKhz = 500*1000; /* default */
    static Bool isInit = FALSE;

    Types_Timestamp64 ts64;
    UInt64 curTs;

    if(!isInit)
    {
        /* do this only once */

        Types_FreqHz cpuHz;

        isInit = TRUE;

        Timestamp_getFreq(&cpuHz);

        cpuKhz = cpuHz.lo / 1000; /* convert to Khz */

        Vps_printf(" *** UTILS: CPU KHz = %d Khz ***\n", cpuKhz);
    }

    Timestamp_get64(&ts64);

    curTs = ((UInt64) ts64.hi << 32) | ts64.lo;

    return (UInt32)(curTs/(UInt64)cpuKhz);
}

/**
 *******************************************************************************
 *
 * \brief Get current time in units of micro sec's
 *
 * \return Current time in units of micro sec's
 *
 *******************************************************************************
 */
UInt64 Utils_getCurTimeInUsec()
{
    static UInt32 cpuMhz = 500; /* default */
    static Bool isInit = FALSE;

    Types_Timestamp64 ts64;
    UInt64 curTs;

    if(!isInit)
    {
        /* do this only once */

        Types_FreqHz cpuHz;

        isInit = TRUE;

        Timestamp_getFreq(&cpuHz);

        cpuMhz = cpuHz.lo / (1000*1000); /* convert to Mhz */

        Vps_printf(" *** UTILS: CPU MHz = %d Mhz ***\n", cpuMhz);
    }

    Timestamp_get64(&ts64);

    curTs = ((UInt64) ts64.hi << 32) | ts64.lo;

    return (curTs/cpuMhz);

}

/**
 *******************************************************************************
 *
 * \brief Reset and init buffer skip context
 *
 * \param skipCtx           [OUT] Buffer skip context
 * \param inputFrameRate    [IN]  Expected input frame rate
 * \param outputFrameRate   [IN]  Required output frame rate
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_resetSkipBufContext(Utils_BufSkipContext *skipCtx,
                               UInt32 inputFrameRate,
                               UInt32 outputFrameRate)
{
    skipCtx->firstTime = TRUE;
    skipCtx->inputFrameRate = inputFrameRate;
    skipCtx->outputFrameRate = outputFrameRate;
}

/**
 *******************************************************************************
 *
 * \brief This function tells the caller whether current buffer should be
 *        skipped or not
 *
 *        The function uses Utils_BufSkipContext to keep track of rate
 *        of incoming buffers and outgoing buffers and based on this tells
 *        if current buffer should be skipped (return TRUE) or not skipped
 *        i.e used by the application (return FALSE)
 *
 *        Application should call this function everytime a buffers is received
 *        or need to be output and this function will tell whether to skip
 *        this buffer or not
 *
 *        Application should make sure Utils_resetSkipBufContext() once before
 *        first buffer is received
 *
 *        NOTE, the Utils_BufSkipContext is just a counter, actuall buffer
 *        pointer need not be a input to this function.
 *
 * \param skipCtx           [IN/OUT] Buffer skip context
 *
 * \return TRUE, skip this buffer \n
 *         FALSE, no not skip this buffer
 *
 *******************************************************************************
 */
Bool Utils_doSkipBuf(Utils_BufSkipContext *skipCtx )
{
    /*
     * if the target bufferrate has changed, first time case
     * needs to be visited?
     */
    if(skipCtx->firstTime)
    {
        skipCtx->outCnt = 0;
        skipCtx->inCnt = 0;

        skipCtx->multipleCnt = skipCtx->inputFrameRate * skipCtx->outputFrameRate;
        skipCtx->firstTime = FALSE;
    }

    if (skipCtx->inCnt > skipCtx->outCnt)
    {
        skipCtx->outCnt += skipCtx->outputFrameRate;
        /* skip this frame, return true */
        return TRUE;
    }

    /* out will also be multiple */
    if (skipCtx->inCnt == skipCtx->multipleCnt)
    {
        /* reset to avoid overflow */
        skipCtx->inCnt = skipCtx->outCnt = 0;
    }

    skipCtx->inCnt += skipCtx->inputFrameRate;
    skipCtx->outCnt += skipCtx->outputFrameRate;

    /* display this frame, hence return false */
    return FALSE;
}


/**
 *******************************************************************************
 *
 * \brief Return the application initialization state of the
 *        specified core
 *
 * \param coreId    [IN]  Id of the core
 * \param pState    [OUT] Application initialization state
 *
 * \return returns 0 on success
 *
 *******************************************************************************
 */
Int32 Utils_getAppInitState(int coreId, unsigned int *pState)
{
    Int32 status;

    status = RemoteLog_getAppInitState(coreId, pState);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Set the application initialization state of the
 *        specified core
 *
 * \param coreId    [IN] Core ID of the core to send the char
 * \param state     [IN] Value of the application intialization state to set
 *
 * \return returns 0 on success
 *
 *******************************************************************************
 */
Int32 Utils_setAppInitState(int coreId, unsigned int state)
{
    Int32 status;

    status = RemoteLog_setAppInitState(coreId, state);

    return status;
}

Void Utils_commonErrorRaiseHook(Error_Block *eb)
{
    Vps_printf("\n");
    Vps_printf(" ### XDC ASSERT - ERROR CALLBACK START ### \n");

    if (eb != NULL) {
        if (eb->msg) {
            Vps_printf("\n");
            Text_putSite(Error_getSite(eb), NULL, -1);
            if (Text_isLoaded) {
                Vps_printf(eb->msg, eb->data.arg[0], eb->data.arg[1]);
            }
            else {
                Vps_printf("error {id:0x%x, args:[0x%x, 0x%x]}",
                    eb->id, eb->data.arg[0], eb->data.arg[1]);
            }
            Vps_printf("\n");
        }
    }

    Vps_printf(" ### XDC ASSERT - ERROR CALLBACK END ### \n");
    Vps_printf("\n");

    Error_print(eb);
}
