/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file mergeLink_tsk.c
 *
 * \brief  This file has the implementation of MERGE Link API
 **
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 * \version 0.0 (Aug 2013) : [SL] First version
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
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
MergeLink_Obj gMergeLink_obj[MERGE_LINK_OBJ_MAX];

/**
 *******************************************************************************
 * \brief MERGE Link collects incoming buffers from multiple links and merges
 *     them into single output queue. There is no driver involved in merging the
 *     buffers. To keep code across all links consistent we use the same
 *     convention as MergeLink_drvCreate. This function does the following,
 *
 *     - Copies the user passed create params into the link object create params
 *     - Prepares output queue
 *
 * \param  pObj     [IN]  MERGE link instance handle
 * \param  pPrm     [IN]  Create params for MERGE link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 MergeLink_drvCreate(MergeLink_Obj * pObj, MergeLink_CreateParams * pPrm)
{
    Int32 status;
    System_LinkQueInfo *pInQueInfo, *pOutQueInfo;
    UInt32 inQue, numOutCh, chId;

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    UTILS_assert(pPrm->numInQue <= MERGE_LINK_MAX_IN_QUE);

    status = Utils_bufCreate(&pObj->outBufQue, FALSE, FALSE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pObj->lock = BspOsal_semCreate(1u, TRUE);
    UTILS_assert(pObj->lock != NULL);

    numOutCh = 0;

    pOutQueInfo = &pObj->info.queInfo[0];

    for (inQue = 0; inQue < pPrm->numInQue; inQue++)
    {
        status =
            System_linkGetInfo(pPrm->inQueParams[inQue].prevLinkId,
                               &pObj->inTskInfo[inQue]);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        UTILS_assert(pPrm->inQueParams[inQue].prevLinkQueId <
                     pObj->inTskInfo[inQue].numQue);

        pInQueInfo = &pObj->inTskInfo[inQue].queInfo
            [pPrm->inQueParams[inQue].prevLinkQueId];

        UTILS_assert(pInQueInfo->numCh <= SYSTEM_MAX_CH_PER_OUT_QUE);
        for (chId = 0; chId < pInQueInfo->numCh; chId++)
        {
            UTILS_assert(chId < SYSTEM_MAX_CH_PER_OUT_QUE);
            UTILS_assert(numOutCh < SYSTEM_MAX_CH_PER_OUT_QUE);

            memcpy(&pOutQueInfo->chInfo[numOutCh],
                   &pInQueInfo->chInfo[chId],
                   sizeof(pOutQueInfo->chInfo[numOutCh]));

            pObj->inQueChNumMap[inQue][chId] = numOutCh;
            pObj->outQueChToInQueMap[numOutCh] = inQue;
            pObj->outQueChMap[numOutCh] = chId;

            numOutCh++;
        }

        pObj->inQueMaxCh[inQue] = pInQueInfo->numCh;
    }

    pObj->info.numQue = 1;
    pOutQueInfo->numCh = numOutCh;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function to delete MERGE link. This will simply delete the output
 *    queue and the semaphore
 *
 * \param  pObj     [IN]  MERGE link instance handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 MergeLink_drvDelete(MergeLink_Obj * pObj)
{
    Int32 status;

    status = Utils_bufDelete(&pObj->outBufQue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    BspOsal_semDelete(&pObj->lock);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to MERGE link to get output queue
 *    Information of MERGE link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  info     [OUT] output queue information of MERGE link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 MergeLink_getLinkInfo(Void * ptr, System_LinkInfo * info)
{
  Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    MergeLink_Obj *pObj = (MergeLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->info, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to MERGE link to get data from
 *    the output queue of MERGE link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [OUT] A List of buffers needed for the next link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 MergeLink_getFullBuffers(Void * ptr, UInt16 queId,
                                System_BufferList * pBufList)
{
    Int32 status;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    MergeLink_Obj *pObj = (MergeLink_Obj *) pTsk->appData;

    status =  Utils_bufGetFull(&pObj->outBufQue, pBufList, BSP_OSAL_NO_WAIT);

    if (status == 0)
    {
        pObj->stats.forwardCount += pBufList->numBuf;
    }
    return status;
}

/**
 *******************************************************************************
 * \brief MERGE Link collects incoming buffers and sends to single output queue.
 *     This function does the following,
 *
 *     - Merge buffers and sends across it's output queue
 *     - Send SYSTEM_CMD_NEW_DATA to it's connected link
 *
 * \param  pObj     [IN]  MERGE link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 MergeLink_drvProcessData(MergeLink_Obj * pObj)
{
    MergeLink_CreateParams *pCreateArgs;
    UInt32 inQue, bufId;
    Int32 status;
    System_Buffer *pBuf;
    Bool newDataAvailable;

    newDataAvailable = FALSE;

    pCreateArgs = &pObj->createArgs;

    for (inQue = 0; inQue < pCreateArgs->numInQue; inQue++)
    {
        System_getLinksFullBuffers(pCreateArgs->inQueParams[inQue].prevLinkId,
                                  pCreateArgs->inQueParams[inQue].prevLinkQueId,
                                  &pObj->inBufList);

        if (pObj->inBufList.numBuf)
        {
            pObj->stats.recvCount[inQue] += pObj->inBufList.numBuf;
            for (bufId = 0; bufId < pObj->inBufList.numBuf; bufId++)
            {
                /* remap channel number */
                pBuf = pObj->inBufList.buffers[bufId];

                UTILS_assert(pBuf->chNum < pObj->inQueMaxCh[inQue]);

                pBuf->chNum =
                    pObj->inQueChNumMap[inQue][pBuf->chNum];
            }

            status = Utils_bufPutFull(&pObj->outBufQue, &pObj->inBufList);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            newDataAvailable = TRUE;
        }
    }

    if (pCreateArgs->notifyNextLink && newDataAvailable)
    {
        System_sendLinkCmd(pCreateArgs->outQueParams.nextLink,
                           SYSTEM_CMD_NEW_DATA, NULL);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to MERGE link to return back
 *    buffers
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [IN]  A List of buffers returned back to MERGE link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 MergeLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                                System_BufferList * pBufList)
{
    UInt32 bufId, inQue;
    MergeLink_CreateParams *pCreateArgs;
    System_Buffer *pBuf;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    MergeLink_Obj *pObj = (MergeLink_Obj *) pTsk->appData;

    BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);

    pCreateArgs = &pObj->createArgs;

    for (inQue = 0; inQue < pCreateArgs->numInQue; inQue++)
    {
        pObj->freeBufList[inQue].numBuf = 0;
    }

    for (bufId = 0; bufId < pBufList->numBuf; bufId++)
    {
        pBuf = pBufList->buffers[bufId];

        UTILS_assert(pBuf != NULL);

        inQue = pObj->outQueChToInQueMap[pBuf->chNum];

        pBuf->chNum = pObj->outQueChMap[pBuf->chNum];


        UTILS_assert(inQue < pCreateArgs->numInQue);

        pObj->freeBufList[inQue].buffers
            [pObj->freeBufList[inQue].numBuf] = pBuf;

        pObj->freeBufList[inQue].numBuf++;
        pObj->stats.releaseCount[inQue]++;
    }

    for (inQue = 0; inQue < pCreateArgs->numInQue; inQue++)
    {
        if (pObj->freeBufList[inQue].numBuf)
        {
            System_putLinksEmptyBuffers(pCreateArgs->inQueParams[inQue].
                                       prevLinkId,
                                       pCreateArgs->inQueParams[inQue].
                                       prevLinkQueId,
                                       &pObj->freeBufList[inQue]);
        }
    }

    BspOsal_semPost(pObj->lock);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating MERGE link
 *     - Arrival of new data
 *     - Deleting MERGE link
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Void MergeLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = 0;
    MergeLink_Obj *pObj = (MergeLink_Obj *) pTsk->appData;

    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:
            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
                status = MergeLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));
                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    pObj->state = SYSTEM_LINK_STATE_RUNNING;
                }
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
        case SYSTEM_CMD_DELETE:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                MergeLink_drvDelete(pObj);
                pObj->state = SYSTEM_LINK_STATE_IDLE;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
        case SYSTEM_CMD_NEW_DATA:
            Utils_tskAckOrFreeMsg(pMsg, status);

            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                MergeLink_drvProcessData(pObj);
            }
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
 * \brief Init function for MERGE link. This function does the following for
 *   each MERGE link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 MergeLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 mergeId;
    MergeLink_Obj *pObj;
    UInt32 procId = System_getSelfProcId();

    for (mergeId = 0; mergeId < MERGE_LINK_OBJ_MAX; mergeId++)
    {
        pObj = &gMergeLink_obj[mergeId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->tskId =
            SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_MERGE_0 + mergeId);

        pObj->state = SYSTEM_LINK_STATE_IDLE;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = MergeLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = MergeLink_putEmptyBuffers;
        linkObj.getLinkInfo = MergeLink_getLinkInfo;

        System_registerLink(pObj->tskId, &linkObj);

        status = MergeLink_tskCreate(mergeId);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for MERGE link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 MergeLink_deInit()
{
    UInt32 mergeId;

    for (mergeId = 0; mergeId < MERGE_LINK_OBJ_MAX; mergeId++)
    {
        Utils_tskDelete(&gMergeLink_obj[mergeId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
}
