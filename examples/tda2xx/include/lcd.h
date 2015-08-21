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
 *
 * \ingroup EXAMPLES_API
 * \defgroup EXAMPLES_LCD_API APIs for controlling external LCD
 *
 * \brief  LCD API to control external LCD
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file lcd.h
 *
 * \brief APIs for controlling external LCD
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _LCD_H_
#define _LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *
 * \brief LCD DEVICE INST ID: LCD Instance Id 0
 *
 *  Lcd instance id refers to the default LCD
 *
 *  \param None
 *
 *  \return None
 *
 *******************************************************************************
 */
#define LCD_DEVICE_INST_ID_0         0

/**
 *******************************************************************************
 *
 *
 * \brief LCD DEVICE INST ID: LCD Instance Id 1
 *
 *  Lcd instance id to select the other LCD display
 *
 *  \param None
 *
 *  \return None
 *
 *******************************************************************************
 */
#define LCD_DEVICE_INST_ID_1         1

/**
 *******************************************************************************
 *
 *
 * \brief LCD DEVICE INST ID: LCD Instance Id Max
 *
 *  Maximum number of LCD supported
 *
 *  \param None
 *
 *  \return None
 *
 *******************************************************************************
 */
#define LCD_DEVICE_INST_ID_MAX       2

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */
typedef enum
{
    LCD_CNTR_DRV,
    /**< LCD Controller driver ID */
    LCD_DRV_ID_MAX = 0xFFFFu,
    /**< Max */
    LCD_DRV_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
    *   This is to make sure enum size defaults to 32 bits always regardless
    *   of compiler.
    */
}Driver_Id;

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 *  \brief  Structure containing LCD create parameters.
 *
 *  This structure is used to set LCD parameters .
 *  This structure is set by the usecase file.
 *  Create params are inParams for Lcd_turnOn fnx.
 *
 *******************************************************************************
*/
typedef struct
{
    Driver_Id                   drvId;
    /**< Set the driver id */
    UInt32                      brightnessValue;
    /**< Brightness Value to be set for the LCD 0 - 100 */
}Lcd_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief This function initialize the LCD and turns on the LCD
 *
 * This will configure Venc with LCD timings
 * In this function LCDObject handle is created with a Fvid2Create and
          Controlled with Fvid2Control
          Lcd power on with             IOCTL IOCTL_BSP_LCDCTRL_POWER_ON .
          Lcd enable backlight          IOCTL_BSP_LCDCTRL_ENABLE_BACKLIGHT
          Lcd select sync mode          IOCTL_BSP_LCDCTRL_SELECT_MODE_DE_OR_SYNC
          and control brightness        IOCTL_BSP_LCDCTRL_CONTROL_BRIGHTNESS
 *
 * \param  lcdInstId  [IN] Lcd id to be turned on
 *
 * \param  createPrm  [IN]Lcd_CreateParams
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Lcd_turnOn(UInt32 lcdInstId, Lcd_CreateParams *createPrm);

/**
 *******************************************************************************
 *
 * \brief This function deInitialize the LCD and turns off the LCD.
 *
 * In this function LCDObject handle is deleted with a Fvid2_delete and
          Controlled with Fvid2_control
          Lcd disable backlight             IOCTL_BSP_LCDCTRL_DISABLE_BACKLIGHT
          Lcd poweroff                      IOCTL_BSP_LCDCTRL_POWER_OFF
 *
 * \param  lcdInstId     [IN] LCD_Device_Instance_ID
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Lcd_turnOff(UInt32 lcdInstId);

/**
 *******************************************************************************
 *
 * \brief This function initializes Max number of LCD Objects.
 *
 * In this function LCDObject handle is set to NULL
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Lcd_init();

/**
 *******************************************************************************
 *
 * \brief This function deinitializes Maximum number of LCD Objects created.
 *
 * In this function LCDObject handle is set to NULL
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Lcd_deInit();


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _LCD_H_ */

/* @} */
