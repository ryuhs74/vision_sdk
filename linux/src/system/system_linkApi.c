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
 * \file system_linkApi.c
 *
 * \brief  This file implements wrapper around few of link APIs.
 *
 *          This file implements wrapper around few of link APIs. It also
 *          implements utility functions to hide some of the complexity in
 *          links like sending command to other link on local or remote
 *          processor is transparent to link. That complexity is handled by
 *          system link APIs.
 *
 * \version 0.0 (Apr 2014) : [YM] First version taken from DVR RDK and
 *                                cleaned up for Vision_sdk
 *
 *******************************************************************************
 */

#include "system_priv_common.h"
#include "system_priv_ipc.h"

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
Int32 System_linkCreate(UInt32 linkId, Ptr createArgs, UInt32 argsSize)
{
    return System_linkControl(linkId, SYSTEM_CMD_CREATE, createArgs, argsSize, TRUE);
}

Int32 System_linkStart(UInt32 linkId)
{
    return System_linkControl(linkId, SYSTEM_CMD_START, NULL, 0, TRUE);
}

Int32 System_linkStop(UInt32 linkId)
{
    return System_linkControl(linkId, SYSTEM_CMD_STOP, NULL, 0, TRUE);
}

Int32 System_linkDelete(UInt32 linkId)
{
    return System_linkControl(linkId, SYSTEM_CMD_DELETE, NULL, 0, TRUE);
}

Int32 System_linkPrintBufferStatistics(UInt32 linkId)
{
    return System_linkControl(linkId,
                              SYSTEM_CMD_PRINT_BUFFER_STATISTICS,
                              NULL,
                              0,
                              TRUE);
}

Int32 System_linkPrintStatistics(UInt32 linkId)
{
    return System_linkControl(linkId,
                              SYSTEM_CMD_PRINT_STATISTICS,
                              NULL,
                              0,
                              TRUE);
}

Int32 System_linkControl(UInt32 linkId, UInt32 cmd, Void *pPrm, UInt32 prmSize, Bool waitAck)
{
    Int32  status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    if(procId >= SYSTEM_PROC_MAX)
    {
        Vps_printf(" SYSTEM: Invalid proc ID ( procID = %d, linkID = 0x%08x, cmd = 0x%08x) \r\n", procId, linkId, cmd);
    }
    OSA_assert(  procId < SYSTEM_PROC_MAX);

    if(procId != System_getSelfProcId())
    {
        status = System_ipcMsgQSendMsg(linkId, cmd, pPrm, prmSize, waitAck);
    }
    else
    {
        status = System_linkControl_local(linkId, cmd, pPrm, prmSize, waitAck);
    }

    return status;
}

Int32 System_linkControlWithTimeout(UInt32 linkId, UInt32 cmd, Void *pPrm, UInt32 prmSize, Bool waitAck, UInt32 timeout)
{
    Int32  status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    if(procId >= SYSTEM_PROC_MAX)
    {
        Vps_printf(" SYSTEM: Invalid proc ID ( procID = %d, linkID = 0x%08x, cmd = 0x%08x) \r\n", procId, linkId, cmd);
    }
    OSA_assert(  procId < SYSTEM_PROC_MAX);

    if(procId != System_getSelfProcId())
    {
        status = System_ipcMsgQSendMsg(linkId, cmd, pPrm, prmSize, waitAck);
    }
    else
    {
        status = System_linkControl_local(linkId, cmd, pPrm, prmSize, waitAck);
    }

    return status;
}

Int32 System_sendLinkCmd(UInt32 linkId, UInt32 cmd, Void *payload)
{
    Int32  status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    OSA_assert(  procId < SYSTEM_PROC_MAX);

    if(procId != System_getSelfProcId())
    {
        status = System_ipcMsgQSendMsg(linkId, cmd, NULL, 0, FALSE);
    }
    else
    {
        status = System_sendLinkCmd_local(linkId, cmd, payload);
    }

    return status;
}

Int32 System_linkGetInfo(UInt32 linkId, System_LinkInfo * info)
{
    Int32  status;
    UInt32 procId;

    procId = SYSTEM_GET_PROC_ID(linkId);

    OSA_assert(  procId < SYSTEM_PROC_MAX);

    if(procId != System_getSelfProcId())
    {
        status = System_ipcMsgQSendMsg(linkId, SYSTEM_CMD_GET_INFO, info, sizeof(*info), TRUE);
    }
    else
    {
        status = System_linkGetInfo_local(linkId, info);
    }

    return status;
}

OSA_TskHndl *System_getLinkTskHndl(UInt32 linkId)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    OSA_assert(  linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    return pTsk->pTsk;
}

Int32 System_registerLink(UInt32 linkId, System_LinkObj *pTskObj)
{
    linkId = SYSTEM_GET_LINK_ID(linkId);

    OSA_assert(  linkId < SYSTEM_LINK_ID_MAX);

    if(gSystem_objCommon.linkObj[linkId].pTsk!=NULL)
    {
        Vps_printf(" SYSTEM: ERROR: Link is already registered at linkId = %d !!!\n", linkId);
        OSA_assert(0);
    }

    memcpy(&gSystem_objCommon.linkObj[linkId], pTskObj, sizeof(*pTskObj));

    return OSA_SOK;
}

Int32 System_getLinksFullBuffers(UInt32 linkId, UInt16 queId,
                                 System_BufferList * pBufList)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if (pTsk->linkGetFullBuffers != NULL)
        return pTsk->linkGetFullBuffers(pTsk->pTsk, queId, pBufList);

    return SYSTEM_LINK_STATUS_EFAIL;
}


Int32 System_putLinksEmptyBuffers(UInt32 linkId, UInt16 queId,
                                  System_BufferList * pBufList)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if (pTsk->linkPutEmptyBuffers != NULL)
        return pTsk->linkPutEmptyBuffers(pTsk->pTsk, queId, pBufList);

    return SYSTEM_LINK_STATUS_EFAIL;
}

