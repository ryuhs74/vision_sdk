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
 * \defgroup REMOTE_LOG_CLIENT    Remote log implementation
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file Remote log client  header file.
 *
 * \brief  Internal header file for remote log client
 *
 * \version 0.1 (Jul 2013) : [HS] First version
 *
 *******************************************************************************
 */

#ifndef _REMOTE_LOG_CLIENT_
#define _REMOTE_LOG_CLIENT_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <osa.h>
#include <osa_mem.h>
#include <osa_thr.h>
#include <osa_remote_log_if.h>

/**
 *******************************************************************************
 *
 * \brief Period for remote log client task to run
 *
 *******************************************************************************
 */
#define REMOTE_LOG_CLIENT_PERIOD_MS           (10u)



/**
 *******************************************************************************
 *
 * \brief Line buffer size for remote shared buffer
 *
 *******************************************************************************
 */
#define REMOTE_LOG_LINE_BUF_SIZE         (1024)



/**
 *******************************************************************************
 *
 *   \brief Remote log client object
 *
 *         Contains fields required for remote log client
 *
 *******************************************************************************
*/
typedef struct {

    unsigned int coreObjPhysBaseAddr;
    /**< Physical address of the memory to which all remote and
     *   local core writes for printing it on  uart on local
     *   core
     */
    unsigned char *coreObjVirtBaseAddr;
    /**< Virtual address of the above physical memory. For non
     *   Linux system this  will be same as physical address
     */
    unsigned int coreObjTotalMemSize;
    /**< Size of the the shared memory */

    volatile RemoteLog_MemInfo *pMemInfo[SYSTEM_PROC_MAX];
    /**< Remote log server memory Information for the remote cores. */

    volatile unsigned char *pServerLogBuf;
    /**< Pointer to Server Log Buf memory */

    char lineBuf[REMOTE_LOG_LINE_BUF_SIZE];
    /**< Temporary buffer for print on uart from shared buffer */

    OSA_ThrHndl thrHndl;
    /**< Log print thread */

    volatile int exitThr;
    /**< flag to exit thread */

} RemoteLog_ClientObj;


#endif /* _REMOTE_LOG_CLIENT_ */
