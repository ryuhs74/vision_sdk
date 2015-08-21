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
 *
 * \ingroup SYSTEM_IMPL
 *
 * \defgroup SYSTEM_DSP_IMPL System implementation for DSP Core 0
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file system_priv_c6xdsp.h DSP private file containing all the header files
 *                            util files required by DSP
 *
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_PRIV_C6XDSP_H_
#define _SYSTEM_PRIV_C6XDSP_H_

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_common.h>
#include <include/link_api/system.h>
#include <include/link_api/systemLink_c6xdsp.h>
#include <include/link_api/dupLink.h>
#include <include/link_api/gateLink.h>
#include <include/link_api/selectLink.h>
#include <include/link_api/syncLink.h>
#include <include/link_api/mergeLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/nullLink.h>


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure to hold the DSP global objects.
 *          Any Link in the DSP core can use these objects.
 *
 *******************************************************************************
*/
typedef struct {
    UInt32 c66CpuIdleInitDone;
    /**< DSP CPU Idle Initialization done Flag */
    UInt32 reserved;

} System_DspObj;

extern System_DspObj gSystem_objDsp;

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
