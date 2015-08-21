/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file hdmi_tx.c
 *
 * \brief  This file has the implementataion of hdmi transmitter Control API
 *
 *         APIs can be used to control hdmi transmitter.
 *         Drivers for hdmi transmitter can be part of BSP or any other package.
 *
 *
 * \version 0.0 (Nov 2013) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <examples/tda2xx/include/hdmi_tx.h>



/**
 *******************************************************************************
 *
 * \brief Set the default Create Params for Hdmi transmitter.
 *
 * \param  createParams   [IN] Create parameters for hdmi transmitter
 *
 *******************************************************************************
*/
Void HdmiTx_CreateParams_Init(HdmiTx_CreateParams *createParams)
{
    createParams->hdmiTxId        =  HDMI_TX_SII_9022A;
    createParams->standard        =  SYSTEM_STD_1080P_60;
    createParams->dssOvlyId       =  SYSTEM_DSS_DISPC_OVLY_DPI1;
    createParams->boardMode       =  BSP_BOARD_MODE_VIDEO_24BIT;
}

/**
 *******************************************************************************
 *
 * \brief Create function to create hdmi transmitter.
 *
 *        Creates the hdmi transmitter handle using bsp function calls.
 *
 * \param  createParams   [IN] Create parameters for hdmi transmitter
 *
 * \param  createStatus   [OUT] Status
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 HdmiTx_create(HdmiTx_CreateParams *createParams,
                      HdmiTx_CreateStatus *createStatus)
{
    Int32  retVal = SYSTEM_LINK_STATUS_EFAIL;
    UInt32 vidEncInstId, vidEncI2cInstId, vidEncI2cAddr;
    UInt32 vidEncDrvId;
    Bsp_VidEncCreateParams encCreateParams;
    Bsp_Sii9022aHpdParams hpdPrms;
    Bsp_Sii9022aHdmiChipId hdmiId;
    Bsp_VidEncConfigParams modePrms;
    Bsp_VidEncCreateStatus encCreateStatus;

    createStatus->retVal    = SYSTEM_LINK_STATUS_EFAIL;
    vidEncDrvId             = FVID2_VID_ENC_SII9022A_DRV;

    if(createParams->hdmiTxId == HDMI_TX_SII_9022A)
    {
        vidEncDrvId         = FVID2_VID_ENC_SII9022A_DRV;
    }

     vidEncInstId = Bsp_boardGetVideoDeviceInstId(
        vidEncDrvId,
        FVID2_VPS_DCTRL_DRV,
        SYSTEM_DSS_DISPC_OVLY_DPI1);

    vidEncI2cInstId = Bsp_boardGetVideoDeviceI2cInstId(
        vidEncDrvId,
        FVID2_VPS_DCTRL_DRV,
        createParams->dssOvlyId);

    vidEncI2cAddr = Bsp_boardGetVideoDeviceI2cAddr(
        vidEncDrvId,
        FVID2_VPS_DCTRL_DRV,
        createParams->dssOvlyId);

    retVal = Bsp_boardSetPinMux(FVID2_VPS_DCTRL_DRV,
                                createParams->dssOvlyId,
                                createParams->boardMode);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" HDMI_TX: Pin Muxing Failed !!! \n");
    }

    /* Power on Video Encoder */
    retVal = Bsp_boardPowerOnDevice(vidEncDrvId, vidEncInstId, TRUE);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" HDMI_TX: Device Power On failed !!!\n");
    }

    /* select Video Encoder at board level mux */
    retVal = Bsp_boardSelectDevice(vidEncDrvId, vidEncInstId);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" HDMI_TX: Device select failed !!!\n");
    }

    /* Perform any reset needed at board level */
    retVal = Bsp_boardResetDevice(vidEncDrvId, vidEncInstId);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" HDMI_TX: Device reset failed !!!\n");
    }

    /* Select specific mode */
    retVal = Bsp_boardSelectMode(
                 vidEncDrvId,
                 vidEncInstId,
                 createParams->boardMode);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" HDMI_TX: Device select mode failed !!!\n");
    }

    if (FVID2_SOK == retVal)
    {
       /* Open HDMI Tx */
       encCreateParams.deviceI2cInstId = vidEncI2cInstId;
       encCreateParams.deviceI2cAddr = vidEncI2cAddr;
       encCreateParams.inpClk = 0u;
       encCreateParams.hotPlugGpioIntrLine = 0u;
       encCreateParams.clkEdge = FALSE;

       createParams->hdmiTxHandle = Fvid2_create(
                                vidEncDrvId,
                                0u,
                                &encCreateParams,
                                &encCreateStatus,
                                NULL);
       if (NULL == createParams->hdmiTxHandle)
       {
           Vps_printf( " HDMI_TX: "
                     " ERROR: SII9022 create failed !!!\n");
           retVal = FVID2_EFAIL;
       }
    }

    if (FVID2_SOK == retVal)
    {
       retVal = Fvid2_control(
                    createParams->hdmiTxHandle,
                    IOCTL_BSP_SII9022A_GET_DETAILED_CHIP_ID,
                    &hdmiId,
                    NULL);
       if (FVID2_SOK != retVal)
       {
           Vps_printf( " HDMI_TX: "
                     "ERROR:  Could not get detailed chip ID!!\n");
       }
       else
       {
           Vps_printf( " HDMI_TX:"
                       " hdmiId.deviceId = %d,hdmiId.deviceProdRevId = %d,"
                       " hdmiId.hdcpRevTpi = %d, hdmiId.tpiRevId = %d\n",
                           hdmiId.deviceId,
                           hdmiId.deviceProdRevId,
                           hdmiId.hdcpRevTpi,
                           hdmiId.tpiRevId);
       }
    }

    if (FVID2_SOK == retVal)
    {
       retVal = Fvid2_control(
                    createParams->hdmiTxHandle,
                    IOCTL_BSP_SII9022A_QUERY_HPD,
                    &hpdPrms,
                    NULL);
       if (FVID2_SOK != retVal)
       {
           Vps_printf( " HDMI_TX: "
                     "ERROR:  Could not detect HPD!!\n");
       }
       else
       {
           Vps_printf( " HDMI_TX: "
                       " hpdPrms.busError = %d,"
                       " hpdPrms.hpdEvtPending = %d, hpdPrms.hpdStatus = %d\n",
                        hpdPrms.busError,
                        hpdPrms.hpdEvtPending,
                        hpdPrms.hpdStatus);
       }
    }

    if (FVID2_SOK == retVal)
    {
       BspVidEncConfigParams_init(&modePrms);
       modePrms.standard = createParams->standard;
       modePrms.videoIfMode = FVID2_VIFM_SCH_DS_AVID_VSYNC;
       modePrms.videoIfWidth = FVID2_VIFW_24BIT;
       modePrms.videoDataFormat = FVID2_DF_RGB24_888;

       retVal = Fvid2_control(
                    createParams->hdmiTxHandle,
                    IOCTL_BSP_VID_ENC_SET_MODE,
                    &modePrms,
                    NULL);
       if (FVID2_SOK != retVal)
       {
           Vps_printf( " HDMI_TX: "
                     "ERROR:  Could not set mode !!!\n");
       }
    }

    if ((FVID2_SOK != retVal) && (NULL != createParams->hdmiTxHandle))
    {
       /* Close HDMI transmitter */
       retVal += Fvid2_delete(createParams->hdmiTxHandle, NULL);
       createParams->hdmiTxHandle = NULL;
    }


    createStatus->retVal = retVal;

    return (createStatus->retVal);
}


/*******************************************************************************
 *
 * \brief Delete function to delete hdmi transmitter.
 *
 *        Deletes the hdmi transmitter handle using Fvid2_delete function calls.
 *
 * \param  createParams    [IN] Create parameters for hdmi transmitter
 *
 * \param  deleteArgs      Not used.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 HdmiTx_delete(HdmiTx_CreateParams *createParams, Ptr deleteArgs)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if(createParams->hdmiTxHandle != NULL)
    {
        status = Fvid2_delete((Fvid2_Handle)createParams->hdmiTxHandle, NULL);
    }

    return status;
}


/*******************************************************************************
 *
 * \brief Control function to start stop and reset hdmi transmitter.
 *
 *        Control the transmitter operation like start and stop of the transmitter using
 *        Fvid2 calls.
 *        transmitter reset is performed using IOCTL call IOCTL_BSP_VID_DEC_RESET
 *
 * \param  handle        [IN] Handle to control hdmi transmitter.
 *
 * \param  cmd           [IN] Control command for hdmi transmitter.
 *
 * \param  cmdArgs       [IN] Arguments for command if any.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 HdmiTx_control(HdmiTx_CreateParams *createParams,
                        UInt32 cmd,
                        Ptr    cmdArgs,
                        UInt32 cmdStatusArgs)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    switch(cmd)
    {
        case HDMI_TX_CMD_START:

                                status = Fvid2_start(
                                (Fvid2_Handle)createParams->hdmiTxHandle,
                                NULL);
                                break;

        case HDMI_TX_CMD_STOP:

                                status = Fvid2_stop(
                                (Fvid2_Handle)createParams->hdmiTxHandle,
                                NULL);
                                break;

    }
    return status;
}
