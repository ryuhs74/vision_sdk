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
 * \file system_ipc.c
 *
 * \brief  Wrapper around system rpmsg components calls.
 *
 *         This file implements wrapper around rpmsg calls.
 *
 * \version 0.0 (May 2014) : [YM] First version implemented
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"
#include <sys/types.h>

#define __USE_GNU
#include <sys/socket.h>
#include <errno.h>

/* Ipc Socket Protocol Family */
#include <net/rpmsg.h>

System_IpcSharedMemObj  *gSystem_ipcSharedMemObj;

/**
 *******************************************************************************
 *
 * \brief Wrapper to initialize IPC sub-system.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_ipcInit()
{
    Int32 status;

    Vps_printf(" SYSTEM: IPC: Init in progress !!!\n");

    gSystem_ipcSharedMemObj = (System_IpcSharedMemObj*)
                                OSA_memPhys2Virt(
                                    SYSTEM_IPC_SHM_MEM_ADDR,
                                    OSA_MEM_REGION_TYPE_SYSTEM_IPC
                                );
    OSA_assert(gSystem_ipcSharedMemObj!=NULL);

    status = System_ipcNotifyInit();
    OSA_assertSuccess(status);

    status = System_ipcMsgQInit();
    OSA_assertSuccess(status);

    Vps_printf(" SYSTEM: IPC: Init DONE !!!\n");

    return SYSTEM_LINK_STATUS_SOK;
}
/**
 *******************************************************************************
 *
 * \brief Wrapper to de-initialize IPC sub-system.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_ipcDeInit()
{
    Vps_printf(" SYSTEM: IPC: De-init in progress !!!\n");

    System_ipcMsgQDeInit();
    System_ipcNotifyDeInit();

    Vps_printf(" SYSTEM: IPC: De-init DONE !!!\n");

    return SYSTEM_LINK_STATUS_SOK;
}

Bool System_isProcEnabled(UInt32 procId)
{
     UInt32 i;
     Bool isEnabled;

     i = 0;
     isEnabled = FALSE;

     while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
     {
         if(procId == gSystem_ipcEnableProcId[i])
         {
             isEnabled = TRUE;
             break;
         }

         i++;
     }

     return isEnabled;
}

char *System_getProcName(UInt32 procId)
{
    if(procId==SYSTEM_PROC_DSP1)
        return SYSTEM_IPC_PROC_NAME_DSP1;

    if(procId==SYSTEM_PROC_DSP2)
        return SYSTEM_IPC_PROC_NAME_DSP2;

    if(procId==SYSTEM_PROC_EVE1)
        return SYSTEM_IPC_PROC_NAME_EVE1;

    if(procId==SYSTEM_PROC_EVE2)
        return SYSTEM_IPC_PROC_NAME_EVE2;

    if(procId==SYSTEM_PROC_EVE3)
        return SYSTEM_IPC_PROC_NAME_EVE3;

    if(procId==SYSTEM_PROC_EVE4)
        return SYSTEM_IPC_PROC_NAME_EVE4;

    if(procId==SYSTEM_PROC_IPU1_0)
        return SYSTEM_IPC_PROC_NAME_IPU1_0;

    if(procId==SYSTEM_PROC_IPU1_1)
        return SYSTEM_IPC_PROC_NAME_IPU1_1;

    if(procId==SYSTEM_PROC_A15_0)
        return SYSTEM_IPC_PROC_NAME_A15_0;

    return SYSTEM_IPC_PROC_NAME_INVALID;
}


Int32 System_ipcCreateChannel(UInt32 procId, UInt32 endpt, UInt32 channelType)
{
    struct sockaddr_rpmsg rpmsg_addr;
    socklen_t len;
    UInt32 syslinkProcId = System_getSyslinkProcId(procId);
    Int32 err = 0;
    Int32 fd  = 0;

    /* create an RPMSG socket */
    fd = socket(AF_RPMSG, SOCK_SEQPACKET, 0);
    if (fd < 0) {
        Vps_printf(" SYSTEM: IPC: [%s] socket open failed (%s, %d) !!!\n",
                System_getProcName(procId), strerror(errno), errno);
        return -1;
    }

    /* Based on channelType bind or connect the socket */
    memset(&rpmsg_addr, 0, sizeof(rpmsg_addr));
    rpmsg_addr.family     = AF_RPMSG;
    rpmsg_addr.vproc_id   = _MultiProc_cfg.rprocList[syslinkProcId];
    rpmsg_addr.addr       = endpt;

    if(channelType == SYSTEM_RPMSG_RX_CHANNEL)
    {
        len = sizeof(struct sockaddr_rpmsg);
        err = bind(fd, (struct sockaddr *)&rpmsg_addr, len);
        if (err >= 0) {
            Vps_printf(" SYSTEM: IPC: [%s] socket bind success !!!"
                    " (dst vproc = %d, endpt = %d)\n",
                    System_getProcName(procId),
                    rpmsg_addr.vproc_id,
                    rpmsg_addr.addr);
        }
        else
        {
            Vps_printf(" SYSTEM: IPC: [%s] socket bind failed (%s, %d) !!!\n",
                System_getProcName(procId), strerror(errno), errno);

            return -1;
        }
    }
    else if(channelType == SYSTEM_RPMSG_TX_CHANNEL)
    {
        len = sizeof(struct sockaddr_rpmsg);
        err = connect(fd, (struct sockaddr *)&rpmsg_addr, len);
        if(err>=0)
        {
            Vps_printf(" SYSTEM: IPC: [%s] socket connect success !!!"
                        " (dst vproc = %d, endpt = %d)\n",
                        System_getProcName(procId),
                        rpmsg_addr.vproc_id,
                        rpmsg_addr.addr);
        }
        else
        {
            Vps_printf(" SYSTEM: IPC: [%s] socket connect failed (%s, %d) !!!\n",
                System_getProcName(procId), strerror(errno), errno);
            return -1;
        }
    }
    else
    {
        OSA_assert(0);
    }

    /* let's see what local address we got, this will be required to validate source
     * of the received message on remote end to avoid acting on any malicious message.
     */
    err = getsockname(fd, (struct sockaddr *)&rpmsg_addr, &len);
    if (err < 0) {
        Vps_printf(" SYSTEM: IPC: [%s] getsockname failed (%s, %d) !!!\n",
            System_getProcName(procId), strerror(errno), errno);
        return -1;
    }

    Vps_printf(" SYSTEM: IPC: [%s] socket info "
               "(family = %d, dst proc id = %d, endpt = %d) !!!",
            System_getProcName(procId),
            rpmsg_addr.family,
            rpmsg_addr.vproc_id,
            rpmsg_addr.addr);

    return fd;

}

Int32 System_ipcDeleteChannel(Int32 fd)
{
    Int32 err = 0;

    err = close(fd);

    if(err < 0)
    {
        Vps_printf(" SYSTEM: IPC: socket close failed (%s, %d) !!!\n",
                strerror(errno), errno);
    }

    return err;
}

UInt32 System_getSyslinkProcId(UInt32 procId)
{
    char *procName = System_getProcName(procId);

    if(strcmp(procName, SYSTEM_IPC_PROC_NAME_INVALID)!=0)
    {
        return MultiProc_getId(procName);
    }

    return MultiProc_INVALIDID;
}

System_IpcMsg *System_ipcGetMsg(uint32_t procId)
{
    System_IpcSharedMemObj *pObj = gSystem_ipcSharedMemObj;
    System_IpcMsg *pPrm = NULL;

    if(procId < SYSTEM_PROC_MAX)
    {
        pPrm = &pObj->ipcMsgObj.procMsg[procId];
    }

    return pPrm;
}

System_IpcQueObj *System_ipcGetIpcOut2InQue(uint32_t ipcOutLinkId)
{
    System_IpcSharedMemObj *pObj = gSystem_ipcSharedMemObj;
    System_IpcQueObj *pPrm = NULL;
    UInt32 procId = SYSTEM_GET_PROC_ID(ipcOutLinkId);
    UInt32 linkId = SYSTEM_GET_LINK_ID(ipcOutLinkId);
    UInt32 linkInstId;

    if(     procId < SYSTEM_PROC_MAX
        &&  linkId < SYSTEM_LINK_ID_MAX
        &&  (Int32)linkId >= SYSTEM_LINK_ID_IPC_OUT_0
        )
    {
        linkInstId = linkId - SYSTEM_LINK_ID_IPC_OUT_0;

        if(linkInstId < SYSTEM_IPC_OUT_LINK_MAX)
        {
            pPrm =
                &pObj->ipcQueObj.ipcQueProcObj[procId].queOut2InObj[linkInstId];
        }
    }

    return pPrm;
}

System_IpcQueObj *System_ipcGetIpcIn2OutQue(uint32_t ipcOutLinkId)
{
    System_IpcSharedMemObj *pObj = gSystem_ipcSharedMemObj;
    System_IpcQueObj *pPrm = NULL;
    UInt32 procId = SYSTEM_GET_PROC_ID(ipcOutLinkId);
    UInt32 linkId = SYSTEM_GET_LINK_ID(ipcOutLinkId);
    UInt32 linkInstId;

    if(     procId < SYSTEM_PROC_MAX
        &&  linkId < SYSTEM_LINK_ID_MAX
        &&  (Int32)linkId >= SYSTEM_LINK_ID_IPC_OUT_0
        )
    {
        linkInstId = linkId - SYSTEM_LINK_ID_IPC_OUT_0;

        if(linkInstId < SYSTEM_IPC_OUT_LINK_MAX)
        {
            pPrm =
                &pObj->ipcQueObj.ipcQueProcObj[procId].queIn2OutObj[linkInstId];
        }
    }

    return pPrm;
}

System_IpcBuffer *System_ipcGetIpcBuffer(uint32_t ipcOutLinkId, uint32_t index)
{
    System_IpcSharedMemObj *pObj = gSystem_ipcSharedMemObj;
    System_IpcBuffer *pPrm = NULL;
    UInt32 procId = SYSTEM_GET_PROC_ID(ipcOutLinkId);
    UInt32 linkId = SYSTEM_GET_LINK_ID(ipcOutLinkId);
    UInt32 linkInstId;

    if(     procId < SYSTEM_PROC_MAX
        &&  linkId < SYSTEM_LINK_ID_MAX
        &&  index  < SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS
        &&  (Int32)linkId >= SYSTEM_LINK_ID_IPC_OUT_0
        )
    {
        linkInstId = linkId - SYSTEM_LINK_ID_IPC_OUT_0;

        if(linkInstId < SYSTEM_IPC_OUT_LINK_MAX)
        {
            pPrm =
                &pObj->ipcQueObj.ipcQueProcObj[procId].queElements[linkInstId][index];
        }
    }

    return pPrm;
}


UInt32 System_ipcMemAlloc(UInt32 heapId, UInt32 memSize, UInt32 memAlign)
{
    Int32 status;
    SystemCommon_AllocBuffer prms;

    prms.heapId = heapId;
    prms.size   = memSize;
    prms.align  = memAlign;

    status = System_linkControl(SYSTEM_LINK_ID_IPU1_0,
                                SYSTEM_COMMON_CMD_ALLOC_BUFFER,
                                (Void *)&prms,
                                sizeof(SystemCommon_AllocBuffer),
                                TRUE);


    OSA_assert((UInt32 *)prms.bufferPtr != NULL);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    return (prms.bufferPtr);
}


Void System_ipcMemFree(UInt32 heapId, UInt32 addr, UInt32 memSize)
{
    Int32 status;
    SystemCommon_FreeBuffer prms;

    prms.heapId    = heapId;
    prms.size      = memSize;
    prms.bufferPtr = addr;

    status = System_linkControl(SYSTEM_LINK_ID_IPU1_0,
                                SYSTEM_COMMON_CMD_FREE_BUFFER,
                                (Void *)&prms,
                                sizeof(SystemCommon_FreeBuffer),
                                TRUE);

    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
}
