/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "dupLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gDupLink_tskStack, 32)
#pragma DATA_SECTION(gDupLink_tskStack, ".bss:taskStackSection")

UInt8 gDupLink_tskStack[DUP_LINK_OBJ_MAX][DUP_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 DupLink_tskCreate(UInt32 instId)
{
    Int32                status;
    DupLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gDupLink_obj[instId];

    sprintf(tskName, "DUP%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * DupLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             DupLink_tskMain,
                             DUP_LINK_TSK_PRI,
                             gDupLink_tskStack[instId],
                             DUP_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

