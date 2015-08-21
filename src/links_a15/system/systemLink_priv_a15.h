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
 * \ingroup SYSTEM_A15_LINK_API
 *
 * \defgroup SYSTEM_A15_LINK_IMPL Processor Link API: A15 Core0 Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file systemLink_priv_a15.h A15 private file containing all
 *                             the header files util files required by A15
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SYSTEMLINK_PRIV_A15_H_
#define _SYSTEMLINK_PRIV_A15_H_

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

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  system link object handle
 *          Contains LinkObject Handle and TaskId
 *
 *******************************************************************************
*/
typedef struct {
    UInt32 tskId;
    /**< SystemLink Task Id */

    Utils_TskHndl tsk;
    /**< SystemLink Task Handle */

} SystemLink_Obj;

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
