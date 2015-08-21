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
 * \file system_a15.c
 *
 * \brief  This file has the implementataion of system and link init and deinit
 *
 *         This file implements system Init deinit , Link init and deinit calls,
 *         A15 Alg link and platform initialization for A15
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
#include <xdc/std.h>
#include "system_priv_a15.h"

/**
 *******************************************************************************
 * \brief A15 global Object
 *******************************************************************************
 */
System_A15Obj gSystem_objA15;

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
    Vps_printf(" SYSTEM: System A15 Init in progress !!!\n");
#endif

    Utils_dmaInit();

    System_initLinks();

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
Int32 System_deInit()
{
#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System A15 De-Init in progress !!!\n");
#endif
    System_deInitLinks();

    Utils_dmaDeInit();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System A15 De-Init Done !!!\n");
#endif

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

    Vps_printf(" SYSTEM: De-Initializing A15 Links ... DONE !!! \r\n");
}

/* Nothing beyond this point */

