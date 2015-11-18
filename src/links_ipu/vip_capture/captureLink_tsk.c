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
 * \file captureLink_tsk.c
 *
 * \brief  This file has the implementation of Capture Link API
 **
 *           This file implements the state machine logic for this link.
 *           A message command will cause the state machine
 *           to take some action and then move to a different state.
 *
 *           The state machine table is as shown below
 *
 *   Cmds   | CREATE | DETECT_VIDEO | START | NEW_DATA  | STOP   | DELETE |
 *   States |========|==============|=======|===========|========|========|
 *   IDLE   | READY  | -            | -     | -         | -      | -      |
 *   READY  | -      | READY        | RUN   | -         | READY  | IDLE   |
 *   RUN    | -      | -            | -     | RUN       | READY  | IDLE   |
 *
 *
 * \version 0.0 (Jun 2013) : [HS] First version
 * \version 0.1 (Jul 2013) : [HS] Updates as per code review comments
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "captureLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gCaptureLink_tskStack, 32)
#pragma DATA_SECTION(gCaptureLink_tskStack, ".bss:taskStackSection")
UInt8 gCaptureLink_tskStack[CAPTURE_LINK_OBJ_MAX][CAPTURE_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
/* link object, stores all link related information */
CaptureLink_Obj gCaptureLink_obj[CAPTURE_LINK_OBJ_MAX];

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
Int32 CaptureLink_tskRun(CaptureLink_Obj * pObj, Utils_TskHndl * pTsk,
                         Utils_MsgHndl ** pMsg, Bool * done, Bool * ackMsg)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool runDone, runAckMsg;
    Utils_MsgHndl *pRunMsg;
    UInt32 cmd, instId, frmSkipMask;

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
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_NEW_DATA start !!!\n");
//#endif
                /* new data frames have been captured, process them */
                instId = (UInt32)Utils_msgGetPrm(pRunMsg);
                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                status = CaptureLink_drvProcessData(pObj, instId);
                if (status != SYSTEM_LINK_STATUS_SOK)
                {
                    /* in case of error exit RUN loop */
                    runDone = TRUE;

                    /* since message is already ACK'ed or free'ed do not ACK
                     * or free it again */
                    runAckMsg = FALSE;
                }
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_NEW_DATA end !!!\n");
//#endif
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_PRINT_STATISTICS start !!!\n");
//#endif
                /* new data frames have been captured, process them */

                CaptureLink_drvPrintStatus(pObj);

                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_PRINT_STATISTICS end !!!\n");
//#endif
                break;

            case SYSTEM_CMD_PRINT_BUFFER_STATISTICS:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_PRINT_BUFFER_STATISTICS start !!!\n");
//#endif
                CaptureLink_printBufferStatus(pObj);
                Utils_tskAckOrFreeMsg(pRunMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_PRINT_BUFFER_STATISTICS end !!!\n");
//#endif
                break;

            case SYSTEM_CMD_STOP:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_STOP start !!!\n");
//#endif
                /* stop RUN loop and goto READY state */
                runDone = TRUE;

                /* ACK message after actually stopping the driver outside the
                 * RUN loop */
                runAckMsg = TRUE;
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_STOP end !!!\n");
//#endif
                break;

            case SYSTEM_CMD_DELETE:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_DELETE start !!!\n");
//#endif
                /* stop RUN loop and goto IDLE state */

                /* exit RUN loop */
                runDone = TRUE;

                /* exit READY loop */
                *done = TRUE;

                /* ACK message after exiting READY loop */
                *ackMsg = TRUE;

                /* Pass the received message to the READY loop */
                *pMsg = pRunMsg;
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//   Vps_printf(" CAPTURE: tskRun SYSTEM_CMD_DELETE end !!!\n");
//#endif
                break;
            case CAPTURE_LINK_CMD_SET_FRAME_SKIP_MASK:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun CAPTURE_LINK_CMD_SET_FRAME_SKIP_MASK start !!!\n");
//#endif
                /* Command to change the frame skip mask parameter */
                frmSkipMask = *(UInt32 *)Utils_msgGetPrm(pRunMsg);
                /* This command would be called in limp home mode where
                 * the FPS is reduced when the temperature is very high.
                 */
                status = CaptureLink_drvUpdateFrmSkip(pObj, frmSkipMask);
                /* ACK or free message before proceding */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun CAPTURE_LINK_CMD_SET_FRAME_SKIP_MASK end !!!\n");
//#endif
                break;
            default:
//#ifdef SYSTEM_RT_STATS_LOG_CMD
//    Vps_printf(" CAPTURE: tskRun default !!!\n");
//#endif
                /* invalid command for this state ACK it and continue RUN
                 * loop */
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }

    }

    /* RUN loop exited, stop driver */
    CaptureLink_drvStop(pObj);

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
Void CaptureLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status;
    CaptureLink_Obj *pObj;

    /* IDLE state */

    pObj = (CaptureLink_Obj *) pTsk->appData;

    if (cmd != SYSTEM_CMD_CREATE)
    {
        /* invalid command recived in IDLE status, be in IDLE state and ACK
         * with error status */
        Utils_tskAckOrFreeMsg(pMsg, FVID2_EFAIL);
        return;
    }

    /* Create command received, create the driver */

    status = CaptureLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

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
                status = CaptureLink_drvStart(pObj);

                /* ACK based on create status */
                Utils_tskAckOrFreeMsg(pMsg, status);

                /* if start status is error then remain in READY state */
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    /* start success, entering RUN state */
                    status =
                        CaptureLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);

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

            case CAPTURE_LINK_GET_SUBFRAME_INFO:
                /* This cmd will be called by downstream link which processes
                * subframe dataduring its create time.
                */
                status = CaptureLink_subframe_drvGetVIPOutFrameInfo
                                            (pObj, Utils_msgGetPrm(pMsg));
                /* ACK based on create status */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;

            default:
                /* invalid command for this state ACK it and continue READY
                 * loop */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    /* exiting READY state, delete driver */
    CaptureLink_drvDelete(pObj);

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
Int32 CaptureLink_init()
{
    Int32 status, i;
    System_LinkObj linkObj;
    CaptureLink_Obj *pObj;

    /* register link with system API */

    for(i=0; i<CAPTURE_LINK_OBJ_MAX; i++)
    {
        pObj = &gCaptureLink_obj[i];

        memset(pObj, 0, sizeof(*pObj));

        pObj->linkId = SYSTEM_LINK_ID_CAPTURE_0+i;

        linkObj.pTsk                = &pObj->tsk;
        linkObj.linkGetFullBuffers  = CaptureLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = CaptureLink_putEmptyBuffers;
        linkObj.getLinkInfo         = CaptureLink_getInfo;

        System_registerLink(pObj->linkId, &linkObj);

        /* Create link task, task remains in IDLE state
         * CaptureLink_tskMain is called when a message command is received
         */

        status = Utils_tskCreate(&pObj->tsk,
                CaptureLink_tskMain,
                CAPTURE_LINK_TSK_PRI,
                gCaptureLink_tskStack[i],
                CAPTURE_LINK_TSK_STACK_SIZE, pObj, "CAPTURE ");
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
Int32 CaptureLink_deInit()
{
    Int32 i;
    CaptureLink_Obj *pObj;

    for(i=0; i<CAPTURE_LINK_OBJ_MAX; i++)
    {
        pObj = &gCaptureLink_obj[i];

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
Int32 CaptureLink_getInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    CaptureLink_Obj *pObj = (CaptureLink_Obj *) pTsk->appData;

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
Int32 CaptureLink_getFullBuffers(Void * ptr, UInt16 queId,
                                 System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    CaptureLink_Obj *pObj = (CaptureLink_Obj *) pTsk->appData;
    Int32 status;

    UTILS_assert(queId < CAPTURE_LINK_MAX_OUT_QUE);
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->linkStatsInfo->linkStats.getFullBufCount++;

    /* CaptureLink_Obj uses a single queue. queId is still passed as
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
Int32 CaptureLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                                  System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    CaptureLink_Obj *pObj = (CaptureLink_Obj *) pTsk->appData;

    UTILS_assert(queId < CAPTURE_LINK_MAX_OUT_QUE);

    return CaptureLink_drvPutEmptyBuffers(pObj, pBufList);
}
