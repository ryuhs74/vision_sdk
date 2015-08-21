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
 * \file system_common.c
 *
 * \brief   System level initialization for all components.
 *
 *          This file deals with system level initialization of links and
 *          chains framework. It creates a system tasks for handling all system
 *          wide functions. Initializes the system wide resources like
 *          and components performance counters, mail boxes, IPC, memory
 *          allocators. It also initializes all links in the system such that
 *          links are ready to take commands from application.
 *
 * \version 0.0 (Apr 2014) : [YM] First version taken from bios side vision sdk

 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_common.h"
#include "system_priv_ipc.h"
#include <osa_remote_log_if.h>

/**
 *******************************************************************************
 * \brief Global object for system link.
 *
 *        All the links registers itself to system link. This global object
 *        has information about all links like link's call back functions
 *        and linkInfo
 *******************************************************************************
 */
System_CommonObj gSystem_objCommon;

/**
 *******************************************************************************
 * \brief Global object for defining which Processors needs to be enabled.
 *
 * This is required to loop around the IPC attach and other ipc functions
 * based on processors enabled in links and chains.
 *******************************************************************************
 */
const UInt32 gSystem_ipcEnableProcId[] = {
#if defined(PROC_IPU1_0_INCLUDE)
    SYSTEM_PROC_IPU1_0,
#endif
#if defined(PROC_IPU1_1_INCLUDE)
    SYSTEM_PROC_IPU1_1,
#endif
#if defined(PROC_DSP1_INCLUDE)
    SYSTEM_PROC_DSP1,
#endif
#if defined(PROC_DSP2_INCLUDE)
    SYSTEM_PROC_DSP2,
#endif
#if defined(PROC_EVE1_INCLUDE)
    SYSTEM_PROC_EVE1,
#endif
#if defined(PROC_EVE2_INCLUDE)
    SYSTEM_PROC_EVE2,
#endif
#if defined(PROC_EVE3_INCLUDE)
    SYSTEM_PROC_EVE3,
#endif
#if defined(PROC_EVE4_INCLUDE)
    SYSTEM_PROC_EVE4,
#endif
#if defined(PROC_A15_0_INCLUDE)
    SYSTEM_PROC_A15_0,
#endif
    SYSTEM_PROC_MAX /* MUST be the last entry in this list */
};

/**
 *******************************************************************************
 * \brief Global object for mapping scan format enum to string.
 *
 *******************************************************************************
 */
const char *gSystem_nameScanFormat[] = SYSTEM_SCAN_FORMAT_STRINGS;

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
    return SYSTEM_PROC_A15_0;
}

/**
 *******************************************************************************
 *
 * \brief This function waits for all cores to complete the application level
 *        initialization.
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_waitAppInitComplete()
{
    UInt32 procId;
    UInt32 initDone[SYSTEM_PROC_MAX] = {0,};
    UInt32 allInitDone;
    unsigned int state;

    initDone[SYSTEM_PROC_A15_0] = TRUE;

    while (1)
    {
        for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
        {
            if(System_isProcEnabled(procId))
            {
                if (FALSE == initDone[procId])
                {
                    RemoteLog_getAppInitState(procId, &state);

                    if (state == CORE_APP_INITSTATUS_INIT_DONE)
                    {
                        initDone[procId] = TRUE;
                    }
                }
            }
        }

        allInitDone = TRUE;

        for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
        {
            if(System_isProcEnabled(procId))
            {
                if (FALSE == initDone[procId])
                {
                    allInitDone = FALSE;
                    break;
                }
            }
        }

        if(TRUE == allInitDone)
        {
            break;
        }
        OSA_waitMsecs(1);
    }
}

/**
 *******************************************************************************
 *
 * \brief This function set the trigger to exit the application
 *        for all remote processor's in the system
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_triggerAppExit()
{
    unsigned int state = CORE_APP_INITSTATUS_DO_EXIT;

    /* only trigger app exit for IPU1-0, it will in turn
     * trigger exit and wait for other's CPU to exit
     */
    RemoteLog_setAppInitState(SYSTEM_PROC_IPU1_0, state);
}

/**
 *******************************************************************************
 *
 * \brief This function waits for all cores to complete the application level
 *        initialization.
 *
 *        This function is effective only for the "host" processor
 *
 *******************************************************************************
 */
Void System_waitAppExitComplete()
{
    UInt32 procId;
    UInt32 exitDone[SYSTEM_PROC_MAX] = {0,};
    UInt32 allInitDone;
    unsigned int state;

    exitDone[SYSTEM_PROC_A15_0] = TRUE;

    while (1)
    {
        for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
        {
            if(System_isProcEnabled(procId))
            {
                if (FALSE == exitDone[procId])
                {
                    RemoteLog_getAppInitState(procId, &state);

                    if (state == CORE_APP_INITSTATUS_EXIT_DONE)
                    {
                        exitDone[procId] = TRUE;
                    }
                }
            }
        }

        allInitDone = TRUE;

        for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
        {
            if(System_isProcEnabled(procId))
            {
                if (FALSE == exitDone[procId])
                {
                    allInitDone = FALSE;
                    break;
                }
            }
        }

        if(TRUE == allInitDone)
        {
            break;
        }
        OSA_waitMsecs(1);
    }
}

/**
 *******************************************************************************
 * \brief API to Initialize the system
 *
 *   - Initialize various links present in the core
 *   - Initialize the resources
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_init()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    OSA_init();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System A15 Init in progress !!!\n");
#endif

    memset(&gSystem_objCommon, 0, sizeof(gSystem_objCommon));

    status = OSA_mutexCreate(&gSystem_objCommon.linkControlMutex);
    OSA_assertSuccess(status);

    status = OSA_mbxCreate(&gSystem_objCommon.mbx);
    OSA_assertSuccess(status);

    System_ipcInit();

    System_initLinks();

    System_waitAppInitComplete();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System A15 Init Done !!!\n");
#endif
    return status;
}

/**
 *******************************************************************************
 *
 * \brief API to De-Initialize the system
 *
 *  - De-Initialize various links present in the core
 *  - De-Initialize the resources
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_deInit(UInt32 shutdownRemoteProcs)
{
#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System A15 De-Init in progress !!!\n");
#endif

    if(shutdownRemoteProcs)
    {
        System_triggerAppExit();
        System_waitAppExitComplete();
    }

    System_ipcDeInit();
#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System A15 De-Init Done !!!\n");
#endif

    /* wait for remote log prints to get flushed */
    OSA_waitMsecs(20);

    OSA_deInit();

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the A15 system links
 *
 *******************************************************************************
*/
Void System_initLinks()
{
    Vps_printf(" SYSTEM: Initializing A15 Links !!! \r\n");

    IpcOutLink_init();
    IpcInLink_init();
    NullLink_init();
#ifdef BUILD_TDA2XX_LINKS
    SgxDisplayLink_init();
    Sgx3DsrvLink_init();
#endif
#ifdef BUILD_DRA7XX_LINKS
    EpLink_init();
#endif
    Vps_printf(" SYSTEM: Initializing A15 Links ... DONE !!! \r\n");
}

/**
 *******************************************************************************
 *
 * \brief De-initialize the previously initialized A15 link
 *
 *******************************************************************************
*/
Void System_deInitLinks()
{
    Vps_printf(" SYSTEM: De-Initializing A15 Links !!! \r\n");

    IpcOutLink_deInit();
    IpcInLink_deInit();
    NullLink_deInit();
#ifdef BUILD_TDA2XX_LINKS
    SgxDisplayLink_deInit();
    Sgx3DsrvLink_deInit();
#endif
#ifdef BUILD_DRA7XX_LINKS
    EpLink_deInit();
#endif
    Vps_printf(" SYSTEM: De-Initializing A15 Links ... DONE !!! \r\n");
}

