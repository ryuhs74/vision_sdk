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
 * \file system_ipc_notify.c
 *
 * \brief  This file implements the function for the IPC notify functionality
 *
 *         Registers callback with notify sub-system of IPC. Registers
 *         system wide callback to notify. Handles the notify event and pass
 *         it to the link for which notify is intended by calling link
 *         notify handler function.
 *
 * \version 0.0 (Jun 2013) : [KC] First version taken from DVR RDK and
 *                                cleaned up for Vision_sdk
 * \version 0.1 (Jul 2013) : [HS] Commenting style update as per defined
 *                                format.
 * \version 0.2 (Sep 2014) : [YM] Implemented temporary routing messaging
 *                                for EVE-> A15 + Linux.
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipc.h"
#include <ti/ipc/Ipc.h>
#include <ti/sdo/ipc/_Ipc.h>
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/Notify.h>

/**
 *******************************************************************************
 *
 * \brief Wrapper around Ipc_start function of IPC component.
 *
 * \return  None
 *
 *******************************************************************************
 */
Void System_ipcStart()
{
    Int32 status = Ipc_S_SUCCESS;

    do
    {
        status = Ipc_start();
    } while (status != Ipc_S_SUCCESS);

    /* In case of fast boot System_ipcStart() will be called late,
     * after all cores are up & Notify_registerEvent is not called
     * under System_ipcNotifyInit(), so needs to be called here.
     */
    if(ti_sdo_ipc_Ipc_procSync == ti_sdo_ipc_Ipc_ProcSync_ALL)
    {
        System_ipcNotifyRegister();
    }
}

/**
 *******************************************************************************
 *
 * \brief Wrapper around Ipc_stop function of IPC component.
 *
 *
 * \return  None
 *
 *******************************************************************************
 */
Void System_ipcStop()
{
    Int32 status = Ipc_S_SUCCESS;

    do
    {
        status = Ipc_stop();
    } while (status != Ipc_S_SUCCESS);
}


/**
 *******************************************************************************
 *
 * \brief Attach local core to all cores for further IPC communication between
 *        cores
 *
 * \return  None
 *
 *******************************************************************************
 */
Void System_ipcAttach()
{
    UInt32 i;
    UInt32 procId, syslinkProcId;
    Int32 status;
    UInt retryCount = 0;

    i = 0;

    while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
    {
        procId = gSystem_ipcEnableProcId[i];

        if ((procId != System_getSelfProcId()) && (procId != SYSTEM_PROC_INVALID))
        {
            do
            {
                if ((retryCount % 1000) == 0)
                {
                    Vps_printf(" SYSTEM: Attaching to [%s] ... \n",
                               System_getProcName(procId));
                }
                syslinkProcId = System_getSyslinkProcId(procId);

                status = Ipc_attach(syslinkProcId);
                if (status != 0)
                {
                    BspOsal_sleep(1);
                    retryCount++;
                }
            } while (status < 0);
            Vps_printf(" SYSTEM: Attaching to [%s] ... SUCCESS !!!\n",
                       System_getProcName(procId));
        }
        i++;
    }
}

/**
 *******************************************************************************
 *
 * \brief System wide notify handler function.
 *
 *        This function gets registered with the ipc_notifyhandler. Based on
 *        payload linkId is derived. From linkID callback of the specific
 *        link gets invoked.
 * \param   procId      [IN] processorId which invoked notify
 * \param   lineId      [IN] LineId of the notify
 * \param   eventId     [IN] Notify eventId
 * \param   arg         [IN] not used
 * \param   payload     [IN] 4-byte notify payload. From this procId and
 *                           linkId is derived to invoke link specific
 *                           callback function
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Void System_ipcNotifyHandler(UInt16 procId, UInt16 lineId,
                             UInt32 eventId, UArg arg, UInt32 payload)
{
    if (lineId != SYSTEM_IPC_NOTIFY_LINE_ID)
        return;

    if (eventId != SYSTEM_IPC_NOTIFY_EVENT_ID)
        return;

#ifdef A15_TARGET_OS_LINUX
    #ifdef BUILD_M4_0
    if(SYSTEM_LINK_ID_TEST_ROUTE_BIT_TRUE(payload))
    {
        SYSTEM_LINK_ID_CLEAR_ROUTE_BIT(payload);
        System_rpmsgSendNotify(payload);
        return;
    }
    #endif
#endif

    System_ipcHandler(payload);
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
Int32 System_ipcNotifySendEvent(UInt32 linkId)
{
    Int32 status;
    UInt32 procId = SYSTEM_GET_PROC_ID(linkId), syslinkProcId;

    UTILS_assert(procId < SYSTEM_PROC_MAX);

#ifdef SYSTEM_IPC_MSGQ_DEBUG
    Vps_printf(" SYSTEM: NOTIFY: Sending Notify to [%s] !!!\n", System_getProcName(procId));
#endif

#ifdef A15_TARGET_OS_LINUX
    /* IPU and DSPs should never call this function directly
     * They must use System_ipcSendNotify()
     */
    #ifdef BUILD_ARP32
    if(procId == SYSTEM_PROC_A15_0)
    {
        /* Set route bit and send it to IPU1 */
        SYSTEM_LINK_ID_SET_ROUTE_BIT(linkId);
        status = Notify_sendEvent(System_getSyslinkProcId(SYSTEM_PROC_IPU1_0),
                                  SYSTEM_IPC_NOTIFY_LINE_ID,
                                  SYSTEM_IPC_NOTIFY_EVENT_ID, linkId, TRUE);

        if (status != Notify_S_SUCCESS)
        {
            Vps_printf
                (" SYSTEM: NOTIFY: Send Event to [%s][%d] failed !!! (status = %d)\n",
                 System_getProcName(SYSTEM_GET_PROC_ID(linkId)),
                 SYSTEM_GET_LINK_ID(linkId), status);

            UTILS_assert(status == Notify_S_SUCCESS);
        }

        return SYSTEM_LINK_STATUS_SOK;
    }
    #endif
#endif

    if(System_isProcEnabled(procId)==FALSE)
    {
        Vps_printf(
            " SYSTEM: NOTIFY: [%s] is NOT ENABLED in this build !!!\n",
            System_getProcName(procId)
            );
        UTILS_assert(0);
    }

    syslinkProcId = System_getSyslinkProcId(procId);

    status = Notify_sendEvent(syslinkProcId,
                              SYSTEM_IPC_NOTIFY_LINE_ID,
                              SYSTEM_IPC_NOTIFY_EVENT_ID, linkId, TRUE);

    if (status != Notify_S_SUCCESS)
    {
        Vps_printf
            (" SYSTEM: NOTIFY: Send Event to [%s][%d] failed !!! (status = %d)\n",
             System_getProcName(SYSTEM_GET_PROC_ID(linkId)),
             SYSTEM_GET_LINK_ID(linkId), status);

        UTILS_assert(status == Notify_S_SUCCESS);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Utility to initialize notify sub-system for links and chains
 *        work.
 *
 *        Initialize the notify event for local processor. It also registers
 *        callback with notify sub-system to get notify events from remote
 *        processor.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcNotifyRegister()
{
    UInt32 procId, i, syslinkProcId;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 errorProcId = SYSTEM_PROC_INVALID;
    volatile int j = 1;

    i = 0;
    while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
    {
        procId = gSystem_ipcEnableProcId[i];
        if ((procId != System_getSelfProcId()) && (procId != SYSTEM_PROC_INVALID))
        {
            Vps_printf
                (" SYSTEM: Notify register to [%s] line %d, event %d... \n",
                 System_getProcName(procId),
                 SYSTEM_IPC_NOTIFY_LINE_ID, SYSTEM_IPC_NOTIFY_EVENT_ID);

            syslinkProcId = System_getSyslinkProcId(procId);

            if (Notify_intLineRegistered
                (syslinkProcId, SYSTEM_IPC_NOTIFY_LINE_ID) ==
                FALSE)
            {
                /* Removed UTILS_assert(0) to avoid Task_sleep */
                errorProcId = procId;
                break;
            }

            if (Notify_eventAvailable
                (syslinkProcId, SYSTEM_IPC_NOTIFY_LINE_ID,
                 SYSTEM_IPC_NOTIFY_EVENT_ID) == FALSE)
            {
                /* Removed UTILS_assert(0) to avoid Task_sleep */
                errorProcId = procId;
                break;
            }

            status = Notify_registerEvent(syslinkProcId,
                                          SYSTEM_IPC_NOTIFY_LINE_ID,
                                          SYSTEM_IPC_NOTIFY_EVENT_ID,
                                          System_ipcNotifyHandler, (UArg)NULL);

            if(status != Notify_S_SUCCESS)
            {
                /* Removed UTILS_assert() to avoid Task_sleep */
                errorProcId = procId;
                break;
            }
        }
        i++;
    }
    if(errorProcId != SYSTEM_PROC_INVALID)
    {
        Vps_printf(" SYSTEM: System_ipcNotifyRegister failed for proc %s\n", System_getProcName(errorProcId));
        while(j);
    }

    return status;
}

Int32 System_ipcNotifyInit()
{
    memset(gSystem_ipcObj.notifyCb, 0, sizeof(gSystem_ipcObj.notifyCb));

    /* This check is needed especially when Ipc.procSync is procSync.PAIR
     * System_ipcAttach() needs to be done before Notify_registerEvent is called.
     * Note: In procSync.PAIR Ipc_start() doesn't call Ipc_attach().
     */
    if(ti_sdo_ipc_Ipc_procSync != ti_sdo_ipc_Ipc_ProcSync_ALL)
    {
        System_ipcAttach();
        System_ipcNotifyRegister();
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Utility to de-initialize notify sub-system for links and chains
 *        work.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_ipcNotifyDeInit()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 i, procId, syslinkProcId;

    Vps_printf(" SYSTEM: IPC Notify de-init in progress !!!\n");

    i = 0;

    while (gSystem_ipcEnableProcId[i] != SYSTEM_PROC_MAX)
    {
        procId = gSystem_ipcEnableProcId[i];
        if ((procId != System_getSelfProcId()) && (procId != SYSTEM_PROC_INVALID))
        {
            syslinkProcId = System_getSyslinkProcId(procId);

            status = Notify_unregisterEvent(syslinkProcId,
                                            SYSTEM_IPC_NOTIFY_LINE_ID,
                                            SYSTEM_IPC_NOTIFY_EVENT_ID,
                                            System_ipcNotifyHandler,
                                            (UArg)NULL);

            UTILS_assert(status == Notify_S_SUCCESS);
        }
        i++;
    }

    Vps_printf(" SYSTEM: IPC Notify de-init DONE !!!\n");

    return status;
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

/**
 *******************************************************************************
 *
 * \brief Returns processor ID on which function is called.
 *
 *        Returns the processor id on which this function is getting called.
 *        Suppose code on DSP calls this function it returns processor id as
 *        DSP
 *
 * \return  ProcessorId for valid processor else SYSTEM_PROC_INVALID
 *
 *******************************************************************************
 */
UInt32 System_getSelfProcId()
{
    static UInt32 selfProcId = SYSTEM_PROC_INVALID;
    UInt32 syslinkProcId;
    String procName;

    /*  find self proc ID only once, since it never changes run-time
     *  Doing this optimization since, this API is called for every
     *  System_linkXxxx API
     */
    if(selfProcId!=SYSTEM_PROC_INVALID)
    {
        return selfProcId;
    }
    syslinkProcId = MultiProc_self();
    procName = MultiProc_getName(syslinkProcId);

    if(procName==NULL || syslinkProcId >= SYSTEM_PROC_MAX)
    {
        selfProcId = SYSTEM_PROC_INVALID;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_DSP1)==0)
    {
        selfProcId = SYSTEM_PROC_DSP1;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_DSP2)==0)
    {
        selfProcId = SYSTEM_PROC_DSP2;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_EVE1)==0)
    {
        selfProcId = SYSTEM_PROC_EVE1;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_EVE2)==0)
    {
        selfProcId = SYSTEM_PROC_EVE2;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_EVE3)==0)
    {
        selfProcId = SYSTEM_PROC_EVE3;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_EVE4)==0)
    {
        selfProcId = SYSTEM_PROC_EVE4;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_IPU1_0)==0)
    {
        selfProcId = SYSTEM_PROC_IPU1_0;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_IPU1_1)==0)
    {
        selfProcId = SYSTEM_PROC_IPU1_1;
    }
    else if(strcmp(procName, SYSTEM_IPC_PROC_NAME_A15_0)==0)
    {
        selfProcId = SYSTEM_PROC_A15_0;
    }
    else
    {
        selfProcId = SYSTEM_PROC_INVALID;
    }
    return selfProcId;
}
