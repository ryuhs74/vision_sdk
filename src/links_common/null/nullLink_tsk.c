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
 * \version 0.0 (Jul 2013) : [NN] First version
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
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gNullLink_tskStack, 32)
#pragma DATA_SECTION(gNullLink_tskStack, ".bss:taskStackSection")
UInt8 gNullLink_tskStack[NULL_LINK_OBJ_MAX][NULL_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
NullLink_Obj gNullLink_obj[NULL_LINK_OBJ_MAX];



/**
 *******************************************************************************
 * \brief Create resources and setup info required to dump frames to memory
 *
 * \param  pObj     [IN]  Null link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 NullLink_drvCreateDumpFramesObj(NullLink_Obj * pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Utils_DmaChCreateParams dmaParams;
    NullLink_DumpFramesObj  *pDumpFramesObj;
    UInt16 inQue, chId;
    System_LinkQueInfo *pInQueInfo;
    NullLink_CreateParams * pPrm;
    UInt32 inQueMemSize, chMemSize;

    pPrm = &pObj->createArgs;

    if(pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_2D_MEMORY)
    {
        Utils_DmaChCreateParams_Init(&dmaParams);

        status = Utils_dmaCreateCh(
                        &pObj->dumpFramesDmaObj,
                        &dmaParams
                        );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        inQueMemSize =
            pPrm->dumpFramesMemorySize/pPrm->numInQue;

        for (inQue = 0; inQue < pPrm->numInQue; inQue++)
        {
            pInQueInfo = &pObj->inQueInfo[inQue];

            chMemSize = inQueMemSize/pInQueInfo->numCh;

            UTILS_assert(pInQueInfo->numCh <= SYSTEM_MAX_CH_PER_OUT_QUE);
            for (chId = 0; chId < pInQueInfo->numCh; chId++)
            {
                pDumpFramesObj = &pObj->dumpFramesObj[inQue][chId];

                pDumpFramesObj->inQueId = inQue;
                pDumpFramesObj->chId    = chId;
                pDumpFramesObj->numFrames = 0;

                pDumpFramesObj->memAddr =
                    pPrm->dumpFramesMemoryAddr
                        + inQueMemSize*inQue
                        + chMemSize*chId;

                pDumpFramesObj->memSize = chMemSize;

                pDumpFramesObj->curMemOffset = 0;
            }
        }
    }
    else if(pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_BITSTREAM_MEMORY)
    {

    }
    else if(pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_NETWORK)
    {
        NullLink_networkTxCreate(pObj);
    }

    return status;
}

/**
 *******************************************************************************
 * \brief Delete resources required to dump frames to memory
 *
 * \param  pObj     [IN]  Null link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 NullLink_drvDeleteDumpFramesObj(NullLink_Obj * pObj)
{
    if(pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_2D_MEMORY)
    {
        Utils_dmaDeleteCh(&pObj->dumpFramesDmaObj);
    }
    else if(pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_NETWORK)
    {
        NullLink_networkTxDelete(pObj);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Delete resources required to dump frames to memory
 *
 * \param  pObj     [IN]  Null link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 NullLink_drvDumpFrames(NullLink_Obj * pObj,
                            UInt32 inQue,
                            System_Buffer *pBuf)
{
    System_LinkChInfo *pChInfo;
    Int32 status;
    UInt32 dataFormat, memCopySize = 0;

    if(pObj->createArgs.dumpDataType
            &&
       pBuf!=NULL
            &&
       inQue < NULL_LINK_MAX_IN_QUE
            &&
       pBuf->chNum < SYSTEM_MAX_CH_PER_OUT_QUE
            &&
       inQue < pObj->createArgs.numInQue
            &&
       pBuf->chNum < pObj->inQueInfo[inQue].numCh
    )
    {
        NullLink_DumpFramesObj  *pDumpFramesObj;

        pDumpFramesObj = &pObj->dumpFramesObj[inQue][pBuf->chNum];

        pChInfo = &pObj->inQueInfo[inQue].chInfo[pBuf->chNum];

        if((pBuf->bufType == SYSTEM_BUFFER_TYPE_VIDEO_FRAME) &&
           (pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_2D_MEMORY))
        {
            UInt32 offsetUV = 0;
            System_VideoFrameBuffer *pVidFrame =
                (System_VideoFrameBuffer *)pBuf->payload;

            Utils_DmaCopyFill2D dmaPrm;

            dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pChInfo->flags);

            if(dataFormat==SYSTEM_DF_YUV422I_YUYV
                ||
                dataFormat==SYSTEM_DF_YUV422I_UYVY
                ||
                dataFormat==SYSTEM_DF_RGB16_565
                ||
                dataFormat==SYSTEM_DF_BGR16_565
                )
            {
                memCopySize = pChInfo->pitch[0]*pChInfo->height*2;

                dmaPrm.dataFormat = SYSTEM_DF_RAW16;
            }
            if(dataFormat==SYSTEM_DF_YUV420SP_UV)
            {
                offsetUV = pChInfo->pitch[0]*pChInfo->height;

                memCopySize = (offsetUV*3)/2;

                dmaPrm.dataFormat = SYSTEM_DF_YUV420SP_UV;
            }

            if((pDumpFramesObj->curMemOffset+memCopySize)
                    >= pDumpFramesObj->memSize
               )
            {
                Vps_printf(
                    " NULL_SINK: Q%d: CH%d: Dumped %d frames @ 0x%08x, size: %d bytes !!!\n",
                        pDumpFramesObj->inQueId,
                        pDumpFramesObj->chId,
                        pDumpFramesObj->numFrames,
                        pDumpFramesObj->memAddr,
                        pDumpFramesObj->curMemOffset
                );

                pDumpFramesObj->curMemOffset = 0;
                pDumpFramesObj->numFrames = 0;
            }

            dmaPrm.destAddr[0]= (Ptr)(pDumpFramesObj->memAddr
                                + pDumpFramesObj->curMemOffset);
            dmaPrm.destAddr[1]= (Ptr)((UInt32)dmaPrm.destAddr[0] +
                                    offsetUV);
            dmaPrm.destPitch[0] = pChInfo->pitch[0];
            dmaPrm.destPitch[1] = pChInfo->pitch[1];
            dmaPrm.destStartX   = 0;
            dmaPrm.destStartY   = 0;
            dmaPrm.width        = pChInfo->width;
            dmaPrm.height       = pChInfo->height;
            dmaPrm.srcAddr[0]   = pVidFrame->bufAddr[0];
            dmaPrm.srcAddr[1]   = pVidFrame->bufAddr[1];
            dmaPrm.srcPitch[0]  = pChInfo->pitch[0];
            dmaPrm.srcPitch[1]  = pChInfo->pitch[1];
            dmaPrm.srcStartX    = pChInfo->startX;
            dmaPrm.srcStartY    = pChInfo->startY;

            status = Utils_dmaCopy2D(&pObj->dumpFramesDmaObj, &dmaPrm, 1);
            UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

            pDumpFramesObj->curMemOffset += memCopySize;
            pDumpFramesObj->numFrames++;

        }else if((pBuf->bufType == SYSTEM_BUFFER_TYPE_BITSTREAM) &&
                 (pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_BITSTREAM_MEMORY))
        {
            System_BitstreamBuffer *bitstreamBuf;
            bitstreamBuf = ((System_BitstreamBuffer *)pBuf->payload);
            if ((pObj->dataDumpSize + bitstreamBuf->fillLength) <
                 pObj->createArgs.dumpFramesMemorySize)
            {
                memcpy(pObj->dataDumpPtr,
                       bitstreamBuf->bufAddr,
                       bitstreamBuf->fillLength);
                pObj->dataDumpPtr += bitstreamBuf->fillLength;
                pObj->dataDumpSize += bitstreamBuf->fillLength;
            }
        }
        else if(pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_NETWORK)
        {
            NullLink_networkTxSendData(
                    pObj,
                    inQue,
                    pBuf->chNum,
                    pBuf
                );
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}


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

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));
    UTILS_assert(pObj->createArgs.numInQue < NULL_LINK_MAX_IN_QUE);

    pObj->recvCount= 0;

    for (inQue = 0; inQue < pPrm->numInQue; inQue++)
    {
        status =
            System_linkGetInfo(pPrm->inQueParams[inQue].prevLinkId,
                               &inTskInfo);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        UTILS_assert(pPrm->inQueParams[inQue].prevLinkQueId <
                     inTskInfo.numQue);

        pObj->inQueInfo[inQue]
            =
                inTskInfo.queInfo
            [pPrm->inQueParams[inQue].prevLinkQueId];
    }

    NullLink_drvCreateDumpFramesObj(pObj);

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
    UInt32 queId, bufId;

    for (queId = 0; queId < pObj->createArgs.numInQue; queId++)
    {
        pInQueParams = &pObj->createArgs.inQueParams[queId];

        System_getLinksFullBuffers(pInQueParams->prevLinkId,
                                  pInQueParams->prevLinkQueId, &bufList);

        if (bufList.numBuf)
        {
            pObj->recvCount += bufList.numBuf;

            for (bufId = 0; bufId < bufList.numBuf; bufId++)
            {
                NullLink_drvDumpFrames(pObj, queId, bufList.buffers[bufId]);
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

Void NullLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    NullLink_Obj *pObj = (NullLink_Obj *) pTsk->appData;

    if (cmd != SYSTEM_CMD_CREATE)
    {
        Utils_tskAckOrFreeMsg(pMsg, FVID2_EFAIL);
        return;
    }

    status = NullLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

    Utils_tskAckOrFreeMsg(pMsg, status);

    if (status != SYSTEM_LINK_STATUS_SOK)
        return;

    done = FALSE;
    ackMsg = FALSE;

    while (!done)
    {
        status = Utils_tskRecvMsg(pTsk, &pMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        cmd = Utils_msgGetCmd(pMsg);

        switch (cmd)
        {
            case SYSTEM_CMD_START:
                if (pObj->createArgs.dumpDataType == NULL_LINK_COPY_TYPE_BITSTREAM_MEMORY)
                {
                    pObj->dataDumpPtr = (char *)pObj->createArgs.dumpFramesMemoryAddr;
                    UTILS_assert (pObj->createArgs.dumpFramesMemorySize != 0);
                    pObj->dataDumpSize = 0;
                }
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
            case SYSTEM_CMD_DELETE:
                done = TRUE;
                ackMsg = TRUE;
                break;
            case SYSTEM_CMD_NEW_DATA:
                Utils_tskAckOrFreeMsg(pMsg, status);

                NullLink_drvProcessFrames(pObj);
                break;
            default:
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    NullLink_drvDeleteDumpFramesObj(pObj);

    if (ackMsg && pMsg != NULL)
        Utils_tskAckOrFreeMsg(pMsg, status);

    return;
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
    char tskName[32];
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

        sprintf(tskName, "NULL%u", (unsigned int)nullId);

        status = Utils_tskCreate(&pObj->tsk,
                                 NullLink_tskMain,
                                 NULL_LINK_TSK_PRI,
                                 gNullLink_tskStack[nullId],
                                 NULL_LINK_TSK_STACK_SIZE, pObj, tskName);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
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
        Utils_tskDelete(&gNullLink_obj[nullId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
}
