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
 * \ingroup SYSTEM_IPU1_0_IMPL
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file system_bsp_init.h
 *
 * \brief APIs for initializing BIOS Drivers.
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef BSP_INIT_H
#define BSP_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


#ifdef TDA3XX_FAMILY_BUILD

/** \brief Size of descriptor memory pool in bytes. */
#define BSP_DESC_MEM_SIZE (256*1024)

/** \brief Size of BSS memory pool in bytes. */
#define BSP_BSS_MEM_SIZE (288*1024)

#else

/** \brief Size of descriptor memory pool in bytes. */
#define BSP_DESC_MEM_SIZE (1024*1024)

/** \brief Size of BSS memory pool in bytes. */
#define BSP_BSS_MEM_SIZE (1024*1024)

#endif

 /*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

 /*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Initialize the required modules of BIOS video drivers
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 System_bspInit();

/**
 *******************************************************************************
 *
 * \brief De-initialize the previously initialized modules
 *  of BIOS video drivers
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 System_bspDeInit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BSP_INIT_H */

/* @} */
