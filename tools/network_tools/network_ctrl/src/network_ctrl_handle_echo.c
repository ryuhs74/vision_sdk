/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"


void handleEcho()
{
    UInt32 prmSize;

    SendCommand(gNetworkCtrl_obj.command, gNetworkCtrl_obj.params[0], strlen(gNetworkCtrl_obj.params[0])+1);
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);
}

