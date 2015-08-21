/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"
#include <src/utils_common/include/utils_qspi.h>
#include <src/utils_common/include/utils_mem.h>

#define NETWORKCTRL_APP_IMAGE_FILE_SIZE     (32*1024*1024U)


Void NetworkCtrl_cmdHandlerQspiWrite(char *cmd, UInt32 prmSize)
{
    Int32 status = FVID2_SOK;
    UInt32 qSpiOffset;
    UInt8 *gDataBuf = NULL;

    gDataBuf = Utils_memAlloc(
            UTILS_HEAPID_DDR_CACHED_SR,
            NETWORKCTRL_APP_IMAGE_FILE_SIZE,
            8);
    UTILS_assert(gDataBuf != NULL);

    Vps_printf(" NETWORK_CTRL: Max file size that can be flashed to QSPI is %d B\n",NETWORKCTRL_APP_IMAGE_FILE_SIZE);
    Vps_printf(" NETWORK_CTRL: Received file size is %d B\n",prmSize);

    if (prmSize > NETWORKCTRL_APP_IMAGE_FILE_SIZE)
    {
        Vps_printf(" NETWORK_CTRL ERROR : Max file size that can be flashed to QSPI is %d B \
                     but received file size is %d B \n",NETWORKCTRL_APP_IMAGE_FILE_SIZE,prmSize);
        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);
    }
    else
    {
        /* read parameters */
        NetworkCtrl_readParams(gDataBuf, prmSize);

        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);

        qSpiOffset = *((UInt32 *)gDataBuf);

        Vps_printf(" NETWORK_CTRL: Flashing %d B of file to QSPI at offset 0x%08x ...\n",(prmSize - 1U),qSpiOffset); 

         /* QSPI Init is needed here otherwise erase/write will not happen even though we initialized during start  */
        System_qspiInit();
        System_qspiWriteSector(qSpiOffset,
                               (UInt32)(((UInt32 *)gDataBuf) + 1U),
                               SystemUtils_align((prmSize - 1U),SYSTEM_QSPI_FLASH_BLOCK_SIZE));
        Vps_printf(" QSPI Flash Complete !!!\n");
    }

    status = Utils_memFree( UTILS_HEAPID_DDR_CACHED_SR, gDataBuf, prmSize);
    UTILS_assert(status==0);

}
