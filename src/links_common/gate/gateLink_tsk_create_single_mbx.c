/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "gateLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gGateLink_tskStack, 32)
#pragma DATA_SECTION(gGateLink_tskStack, ".bss:taskStackSection")

UInt8 gGateLink_tskStack[GATE_LINK_OBJ_MAX][GATE_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 GateLink_tskCreate(UInt32 instId)
{
    Int32                status;
    GateLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gGateLink_obj[instId];

    sprintf(tskName, "GATE%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * GateLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             GateLink_tskMain,
                             GATE_LINK_TSK_PRI,
                             gGateLink_tskStack[instId],
                             GATE_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

