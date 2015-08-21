/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file DecLink_tsk.c
 *
 * \brief  This file has the implementation of Decode Link Init and Run API
 *
 *         This file implements the state machine logic for this link.
 *         DecodeLink_init() get calls from system_init and the same create
 *         the link task and basic messaging interfaces. Once the link is
 *         initiated it waits for the create cmd. This create cmd creates the
 *         complete link infrastructure.  Then waits for various data and
 *         control cmds.
 *
 *         This file also implements the Decode link tear down functionality
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "decLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gDecLink_tskStack, 32)
#pragma DATA_SECTION(gDecLink_tskStack, ".bss:taskStackSection")
UInt8 gDecLink_tskStack[DEC_LINK_OBJ_MAX][DEC_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Decode Link object, stores all link related information
 *******************************************************************************
 */
#pragma DATA_ALIGN(gDecLink_obj, 32)
#pragma DATA_SECTION(gDecLink_obj, ".bss:gDecLink_objSection")
DecLink_Obj gDecLink_obj[DEC_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief This function implements the START/RUN state of Decode Link.
 *
 *        In this state link gets commands to
 *         - Create the codec instance
 *         - Allocate output buffers
 *         - Moves to RUN state
 *        In this state link gets commands to
 *         - Stop/delete of link
 *         - Data events/cmds
 *         - All dynamic cmds that the link supports
 *         - All stats/status cmds
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void DecLink_tskMain(struct Utils_TskHndl *pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    DecLink_Obj *pObj;
    UInt32 flushCmds[2];
    UInt32 originalCmd;

    pObj = (DecLink_Obj *) pTsk->appData;

    /*
     * At this stage only create command is the expected command.
     * If other message gets received Ack with error status
     */
    if (cmd != SYSTEM_CMD_CREATE)
    {
        #ifndef DEC_LINK_SUPRESS_ERROR_AND_RESET
        Vps_printf( " DECODE: ERROR:"
                    " SYSTEM_CMD_CREATE should be first cmd."
                    " Received CMD is 0x%08x !!!\n", cmd);
        #endif
        Utils_tskAckOrFreeMsg(pMsg, SYSTEM_LINK_STATUS_EFAIL);
        return;
    }

    /*
     * Create command received, create the codec
     */
    status = DecLink_codecCreate(pObj, Utils_msgGetPrm(pMsg));

    Utils_tskAckOrFreeMsg(pMsg, status);

    if (status != SYSTEM_LINK_STATUS_SOK)
        return;

    Utils_encdecHdvicpPrfInit();
    done = FALSE;
    ackMsg = FALSE;
    pObj->pMsgTmp = NULL;
    pObj->lateAckStatus = SYSTEM_LINK_STATUS_SOK;

    /*
     * This while loop implements RUN state. All the run time commands for
     * ackMsg Link are received and serviced in this while loop.
     * Control remains in this loop until delete commands arrives.
     */
    while (!done)
    {
        status = Utils_tskRecvMsg(pTsk, &pMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        cmd = Utils_msgGetCmd(pMsg);

        /*
         * Different commands are serviced via this switch case. For each
         * command, after servicing, ACK or free message is sent before
         * proceeding to next state.
         */
        switch (cmd)
        {
            case SYSTEM_CMD_NEW_DATA:
                Utils_tskAckOrFreeMsg(pMsg, status);

                flushCmds[0] = SYSTEM_CMD_NEW_DATA;
                Utils_tskFlushMsg(pTsk, flushCmds, 1);

                DecLink_codecProcessData(pObj);
                break;

            case DEC_LINK_CMD_GET_PROCESSED_DATA:
                Utils_tskAckOrFreeMsg(pMsg, status);

                flushCmds[0] = DEC_LINK_CMD_GET_PROCESSED_DATA;
                Utils_tskFlushMsg(pTsk, flushCmds, 1);

                DecLink_codecGetProcessedDataMsgHandler(pObj);
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                DecLink_printStatistics(pObj, TRUE);
                Utils_encdecHdvicpPrfPrint();
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            case SYSTEM_CMD_PRINT_BUFFER_STATISTICS:
                Utils_tskAckOrFreeMsg(pMsg, status);
                DecLink_printBufferStatus(pObj);
                break;

            case DEC_LINK_CMD_CREATE_CHANNEL:
                {
                    DecLink_CreateChannelInfo *params;

                    params = (DecLink_CreateChannelInfo*) Utils_msgGetPrm(pMsg);
                    if (DEC_LINK_S_SUCCESS ==
                        DecLink_codecCreateChannelHandler(pObj, params))
                    {
                        Utils_tskAckOrFreeMsg(pMsg, status);
                    }
                    else
                    {
                        UTILS_assert(pObj->pMsgTmp == NULL);
                        pObj->pMsgTmp = pMsg;
                        pObj->lateAckStatus = SYSTEM_LINK_STATUS_EFAIL;
                    }
                }
                break;

            case DEC_LINK_CMD_DELETE_CHANNEL:
                {
                    DecLink_ChannelInfo *params;

                    params = (DecLink_ChannelInfo *) Utils_msgGetPrm(pMsg);
                    DecLink_codecDeleteChannelHandler(pObj, params);
                    UTILS_assert(pObj->pMsgTmp == NULL);
                    pObj->pMsgTmp = pMsg;
                    pObj->lateAckStatus = SYSTEM_LINK_STATUS_SOK;
                }
                break;

            case DEC_LINK_CMD_LATE_ACK:
                UTILS_assert(pObj->pMsgTmp != NULL);
                originalCmd = Utils_msgGetCmd(pObj->pMsgTmp);
                Utils_tskAckOrFreeMsg(pMsg, status);
                UTILS_assert((originalCmd == DEC_LINK_CMD_DELETE_CHANNEL) ||
                             ((originalCmd == DEC_LINK_CMD_CREATE_CHANNEL) &&
                              (pObj->lateAckStatus == SYSTEM_LINK_STATUS_EFAIL)));
                if (pObj->pMsgTmp != NULL)
                {
                    Utils_tskAckOrFreeMsg(pObj->pMsgTmp, pObj->lateAckStatus);
                }
                pObj->pMsgTmp = NULL;
                break;

            case DEC_LINK_CMD_DISABLE_CHANNEL:
                {
                    DecLink_ChannelInfo *params;

                    params = (DecLink_ChannelInfo *) Utils_msgGetPrm(pMsg);
                    DecLink_codecDisableChannel(pObj, params);
                    Utils_tskAckOrFreeMsg(pMsg, status);
                }
                break;
            case DEC_LINK_CMD_ENABLE_CHANNEL:
                {
                    DecLink_ChannelInfo *params;

                    params = (DecLink_ChannelInfo *) Utils_msgGetPrm(pMsg);
                    DecLink_codecEnableChannel(pObj, params);
                    Utils_tskAckOrFreeMsg(pMsg, status);
                }
                break;

            case DEC_LINK_CMD_SET_TRICKPLAYCONFIG:
                {
                    DecLink_TPlayConfig * params;
                    params = (DecLink_TPlayConfig *) Utils_msgGetPrm(pMsg);
                    DecLink_setTPlayConfig(pObj, params);
                    Utils_tskAckOrFreeMsg(pMsg, status);
                }
                break;

            case SYSTEM_CMD_STOP:
                DecLink_codecStop(pObj);
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            case SYSTEM_CMD_DELETE:
                DecLink_codecStop(pObj);
                done = TRUE;
                ackMsg = TRUE;
                break;
            case DEC_LINK_CMD_GET_BUFFER_STATISTICS:
            {
                DecLink_BufferStats * params;

                params = (DecLink_BufferStats *) Utils_msgGetPrm(pMsg);
                DecLink_getBufferStatus(pObj, params);
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
            }
            default:
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    DecLink_codecDelete(pObj);

    if (ackMsg && pMsg != NULL)
        Utils_tskAckOrFreeMsg(pMsg, status);

    return;
}

/**
 *******************************************************************************
 *
 *   \brief Decode link register and init function
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 DecLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    DecLink_Obj *pObj;
    char name[32];
    UInt32 objId;

    Utils_encdecInit();

    for (objId = 0; objId < DEC_LINK_OBJ_MAX; objId++)
    {
        pObj = &gDecLink_obj[objId];

        memset(pObj, 0, sizeof(*pObj));
        pObj->linkId = SYSTEM_LINK_ID_VDEC_0 + objId;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers  = DecLink_getFullFrames;
        linkObj.linkPutEmptyBuffers = DecLink_putEmptyFrames;
        linkObj.getLinkInfo = DecLink_getInfo;

        sprintf(name, "DEC%d   ", objId);

        System_registerLink(pObj->linkId, &linkObj);

        /*
         * Create link task, task remains in IDLE state.
         * DecodeLink_tskMain is called when a message command is received.
         */
        status = Utils_tskCreate(&pObj->tsk,
                                 DecLink_tskMain,
                                 DEC_LINK_TSK_PRI,
                                 gDecLink_tskStack[objId],
                                 DEC_LINK_TSK_STACK_SIZE, pObj, name);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Decode link de-register and de-init function
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 DecLink_deInit()
{
    UInt32 objId;
    DecLink_Obj *pObj;

    for (objId = 0; objId < DEC_LINK_OBJ_MAX; objId++)
    {
        pObj = &gDecLink_obj[objId];

        Utils_tskDelete(&pObj->tsk);
    }

    Utils_encdecDeInit();

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function return the channel info to the next link
 *
 * \param  pTsk     [IN]  Task Handle
 * \param  info     [OUT] channel info
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 DecLink_getInfo(Void * pTsk, System_LinkInfo * info)
{
    Utils_TskHndl * pTskHndl = (Utils_TskHndl *)pTsk;
    DecLink_Obj *pObj = (DecLink_Obj *) pTskHndl->appData;

    /* 'info' structure is set with valid values during 'create' phase */
    memcpy(info, &pObj->info, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to give full buffers to next
 *        link.
 *
 * Dec link sends message to next link about availability of buffers.
 * Next link calls this callback function to get full buffers from Dec link
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
Int32 DecLink_getFullFrames(Void * ptr, UInt16 queId,
                            System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    DecLink_Obj *pObj = (DecLink_Obj *) pTsk->appData;

    UTILS_assert(queId < DEC_LINK_MAX_OUT_QUE);
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->linkStatsInfo->linkStats.getFullBufCount++;

    return Utils_bufGetFullExt(&pObj->outObj.bufOutQue, pBufList,
                               BSP_OSAL_NO_WAIT);
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
Int32 DecLink_putEmptyFrames(Void * ptr, UInt16 queId,
                             System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    DecLink_Obj *pObj = (DecLink_Obj *) pTsk->appData;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    UTILS_assert(queId < DEC_LINK_MAX_OUT_QUE);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBuf <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    status =  DecLink_codecFreeProcessedFrames(pObj, pBufList);

    return status;
}

/* Nothing beyond this point */

