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


#include <networkCtrl_api.h>
#include <network_api.h>


#define NETWORK_CTRL_MAX_CMDS        (64)

#define NETWORK_CTRL_MAX_PARAMS     (32)

typedef struct {

    char cmd[NETWORK_CTRL_CMD_STRLEN_MAX];
    void (*handler)();
    int numParams;

} CommandHandler;

typedef struct {

    UInt16 serverPort;
    /**< Server port to use */

    char command[NETWORK_CTRL_CMD_STRLEN_MAX];

    char params[NETWORK_CTRL_MAX_PARAMS][NETWORK_CTRL_CMD_STRLEN_MAX];

    int numParams;

    char ipAddr[32];

    Network_SockObj sockObj;

    CommandHandler cmdHandler[NETWORK_CTRL_MAX_CMDS];

} NetworkCtrl_Obj;

extern NetworkCtrl_Obj gNetworkCtrl_obj;

void ShowUsage();
void ParseCmdLineArgs(int argc, char *argv[]);
void ConnectToServer();
void CloseConnection();
void CommandExecute();

void SendCommand(char *command, void *params, int size);
int RecvResponse(char *command, UInt32 *prmSize);
int RecvResponseParams(char *command, UInt8 *pPrm, UInt32 prmSize);
void RegisterHandler(char *command, void (*handler)(), int numParams);

void handleEcho();
void handleMemRd();
void handleMemWr();
void handleMemSave();
void handleIssRawSave();
void handleIssYuvSave();
void handleIssSendDccFile();
void handleIssSaveDccFile();
void handleIssClearDccQspiMem();
void handleIssSensorRegWrite();
void handleIssSensorRegRead();
void handleIssRead2AParams();
void handleIssWrite2AParams();
void handleStereoCalibImageSave();
void handleStereoCalibSetParams();
void handleStereoSetDynamicParams();
void handleStereoWriteCalibLUTDataToQSPI();
void handleQspiSendFile();
void handleSysReset();
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


