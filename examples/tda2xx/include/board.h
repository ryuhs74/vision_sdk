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
 * \ingroup EXAMPLES_API
 * \defgroup EXAMPLE_BOARD_API Board Control API
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file board.h Board Control API
 *
 * \brief  This module has the interface for Board Initialization
 *
 *         In this function BspBoardInit functions are called.
 *         Bsp_board,I2C,Fvid2,BSP_device init and deinit functions are called
 *         This should be called first call after initializing the uart driver.
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _BOARD_
#define _BOARD_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief This function Initialize the Board related modules .
 *
 *        In this function
 *        Bsp_board
 *        I2C
 *        BSP_device are initialized
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Board_init();

/**
 *******************************************************************************
 *
 * \brief This function De-initialize the previously initialized modules .
 *
 *        In this function
 *        Bsp_board
 *        I2C
 *        BSP_device are deinitialized
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Board_deInit();

/**
 *******************************************************************************
 *
 * \brief This function probes the Board if connected or no
 *
 *        In this function
 *
 * \param   boardI2cInstId      [IN] Instance Id of board which is to be probed
 *
 * \param   boardI2cAddr        [IN] I2c Address of board which is to be probed
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Board_probe(UInt32 boardI2cInstId,UInt32 boardI2cAddr);

/**
 *******************************************************************************
 *
 * \brief This function probes the MultiDes Board if connected or no
 *
 *        In this function
 *
 * \return  TRUE on success
 *
 *******************************************************************************
 */
Bool Board_isMultiDesConnected();

/**
 *******************************************************************************
 *
 * \brief This enables charging via USB port by setting a board specific GPIO
 *
 *******************************************************************************
 */
Void Board_enableUsbCharging();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
