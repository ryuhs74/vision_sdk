/*
 *******************************************************************************
 *
 * Copyright (C) 2015 CAMMSYS - http://www.cammsys.net
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */
/**
 *******************************************************************************
 *
 * \ingroup EXAMPLES_API
 * \defgroup EXAMPLES_Ypbpr_TX_API APIs for controlling Ypbpr transmitter
 *
 *         Ypbpr transmitter APIs can be used to control Ypbpr transmitter.
 *         Drivers for Ypbpr transmitter can be part of BSP or any other package.
 *
 * @{
 *
 *******************************************************************************
 */
 /**
 *******************************************************************************
 *
 * \file component.h
 *
 * \brief APIs for controlling component transmitter.
 *
 * \version 0.0 (Nov 2015) : [CM] First version *
 *
 *******************************************************************************
 */

#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#ifdef __cplusplus
extern "C" {
#endif

 /*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <fvid2/fvid2.h>
#include <vps/vps.h>
#include <devices/bsp_videoEncoder.h>
#include <devices/bsp_ch7026.h>
#include <boards/bsp_board.h>

 /*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Base command number
 *
 *******************************************************************************
*/
#define COMPONENT_CMD_BASE     (0x0)
/**
 *******************************************************************************
 *
 *   \brief CMD: To start the ypbpr transmitter
 *
 *           ypbpr transmitter will start giving data to
 *           VIP or ISS
 *
 *******************************************************************************
 */
 #define COMPONENT_CMD_START    (COMPONENT_CMD_BASE + 0x1)

/*******************************************************************************
 *
 *   \brief CMD: To stop the ypbpr transmitter
 *
 *           ypbpr transmitter will stop giving data to
 *           VIP or ISS
 *
 *
 *******************************************************************************
 */
#define COMPONENT_CMD_STOP     (COMPONENT_CMD_BASE + 0x2)

/*******************************************************************************
 *
 *   \brief CMD: Command max
 *
 *           There cannont be any sensor command after this.
 *
 *   \param None
 *
 *******************************************************************************
 */
#define COMPONENT_CMD_MAX      (COMPONENT_CMD_BASE + 0x3)


 /**
 *******************************************************************************
 *
 *  \brief FVID2 driver handle returned by individual drivers
 *
 *******************************************************************************
*/
typedef Ptr Component_Handle;
 /**
 *******************************************************************************
 *
 *  \brief  Enum for transmitter ID for ypbpr transmitters.
 *
 *  These are set of ypbpr transmitters currently supported on the EVM
 *
 *******************************************************************************
*/
typedef enum {
    COMPONENT_CH7026,
    /**< ypbpr transmitter CH7026 */
    YPBPRTX_MAX = 0xFFFFu,
    /**<Max */
    COMPONENT_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
}Component_Id;
 /*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */
 /**
 *******************************************************************************
 *
 *  \brief  Ypbpr Receiver Create params.
 *
 *        This structure is used to set input parameters to create Ypbpr transmitter.
 *        This structure is used as an argument to make certain BSP function
 *        calls like Bsp_boardGetVideoDeviceInstId to get the sensor device id.
 *        Bsp_boardGetVideoDeviceI2cInstId to get the i2c instance id.
 *        Bsp_boardGetVideoDeviceI2cAddr to get the i2c address.
 *
 *******************************************************************************
*/

typedef struct
{
    Component_Id                componentId;
    /**< ID of the Ypbpr transmitter for which create is getting called. */

    System_Standard          standard;
    /**< 720p or 1080p standard */

    Bsp_BoardMode            boardMode;
    /**< Board mode */

    Component_Handle            component_Handle;
    /**< Ypbpr transmitter handle  */
}Component_CreateParams;

 /**
 *******************************************************************************
 *
 *  \brief  Ypbpr transmitter Create return parameters.
 *
 *        This structure is used to set output status of Ypbpr create
 *          create function.
 *
 *******************************************************************************
*/
typedef struct
{
    Int32 retVal;
    /**< Return value of CreateApi */
}Component_CreateStatus;
 /*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Set the default Create Params for Ypbpr transmitter .
 *
 * \param  createParams   [IN] Create parameters for Ypbpr transmitter
 *
 *******************************************************************************
*/
Void Component_CreateParams_Init(Component_CreateParams *createParams);

/**
 *******************************************************************************
 *
 * \brief Create function to create Ypbpr transmitter.
 *
 *        Creates the Ypbpr transmitter handle using bsp function calls.
 *
 * \param  createParams   [IN] Create parameters for Ypbpr transmitter
 *
 * \param  createStatus   [OUT] Status
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Component_create(Component_CreateParams *createParams,
                      Component_CreateStatus *createStatus);



/*******************************************************************************
 *
 * \brief Delete function to delete Ypbpr transmitter.
 *
 *        Deletes the Ypbpr transmitter handle using Fvid2_delete function calls.
 *
 * \param  createParams    [IN] Create parameters for Ypbpr transmitter
 *
 * \param  deleteArgs      Not used.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Component_delete(Component_CreateParams *createParams, Ptr deleteArgs);




/*******************************************************************************
 *
 * \brief Control function to start stop and reset Ypbpr transmitter.
 *
 *        Control the transmitter operation like start and stop of the transmitter using
 *        Fvid2 calls.
 *        transmitter reset is performed using IOCTL call IOCTL_BSP_VID_DEC_RESET
 *
 * \param  handle        [IN] Handle to control Ypbpr transmitter.
 *
 * \param  cmd           [IN] Control command for Ypbpr transmitter.
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
                        UInt32 cmdStatusArgs);


#ifdef __cplusplus
}
#endif

#endif /* _COMPONENT_H_*/
