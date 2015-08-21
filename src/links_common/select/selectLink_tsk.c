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
 * \file selectLink_tsk.c
 *
 * \brief  This file has the implementation of Select Link API
 **
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 * \version 0.0 (Nov 2013) : [CM] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "selectLink_priv.h"


/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
SelectLink_Obj gSelectLink_obj[SELECT_LINK_OBJ_MAX];

/**
 *******************************************************************************
 * \brief
 *
 *     - Gets the out queue channel information
 *     - Prepares output queues
 *
 * \param  pObj     [IN]  Select link instance handle
 * \param  pPrm     [IN]  Create params for SELECT link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/

Int32 SelectLink_drvGetOutQueChInfo(SelectLink_Obj * pObj,
                                        SelectLink_OutQueChInfo *pPrm)
{
    UInt32 outChNum;
    SelectLink_OutQueChInfo *pPrevPrm;

    pPrm->numOutCh = 0;

    if(pPrm->outQueId > pObj->createArgs.numOutQue)
        return SYSTEM_LINK_STATUS_EFAIL;

    pPrevPrm = &pObj->prevOutQueChInfo[pPrm->outQueId];

    /* copy current output que info to user supplied pointer */
    pPrm->numOutCh = pPrevPrm->numOutCh;

    if(pPrm->numOutCh > SYSTEM_MAX_CH_PER_OUT_QUE)
    {
        pPrm->numOutCh = 0;
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    for(outChNum=0; outChNum<pPrevPrm->numOutCh; outChNum++)
    {
        pPrm->inChNum[outChNum] = pPrevPrm->inChNum[outChNum];
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief
 *
 *     - Sets the out queue channel information
 *     - Prepares output queues
 *
 * \param  pObj     [IN]  Select link instance handle
 * \param  pPrm     [IN]  Create params for SELECT link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SelectLink_drvSetOutQueChInfo(SelectLink_Obj *pObj,
                                    SelectLink_OutQueChInfo *pPrm)
{
    UInt32 inChNum, outChNum;
    SelectLink_ChInfo *pChInfo;
    SelectLink_OutQueChInfo *pPrevPrm;

    if(pPrm->numOutCh > SYSTEM_MAX_CH_PER_OUT_QUE)
        return SYSTEM_LINK_STATUS_EFAIL;

    if(pPrm->outQueId > pObj->createArgs.numOutQue)
        return SYSTEM_LINK_STATUS_EFAIL;

    pPrevPrm = &pObj->prevOutQueChInfo[pPrm->outQueId];

    /* remove prev output queue channel mapping */
    for(outChNum=0; outChNum<pPrevPrm->numOutCh; outChNum++)
    {
        inChNum = pPrevPrm->inChNum[outChNum];

        if(inChNum >= SYSTEM_MAX_CH_PER_OUT_QUE)
        {
            Vps_printf(" SELECT   : Invalid input channel number (%d) \n",
                    inChNum
                );
            continue;
        }

        pChInfo = &pObj->inChInfo[inChNum];

        pChInfo->queId = SELECT_LINK_CH_NOT_MAPPED;
        pChInfo->outChNum = 0;
        pChInfo->rtChInfoUpdate = FALSE;
    }

    /* mapped input to output channels */
    for(outChNum=0; outChNum<pPrm->numOutCh; outChNum++)
    {
        inChNum = pPrm->inChNum[outChNum];

        if(inChNum >= SYSTEM_MAX_CH_PER_OUT_QUE)
        {
            Vps_printf(" SELECT   : Invalid input channel number (%d) \n",
                    inChNum
                );
            continue;
        }

        pChInfo = &pObj->inChInfo[inChNum];

        pChInfo->queId = pPrm->outQueId;
        pChInfo->outChNum = outChNum;
        pChInfo->rtChInfoUpdate = TRUE;

        Vps_printf(" SELECT: OUT QUE%d: OUT CH%d: IN CH%d: %d x %d,"
                   " pitch = (%d, %d) \n",
                pChInfo->queId ,
                outChNum,
                inChNum,
                pChInfo->rtChInfo.width,
                pChInfo->rtChInfo.height,
                pChInfo->rtChInfo.pitch[0],
                pChInfo->rtChInfo.pitch[1]
            );

    }

    *pPrevPrm = *pPrm;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief
 *
 *     - Copies the user passed create params into the link object create params
 *     - Prepares output queues
 *
 * \param  pObj     [IN]  Select link instance handle
 * \param  pPrm     [IN]  Create params for SELECT link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SelectLink_drvCreate(SelectLink_Obj * pObj,
                            SelectLink_CreateParams * pPrm)
{
    UInt32 outQueId, inChId, outChId;
    Int32 status;
    System_LinkQueInfo *pInQueInfo;
    System_LinkQueInfo *pOutQueInfo;
    SelectLink_ChInfo  *pInChInfo;
    SelectLink_OutQueChInfo *pOutQueChInfo;

    /* copy create args */
    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    UTILS_assert(pObj->createArgs.numOutQue <= SELECT_LINK_MAX_OUT_QUE);

    /* get previous link info */
    status = System_linkGetInfo(pObj->createArgs.inQueParams.prevLinkId,
                                    &pObj->inTskInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    UTILS_assert(pPrm->inQueParams.prevLinkQueId < pObj->inTskInfo.numQue);

    /* point to correct previous link output que info */
    pInQueInfo = &pObj->inTskInfo.queInfo[pPrm->inQueParams.prevLinkQueId];

    /* mark all Chs as not mapped */
    for(inChId=0; inChId<SYSTEM_MAX_CH_PER_OUT_QUE; inChId++)
    {
        pInChInfo = &pObj->inChInfo[inChId];

        pInChInfo->queId = SELECT_LINK_CH_NOT_MAPPED;
        pInChInfo->outChNum = 0;
        pInChInfo->rtChInfoUpdate = FALSE;

        memset(&pInChInfo->rtChInfo, 0, sizeof(pInChInfo->rtChInfo));
    }

    /* copy previous link channel info to local params */
    for(inChId=0; inChId<pInQueInfo->numCh; inChId++)
    {
        pInChInfo = &pObj->inChInfo[inChId];

        pInChInfo->rtChInfo = pInQueInfo->chInfo[inChId];
    }

    /* create out que info */
    pObj->info.numQue = pObj->createArgs.numOutQue;

    for (outQueId = 0; outQueId < pObj->createArgs.numOutQue; outQueId++)
    {
        status = Utils_bufCreate(&pObj->outFrameQue[outQueId], FALSE, FALSE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        pObj->prevOutQueChInfo[outQueId].outQueId = outQueId;
        pObj->prevOutQueChInfo[outQueId].numOutCh = 0;

        pOutQueChInfo = &pObj->createArgs.outQueChInfo[outQueId];
        pOutQueInfo   = &pObj->info.queInfo[outQueId];

        pOutQueChInfo->outQueId = outQueId;

        status = SelectLink_drvSetOutQueChInfo(pObj, pOutQueChInfo);
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

        pOutQueInfo->numCh = pOutQueChInfo->numOutCh;

        for(outChId=0; outChId<pOutQueInfo->numCh; outChId++)
        {
            inChId = pOutQueChInfo->inChNum[outChId];

            if(inChId >= SYSTEM_MAX_CH_PER_OUT_QUE)
            {
                Vps_printf(" SELECT   : Invalid input channel number (%d) \
                                specified, ignoring it !!!\n",
                        inChId
                    );
                continue;
            }

            pInChInfo = &pObj->inChInfo[inChId];

            pOutQueInfo->chInfo[outChId] = pInChInfo->rtChInfo;
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief SELECT Link just duplicates incoming buffers and sends across all output
 *      queues. This function does the following,
 *
 *     - Send SYSTEM_CMD_NEW_DATA to all it's connected links
 *
 * \param  pObj     [IN]  SELECT link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SelectLink_drvProcessData(SelectLink_Obj * pObj)
{

    SelectLink_CreateParams *pCreateArgs;
    SelectLink_ChInfo       *pChInfo;
    UInt32 bufId, queId;
    Int32 status;
    System_Buffer *pBuf;
    System_BufferList freeBufferList;
    System_BufferList inBufList;
    UInt32 newDataAvailable;
    System_VideoFrameBuffer *pFrameInfo;
    newDataAvailable = 0;
    UInt32 flags;

    freeBufferList.numBuf = 0;

    pCreateArgs = &pObj->createArgs;

    System_getLinksFullBuffers(pCreateArgs->inQueParams.prevLinkId,
                              pCreateArgs->inQueParams.prevLinkQueId,
                              &inBufList);

    if (inBufList.numBuf)
    {
        for (bufId = 0; bufId < inBufList.numBuf; bufId++)
        {
            /* remap channel number */
            pBuf = inBufList.buffers[bufId];

            UTILS_assert(pBuf->chNum < SYSTEM_MAX_CH_PER_OUT_QUE);

            pChInfo = &pObj->inChInfo[pBuf->chNum];

            if(pChInfo->queId >= pCreateArgs->numOutQue)
            {
                /* input ch not mapped to any output que,release the frame */
                freeBufferList.buffers[freeBufferList.numBuf] = pBuf;
                freeBufferList.numBuf++;
            }
            else
            {
                /* input channel mapped to a output que */
                pFrameInfo = (System_VideoFrameBuffer *) pBuf->payload;
                UTILS_assert(pFrameInfo != NULL);
                flags = pFrameInfo->chInfo.flags;

                /* if rt params are updated copy to local rt params as well */
                if(SYSTEM_LINK_CH_INFO_GET_FLAG_IS_RT_PRM_UPDATE(flags) == TRUE)
                {
                    pChInfo->rtChInfo = pFrameInfo->chInfo;
                }

                /* if channel mapping is changed dynamically then set
                    rtChInfoUpdate = TRUE for first time */
                if(pChInfo->rtChInfoUpdate)
                {
                    pChInfo->rtChInfoUpdate = FALSE;

                    SYSTEM_LINK_CH_INFO_SET_FLAG_IS_RT_PRM_UPDATE(flags, 1);

                    pFrameInfo->chInfo = pChInfo->rtChInfo;
                }

                /* save original channelNum so that we can restore it
                later while releasing the frame */
                pBuf->selectOrgChNum = pBuf->chNum;

                /* change ch number according to output que channel number */
                pBuf->chNum = pChInfo->outChNum;

                /* put the frame in output que */
                status = Utils_bufPutFullBuffer(
                                    &pObj->outFrameQue[pChInfo->queId], pBuf);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                /* mark as data put in output que so that we can send a
                            notification to next link */
                newDataAvailable |= (1<<pChInfo->queId);
            }
        }

        if(freeBufferList.numBuf)
        {
            /* free channels not mapped to any output queue */
            System_putLinksEmptyBuffers(pObj->createArgs.inQueParams.prevLinkId,
                                     pObj->createArgs.inQueParams.prevLinkQueId,
                                     &freeBufferList);

        }

        for(queId=0; queId<pCreateArgs->numOutQue; queId++)
        {
            /* send notification if needed */
            if (newDataAvailable & (1<<queId))
            {
                System_sendLinkCmd(pCreateArgs->outQueParams[queId].nextLink,
                               SYSTEM_CMD_NEW_DATA, NULL);
            }
        }

    }

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
Int32 SelectLink_getFullBuffers(Void * ptr, UInt16 queId,
                            System_BufferList * pBufList)
{
    Int32 status;
    SelectLink_Obj *pObj;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    pObj = (SelectLink_Obj *) pTsk->appData;
    UTILS_assert(queId < SELECT_LINK_MAX_OUT_QUE);

    status = Utils_bufGetFull(&pObj->outFrameQue[queId], pBufList,
                              BSP_OSAL_NO_WAIT);

    if(status == SYSTEM_LINK_STATUS_SOK)
    {
        pObj->linkStats.getFullBufCount++;
    }
    return status;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to Select link to get output queue
 *    Information of Select link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  info     [OUT] output queues information of SELECT link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SelectLink_getLinkInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    SelectLink_Obj *pObj = (SelectLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->info, sizeof(*info));

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
Int32 SelectLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                              System_BufferList * pBufList)
{

    UInt32 bufId;

    System_BufferList freeBufferList;
    System_Buffer *pBuf;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    SelectLink_Obj *pObj = (SelectLink_Obj *) pTsk->appData;

    pObj->linkStats.putEmptyBufCount++;

    freeBufferList.numBuf = 0;

    for (bufId = 0; bufId < pBufList->numBuf; bufId++)
    {
        pBuf = pBufList->buffers[bufId];

        if (pBuf == NULL)
        {
           continue;
        }

        pBuf->chNum = pBuf->selectOrgChNum;
        freeBufferList.buffers[freeBufferList.numBuf] = pBuf;
        freeBufferList.numBuf++;

    }
    pObj->putFrameCount += freeBufferList.numBuf;

    if(freeBufferList.numBuf)
    {
        System_putLinksEmptyBuffers(pObj->createArgs.inQueParams.prevLinkId,
                               pObj->createArgs.inQueParams.prevLinkQueId,
                               &freeBufferList);
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function to delete Select link. This will simply delete all created
 *        output queues
 *
 * \param  pObj     [IN]  DUP link instance handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 SelectLink_drvDelete(SelectLink_Obj * pObj)
{
    UInt32 outQueId;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    for(outQueId=0; outQueId<pObj->createArgs.numOutQue; outQueId++)
    {
        status = Utils_bufDelete(&pObj->outFrameQue[outQueId]);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating Select link
 *     - Arrival of new data
 *     - Get Output queue information
 *     - Set Output queue information
 *     - Deleting Select link
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Void SelectLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = 0;

    SelectLink_Obj *pObj = (SelectLink_Obj *) pTsk->appData;

    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:
            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
                status = SelectLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));
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
                status = SelectLink_drvProcessData(pObj);
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SYSTEM_CMD_DELETE:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                SelectLink_drvDelete(pObj);
                pObj->state = SYSTEM_LINK_STATE_IDLE;
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SELECT_LINK_CMD_SET_OUT_QUE_CH_INFO:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                SelectLink_drvSetOutQueChInfo( pObj,
                    (SelectLink_OutQueChInfo*)Utils_msgGetPrm(pMsg));
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        case SELECT_LINK_CMD_GET_OUT_QUE_CH_INFO:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                SelectLink_drvGetOutQueChInfo(pObj,
                (SelectLink_OutQueChInfo*)Utils_msgGetPrm(pMsg));
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
 * \brief Init function for Select link. This function does the following for
 *   every select link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SelectLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 selectId;
    SelectLink_Obj *pObj;
    UInt32 procId = System_getSelfProcId();

    for(selectId = 0; selectId < SELECT_LINK_OBJ_MAX; selectId++)
    {
        pObj = &gSelectLink_obj[selectId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->tskId = SYSTEM_MAKE_LINK_ID(procId,
                                          SYSTEM_LINK_ID_SELECT_0 + selectId);

        pObj->state = SYSTEM_LINK_STATE_IDLE;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = SelectLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = SelectLink_putEmptyBuffers;
        linkObj.getLinkInfo = SelectLink_getLinkInfo;

        System_registerLink(pObj->tskId, &linkObj);

        /*
         * Create link task, task remains in IDLE state.
         * SelectLink_tskMain is called when a message command is received.
         */
        status = SelectLink_tskCreate(selectId);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for Select link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
 Int32 SelectLink_deInit()
 {
    UInt32 selectId;

    for(selectId = 0; selectId < SELECT_LINK_OBJ_MAX; selectId++)
    {
        Utils_tskDelete(&gSelectLink_obj[selectId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
 }
