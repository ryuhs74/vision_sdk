 /*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \ingroup SAMPLE_MODULE_API
 *   \defgroup NETWORK_CTRL_API Network Control API
 *
 *   This API is used to enable receiving of commands from a PC based network
 *   controller application.
 *
 *   When command are received a user registered callback gets called to
 *   take action on the command.
 *
 *   The callback can read parameters for the command and send a reply
 *   (with parmaeters) to  the PC application
 *
 *   NOTE: This is NOT a link, NetworkCtrl_init() when called creates
 *         a thread which opens a TCP/IP socket on a predetermined port
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file networkCtrl_api.h
 *
 * \brief Network Control API
 *
 *******************************************************************************
 */


#ifndef _NETWORK_CTRL_API_H_
#define _NETWORK_CTRL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <include/link_api/networkCtrl_if.h>
#include <include/link_api/system.h>

/*******************************************************************************
 *  Typedef's
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief Function callback to handle command received over network
 *
 *         The Network command handler should call
 *         NetworkCtrl_readParams() to read parameters into user supplied buffer
 *         if prmSize > 0
 *
 *         The Network command handler should call
 *         NetworkCtrl_writeParams() to send ACK and parameters, if any,
 *         to the sender.
 *
 *         If there are no return parameters, if should still call
 *         NetworkCtrl_writeParams(NULL, 0, 0) to send ACK to the sender
 *
 *  \param cmd  [IN] The command that is received
 *  \param prmSize [IN] Size of the parameters for the received command
 *
 *******************************************************************************
 */
typedef Void (*NetworkCtrl_Handler)(char *cmd, UInt32 prmSize);

/*******************************************************************************
 *  Functions's
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief Create a thread to handle command recevied over network
 *
 *******************************************************************************
 */
Int32 NetworkCtrl_init();

/*******************************************************************************
 *  \brief Delete the thread used for handling network commands
 *
 *******************************************************************************
 */
Int32 NetworkCtrl_deInit();

/*******************************************************************************
 *  \brief Register a handler that gets called when the specified command
 *         is received
 *
 *         If handler is already registered on this command then error is
 *         returned
 *
 *  \param cmd  [IN] Command, specified as string of max size NETWORK_CTRL_CMD_STRLEN_MAX
 *  \param handler [IN] Handler that is invoked when a command is recieved
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 NetworkCtrl_registerHandler(char *cmd, NetworkCtrl_Handler handler);

/*******************************************************************************
 *  \brief UnRegister a handler for the specified command
 *
 *  \param cmd  [IN] Command, specified as string of max size NETWORK_CTRL_CMD_STRLEN_MAX
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 NetworkCtrl_unregisterHandler(char *cmd);

/*******************************************************************************
 *  \brief Read parameters for the current command that is received
 *
 *  \param pPrm [IN] User buffer into which the parameters will be read
 *  \param prmSize [IN] Size of parmeters to read
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 NetworkCtrl_readParams(UInt8 *pPrm, UInt32 prmSize);

/*******************************************************************************
 *  \brief Writes parameters for the current command that is received
 *         and also send ACK to the sender
 *
 *  \param pPrm [IN] User buffer from which parameters will be sent
 *  \param prmSize [IN] Size of parmeters to write
 *  \param returnStatus [IN] Return status to send to the sender
 *
 *  \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 NetworkCtrl_writeParams(UInt8 *pPrm, UInt32 prmSize, UInt32 returnStatus);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


