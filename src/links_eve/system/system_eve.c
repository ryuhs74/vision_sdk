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
 * \file system_eve.c
 *
 * \brief  This file has the implementataion of system and link init and deinit
 *
 *         This file implements system Init deinit , Link init and deinit calls,
 *         EVE Alg link and platform initialization for EVE
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
#include "system_priv_eve.h"

/**
 *******************************************************************************
 * \brief EVE global Object
 *******************************************************************************
 */
System_EveObj gSystem_objEve;


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
    Vps_printf(" SYSTEM: System EVE Init in progress !!!\n");
#endif

    Utils_dmaInit();

    System_initLinks();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System EVE Init Done !!!\n");
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
    Vps_printf(" SYSTEM: System Eve De-Init in progress !!!\n");
#endif

    System_deInitLinks();

    Utils_dmaDeInit();

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System Eve De-Init Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the EVE system links
 *
 *******************************************************************************
*/
Void System_initLinks()
{
    Vps_printf(" SYSTEM: Initializing EVE Links !!! \r\n");
    System_memPrintHeapStatus();

    IpcOutLink_init();
    IpcInLink_init();
    AlgorithmLink_init();
    DupLink_init();
    GateLink_init();
    MergeLink_init();
    SyncLink_init();
    SelectLink_init();

    Vps_printf(" SYSTEM: Initializing EVE Links ... DONE !!! \r\n");
}

/**
 *******************************************************************************
 *
 * \brief De-initialize the previously initialized EVE link
 *
 *******************************************************************************
*/
Void System_deInitLinks()
{
    Vps_printf(" SYSTEM: De-Initializing EVE Links !!! \r\n");

    DupLink_deInit();
    GateLink_deInit();
    MergeLink_deInit();
    SyncLink_deInit();
    SelectLink_deInit();
    IpcInLink_deInit();
    IpcOutLink_deInit();
    AlgorithmLink_deInit();

    Vps_printf(" SYSTEM: De-Initializing EVE Links ... DONE !!! \r\n");
}

/* Nothing beyond this point */

