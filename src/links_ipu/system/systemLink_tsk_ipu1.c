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
 * \file systemLink_tsk_ipu1.c
 *
 * \brief  This file has the implementataion of systemlink task and
 *         handles the commands received.
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "systemLink_priv_ipu1.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSystemLink_tskStack, 32)
#pragma DATA_SECTION(gSystemLink_tskStack, ".bss:taskStackSection")
UInt8 gSystemLink_tskStack[SYSTEM_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link Object holds the task id and handle
 *******************************************************************************
 */
SystemLink_Obj gSystemLink_obj;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Handles the Commands received.
 *        Handles the commands for Profiling and core stats
 *
 * \param   pObj           [IN] SystemLink_Obj
 *
 * \param   cmd            [IN] input command
 *
 * \param   pPrm           [IN] input message
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SystemLink_cmdHandler(SystemLink_Obj * pObj, UInt32 cmd, Void * pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    switch (cmd)
    {
        case SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START:
            Utils_prfLoadCalcStart();
            break;

        case SYSTEM_COMMON_CMD_CPU_LOAD_CALC_STOP:
            Utils_prfLoadCalcStop();
            break;

        case SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET:
            Utils_prfLoadCalcReset();
            break;

        case SYSTEM_COMMON_CMD_PRINT_STATUS:
        {
            SystemCommon_PrintStatus *prm = (SystemCommon_PrintStatus *) pPrm;

            if (prm->printCpuLoad)
            {
                status = Utils_prfLoadPrintAll(prm->printTskLoad);
            }
            if (prm->printHeapStatus)
            {
                System_memPrintHeapStatus();
            }
        }
            break;

        case SYSTEM_COMMON_CMD_PRINT_STAT_COLL:
            Utils_statCollectorPrintCount();
            break;

        case SYSTEM_COMMON_CMD_RESET_STAT_COLL:
            Utils_statCollectorReset();
            break;

        case SYSTEM_COMMON_CMD_CORE_STATUS:
            Vps_printf(" Core is active\n");
            break;

        case SYSTEM_COMMON_CMD_GET_LOAD:
            Utils_prfGetLoad(pPrm);
            break;

        case SYSTEM_COMMON_CMD_RUN_DMA_TEST:
            Utils_dmaTestCopyFill(FALSE);
            break;

        case SYSTEM_COMMON_CMD_GET_IP_ADDR:
        {
            SystemCommon_IpAddr *pIpAddrPrm = (SystemCommon_IpAddr *)pPrm;

            Utils_ndkGetIpAddrStr(pIpAddrPrm->ipAddr);
        }
            break;
        case SYSTEM_COMMON_CMD_NET_PROC_ID:
        {
            SystemCommon_NetProcId *pNetProcIdPrm = (SystemCommon_NetProcId *)pPrm;

            pNetProcIdPrm->procId = Utils_netGetProcId();
        }
            break;

        #ifdef BUILD_M4_0
        case SYSTEM_COMMON_CMD_ALLOC_BUFFER:
            Utils_memAllocSR((SystemCommon_AllocBuffer *)pPrm);
            break;

        case SYSTEM_COMMON_CMD_FREE_BUFFER:
            Utils_memFreeSR((SystemCommon_FreeBuffer *)pPrm);
            break;
        #endif

        default:
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Creates Command handler task
 *        handles the commands for Profiling and core stats
 *
 * \param   pTsk    [IN]  Utils_TskHndl
 *
 * \param   pMsg    [IN]  Utils_MsgHndl
 *
 *******************************************************************************
*/
Void SystemLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    Int32 status;
    SystemLink_Obj *pObj = (SystemLink_Obj *) pTsk->appData;

    status = SystemLink_cmdHandler(pObj,
                                   Utils_msgGetCmd(pMsg),
                                   Utils_msgGetPrm(pMsg));

    SystemLink_userCmdHandler(Utils_msgGetCmd(pMsg),
                            Utils_msgGetPrm(pMsg));

    Utils_tskAckOrFreeMsg(pMsg, status);

    return;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the ipu systemLink
 *        Initializes the linkObj and creates the task for SystemLink_tskMain
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SystemLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    SystemLink_Obj *pObj;
    char tskName[32];
    unsigned int coreId;

    pObj = &gSystemLink_obj;

    memset(pObj, 0, sizeof(*pObj));
    coreId = System_getSelfProcId();
    pObj->tskId = SYSTEM_MAKE_LINK_ID(coreId, SYSTEM_LINK_ID_PROCK_LINK_ID);;

    linkObj.pTsk = &pObj->tsk;
    linkObj.linkGetFullBuffers = NULL;
    linkObj.linkPutEmptyBuffers = NULL;
    linkObj.getLinkInfo = NULL;

    System_registerLink(pObj->tskId, &linkObj);

    sprintf(tskName, "SYSTEM_IPU1_%d", pObj->tskId);

    status = Utils_tskCreate(&pObj->tsk,
                             SystemLink_tskMain,
                             SYSTEM_TSK_PRI,
                             gSystemLink_tskStack,
                             SYSTEM_TSK_STACK_SIZE, pObj, tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-Initialize the ipu systemLink
 *        Delete the SystemLink_tskMain created
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SystemLink_deInit()
{
    Utils_tskDelete(&gSystemLink_obj.tsk);

    return SYSTEM_LINK_STATUS_SOK;
}
