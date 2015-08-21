/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file utils_iss.c
 *
 * \brief  This file has some utilities for ISS usage
 *
 * \version 0.0 (Dec 2014) : [PS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <xdc/std.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/io/GIO.h>
#include <uart/bsp_uart.h>
#include <platforms/bsp_platform.h>
#include <boards/bsp_board.h>
#include <src/utils_common/include/utils_iss.h>

/**
 *******************************************************************************
 * \brief Semaphore handle for ISS configurations
 *******************************************************************************
 */
BspOsal_SemHandle semIspLock;

/**
 *******************************************************************************
 *
 * \brief Creates the semaphore for guarding ISS configurations
 *
 * \return  None
 *
 *******************************************************************************
 */
void Utils_ispLockCreate()
{
    semIspLock = BspOsal_semCreate(0, TRUE);
    UTILS_assert(semIspLock != NULL);

    Utils_postIspLock();
}

/**
 *******************************************************************************
 *
 * \brief Pend on the semaphore for ISS configurations
 *
 * \return  None
 *
 *******************************************************************************
 */
void Utils_pendIspLock()
{
    UTILS_assert(semIspLock != NULL);
    BspOsal_semWait(semIspLock, BSP_OSAL_WAIT_FOREVER);
}

/**
 *******************************************************************************
 *
 * \brief Post the semaphore for ISS configurations
 *
 * \return  None
 *
 *******************************************************************************
 */
void Utils_postIspLock()
{
    UTILS_assert(semIspLock != NULL);
    BspOsal_semPost(semIspLock);
}
