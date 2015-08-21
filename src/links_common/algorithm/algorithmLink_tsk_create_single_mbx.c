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
#include "algorithmLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gAlgorithmLink_tskStack, 32)
#pragma DATA_SECTION(gAlgorithmLink_tskStack, ".bss:taskStackSection:alg")

UInt8 gAlgorithmLink_tskStack[ALGORITHM_LINK_OBJ_MAX][ALGORITHM_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_tskCreate(UInt32 instId)
{
    Int32                status;
    AlgorithmLink_Obj      *pObj;
    char                 tskName[32];

    pObj = &gAlgorithmLink_obj[instId];

    sprintf(tskName, "ALGORITHM%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * AlgorithmLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             AlgorithmLink_tskMain,
                             ALGORITHM_LINK_TSK_PRI,
                             gAlgorithmLink_tskStack[instId],
                             ALGORITHM_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

