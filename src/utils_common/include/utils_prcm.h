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
 * \ingroup UTILS_API
 * \defgroup UTILS_PRCM_API PRCM init APIs when linux run's on A15
 *
 * \brief  APIs to initialize prcm from IPU.
 *         Only applicable when linux runs on A15
 *
 * @{
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file utils_prcm.h
 *
 * \brief PRCM init APIs when linux run's on A15
 *
 * \version 0.0 First version
 * \version 0.1 Updates as per code review comments
 *
 *******************************************************************************
*/

#ifndef _UTILS_PRCM_H_
#define _UTILS_PRCM_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>

/*******************************************************************************
 *  Functions
 *******************************************************************************
*/
Void Utils_prcmInit();


#endif

/* @} */
