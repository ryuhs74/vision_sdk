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
#include "mergeLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gMergeLink_tskStack, 32)
#pragma DATA_SECTION(gMergeLink_tskStack, ".bss:taskStackSection")

UInt8 gMergeLink_tskStack[MERGE_LINK_OBJ_MAX][MERGE_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 MergeLink_tskCreate(UInt32 instId)
{
    Int32                status;
    MergeLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gMergeLink_obj[instId];

    sprintf(tskName, "MERGE%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * MergeLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             MergeLink_tskMain,
                             MERGE_LINK_TSK_PRI,
                             gMergeLink_tskStack[instId],
                             MERGE_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

