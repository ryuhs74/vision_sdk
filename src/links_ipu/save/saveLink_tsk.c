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
 * \file dupLink_tsk.c
 *
 * \brief  This file has the implementation of DUP Link API
 **
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 * \version 0.0 (Jul 2013) : [NN] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "saveLink_priv.h"


/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
//DupLink_Obj gDupLink_obj[DUP_LINK_OBJ_MAX];



/**
 *******************************************************************************
 * \brief DUP Link just duplicates incoming buffers and sends across all output
 *     queues. There is no driver involved in duplicating the buffers. To keep
 *     code across all links consistent we use the same convention as
 *     DupLink_drvCreate. This function does the following,
 *
 *     - Copies the user passed create params into the link object create params
 *     - Prepares output queues
 *
 * \param  pObj     [IN]  DUP link instance handle
 * \param  pPrm     [IN]  Create params for DUP link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SaveLink_drvCreate(/*DupLink_Obj * pObj, DupLink_CreateParams * pPrm*/)
{
#if 0
    UInt32 outId, bufId;
    Int32 status;
    System_Buffer *pSysBuf;

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));
    UTILS_assert(pObj->createArgs.numOutQue <= DUP_LINK_MAX_OUT_QUE);

    pObj->getFrameCount = 0;
    pObj->putFrameCount = 0;

    status = System_linkGetInfo(
                    pObj->createArgs.inQueParams.prevLinkId, &pObj->inTskInfo);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    pObj->info.numQue = pObj->createArgs.numOutQue;

    /*
     * Copy the output queue information of previous link into the DUP link
     * output queues. Since DUP does not modify any information we need to
     * have the same output queue information as the previous link that DUP
     * link is connected to.
    */

    UTILS_assert(pObj->info.numQue <= SYSTEM_MAX_OUT_QUE);
    for (outId = 0; outId < pObj->info.numQue; outId++)
    {
        memcpy(&pObj->info.queInfo[outId],
           &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId],
           sizeof(pObj->inTskInfo.queInfo[outId]));
    }

    pObj->lock = BspOsal_semCreate(1u, TRUE);

    for (outId = 0; outId < DUP_LINK_MAX_OUT_QUE; outId++)
    {
        status = Utils_bufCreate(&pObj->outFrameQue[outId], FALSE, FALSE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        for (bufId = 0; bufId < DUP_LINK_MAX_FRAMES_PER_OUT_QUE; bufId++)
        {
            pSysBuf = &pObj->sysBufs[DUP_LINK_MAX_FRAMES_PER_OUT_QUE * outId + bufId];
            status = Utils_bufPutEmptyBuffer(&pObj->outFrameQue[outId],
                                             pSysBuf);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    memset(&pObj->stats, 0, sizeof(pObj->stats));
#endif
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief DUP Link just duplicates incoming buffers and sends across all output
 *      queues. This function does the following,
 *
 *     - Duplicates buffers and sends across it's output queues
 *     - Send SYSTEM_CMD_NEW_DATA to all it's connected links
 *
 * \param  pObj     [IN]  DUP link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SaveLink_drvProcessData(/*DupLink_Obj * pObj*/)
{
#if 0
    UInt32 outId, bufId;
    Int32 status;
    DupLink_CreateParams *pCreateArgs;
    System_Buffer *pBuf, *pOrgBuf;

    pCreateArgs = &pObj->createArgs;
    System_getLinksFullBuffers(pCreateArgs->inQueParams.prevLinkId,
                               pCreateArgs->inQueParams.prevLinkQueId,
                               &pObj->inBufList);

    if (pObj->inBufList.numBuf)
    {
        pObj->getFrameCount += pObj->inBufList.numBuf;
        pObj->stats.recvCount += pObj->inBufList.numBuf;

        for (outId = 0; outId < pCreateArgs->numOutQue; outId++)
        {
            pObj->outBufList[outId].numBuf = 0;
        }

        for (bufId = 0; bufId < pObj->inBufList.numBuf; bufId++)
        {
            pOrgBuf = pObj->inBufList.buffers[bufId];

            if(pOrgBuf == NULL)
                continue;

            pOrgBuf->dupCount = pCreateArgs->numOutQue;

            for (outId = 0; outId < pCreateArgs->numOutQue; outId++)
            {
                status = Utils_bufGetEmptyBuffer(&pObj->outFrameQue[outId],
                                                           &pBuf, BSP_OSAL_NO_WAIT);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                UTILS_assert(pBuf != NULL);

                memcpy(pBuf, pOrgBuf, sizeof(*pBuf));

                pBuf->pDupOrgFrame = (struct System_Buffer *)pOrgBuf;

                pObj->outBufList[outId].buffers[pObj->outBufList[outId].
                                                    numBuf] = pBuf;

                pObj->outBufList[outId].numBuf++;
            }
        }

        for (outId = 0; outId < pCreateArgs->numOutQue; outId++)
        {
            status = Utils_bufPutFull(&pObj->outFrameQue[outId],
                                     &pObj->outBufList[outId]);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            if (pCreateArgs->notifyNextLink)
            {
                System_sendLinkCmd(pCreateArgs->outQueParams[outId].nextLink,
                                                        SYSTEM_CMD_NEW_DATA, NULL);
            }
        }
    }
#endif
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to DUP link to get data from
 *    the output queue of DUP link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [OUT] A List of buffers needed for the next link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SaveLink_getFullBuffers(Void * ptr, UInt16 queId,
                            System_BufferList * pBufList)
{
    Int32 status;
    status = 0;
#if 0
    DupLink_Obj *pObj;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    pObj = (DupLink_Obj *) pTsk->appData;
    UTILS_assert(queId < DUP_LINK_MAX_OUT_QUE);

    status = Utils_bufGetFull(&pObj->outFrameQue[queId], pBufList,
                              BSP_OSAL_NO_WAIT);

    if(status == SYSTEM_LINK_STATUS_SOK)
    {
        pObj->stats.forwardCount[queId] += pBufList->numBuf;
    }
#endif
    return status;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to DUP link to get output queue
 *    Information of DUP link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  info     [OUT] output queues information of DUP link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SaveLink_getLinkInfo(Void * ptr, System_LinkInfo * info)
{
#if 0
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    DupLink_Obj *pObj = (DupLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->info, sizeof(*info));
#endif
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to DUP link to return back
 *    buffers
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [IN]  A List of buffers returned back to DUP link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SaveLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                              System_BufferList * pBufList)
{
	Int32 status;
	status = 0;
#if 0
    UInt32 bufId;

    System_BufferList freeBufferList;
    System_Buffer *pBuf, *pOrgBuf;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    DupLink_Obj *pObj = (DupLink_Obj *) pTsk->appData;

    UTILS_assert(queId < DUP_LINK_MAX_OUT_QUE);
    UTILS_assert(queId < pObj->createArgs.numOutQue);

    freeBufferList.numBuf = 0;

    BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);

    pObj->stats.releaseCount[queId] += pBufList->numBuf;

    for (bufId = 0; bufId < pBufList->numBuf; bufId++)
    {
        pBuf = pBufList->buffers[bufId];

        if (pBuf == NULL)
           continue;

        pOrgBuf = (System_Buffer *)pBuf->pDupOrgFrame;
        UTILS_assert(pOrgBuf != NULL);

        pOrgBuf->dupCount--;

        if (pOrgBuf->dupCount == 0)
        {
            freeBufferList.buffers[freeBufferList.numBuf] = pOrgBuf;
            freeBufferList.numBuf++;
        }
    }
    pObj->putFrameCount += freeBufferList.numBuf;

    System_putLinksEmptyBuffers(pObj->createArgs.inQueParams.prevLinkId,
                               pObj->createArgs.inQueParams.prevLinkQueId,
                               &freeBufferList);
    BspOsal_semPost(pObj->lock);

    status = Utils_bufPutEmpty(&pObj->outFrameQue[queId], pBufList);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
#endif
    return status;
}

/**
 *******************************************************************************
 * \brief Function to delete DUP link. This will simply delete all output
 *    queues and the semaphore
 *
 * \param  pObj     [IN]  DUP link instance handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SaveLink_drvDelete(/*DupLink_Obj * pObj*/)
{
	Int32 status;
	status = 0;
#if 0
    UInt32 outId;


    for (outId = 0; outId < DUP_LINK_MAX_OUT_QUE; outId++)
    {
        status = Utils_bufDelete(&pObj->outFrameQue[outId]);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
    BspOsal_semDelete(&pObj->lock);
#endif
    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating DUP link
 *     - Arrival of new data
 *     - Deleting DUP link
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Void SaveLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{

#if 0
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = 0;

    DupLink_Obj *pObj = (DupLink_Obj*) pTsk->appData;

    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:
            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
                status = DupLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));
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
                status = DupLink_drvProcessData(pObj);
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_DELETE:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                DupLink_drvDelete(pObj);
                pObj->state = SYSTEM_LINK_STATE_IDLE;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        default:
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
    }
#endif
    return;
}

Int32 SaveLink_init()
{
	Int32 status;
	status = 0;
#if 0
	System_LinkObj linkObj;
	UInt32 saveId;
	DupLink_Obj *pObj;
	UInt32 procId = System_getSelfProcId();

	for(dupId = 0; dupId < DUP_LINK_OBJ_MAX; dupId++)
	{
		pObj = &gDupLink_obj[dupId];

		memset(pObj, 0, sizeof(*pObj));

		pObj->tskId = SYSTEM_MAKE_LINK_ID(procId,
										  SYSTEM_LINK_ID_DUP_0 + dupId);

		pObj->state = SYSTEM_LINK_STATE_IDLE;

		linkObj.pTsk = &pObj->tsk;
		linkObj.linkGetFullBuffers = DupLink_getFullBuffers;
		linkObj.linkPutEmptyBuffers = DupLink_putEmptyBuffers;
		linkObj.getLinkInfo = DupLink_getLinkInfo;

		System_registerLink(pObj->tskId, &linkObj);

		status = DupLink_tskCreate(dupId);
		UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
	}
#endif
	return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for DUP link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SaveLink_deInit()
 {
#if 0
    UInt32 dupId;

    for(dupId = 0; dupId < DUP_LINK_OBJ_MAX; dupId++)
    {
        Utils_tskDelete(&gDupLink_obj[dupId].tsk);
    }
#endif
    return SYSTEM_LINK_STATUS_SOK;
 }
