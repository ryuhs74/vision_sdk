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
 * \file nullLink_tsk.c
 *
 * \brief  This file has the implementation of DUP Link API
 **
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 * \version 0.0 (Jun 2014) : [YM] First version ported to Linux
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "nullLink_priv.h"

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
NullLink_Obj gNullLink_obj[NULL_LINK_OBJ_MAX];

/**
 *******************************************************************************
 * \brief Null Link can be used to take input from a link and then without doing
 *   anything return it back to the same link. This useful when a link output
 *   cannot be given to any other link for testing purpose we just want to run
 *   a given link but not really use the output. In such cases the output queue
 *   of link can be connected to a Null link. The null link will operate like
 *   any other link from interface point of view. But it wont do anything with
 *   the frames it gets. It will simply return it back to the sending link. This
 *   function simply does the following
 *
 *     - Copies the user passed create params into the link object create params
 *     - resets received frame count to zero
 *
 * \param  pObj     [IN]  Null link instance handle
 * \param  pPrm     [IN]  Create params for Null link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 NullLink_drvCreate(NullLink_Obj * pObj, NullLink_CreateParams * pPrm)
{
    Int32 status;
    UInt16 inQue;
    System_LinkInfo inTskInfo;

    Vps_printf(" NULL Link: Create in progress !!!\n");

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));
    OSA_assert(pObj->createArgs.numInQue < NULL_LINK_MAX_IN_QUE);

    pObj->recvCount= 0;

    for (inQue = 0; inQue < pPrm->numInQue; inQue++)
    {
        status =
            System_linkGetInfo(pPrm->inQueParams[inQue].prevLinkId,
                               &inTskInfo);
        OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
        OSA_assert(pPrm->inQueParams[inQue].prevLinkQueId <
                     inTskInfo.numQue);

        pObj->inQueInfo[inQue]
            =
                inTskInfo.queInfo
            [pPrm->inQueParams[inQue].prevLinkQueId];
    }

    Vps_printf(" NULL Link: Create done !!!\n");

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Null Link just receives incoming buffers and returns back to the
 *   sending link. This function does the same
 *
 * \param  pObj     [IN]  Null link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 NullLink_drvProcessFrames(NullLink_Obj * pObj)
{
    System_LinkInQueParams *pInQueParams;
    System_BufferList bufList;
    UInt32 queId;

    for (queId = 0; queId < pObj->createArgs.numInQue; queId++)
    {
        pInQueParams = &pObj->createArgs.inQueParams[queId];

        System_getLinksFullBuffers(pInQueParams->prevLinkId,
                                  pInQueParams->prevLinkQueId, &bufList);

        if (bufList.numBuf)
        {
            System_BitstreamBuffer *bitstreamBuf;
            pObj->recvCount += bufList.numBuf;
            if (pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_BITSTREAM_FILE)
            {
                bitstreamBuf = ((System_BitstreamBuffer *)bufList.buffers[0]->payload);
                /* Cache invalidate required as CPU copy is used */
                OSA_memCacheInv((UInt32)bitstreamBuf->bufAddr, bitstreamBuf->fillLength);

                fwrite(bitstreamBuf->bufAddr, bitstreamBuf->fillLength, 1, pObj->fpDataStream);
            }

            System_putLinksEmptyBuffers(pInQueParams->prevLinkId,
                                       pInQueParams->prevLinkQueId, &bufList);
        }

    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating Null link
 *     - Arrival of new data
 *     - Deleting Null link
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Int32 NullLink_tskMain(struct OSA_TskHndl * pTsk, OSA_MsgHndl * pMsg, UInt32 curState)
{
    UInt32 cmd = OSA_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    NullLink_Obj *pObj = (NullLink_Obj *) pTsk->appData;

    if (cmd != SYSTEM_CMD_CREATE)
    {
        OSA_tskAckOrFreeMsg(pMsg, SYSTEM_LINK_STATUS_EFAIL);
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    status = NullLink_drvCreate(pObj, OSA_msgGetPrm(pMsg));

    OSA_tskAckOrFreeMsg(pMsg, status);

    if (status != SYSTEM_LINK_STATUS_SOK)
        return status;

    done = FALSE;
    ackMsg = FALSE;

    while (!done)
    {
        status = OSA_tskWaitMsg(pTsk, &pMsg);
        if (status != OSA_SOK)
            break;

        cmd = OSA_msgGetCmd(pMsg);

        switch (cmd)
        {
            case SYSTEM_CMD_START:
                OSA_tskAckOrFreeMsg(pMsg, status);
                if (pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_BITSTREAM_FILE)
                {
                   Vps_printf(" NULL LINK: Opening file for Dump \n");
                   pObj->fpDataStream = fopen(pObj->createArgs.nameDataFile, "wb");
                   OSA_assert(pObj->fpDataStream != NULL);
                   Vps_printf(" NULL LINK: Opened file for Dump \n");
                }
                break;
            case SYSTEM_CMD_STOP:
                OSA_tskAckOrFreeMsg(pMsg, status);
                if (pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_BITSTREAM_FILE)
                {
                   Vps_printf(" NULL LINK: Closing Dump file \n");
                   if(pObj->fpDataStream)
                   {
                       fflush(pObj->fpDataStream);
                       fclose(pObj->fpDataStream);
                   }
                   Vps_printf(" NULL LINK: Closed dump file \n");
                }
                break;
            case SYSTEM_CMD_DELETE:
                done = TRUE;
                ackMsg = TRUE;
                break;
            case SYSTEM_CMD_NEW_DATA:
                OSA_tskAckOrFreeMsg(pMsg, status);

                NullLink_drvProcessFrames(pObj);
                break;
            default:
                OSA_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    if (ackMsg && pMsg != NULL)
        OSA_tskAckOrFreeMsg(pMsg, status);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Init function for Null link. This function does the following for each
 *   Null link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */

Int32 NullLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 nullId;
    NullLink_Obj *pObj;
    UInt32 procId = System_getSelfProcId();

    for (nullId = 0; nullId < NULL_LINK_OBJ_MAX; nullId++)
    {
        pObj = &gNullLink_obj[nullId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->tskId = SYSTEM_MAKE_LINK_ID(procId,
                                          SYSTEM_LINK_ID_NULL_0 + nullId);
        memset(&linkObj, 0, sizeof(linkObj));
        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers= NULL;
        linkObj.linkPutEmptyBuffers= NULL;
        linkObj.getLinkInfo = NULL;

        System_registerLink(pObj->tskId, &linkObj);

        /*
         * Create link task, task remains in IDLE state.
         * ipcOutLink_tskMain is called when a message command is received.
         */
        status = OSA_tskCreate(&pObj->tsk,
                               NullLink_tskMain,
                               NULL_LINK_TSK_PRI,
                               NULL_LINK_TSK_STACK_SIZE,
                               0,
                               pObj);
        OSA_assert(status == OSA_SOK);


    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for Null link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 NullLink_deInit()
{
    UInt32 nullId;

    for (nullId = 0; nullId < NULL_LINK_OBJ_MAX; nullId++)
    {
        OSA_tskDelete(&gNullLink_obj[nullId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
}
