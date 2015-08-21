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
 * \defgroup EXAMPLES_HDMI_TX_API APIs for controlling hdmi transmitter
 *
 *         hdmi transmitter APIs can be used to control hdmi transmitter.
 *         Drivers for hdmi transmitter can be part of BSP or any other package.
 *
 * @{
 *
 *******************************************************************************
 */
 /**
 *******************************************************************************
 *
 * \file hdmi_tx.h
 *
 * \brief APIs for controlling hdmi transmitter.
 *
 * \version 0.0 (Nov 2013) : [CM] First version *
 *
 *******************************************************************************
 */

#ifndef _HDMI_TX_H_
#define _HDMI_TX_H_

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
#include <devices/bsp_sii9022a.h>
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
#define HDMI_TX_CMD_BASE     (0x0)
/**
 *******************************************************************************
 *
 *   \brief CMD: To start the hdmi transmitter
 *
 *           hdmi transmitter will start giving data to
 *           VIP or ISS
 *
 *******************************************************************************
 */
 #define HDMI_TX_CMD_START    (HDMI_TX_CMD_BASE + 0x1)

/*******************************************************************************
 *
 *   \brief CMD: To stop the hdmi transmitter
 *
 *           hdmi transmitter will stop giving data to
 *           VIP or ISS
 *
 *
 *******************************************************************************
 */
#define HDMI_TX_CMD_STOP     (HDMI_TX_CMD_BASE + 0x2)

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
#define HDMI_TX_CMD_MAX      (HDMI_TX_CMD_BASE + 0x3)


 /**
 *******************************************************************************
 *
 *  \brief FVID2 driver handle returned by individual drivers
 *
 *******************************************************************************
*/
typedef Ptr HdmiTx_Handle;
 /**
 *******************************************************************************
 *
 *  \brief  Enum for transmitter ID for hdmi transmitters.
 *
 *  These are set of hdmi transmitters currently supported on the EVM
 *
 *******************************************************************************
*/
typedef enum {
    HDMI_TX_SII_9022A,
    /**< HDMI transmitter SiI 9022 */
    HDMI_TX_MAX = 0xFFFFu,
    /**<Max */
    HDMI_TX_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
}HdmiTx_Id;
 /*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */
 /**
 *******************************************************************************
 *
 *  \brief  HDMI Receiver Create params.
 *
 *        This structure is used to set input parameters to create hdmi transmitter.
 *        This structure is used as an argument to make certain BSP function
 *        calls like Bsp_boardGetVideoDeviceInstId to get the sensor device id.
 *        Bsp_boardGetVideoDeviceI2cInstId to get the i2c instance id.
 *        Bsp_boardGetVideoDeviceI2cAddr to get the i2c address.
 *
 *******************************************************************************
*/

typedef struct
{
    HdmiTx_Id                hdmiTxId;
    /**< ID of the hdmi transmitter for which create is getting called. */

    System_Standard          standard;
    /**< 720p or 1080p standard */

    System_DssDispcOvly      dssOvlyId;
    /**< SYSTEM_DSS_DISPC_OVLY_xxx to use for this HDMI TX */

    Bsp_BoardMode            boardMode;
    /**< Board mode */

    HdmiTx_Handle            hdmiTxHandle;
    /**< Hdmi transmitter handle  */
}HdmiTx_CreateParams;

 /**
 *******************************************************************************
 *
 *  \brief  Hdmi transmitter Create return parameters.
 *
 *        This structure is used to set output status of hdmi create
 *          create function.
 *
 *******************************************************************************
*/
typedef struct
{
    Int32 retVal;
    /**< Return value of CreateApi */
}HdmiTx_CreateStatus;
 /*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Set the default Create Params for HDMI transmitter .
 *
 * \param  createParams   [IN] Create parameters for Hdmi transmitter
 *
 *******************************************************************************
*/
Void HdmiTx_CreateParams_Init(HdmiTx_CreateParams *createParams);

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
                      HdmiTx_CreateStatus *createStatus);



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
Int32 HdmiTx_delete(HdmiTx_CreateParams *createParams, Ptr deleteArgs);




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
                        UInt32 cmdStatusArgs);


#ifdef __cplusplus
}
#endif

#endif /* HDMI_RCVR_H*/
