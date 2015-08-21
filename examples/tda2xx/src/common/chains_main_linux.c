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
 * \file chains_main_linux.c
 *
 * \brief  Entry point for IPU1-0 application when A15 OS is Linux
 *
 * \version 0.0 (Jun 2013) : [KC] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <examples/tda2xx/include/chains.h>
#include <src/utils_common/include/utils_tsk.h>

#include <linux/examples/common/appCtrlLink.h>


#define APP_CTRL_TSK_PRI                   (4)
#define APP_CTRL_TSK_STACK_SIZE            (16*1024)

/**
 *******************************************************************************
 * \brief Link Specific Object
 *******************************************************************************
 */
typedef struct {

    /**< App Ctrl Task Id */
    UInt32 tskId;

    /**< App Ctrl Task Handle */
    Utils_TskHndl tsk;

} AppCtrl_Obj;

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gAppCtrl_tskStack, 32)
#pragma DATA_SECTION(gAppCtrl_tskStack, ".bss:taskStackSection")
UInt8 gAppCtrl_tskStack[APP_CTRL_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link Object holds the task id and handle
 *******************************************************************************
 */
AppCtrl_Obj gAppCtrl_obj;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Set DMM priorities
 *
 *******************************************************************************
 */
Void AppCtrl_setSystemL3DmmPri()
{
    /* Assert Mflag of DSS to give DSS highest priority */
    Utils_setDssMflagMode(UTILS_DSS_MFLAG_MODE_FORCE_ENABLE);

    /* enable usage of Mflag at DMM */
    Utils_setDmmMflagEmergencyEnable(TRUE);

    /* Set DMM as higest priority at DMM and EMIF */
    Utils_setDmmPri(UTILS_DMM_INITIATOR_ID_DSS, 0);

    Utils_setBWLimiter(UTILS_DMM_INITIATOR_ID_GPU_P1, 1000);
    Utils_setBWLimiter(UTILS_DMM_INITIATOR_ID_GPU_P2, 1000);
}


/**
 *******************************************************************************
 *
 * \brief Handles the Commands received.
 *
 * \param   pObj           [IN] AppCtrl_Obj
 *
 * \param   cmd            [IN] input command
 *
 * \param   pPrm           [IN] input message
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AppCtrl_cmdHandler(AppCtrl_Obj * pObj, UInt32 cmd, Void * pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    switch (cmd)
    {

        case APP_CTRL_LINK_CMD_BOARD_ENABLE_USB_CHARGING:
            Board_enableUsbCharging();
            break;

        case APP_CTRL_LINK_CMD_BOARD_IS_MULTI_DES_CONNECTED:
        {
            AppCtrlCmd_BoardIsMultiDesConnectedPrm *pAppPrm
                = (AppCtrlCmd_BoardIsMultiDesConnectedPrm*)pPrm;

            pAppPrm->isConnected = Board_isMultiDesConnected();

            break;
        }

        case APP_CTRL_LINK_CMD_SET_DMM_PRIORITIES:
             AppCtrl_setSystemL3DmmPri();
            break;

        case APP_CTRL_LINK_CMD_SURROUNDVIEW_CALIBRATION:
        {
             Chain_Common_SRV_CalibParams  *calInfo
                = (Chain_Common_SRV_CalibParams*)pPrm;
             Chain_Common_SRV_Calibration(calInfo);
            break;
        }

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
Void AppCtrl_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    Int32 status;
    AppCtrl_Obj *pObj = (AppCtrl_Obj *) pTsk->appData;

    status = AppCtrl_cmdHandler(pObj,
                                   Utils_msgGetCmd(pMsg),
                                   Utils_msgGetPrm(pMsg));
    Utils_tskAckOrFreeMsg(pMsg, status);

    return;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the ipu systemLink
 *        Initializes the linkObj and creates the task for AppCtrl_tskMain
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AppCtrl_init()
{
    Int32 status;
    System_LinkObj linkObj;
    AppCtrl_Obj *pObj;
    char tskName[32];
    unsigned int coreId;

    pObj = &gAppCtrl_obj;

    memset(pObj, 0, sizeof(*pObj));
    coreId = System_getSelfProcId();
    pObj->tskId = SYSTEM_MAKE_LINK_ID(coreId, SYSTEM_LINK_ID_APP_CTRL);

    linkObj.pTsk = &pObj->tsk;
    linkObj.linkGetFullBuffers = NULL;
    linkObj.linkPutEmptyBuffers = NULL;
    linkObj.getLinkInfo = NULL;

    System_registerLink(pObj->tskId, &linkObj);

    sprintf(tskName, "APP_CTRL");

    status = Utils_tskCreate(&pObj->tsk,
                             AppCtrl_tskMain,
                             APP_CTRL_TSK_PRI,
                             gAppCtrl_tskStack,
                             sizeof(gAppCtrl_tskStack), pObj, tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete the AppCtrl_tskMain created
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AppCtrl_deInit()
{
    Utils_tskDelete(&gAppCtrl_obj.tsk);

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief   Entry point for application on IPU1-0
 *
 * \param   arg0    [IN]  default args
 *
 * \param   arg1    [IN]  default args
 *
 *******************************************************************************
*/
Void Chains_main(UArg arg0, UArg arg1)
{
    Void IPU1_0_main(UArg arg0, UArg arg1);

    Vps_printf(" CHAINS: Application Started !!!");

    AppCtrl_init();
    GrpxSrcLink_init();
    SplitLink_init();

    IPU1_0_main(NULL, NULL);

    AppCtrl_deInit();
    GrpxSrcLink_deInit();
    SplitLink_deInit();

    Vps_printf(" CHAINS: Application Exited !!!");
}

