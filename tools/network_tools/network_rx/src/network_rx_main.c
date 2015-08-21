/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_rx_priv.h"


NetworkRx_Obj gNetworkRx_obj;

void ShowUsage()
{
    printf(" \n");
    printf("# \n");
    printf("# network_rx --ipaddr <ipaddr> [--port <server port>] --files <CH0 file> <CH1 file> \n");
    printf("# \n");
    printf("# (c) Texas Instruments 2014\n");
    printf("# \n");
    exit(0);
}

int OpenDataFile(NetworkRx_CmdHeader *pHeader)
{
    int chId = pHeader->chNum;

    if(pHeader->chNum >= gNetworkRx_obj.numCh)
    {
        return -1;
    }

    if(gNetworkRx_obj.fd[chId] == NULL )
    {
        gNetworkRx_obj.frameCount[chId] = 0;
        gNetworkRx_obj.fd[chId] = fopen(gNetworkRx_obj.fileName[chId],  "wb");
        if(gNetworkRx_obj.fd[chId] == NULL)
        {
            printf("# ERROR: Unable to open file [%s]\n", gNetworkRx_obj.fileName[chId]);
            pHeader->dataSize = 0;
            return -1;
        }
    }

    return 0;
}

int WriteBytes(NetworkRx_CmdHeader *pHeader)
{
    int chId = pHeader->chNum;
    int bytesWr;

    bytesWr = fwrite(gNetworkRx_obj.dataBuf, 1, pHeader->dataSize, gNetworkRx_obj.fd[chId]);
    if(bytesWr != pHeader->dataSize)
    {

        /* reached end of file, restart from begining */
        printf("# ERROR: CH%d: File [%s] write failed !!!\n",
            chId,
            gNetworkRx_obj.fileName[chId]);
        fclose(gNetworkRx_obj.fd[chId]);
        gNetworkRx_obj.fd[chId] = NULL;
        gNetworkRx_obj.frameCount[chId] = 0;
        return -1;
    }

    #ifdef DEBUG_LOG
    printf("# INFO: DATA: CH%d: Frame%d: %d bytes\n",
        pHeader->chNum,
        gNetworkRx_obj.frameCount[chId],
        pHeader->dataSize
       );
    #endif
    gNetworkRx_obj.frameCount[chId]++;
    gNetworkRx_obj.totalDataSize[chId] += pHeader->dataSize;

    if(gNetworkRx_obj.frameCount[chId] &&
        (gNetworkRx_obj.frameCount[chId] % 10)==0
      )
    {
        printf("# INFO: DATA: CH%d: Recevied %d frames, %10.2f MB\n",
            pHeader->chNum,
            gNetworkRx_obj.frameCount[chId],
            gNetworkRx_obj.totalDataSize[chId]/(1024.0*1024)
        );
    }

    return 0;
}

int WriteData(NetworkRx_CmdHeader *pHeader)
{
    int status = 0;

    status = OpenDataFile(pHeader);
    if(status < 0)
        return status;

    status = WriteBytes(pHeader);

    return status;
}

int main(int argc, char *argv[])
{
    int status = 0;

    ParseCmdLineArgs(argc, argv);
    Init();

    status = ConnectToServer();
    if(status==0)
    {
        RecvData();
    }
    CloseConnection();

    DeInit();
    return 0;
}

void Init()
{
    Network_init();

    gNetworkRx_obj.dataBuf = malloc(MAX_BUF_SIZE);
    if(gNetworkRx_obj.dataBuf==NULL)
    {
        printf("# ERROR: Unable to allocate memory for buffer !!! \n");
        exit(0);
    }
}

void DeInit()
{
    Network_deInit();

    free(gNetworkRx_obj.dataBuf);
}

int ReadCmdHeader(NetworkRx_CmdHeader *pHeader)
{
    UInt32 dataSize;
    Int32 status;

    dataSize = sizeof(*pHeader);

    status = Network_read(&gNetworkRx_obj.sockObj, (UInt8*)pHeader, &dataSize);

    if(status!=0)
        return NETWORK_ERROR;

    if(pHeader->header!=NETWORK_TX_HEADER
        ||
        pHeader->dataSize > MAX_BUF_SIZE
        )
    {
        return NETWORK_INVALID_HEADER;
    }

    return 0;
}

int ReadData(NetworkRx_CmdHeader *pHeader)
{
    Int32 status;
    UInt32 readDataSize;

    if(pHeader->dataSize==0)
        return 0;

    readDataSize = pHeader->dataSize;

    status = Network_read(&gNetworkRx_obj.sockObj, gNetworkRx_obj.dataBuf, &readDataSize);
    if(status!=0)
        return NETWORK_ERROR;

    pHeader->dataSize = readDataSize;

    return 0;
}

void RecvData()
{
    Int32 status = 0;
    NetworkRx_CmdHeader cmdHeader;

    while(status==0)
    {
        status = ReadCmdHeader(&cmdHeader);
        if(status == 0)
        {
            status = ReadData(&cmdHeader);
        }
        if(status==0)
        {
            status = WriteData(&cmdHeader);
        }
    }
}

int ConnectToServer()
{
    int status;

    printf("# Connecting to server %s:%d ...\n", gNetworkRx_obj.ipAddr, gNetworkRx_obj.serverPort);
    status = Network_connect(&gNetworkRx_obj.sockObj, gNetworkRx_obj.ipAddr, gNetworkRx_obj.serverPort);
    return status;
}

void CloseConnection()
{
    Network_close(&gNetworkRx_obj.sockObj);
}

void ParseCmdLineArgs(int argc, char *argv[])
{
    int i, p;

    memset(&gNetworkRx_obj, 0, sizeof(gNetworkRx_obj));

    gNetworkRx_obj.serverPort = NETWORK_TX_SERVER_PORT;
    gNetworkRx_obj.numCh = 0;
    memset(gNetworkRx_obj.fileName, 0, sizeof(gNetworkRx_obj.fileName));

    for(i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "--ipaddr")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            strcpy(gNetworkRx_obj.ipAddr, argv[i]);
        }
        else
        if(strcmp(argv[i], "--port")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            gNetworkRx_obj.serverPort = atoi(argv[i]);
        }
        else
        if(strcmp(argv[i], "--files")==0)
        {
            i++;
            p=0;
            for( ;i<argc;i++)
            {
                strcpy(gNetworkRx_obj.fileName[p], argv[i]);
                p++;
            }

            gNetworkRx_obj.numCh = p;
        }
    }

    if(gNetworkRx_obj.ipAddr[0]==0
        ||
       gNetworkRx_obj.numCh==0
        )
    {

        if(gNetworkRx_obj.ipAddr[0]==0)
        {
            printf("# ERROR: IP Address of server MUST be specified\n");
        }
        if(gNetworkRx_obj.numCh==0)
        {
            printf("# ERROR: Atleast one output file MUST be specified\n");
        }

        ShowUsage();
        exit(0);
    }

    for(p=0; p<gNetworkRx_obj.numCh; p++)
    {
        gNetworkRx_obj.fd[p] = NULL;
    }
}
