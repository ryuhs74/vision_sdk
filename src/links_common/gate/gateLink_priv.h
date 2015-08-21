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
 * \ingroup GATE_LINK_API
 * \defgroup GATE_LINK_IMPL Gate Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file gateLink_priv.h Gate Link private API/Data structures
 *
 * \brief  This file is a private header file for gate link implementation
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of gate link.
 *
 * \version 0.0 (Apr 2015) : [YM] First version
 *
 *******************************************************************************
 */

#ifndef _GATE_LINK_PRIV_H_
#define _GATE_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/gateLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Maximum number of gate link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GATE_LINK_OBJ_MAX    (4)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *  \brief Enumerations for AVB Src notification types
 *
 *  SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
typedef enum
{
    GATE_LINK_OPERATION_STATUS_ON,
    /**< Switches the operation on at runtime
     *   It it indicates gate link can forward the data to next link */

    GATE_LINK_OPERATION_STATUS_OFF,
    /**< Switches the operation off at runtime
     *   It it indicates gate link can not forward the data to next link
     */

    GATE_LINK_OPERATION_STATUE_FORCE32BITS = 0x7FFFFFFF
     /**< This should be the last value after the max enumeration value.
      *   This is to make sure enum size defaults to 32 bits always regardless
      *   of compiler.
      */
} GateLink_OperationStatus;



/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Structure to hold all Gate link related information
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 tskId;
    /**< Placeholder to store gate link task id */

    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */

    Utils_TskHndl tsk;
    /**< Handle to capture link task */

    GateLink_OperationStatus opStatus;
    /**< Decision on received buffer is taken based on this,
     *   Buffers are forwarded as it is or returned back
     */

    Int32 bufCount;
    /**< This signifies number of buffers to be released by
      *  gate link, this is useful while turning opStatus on->off
      *  or off->on */

    BspOsal_SemHandle lock;
    /**< Link level lock, used while updating the link params */

    GateLink_CreateParams createArgs;
    /**< Create params for gate link */

} GateLink_Obj;

extern GateLink_Obj gGateLink_obj[];

/**
 *******************************************************************************
 *
 *   \brief Gate link's task
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void GateLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

/**
 *******************************************************************************
 *
 *   \brief Gate link's task creation fuciton
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 GateLink_tskCreate(UInt32 instId);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


