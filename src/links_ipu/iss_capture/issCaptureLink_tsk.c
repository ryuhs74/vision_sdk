/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

 /**
 *******************************************************************************
 * \file issCaptureLink_tsk.c
 *
 * \brief  This file has the implementation of Iss Capture Link API
 *
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 *           The state machine table is as shown below
 *
 *   Cmds   | CREATE | START | NEW_DATA | STOP   | DELETE |
 *   States |========|=======|==========|========|========|========|
 *   IDLE   | READY  | -     | -        | -      | -      | -      |
 *   READY  | -      | READY | RUN      | -      | READY  | IDLE   |
 *   RUN    | -      | -     | -        | RUN    | READY  | IDLE   |
 *
 *
 * \version 0.0 (Apr 2014) : [PS] First version
 * \version 0.1 (Apr 2015) : [SUJ] Updated for multi-channel support
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "issCaptureLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gIssCaptureLink_tskStack, 32)
#pragma DATA_SECTION(gIssCaptureLink_tskStack, ".bss:taskStackSection")
UInt8 gIssCaptureLink_tskStack[ISSCAPTURE_LINK_OBJ_MAX]
                              [ISSCAPTURE_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
/* link object, stores all link related information */
IssCaptureLink_Obj gIssCaptureLink_obj[ISSCAPTURE_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief This function is the implementation of Run state of link.
 *
 * In this state link waits for command from application or next link or from
 * driver. Basically all are control commands except the new_data command where
 * link gets the captured frames from driver and puts in output queue. After
 * that it sends command to next link.
 *
 * NOTE: Currently, with ISS drivers not being FVID_Q APIs,
 *       providing/taking empty/full buffers to/from driver happens in ISR
 *       context. So for now, NEW_DATA command flow is present just to
 *       inform next link about NEW_DATA.
 *
 * \param  pObj     [IN] Capture link object
 * \param  pTsk     [IN] Capture link Task handle
 * \param  pMsg     [IN] Message for the link. Contains command and args.
 * \param  done     [IN] Flag to exit idle state.
 * \param  ackMsg   [IN] Flag to decide whether to send ack message or not to
 *                       caller
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_tskRun(IssCaptureLink_Obj * pObj, Utils_TskHndl * pTsk,
                         Utils_MsgHndl ** pMsg, Bool * done, Bool * ackMsg)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool runDone, runAckMsg;
    Utils_MsgHndl *pRunMsg;
    UInt32 cmd;

    /* READY loop done and ackMsg status */
    *done = FALSE;
    *ackMsg = FALSE;
    *pMsg = NULL;

    /* RUN loop done and ackMsg status */
    runDone = FALSE;
    runAckMsg = FALSE;

    /* RUN state loop */
    while (!runDone)
    {
        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pRunMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pRunMsg);

        switch (cmd)
        {
            case SYSTEM_CMD_NEW_DATA:

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                status = IssCaptureLink_drvProcessData(pObj);
                if (status != SYSTEM_LINK_STATUS_SOK)
                {
                    /* in case of error exit RUN loop */
                    runDone = TRUE;

                    /* since message is already ACK'ed or free'ed do not ACK
                     * or free it again */
                    runAckMsg = FALSE;
                }
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                /* new data frames have been captured, process them */

                IssCaptureLink_drvPrintStatus(pObj);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_PRINT_BUFFER_STATISTICS:
                IssCaptureLink_drvPrintBufferStatus(pObj);
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_STOP:
                /* stop RUN loop and goto READY state */
                runDone = TRUE;

                /* ACK message after actually stopping the driver outside the
                 * RUN loop */
                runAckMsg = TRUE;
                break;

            case SYSTEM_CMD_DELETE:

                /* stop RUN loop and goto IDLE state */

                /* exit RUN loop */
                runDone = TRUE;

                /* exit READY loop */
                *done = TRUE;

                /* ACK message after exiting READY loop */
                *ackMsg = TRUE;

                /* Pass the received message to the READY loop */
                *pMsg = pRunMsg;
                break;

            case ISSCAPTURE_LINK_CMD_SAVE_FRAME:

                IssCaptureLink_drvSaveFrame(pObj);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case ISSCAPTURE_LINK_CMD_GET_SAVE_FRAME_STATUS:

                status = IssCaptureLink_drvGetSaveFrameStatus(pObj,
                                                    Utils_msgGetPrm(pRunMsg));

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            default:

                /* invalid command for this state ACK it and continue RUN
                 * loop */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }

    }

    /* RUN loop exited, stop driver */
    IssCaptureLink_drvStop(pObj);

    /* ACK message if not ACKed earlier */
    if (runAckMsg)
        Utils_tskAckOrFreeMsg(pRunMsg, status);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function is the implementation of Idle state.
 *
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void IssCaptureLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    IssCaptureLink_Obj *pObj;

    /* IDLE state */

    pObj = (IssCaptureLink_Obj *) pTsk->appData;

    if (cmd != SYSTEM_CMD_CREATE)
    {
        /* invalid command recived in IDLE status, be in IDLE state and ACK
         * with error status */
        Utils_tskAckOrFreeMsg(pMsg, FVID2_EFAIL);
        return;
    }

    /* Create command received, create the driver */

    status = IssCaptureLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

    /* ACK based on create status */
    Utils_tskAckOrFreeMsg(pMsg, status);

    /* if create status is error then remain in IDLE state */
    if (status != SYSTEM_LINK_STATUS_SOK)
        return;

    /* create success, entering READY state */

    done = FALSE;
    ackMsg = FALSE;

    /* READY state loop */
    while (!done)
    {
        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pMsg);

        switch (cmd)
        {
            case SYSTEM_CMD_START:
                /* Start capture driver */
                status = IssCaptureLink_drvStart(pObj);

                /* ACK based on create status */
                Utils_tskAckOrFreeMsg(pMsg, status);

                /* if start status is error then remain in READY state */
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    /* start success, entering RUN state */
                    status =
                        IssCaptureLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);

                    /** done = FALSE, exit RUN state
                      done = TRUE, exit RUN and READY state
                     */
                }

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
    IssCaptureLink_drvDelete(pObj);

    /* ACK message if not previously ACK'ed */
    if (ackMsg && pMsg != NULL)
        Utils_tskAckOrFreeMsg(pMsg, status);

    /* entering IDLE state */
    return;
}

/**
 *******************************************************************************
 *
 * \brief Init function for capture link. BIOS task for capture
 *  link gets created / registered in this function.
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_init(void)
{
    Int32 status;
    System_LinkObj linkObj;
    IssCaptureLink_Obj *pObj;
    UInt32 instId;

    IssCaptureLink_enumAssertCheck();

    /* register link with system API */
    for(instId = 0; instId < ISSCAPTURE_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIssCaptureLink_obj[instId];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId = SYSTEM_LINK_ID_ISSCAPTURE_0 + instId;

        linkObj.pTsk                = &pObj->tsk;
        linkObj.linkGetFullBuffers  = IssCaptureLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = IssCaptureLink_putEmptyBuffers;
        linkObj.getLinkInfo         = IssCaptureLink_getInfo;

        System_registerLink(pObj->linkId, &linkObj);

        /* Create link task, task remains in IDLE state
         * IssCaptureLink_tskMain is called when a message command is received
         */

        status = Utils_tskCreate(&pObj->tsk,
            IssCaptureLink_tskMain,
            ISSCAPTURE_LINK_TSK_PRI,
            &gIssCaptureLink_tskStack[instId][0],
            ISSCAPTURE_LINK_TSK_STACK_SIZE, pObj, "ISSCAPTURE ");

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
Int32 IssCaptureLink_deInit(void)
{
    IssCaptureLink_Obj *pObj;
    UInt32 instId;

    for(instId = 0; instId < ISSCAPTURE_LINK_OBJ_MAX; instId++ )
    {
        pObj = &gIssCaptureLink_obj[instId];

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
Int32 IssCaptureLink_getInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    IssCaptureLink_Obj *pObj = (IssCaptureLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->info, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to give full buffers to next
 *        link.
 *
 * Capture link sends message to next link about availability of buffers.
 * Next link calls this callback function to get full buffers from capture
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
Int32 IssCaptureLink_getFullBuffers(Void * ptr, UInt16 queId,
                                 System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    IssCaptureLink_Obj *pObj = (IssCaptureLink_Obj *) pTsk->appData;
    Int32 status;

    UTILS_assert(queId < ISSCAPTURE_LINK_MAX_OUT_QUE);
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->linkStatsInfo->linkStats.getFullBufCount++;

    /* IssCaptureLink_Obj uses a single queue. queId is still passed as
     * this function is common to all the links. Here we just ignore
     * the queId */
    status =  Utils_bufGetFull(&pObj->bufQue, pBufList, BSP_OSAL_NO_WAIT);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to get empty buffers from next
 *        link.
 *
 *
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId from which buffers are required.
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 IssCaptureLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                                  System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    IssCaptureLink_Obj *pObj = (IssCaptureLink_Obj *) pTsk->appData;

    UTILS_assert(queId < ISSCAPTURE_LINK_MAX_OUT_QUE);

    /* IssCaptureLink_Obj uses a single queue. queId is still passed as
     * this function is common to all the links. Here we just ignore
     * the queId */

    return IssCaptureLink_drvPutEmptyBuffers(pObj, pBufList);
}

Void IssCaptureLink_enumAssertCheck()
{
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV420_8B
        == VPS_ISS_CAL_CSI2_YUV420_8B);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV420_10B
        == VPS_ISS_CAL_CSI2_YUV420_10B);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV420_8B_LEGACY
        == VPS_ISS_CAL_CSI2_YUV420_8B_LEGACY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV420_8B_CHROMA_SHIFT
        == VPS_ISS_CAL_CSI2_YUV420_8B_CHROMA_SHIFT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV420_10B_CHROMA_SHIFT
        == VPS_ISS_CAL_CSI2_YUV420_10B_CHROMA_SHIFT);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV422_8B
        == VPS_ISS_CAL_CSI2_YUV422_8B);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_YUV422_10B
        == VPS_ISS_CAL_CSI2_YUV422_10B);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RGB444
        == VPS_ISS_CAL_CSI2_RGB444);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RGB555
        == VPS_ISS_CAL_CSI2_RGB555);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RGB565
        == VPS_ISS_CAL_CSI2_RGB565);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RGB666
        == VPS_ISS_CAL_CSI2_RGB666);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RGB888
        == VPS_ISS_CAL_CSI2_RGB888);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RAW6
        == VPS_ISS_CAL_CSI2_RAW6);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RAW7
        == VPS_ISS_CAL_CSI2_RAW7);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RAW8
        == VPS_ISS_CAL_CSI2_RAW8);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RAW10
        == VPS_ISS_CAL_CSI2_RAW10);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RAW12
        == VPS_ISS_CAL_CSI2_RAW12);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_RAW14
        == VPS_ISS_CAL_CSI2_RAW14);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_ANY
        == VPS_ISS_CAL_CSI2_ANY);
    UTILS_COMPILETIME_ASSERT(SYSTEM_CSI2_DISABLE_CONTEXT
        == VPS_ISS_CAL_CSI2_DISABLE_CONTEXT);
}
