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
 * \defgroup SYSTEM_IPU1_1_IMPL System implementation for IPU1 Core 1
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file system_priv_ipu1_1.h IPU1_1 private file containing all the header files
 *                            util files required by IPU1_1
 *
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_PRIV_IPU1_1_H_
#define _SYSTEM_PRIV_IPU1_1_H_

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
#include <include/link_api/dupLink.h>
#include <include/link_api/gateLink.h>
#include <include/link_api/selectLink.h>
#include <include/link_api/syncLink.h>
#include <include/link_api/mergeLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/nullLink.h>
#include <include/link_api/nullSrcLink.h>
#include <include/link_api/avbRxLink.h>
#include <include/link_api/saveLink.h> //ryuhs74@20151027 - Add Save Link Header File Include


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure to hold the IPU1_1 global objects.
 *          Any Link in the IPU1_1 core can use these objects.
 *
 *******************************************************************************
*/
typedef struct {
    UInt32 reserved;

} System_Ipu1_1_Obj;

extern System_Ipu1_1_Obj gSystem_objIpu1_1;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
