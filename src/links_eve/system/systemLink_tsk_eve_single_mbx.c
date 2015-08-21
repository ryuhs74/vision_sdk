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
#include "systemLink_priv_eve.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSystemLink_tskStack, 32)
#pragma DATA_SECTION(gSystemLink_tskStack, ".bss:taskStackSection")

UInt8 gSystemLink_tskStack[SYSTEM_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 SystemLink_tskCreate()
{
    Int32                status;
    SystemLink_Obj      *pObj;
    char                 tskName[32];

    pObj = &gSystemLink_obj;

    sprintf(tskName, "SYSTEM_EVE_%u", (unsigned int)pObj->tskId);

    /*
     * Create link task, task remains in IDLE state.
     * SystemLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             SystemLink_tskMain,
                             SYSTEM_TSK_PRI,
                             gSystemLink_tskStack,
                             SYSTEM_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

