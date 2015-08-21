
#include <network_api.h>

int Network_init()
{
    WORD         wVersionRequested;
    WSADATA      wsaData;

    wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData)) {
        printf("# ERROR: Unable to initialize WinSock for host info");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int Network_deInit()
{
    WSACleanup();

    return 0;
}

int Network_connect(Network_SockObj *pObj, char *ipAddr, UInt32 port)
{
  int sin_size;
  struct hostent *host;
  struct sockaddr_in server;

  host = gethostbyname(ipAddr);

  strcpy(pObj->ipAddr, ipAddr);

  pObj->serverPort = port;

  pObj->clientSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (pObj->clientSocketId == INVALID_SOCKET ) {
    printf("# ERROR: NETWORK: Socket open failed (%s:%d)!!!\n", pObj->ipAddr, pObj->serverPort);
    return OSA_EFAIL;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr = *((struct in_addr *)host->h_addr);
  memset(&(server.sin_zero), 0, 8);

  sin_size = sizeof(server);

  if (connect(pObj->clientSocketId, (struct sockaddr *)&server, sin_size) == -1)
  {
    printf("# ERROR: NETWORK: Server connect Failed (%s:%d)!!!\n", pObj->ipAddr, pObj->serverPort);
    return OSA_EFAIL;
  }

  OSA_printf("# NETWORK: Connected to Server (%s:%d)!!!\n", pObj->ipAddr, pObj->serverPort);

  return OSA_SOK;
}

int Network_close(Network_SockObj *pObj)
{
  int ret;

  if(pObj->clientSocketId < 0)
    return OSA_EFAIL;

  ret = close(pObj->clientSocketId);

  return ret;
}

int Network_read(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 *dataSize)
{
    int actDataSize = 0;
    UInt32 tmpDataSize;

    tmpDataSize = *dataSize;

    while(tmpDataSize > 0)
    {
        actDataSize = recv(pObj->clientSocketId, (void*)dataBuf, tmpDataSize, 0);
        if(actDataSize<=0)
        {
            *dataSize = 0;
            return -1;
        }
        dataBuf += actDataSize;
        tmpDataSize -= actDataSize;
    }

    return 0;
}

int Network_write(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 dataSize)
{
    int actDataSize=0;

    while(dataSize > 0 ) {
        actDataSize = send(pObj->clientSocketId, (void*)dataBuf, dataSize, 0);

        if(actDataSize<=0)
            break;
        dataBuf += actDataSize;
        dataSize -= actDataSize;
    }

    if( dataSize > 0 ) {
        return -1;
    }

    return 0;
}

