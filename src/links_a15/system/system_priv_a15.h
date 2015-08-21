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
 * \defgroup SYSTEM_A15_IMPL System implementation for A15 Core 0
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file system_priv_a15.h A15 private file containing all the header files
 *                            util files required by A15
 *
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_PRIV_A15_H_
#define _SYSTEM_PRIV_A15_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_common.h>
#include <include/link_api/system.h>
#include <include/link_api/systemLink_a15.h>
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
 *  \brief  Structure to hold the A15 global objects.
 *          Any Link in the A15 core can use these objects.
 *
 *******************************************************************************
*/
typedef struct {
    UInt32 reserved;

} System_A15Obj;

extern System_A15Obj gSystem_objA15;

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
