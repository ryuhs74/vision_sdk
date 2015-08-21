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
 * \ingroup NETWORK_CTRL_API
 * \defgroup NETWORK_CTRL_IMPL NetworkCtrl  Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file NetworkCtrl_priv.h NetworkCtrl  private API/Data structures
 *
 *******************************************************************************
 */

#ifndef _NETWORK_CTRL_PRIV_H_
#define _NETWORK_CTRL_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */

#include <include/link_api/networkCtrl_api.h>
#include <src/links_common/system/system_priv_common.h>
#include <src/utils_common/include/network_api.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#define NETWORK_CTRL_MAX_CMDS           (64)


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */


typedef struct {

    char cmd[NETWORK_CTRL_CMD_STRLEN_MAX];
    NetworkCtrl_Handler handler;

} NetworkCtrl_CmdHandler;


/**
 *******************************************************************************
 *
 * \brief Structure to hold all NetworkCtrl  related information
 *
 *******************************************************************************
 */
typedef struct {

    BspOsal_TaskHandle task;
    /**< Task to handle commands from networking */

    Bool tskExit;
    /**< Flag to exit task */

    UInt16 serverPort;
    /**< Server port to use */

    NetworkCtrl_CmdHeader cmdBuf;
    /**< Buffer for recevied command header */

    Network_SockObj sockObj;
    /**< Networking socket */

    NetworkCtrl_CmdHandler cmdHandler[NETWORK_CTRL_MAX_CMDS];

} NetworkCtrl_Obj;


Int32 NetworkCtrl_init();
Int32 NetworkCtrl_deInit();

Void NetworkCtrl_cmdHandlerUnsupportedCmd(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerEcho(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerMemRd(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerMemWr(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerMemSave(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerIssRawSave(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerIssYuvSave(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerIssDccSendFile(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerIssSaveDccFile(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerIssClearDccQspiMem(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandleIssWriteSensorReg(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandleIssReadSensorReg(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandleIssRead2AParams(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandleIssWrite2AParams(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerStereoCalibImageSave(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerStereoWriteCalibLUTToQSPI(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerStereoSetParams(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerStereoSetDynamicParams(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerQspiWrite(char *cmd, UInt32 prmSize);
Void NetworkCtrl_cmdHandlerSysReset(char *cmd, UInt32 prmSize);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


