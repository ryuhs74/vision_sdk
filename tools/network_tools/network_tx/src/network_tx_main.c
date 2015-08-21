/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_tx_priv.h"


NetworkTx_Obj gNetworkTx_obj;

void ShowUsage()
{
    printf(" \n");
    printf("# \n");
    printf("# network_tx --ipaddr <ipaddr> [--port <server port>] --files <CH0 file> <CH1 file> ... \n");
    printf("# \n");
    printf("# (c) Texas Instruments 2014\n");
    printf("# \n");
    exit(0);
}

int OpenDataFile(NetworkRx_CmdHeader *pHeader)
{
    int chId = pHeader->chNum;

    if(gNetworkTx_obj.fd[chId] == NULL )
    {
        gNetworkTx_obj.frameCount[chId] = 0;
        gNetworkTx_obj.fd[chId] = fopen(gNetworkTx_obj.fileName[chId],  "rb");
        if(gNetworkTx_obj.fd[chId] == NULL)
        {
            printf("# ERROR: Unable to open file [%s]\n", gNetworkTx_obj.fileName[chId]);
            pHeader->dataSize = 0;
            return -1;
        }
    }

    return 0;
}

int ReadBytes(NetworkRx_CmdHeader *pHeader)
{
    int chId = pHeader->chNum;
    int bytesRead;

read_again:
    bytesRead = fread(gNetworkTx_obj.dataBuf, 1, pHeader->dataSize, gNetworkTx_obj.fd[chId]);
    if(bytesRead != pHeader->dataSize)
    {

        /* reached end of file, restart from begining */
        #ifdef DEBUG_LOG
        printf("# INFO: DATA: CH%d: Frames %d: Reached end of file [%s] !!!\n",
            chId,
            gNetworkTx_obj.frameCount[chId],
            gNetworkTx_obj.fileName[chId]);
        #endif
        fseek(gNetworkTx_obj.fd[chId], 0, SEEK_SET);
        gNetworkTx_obj.frameCount[chId] = 0;
        goto read_again;
    }

    #ifdef DEBUG_LOG
    printf("# INFO: DATA: CH%d: Frame%d: %d bytes\n",
        pHeader->chNum,
        gNetworkTx_obj.frameCount[chId],
        pHeader->dataSize
       );
    #endif
    gNetworkTx_obj.frameCount[chId]++;

    return 0;
}

#define FOUND_NONE      (0)
#define FOUND_FF        (1)
#define FOUND_FF_D8     (2)
#define FOUND_FF_D9     (3)


int ReadJpeg(NetworkRx_CmdHeader *pHeader)
{
    int chId = pHeader->chNum;
    int bytesRead, i;
    unsigned char dataByte;
    int state, state1;

    state = FOUND_NONE;
    state1 = FOUND_NONE;
    i = 0;

    while(state!=FOUND_FF_D9)
    {
read_again:
        bytesRead = fread(&dataByte, 1, 1, gNetworkTx_obj.fd[chId]);
        if(bytesRead != 1)
        {
            /* reached end of file, restart from begining */
            #ifdef DEBUG_LOG
            printf("# INFO: JPEG: CH%d: Frames %d: Reached end of file [%s] !!!\n",
                chId,
                gNetworkTx_obj.frameCount[chId],
                gNetworkTx_obj.fileName[chId]);
            #endif
            fseek(gNetworkTx_obj.fd[chId], 0, SEEK_SET);
            gNetworkTx_obj.frameCount[chId] = 0;

            state = FOUND_NONE;
            state1 = FOUND_NONE;
            i = 0;
            goto read_again;
        }

        if(state==FOUND_NONE)
        {
            if(dataByte==0xFFu)
            {
                state = FOUND_FF;
            }
        }
        else
        if(state==FOUND_FF)
        {
            if(dataByte==0xD8u)
            {
                state = FOUND_FF_D8;
                i=0;
                gNetworkTx_obj.dataBuf[i++] = 0xFF;
                gNetworkTx_obj.dataBuf[i++] = 0xD8;
            }
            else
            if(dataByte==0xFFu)
            {
                state = FOUND_FF;
            }
            else
            {
                state = FOUND_NONE;
            }
        }
        else
        if(state==FOUND_FF_D8)
        {
            gNetworkTx_obj.dataBuf[i++] = dataByte;

            if(dataByte==0xFF)
            {
                state1 = FOUND_FF;
            }
            else
            if(dataByte==0xD9)
            {
                if(state1 == FOUND_FF)
                    state = FOUND_FF_D9;
            }
            else
            {
                state1 = FOUND_NONE;
            }
        }
    }
    pHeader->dataSize = i;
    #ifdef DEBUG_LOG
    printf("# INFO: JPEG: CH%d: Frame%d: %d bytes\n",
        pHeader->chNum,
        gNetworkTx_obj.frameCount[chId],
        pHeader->dataSize
       );
    #endif
    gNetworkTx_obj.frameCount[chId]++;

    return 0;
}

int ReadData(NetworkRx_CmdHeader *pHeader)
{
    int status = 0;

    status = OpenDataFile(pHeader);
    if(status < 0)
        return status;

    if(pHeader->payloadType==NETWORK_RX_TYPE_BITSTREAM_MJPEG)
    {
        ReadJpeg(pHeader);
    }
    else
    {
        ReadBytes(pHeader);
    }

    return status;
}

int main(int argc, char *argv[])
{
    int status = 0;

    ParseCmdLineArgs(argc, argv);
    Init();

    while(1)
    {
        status = ConnectToServer();
        if(status==0)
        {
            SendData();
        }
        CloseConnection();
    }

    DeInit();
    return 0;
}

void Init()
{
    Network_init();

    gNetworkTx_obj.dataBuf = malloc(MAX_BUF_SIZE);
    if(gNetworkTx_obj.dataBuf==NULL)
    {
        printf("# ERROR: Unable to allocate memory for buffer !!! \n");
        exit(0);
    }
}

void DeInit()
{
    Network_deInit();

    free(gNetworkTx_obj.dataBuf);
}

int ReadCmdHeader(NetworkRx_CmdHeader *pHeader)
{
    UInt32 dataSize;
    Int32 status;

    dataSize = sizeof(*pHeader);

    status = Network_read(&gNetworkTx_obj.sockObj, (UInt8*)pHeader, &dataSize);

    if(status!=0)
        return NETWORK_ERROR;

    if(pHeader->header!=NETWORK_RX_HEADER
        ||
       pHeader->chNum >= gNetworkTx_obj.numCh
        ||
       pHeader->dataSize >= MAX_BUF_SIZE
        )
    {
        pHeader->dataSize = 0;
        pHeader->width = 0;
        pHeader->height = 0;
        pHeader->pitch[0] = pHeader->pitch[1] = 0;
        return NETWORK_INVALID_HEADER;
    }

    return 0;
}

int WriteCmdHeader(NetworkRx_CmdHeader *pHeader)
{
    UInt32 dataSize;
    Int32 status;

    pHeader->header = NETWORK_RX_HEADER;

    dataSize = sizeof(*pHeader);

    status = Network_write(&gNetworkTx_obj.sockObj, (UInt8*)pHeader, dataSize);
    if(status!=0)
        return NETWORK_ERROR;

    return 0;
}

int WriteData(NetworkRx_CmdHeader *pHeader)
{
    Int32 status;

    if(pHeader->dataSize==0)
        return 0;

    status = Network_write(&gNetworkTx_obj.sockObj, gNetworkTx_obj.dataBuf, pHeader->dataSize);
    if(status!=0)
        return NETWORK_ERROR;

    return 0;
}

void SendData()
{
    Int32 status = 0;
    NetworkRx_CmdHeader cmdHeader;

    while(1)
    {

        status = ReadCmdHeader(&cmdHeader);
        if(status==NETWORK_ERROR)
            break;

        if(status == 0)
        {
            ReadData(&cmdHeader);
        }


        status = WriteCmdHeader(&cmdHeader);
        if(status==NETWORK_ERROR)
            break;

        status = WriteData(&cmdHeader);
        if(status==NETWORK_ERROR)
            break;

    }
}

int ConnectToServer()
{
    int status;

    printf("# Connecting to server %s:%d ...\n", gNetworkTx_obj.ipAddr, gNetworkTx_obj.serverPort);
    status = Network_connect(&gNetworkTx_obj.sockObj, gNetworkTx_obj.ipAddr, gNetworkTx_obj.serverPort);
    return status;
}

void CloseConnection()
{
    Network_close(&gNetworkTx_obj.sockObj);
}

void ParseCmdLineArgs(int argc, char *argv[])
{
    int i, p;
    struct stat fileStat;
    int status;

    memset(&gNetworkTx_obj, 0, sizeof(gNetworkTx_obj));

    gNetworkTx_obj.serverPort = NETWORK_RX_SERVER_PORT;
    gNetworkTx_obj.numCh = 0;
    memset(gNetworkTx_obj.fileName, 0, sizeof(gNetworkTx_obj.fileName));

    for(i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "--ipaddr")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            strcpy(gNetworkTx_obj.ipAddr, argv[i]);
        }
        else
        if(strcmp(argv[i], "--port")==0)
        {
            i++;
            if(i>=argc)
            {
                ShowUsage();
            }
            gNetworkTx_obj.serverPort = atoi(argv[i]);
        }
        else
        if(strcmp(argv[i], "--files")==0)
        {
            i++;
            p=0;
            for( ;i<argc;i++)
            {
                strcpy(gNetworkTx_obj.fileName[p], argv[i]);
                p++;
            }

            gNetworkTx_obj.numCh = p;
        }
    }

    if(gNetworkTx_obj.ipAddr[0]==0
        ||
       gNetworkTx_obj.numCh==0
        )
    {

        if(gNetworkTx_obj.ipAddr[0]==0)
        {
            printf("# ERROR: IP Address of server MUST be specified\n");
        }
        if(gNetworkTx_obj.numCh==0)
        {
            printf("# ERROR: Atleast one input file MUST be specified\n");
        }

        ShowUsage();
        exit(0);
    }

    for(p=0; p<gNetworkTx_obj.numCh; p++)
    {
        status = stat(gNetworkTx_obj.fileName[p], &fileStat);
        if(status<0)
        {
            printf("# ERROR: [%s] input file NOT found !!!\n", gNetworkTx_obj.fileName[p]);
            ShowUsage();
            exit(0);
        }
        gNetworkTx_obj.fd[p] = NULL;
    }
}
