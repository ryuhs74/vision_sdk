/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_common.h"

#ifdef BUILD_DSP_1
/* This size of 512KB is for PD algo requirement */
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (512*KB)
#endif

#ifdef BUILD_DSP_2
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (1024*1024)
#endif

#ifdef BUILD_ARP32_1
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (8*KB)
#endif

#ifdef BUILD_ARP32_2
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (8*KB)
#endif

#ifdef BUILD_ARP32_3
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (8*KB)
#endif

#ifdef BUILD_ARP32_4
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (8*KB)
#endif

#ifdef BUILD_M4
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#endif

#ifdef BUILD_A15
#define SYSTEM_TSK_MULTI_MBX_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#endif

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSystem_tskMultiMbxStack, 32)
#pragma DATA_SECTION(gSystem_tskMultiMbxStack, ".bss:taskStackSection:systemTskMultiMbx")
UInt8 gSystem_tskMultiMbxStack[SYSTEM_TSK_MULTI_MBX_STACK_SIZE];


Utils_TskMultiMbxHndl gSystem_tskMultiMbx;


Utils_TskMultiMbxHndl *System_getTskMultiMbxHndl()
{
    char tskName[32];
    static Bool isInit = FALSE;

    if(!isInit)
    {
        sprintf(tskName, "SYSTEM_TSK_MULTI_MBX");

        Utils_tskMultiMbxSetupTskHndl(
            &gSystem_tskMultiMbx,
            gSystem_tskMultiMbxStack,
            sizeof(gSystem_tskMultiMbxStack),
            SYSTEM_TSK_MULTI_MBX_TSK_PRI,
            UTILS_TASK_MULTI_MBX_RECV_QUE_MAX,
            tskName
            );

        isInit = TRUE;
    }
    return &gSystem_tskMultiMbx;
}
