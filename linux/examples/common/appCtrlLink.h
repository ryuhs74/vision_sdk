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
 *  \ingroup LINK_API
 *  \defgroup APP_CTRL_LINK_API Application Control link
 *
 *  This module defines the control commands that are applicable to
 *  control application specific function when Linux on A15.
 *
 *  When Linux runs on A15, IPU1-0 still needs to control some board level
 *  devices like capture sensors
 *
 *  This link implemented on IPU1-0 exports APIs which application on A15
 *  can invoke to control these devices
 *
 *  This API is only valid when Linux runs on A15
 *
 *  This is control ONLY link, i.e is does not take any buffers as input
 *  or output.
 *
 *   @{
*/

/**
 *******************************************************************************
 *
 *  \file appCtrlLink.h
 *  \brief Application Control link
 *
 *******************************************************************************
*/

#ifndef _APP_CTRL_LINK_H_
#define _APP_CTRL_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <examples/tda2xx/include/chains_main_srv_calibration.h>

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Params for command APP_CTRL_LINK_CMD_BOARD_IS_MULTI_DES_CONNECTED
 *******************************************************************************
 */
typedef struct {

    UInt32 isConnected;
    /**< [OUT]
         TRUE: Multi-des board is connected,
         FALSE: Multi-des board is NOT connected
     */

} AppCtrlCmd_BoardIsMultiDesConnectedPrm;


/* Control Command's    */

/**
    \ingroup LINK_API_CMD
    \addtogroup APP_CTRL_LINK_API_CMD  Application Control link Control Commands

    @{
*/

/**
 *******************************************************************************
 * \brief Link CMD: Command to enable USB port for charging
 *
 * \param NONE
 *
 *******************************************************************************
*/
#define APP_CTRL_LINK_CMD_BOARD_ENABLE_USB_CHARGING         (0x1002)

/**
 *******************************************************************************
 * \brief Link CMD: Command to check if Multi-des board is connectoed or not
 *
 * \param  AppCtrlCmd_BoardIsMultiDesConnectedPrm *pPrm [OUT]
 *
 *******************************************************************************
*/
#define APP_CTRL_LINK_CMD_BOARD_IS_MULTI_DES_CONNECTED      (0x1003)

/**
 *******************************************************************************
 * \brief Link CMD: Command to set DMM priorities
 *
 *   \param  NONE
 *
 *******************************************************************************
*/
#define APP_CTRL_LINK_CMD_SET_DMM_PRIORITIES                (0x1004)

/**
 *******************************************************************************
 * \brief Link CMD: Command to perform surround view calibrtaion + QSPI storage
 *
 *   \param  NONE
 *
 *******************************************************************************
*/
#define APP_CTRL_LINK_CMD_SURROUNDVIEW_CALIBRATION          (0x1005)

/*@}*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
