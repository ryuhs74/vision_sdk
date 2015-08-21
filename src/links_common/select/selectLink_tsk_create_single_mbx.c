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
#include "selectLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSelectLink_tskStack, 32)
#pragma DATA_SECTION(gSelectLink_tskStack, ".bss:taskStackSection")

UInt8 gSelectLink_tskStack[SELECT_LINK_OBJ_MAX][SELECT_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 SelectLink_tskCreate(UInt32 instId)
{
    Int32                status;
    SelectLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gSelectLink_obj[instId];

    sprintf(tskName, "SELECT%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * SelectLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             SelectLink_tskMain,
                             SELECT_LINK_TSK_PRI,
                             gSelectLink_tskStack[instId],
                             SELECT_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

