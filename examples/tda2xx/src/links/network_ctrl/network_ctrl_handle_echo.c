/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"


Void NetworkCtrl_cmdHandlerEcho(char *cmd, UInt32 prmSize)
{

    UInt8 *pBuf;
    Int32 status;

    /* alloc tmp buffer for parameters */
    if(prmSize)
    {
        pBuf = Utils_memAlloc( UTILS_HEAPID_DDR_CACHED_LOCAL, prmSize, 32);
        UTILS_assert(pBuf != NULL);

        /* read parameters */
        NetworkCtrl_readParams(pBuf, prmSize);

        Vps_printf(" NETWORK_CTRL: %s: %s", cmd, pBuf);

        status = Utils_memFree( UTILS_HEAPID_DDR_CACHED_LOCAL, pBuf, prmSize);
        UTILS_assert(status==0);
    }

    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);

}

