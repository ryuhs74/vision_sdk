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
#include "ipcOutLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gIpcOutLink_tskStack, 32)
#pragma DATA_SECTION(gIpcOutLink_tskStack, ".bss:taskStackSection:ipc")

UInt8 gIpcOutLink_tskStack[IPC_OUT_LINK_OBJ_MAX][IPC_OUT_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 IpcOutLink_tskCreate(UInt32 instId)
{
    Int32                status;
    IpcOutLink_Obj      *pObj;
    char                 tskName[32];

    pObj = &gIpcOutLink_obj[instId];

    sprintf(tskName, "IPC_OUT_%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * IpcOutLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             IpcOutLink_tskMain,
                             IPC_LINK_TSK_PRI,
                             gIpcOutLink_tskStack[instId],
                             IPC_OUT_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

