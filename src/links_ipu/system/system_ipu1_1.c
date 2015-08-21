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
 * \file system_ipu1_1.c
 *
 * \brief  This file has the implementataion of system and link init and deinit
 *
 *         This file implements system Init deinit , Link init and deinit calls,
 *         IPU1_1 link and platform initialization for IPU1_1
 *
 *
 * \version 0.0 (Jul 2013) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "system_priv_ipu1_1.h"
#include "include/link_api/dcanCtrl_api.h"


/**
 *******************************************************************************
 * \brief IPU1_1 global Object
 *******************************************************************************
 */
System_Ipu1_1_Obj gSystem_objIpu1_1;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

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

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System IPU1_1 Init in progress !!!\n");
#endif
    Utils_dmaInit();
    System_initLinks();


#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System IPU1_1 Init Done !!!\n");
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
Int32 System_deInit()
{
#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System IPU1_1 De-Init in progress !!!\n");
#endif

    System_deInitLinks();

    Utils_dmaDeInit();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System IPU1_1 De-Init Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the IPU1_1 system links
 *
 *******************************************************************************
*/
Void System_initLinks()
{
    Vps_printf(" SYSTEM: Initializing IPU1_1 Links !!! \r\n");
    System_memPrintHeapStatus();

    DupLink_init();
    GateLink_init();
    MergeLink_init();
    SelectLink_init();
    SyncLink_init();
    NullLink_init();
    NullSrcLink_init();
    IpcOutLink_init();
    IpcInLink_init();
    AlgorithmLink_init();

    #ifdef AVBRX_INCLUDE
        #ifdef NDK_PROC_TO_USE_IPU1_1
        AvbRxLink_init();
        #endif
    #endif
    #ifdef DCAN_INCLUDE
    System_dcanInit();
    #endif

    Vps_printf(" SYSTEM: Initializing IPU1_1 Links ... DONE !!! \r\n");
}

/**
 *******************************************************************************
 *
 * \brief De-initialize the previously initialized IPU1_1 link
 *
 *******************************************************************************
*/
Void System_deInitLinks()
{
    Vps_printf(" SYSTEM: De-Initializing IPU1_1 Links !!! \r\n");

    DupLink_deInit();
    GateLink_deInit();
    MergeLink_deInit();
    SelectLink_deInit();
    SyncLink_deInit();
    NullLink_deInit();
    NullSrcLink_deInit();
    IpcOutLink_deInit();
    IpcInLink_deInit();
    AlgorithmLink_deInit();

    #ifdef AVBRX_INCLUDE
        #ifdef NDK_PROC_TO_USE_IPU1_1
        AvbRxLink_deInit();
        #endif
    #endif
    #ifdef DCAN_INCLUDE
    System_dcanDeInit();
    #endif

    System_memPrintHeapStatus();
    Vps_printf(" SYSTEM: De-Initializing IPU1_1 Links ... DONE !!! \r\n");
}

/* Nothing beyond this point */

