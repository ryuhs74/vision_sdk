/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_tsk.h>
#if defined (BUILD_ARP32)
#include <src/utils_common/include/utils_idle.h>
#endif
/**
 *******************************************************************************
 *
 * \brief Multi-mbx Task main
 *
 *        This task is trigger whenever a message is sent to
 *        any of the possible mbx's registers with this task
 *
 * \param arg0         [IN]  Utils_TskMultiMbxHndl *
 * \param arg1         [IN]  NOT USED
 *
 *******************************************************************************
 */
Void Utils_tskMultiMbxMain(UArg arg0, UArg arg1)
{
    Utils_TskMultiMbxHndl *pMultiMbxTsk = (Utils_TskMultiMbxHndl *) arg0;
    Utils_MsgHndl *pMsg;
    Utils_MbxHndl *pMbx;
    Utils_TskHndl *pTskHndl;
    Int32 status;
    UInt32 i;

    UTILS_assert(pMultiMbxTsk != NULL);

#if defined (BUILD_ARP32)
        Utils_idleGetPreEveAutoCgTime();
#endif

    while (1)
    {
        BspOsal_semWait(pMultiMbxTsk->semTskPend, BSP_OSAL_WAIT_FOREVER);
#if defined (BUILD_ARP32)
        Utils_idleGetPostEveAutoCgTime();
#endif
        if(pMultiMbxTsk->doExitTask)
        {
            /* task exit flag set, break from loop and exit task */
            break;
        }

        /* task is un-pended, see in which que message is recevied
         * starting with highest priority queue
         */
        for(i=0; i<pMultiMbxTsk->numPriQue; i++)
        {
            /* loop until all message's from a priority queue
             * are handled
             */
            while(1)
            {
                pMbx = NULL;
                status = Utils_queGet(
                            &pMultiMbxTsk->recvQue[i],
                            (Ptr*)&pMbx,
                            1,
                            BSP_OSAL_NO_WAIT
                            );

                if(pMbx==NULL)
                {
                    /* nothing more in this queue
                     * goto next queue or pend on semaphore again
                     */
                    break;
                }

                /* recevied message in one of the mbx's, handle it */
                status = Utils_mbxRecvMsg(pMbx, &pMsg, BSP_OSAL_NO_WAIT);
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    pTskHndl = (Utils_TskHndl *)pMbx->pTsk;

                    if (pTskHndl->funcMain)
                        pTskHndl->funcMain(pTskHndl, pMsg);
                }
            }
        }
#if defined (BUILD_ARP32)
        Utils_idleGetPreEveAutoCgTime();
#endif

    }
}

/**
 *******************************************************************************
 *
 * \brief Function to trigger Multi-mbx to process messages
 *
 *        This function is registerd during Utils_tskMultiMbxCreate()
 *        This insert the message box to whom message is sent
 *
 *        into appropiate multi-mbx receive que
 *        A semaphore is set to wakeup the multi-mbx task
 *
 * \param pTsk         [IN]  Utils_TskHndl *
 * \param timeout      [IN]  pend timeout for que put
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskMultiMbxTrigger(Void * pTsk, Int32 timeout)
{
    Int32 status=0;

    Utils_TskHndl *pTskHndl
        = (Utils_TskHndl *)pTsk;

    Utils_TskMultiMbxHndl * pMultiMbxTsk
        = (Utils_TskMultiMbxHndl *)pTskHndl->pMultiMbxTsk;

    status = Utils_quePut(
                &pMultiMbxTsk->recvQue[pTskHndl->tskPri],
                (Ptr)&pTskHndl->mbx,
                timeout
            );

    BspOsal_semPost(pMultiMbxTsk->semTskPend);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function init's values in multi-mbx task handle
 *
 *        This function MUST be called before calling Utils_tskMultiMbxCreate
 *        The multi-mbx handle init in this function MUST be provided as
 *        as input during Utils_tskMultiMbxCreate().
 *
 *        NOTE, task itself is not created here
 *
 * \param pMultiMbxTsk [OUT] Multi-mbx task handle
 * \param stackAddr    [IN]  Task stack address
 * \param stackSize    [IN]  Task stack size
 * \param tskPri       [IN]  Task priority as defined by BIOS
 * \param numPriQue    [IN]  Number of priorites possible
 *                           when servicing messages
 *                           MUST be <= UTILS_TASK_MULTI_MBX_RECV_QUE_MAX
 * \param tskName      [IN]  Task name
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskMultiMbxSetupTskHndl(
                        Utils_TskMultiMbxHndl * pMultiMbxTsk,
                        UInt8 *stackAddr,
                        UInt32 stackSize,
                        UInt32 tskPri,
                        UInt32 numPriQue,
                        char *tskName
        )
{
    memset(pMultiMbxTsk, 0, sizeof(*pMultiMbxTsk));
    pMultiMbxTsk->doExitTask = 0;
    pMultiMbxTsk->numRefs = 0;
    pMultiMbxTsk->stackAddr = stackAddr;
    pMultiMbxTsk->stackSize = stackSize;
    pMultiMbxTsk->tskPri = tskPri;
    pMultiMbxTsk->numPriQue = numPriQue;
    UTILS_assert(numPriQue<=UTILS_TASK_MULTI_MBX_RECV_QUE_MAX);
    strncpy(pMultiMbxTsk->tskName, tskName, (sizeof(pMultiMbxTsk->tskName) - 1U));
    /* The maximum size of name permitted is 31 characters now.
        In cases where the name was exactly 32 characters, there would
        be no space for the string null terminator.
        Also, if the name was > 32 characters and dosent have a null
        terminator. The destination pMultiMbxTsk->tskName will be a non-null
        terminated string. */
    pMultiMbxTsk->tskName[sizeof(pMultiMbxTsk->tskName) - 1U] = '\0';

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function Create a task which can receive messages from multiple
 *        mail box's
 *
 *        Multi-mbx task gets created when reference count is 0.
 *
 * \param pHndl        [OUT] Task handle
 * \param pMultiMbxTsk [IN]  Multi-mbx task handle, this MUST be setup via
 *                           Utils_tskMultiMbxSetupTskHndl() before calling
 *                           this function
 * \param funcMain     [IN]  Task main,
 *                           Note, this is different from BIOS Task, since
 *                           this function
 *                           is entered ONLY when a message is received.
 * \param tskMultiMbxPri [IN]  Receive que task priority
 *                           0 is higest priority,
 *                           pMultiMbxTsk->numPriQue - 1 is lowest priority.
 *                           Received mailbox are servied in this order
 *                           by the multi-mbx task
 * \param appData      [IN]  Application specific data
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskMultiMbxCreate(Utils_TskHndl * pHndl,
                              Utils_TskMultiMbxHndl * pMultiMbxTsk,
                              Utils_TskFuncMain funcMain,
                              UInt32 tskMultiMbxPri,
                              Ptr appData)
{
    Int32 status;
    Utils_MbxCreatePrm mbxCreatePrm;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pMultiMbxTsk != NULL);
    UTILS_assert(funcMain != NULL);
    UTILS_assert(pMultiMbxTsk->numPriQue > 0);
    UTILS_assert(pMultiMbxTsk->numPriQue <= UTILS_TASK_MULTI_MBX_RECV_QUE_MAX);
    UTILS_assert(pMultiMbxTsk->stackAddr != NULL);
    UTILS_assert(pMultiMbxTsk->stackSize != 0);

    memset(pHndl, 0, sizeof(*pHndl));

    Utils_MbxCreatePrm_Init(&mbxCreatePrm);

    if(tskMultiMbxPri>=pMultiMbxTsk->numPriQue)
    {
        /* if priority is beyond valid range
         * then make it lowest priority
         */
        tskMultiMbxPri = pMultiMbxTsk->numPriQue-1;
    }

    pHndl->funcMain  = funcMain;
    pHndl->stackSize = 0;
    pHndl->stackAddr = NULL;
    pHndl->tsk = NULL;
    pHndl->appData   = appData;
    pHndl->pMultiMbxTsk = (struct Utils_TskMultiMbxHndl*)pMultiMbxTsk;
    pHndl->tskPri    = tskMultiMbxPri;

    mbxCreatePrm.pTsk = pHndl;
    mbxCreatePrm.pFuncMultiMbxTrigger = Utils_tskMultiMbxTrigger;

    status = Utils_mbxCreate(&pHndl->mbx, &mbxCreatePrm);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    if(pMultiMbxTsk->numRefs==0)
    {
        /* Task is not created, create it */

        Int32 i;

        /* create task pend semaphore */
        pMultiMbxTsk->semTskPend = BspOsal_semCreate(0, TRUE);

        UTILS_assert(pMultiMbxTsk->semTskPend != NULL);

        /* create task mbx queue's, one for each priority */
        for(i=0; i<pMultiMbxTsk->numPriQue; i++)
        {
            status = Utils_queCreate(
                        &pMultiMbxTsk->recvQue[i],
                        UTILS_TASK_MULTI_MBX_RECV_QUE_LEN_MAX,
                        pMultiMbxTsk->memRecvQue[i],
                        UTILS_QUE_FLAG_BLOCK_QUE
                        );

            UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
        }

        /* create the task itself */
        pMultiMbxTsk->tsk = BspOsal_taskCreate(
                                (BspOsal_TaskFuncPtr)Utils_tskMultiMbxMain,
                                pMultiMbxTsk->tskPri,
                                pMultiMbxTsk->stackAddr,
                                pMultiMbxTsk->stackSize,
                                pMultiMbxTsk
                              );
        UTILS_assert(pMultiMbxTsk->tsk != NULL);

        Utils_prfLoadRegister(pMultiMbxTsk->tsk, pMultiMbxTsk->tskName);
    }

    pMultiMbxTsk->numRefs++;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete the task and its associated mail box.
 *
 *        Multi-mbx task gets delete when reference count reaches 0.
 *
 * \param pHndl        [IN] Task handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success else failure
 *
 *******************************************************************************
 */
Int32 Utils_tskMultiMbxDelete(Utils_TskHndl * pHndl)
{
    Utils_TskMultiMbxHndl *pMultiMbxTsk
        = (Utils_TskMultiMbxHndl *)pHndl->pMultiMbxTsk;

    Int32 status;
    UInt32 i;

    pMultiMbxTsk->numRefs--;

    if(pMultiMbxTsk->numRefs==0)
    {
        pMultiMbxTsk->doExitTask = TRUE;

        /* post semaphore to unblock task */
        BspOsal_semPost(pMultiMbxTsk->semTskPend);

        /* wait for flag to be received and task to be exited */
        BspOsal_sleep(1);

        Utils_prfLoadUnRegister(pMultiMbxTsk->tsk);

        BspOsal_taskDelete(&pMultiMbxTsk->tsk);

        for(i=0; i<pMultiMbxTsk->numPriQue; i++)
        {
            status = Utils_queDelete(&pMultiMbxTsk->recvQue[i]);
            UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
        }

        BspOsal_semDelete(&pMultiMbxTsk->semTskPend);
    }

    Utils_mbxDelete(&pHndl->mbx);

    return SYSTEM_LINK_STATUS_SOK;
}


/* Nothing beyond this point */
