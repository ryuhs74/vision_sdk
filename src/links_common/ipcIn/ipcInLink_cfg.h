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
 * \ingroup IPC_IN_LINK_API
 * \defgroup IPC_IN_LINK_IMPL IPC IN Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file ipcInLink_cfg.h IPC Link private API/Data structures
 *
 * \brief  This link private header file has all the definitions which
 *         are specific to the processor type / processor core
 *         Functions APIs which need to be called by the use case are listed
 *         here
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _IPC_IN_LINK_CFG_H_
#define _IPC_IN_LINK_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/ipcLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Maximum number of IPC links to be used and stack sizes
 *          to be used.
 *
 *          It is defined based on the processor core, thus giving
 *          flexibility of different number of links / stack sizes for different
 *          processor cores. However for different links on the same processor
 *          core, stack size is kept same.
 *
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */

#ifdef BUILD_DSP_1
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#endif

#ifdef BUILD_DSP_2
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#endif

#ifdef BUILD_ARP32_1
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (2*KB)
#endif

#ifdef BUILD_ARP32_2
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (2*KB)
#endif

#ifdef BUILD_ARP32_3
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (2*KB)
#endif

#ifdef BUILD_ARP32_4
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (2*KB)
#endif

#ifdef BUILD_M4
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#endif

#ifdef BUILD_A15
#define IPC_IN_LINK_OBJ_MAX        (8)
#define IPC_IN_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
