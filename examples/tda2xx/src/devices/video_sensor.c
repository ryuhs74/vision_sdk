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
 * \file video_sensor.c
 *
 * \brief  This file has the implementataion of Sensor Control API
 *
 *         Sensor APIs can be used to control external sensors.
 *         Drivers for sensors can be part of BSP or any other package.
 *
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <examples/tda2xx/include/video_sensor.h>
#include <fvid2/fvid2.h>
#include <vps/vps.h>

#include "video_sensor_priv.h"
#include <src/utils_common/include/utils_mem.h>

/*static UInt32 VidSensor_rgb2rgb_stabilize(
    VidSensor_AewbPrivParams *prm,
    UInt32 currIdx,
    UInt32 reset);*/
static Void VidSensor_InitAwbData(
    VidSensor_CreateParams *createPrms,
    UInt32 chanNum,
    Void *pParams);



/**
 *******************************************************************************
 *
 * \brief Set the default Create Params for OVI sensor params .
 *
 * \param  createParams   [IN] Create parameters for Sensor
 *
 *******************************************************************************
*/
Void VidSensor_CreateParams_Init(VidSensor_CreateParams *createParams)
{
    UInt32 i;

    createParams->sensorId      = VID_SENSOR_OV10635;
    createParams->vipInstId[0]   = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
    createParams->standard      = SYSTEM_STD_720P_60;
    createParams->dataformat    = SYSTEM_DF_YUV422I_UYVY;
    createParams->videoIfWidth  = SYSTEM_VIFW_8BIT;
    createParams->fps           = SYSTEM_FPS_30;
    createParams->isLVDSCaptMode = FALSE;
    createParams->numChan       = 1;
    createParams->videoIfMode   = SYSTEM_VIFM_SCH_DS_AVID_VSYNC;

    for (i = 0u; i < VIDEO_SENSOR_MAX_LVDS_CAMERAS; i ++)
    {
        createParams->privData[i] = NULL;
        createParams->sensorHandle[i] = NULL;
    }

    if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
    {
        createParams->sensorId      = VID_SENSOR_MULDES_AR0132RCCC;
        createParams->vipInstId[0]   = SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
        createParams->vipInstId[1]   = SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
        createParams->standard      = SYSTEM_STD_720P_60;
        createParams->dataformat    = SYSTEM_DF_YUV422I_UYVY;
        createParams->videoIfWidth  = SYSTEM_VIFW_16BIT;
        createParams->fps           = SYSTEM_FPS_60;
        createParams->isLVDSCaptMode = FALSE;
        createParams->numChan       = 2;
    }
}

/**
 *******************************************************************************
 *
 * \brief Create function to create video sensor.
 *
 *        Creates the sensor handle using bsp function calls.
 *
 * \param  createParams   [IN] Create parameters for Sensor
 * \param  sensorVariant  [IN] Indicate variant of a given sensor
 * \param  createStatus   [OUT] Status
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 VidSensor_create(VidSensor_CreateParams *createParams,
                        VidSensorVar_Id sensorVariant,
                        VidSensor_CreateStatus *createStatus)
{
    Int32  retVal = SYSTEM_LINK_STATUS_EFAIL,chanNum;
    UInt32 sensorInstId, sensorI2cInstId, sensorI2cAddr, sensorDrvId;
    Bsp_VidSensorChipIdParams sensorChipIdPrms;
    Bsp_VidSensorChipIdStatus sensorChipIdStatus;
    Bsp_VidSensorCreateStatus sensorCreateStatus;
    Bsp_VidSensorCreateParams sensorCreateParams;
    Bsp_BoardMode   boardMode;
    BspUtils_Ub960Status ub960Status;
    BspUtils_Ub960SourceI2cAddr ub960I2cAddr;

    createStatus->retVal = SYSTEM_LINK_STATUS_EFAIL;
    sensorDrvId = VID_SENSOR_MAX;
    if(createParams->sensorId==VID_SENSOR_OV10635
        ||
        createParams->sensorId==VID_SENSOR_OV10630
        )
    {
        sensorDrvId = FVID2_VID_SENSOR_OV1063X_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_MT9M024)
    {
        sensorDrvId = BSP_VID_SENSOR_MT9M024;
    }
    else if(createParams->sensorId==VID_SENSOR_MULDES_OV1063X)
    {
        sensorDrvId = FVID2_VID_SENSOR_MULDES_OV1063X_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_MULDES_AR0132RCCC)
    {
        sensorDrvId = FVID2_VID_SENSOR_APT_AR0132RCCC_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_OV10640)
    {
        if(createParams->videoIfMode == SYSTEM_VIFM_SCH_CSI2)
            sensorDrvId = FVID2_VID_SENSOR_OV10640_CSI2_DRV;
        else
            sensorDrvId = FVID2_VID_SENSOR_OV10640_CPI_DRV;
    }
    else if ((createParams->sensorId==VID_SENSOR_AR0132_BAYER) ||
             (createParams->sensorId==VID_SENSOR_AR0132_MONOCHROME))
    {
        sensorDrvId = FVID2_VID_SENSOR_APT_AR0132_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_AR0140_BAYER)
    {
        sensorDrvId = FVID2_VID_SENSOR_APT_AR0140_DRV;
        /* Using AR0140 Sensor ID TIDA for even single pass flow to
           get the different DCC profile. */
        if (sensorVariant==VID_SENSOR_VAR_AR0140_BAYER_TIDA00262)
        {
            sensorDrvId = FVID2_VID_SENSOR_TIDA00262_APT_AR0140_DRV;
        }

        if (sensorVariant==VID_SENSOR_VAR_AR0140_BAYER_SINGLE_PASS)
        {
            sensorDrvId = FVID2_VID_SENSOR_APT_AR0140_ONEPASSWDR_DRV;
        }
    }
    else if(createParams->sensorId==VID_SENSOR_IMX224_CSI2)
    {
        sensorDrvId = FVID2_VID_SENSOR_SONY_IMX224_CSI2_DRV;
    }
    else
    {
        /* unsupported sensor */
        UTILS_assert(0);
    }


    if (sensorVariant==VID_SENSOR_VAR_AR0140_BAYER_TIDA00262)
    {
        ub960I2cAddr.slaveAddr = UB960_SLAVE_ADDR;
        ub960I2cAddr.numSource = createParams->numChan;
        for(chanNum = 0 ; chanNum < createParams->numChan; chanNum++)
        {
            ub960I2cAddr.rSlave1Addr[chanNum] =
                            BspUtils_getSerAddrTida00262(chanNum);
            ub960I2cAddr.rSlave2Addr[chanNum] =
                            Bsp_boardGetVideoDeviceI2cAddr(
                                sensorDrvId,
                                FVID2_VPS_CAPT_VID_DRV,
                                createParams->vipInstId[chanNum] + chanNum);
        }
        retVal = BspUtils_appInitUb960(0U, &ub960I2cAddr, &ub960Status);
    }

    for(chanNum = 0 ; chanNum < createParams->numChan ;chanNum++)
    {
        sensorInstId = Bsp_boardGetVideoDeviceInstId(
                sensorDrvId,
                FVID2_VPS_CAPT_VID_DRV,
                createParams->vipInstId[chanNum]);
        sensorI2cInstId = Bsp_boardGetVideoDeviceI2cInstId(
                sensorDrvId,
                FVID2_VPS_CAPT_VID_DRV,
                createParams->vipInstId[chanNum]);
        if (sensorDrvId == FVID2_VID_SENSOR_TIDA00262_APT_AR0140_DRV)
        {
            sensorI2cAddr = Bsp_boardGetVideoDeviceI2cAddr(
                    sensorDrvId,
                    FVID2_VPS_CAPT_VID_DRV,
                    createParams->vipInstId[chanNum] + chanNum);
        }
        else
        {
            sensorI2cAddr = Bsp_boardGetVideoDeviceI2cAddr(
                    sensorDrvId,
                    FVID2_VPS_CAPT_VID_DRV,
                    createParams->vipInstId[chanNum]);
        }

        /* set capture port pinmux based on video interface bus width */
        if(createParams->videoIfWidth==SYSTEM_VIFW_8BIT)
        {
            boardMode = BSP_BOARD_MODE_VIDEO_8BIT;
        }
        else
        if(createParams->videoIfWidth==SYSTEM_VIFW_10BIT)
        {
            boardMode = BSP_BOARD_MODE_VIDEO_10BIT;
        }
        else
        if(createParams->videoIfWidth==SYSTEM_VIFW_12BIT)
        {
            boardMode = BSP_BOARD_MODE_VIDEO_12BIT;
        }
        else
        if(createParams->videoIfWidth==SYSTEM_VIFW_14BIT)
        {
            boardMode = BSP_BOARD_MODE_VIDEO_14BIT;
        }
        else
        if(createParams->videoIfWidth==SYSTEM_VIFW_16BIT)
        {
            boardMode = BSP_BOARD_MODE_VIDEO_16BIT;
        }
        else
        if(createParams->videoIfWidth==SYSTEM_VIFW_24BIT)
        {
            boardMode = BSP_BOARD_MODE_VIDEO_24BIT;
        }
        else
        {
            /* assume 8-bit mode if no match found */
            boardMode = BSP_BOARD_MODE_VIDEO_8BIT;
        }

        if( createParams->vipInstId[chanNum] == VPS_CAPT_INST_ISS_CAL_A
            &&
            createParams->videoIfMode == SYSTEM_VIFM_SCH_CSI2
            )
        {
            /* No need to set pin mux for CSI2 interface */
        }
        else
        {
            retVal = Bsp_boardSetPinMux(FVID2_VPS_CAPT_VID_DRV,
                                    createParams->vipInstId[chanNum],
                                    boardMode);
            UTILS_assert (retVal == 0);
        }

         /* Power on video sensor at board level mux */
        retVal = Bsp_boardPowerOnDevice(sensorDrvId, sensorInstId, TRUE);
        UTILS_assert (retVal == 0);

        if (SYSTEM_LINK_STATUS_SOK == retVal)
        {
            /* select video sensor at board level mux */
            retVal = Bsp_boardSelectDevice(sensorDrvId, sensorInstId);
            if (SYSTEM_LINK_STATUS_SOK != retVal)
            {
                Vps_printf(" VIDEO_SENSOR: Device select failed !!!\n");
            }
        }

        if( createParams->vipInstId[chanNum] == VPS_CAPT_INST_ISS_CAL_A
            &&
            createParams->videoIfMode == SYSTEM_VIFM_SCH_CSI2
            )
        {
            /* No need to select sensor for CSI2 mode */
        }
        else
        {
            retVal = Bsp_boardSelectMode(
                             sensorDrvId,
                             sensorInstId,
                             boardMode);
            if (SYSTEM_LINK_STATUS_SOK != retVal)
            {
                Vps_printf(" VIDEO_SENSOR: Board select failed !!!\n");
            }
        }

        Vps_printf(" VIDEO_SENSOR: INST%d : I2C%d : I2C Addr = 0x%x\n",
                sensorInstId, sensorI2cInstId, sensorI2cAddr);

        if (SYSTEM_LINK_STATUS_SOK == retVal)
        {
            if (((BSP_BOARD_MULTIDES == Bsp_boardGetId()) || (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())) &&
                (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == sensorDrvId))
            {
                retVal = BspUtils_appConfSerDeSer(sensorDrvId, sensorInstId);
                if (retVal != SYSTEM_LINK_STATUS_SOK)
                {
                    Vps_printf(" VIDEO_SENSOR: MULTI_DES: Configuring instance %d failed !!!\n",sensorInstId);
                }
            }
        }
        sensorCreateParams.deviceI2cInstId    = sensorI2cInstId;
        sensorCreateParams.numDevicesAtPort   = 1u;
        sensorCreateParams.deviceI2cAddr[0]   = sensorI2cAddr;
        sensorCreateParams.deviceResetGpio[0] = BSP_VID_SENSOR_GPIO_NONE;
        sensorCreateParams.sensorCfg          = NULL;
        sensorCreateParams.numSensorCfg       = 0U;

        createParams->sensorHandle[chanNum] = Fvid2_create(
                                                        sensorDrvId,
                                                        sensorInstId,
                                                        &sensorCreateParams,
                                                        &sensorCreateStatus,
                                                        NULL);
        if (createParams->sensorHandle[chanNum] == NULL)
        {
            if (chanNum != 4)
            {
                createStatus->retVal = SYSTEM_LINK_STATUS_EFAIL;
            }
            else
            {
                /* Continue the use case if the front camera sensor is not connected. */
                createStatus->retVal = SYSTEM_LINK_STATUS_SOK;
                Vps_printf(
                " VIDEO_SENSOR: WARNING: Front Camera Sensor is NOT Connected !!! \n");
                /* Avoid sending any more command to front camera sensor. */
                createParams->numChan -= 1;
            }
        }
        else
        {
            /* Get the Features supported by Sensor */
            retVal = Fvid2_control(createParams->sensorHandle[chanNum],
                                   IOCTL_BSP_VID_SENSOR_GET_FEATURES,
                                   &createParams->sensorFeatures[chanNum],
                                   NULL);
            UTILS_assert (retVal == 0);

            if(Bsp_platformIsTda3xxFamilyBuild())
            {
                if(sensorDrvId==FVID2_VID_SENSOR_OV1063X_DRV)
                {
                    Bsp_VidSensorFlipParams flipParams;

                    flipParams.hFlip = TRUE;
                    flipParams.vFlip = TRUE;

                    retVal = Fvid2_control( createParams->sensorHandle[chanNum],
                                        IOCTL_BSP_VID_SENSOR_SET_FLIP_PARAMS,
                                        &flipParams,
                                        NULL);
                    UTILS_assert (retVal == 0);

                    Vps_printf(
                        " VIDEO_SENSOR: Flipping sensor output in H and V direction\n");
                }
            }
            sensorChipIdPrms.deviceNum = 0;
            retVal = Fvid2_control( createParams->sensorHandle[chanNum],
                                    IOCTL_BSP_VID_SENSOR_GET_CHIP_ID,
                                    &sensorChipIdPrms,
                                    &sensorChipIdStatus);
            UTILS_assert (retVal == 0);

            Vps_printf(
                " VIDEO_SENSOR: VIP %d: DRV ID %04x (I2C ADDR 0x%02x): %04x:%04x:%04x\n",
                    createParams->vipInstId[chanNum],
                    sensorDrvId,
                    sensorCreateParams.deviceI2cAddr[0],
                    sensorChipIdStatus.chipId,
                    sensorChipIdStatus.chipRevision,
                    sensorChipIdStatus.firmwareVersion);

            /* For OV10640 CSI2, enable horizontal flip and for
               ov10640 parallel, enable vertical flip */
            if (TRUE == createParams->sensorFeatures[chanNum].isFlipSupported)
            {
                Bsp_VidSensorFlipParams flipPrms = {0};

                if (FVID2_VID_SENSOR_OV10640_CSI2_DRV == sensorDrvId)
                {
                    flipPrms.hFlip = TRUE;
                    flipPrms.vFlip = FALSE;
                }
                if (FVID2_VID_SENSOR_OV10640_CPI_DRV == sensorDrvId)
                {
                    flipPrms.hFlip = FALSE;
                    flipPrms.vFlip = TRUE;
                }
                retVal =
                    Fvid2_control(createParams->sensorHandle[chanNum],
                                  IOCTL_BSP_VID_SENSOR_SET_FLIP_PARAMS,
                                  &flipPrms,
                                  NULL);
                UTILS_assert (retVal == 0);
            }

            if(TRUE == createParams->sensorFeatures[chanNum].isSetCfgSupported)
            {
                Bsp_VidSensorConfigParams configParams;
                configParams.videoIfWidth = createParams->videoIfWidth;
                configParams.dataformat   = createParams->dataformat;
                configParams.standard     = createParams->standard;
                configParams.fps          = createParams->fps;
                retVal =
                    Fvid2_control(createParams->sensorHandle[chanNum], IOCTL_BSP_VID_SENSOR_SET_CONFIG,
                                &configParams,
                                NULL);
                UTILS_assert (retVal == 0);

            }

            if(TRUE == createParams->sensorFeatures[chanNum].isDccCfgSupported)
            {
                retVal =
                    Fvid2_control(
                        createParams->sensorHandle[chanNum],
                        IOCTL_BSP_VID_SENSOR_GET_DCC_PARAMS,
                        &createParams->dccPrms[chanNum],
                        NULL);
                UTILS_assert (retVal == 0);

            }

            createStatus->retVal = retVal;

            /* Allocated some private data structure for each sensor */
            createParams->privData[chanNum] = Utils_memAlloc(
                UTILS_HEAPID_DDR_CACHED_LOCAL,
                sizeof(VidSensor_AewbPrivParams),
                2);
            UTILS_assert (NULL != createParams->privData[chanNum]);

            VidSensor_InitAwbData(
                createParams,
                chanNum,
                createParams->privData[chanNum]);
        }
    }
    return (createStatus->retVal);
}
/*******************************************************************************
 *
 * \brief Delete function to delete video sensor.
 *
 *        Deletes the sensor handle using Fvid2_delete function calls.
 *
 * \param  handle         [IN] Handle to delete the sensor
 *
 * \param  deleteArgs      Not used.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 VidSensor_delete(VidSensor_CreateParams *createParams,
                        VidSensorVar_Id sensorVariant,
                        Ptr deleteArgs)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Int32 chanNum, sensorInstId, sensorDrvId;

    sensorDrvId = VID_SENSOR_MAX;
    if(createParams->sensorId==VID_SENSOR_OV10635
        ||
        createParams->sensorId==VID_SENSOR_OV10630
        )
    {
        sensorDrvId = FVID2_VID_SENSOR_OV1063X_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_MT9M024)
    {
        sensorDrvId = BSP_VID_SENSOR_MT9M024;
    }
    else if(createParams->sensorId==VID_SENSOR_MULDES_OV1063X)
    {
        sensorDrvId = FVID2_VID_SENSOR_MULDES_OV1063X_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_IMX224_CSI2)
    {
        sensorDrvId = FVID2_VID_SENSOR_SONY_IMX224_CSI2_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_OV10640)
    {
        if(createParams->videoIfMode == SYSTEM_VIFM_SCH_CSI2)
            sensorDrvId = FVID2_VID_SENSOR_OV10640_CSI2_DRV;
        else
            sensorDrvId = FVID2_VID_SENSOR_OV10640_CPI_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_MULDES_AR0132RCCC)
    {
        sensorDrvId = FVID2_VID_SENSOR_APT_AR0132RCCC_DRV;
    }
    else if((createParams->sensorId==VID_SENSOR_AR0132_BAYER) ||
            (createParams->sensorId==VID_SENSOR_AR0132_MONOCHROME))
    {
        sensorDrvId = FVID2_VID_SENSOR_APT_AR0132_DRV;
    }
    else if(createParams->sensorId==VID_SENSOR_AR0140_BAYER)
    {
        sensorDrvId = FVID2_VID_SENSOR_APT_AR0140_DRV;
        if (sensorVariant==VID_SENSOR_VAR_AR0140_BAYER_TIDA00262)
        {
            sensorDrvId = FVID2_VID_SENSOR_TIDA00262_APT_AR0140_DRV;
            status = BspUtils_appDeInitUb960(0U);
        }
    }
    else if(createParams->sensorId==VID_SENSOR_IMX224_CSI2)
    {
        sensorDrvId = FVID2_VID_SENSOR_SONY_IMX224_CSI2_DRV;
    }
    else
    {
        /* For Misra C */
    }
    UTILS_assert(sensorDrvId != VID_SENSOR_MAX);

    for(chanNum = 0; chanNum < createParams->numChan; chanNum++)
    {
        sensorInstId = Bsp_boardGetVideoDeviceInstId(
                        sensorDrvId,
                        FVID2_VPS_CAPT_VID_DRV,
                        createParams->vipInstId[chanNum]);

        if(createParams->sensorHandle[chanNum] != NULL)
        {
            status = Fvid2_delete(
                        (Fvid2_Handle)createParams->sensorHandle[chanNum],
                        NULL);

            /* Free up the memory allocated for the AWB private data */
            UTILS_assert(NULL != createParams->privData[chanNum]);
            status = Utils_memFree(
                UTILS_HEAPID_DDR_CACHED_LOCAL,
                createParams->privData[chanNum],
                sizeof(VidSensor_AewbPrivParams));

            UTILS_assert(0 == status);
        }

        if (((BSP_BOARD_MULTIDES == Bsp_boardGetId()) || (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())) &&
            (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == sensorDrvId))
        {
            status   =  BspUtils_appDeConfSerDeSer(sensorDrvId, sensorInstId);
        }
    }

    return status;
}

/*******************************************************************************
 *
 * \brief Control function to start stop and reset video sensor.
 *
 *        Control the sensor operation liske start and stop of the sensor using
 *        Fvid2 calls.
 *        Sensor reset is performed using IOCTL call IOCTL_BSP_VID_SENSOR_RESET
 *
 * \param  handle        [IN] Handle to control the sensor.
 *
 * \param  cmd           [IN] Control command for sensor
 *
 * \param  cmdArgs       [IN] Arguments for command if any.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 VidSensor_control(VidSensor_CreateParams *createParams,
                        UInt32 cmd,
                        Ptr    cmdArgs,
                        UInt32 cmdStatusArgs)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Int32 chanNum;

    if(cmd==VID_SENSOR_CMD_START)
    {
        for(chanNum = 0; chanNum < createParams->numChan; chanNum++)
        {
            status = Fvid2_start(
                            (Fvid2_Handle)createParams->sensorHandle[chanNum],
                             NULL);
        }
    }
    else if(cmd==VID_SENSOR_CMD_STOP)
    {
        for(chanNum = 0; chanNum < createParams->numChan; chanNum++)
        {
            status = Fvid2_stop(
                        (Fvid2_Handle)createParams->sensorHandle[chanNum],
                         NULL);
        }
    }
    else if(cmd==VID_SENSOR_CMD_RESET)
    {
        for(chanNum = 0; chanNum < createParams->numChan; chanNum++)
        {
            status = Fvid2_control(
                        (Fvid2_Handle)createParams->sensorHandle[chanNum],
                        IOCTL_BSP_VID_SENSOR_RESET,
                        NULL, NULL);
        }
    }
    else if (cmd == VID_SENSOR_CMD_SET_WDR_MODE)
    {
        Bsp_VidSensorWdrParams wdrPrms;
        UInt32 issWdrMode;

        if (NULL == cmdArgs)
        {
            return -1;
        }

        issWdrMode = *((UInt32 *)cmdArgs);
        /* Since Chains_issWdrmode is not accessible here,
           comparing wdrmode with 0.
           The value of CHAINS_ISS_WDR_MODE_DISABLE is set to 0, so below
           code will work fine. */
        if (issWdrMode)
        {
            wdrPrms.wdrEnable = TRUE;
        }
        else
        {
            wdrPrms.wdrEnable = FALSE;
        }

        for(chanNum = 0; chanNum < createParams->numChan; chanNum++)
        {
            /* Call SET_WDR_PARAMS only if WDR mode is supported */
            if (TRUE == createParams->sensorFeatures[chanNum].isWdrModeSupported)
            {
                status =
                    Fvid2_control(
                        createParams->sensorHandle[chanNum],
                        IOCTL_BSP_VID_SENSOR_SET_WDR_PARAMS,
                        &wdrPrms,
                        NULL);
            }
        }
    }
    /* Command to get exposure ratio from sensor */
    else if (cmd == VID_SENSOR_CMD_GET_EXP_RATIO_PARAMS)
    {
        Bsp_VidSensorExpRatioParams expRatioPrms;
        IssAewbAlgOutParams *pAlgOut = (IssAewbAlgOutParams *)cmdArgs;

        expRatioPrms.exposureRatio = 0; /* Init for KW error */

        /* Typically used only for one channel */
        for(chanNum = 0; chanNum < createParams->numChan; chanNum++)
        {
            /* Get Exposure ration is used only for the WDR mode, so
               Call SET_WDR_PARAMS only if WDR mode is supported */
            if (TRUE == createParams->sensorFeatures[chanNum].isWdrModeSupported)
            {
                status =
                    Fvid2_control(
                        createParams->sensorHandle[chanNum],
                        IOCTL_BSP_VID_SENSOR_GET_EXP_RATIO_PARAMS,
                        &expRatioPrms,
                        NULL);
                UTILS_assert (status == 0);
            }
        }

        /* Typically used only for one channel */
        pAlgOut->exposureRatio = expRatioPrms.exposureRatio;
    }
    else if (VID_SENSOR_CMD_GET_DCC_INFO == cmd)
    {
        UInt32 chanNum;
        VidSensor_DccInfo *dccInfo = (VidSensor_DccInfo *)cmdArgs;
        Bsp_VidSensorDccParams *bspDccPrms = NULL;

        if ((NULL != dccInfo) && (dccInfo->chanNum < createParams->numChan))
        {
            chanNum = dccInfo->chanNum;
            dccInfo->isDccCfgSupported =
                createParams->sensorFeatures[chanNum].isDccCfgSupported;
            bspDccPrms = &createParams->dccPrms[chanNum];

            if (TRUE == dccInfo->isDccCfgSupported)
            {
                status =
                    Fvid2_control(
                        createParams->sensorHandle[chanNum],
                        IOCTL_BSP_VID_SENSOR_GET_DCC_PARAMS,
                        bspDccPrms,
                        NULL);
                UTILS_assert (status == 0);

                dccInfo->cameraId = bspDccPrms->dccCameraId;
                dccInfo->pDccCfg = bspDccPrms->pDccCfg;
                dccInfo->dccCfgSize = bspDccPrms->dccCfgSize;
            }
        }
    }
    else if (VID_SENSOR_CMD_READ_REG == cmd)
    {
        Bsp_VidSensorRegRdWrParams regRdWrPrms;
        VidSensor_RegRdWrParams *vidSensRegRdWrPrms =
            (VidSensor_RegRdWrParams *)cmdArgs;

        regRdWrPrms.deviceNum = 0;
        regRdWrPrms.numRegs = 1;
        regRdWrPrms.regAddr = &vidSensRegRdWrPrms->regAddr;
        regRdWrPrms.regValue16 = &vidSensRegRdWrPrms->regValue;

        status = FVID2_EINVALID_PARAMS;
        if (NULL != createParams->sensorHandle[vidSensRegRdWrPrms->chanNum])
        {
            status =
                Fvid2_control(
                    createParams->sensorHandle[vidSensRegRdWrPrms->chanNum],
                    IOCTL_BSP_VID_SENSOR_REG_READ,
                    &regRdWrPrms,
                    NULL);
            UTILS_assert (status == 0);
        }
    }
    else if (VID_SENSOR_CMD_WRITE_REG == cmd)
    {
        Bsp_VidSensorRegRdWrParams regRdWrPrms;
        VidSensor_RegRdWrParams *vidSensRegRdWrPrms =
            (VidSensor_RegRdWrParams *)cmdArgs;

        regRdWrPrms.deviceNum = 0;
        regRdWrPrms.numRegs = 1;
        regRdWrPrms.regAddr = &vidSensRegRdWrPrms->regAddr;
        regRdWrPrms.regValue16 = &vidSensRegRdWrPrms->regValue;

        status = FVID2_EINVALID_PARAMS;
        if (NULL != createParams->sensorHandle[vidSensRegRdWrPrms->chanNum])
        {
            status =
                Fvid2_control(
                    createParams->sensorHandle[vidSensRegRdWrPrms->chanNum],
                    IOCTL_BSP_VID_SENSOR_REG_WRITE,
                    &regRdWrPrms,
                    NULL);
            UTILS_assert (status == 0);
        }
    }


    return status;
}

Void VidSensor_UpdateAewbParams(
    VidSensor_CreateParams *createParams,
    IssAewbAlgOutParams *pAewbAlgOut)
{
    Int32 status;
    UInt32 chanNum;
    Bsp_VidSensorExposureParams expPrms;
    Bsp_VidSensorGainParams gainPrms;

    gainPrms.analogGain = pAewbAlgOut->outPrms[0].analogGain;
    expPrms.exposureTime = pAewbAlgOut->outPrms[0].exposureTime;
    chanNum = pAewbAlgOut->channelId;

    if (((VIDEO_SENSOR_MAX_LVDS_CAMERAS > chanNum) &&
         (NULL != createParams->sensorHandle[chanNum])) &&
        (createParams->numChan > chanNum))
    {
        if (TRUE ==
            createParams->sensorFeatures[chanNum].isManualAnalogGainSupported)
        {
            status = Fvid2_control(
                createParams->sensorHandle[chanNum],
                IOCTL_BSP_VID_SENSOR_SET_GAIN_PARAMS,
                &gainPrms,
                NULL);
            UTILS_assert (status == 0);
        }

        if (TRUE ==
            createParams->sensorFeatures[chanNum].isManualExposureSupported)
        {
            status = Fvid2_control(
                createParams->sensorHandle[chanNum],
                IOCTL_BSP_VID_SENSOR_SET_EXP_PARAMS,
                &expPrms,
                NULL);
            UTILS_assert (status == 0);
        }
    }
}

Void VidSensor_SetAewbParams(
    VidSensor_CreateParams *createParams,
    AlgorithmLink_IssAewbCreateParams *prms,
    UInt32 isOnePassWdr)
{
    UTILS_assert(NULL != createParams);
    UTILS_assert(NULL != prms);

    if ((createParams->sensorId == VID_SENSOR_AR0132_BAYER) ||
        (createParams->sensorId==VID_SENSOR_AR0132_MONOCHROME))
    {
        VidSensor_SetAewbParams_ar0132(prms);
    }
    if(createParams->sensorId == VID_SENSOR_AR0140_BAYER)
    {
        VidSensor_SetAewbParams_ar0140(prms, isOnePassWdr);
    }
    if(createParams->sensorId == VID_SENSOR_OV10640)
    {
        VidSensor_SetAewbParams_ov10640(prms);
    }
    if(createParams->sensorId == VID_SENSOR_IMX224_CSI2)
    {
        VidSensor_SetAewbParams_imx224(prms);
    }
}

Void VidSensor_SetIssIspConfig(
    VidSensor_CreateParams *createParams,
    IssIspConfigurationParameters *pIspConfig)
{
    UTILS_assert(NULL != pIspConfig);

    if ((createParams->sensorId == VID_SENSOR_AR0132_BAYER) ||
        (createParams->sensorId==VID_SENSOR_AR0132_MONOCHROME))
    {
        VidSensor_SetIssIspConfig_ar0132(pIspConfig);
    }
    if(createParams->sensorId == VID_SENSOR_AR0140_BAYER)
    {
        VidSensor_SetIssIspConfig_ar0140(pIspConfig);
    }
    if(createParams->sensorId == VID_SENSOR_OV10640)
    {
        VidSensor_SetIssIspConfig_ov10640(pIspConfig);
    }
    if(createParams->sensorId == VID_SENSOR_IMX224_CSI2)
    {
        VidSensor_SetIssIspConfig_imx224(pIspConfig);
    }
}

Void VidSensor_SetIssIspExtraConfig(
    VidSensor_CreateParams *createParams,
    IssIspConfigurationParameters *pIspConfig)
{
    UTILS_assert(NULL != pIspConfig);

    /* This will be called only for monochrome mode for AR0132 sensor,
        so use GLBCE config even for Bayer sensor */
    if ((createParams->sensorId==VID_SENSOR_AR0132_MONOCHROME) ||
        (createParams->sensorId==VID_SENSOR_AR0132_BAYER))
    {
        VidSensor_SetIssIspGlbceConfig_ar0132(pIspConfig);
    }
    if(createParams->sensorId == VID_SENSOR_AR0140_BAYER)
    {
        VidSensor_SetIssIspGlbceConfig_ar0140(pIspConfig);
    }
}

static Void VidSensor_InitAwbData(
    VidSensor_CreateParams *createPrms,
    UInt32 chanNum,
    Void *pParams)
{
    UInt32 cnt;

    VidSensor_AewbPrivParams *pAewbPrms = (VidSensor_AewbPrivParams *)pParams;

    pAewbPrms->steadySet = -1;
    pAewbPrms->curSetIdx = -1;
    pAewbPrms->curSetCnt = 0;

    pAewbPrms->awbCount = 1U;
    pAewbPrms->prevRgb2RgbIdx = 0U;

    for (cnt = 0U; cnt < VID_SENSOR_AWB_AVG_BUF_LENGTH; cnt ++)
    {
        pAewbPrms->historyIdx[cnt] = 0U;
    }
}
