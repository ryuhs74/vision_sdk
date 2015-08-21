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
 * \ingroup SYSTEM_DSP_LINK_API
 *
 * \defgroup SYSTEM_DSP_LINK_IMPL Processor Link API: DSP Core0 Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file systemLink_priv_c6xdsp.h DSP private file containing all
 *                                the header files util files required by DSP
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SYSTEMLINK_PRIV_C6XDSP_H_
#define _SYSTEMLINK_PRIV_C6XDSP_H_

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
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
