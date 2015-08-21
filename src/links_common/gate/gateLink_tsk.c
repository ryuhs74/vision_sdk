/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file gateLink_tsk.c
 *
 * \brief  This file has the implementation of GATE Link API
 **
 *         This file implements the state machine logic for this link.
 *         A message command will cause the state machine
 *         to take some action and then move to a different state.
 *
 * \version 0.0 (Apr 2015) : [YM] First version
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
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
GateLink_Obj gGateLink_obj[GATE_LINK_OBJ_MAX];

/**
 *******************************************************************************
 * \brief Gate link allows application writers to have connections with links
 *        which will be instantiated later in the course of execution.
 *        Based on the state this is in, it either returns buffer immediately
 *        or forwards it to the next link.
 *
 *
 *        This function does the following,
 *
 *        - Copies the user passed create params into the link object
 *          create params
 *        - Based on prevLinkIsCreated calls linkGetInfo or expects data about
 *          "would be" previous link from create parameters
 *
 * \param  pObj     [IN]  GATE link instance handle
 * \param  pPrm     [IN]  Create params for GATE link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 GateLink_drvCreate(GateLink_Obj * pObj, GateLink_CreateParams * pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    pObj->opStatus = GATE_LINK_OPERATION_STATUS_OFF;

    if(pObj->createArgs.prevLinkIsCreated)
    {
        status = System_linkGetInfo(
                        pObj->createArgs.inQueParams.prevLinkId, &pObj->createArgs.prevLinkInfo);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->createArgs.prevLinkInfo.numQue);
    }

    pObj->lock = BspOsal_semCreate(1u, TRUE);

    pObj->bufCount = 0;

    return status;
}

/**
 *******************************************************************************
 * \brief GATE Link based on its state either forwards buffers to next link
 *        or returns buffers to previous link, it doesnt have any input or
 *        output queues of its own.
 *
 * \param  pObj     [IN]  GATE link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 GateLink_drvProcessData(GateLink_Obj * pObj)
{
    GateLink_CreateParams *pCreateArgs;
    System_BufferList bufList;

    pCreateArgs = &pObj->createArgs;

    if (pObj->opStatus == GATE_LINK_OPERATION_STATUS_ON)
    {
        System_sendLinkCmd(pCreateArgs->outQueParams.nextLink,
                        SYSTEM_CMD_NEW_DATA, NULL);
    }
    else if(pObj->opStatus == GATE_LINK_OPERATION_STATUS_OFF)
    {
        System_getLinksFullBuffers(
                    pCreateArgs->inQueParams.prevLinkId,
                    pCreateArgs->inQueParams.prevLinkQueId,
                    &bufList);

        /* Return the buffers to previous link */
        System_putLinksEmptyBuffers(
                    pObj->createArgs.inQueParams.prevLinkId,
                    pObj->createArgs.inQueParams.prevLinkQueId,
                    &bufList);

    }
    else
    {
        UTILS_assert(0);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to GATE link to get data from
 *    the output queue of GATE link's previous link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [OUT] A List of buffers needed for the next link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 GateLink_getFullBuffers(Void * ptr, UInt16 queId,
                            System_BufferList * pBufList)
{
    GateLink_Obj *pObj;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    GateLink_CreateParams *pCreateArgs;

    pObj = (GateLink_Obj *) pTsk->appData;

    BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);
    pCreateArgs = &pObj->createArgs;
    if(pObj->opStatus == GATE_LINK_OPERATION_STATUS_ON)
    {
        System_getLinksFullBuffers(
                    pCreateArgs->inQueParams.prevLinkId,
                    pCreateArgs->inQueParams.prevLinkQueId,
                    pBufList);

        pObj->bufCount += pBufList->numBuf;
    }
    else if(pObj->opStatus == GATE_LINK_OPERATION_STATUS_OFF)
    {
        pBufList->numBuf = 0;
    }
    else
    {
        UTILS_assert(0);
    }
    BspOsal_semPost(pObj->lock);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to GATE link to get output queue
 *    Information of GATE link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  info     [OUT] output queues information of GATE link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 GateLink_getLinkInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    GateLink_Obj *pObj = (GateLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->createArgs.prevLinkInfo, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to GATE link to return back
 *    buffers
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [IN]  A List of buffers returned back to GATE link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 GateLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                              System_BufferList * pBufList)
{

    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    GateLink_Obj *pObj = (GateLink_Obj *) pTsk->appData;

    BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);
    System_putLinksEmptyBuffers(
                    pObj->createArgs.inQueParams.prevLinkId,
                    pObj->createArgs.inQueParams.prevLinkQueId,
                    pBufList);

    pObj->bufCount -= pBufList->numBuf;
    UTILS_assert(pObj->bufCount >= 0);
    BspOsal_semPost(pObj->lock);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function to delete GATE link. This will simply delete all output
 *    queues and the semaphore
 *
 * \param  pObj     [IN]  GATE link instance handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 GateLink_drvDelete(GateLink_Obj * pObj)
{

    BspOsal_semDelete(&pObj->lock);
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating GATE link
 *     - Arrival of new data
 *     - Deleting GATE link
 *     - Changing operation status
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Void GateLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = 0;

    GateLink_Obj *pObj = (GateLink_Obj*) pTsk->appData;

    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:
            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
                status = GateLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));
                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    pObj->state = SYSTEM_LINK_STATE_RUNNING;
                }
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_NEW_DATA:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                status = GateLink_drvProcessData(pObj);
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_DELETE:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                GateLink_drvDelete(pObj);
                pObj->state = SYSTEM_LINK_STATE_IDLE;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case GATE_LINK_CMD_SET_OPERATION_MODE_ON:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);
                pObj->opStatus = GATE_LINK_OPERATION_STATUS_ON;
                BspOsal_semPost(pObj->lock);
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case GATE_LINK_CMD_SET_OPERATION_MODE_OFF:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);
                pObj->opStatus = GATE_LINK_OPERATION_STATUS_OFF;
                BspOsal_semPost(pObj->lock);
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case GATE_LINK_CMD_GET_BUFFER_FORWARD_COUNT:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                UInt32 *bufCount = (UInt32 *) Utils_msgGetPrm(pMsg);
                *bufCount = pObj->bufCount;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;


        default:
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
    }

    return;
}

/**
 *******************************************************************************
 *
 * \brief Init function for GATE link. This function does the following for each
 *   GATE link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 GateLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 gateId;
    GateLink_Obj *pObj;
    UInt32 procId = System_getSelfProcId();

    for(gateId = 0; gateId < GATE_LINK_OBJ_MAX; gateId++)
    {
        pObj = &gGateLink_obj[gateId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->tskId = SYSTEM_MAKE_LINK_ID(procId,
                                          SYSTEM_LINK_ID_GATE_0 + gateId);

        pObj->state = SYSTEM_LINK_STATE_IDLE;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers  = GateLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = GateLink_putEmptyBuffers;
        linkObj.getLinkInfo = GateLink_getLinkInfo;

        System_registerLink(pObj->tskId, &linkObj);

        status = GateLink_tskCreate(gateId);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for GATE link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 GateLink_deInit()
 {
    UInt32 gateId;

    for(gateId = 0; gateId < GATE_LINK_OBJ_MAX; gateId++)
    {
        Utils_tskDelete(&gGateLink_obj[gateId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
 }
