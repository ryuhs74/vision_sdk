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
 * \file utils_remote_log_server.c
 *
 * \brief  This file has the implementation for logging log information
 *
 *         This file has implementation for writing and reading data into/from
 *         a shared memory.
 * \version 0.0 (Jul 2013) : [HS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include <src/utils_common/include/utils_uart.h>
#include <include/link_api/system_common.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Max allowed parameter buffer size
 *
 *******************************************************************************
 */
#define REMOTE_LOG_SERVER_PRINT_BUF_LEN        (1024*4)

#define REMOTE_LOG_SERVER_IPU1_0_INST_START (0U)
#define REMOTE_LOG_SERVER_IPU1_0_INST_SIZE  (30*1024U)
#define REMOTE_LOG_SERVER_IPU1_0_INST_END   (REMOTE_LOG_SERVER_IPU1_0_INST_START +\
                                            REMOTE_LOG_SERVER_IPU1_0_INST_SIZE)

#define REMOTE_LOG_SERVER_IPU1_1_INST_START (REMOTE_LOG_SERVER_IPU1_0_INST_END)
#define REMOTE_LOG_SERVER_IPU1_1_INST_SIZE  (16*1024U)
#define REMOTE_LOG_SERVER_IPU1_1_INST_END   (REMOTE_LOG_SERVER_IPU1_1_INST_START +\
                                            REMOTE_LOG_SERVER_IPU1_1_INST_SIZE)

#define REMOTE_LOG_SERVER_A15_0_INST_START  (REMOTE_LOG_SERVER_IPU1_1_INST_END)
#define REMOTE_LOG_SERVER_A15_0_INST_SIZE   (16*1024U)
#define REMOTE_LOG_SERVER_A15_0_INST_END    (REMOTE_LOG_SERVER_A15_0_INST_START +\
                                            REMOTE_LOG_SERVER_A15_0_INST_SIZE)

#define REMOTE_LOG_SERVER_DSP1_INST_START   (REMOTE_LOG_SERVER_A15_0_INST_END)
#define REMOTE_LOG_SERVER_DSP1_INST_SIZE    (16*1024U)
#define REMOTE_LOG_SERVER_DSP1_INST_END     (REMOTE_LOG_SERVER_DSP1_INST_START +\
                                            REMOTE_LOG_SERVER_DSP1_INST_SIZE)

#define REMOTE_LOG_SERVER_DSP2_INST_START   (REMOTE_LOG_SERVER_DSP1_INST_END)
#define REMOTE_LOG_SERVER_DSP2_INST_SIZE    (16*1024U)
#define REMOTE_LOG_SERVER_DSP2_INST_END     (REMOTE_LOG_SERVER_DSP2_INST_START +\
                                            REMOTE_LOG_SERVER_DSP2_INST_SIZE)

#define REMOTE_LOG_SERVER_EVE1_INST_START   (REMOTE_LOG_SERVER_DSP2_INST_END)
#define REMOTE_LOG_SERVER_EVE1_INST_SIZE    (16*1024U)
#define REMOTE_LOG_SERVER_EVE1_INST_END     (REMOTE_LOG_SERVER_EVE1_INST_START +\
                                            REMOTE_LOG_SERVER_EVE1_INST_SIZE)

#define REMOTE_LOG_SERVER_EVE2_INST_START   (REMOTE_LOG_SERVER_EVE1_INST_END)
#define REMOTE_LOG_SERVER_EVE2_INST_SIZE    (16*1024U)
#define REMOTE_LOG_SERVER_EVE2_INST_END     (REMOTE_LOG_SERVER_EVE2_INST_START +\
                                            REMOTE_LOG_SERVER_EVE2_INST_SIZE)

#define REMOTE_LOG_SERVER_EVE3_INST_START   (REMOTE_LOG_SERVER_EVE2_INST_END)
#define REMOTE_LOG_SERVER_EVE3_INST_SIZE    (16*1024U)
#define REMOTE_LOG_SERVER_EVE3_INST_END     (REMOTE_LOG_SERVER_EVE3_INST_START +\
                                            REMOTE_LOG_SERVER_EVE3_INST_SIZE)

#define REMOTE_LOG_SERVER_EVE4_INST_START   (REMOTE_LOG_SERVER_EVE3_INST_END)
#define REMOTE_LOG_SERVER_EVE4_INST_SIZE    (16*1024U)
#define REMOTE_LOG_SERVER_EVE4_INST_END     (REMOTE_LOG_SERVER_EVE4_INST_START +\
                                            REMOTE_LOG_SERVER_EVE4_INST_SIZE)

/** \brief Guard macro */
#if (REMOTE_LOG_SERVER_EVE4_INST_END > REMOTE_LOG_LOG_BUF_SIZE)
    #error "Increase REMOTE_LOG_LOG_BUF_SIZE in file utils_remote_log_if.h"
#endif

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure for remote log object (local).
 *
 *          This structure is used by a remote core to store information about
 *          log buffer.
 *
 *******************************************************************************
 */
typedef struct
{
    unsigned int coreId;
    /**< Used to identify core */
    char printBuf[REMOTE_LOG_SERVER_PRINT_BUF_LEN];
    /**< local buffer which can hold one line of log */
} RemoteLog_ServerObj;

/**
 *******************************************************************************
 *
 *  \brief  Global array for remote log object structure, one for each core.
 *
 *
 *******************************************************************************
 */




#ifndef BUILD_A15
#pragma DATA_SECTION(gRemoteLog_coreObj,".bss:extMemNonCache:remoteLogCoreShm");
#pragma DATA_ALIGN(gRemoteLog_coreObj, 4);
#endif
RemoteLog_CoreObj gRemoteLog_coreObj
#ifdef BUILD_A15
__attribute__ ((section(".bss:extMemNonCache:remoteLogCoreShm")))
__attribute__ ((aligned(4)))
#endif
;



/**
 *******************************************************************************
 *
 *  \brief  Global remote log server object
 *
 *
 *******************************************************************************
 */
RemoteLog_ServerObj gRemoteLog_serverObj;

RemoteLog_ServerIndexInfo gRemoteLog_ServerIdxInfo[SYSTEM_PROC_MAX] =
{
    {REMOTE_LOG_SERVER_IPU1_0_INST_START, REMOTE_LOG_SERVER_IPU1_0_INST_SIZE},
    {REMOTE_LOG_SERVER_IPU1_1_INST_START, REMOTE_LOG_SERVER_IPU1_1_INST_SIZE},
    {REMOTE_LOG_SERVER_A15_0_INST_START, REMOTE_LOG_SERVER_A15_0_INST_SIZE},
    {REMOTE_LOG_SERVER_DSP1_INST_START, REMOTE_LOG_SERVER_DSP1_INST_SIZE},
    {REMOTE_LOG_SERVER_DSP2_INST_START, REMOTE_LOG_SERVER_DSP2_INST_SIZE},
    {REMOTE_LOG_SERVER_EVE1_INST_START, REMOTE_LOG_SERVER_EVE1_INST_SIZE},
    {REMOTE_LOG_SERVER_EVE2_INST_START, REMOTE_LOG_SERVER_EVE2_INST_SIZE},
    {REMOTE_LOG_SERVER_EVE3_INST_START, REMOTE_LOG_SERVER_EVE3_INST_SIZE},
    {REMOTE_LOG_SERVER_EVE4_INST_START, REMOTE_LOG_SERVER_EVE4_INST_SIZE}
};


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Put a string into shared memory
 *
 * \param  coreId   [IN] Id of the core to log
 * \param  pString  [IN] String to put into shared memory

 * \return  returns 0 on success
 *
 *******************************************************************************
 */
static int RemoteLog_serverPutString(unsigned int coreId, char *pString)
{
    volatile unsigned int maxBytes, numBytes, copyBytes, serverIdx, clientIdx;
    volatile unsigned char *pDst;
    RemoteLog_ServerIndexInfo *pIdxInfo;
    RemoteLog_CoreObj *pCoreObj;

    if (coreId >= SYSTEM_PROC_MAX)
        return -1;

    pCoreObj = &gRemoteLog_coreObj;
    pIdxInfo = &gRemoteLog_ServerIdxInfo[coreId];

    if (pCoreObj->memInfo[coreId].headerTag != REMOTE_LOG_HEADER_TAG)
        return -1;

    numBytes = strlen(pString);

    if (numBytes <= 0)
        return -1;

    serverIdx = pCoreObj->memInfo[coreId].serverIdx;
    clientIdx = pCoreObj->memInfo[coreId].clientIdx;

    if (serverIdx < clientIdx)
        maxBytes = clientIdx - serverIdx;
    else
        maxBytes = (pIdxInfo->size - serverIdx) + clientIdx;

    if (numBytes > maxBytes)
        return -1;

    pDst = &pCoreObj->serverLogBuf[pIdxInfo->startIdx];

    for (copyBytes = 0; copyBytes < numBytes; copyBytes++)
    {
        if (serverIdx >= pIdxInfo->size)
            serverIdx = 0;

        pDst[serverIdx] = *pString++;
        serverIdx++;
    }

    if (serverIdx >= pIdxInfo->size)
        serverIdx = 0;

    pDst[serverIdx] = 0;
    serverIdx++;

    pCoreObj->memInfo[coreId].serverIdx = serverIdx;

    /* dummy read to resure data is written to memory */
    serverIdx = pCoreObj->memInfo[coreId].serverIdx;

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Put variable arguments into shared memory
 *        Provides a C style printf which puts variable arguments as a string
 *        into a shared memory
 *
 * \param  format   [IN] variable argument list
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
Int32 Vps_printf(const char *format, ...)
{
    int retVal;
    va_list vaArgPtr;
    char *buf = NULL;
    UInt32 cookie;
    UInt32 strnlen = 0;
    UInt64 timeInUsec;


    cookie = Hwi_disable();

    buf = &gRemoteLog_serverObj.printBuf[0];

    timeInUsec = (UInt32)Utils_getCurGlobalTimeInUsec();
    strnlen = sprintf(buf, "%6d.%06u s: ",
                (unsigned int)timeInUsec/1000000,
                (unsigned int)timeInUsec%1000000
              );

    va_start(vaArgPtr, format);
    vsnprintf(buf + strnlen,
              REMOTE_LOG_SERVER_PRINT_BUF_LEN,
              format, vaArgPtr);
    va_end(vaArgPtr);

    retVal = RemoteLog_serverPutString(gRemoteLog_serverObj.coreId, buf);

    Hwi_restore(cookie);

    if (BIOS_getThreadType() == BIOS_ThreadType_Task)
    {
        /* Printf should be called only from Task context as it does pend.
         * Calling from other context will cause exception
         */
#ifndef ENABLE_UART
        System_printf(buf);
#endif
    }

    return (retVal);
}


int RemoteLog_init()
{
    volatile RemoteLog_CoreObj *pCoreObj;
    UInt32 coreId = System_getSelfProcId();

    if (coreId >= SYSTEM_PROC_MAX)
        return -1;

    pCoreObj = &gRemoteLog_coreObj;
    pCoreObj->memInfo[coreId].headerTag = REMOTE_LOG_HEADER_TAG;
    pCoreObj->memInfo[coreId].serverIdx = 0;
    pCoreObj->memInfo[coreId].clientIdx = 0;
    pCoreObj->memInfo[coreId].appInitState = CORE_APP_INITSTATUS_PRE_INIT;

    gRemoteLog_serverObj.coreId = coreId;


    return 0;
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
int RemoteLog_setAppInitState(int coreId, unsigned int state)
{
    volatile RemoteLog_CoreObj *pCoreObj;

    if (coreId >= SYSTEM_PROC_MAX)
        return -1;

    pCoreObj = &gRemoteLog_coreObj;
    pCoreObj->memInfo[coreId].appInitState = state;

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Return the application initialization state of the
 *        specified core
 *
 * \param pState    [OUT] Application initialization state
 * \param coreId    [IN]  Id of the core
 *
 * \return returns 0 on success
 *
 *******************************************************************************
 */
int RemoteLog_getAppInitState(int coreId, unsigned int *pState)
{
    volatile RemoteLog_CoreObj *pCoreObj;

    if (coreId >= SYSTEM_PROC_MAX)
        return -1;

    pCoreObj = &gRemoteLog_coreObj;
    *pState = pCoreObj->memInfo[coreId].appInitState;

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Return Index information for the given core
 *
 * \param coreId    [IN]  Id of the core
 *
 * \return returns 0 on success
 *
 *******************************************************************************
 */
RemoteLog_ServerIndexInfo *RemoteLog_getCoreIdxInfo(int coreId)
{
    RemoteLog_ServerIndexInfo *pIdxInfo = NULL;

    if (coreId < SYSTEM_PROC_MAX)
    {
        pIdxInfo = &gRemoteLog_ServerIdxInfo[coreId];
    }

    return pIdxInfo;
}

