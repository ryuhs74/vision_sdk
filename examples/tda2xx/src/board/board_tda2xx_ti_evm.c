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
 * \file board_tda2xx_ti_evm.c
 *
 * \brief  This module has the interface for Board Initialization
 *
 *         In this function BspBoardInit functions are called.
 *         Bsp_board,I2C,Fvid2,BSP_device init and deinit functions are called
 *         This should be called first call after initializing the uart driver.
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

#include <include/link_api/system.h>
#include <examples/tda2xx/include/board.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/heaps/HeapMem.h>


#include <fvid2/fvid2.h>
#include <platforms/bsp_platform.h>
#include <devices/bsp_device.h>
#include <common/bsp_types.h>
#include <common/bsp_config.h>
#include <common/bsp_utils.h>
#include <common/bsp_common.h>
#include <boards/bsp_board.h>
#include <devices/bsp_videoSensor.h>
#include <i2c/bsp_i2c.h>
#include <bsputils_lvds.h>
#include <vps/vps.h>

/**
 *******************************************************************************
 *
 * \brief This function probes the MultiDes Board if connected or no
 *
 *        In this function
 *
 * \return  TRUE on success
 *
 *******************************************************************************
 */
Bool Board_isMultiDesConnected()
{
    int32_t boardId;
    Bool status = FALSE;

    if(!System_isFastBootEnabled())
    {
        boardId = Bsp_boardGetId();
        if ((BSP_BOARD_MULTIDES == boardId) || (BSP_BOARD_MONSTERCAM == boardId))
        {
            status = TRUE;
        }
    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function initialize the board related modules .
 *
 *        In this function Bsp_board, I2C, Fvid2 and BSP_device are initialized
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Board_init()
{
    uint32_t nIsI2cInitReq = TRUE;
    int32_t nStatus = SYSTEM_LINK_STATUS_SOK;
    Bsp_BoardInitParams boardInitPrms; /* Initialized using BSP APIs */
    const Bsp_BoardI2cData *pI2cData = NULL;
    uint32_t nInstCnt = 0u;

    Bsp_BoardI2cInstData *pI2cInstData = NULL;
     /* Initialized before usage */
    lld_hsi2c_initParam_t   i2cInitParams[BSP_DEVICE_I2C_INST_ID_MAX];
    I2c_DevInitParams       i2cDevInitParams[BSP_DEVICE_I2C_INST_ID_MAX];
    /* Initialized using BSP APIs */
    Bsp_DeviceInitParams deviceInitPrms;
//    Bsp_PlatformId platformId = BSP_PLATFORM_ID_UNKNOWN;

    do
    {

        BspBoardInitParams_init(&boardInitPrms);

        Vps_printf(" BOARD: Board Init in progress !!!\n");

        nStatus = Bsp_boardInit(&boardInitPrms);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" BOARD: Board Init Failed !!!\n");
            break;
        }

        Vps_printf(" BOARD: Board Init Done !!!\n");

#if 0	///craven@150907
        /* Override I2C init for non-EVM builds */
        platformId = Bsp_platformGetId();


        if (platformId != BSP_PLATFORM_ID_EVM)
        {
            nIsI2cInitReq = (uint32_t) FALSE;
        }
#endif
        if (nIsI2cInitReq == (uint32_t) TRUE)
        {
            pI2cData = Bsp_boardGetI2cData();

            if (NULL == pI2cData)
            {
                Vps_printf(" BOARD: Bsp_boardGetI2cData() failed !!!\n");
                nStatus = SYSTEM_LINK_STATUS_EFAIL;
                break;
            }

            if (pI2cData->numInst > BSP_DEVICE_I2C_INST_ID_MAX)
            {
                Vps_printf(" BOARD: ERROR: I2C num instances exceeded "
                           "BSP_DEVICE_I2C_INST_ID_MAX !!!\n");
                nStatus = SYSTEM_LINK_STATUS_EFAIL;
                break;
            }

            if (NULL == pI2cData->instData)
            {
                Vps_printf(" BOARD: ERROR: I2C instance data is NULL  !!!\n");
                nStatus = SYSTEM_LINK_STATUS_EFAIL;
                break;
            }

            for (nInstCnt = 0u; nInstCnt < pI2cData->numInst; nInstCnt++)
            {
                pI2cInstData = &pI2cData->instData[nInstCnt];
                if (pI2cInstData->instId > BSP_DEVICE_I2C_INST_ID_MAX)
                {
                    Vps_printf(" BOARD: ERROR: I2C num instances exceeded "
                               "BSP_DEVICE_I2C_INST_ID_MAX !!!\n");
                    nStatus = SYSTEM_LINK_STATUS_EFAIL;
                    break;
                }

                i2cInitParams[nInstCnt].opMode       = HSI2C_OPMODE_INTERRUPT;
                i2cInitParams[nInstCnt].isMasterMode = (uint32_t) TRUE;
                i2cInitParams[nInstCnt].is10BitAddr = (uint32_t) FALSE;
                i2cInitParams[nInstCnt].i2cBusFreq   =
                    (lld_i2c_busspeed) pI2cInstData->busClkKHz;
                i2cInitParams[nInstCnt].i2cIntNum     = pI2cInstData->intNum;
                i2cInitParams[nInstCnt].i2cOwnAddr    = 0xCC;
                i2cDevInitParams[nInstCnt].initParams = &i2cInitParams[nInstCnt];
                i2cDevInitParams[nInstCnt].hsi2c_sem  = BspOsal_semCreate(1, TRUE);
                i2cDevInitParams[nInstCnt].instId     = pI2cInstData->instId;
            }

            if (pI2cData->numInst > 0u)
            {
                nStatus = I2c_GlobalInit(pI2cData->numInst,
                    &i2cDevInitParams[0u]);
                if (SYSTEM_LINK_STATUS_SOK != nStatus)
                {
                    Vps_printf(" BOARD: ERROR: I2C Init failed!!!\n");
                    break;
                }
            }

            BspDeviceInitParams_init(&deviceInitPrms);

            /*
             * WorkAround:
             * Probing disabled for the timebeing. This is done as there will
             * be a I2C address conflict between the SII9127 present in the
             * Vayu VISION board and one of the deserializer chip.
             */
            nStatus = Bsp_deviceInit(&deviceInitPrms);

            if (SYSTEM_LINK_STATUS_SOK != nStatus)
            {
                Vps_printf(" BOARD: Device Init failed!!!\n");
                break;
            }

            if (Board_isMultiDesConnected())
            {
                nStatus = BspUtils_appInitSerDeSer(FVID2_VID_SENSOR_MULDES_OV1063X_DRV);
                if (nStatus != SYSTEM_LINK_STATUS_SOK)
                {
                    Vps_printf(" MultiDes Board Init failed!!!\n");
                }
                else
                {
                    Vps_printf(" MultiDes Board Init Done");
                }
            }

        }

        if(!System_isFastBootEnabled())
        {
            /* Print FVID2 and BSP version string and platform info */
           Vps_printf("\r\n");
           Vps_printf(" FVID2 Version         : [%s]\r\n",
           Fvid2_getVersionString());
           Vps_printf(" BSP Version           : [%s]\r\n",
           Bsp_getVersionString());
           Vps_printf(" Platform              : [%s]\r\n",
           Bsp_platformGetString());
           Vps_printf(" SOC                   : [%s]\r\n",
           Bsp_platformGetSocString());
           Vps_printf(" SOC Revision          : [%s]\r\n",
           Bsp_platformGetCpuRevString());
           Vps_printf(" Board Detected        : [%s]\r\n",
           Bsp_boardGetBoardString());
        #ifdef A15_TARGET_OS_BIOS
           Vps_printf(" Base Board Revision   : [%s]\r\n",
           Bsp_boardGetBaseBoardRevString());
           Vps_printf(" Daughter Card Revision: [%s]\r\n",
           Bsp_boardGetDcRevString());
        #endif
        }
        Vps_printf(" \r\n");
    } while (0);

    return nStatus;
}

/**
 *******************************************************************************
 *
 * \brief This function De-initialize the previously initialized modules .
 *
 *        In this function
 *        Bsp_board
 *        I2C
 *        BSP_device are deinitialized
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Board_deInit()
{
    uint32_t nIsI2cDeinitReq = TRUE;
    int32_t nStatus = SYSTEM_LINK_STATUS_SOK;
    const Bsp_BoardI2cData *pI2cData = NULL;
    Bsp_PlatformId platformId = BSP_PLATFORM_ID_UNKNOWN;

    do
    {
        /* Override I2C de-init for non-EVM builds */
        platformId = Bsp_platformGetId();
        if (platformId != BSP_PLATFORM_ID_EVM)
        {
            nIsI2cDeinitReq = (uint32_t) FALSE;
        }

        if (nIsI2cDeinitReq == (uint32_t) TRUE)
        {
            pI2cData = Bsp_boardGetI2cData();
            if (NULL == pI2cData)
            {
                Vps_printf(" BOARD: Bsp_boardGetI2cData() failed !!!\n");
                nStatus = SYSTEM_LINK_STATUS_EFAIL;
                break;
            }

            if (pI2cData->numInst > BSP_DEVICE_I2C_INST_ID_MAX)
            {
                Vps_printf(" BOARD: ERROR: I2C num instances exceeded "
                           "BSP_DEVICE_I2C_INST_ID_MAX !!!\n");
                nStatus = SYSTEM_LINK_STATUS_EFAIL;
                break;
            }

            nStatus = Bsp_deviceDeInit(NULL);
            if (SYSTEM_LINK_STATUS_SOK != nStatus)
            {
                Vps_printf(" BOARD: Device De-init failed !!!\n");
                break;
            }

            if (pI2cData->numInst > 0u)
            {
                nStatus = I2c_GlobalDeInit(NULL);
                if (SYSTEM_LINK_STATUS_SOK != nStatus)
                {
                    Vps_printf(" BOARD: I2C De-init failed !!!\n");
                    break;
                }
            }
        }

        if (Board_isMultiDesConnected())
        {
            BspUtils_appDeInitSerDeSer(FVID2_VID_SENSOR_MULDES_OV1063X_DRV);
        }

        nStatus = Bsp_boardDeInit(NULL);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" BOARD: Board De-init failed !!!\n");
            break;
        }
    } while (0);

    return nStatus;
}

Void Board_enableUsbCharging()
{
    if(Bsp_platformIsTda2xxFamilyBuild())
    {
        volatile UInt32 temp;

        // Enable USB2 Charging
        // Set USB2_DRVVBUS Pin to GPIO6_13
        *((volatile UInt32 *) 0x4A003684) = (UInt32) 0x0004000E;
        // Set GPIO6_13 pin to HIGH
        temp = *((volatile UInt32 *) 0x4805D134);
        *((volatile UInt32 *) 0x4805D134) = temp & (UInt32) 0xFFFFDFFF;
        *((volatile UInt32 *) 0x4805D194) = (UInt32) 0x00002000;
    }
    else
    {
        Vps_printf(" BOARD: API is NOT applicable for this platform !!!\n");
    }
}
