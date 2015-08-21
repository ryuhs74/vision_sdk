 /*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _NETWORK_RX_PRIV_H_
#define _NETWORK_RX_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <osa.h>
#include <networkCtrl_if.h>
#include <network_api.h>

#define MAX_CH  (8)

#define NETWORK_ERROR   (-1)
#define NETWORK_INVALID_HEADER  (-2)

#define MAX_BUF_SIZE    (1920*1080*2)

//#define DEBUG_LOG

typedef struct {

    UInt16 serverPort;
    /**< Server port to use */

    char ipAddr[32];

    Network_SockObj sockObj;

    int numCh;

    char fileName[MAX_CH][1024];

    unsigned int payloadType;

    UInt8 *dataBuf;

    FILE *fd[MAX_CH];

    int frameCount[MAX_CH];

    unsigned long long totalDataSize[MAX_CH];

} NetworkRx_Obj;

extern NetworkRx_Obj gNetworkRx_obj;

void ShowUsage();
void ParseCmdLineArgs(int argc, char *argv[]);
int  ConnectToServer();
void CloseConnection();
void Init();
void DeInit();

int ReadCmdHeader(NetworkRx_CmdHeader *pHeader);
int ReadData(NetworkRx_CmdHeader *pHeader);
void RecvData();
int WriteData(NetworkRx_CmdHeader *pHeader);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


