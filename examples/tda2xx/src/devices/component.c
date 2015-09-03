/*
 *******************************************************************************
 *
 * Copyright (C) 2015 CAMMSYS www.cammsys.net
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file component.c
 *
 * \brief  This file has the implementataion of component transmitter Control API
 *
 *         APIs can be used to control component transmitter.
 *         Drivers for component transmitter can be part of BSP or any other package.
 *
 *
 * \version 0.0 (Nov 2015) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <examples/tda2xx/include/component.h>

#if 1

/**
 *******************************************************************************
 *
 * \brief Set the default Create Params for Ypbpr transmitter.
 *
 * \param  createParams   [IN] Create parameters for component transmitter
 *
 *******************************************************************************
*/
Void Component_CreateParams_Init(Component_CreateParams *createParams)
{
    createParams->componentId       =  COMPONENT_CH7026;
    createParams->standard        =  SYSTEM_STD_1080P_60;
    createParams->boardMode       =  BSP_BOARD_MODE_VIDEO_24BIT;
}

/**
 *******************************************************************************
 *
 * \brief Create function to create component transmitter.
 *
 *        Creates the component transmitter handle using bsp function calls.
 *
 * \param  createParams   [IN] Create parameters for component transmitter
 *
 * \param  createStatus   [OUT] Status
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Component_create(Component_CreateParams *createParams,
                      Component_CreateStatus *createStatus)
{
    Int32  retVal = SYSTEM_LINK_STATUS_EFAIL;
    UInt32 vidEncInstId, vidEncI2cInstId, vidEncI2cAddr;
    UInt32 vidEncDrvId;
    Bsp_VidEncCreateParams encCreateParams;
  //  Bsp_Ch7026HpdParams hpdPrms;
    Bsp_Ch7026ChipId componentId;
    Bsp_VidEncConfigParams modePrms;
    Bsp_VidEncCreateStatus encCreateStatus;

    createStatus->retVal    = SYSTEM_LINK_STATUS_EFAIL;
    vidEncDrvId             = FVID2_VID_ENC_CH7026_DRV;

    if(createParams->componentId == COMPONENT_CH7026)
    {
        vidEncDrvId         = FVID2_VID_ENC_CH7026_DRV;
    }

     vidEncInstId = Bsp_boardGetVideoDeviceInstId(
        vidEncDrvId,
        FVID2_VPS_DCTRL_DRV,
        0);

    vidEncI2cInstId = Bsp_boardGetVideoDeviceI2cInstId(
        vidEncDrvId,
        FVID2_VPS_DCTRL_DRV,
        0);

    vidEncI2cAddr = Bsp_boardGetVideoDeviceI2cAddr(
        vidEncDrvId,
        FVID2_VPS_DCTRL_DRV,
        0);

    retVal = Bsp_boardSetPinMux(FVID2_VPS_DCTRL_DRV,
    							VPS_DSS_DISPC_OVLY_DPI1,
                                createParams->boardMode);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" COMPONENT: Pin Muxing Failed !!! \n");
    }

    /* Power on Video Encoder */
    retVal = Bsp_boardPowerOnDevice(vidEncDrvId, vidEncInstId, TRUE);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" COMPONENT: Device Power On failed !!!\n");
    }

    /* select Video Encoder at board level mux */
    retVal = Bsp_boardSelectDevice(vidEncDrvId, vidEncInstId);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" COMPONENT: Device select failed !!!\n");
    }

    /* Perform any reset needed at board level */
    retVal = Bsp_boardResetDevice(vidEncDrvId, vidEncInstId);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" COMPONENT: Device reset failed !!!\n");
    }

    /* Select specific mode */
    retVal = Bsp_boardSelectMode(
                 vidEncDrvId,
                 vidEncInstId,
                 createParams->boardMode);
    if (FVID2_SOK != retVal)
    {
        Vps_printf(" COMPONENT: Device select mode failed !!!\n");
    }

    if (FVID2_SOK == retVal)
    {
       /* Open YPBPR Tx */
       encCreateParams.deviceI2cInstId = vidEncI2cInstId;
       encCreateParams.deviceI2cAddr = vidEncI2cAddr;
       encCreateParams.inpClk = 0u;
       encCreateParams.hotPlugGpioIntrLine = 0u;
       encCreateParams.clkEdge = FALSE;

       createParams->component_Handle = Fvid2_create(
                                vidEncDrvId,
                                0u,
                                &encCreateParams,
                                &encCreateStatus,
                                NULL);
       if (NULL == createParams->component_Handle)
       {
           Vps_printf( " COMPONENT: "
                     " ERROR: SII9022 create failed !!!\n");
           retVal = FVID2_EFAIL;
       }
    }

#if 1
    if (FVID2_SOK == retVal)
    {
       retVal = Fvid2_control(
                    createParams->component_Handle,
					IOCTL_BSP_VID_ENC_GET_CHIP_ID,
                    &componentId,
                    NULL);
       if (FVID2_SOK != retVal)
       {
           Vps_printf( " COMPONENT: "
                     "ERROR:  Could not get detailed chip ID!!\n");
       }
       else
       {
           Vps_printf( " COMPONENT:"
                       " componentId.deviceId = %d,componentId.deviceProdRevId = %d,"
                       " componentId.hdcpRevTpi = %d, componentId.tpiRevId = %d\n",
                           componentId.deviceId,
                           componentId.deviceProdRevId,
                           componentId.hdcpRevTpi,
                           componentId.tpiRevId);
       }
    }
#if 0
    if (FVID2_SOK == retVal)
    {
       retVal = Fvid2_control(
                    createParams->component_Handle,
                    IOCTL_BSP_SII9022A_QUERY_HPD,
                    &hpdPrms,
                    NULL);
       if (FVID2_SOK != retVal)
       {
           Vps_printf( " COMPONENT: "
                     "ERROR:  Could not detect HPD!!\n");
       }
       else
       {
           Vps_printf( " COMPONENT: "
                       " hpdPrms.busError = %d,"
                       " hpdPrms.hpdEvtPending = %d, hpdPrms.hpdStatus = %d\n",
                        hpdPrms.busError,
                        hpdPrms.hpdEvtPending,
                        hpdPrms.hpdStatus);
       }
    }
#endif
    if (FVID2_SOK == retVal)
    {
       BspVidEncConfigParams_init(&modePrms);
       modePrms.standard = createParams->standard;
       modePrms.videoIfMode = FVID2_VIFM_SCH_DS_AVID_VSYNC;
       modePrms.videoIfWidth = FVID2_VIFW_24BIT;
       modePrms.videoDataFormat = FVID2_DF_RGB24_888;

       retVal = Fvid2_control(
                    createParams->component_Handle,
                    IOCTL_BSP_VID_ENC_SET_MODE,
                    &modePrms,
                    NULL);
       if (FVID2_SOK != retVal)
       {
           Vps_printf( " COMPONENT: "
                     "ERROR:  Could not set mode !!!\n");
       }
    }

    if ((FVID2_SOK != retVal) && (NULL != createParams->component_Handle))
    {
       /* Close HDMI transmitter */
       retVal += Fvid2_delete(createParams->component_Handle, NULL);
       createParams->component_Handle = NULL;
    }

#endif
    createStatus->retVal = retVal;

    return (createStatus->retVal);
}


/*******************************************************************************
 *
 * \brief Delete function to delete component transmitter.
 *
 *        Deletes the component transmitter handle using Fvid2_delete function calls.
 *
 * \param  createParams    [IN] Create parameters for component transmitter
 *
 * \param  deleteArgs      Not used.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Component_delete(Component_CreateParams *createParams, Ptr deleteArgs)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if(createParams->component_Handle != NULL)
    {
        status = Fvid2_delete((Fvid2_Handle)createParams->component_Handle, NULL);
    }

    return status;
}


/*******************************************************************************
 *
 * \brief Control function to start stop and reset component transmitter.
 *
 *        Control the transmitter operation like start and stop of the transmitter using
 *        Fvid2 calls.
 *        transmitter reset is performed using IOCTL call IOCTL_BSP_VID_DEC_RESET
 *
 * \param  handle        [IN] Handle to control component transmitter.
 *
 * \param  cmd           [IN] Control command for component transmitter.
 *
 * \param  cmdArgs       [IN] Arguments for command if any.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Component_control(Component_CreateParams *createParams,
                        UInt32 cmd,
                        Ptr    cmdArgs,
                        UInt32 cmdStatusArgs)
{

    Int32 status = SYSTEM_LINK_STATUS_SOK;

#if 0
    switch(cmd)
    {
        case COMPONENT_CMD_START:

                                status = Fvid2_start(
                                (Fvid2_Handle)createParams->component_Handle,
                                NULL);
                                break;

        case COMPONENT_CMD_STOP:

                                status = Fvid2_stop(
                                (Fvid2_Handle)createParams->component_Handle,
                                NULL);
                                break;

    }
#endif
    return status;
}

#endif
