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
 * \file remote_log_client.c
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
#include "osa_remote_log_client.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
#define OSA_DEBUG_REMOTE_LOG

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
                OSA_memPhys2Virt(gRemoteLog_clientObj.coreObjPhysBaseAddr,
                                OSA_MEM_REGION_TYPE_REMOTE_LOG
                                );

    if (gRemoteLog_clientObj.coreObjVirtBaseAddr==0)
    {
        printf(" OSA: REMOTELOG: ERROR: mmap() failed !!!\n");
        return -1;
    }

    return 0;
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
static void *RemoteLog_clientRun(void *pPrm)
{
    UInt32 coreId;
    UInt32 numBytes, strSize;
    char procName[16];

    while (1)
    {
        OSA_waitMsecs(REMOTE_LOG_CLIENT_PERIOD_MS);

        if(gRemoteLog_clientObj.exitThr==1)
            break;

        for(coreId=0; coreId<SYSTEM_PROC_MAX; coreId++)
        {
            if(System_isProcEnabled(coreId))
            {
                sprintf(procName, "[%-6s] ", System_getProcName(coreId));
                do {
                    strSize = 0;
                    numBytes = RemoteLog_clientGetLine(coreId,
                            gRemoteLog_clientObj.lineBuf,
                            &strSize );
                    if(strSize>0)
                    {
                        printf( "%s%s\r\n", procName, gRemoteLog_clientObj.lineBuf);
                    }
                }
                while(numBytes);
            }
        }
    }

    gRemoteLog_clientObj.exitThr = 2;

    return NULL;
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
Int32 OSA_remoteLogClientInit()
{
    UInt32 coreId;
    Int32 status;

    memset(&gRemoteLog_clientObj, 0, sizeof(gRemoteLog_clientObj));

    gRemoteLog_clientObj.coreObjPhysBaseAddr =
                                    (UInt32)REMOTE_LOG_MEM_ADDR;

    gRemoteLog_clientObj.coreObjTotalMemSize =
                        sizeof(RemoteLog_CoreObj);

    status = RemoteLog_mapMem();
    if(status != 0)
        return -1;

    for(coreId=0; coreId<SYSTEM_PROC_MAX; coreId++)
    {
        gRemoteLog_clientObj.pMemInfo[coreId] =
            (RemoteLog_MemInfo*)(gRemoteLog_clientObj.coreObjVirtBaseAddr +
                sizeof(RemoteLog_MemInfo)*coreId);

        #ifdef OSA_DEBUG_REMOTE_LOG
        if(System_isProcEnabled(coreId))
        {
            printf(" OSA: %s Remote Log Shared Memory @ 0x%08x\n",
                System_getProcName(coreId),
                (unsigned int)gRemoteLog_clientObj.coreObjPhysBaseAddr +
                sizeof(RemoteLog_CoreObj)*coreId);
        }
        #endif

    }
    gRemoteLog_clientObj.pServerLogBuf =
        gRemoteLog_clientObj.coreObjVirtBaseAddr +
                sizeof(RemoteLog_MemInfo)*SYSTEM_PROC_MAX;

    /* Create task for printing remote core and local core messages from
     * buffer
     */

    OSA_thrCreate(  &gRemoteLog_clientObj.thrHndl,
                    RemoteLog_clientRun,
                    OSA_THR_PRI_DEFAULT,
                    OSA_THR_STACK_SIZE_DEFAULT,
                    NULL
                );

    return SYSTEM_LINK_STATUS_SOK;
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
Int32 OSA_remoteLogClientDeInit()
{
    /* Create task for printing remote core and local core messages from
     * buffer
     */

    gRemoteLog_clientObj.exitThr = 1;

    while(gRemoteLog_clientObj.exitThr!=2)
        OSA_waitMsecs(1);

    OSA_thrDelete(  &gRemoteLog_clientObj.thrHndl );

    return SYSTEM_LINK_STATUS_SOK;
}


