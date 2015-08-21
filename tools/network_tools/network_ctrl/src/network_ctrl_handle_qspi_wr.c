/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"
#include <osa_file.h>

#define NETWORKCTRL_APP_IMAGE_FILE_SIZE   (32*1024*1024U)

UInt32   pDataBuf[NETWORKCTRL_APP_IMAGE_FILE_SIZE];

void handleQspiSendFile()
{
    Int32 status;
    UInt32 size;

    status = OSA_fileReadFile(
        gNetworkCtrl_obj.params[1],
        (UInt8 *)(pDataBuf + 1U),
        NETWORKCTRL_APP_IMAGE_FILE_SIZE,
        &size);

    if (0 == status)
    {
        *pDataBuf = xstrtoi(gNetworkCtrl_obj.params[0]);

        SendCommand(gNetworkCtrl_obj.command, pDataBuf, size + 1U);
        RecvResponse(gNetworkCtrl_obj.command, NULL);
    }
}

