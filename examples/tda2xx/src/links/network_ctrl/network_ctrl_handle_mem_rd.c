/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"

#define NETWORK_MEM_RDWR_START_ADDR1      (0x40000000U)
#define NETWORK_MEM_RDWR_END_ADDR1        (0x60000000U)
#define NETWORK_MEM_RDWR_START_ADDR2      (0x80000000U)
#define NETWORK_MEM_RDWR_END_ADDR2        (0xC0000000U)

#define CHECK_ADDR(addr, a, b)  \
    (((addr) >= (a)) && ((addr) < (b)))

Void NetworkCtrl_cmdHandlerMemRd(char *cmd, UInt32 prmSize)
{
    UInt32 *pBuf, *pAddr;
    Int32 status = FVID2_SOK;
    UInt32 prm[2], size, i;

    memset((void*) prm, 0U, sizeof(prm));
    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(prm))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8*)prm, sizeof(prm));

        pAddr = (UInt32*)prm[0];
        size = prm[1];

        /* Check for errors */
        if (!
             (CHECK_ADDR(((UInt32)pAddr), NETWORK_MEM_RDWR_START_ADDR1,
                            NETWORK_MEM_RDWR_END_ADDR1) ||
              CHECK_ADDR(((UInt32)pAddr), NETWORK_MEM_RDWR_START_ADDR2,
                            NETWORK_MEM_RDWR_END_ADDR2))
           )
        {
            Vps_printf(" NetworkCtrl: Out of Range Address\n");
            status = FVID2_EINVALID_PARAMS;
        }

        if (!
             (CHECK_ADDR((((UInt32)pAddr)+size), NETWORK_MEM_RDWR_START_ADDR1,
                            NETWORK_MEM_RDWR_END_ADDR1) ||
              CHECK_ADDR((((UInt32)pAddr)+size), NETWORK_MEM_RDWR_START_ADDR2,
                            NETWORK_MEM_RDWR_END_ADDR2))
            )
        {
            Vps_printf(" NetworkCtrl: Incorrect size for this address\n");
            status = FVID2_EINVALID_PARAMS;
        }

        if (FVID2_SOK == status)
        {
            pBuf = (UInt32*)Utils_memAlloc( UTILS_HEAPID_DDR_CACHED_SR, size, 32);

            UTILS_assert(pBuf != NULL);

            for(i=0; i<size/sizeof(UInt32); i++)
            {
                if(pAddr != NULL)
                {
                    pBuf[i] = pAddr[i];
                }
                else
                {
                    pBuf[i] = 0U;
                }
            }

            /* send response */
            NetworkCtrl_writeParams((UInt8*)pBuf, size, 0);

            status = Utils_memFree( UTILS_HEAPID_DDR_CACHED_SR, pBuf, size);
            UTILS_assert(status==0);
        }
        else
        {
            /* send response */
            NetworkCtrl_writeParams(NULL, 0, (UInt32)-1);
        }
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandlerMemWr(char *cmd, UInt32 prmSize)
{
    Int32 status = FVID2_SOK;
    volatile UInt32 *pAddr;
    UInt32 prm[2], value;

    memset((void*) prm, 0U, sizeof(prm));
    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(prm))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8*)prm, sizeof(prm));

        pAddr = (volatile UInt32*)prm[0];
        value = prm[1];

        /* Check for errors */
        if (!
             (CHECK_ADDR(((UInt32)pAddr), NETWORK_MEM_RDWR_START_ADDR1,
                            NETWORK_MEM_RDWR_END_ADDR1) ||
              CHECK_ADDR(((UInt32)pAddr), NETWORK_MEM_RDWR_START_ADDR2,
                            NETWORK_MEM_RDWR_END_ADDR2))
           )
        {
            Vps_printf(" NetworkCtrl: Out of Range Address\n");
            status = FVID2_EINVALID_PARAMS;
        }

        if (FVID2_SOK == status)
        {
            /* TODO: Check for the valid range of pAddr */

            *pAddr = value;

            /* send response */
            NetworkCtrl_writeParams(NULL, 0, 0);
        }
        else
        {
            /* send response */
            NetworkCtrl_writeParams(NULL, 0, (UInt32)-1);
        }
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

