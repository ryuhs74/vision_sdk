/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"


void handleSysReset()
{
    UInt32 prmSize;

    SendCommand(gNetworkCtrl_obj.command, NULL, 0);
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);
}

