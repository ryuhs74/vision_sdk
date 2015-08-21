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
 * \file utils_tsk.c
 *
 * \brief  APIs from this file are used to interface the sysbios task functions
 *
 *         The APIs allow a user to create/delete the Bios tasks and wait
 *         for ACK before proceeding further.
 *
 * \version 0.0 (Jan 2013) : [SS] First version
 * \version 0.1 (Feb 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_tsk.h>

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief The command to exit a task
 *
 *******************************************************************************
 */
#define UTILS_TSK_CMD_EXIT   (0xFFFFFFFF)

/**
 *******************************************************************************
 *
 * \brief This function implements the message infrastructure of the Task
 *
 *        waits for
 *         - Till it respective the message
 *         - Extract the cmd from the message
 *         - Ack or free the message if the cmd is an exit command
 *         - Jump to Task function
 *
 * \param  arg0 [IN] Task Handle
 * \param  arg1 [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void Utils_tskMain(UArg arg0, UArg arg1)
{
    Utils_TskHndl *pHndl = (Utils_TskHndl *) arg0;
    Utils_MsgHndl *pMsg;
    Int32 status;
    UInt32 cmd;

    UTILS_assert(pHndl != NULL);

    while (1)
    {
        status = Utils_mbxRecvMsg(&pHndl->mbx, &pMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        cmd = Utils_msgGetCmd(pMsg);
        if (cmd == UTILS_TSK_CMD_EXIT)
        {
            Utils_tskAckOrFreeMsg(pMsg, SYSTEM_LINK_STATUS_SOK);
            break;
        }

        if (pHndl->funcMain)
            pHndl->funcMain(pHndl, pMsg);
    }
}

/**
 *******************************************************************************
 *
 * \brief This function Create a task and its mail box
 *
 * \param pHndl        [OUT] Task handle
 * \param funcMain     [IN]  Task main,
 *                           Note, this is different from BIOS Task, since
 *                           this function
 *                           is entered ONLY when a message is received.
 * \param tskPri       [IN]  Task priority as defined by BIOS
 * \param stackAddr    [IN]  Task stack address
 * \param stackSize    [IN]  Task stack size
 * \param appData      [IN]  Application specific data
 * \param tskName      [IN]  Task name
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskCreate(Utils_TskHndl * pHndl,
                      Utils_TskFuncMain funcMain,
                      UInt32 tskPri,
                      UInt8 * stackAddr,
                      UInt32 stackSize, Ptr appData, char *tskName)
{
    Int32 status;
    Utils_MbxCreatePrm mbxCreatePrm;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(funcMain != NULL);

    Utils_MbxCreatePrm_Init(&mbxCreatePrm);

    pHndl->funcMain = funcMain;
    pHndl->stackSize = stackSize;
    pHndl->stackAddr = stackAddr;
    pHndl->tskPri = tskPri;
    pHndl->appData = appData;
    pHndl->pMultiMbxTsk = NULL;

    status = Utils_mbxCreate(&pHndl->mbx, &mbxCreatePrm);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    pHndl->tsk = BspOsal_taskCreate(
                    (BspOsal_TaskFuncPtr)Utils_tskMain,
                    pHndl->tskPri,
                    pHndl->stackAddr,
                    pHndl->stackSize,
                    pHndl
                  );

    UTILS_assert(pHndl->tsk != NULL);

    Utils_prfLoadRegister(pHndl->tsk, tskName);
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete the task and its associated mail box.
 *
 * \param pHndl        [OUT] Task handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskDelete(Utils_TskHndl * pHndl)
{
    if(pHndl->pMultiMbxTsk==NULL)
    {
        /* NOT a multi-mbx task */

        Utils_tskSendCmd(pHndl, UTILS_TSK_CMD_EXIT, NULL);

        /* wait for command to be received and task to be exited */
        BspOsal_sleep(1);

        Utils_prfLoadUnRegister(pHndl->tsk);

        BspOsal_taskDelete(&pHndl->tsk);

        Utils_mbxDelete(&pHndl->mbx);
    }
    else
    {
        /* Multi-mbx task */
        Utils_tskMultiMbxDelete(pHndl);
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This flush a specified number of message
 *
 *        Each task has an associated mail box and this flushes "n" number of
 *        messages from the mail box
 *
 * \param  pHndl      [IN] Task Handle
 * \param  flushCmdId [IN] Flush command
 * \param  numCmds    [IN] Number of messages to be Flushed
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskFlushMsg(Utils_TskHndl * pHndl, UInt32 *flushCmdId, UInt32 numCmds)
{
    Utils_MsgHndl *pMsg;
    UInt32 i, cmd;
    Int32 status;
    Bool done;

    do
    {
        status = Utils_tskPeekMsg(pHndl, &pMsg);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        cmd = Utils_msgGetCmd(pMsg);

        done = TRUE;

        /* search the commands the need to be flushed */
        for(i=0; i<numCmds; i++)
        {
            if(cmd==flushCmdId[i])
            {
                /*
                 * same command in queue to pull it out of
                 * the queue and free it */
                status = Utils_tskRecvMsg(pHndl, &pMsg, BSP_OSAL_NO_WAIT);
                UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
                Utils_tskAckOrFreeMsg(pMsg, status);

                done = FALSE;
                break;
            }
        }
    } while(!done);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function change priority of a task
 *
 * \param pHndl     [IN] Task handle
 * \param newPri    [IN] new priority value of the task
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskSetPri(Utils_TskHndl * pHndl, UInt32 newPri)
{

    if((Task_Mode_TERMINATED != Task_getMode(pHndl->tsk)) &&
       (newPri != 0) && (newPri < (UInt32)Task_numPriorities))
    {
       Task_setPri(pHndl->tsk, newPri);
       pHndl->tskPri = newPri;
    }
    else
       return FVID2_EFAIL;

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
