/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"
#include <include/link_api/algorithmLink_remapMerge.h>
#include <src/utils_common/include/utils_qspi.h>

Void NetworkCtrl_cmdHandlerStereoCalibImageSave
                                                    (char *cmd, UInt32 prmSize)
{
    AlgorithmLink_RemapMergeSaveFrameStatus saveFrameStatus;
    UInt32 linkId, retry;
    Int32 status;

    /* alloc tmp buffer for parameters */
    if(prmSize == 0)
    {
        /* NO parameters to read */
        linkId = EVE2_LINK (SYSTEM_LINK_ID_ALG_0);

        saveFrameStatus.baseClassControl.controlCmd =
            REMAP_LINK_CMD_SAVE_FRAME;
        saveFrameStatus.baseClassControl.size = sizeof(saveFrameStatus);

        /* get results */
        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &saveFrameStatus,
            sizeof(saveFrameStatus),
            TRUE);
        UTILS_assert(0 == status);

        retry = 5;
        status = SYSTEM_LINK_STATUS_EFAIL;
        while(retry--)
        {
            Task_sleep(40);

            memset(&saveFrameStatus, 0, sizeof(saveFrameStatus));

            saveFrameStatus.baseClassControl.controlCmd =
                REMAP_LINK_CMD_GET_SAVE_FRAME_STATUS;
            saveFrameStatus.baseClassControl.size = sizeof(saveFrameStatus);

            /* get results */
            status = System_linkControl(
                linkId,
                ALGORITHM_LINK_CMD_CONFIG,
                &saveFrameStatus,
                sizeof(saveFrameStatus),
                TRUE);
            UTILS_assert(0 == status);

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                /* raw data saving not enabled or use-case not running*/
                break;
            }
            if(saveFrameStatus.isSaveFrameComplete)
            {
                break;
            }
        }

        if(status!=SYSTEM_LINK_STATUS_SOK)
        {
            /* some error, could not save raw data */
            saveFrameStatus.bufAddr = 0;
            saveFrameStatus.bufSize = 0;
        }
        else
        {
            Cache_inv(
                (xdc_Ptr)SystemUtils_floor(saveFrameStatus.bufAddr, 128),
                SystemUtils_align(saveFrameStatus.bufSize+128, 128),
                Cache_Type_ALLD, TRUE
                );
        }

        /* send response */
        NetworkCtrl_writeParams
            ((UInt8*)saveFrameStatus.bufAddr, saveFrameStatus.bufSize, status);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandlerStereoWriteCalibLUTToQSPI
                                                    (char *cmd, UInt32 prmSize)
{
    UInt32 qSpiOffset, size;
    UInt8 *pCalibLUTBuf;

    /* alloc tmp buffer for parameters */
    if(prmSize)
    {
        
        pCalibLUTBuf = Utils_memAlloc(
                            UTILS_HEAPID_DDR_CACHED_SR,
                            prmSize,
                            32);
        if (NULL == pCalibLUTBuf)
        {
            Vps_printf(" NETWORK_CTRL: Calib LUT Buffer is NULL");

            /* send response */
            NetworkCtrl_writeParams(NULL, 0, 0);
            return ;
        }

        /* read parameters */
        NetworkCtrl_readParams(pCalibLUTBuf, prmSize);

        /* Third word contains the QSPI offset */
        qSpiOffset = *(((UInt32 *)pCalibLUTBuf) + 2U);
        size = *(((UInt32 *)pCalibLUTBuf) + 1U);
        Vps_printf(" NETWORK_CTRL: qSpiOffset = %u, size = %u", qSpiOffset, size);
        /* QSPI Init is needed here otherwise erase/write will not happen even though we initialized during start  */
        System_qspiInit();
        /* Write complete bin file */
        System_qspiWriteSector(qSpiOffset,
                               (UInt32)pCalibLUTBuf,
                               SystemUtils_align(size, SYSTEM_QSPI_FLASH_BLOCK_SIZE));
    }

    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);
    Utils_memFree(
                UTILS_HEAPID_DDR_CACHED_SR,
                pCalibLUTBuf,
                prmSize);
}


