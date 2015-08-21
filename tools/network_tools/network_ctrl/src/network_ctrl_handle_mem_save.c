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

void handleMemSave()
{
    UInt32 prm[2];
    UInt32 *pMem;
    UInt32 prmSize = 0;

    prm[0] = xstrtoi(gNetworkCtrl_obj.params[0]);
    prm[1] = atoi(gNetworkCtrl_obj.params[1]);

    SendCommand(gNetworkCtrl_obj.command, prm, sizeof(prm));
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);

    if(prmSize)
    {
        pMem = malloc(prmSize);
        if(pMem==NULL)
        {
            printf("# ERROR: Command %s: Unable to allocate memory for response parameters\n", gNetworkCtrl_obj.command);
            exit(0);
        }

        RecvResponseParams(gNetworkCtrl_obj.command, (UInt8*)pMem, prmSize);

        OSA_fileWriteFile(gNetworkCtrl_obj.params[2], (UInt8*)pMem, prmSize);
    }
}

