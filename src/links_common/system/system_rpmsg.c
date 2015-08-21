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
 * \file system_rpmsg.c
 *
 * \brief  This file implements System_rpmsg module used by System_rpmsgNotify
 *         and System_rpmsgMessageQ for IPC. This is done to move system from
 *         from multiendpt to single endpt per remote core.
 *
 *         RPMessage is used for notify between A15 and IPU/DSP
 *         when A15_TARGET_OS is Linux
 *
 * \version 0.0 (Mar 2015) : [YM] First version implemented
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <src/links_common/system/system_priv_ipc.h>
#include <ti/ipc/MultiProc.h>

#define RPMSG_NS_2_0

#include <ti/ipc/rpmsg/NameMap.h>
#include <ti/ipc/rpmsg/RPMessage.h>

typedef struct {
    RPMessage_Handle rpmsgHandle;
    /**< Handle for IPC notify */
    UInt32 localEndpoint;
    /**< RPMessage end point */
} System_RpMsg_Obj;

System_RpMsg_Obj gSystem_rpmsgObj;

/**
 *******************************************************************************
 *
 * \brief System wide notify handler function.
 *
 *        This function gets registered via System_ipcRegisterNotifyCb.
 *        Based on
 *        payload linkId is derived. From linkID callback of the specific
 *        link gets invoked.
 *
 * \param   handle      [IN] RP Message handle
 * \param   arg         [IN] not used
 * \param   payload     [IN] 4-byte notify payload. From this procId and
 *                           linkId is derived to invoke link specific
 *                           callback function
 * \param   len         [IN] length of payload
 * \param   src         [IN] source of the notify
 *
 *******************************************************************************
 */

static Void System_rpmsgCbHandler(RPMessage_Handle handle,
                                  UArg arg,
                                  Ptr payload,
                                  UInt16 len,
                                  UInt32 src)
{
    UInt32 payloadValue;

    payloadValue = *(volatile UInt32 *)payload;

    #ifdef BUILD_M4_0
    if(SYSTEM_LINK_ID_TEST_ROUTE_BIT_TRUE(payloadValue))
    {
        SYSTEM_LINK_ID_CLEAR_ROUTE_BIT(payloadValue);
        System_ipcNotifySendEvent(payloadValue);
        return;
    }
    #endif

    System_ipcHandler(payloadValue);
}

/**
 *******************************************************************************
 *
 * \brief Function to create RPMessage instance and announce rpmsg service
 *
 *        It registers a callback function to receive commands from A15
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_rpmsgInit()
{
    memset(&gSystem_rpmsgObj, 0, sizeof(gSystem_rpmsgObj));

    /* Create the messageQ for receiving, and register callback: */
    gSystem_rpmsgObj.rpmsgHandle =
            RPMessage_create(
                    SYSTEM_RPMSG_ENDPT_REMOTE,
                    System_rpmsgCbHandler,
                    NULL,
                    &gSystem_rpmsgObj.localEndpoint
                    );
    UTILS_assert(gSystem_rpmsgObj.rpmsgHandle!=NULL);

    /* Announce we are here: */
    NameMap_register(
            "rpmsg-proto",
            "rpmsg-proto",
            SYSTEM_RPMSG_ENDPT_REMOTE
            );

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 System_rpmsgDeInit()
{
    NameMap_unregister(
            "rpmsg-proto",
            "rpmsg-proto",
            SYSTEM_RPMSG_ENDPT_REMOTE);

    RPMessage_delete(&gSystem_rpmsgObj.rpmsgHandle);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Utility function to invoke the notify to remote processor.
 *
 *        This function generates the notify event. It takes linkId as input
 *        maps linkId with procId and generates notify event on that processor
 *
 * \param   linkId      [IN] Link Id to which notify needs to be generated.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_rpmsgSendNotify(UInt32 linkId)
{
    UInt32 procId = SYSTEM_GET_PROC_ID(linkId);
    Int32 status;

    UTILS_assert(procId < SYSTEM_PROC_MAX);

    /* RPMessage exists only for communication with A15 (running Linux)*/
    UTILS_assert(procId == SYSTEM_PROC_A15_0);

#ifdef SYSTEM_IPC_MSGQ_DEBUG
    Vps_printf(" SYSTEM: NOTIFY: Sending Notify to [%s] !!!\n", System_getProcName(procId));
#endif

    /* Payload in the message is a linkId */
    status = RPMessage_send(System_getSyslinkProcId(SYSTEM_PROC_A15_0),
                            SYSTEM_RPMSG_NOTIFY_ENDPT_HOST,
                            gSystem_rpmsgObj.localEndpoint,
                            (Ptr) &linkId,
                            sizeof(UInt32));

    if (status != RPMessage_S_SUCCESS)
    {
        Vps_printf
            (" SYSTEM: RPMSG: Send to A15 failed !!! (status = %d)\n", status);

        UTILS_assert(status == RPMessage_S_SUCCESS);
    }

    return status;
}

