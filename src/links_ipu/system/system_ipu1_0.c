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
 * \file system_ipu1_0.c
 *
 * \brief  This file has the implementataion of system and link init and deinit
 *
 *         This file implements system Init deinit , Link init and deinit calls,
 *         BSP driver and platform initialized from IPU1
 *
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "system_priv_ipu1_0.h"
#include "system_bsp_init.h"
#include <src/links_common/system/system_priv_common.h>
#include <src/links_ipu/iva/codec_utils/hdvicp2_config.h>


/**
 *******************************************************************************
 * \brief IPU1_0 global Object
 *******************************************************************************
 */
System_Ipu1_0_Obj gSystem_objIpu1_0;

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
 *   Links and chains APIs allow user to connect different drivers in a
 *   logical consistent way in order to make a chain of data flow.
 *
 *   Example,
 *   Capture + NSF + DEI + SC + Display
 *   OR
 *   Capture + Display
 *
 *   A link is basically a task which exchange frames with other links and
 *   makes FVID2 driver  calls to process the frames.
 *
 *   A chain is a connection of links.
 *
 *   Links exchange frames with each other via buffer queue's.
 *
 *   Links exchange information with each other and the top level system task
 *   via mail box.
 *
 *   When a link is connected to another link, it basically means output queue
 *   of one link is connected to input que of another link.
 *
 *   All links have a common minimum interface which makes it possible for a
 *   link to exchange frames with another link without knowing the other links
 *   specific details. This allow the same link to connect to different other
 *   links in different data flow scenario's
 *
 *   Example,
 *   Capture can be connected to either display in the Capture + Display chain
 *   OR
 *   Capture can be connected to NSF in the Capture + NSF + DEI + SC + Display
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 System_init()
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG
    Vps_printf(" SYSTEM: System Init in progress !!!\n");
#endif

   /* BSP driver and platform initialized from IPU1 */
   System_bspInit();

   Utils_dmaInit();
   Utils_statCollectorInit(); /* Initializing the statCollector */

   System_initLinks();
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
   Int32 status = SYSTEM_LINK_STATUS_SOK;

   System_deInitLinks();

   Utils_dmaDeInit();
   Utils_statCollectorDeInit();

   System_bspDeInit();

#ifdef SYSTEM_DEBUG
   Vps_printf(" SYSTEM: System De-Init Done !!!\n");
#endif

   return status;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the ipu system links
 *
 *******************************************************************************
*/
Void System_initLinks()
{

    Vps_printf(" SYSTEM: Initializing Links !!! \r\n");

    #ifdef ISS_INCLUDE
    IssCaptureLink_init();
    IssM2mIspLink_init();
    IssM2mSimcopLink_init();
    Utils_ispLockCreate();
    #endif
    DupLink_init();
    GateLink_init();
    MergeLink_init();
    SyncLink_init();
    IpcOutLink_init();
    IpcInLink_init();

    #ifdef DSS_INCLUDE
    DisplayLink_init();
    DisplayCtrlLink_init();
    #endif
    AlgorithmLink_init();

    #if defined(VPE_INCLUDE) || defined(ISS_INCLUDE)
    VpeLink_init();
    #endif

    CaptureLink_init();
    System_memPrintHeapStatus();

    NullLink_init();
    SelectLink_init();
    NullSrcLink_init();
    #ifdef IVAHD_INCLUDE
    DecLink_init();
    EncLink_init();
    #endif
    #ifdef AVBRX_INCLUDE
        #ifdef NDK_PROC_TO_USE_IPU1_0
        AvbRxLink_init();
        #endif
    #endif

    Vps_printf(" SYSTEM: Initializing Links ... DONE !!! \r\n");
}

/**
 *******************************************************************************
 *
 * \brief De-initialize the previously initialized ipu link
 *
 *******************************************************************************
*/
Void System_deInitLinks()
{
    Vps_printf(" SYSTEM: De-Initializing Links !!! \r\n");

    #ifdef DSS_INCLUDE
    DisplayCtrlLink_deInit();
    DisplayLink_deInit();
    #endif

    DupLink_deInit();
    GateLink_deInit();
    SyncLink_deInit();
    MergeLink_deInit();
    IpcOutLink_deInit();
    IpcInLink_deInit();
    AlgorithmLink_deInit();

    #ifdef ISS_INCLUDE
    IssCaptureLink_deInit();
    IssM2mIspLink_deInit();
    IssM2mSimcopLink_deInit();
    #endif

    #if defined(VPE_INCLUDE) || defined(ISS_INCLUDE)
    VpeLink_deInit();
    #endif

    CaptureLink_deInit();

    NullLink_deInit();
    SelectLink_deInit();
    NullSrcLink_deInit();

    #ifdef IVAHD_INCLUDE
    DecLink_deInit();
    EncLink_deInit();
    #endif

    #ifdef AVBRX_INCLUDE
        #ifdef NDK_PROC_TO_USE_IPU1_0
        AvbRxLink_deInit();
        #endif
    #endif

    System_memPrintHeapStatus();
    Vps_printf(" SYSTEM: De-Initializing Links ... DONE !!! \r\n");
}

