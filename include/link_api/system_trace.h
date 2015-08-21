/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_TRACE_H_
#define _SYSTEM_TRACE_H_


#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <osal/bsp_osal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *******************************************************************************
 *
 *  \ingroup SYSTEM_LINK_API
 *  \defgroup SYSTEM_LINK_TRACE_API  System Link Trace, Debug, Utility APIs
 *
 *  Utility APIs for system tracing, debug and other function's
 *
 *  @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \file system_trace.h
 *  \brief  System Link Trace, Debug, Utility APIs
 *
 *******************************************************************************
 */


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Prints to Shared memory and CCS console
 *
 *  This function prints the provided formatted string to shared memory and CCS
 *  console
 *
 *  \param format       [IN] Formatted string followed by variable arguments
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success, else appropriate error code on
 *          failure.
 *
 *******************************************************************************
 */
Int32 Vps_printf(const char * format, ... );

#define UTILS_COMPILETIME_ASSERT(condition)                                     \
                   do {                                                         \
                       typedef char ErrorCheck[((condition) == TRUE) ? 1 : -1]; \
                   } while(0)

#define UTILS_ISERROR(errCode)                                  ((errCode) < 0)

#ifdef UTILS_ASSERT_ENABLE
// #define __KLOCWORK__
#ifdef __KLOCWORK__
// #warn "Using assert version for klocwork.Do not use binary on target"
#define UTILS_assert(y)                                                   \
do {                                                                      \
    if (!(y)) {                                                           \
        Vps_printf (" Assertion @ Line: %d in %s: %s : failed !!!\n",     \
                __LINE__, __FILE__, #y);                                  \
        abort();                                                          \
    }                                                                     \
} while (0)

#else
#if defined (BUILD_ARP32)
/* Use C assert. */
#define UTILS_assert(y)                                                   \
do {                                                                      \
    extern volatile Int g_AssertFailLoop;                                 \
    if (!(y)) {                                                           \
        Vps_printf   (" Assertion @ Line: %d in %s: %s : failed !!!\n",   \
                __LINE__, __FILE__, #y);                                  \
        System_printf(" Assertion @ Line: %d in %s: %s : failed !!!\n",   \
                __LINE__, __FILE__, #y);                                  \
        while(g_AssertFailLoop)                                           \
        {                                                                 \
            ;                                                             \
        }                                                                 \
    }                                                                     \
} while (0)
#else
/* Use C assert. */
#define UTILS_assert(y)                                                   \
do {                                                                      \
    extern volatile Int g_AssertFailLoop;                                 \
    if (!(y)) {                                                           \
        Vps_printf   (" Assertion @ Line: %d in %s: %s : failed !!!\n",   \
                __LINE__, __FILE__, #y);                                  \
        System_printf(" Assertion @ Line: %d in %s: %s : failed !!!\n",   \
                __LINE__, __FILE__, #y);                                  \
        while(g_AssertFailLoop)                                           \
        {                                                                 \
            BspOsal_sleep(1);                                             \
        }                                                                 \
    }                                                                     \
} while (0)
#endif
#endif
#else

#define UTILS_assert(y)

#endif

#ifdef UTILS_ASSERT_ENABLE
#define UTILS_assertError(condition, statusVar, errorCode, linkID, channelID)     \
do {                                                                              \
    if (!(condition)) {                                                           \
        statusVar = errorCode;                                                    \
        Vps_printf(" DEC_LINK ERR::linkID:%x::channelID:%d::"                     \
                     "errorCode:%d::FileName:%s::linuNum:%d::errorCondition:%s\n",  \
                     linkID, channelID, statusVar, __FILE__, __LINE__,#condition);  \
    }                                                                             \
} while(0)

#else

#define UTILS_assertError(condition, statusVar, errorCode, linkID, channelID)

#endif                                                     /* ifndef
                                                            * UTILS_ASSERT_ENABLE
                                                            */

/**
 *******************************************************************************
 *
 *  \brief Get current timestamp in msec
 *
 *  \return timestamp in msec
 *
 *******************************************************************************
 */
UInt64 Utils_getCurGlobalTimeInMsec();

/**
 *******************************************************************************
 *
 *  \brief Get current timestamp in usec
 *
 *  \return timestamp in msec
 *
 *******************************************************************************
 */
UInt64 Utils_getCurGlobalTimeInUsec();

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* ifndef
         * _SYSTEM_TRACE_H_
         */
/* @} */
