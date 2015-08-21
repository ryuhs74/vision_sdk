/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED.
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file system_ipc.c
 *
 * \brief  Wrapper around IPC components calls.
 *
 *         This file implements wrapper around IPC calls. It also groups
 *         multiple ipc calls in one wrapper function wherever required.
 *
 * \version 0.0 (Jun 2013) : [KC] First version taken from DVR RDK and
 *                            cleaned up for Vision_sdk
 * \version 0.1 (Jul 2013) : [HS] Commenting style update as per defined
 *                                format.
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"


/**
 *******************************************************************************
 * \brief Global object for storing IPC related information..
 *
 *        Handle stores IPC related information like messageQ handles,
 *        Base address to message memories etc.
 *******************************************************************************
 */
System_IpcObj gSystem_ipcObj;

#ifndef BUILD_A15
#pragma DATA_SECTION(gSystem_ipcSharedMemObj,".bss:extMemNonCache:ipcShm");
#pragma DATA_ALIGN(gSystem_ipcSharedMemObj, 4);
#endif
System_IpcSharedMemObj  gSystem_ipcSharedMemObj
#ifdef BUILD_A15
__attribute__ ((section(".bss:extMemNonCache:ipcShm")))
__attribute__ ((aligned(4)))
#endif
;

Void System_ipcHandler(UInt32 payload)
{
    UInt32 linkId, linkProcId, type;

    linkProcId = SYSTEM_GET_PROC_ID(payload);

    UTILS_assert(System_getSelfProcId() == linkProcId);

    type = SYSTEM_LINK_ID_GET_NOTIFY_TYPE(payload);

#ifdef SYSTEM_IPC_MSGQ_DEBUG
    Vps_printf(" SYSTEM: NOTIFY: Recevied notify (type=%d, payload=0x%08x) !!!\n", type, payload);
#endif

    if(type==SYSTEM_LINK_ID_NOTIFY_TYPE_LINK_ID)
    {
        linkId = SYSTEM_GET_LINK_ID(payload);

        if (linkId < SYSTEM_LINK_ID_MAX)
        {
            Utils_TskHndl *pTsk;

            if (gSystem_ipcObj.notifyCb[linkId])
            {
                pTsk = System_getLinkTskHndl(linkId);

                gSystem_ipcObj.notifyCb[linkId] (pTsk);
            }
        }
    }
    else
    {
        System_ipcMsgQHandler(payload);
    }
}

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
    UInt32 procId = System_getSelfProcId();

    Vps_printf(" SYSTEM: IPC init in progress !!!\n");

    System_ipcNotifyInit();

    Vps_printf(" SYSTEM: Notify init done !!!\n");

    System_ipcMsgQInit();

    Vps_printf(" SYSTEM: MsgQ init done !!!\n");

    #ifdef A15_TARGET_OS_LINUX
    #ifndef BUILD_ARP32
    System_rpmsgInit();
    #endif
    #endif

    Vps_printf(" SYSTEM: IPC init DONE !!!\n");

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
    UInt32 procId = System_getSelfProcId();

    Vps_printf(" SYSTEM: IPC de-init in progress !!!\n");

    #ifdef A15_TARGET_OS_LINUX
    #ifndef BUILD_ARP32
    System_rpmsgDeInit();
    #endif
    #endif

    System_ipcNotifyDeInit();

    System_ipcMsgQDeInit();

    Vps_printf(" SYSTEM: IPC de-init DONE !!!\n");

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

/**
 *******************************************************************************
 *
 * \brief Function to register notify callback for links.
 *
 *        Links register its notify handler. This file registers a single call
 *        back function with the notify sub-system. Form that callback
 *        it invokes the callback registered by links based on the linkId
 *        passed with  notify callback.
 *
 * \param   linkId      [IN] linkID of the registering link.
 * \param   notifyCb    [IN] callback function of the link to be registered
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcRegisterNotifyCb(UInt32 linkId, System_ipcNotifyCb notifyCb)
{
    linkId = SYSTEM_GET_LINK_ID(linkId);
    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    gSystem_ipcObj.notifyCb[linkId] = notifyCb;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Utility function to invoke the notify to remote processor.
 *
 *        This function generates the notify event. It takes linkId as input
 *        maps linkId with procId and generates notify event on that processor
 *
 * \param   linkId      [IN] Link Id to which notify needs to be generated.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcSendNotify(UInt32 linkId)
{
    UInt32 selfProcId = System_getSelfProcId();

    #ifdef A15_TARGET_OS_LINUX
    #ifndef BUILD_ARP32
    {
        UInt32 dstProcId = SYSTEM_GET_PROC_ID(linkId);

        if(dstProcId==SYSTEM_PROC_A15_0)
        {
            return System_rpmsgSendNotify(linkId);
        }
    }
    #endif
    #endif

    return System_ipcNotifySendEvent(linkId);
}

/**
 *******************************************************************************
 *
 * \brief   Helper function to send messages to other links.
 *
 *          Allocates the message from free queue and send it to the link
 *          also waits for the acknowledgement for the message.
 *
 * \param  linkId    [IN] LinkId for which message is intended
 * \param  cmd       [IN] Command for message
 * \param  pPrm       [IN] Message parameter
 * \param  prmSize   [IN] Size of parameter
 * \param  waitAck   [IN] If True wait till ack is received, else proceed
 *                        without ack.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcSendMsg(UInt32 linkId, UInt32 cmd, Void * pPrm,
                            UInt32 prmSize, Bool waitAck)
{
    return System_ipcMsgQSendMsg(linkId, cmd, pPrm, prmSize, waitAck);
}

System_IpcMsg *System_ipcGetMsg(uint32_t procId)
{
    System_IpcSharedMemObj *pObj = &gSystem_ipcSharedMemObj;
    System_IpcMsg *pPrm = NULL;

    if(procId < SYSTEM_PROC_MAX)
    {
        pPrm = &pObj->ipcMsgObj.procMsg[procId];
    }

    return pPrm;
}

System_IpcQueObj *System_ipcGetIpcOut2InQue(uint32_t ipcOutLinkId)
{
    System_IpcSharedMemObj *pObj = &gSystem_ipcSharedMemObj;
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
    System_IpcSharedMemObj *pObj = &gSystem_ipcSharedMemObj;
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
    System_IpcSharedMemObj *pObj = &gSystem_ipcSharedMemObj;
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
