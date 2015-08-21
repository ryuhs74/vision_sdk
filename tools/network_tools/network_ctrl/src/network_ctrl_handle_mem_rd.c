/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"


void handleMemRd()
{
    UInt32 prm[2];
    UInt32 *pMem;
    UInt32 addr;
    int i;
    UInt32 prmSize = 0;

    prm[0] = xstrtoi(gNetworkCtrl_obj.params[0]);
    prm[1] = atoi(gNetworkCtrl_obj.params[1])*sizeof(UInt32);

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

        addr = prm[0];
        printf("# \n");
        for(i=0; i<prmSize/sizeof(UInt32); i++)
        {
            printf("# %s: 0x%08x = 0x%08x\n", gNetworkCtrl_obj.command, addr, pMem[i]);

            addr += sizeof(UInt32);
        }
        printf("# \n");
    }
}

void handleMemWr()
{
    UInt32 prm[2];
    UInt32 prmSize = 0;

    prm[0] = xstrtoi(gNetworkCtrl_obj.params[0]);
    prm[1] = xstrtoi(gNetworkCtrl_obj.params[1]);

    SendCommand(gNetworkCtrl_obj.command, prm, sizeof(prm));
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);

    if (0 == prmSize)
    {
        printf ("# Successfully writen Value 0x%x at address 0x%x #\n",
            prm[1], prm[0]);
    }
}
