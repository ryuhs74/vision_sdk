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

    pObj = &gSystemLink_obj;

    status = Utils_tskMultiMbxCreate(&pObj->tsk,
                             System_getTskMultiMbxHndl(),
                             SystemLink_tskMain,
                             UTILS_TASK_MULTI_MBX_PRI_LOWEST,
                             pObj);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

