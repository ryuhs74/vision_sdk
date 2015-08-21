/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"
#include <include/link_api/issCaptureLink.h>
#include <include/link_api/issM2mSimcopLink.h>
#include <include/link_api/algorithmLink_issAewb.h>
#include <src/utils_common/include/utils_qspi.h>
#include <examples/tda2xx/include/chains_common_iss.h>

#define NETWORKCTRL_ISS_DCC_FILE_SIZE           (100*1024U)

Void NetworkCtrl_cmdHandlerIssRawSave(char *cmd, UInt32 prmSize)
{
    IssCaptureLink_GetSaveFrameStatus saveFrameStatus;
    UInt32 linkId, retry;
    Int32 status;

    /* alloc tmp buffer for parameters */
    if(prmSize == 0)
    {
        /* NO parameters to read */
        linkId = SYSTEM_LINK_ID_ISSCAPTURE_0;

        System_linkControl(
            linkId,
            ISSCAPTURE_LINK_CMD_SAVE_FRAME,
            NULL,
            0,
            TRUE
            );

        retry = 5;
        status = SYSTEM_LINK_STATUS_EFAIL;
        while(retry--)
        {
            Task_sleep(40);

            memset(&saveFrameStatus, 0, sizeof(saveFrameStatus));

            status = System_linkControl(
                linkId,
                ISSCAPTURE_LINK_CMD_GET_SAVE_FRAME_STATUS,
                &saveFrameStatus,
                sizeof(saveFrameStatus),
                TRUE
                );

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                /* raw data saving not enabled or use-case not running
                 */
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
        NetworkCtrl_writeParams((UInt8*)saveFrameStatus.bufAddr, saveFrameStatus.bufSize, status);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandlerIssYuvSave(char *cmd, UInt32 prmSize)
{
    IssM2mSimcopLink_GetSaveFrameStatus saveFrameStatus;
    UInt32 linkId, retry;
    Int32 status;

    /* alloc tmp buffer for parameters */
    if(prmSize == 0)
    {
        /* NO parameters to read */
        linkId = SYSTEM_LINK_ID_ISSM2MSIMCOP_0;

        System_linkControl(
            linkId,
            ISSM2MSIMCOP_LINK_CMD_SAVE_FRAME,
            NULL,
            0,
            TRUE
            );

        retry = 5;
        status = SYSTEM_LINK_STATUS_EFAIL;
        while(retry--)
        {
            Task_sleep(40);

            memset(&saveFrameStatus, 0, sizeof(saveFrameStatus));

            status = System_linkControl(
                linkId,
                ISSM2MSIMCOP_LINK_CMD_GET_SAVE_FRAME_STATUS,
                &saveFrameStatus,
                sizeof(saveFrameStatus),
                TRUE
                );

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                /* raw data saving not enabled or use-case not running
                 */
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
        NetworkCtrl_writeParams((UInt8*)saveFrameStatus.bufAddr, saveFrameStatus.bufSize, status);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandlerIssDccSendFile(char *cmd, UInt32 prmSize)
{
    Int32 status = FVID2_SOK;
    UInt32 linkId;
    AlgorithmLink_IssAewbDccControlParams dccCtrlPrms;

    if (prmSize > NETWORKCTRL_ISS_DCC_FILE_SIZE)
    {
        Vps_printf(" NETWORK_CTRL: Insufficient DCC Buffer\n");
        Vps_printf(" NETWORK_CTRL: Increase DCC buffer size in NETWORKCTRL_ISS_DCC_FILE_SIZE\n");

        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);

        return ;
    }

    if(prmSize)
    {
        linkId = IPU1_0_LINK (SYSTEM_LINK_ID_ALG_0);

        dccCtrlPrms.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_GET_DCC_BUF_PARAMS;
        dccCtrlPrms.baseClassControl.size = sizeof(dccCtrlPrms);

        /* get results */
        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &dccCtrlPrms,
            sizeof(dccCtrlPrms),
            TRUE);
        UTILS_assert(0 == status);

        if (NULL == dccCtrlPrms.dccBuf)
        {
            Vps_printf(" NETWORK_CTRL: DCC Buffer is NULL");

            /* send response */
            NetworkCtrl_writeParams(NULL, 0, 0);

            return ;
        }

        /* read parameters */
        NetworkCtrl_readParams(dccCtrlPrms.dccBuf, prmSize);

        Vps_printf(" NETWORK_CTRL: %s:", cmd);

        dccCtrlPrms.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_PARSE_AND_SET_DCC_PARAMS;
        dccCtrlPrms.baseClassControl.size = sizeof(dccCtrlPrms);
        dccCtrlPrms.dccBufSize = prmSize;
        dccCtrlPrms.pIspCfg = NULL;
        dccCtrlPrms.pSimcopCfg = NULL;

        /* get results */
        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &dccCtrlPrms,
            sizeof(dccCtrlPrms),
            TRUE);
        UTILS_assert(0 == status);
    }

    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);
}

Void NetworkCtrl_cmdHandlerIssSaveDccFile(char *cmd, UInt32 prmSize)
{
    Int32 status = FVID2_SOK;
    UInt32 linkId;
    AlgorithmLink_IssAewbDccControlParams dccCtrlPrms;

    if (prmSize > NETWORKCTRL_ISS_DCC_FILE_SIZE)
    {
        Vps_printf(" NETWORK_CTRL: Insufficient DCC Buffer\n");
        Vps_printf(" NETWORK_CTRL: Increase DCC buffer size in NETWORKCTRL_ISS_DCC_FILE_SIZE\n");

        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);

        return ;
    }

    /* alloc tmp buffer for parameters */
    if(prmSize)
    {
        linkId = IPU1_0_LINK (SYSTEM_LINK_ID_ALG_0);

        dccCtrlPrms.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_GET_DCC_BUF_PARAMS;
        dccCtrlPrms.baseClassControl.size = sizeof(dccCtrlPrms);

        /* get results */
        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &dccCtrlPrms,
            sizeof(dccCtrlPrms),
            TRUE);
        UTILS_assert(0 == status);

        if (NULL == dccCtrlPrms.dccBuf)
        {
            Vps_printf(" NETWORK_CTRL: DCC Buffer is NULL");

            /* send response */
            NetworkCtrl_writeParams(NULL, 0, 0);

            return ;
        }

        /* read parameters */
        NetworkCtrl_readParams(dccCtrlPrms.dccBuf, prmSize);

        dccCtrlPrms.dccBufSize = prmSize;
        linkId = SYSTEM_LINK_ID_IPU1_0;
        System_linkControl(
            linkId,
            SYSTEM_LINK_CMD_SAVE_DCC_FILE,
            &dccCtrlPrms,
            sizeof(dccCtrlPrms),
            TRUE
        );
    }

    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);
}

Void NetworkCtrl_cmdHandlerIssClearDccQspiMem(char *cmd, UInt32 prmSize)
{
    UInt32 linkId;
    UInt32 dccCameraId;

    memset((void*) &dccCameraId, 0U, sizeof(dccCameraId));

    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(dccCameraId))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8*)&dccCameraId, sizeof(dccCameraId));

        linkId = SYSTEM_LINK_ID_IPU1_0;
        System_linkControl(
            linkId,
            SYSTEM_LINK_CMD_CLEAR_DCC_QSPI_MEM,
            &dccCameraId,
            sizeof(dccCameraId),
            TRUE);

        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandleIssWriteSensorReg(char *cmd, UInt32 prmSize)
{
    UInt32 linkId;
    UInt32 sensorRegRdWr[3] = {0};

    memset(sensorRegRdWr, 0U, sizeof(sensorRegRdWr));

    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(sensorRegRdWr))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8 *)sensorRegRdWr, sizeof(sensorRegRdWr));

        linkId = SYSTEM_LINK_ID_IPU1_0;
        System_linkControl(
            linkId,
            SYSTEM_LINK_CMD_WRITE_SENSOR_REG,
            &sensorRegRdWr,
            sizeof(sensorRegRdWr),
            TRUE);

        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandleIssReadSensorReg(char *cmd, UInt32 prmSize)
{
    UInt32 linkId;
    UInt32 sensorRegRdWr[3] = {0};

    memset(sensorRegRdWr, 0U, sizeof(sensorRegRdWr));

    /* alloc tmp buffer for parameters */
    if (prmSize <= sizeof(sensorRegRdWr))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8 *)sensorRegRdWr, prmSize);

        linkId = SYSTEM_LINK_ID_IPU1_0;
        System_linkControl(
            linkId,
            SYSTEM_LINK_CMD_READ_SENSOR_REG,
            &sensorRegRdWr,
            sizeof(sensorRegRdWr),
            TRUE);

        /* send response */
        NetworkCtrl_writeParams((UInt8 *)&sensorRegRdWr[2],
            sizeof(sensorRegRdWr[2]), 0);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandleIssRead2AParams(char *cmd, UInt32 prmSize)
{
    Int32 status;
    UInt32 linkId;
    AlgorithmLink_IssAewb2AControlParams aewb2ACtrlPrms;

    memset(&aewb2ACtrlPrms, 0U, sizeof(AlgorithmLink_IssAewb2AControlParams));

    if (prmSize == sizeof(AlgorithmLink_IssAewb2AParams))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8 *)(&aewb2ACtrlPrms.aewb2APrms), prmSize);

        /* Hardcoding to ALG_0 since there is only one algo
           running for ISS usecase */
        linkId = IPU1_0_LINK (SYSTEM_LINK_ID_ALG_0);

        aewb2ACtrlPrms.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_GET_2A_PARAMS;
        aewb2ACtrlPrms.baseClassControl.size = sizeof(aewb2ACtrlPrms);

        /* get results */
        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &aewb2ACtrlPrms,
            sizeof(aewb2ACtrlPrms),
            TRUE);
        UTILS_assert(0 == status);

        /* send response */
        NetworkCtrl_writeParams((UInt8 *)&aewb2ACtrlPrms.aewb2APrms,
            sizeof(AlgorithmLink_IssAewb2AParams), 0);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

Void NetworkCtrl_cmdHandleIssWrite2AParams(char *cmd, UInt32 prmSize)
{
    Int32 status;
    UInt32 linkId;
    AlgorithmLink_IssAewb2AControlParams aewb2ACtrlPrms;

    memset(&aewb2ACtrlPrms, 0U, sizeof(AlgorithmLink_IssAewb2AControlParams));

    if (prmSize == sizeof(AlgorithmLink_IssAewb2AParams))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8 *)(&aewb2ACtrlPrms.aewb2APrms), prmSize);

        /* Hardcoding to ALG_0 since there is only one algo
           running for ISS usecase */
        linkId = IPU1_0_LINK (SYSTEM_LINK_ID_ALG_0);

        aewb2ACtrlPrms.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_SET_2A_PARAMS;
        aewb2ACtrlPrms.baseClassControl.size = sizeof(aewb2ACtrlPrms);

        /* get results */
        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &aewb2ACtrlPrms,
            sizeof(aewb2ACtrlPrms),
            TRUE);
        UTILS_assert(0 == status);

        /* send response */
        NetworkCtrl_writeParams(NULL, 0, 0);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
}

