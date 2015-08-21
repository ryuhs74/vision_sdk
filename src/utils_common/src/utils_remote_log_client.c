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
 * \file utils_remote_log_client.c
 *
 * \brief  This file implements remote log client.
 *
 *         This file reads the data from shared memory between host CPU and
 *         all remote cores. It prints the data to console.
 *
 * \version 0.1 (Jul 2013) : [HS] First version
 *
 *******************************************************************************
*/
/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "utils_remote_log_client.h"
#include <src/utils_common/include/utils.h>
#include <src/utils_common/include/utils_uart.h>
#include <include/link_api/system_common.h>
#include <src/links_common/system/system_priv_common.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 * \brief Task Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gRemoteLogClient_tskStack, 32)
#pragma DATA_SECTION(gRemoteLogClient_tskStack, ".bss:taskStackSection")
static UInt8 gRemoteLogClient_tskStack[REMOTE_LOG_CLIENT_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Global remote log client object
 *
 *******************************************************************************
*/
static RemoteLog_ClientObj gRemoteLog_clientObj;

/**
 *******************************************************************************
 *
 * \brief Assign the physical address of the buffer to client object
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
static Int32 RemoteLog_mapMem()
{
    /* Mmap is not required since client is on M4, phy and virt address are
     * same
     */
    gRemoteLog_clientObj.coreObjVirtBaseAddr = (unsigned char *)
                gRemoteLog_clientObj.coreObjPhysBaseAddr;

    if (gRemoteLog_clientObj.coreObjVirtBaseAddr==NULL)
    {
        printf(" ERROR: mmap() failed !!!\n");
        return -1;
    }

  return 0;
}

/**
 *******************************************************************************
 *
 * \brief Remote log client periodic call back function
 *
 *       This function is a periodic callback function for remote log client.
 *       This function post semaphores on which actual function which reads
 *       from remote buffers and print on uart gets unblocked
 *
 * \param   arg     [IN] Remote log client object
 *
 * \return  None
 *
 *******************************************************************************
 */
static Void remoteLogClientPrdFunc(UArg arg)
{
    /* Post semaphore so that  function which prints from the
     * the shared buffer gets scheduled
     */
    BspOsal_semPost(gRemoteLog_clientObj.lock);


}

/**
 *******************************************************************************
 *
 * \brief Get the line of buffer from array of lines.
 *
 * \param  coreId    [IN] Id of the core to log
 * \param  pString   [IN] String to put into shared memory
 * \param  strSize   [OUT] Size of string copied from log buffer
 *
 *
 * \return  returns number of bytes extracted form the log buffer
 *
 *******************************************************************************
 */
static Int32 RemoteLog_clientGetLine(UInt32 coreId, char * pString,
                UInt32 *strSize)
{
    volatile UInt32 numBytes, copyBytes=0, serverIdx, clientIdx;
    volatile unsigned char *pSrc;
    volatile unsigned char curChar;
    volatile RemoteLog_MemInfo *pMemInfo =
                gRemoteLog_clientObj.pMemInfo[coreId];
    RemoteLog_ServerIndexInfo *idxInfo = NULL;

    if(pMemInfo->headerTag != REMOTE_LOG_HEADER_TAG)
        return 0;

    idxInfo = RemoteLog_getCoreIdxInfo(coreId);
    if (NULL == idxInfo)
        return 0;

    serverIdx = pMemInfo->serverIdx;
    clientIdx = pMemInfo->clientIdx;

    if(clientIdx>serverIdx)
        numBytes = (idxInfo->size - clientIdx) + serverIdx;
    else
        numBytes = serverIdx - clientIdx;

    if(numBytes>0)
    {
        pSrc = &gRemoteLog_clientObj.pServerLogBuf[idxInfo->startIdx];

        for(copyBytes=0; copyBytes<numBytes; copyBytes++)
        {
          if(clientIdx>=idxInfo->size)
            clientIdx = 0;

          curChar = pSrc[clientIdx];

          clientIdx++;

          if(curChar==0xA0 || curChar=='\r' ||
             curChar=='\n' ||curChar==0     ||
             copyBytes >= idxInfo->size)
            break;
          else
            *pString++ = curChar;
        }

        pMemInfo->clientIdx = clientIdx;

        /* dummy read to resure data is written to memory */
        clientIdx = pMemInfo->clientIdx;
    }

    *pString = 0;
    *strSize = copyBytes;

    return numBytes;
}

/**
 *******************************************************************************
 *
 * \brief Run the periodic task for printing data from shared memory
 *
 * \param  arg0     [IN]  Not used
 * \param  arg1     [IN] Not used

 * \return  returns 0 on success
 *
 *******************************************************************************
 */
static Void RemoteLog_clientRun(UArg arg0, UArg arg1)
{
    UInt32 coreId;
    UInt32 numBytes, strSize;
    char procName[16];

    while (1)
    {
        BspOsal_semWait(gRemoteLog_clientObj.lock,
                       BSP_OSAL_WAIT_FOREVER);

        #ifdef ENABLE_UART
        if(!System_isUartInitDone())
            continue;
        #endif

        for(coreId=0; coreId<SYSTEM_PROC_MAX; coreId++)
        {
            if(System_isProcEnabled(coreId))
            {
                snprintf(procName, 16, "[%-6s] ", System_getProcName(coreId));
                do {
                    numBytes = RemoteLog_clientGetLine(coreId,
                            gRemoteLog_clientObj.lineBuf,
                            &strSize );
                    if(strSize>0)
                    {
                        uartPrint(procName);
                        uartPrint(gRemoteLog_clientObj.lineBuf);
                        uartPrint("\r\n");

                        #if 0
                        {
                            /* WORKAROUND: Uncomment this is UART does not work on EVM
                             */
                            System_printf("%s%s\r\n",
                                procName,
                                gRemoteLog_clientObj.lineBuf
                            );
                        }
                        #endif
                    }
                }
                while(numBytes);
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief Recursively convert ascii to hex.
 *
 * \param  c        [IN]  Character
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
static char xtod(char c) {
  if (c>='0' && c<='9') return c-'0';
  if (c>='A' && c<='F') return c-'A'+10;
  if (c>='a' && c<='f') return c-'a'+10;
  return c=0;        /* not Hex digit */
}
/**
 *******************************************************************************
 *
 * \brief Recursively convert ascii to hex.
 *
 * \param  l        [IN] Integer
 * \param  hex      [OUT] hex

 * \return  returns 0 on success
 *
 *******************************************************************************
 */
static Int32 HextoDec(char *hex, Int32 l)
{
  if (*hex==0)
    return(l);

  return HextoDec(hex+1, l*16+xtod(*hex)); // hex+1?
}

/**
 *******************************************************************************
 *
 * \brief Initializes the remote log client.
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
Int32 Utils_remoteLogClientInit()
{
    UInt32 coreId;
    Int32 status;
    volatile RemoteLog_MemInfo *pMemInfo;
    RemoteLog_ClientObj *pObj;
    BspOsal_TaskHandle tsk;
    volatile UInt8 *pStartPtr;
    RemoteLog_ServerIndexInfo *idxInfo;

    memset(&gRemoteLog_clientObj, 0, sizeof(gRemoteLog_clientObj));

    gRemoteLog_clientObj.coreObjPhysBaseAddr =
                                    (UInt32)&gRemoteLog_coreObj;
    gRemoteLog_clientObj.coreObjTotalMemSize =
                        sizeof(RemoteLog_CoreObj);

    status = RemoteLog_mapMem();
    if(status != 0)
        return -1;

    pStartPtr = (volatile UInt8 *)(
                    (unsigned int)gRemoteLog_clientObj.coreObjVirtBaseAddr +
                    sizeof(RemoteLog_MemInfo)*SYSTEM_PROC_MAX);
    for(coreId=0; coreId<SYSTEM_PROC_MAX; coreId++)
    {
        gRemoteLog_clientObj.pMemInfo[coreId] =
            (RemoteLog_MemInfo*)(gRemoteLog_clientObj.coreObjVirtBaseAddr +
                sizeof(RemoteLog_MemInfo)*coreId);

        idxInfo = RemoteLog_getCoreIdxInfo(coreId);
        if (NULL == idxInfo)
            continue;

        pMemInfo = gRemoteLog_clientObj.pMemInfo[coreId];

        pMemInfo->serverIdx = 0;
        pMemInfo->clientIdx = 0;
        pMemInfo->headerTag = REMOTE_LOG_HEADER_TAG;

        printf(" %s Remote Log Shared Memory @ 0x%08x\n",
            System_getProcName(coreId),
            pStartPtr +
            idxInfo->startIdx);
    }
    gRemoteLog_clientObj.pServerLogBuf =
        gRemoteLog_clientObj.coreObjVirtBaseAddr +
                sizeof(RemoteLog_MemInfo)*SYSTEM_PROC_MAX;

    pObj = &gRemoteLog_clientObj;

    pObj->lock = BspOsal_semCreate(1u, TRUE);
    UTILS_assert(pObj->lock != NULL);

    pObj->prd.clkHandle = BspOsal_clockCreate(
                            (BspOsal_ClockFuncPtr)remoteLogClientPrdFunc,
                            REMOTE_LOG_CLIENT_PERIOD_MS,
                            FALSE,
                            pObj
                            );
    UTILS_assert(pObj->prd.clkHandle!=NULL);
    pObj->prd.clkStarted = FALSE;

    if(pObj->prd.clkStarted == FALSE)
    {
        BspOsal_clockStart(pObj->prd.clkHandle);
        pObj->prd.clkStarted = TRUE;
    }

    /* Create task for printing remote core and local core messages from
     * buffer
     */
    tsk = BspOsal_taskCreate(
                    (BspOsal_TaskFuncPtr)RemoteLog_clientRun,
                    REMOTE_LOG_CLIENT_TSK_PRI,
                    gRemoteLogClient_tskStack,
                    sizeof(gRemoteLogClient_tskStack),
                    NULL
                );
    UTILS_assert(tsk != NULL);

    return SYSTEM_LINK_STATUS_SOK;
}

