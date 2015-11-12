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
//#include "saveLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#if 0
#pragma DATA_ALIGN(gSaveLink_tskStack, 32)
#pragma DATA_SECTION(gSaveLink_tskStack, ".bss:taskStackSection")

UInt8 gSaveLink_tskStack[Save_LINK_OBJ_MAX][Save_LINK_TSK_STACK_SIZE];
#endif
/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 SaveLink_tskCreate(UInt32 instId)
{
    Int32                status;

    status = 0;
#if 0
    SaveLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gSaveLink_obj[instId];

    sprintf(tskName, "Save%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * SaveLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             SaveLink_tskMain,
                             Save_LINK_TSK_PRI,
                             gSaveLink_tskStack[instId],
                             Save_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
#endif
    return status;
}

