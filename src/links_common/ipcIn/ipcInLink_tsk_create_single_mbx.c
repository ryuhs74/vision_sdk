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
#include "ipcInLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gIpcInLink_tskStack, 32)
#pragma DATA_SECTION(gIpcInLink_tskStack, ".bss:taskStackSection:ipc")

UInt8 gIpcInLink_tskStack[IPC_IN_LINK_OBJ_MAX][IPC_IN_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 IpcInLink_tskCreate(UInt32 instId)
{
    Int32                status;
    IpcInLink_obj        *pObj;
    char                 tskName[32];

    pObj = &gIpcInLink_obj[instId];

    sprintf(tskName, "IPC_IN_%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * IpcInLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             IpcInLink_tskMain,
                             IPC_LINK_TSK_PRI,
                             gIpcInLink_tskStack[instId],
                             IPC_IN_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

