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
 * \file networkCtrl_tsk.c
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "network_ctrl_priv.h"

#define NETWORK_CTRL_TSK_STACK_SIZE  (16*KB)
#define NETWORK_CTRL_TSK_PRI         (4)


/**
 *******************************************************************************
 * \brief  Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gNetworkCtrl_tskStack, 32)
#pragma DATA_SECTION(gNetworkCtrl_tskStack, ".bss:taskStackSection")
UInt8 gNetworkCtrl_tskStack[NETWORK_CTRL_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief stores all module related information
 *******************************************************************************
 */
NetworkCtrl_Obj gNetworkCtrl_obj;

Void NetworkCtrl_cmdHandlerUnsupportedCmd(char *cmd, UInt32 prmSize)
{
    UInt8 *pBuf;
    Int32 status;

    /* alloc tmp buffer for parameters */
    if(prmSize)
    {
        pBuf = Utils_memAlloc( UTILS_HEAPID_DDR_CACHED_SR, prmSize, 32);
        UTILS_assert(pBuf != NULL);

        /* read parameters */
        NetworkCtrl_readParams(pBuf, prmSize);

        Vps_printf(" NETWORK_CTRL: %s: UNSUPPORTED CMD (prmSize=%d) !!!\n", cmd, prmSize);

        status = Utils_memFree( UTILS_HEAPID_DDR_CACHED_SR, pBuf, prmSize);
        UTILS_assert(status==0);
    }

    /* send response */
    NetworkCtrl_writeParams(NULL, 0, (UInt32)-1);
}

Int32 NetworkCtrl_registerHandler(char *cmd, NetworkCtrl_Handler handler)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int i;
    int firstFreeIdx;

    firstFreeIdx = -1;

    /* check if command is already registered */
    for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(pObj->cmdHandler[i].handler)
        {
            if(strncmp(
                pObj->cmdHandler[i].cmd,
                cmd,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
                /* command already register, exit with error */
                return -1;
            }
        }
        else
        {
            if(firstFreeIdx==-1)
            {
                firstFreeIdx = i;
            }
        }
    }

    /* no space to register command */
    if(firstFreeIdx==-1)
        return -1;

    /* command not registered, register it */
    pObj->cmdHandler[firstFreeIdx].handler = handler;
    strcpy(pObj->cmdHandler[firstFreeIdx].cmd, cmd);

    return 0;
}

Int32 NetworkCtrl_unregisterHandler(char *cmd)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    int i;

    /* check if command is already registered */
    for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
    {
        if(pObj->cmdHandler[i].handler)
        {
            if(strncmp(
                pObj->cmdHandler[i].cmd,
                cmd,
                NETWORK_CTRL_CMD_STRLEN_MAX)
                ==0)
            {
                /* command found, unregister it */
                pObj->cmdHandler[i].handler = NULL;
                pObj->cmdHandler[i].cmd[0] = 0;
            }
        }
    }

    return 0;
}

Int32 NetworkCtrl_readParams(UInt8 *pPrm, UInt32 prmSize)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if(prmSize)
    {
        status = Network_read(&pObj->sockObj, pPrm, &prmSize);

        if(status<0)
        {
            Vps_printf(
                " NETWORK_CTRL: Network_read() failed to read parameters (port=%d) !!!\n",
                    pObj->serverPort);
        }
    }

    return status;
}

Int32 NetworkCtrl_writeParams(UInt8 *pPrm, UInt32 prmSize, UInt32 returnStatus)
{
    NetworkCtrl_Obj *pObj = &gNetworkCtrl_obj;
    Int32 status;

    pObj->cmdBuf.prmSize = prmSize;
    pObj->cmdBuf.returnValue = returnStatus;
    pObj->cmdBuf.flags = NETWORK_CTRL_FLAG_ACK;

    status = Network_write(&pObj->sockObj, (UInt8*)&pObj->cmdBuf, sizeof(pObj->cmdBuf));

    if(status<0)
    {
        Vps_printf(
            " NETWORK_CTRL: Network_write() failed to write response header (port=%d) !!!\n",
                pObj->serverPort);

        return status;
    }

    if(prmSize)
    {
        status = Network_write(&pObj->sockObj, pPrm, prmSize);

        if(status<0)
        {
            Vps_printf(
                " NETWORK_CTRL: Network_write() failed to write parameters (port=%d) !!!\n",
                    pObj->serverPort);
        }
    }

    return status;
}

Void NetworkCtrl_tskMain(UArg arg0, UArg arg1)
{
    NetworkCtrl_Obj *pObj = (NetworkCtrl_Obj*)arg0;
    Int32 status;
    volatile UInt32 dataSize;

    Task_sleep(5*1000);

    Vps_printf(
        " NETWORK_CTRL: Starting Server (port=%d) !!!\n", pObj->serverPort
        );

    Network_sessionOpen(NULL);

    status = Network_open(&pObj->sockObj, pObj->serverPort);
    UTILS_assert(status==0);

    Vps_printf(
        " NETWORK_CTRL: Starting Server ... DONE (port=%d) !!!\n", pObj->serverPort
        );

    while(!gNetworkCtrl_obj.tskExit)
    {
        status = Network_waitConnect(&pObj->sockObj, 1000);

        if(status<0)
            break;

        if(status==0)
            continue;

        dataSize = sizeof(pObj->cmdBuf);

        /* read command header */
        status = Network_read(&pObj->sockObj, (UInt8*)&pObj->cmdBuf, (UInt32*)&dataSize);

        if(status==SYSTEM_LINK_STATUS_SOK && dataSize==sizeof(pObj->cmdBuf))
        {
            /* handle command */
            if(pObj->cmdBuf.header != NETWORK_CTRL_HEADER)
            {
                Vps_printf(" NETWORK_CTRL: Invalid header received (port=%d) !!!\n", pObj->serverPort);
            }
            else
            {
                Bool isCmdHandled;
                int i;

                isCmdHandled = FALSE;

                /* valid header received */
                for(i=0; i<NETWORK_CTRL_MAX_CMDS; i++)
                {
                    if(strncmp(
                        pObj->cmdHandler[i].cmd,
                        pObj->cmdBuf.cmd,
                        NETWORK_CTRL_CMD_STRLEN_MAX)
                        ==0)
                    {
                        /* matched a register command */

                        if(pObj->cmdHandler[i].handler)
                        {
                            Vps_printf(" NETWORK_CTRL: Received command [%s], with %d bytes of parameters\n",
                                pObj->cmdBuf.cmd,
                                pObj->cmdBuf.prmSize
                                );

                            pObj->cmdHandler[i].handler(
                                pObj->cmdBuf.cmd,
                                pObj->cmdBuf.prmSize
                                );

                            Vps_printf(" NETWORK_CTRL: Sent response for command [%s], with %d bytes of parameters\n",
                                pObj->cmdBuf.cmd,
                                pObj->cmdBuf.prmSize
                                );

                            isCmdHandled = TRUE;
                            break;
                        }
                    }
                }

                if(isCmdHandled == FALSE)
                {
                    /* if command is not handled, then read the params and ACK it with error */
                    NetworkCtrl_cmdHandlerUnsupportedCmd(
                        pObj->cmdBuf.cmd,
                        pObj->cmdBuf.prmSize
                        );
                }
            }
        }
        else
        {
            Vps_printf(" NETWORK_CTRL: recv() failed (port=%d) !!!\n", pObj->serverPort);
        }

        /* close socket */
        Network_close(&pObj->sockObj, FALSE);
    }

    Vps_printf(
        " NETWORK_CTRL: Closing Server (port=%d) !!!\n", pObj->serverPort
        );

    Network_close(&pObj->sockObj, TRUE);

    Network_sessionClose(NULL);

    Vps_printf(
        " NETWORK_CTRL: Closing Server ... DONE (port=%d) !!!\n", pObj->serverPort
        );
}


Int32 NetworkCtrl_init()
{
    memset(&gNetworkCtrl_obj, 0, sizeof(gNetworkCtrl_obj));

    gNetworkCtrl_obj.serverPort = NETWORK_CTRL_SERVER_PORT;

    NetworkCtrl_registerHandler("echo", NetworkCtrl_cmdHandlerEcho);
    NetworkCtrl_registerHandler("mem_rd", NetworkCtrl_cmdHandlerMemRd);
    NetworkCtrl_registerHandler("mem_wr", NetworkCtrl_cmdHandlerMemWr);
    NetworkCtrl_registerHandler("mem_save", NetworkCtrl_cmdHandlerMemSave);
    NetworkCtrl_registerHandler("iss_raw_save", NetworkCtrl_cmdHandlerIssRawSave);
    NetworkCtrl_registerHandler("iss_yuv_save", NetworkCtrl_cmdHandlerIssYuvSave);
    NetworkCtrl_registerHandler("iss_send_dcc_file", NetworkCtrl_cmdHandlerIssDccSendFile);
    NetworkCtrl_registerHandler("iss_save_dcc_file", NetworkCtrl_cmdHandlerIssSaveDccFile);
    NetworkCtrl_registerHandler("iss_clear_dcc_qspi_mem", NetworkCtrl_cmdHandlerIssClearDccQspiMem);
    NetworkCtrl_registerHandler("iss_write_sensor_reg", NetworkCtrl_cmdHandleIssWriteSensorReg);
    NetworkCtrl_registerHandler("iss_read_sensor_reg", NetworkCtrl_cmdHandleIssReadSensorReg);
    NetworkCtrl_registerHandler("iss_read_2a_params", NetworkCtrl_cmdHandleIssRead2AParams);
    NetworkCtrl_registerHandler("iss_write_2a_params", NetworkCtrl_cmdHandleIssWrite2AParams);
    NetworkCtrl_registerHandler("stereo_calib_image_save", NetworkCtrl_cmdHandlerStereoCalibImageSave);
    NetworkCtrl_registerHandler("stereo_calib_lut_to_qspi", NetworkCtrl_cmdHandlerStereoWriteCalibLUTToQSPI);
    NetworkCtrl_registerHandler("stereo_set_params", NetworkCtrl_cmdHandlerStereoSetParams);
    NetworkCtrl_registerHandler("stereo_set_dynamic_params", NetworkCtrl_cmdHandlerStereoSetDynamicParams);
    NetworkCtrl_registerHandler("qspi_wr", NetworkCtrl_cmdHandlerQspiWrite);
    NetworkCtrl_registerHandler("sys_reset", NetworkCtrl_cmdHandlerSysReset);
    /*
     * Create task
     */
    gNetworkCtrl_obj.task = BspOsal_taskCreate(
                                (BspOsal_TaskFuncPtr)NetworkCtrl_tskMain,
                                NETWORK_CTRL_TSK_PRI,
                                gNetworkCtrl_tskStack,
                                sizeof(gNetworkCtrl_tskStack),
                                &gNetworkCtrl_obj
                            );
    UTILS_assert(gNetworkCtrl_obj.task != NULL);

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 NetworkCtrl_deInit()
{
    gNetworkCtrl_obj.tskExit = TRUE;

    Task_sleep(1);

    BspOsal_taskDelete(&gNetworkCtrl_obj.task);

    NetworkCtrl_unregisterHandler("echo");

    return SYSTEM_LINK_STATUS_SOK;
}

