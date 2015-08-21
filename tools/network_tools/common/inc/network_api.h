
#ifndef _NETWORK_API_H_
#define _NETWORK_API_H_

#include <osa.h>
#include <unistd.h>
#include <winsock.h>
#include <strings.h>

typedef struct {

    SOCKET clientSocketId;
    UInt32 serverPort;
    char ipAddr[32];

} Network_SockObj;

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */

int Network_connect(Network_SockObj *pObj, char *ipAddr, UInt32 port);
int Network_close(Network_SockObj *pObj);
int Network_read(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 *dataSize);
int Network_write(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 dataSize);

int Network_init();
int Network_deInit();

#endif
