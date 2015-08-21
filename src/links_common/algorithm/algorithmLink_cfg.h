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
 * \ingroup ALGORITHM_LINK_API
 * \defgroup ALGORITHM_LINK_IMPL Algorithm Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_cfg.h Algorithm Link private API/Data structures
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

#ifndef _ALGORITHM_LINK_CFG_H_
#define _ALGORITHM_LINK_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/algorithmLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Maximum number of algorithm links to be used and stack sizes
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
#define ALGORITHM_LINK_OBJ_MAX        (8)
/* stack size increased for pedstrain detect object tracking lib */
#define ALGORITHM_LINK_TSK_STACK_SIZE (1024*1024)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_DSP_ALG_MAXNUM)
#endif

#ifdef BUILD_DSP_2
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (1024*1024)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_DSP_ALG_MAXNUM)
#endif

#ifdef BUILD_ARP32_1
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (7*KB)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_EVE_ALG_MAXNUM)
#endif

#ifdef BUILD_ARP32_2
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (7*KB)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_EVE_ALG_MAXNUM)
#endif

#ifdef BUILD_ARP32_3
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (7*KB)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_EVE_ALG_MAXNUM)
#endif

#ifdef BUILD_ARP32_4
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (7*KB)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_EVE_ALG_MAXNUM)
#endif

#ifdef BUILD_M4
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_IPU_ALG_MAXNUM)
#endif

#ifdef BUILD_A15
#define ALGORITHM_LINK_OBJ_MAX        (8)
#define ALGORITHM_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#define ALGORITHM_LINK_ALG_MAXNUM     (ALGORITHM_LINK_A15_ALG_MAXNUM)
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
Int32 AlgorithmLink_initAlgPlugins();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
