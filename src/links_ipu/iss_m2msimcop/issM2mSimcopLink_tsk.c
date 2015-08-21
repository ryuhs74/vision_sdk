/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "issM2mSimcopLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_TSK_STACK_SIZE (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#pragma DATA_ALIGN(gIssM2mSimcopLink_tskStack, 32)
#pragma DATA_SECTION(gIssM2mSimcopLink_tskStack, ".bss:taskStackSection")
UInt8 gIssM2mSimcopLink_tskStack[ISSM2MSIMCOP_LINK_OBJ_MAX][ISSM2MSIMCOP_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
IssM2mSimcopLink_Obj gIssM2mSimcopLink_obj[ISSM2MSIMCOP_LINK_OBJ_MAX];


/**
 *******************************************************************************
 *
 * \brief Link main function
 *
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void IssM2mSimcopLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Bool ackMsg, done, stopDone;
    Int32 status;
    IssM2mSimcopLink_Obj *pObj;

    /* IDLE state */

    pObj = (IssM2mSimcopLink_Obj *) pTsk->appData;

    if (cmd != SYSTEM_CMD_CREATE)
    {
        /* invalid command recived in IDLE status, be in IDLE state and ACK
         * with error status */
        Utils_tskAckOrFreeMsg(pMsg, FVID2_EFAIL);
        return;
    }

    /* Create command received, create the driver */
    status = IssM2mSimcopLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

    /* ACK based on create status */
    Utils_tskAckOrFreeMsg(pMsg, status);

    /* if create status is error then remain in IDLE state */
    if (status != SYSTEM_LINK_STATUS_SOK)
        return;

    /* create success, entering READY state */

    done = FALSE;
    ackMsg = FALSE;
    stopDone = FALSE;

    /* READY state loop */
    while (!done)
    {
        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pMsg);

        if(stopDone==TRUE && cmd!=SYSTEM_CMD_DELETE)
        {
            /* once stop is done, only DELETE command should be accepted */
            Utils_tskAckOrFreeMsg(pMsg, status);
            continue;
        }

        switch (cmd)
        {
            case SYSTEM_CMD_NEW_DATA:
                /* new data frames have been captured, process them */

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pMsg, status);

                IssM2mSimcopLink_drvProcessData(pObj);
                break;

            case ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG:

                status = IssM2mSimcopLink_drvSetSimcopConfig(pObj,
                                Utils_msgGetPrm(pMsg)
                            );
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                /* new data frames have been captured, process them */
                IssM2mSimcopLink_drvPrintStatus(pObj);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            case ISSM2MSIMCOP_LINK_CMD_SAVE_FRAME:

                IssM2mSimcopLink_drvSaveFrame(pObj);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pMsg, status);

                break;
            case ISSM2MSIMCOP_LINK_CMD_GET_SAVE_FRAME_STATUS:

                status = IssM2mSimcopLink_drvGetSaveFrameStatus(pObj,
                            Utils_msgGetPrm(pMsg));

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            case SYSTEM_CMD_STOP:
                /* Nothing to do, just ACK mesage */
                Utils_tskAckOrFreeMsg(pMsg, status);
                stopDone = TRUE;
                break;

            case SYSTEM_CMD_DELETE:

                /* exit READY state */
                done = TRUE;
                ackMsg = TRUE;
                break;
            default:
                /* invalid command for this state ACK it and continue READY
                 * loop */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    /* exiting READY state, delete driver */
    IssM2mSimcopLink_drvDelete(pObj);

    /* ACK message if not previously ACK'ed */
    if (ackMsg && pMsg != NULL)
        Utils_tskAckOrFreeMsg(pMsg, status);

    /* entering IDLE state */
    return;
}

/**
 *******************************************************************************
 *
 * \brief Init function for link. BIOS task for the
 *  link gets created / registered in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 IssM2mSimcopLink_init()
{
    Int32 status;
    UInt32 instId;

    System_LinkObj linkObj;
    IssM2mSimcopLink_Obj *pObj;

    /* register link with system API */
    for(instId = 0; instId < ISSM2MSIMCOP_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIssM2mSimcopLink_obj[instId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId = SYSTEM_LINK_ID_ISSM2MSIMCOP_0 + instId;

        linkObj.pTsk                = &pObj->tsk;
        linkObj.linkGetFullBuffers  = IssM2mSimcopLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = IssM2mSimcopLink_putEmptyBuffers;
        linkObj.getLinkInfo         = IssM2mSimcopLink_getInfo;

        System_registerLink(pObj->linkId, &linkObj);

        /* Create link task, task remains in IDLE state
         * IssM2mSimcopLink_tskMain is called when a message command is received
         */

        status = Utils_tskCreate(&pObj->tsk,
            IssM2mSimcopLink_tskMain,
            ISSM2MSIMCOP_LINK_TSK_PRI,
            &gIssM2mSimcopLink_tskStack[instId][0],
            ISSM2MSIMCOP_LINK_TSK_STACK_SIZE, pObj, "ISSM2MSIMCOP ");

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-Init function for capture link. BIOS task for capture
 *        link gets deleted in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IssM2mSimcopLink_deInit()
{
    IssM2mSimcopLink_Obj *pObj;
    UInt32 instId;

    for(instId = 0; instId < ISSM2MSIMCOP_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIssM2mSimcopLink_obj[instId];

        /*
         * Delete link task
         */
        Utils_tskDelete(&pObj->tsk);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get the buffer and queue information about link.
 *
 * \param  ptr  [IN] Task Handle
 * \param  info [IN] Pointer to link information handle

 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IssM2mSimcopLink_getInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    IssM2mSimcopLink_Obj *pObj = (IssM2mSimcopLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->linkInfo, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to give full buffers to next
 *        link.
 *
 * Link sends message to next link about availability of buffers.
 * Next link calls this callback function to get full buffers from this link
 * output queue.
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId from which buffers are required.
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IssM2mSimcopLink_getFullBuffers(Void * ptr, UInt16 queId,
                                 System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    IssM2mSimcopLink_Obj *pObj = (IssM2mSimcopLink_Obj *) pTsk->appData;
    Int32 status;
    Int32 idx;

    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->linkStatsInfo->linkStats.getFullBufCount++;

    /* IssM2mSimcopLink_Obj uses a two queues. Hence queId is used here */
    for (idx = 0; idx < SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST; idx++)
    {
        status = Utils_queGet(&pObj->fullBufQue,
                              (Ptr *) & pBufList->buffers[idx],
                              1,
                              BSP_OSAL_NO_WAIT);

        if (status != SYSTEM_LINK_STATUS_SOK)
            break;
    }

    pBufList->numBuf = idx;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to get empty buffers from next
 *        link.
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId from which buffers are required.
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IssM2mSimcopLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                                  System_BufferList * pBufList)
{
    Int32 status;
    Int32 idx;
    System_Buffer *pBuf;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    IssM2mSimcopLink_Obj *pObj = (IssM2mSimcopLink_Obj *) pTsk->appData;

    for (idx = 0; idx < pBufList->numBuf; idx++)
    {
        pBuf = pBufList->buffers[idx];

        if(pBuf && pBuf->chNum<ISSM2MSIMCOP_LINK_MAX_CH)
        {
            status = Utils_quePut(&pObj->chObj[pBuf->chNum].emptyBufQue,
                               pBufList->buffers[idx],
                               BSP_OSAL_NO_WAIT);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}
