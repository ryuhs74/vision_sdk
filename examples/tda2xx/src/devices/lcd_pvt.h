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
 * \ingroup EXAMPLES_LCD_API
 * \defgroup EXAMPLES_LCD_INTERAL_API Private APIs for controlling external LCD
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file lcd_pvt.h
 *
 * \brief Private APIs for controlling external LCD
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _LCD_PVT_H_
#define _LCD_PVT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/system_const_displayCtrl.h>
#include <examples/tda2xx/include/lcd.h>

#include <devices/bsp_device.h>
#include <devices/bsp_lcdController.h>
#include <fvid2/fvid2.h>
#include <vps/vps.h>
#include <boards/bsp_board.h>

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 *  \brief  Structure containing LCD create parameters.
 *
 *  This structure holds the lcd handle and create params .
 *
 *******************************************************************************
*/
typedef struct
{
    Bsp_LcdCtrlCreateParams     lcdCtrlCreatePrms;
    /** < LCD controller Create params. */

    Fvid2_Handle                lcdCtrlhdl;
    /** < LCD controller Driver handle. */

    Bsp_BoardMode            boardMode;
    /**< Board mode */

}Lcd_Obj;



#ifdef __cplusplus
}
#endif

#endif /* #ifndef _LCD_PRIV_H_ */

/* @} */
