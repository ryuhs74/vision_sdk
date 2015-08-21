/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"

Void NetworkCtrl_cmdHandlerMemSave(char *cmd, UInt32 prmSize)
{
    UInt32 *pAddr;
    UInt32 prm[2], size;

    memset((void*) prm, 0U, sizeof(prm));
    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(prm))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8*)prm, sizeof(prm));

        pAddr = (UInt32*)prm[0];
        size = prm[1];

        Cache_inv(
            (xdc_Ptr)SystemUtils_floor((UInt32)pAddr, 128),
            SystemUtils_align(size+128, 128),
            Cache_Type_ALLD, TRUE
            );

        /* send response */
        NetworkCtrl_writeParams((UInt8*)pAddr, size, 0);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}
