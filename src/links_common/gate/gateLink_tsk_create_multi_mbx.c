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
#include "gateLink_priv.h"

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
    Utils_TskMultiMbxHndl *pMultiMbxHndl;

    pObj = &gGateLink_obj[instId];

    pMultiMbxHndl = System_getTskMultiMbxHndl();

    /*
     * Create link task, task remains in IDLE state.
     * GateLink_tskMain is called when a message command is received.
     */
    status = Utils_tskMultiMbxCreate(&pObj->tsk,
                             pMultiMbxHndl,
                             GateLink_tskMain,
                             UTILS_TASK_MULTI_MBX_PRI_HIGHEST,
                             pObj);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

