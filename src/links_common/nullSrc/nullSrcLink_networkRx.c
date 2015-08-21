/*
 ******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 ******************************************************************************
 */


 /*****************************************************************************
  *  INCLUDE FILES
  *****************************************************************************
  */
#include "nullSrcLink_priv.h"

#define NETWORK_RX_SERVER_CLOSED        (0)
#define NETWORK_RX_SERVER_LISTEN        (1)
#define NETWORK_RX_SERVER_CONNECTED     (2)

#define NETWORK_RX_SERVER_POLL_TIMEOUT  (10)

Int32 NullSrcLink_networkRxCreate(NullSrcLink_Obj *pObj)
{
    NullSrcLink_NetworkRxObj *pNetRxObj = &pObj->netRxObj;
    Int32 status;

    Network_sessionOpen(NULL);

    status = Network_open(
                    &pNetRxObj->sockObj,
                    pObj->createArgs.networkServerPort
                    );
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    pNetRxObj->state = NETWORK_RX_SERVER_LISTEN;

    Vps_printf(" NULL_SRC: NETWORK_RX: Server listening (port=%d) !!!\n",
        pObj->createArgs.networkServerPort);

    return status;
}

Int32 NullSrcLink_networkRxDelete(NullSrcLink_Obj *pObj)
{
    NullSrcLink_NetworkRxObj *pNetRxObj = &pObj->netRxObj;
    Int32 status;

    status = Network_close(&pNetRxObj->sockObj, TRUE);
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    Network_sessionClose(NULL);

    pNetRxObj->state = NETWORK_RX_SERVER_CLOSED;

    Vps_printf(" NULL_SRC: NETWORK_RX: Server Closed (port=%d) !!!\n",
        pObj->createArgs.networkServerPort);

    return status;
}

Int32 NullSrcLink_networkRxWaitConnect(NullSrcLink_Obj *pObj, NullSrcLink_NetworkRxObj *pNetRxObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool isConnected;

    if(pNetRxObj->state == NETWORK_RX_SERVER_LISTEN)
    {
        isConnected = Network_waitConnect(
                    &pNetRxObj->sockObj,
                    NETWORK_RX_SERVER_POLL_TIMEOUT);

        if(isConnected)
        {
            /* connected to client */
            pNetRxObj->state = NETWORK_RX_SERVER_CONNECTED;
            Vps_printf(" NULL_SRC: NETWORK_RX: Connected to client (port=%d) !!!\n",
                pObj->createArgs.networkServerPort);
        }
        else
        {
            status = SYSTEM_LINK_STATUS_EFAIL;
        }
    }

    return status;
}

Int32 NullSrcLink_networkRxWriteHeader(NullSrcLink_Obj *pObj,
        NullSrcLink_NetworkRxObj *pNetRxObj,
        NetworkRx_CmdHeader *pHeader)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    pHeader->header = NETWORK_RX_HEADER;

    /* write header to client, if write failed, then client
     * disconnected, so go back to listening
     */
    status = Network_write(&pNetRxObj->sockObj,
                (UInt8*)pHeader,
                sizeof(*pHeader)
                );

    if(status!=SYSTEM_LINK_STATUS_SOK)
    {
        Vps_printf(" NULL_SRC: NETWORK_RX: Disconnected from client "
                   "while writing header (port=%d)!!!\n",
                    pObj->createArgs.networkServerPort
                   );

        Network_close(&pNetRxObj->sockObj, FALSE);
        pNetRxObj->state = NETWORK_RX_SERVER_LISTEN;
    }

    return status;
}

Int32 NullSrcLink_networkRxReadHeader(NullSrcLink_Obj *pObj,
            NullSrcLink_NetworkRxObj *pNetRxObj,
            NetworkRx_CmdHeader *pHeader)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 dataSize;

    /* read reply */

    dataSize = sizeof(*pHeader);

    status = Network_read(&pNetRxObj->sockObj,
                (UInt8*)pHeader,
                &dataSize
                );

    if(status!=SYSTEM_LINK_STATUS_SOK)
    {
        Vps_printf(" NULL_SRC: NETWORK_RX: Disconnected from client "
                   "while reading header (port=%d)!!!\n",
                    pObj->createArgs.networkServerPort
                   );

        Network_close(&pNetRxObj->sockObj, FALSE);
        pNetRxObj->state = NETWORK_RX_SERVER_LISTEN;
    }
    if(status==SYSTEM_LINK_STATUS_SOK)
    {
        if(pHeader->header!=NETWORK_RX_HEADER
            ||
           pHeader->dataSize == 0
            )
        {
            /* invalid header or no data to read */
            status = SYSTEM_LINK_STATUS_EFAIL;
            Vps_printf(" NULL_SRC: NETWORK_RX: Invalid header received"
                       " (port=%d)!!!\n",
                        pObj->createArgs.networkServerPort
                       );

        }
    }

    return status;
}

Int32 NullSrcLink_networkRxReadPayload(NullSrcLink_Obj *pObj,
                        NullSrcLink_NetworkRxObj *pNetRxObj,
                        UInt32 numBuf,
                        UInt8  *bufAddr[],
                        UInt32 bufSize[])
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 i;

    for(i=0; i<numBuf; i++)
    {
        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            status = Network_read(&pNetRxObj->sockObj,
                        bufAddr[i],
                        &bufSize[i]
                        );

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                Vps_printf(" NULL_SRC: NETWORK_RX: Disconnected from client "
                           "while reading payload (port=%d)!!!\n",
                            pObj->createArgs.networkServerPort
                           );
                Network_close(&pNetRxObj->sockObj, FALSE);
                pNetRxObj->state = NETWORK_RX_SERVER_LISTEN;
                break;
            }

            Cache_wb(
                      (Ptr)SystemUtils_floor((UInt32)bufAddr[i], 128),
                      SystemUtils_align(bufSize[i]+128, 128),
                      Cache_Type_ALLD,
                      TRUE
                    );

            #if 0
            Vps_printf(" NULL_SRC: NETWORK_RX: BUF%d: %d bytes recevied (port=%d)!!!\n",
                i,
                bufSize[i],
                pObj->createArgs.networkServerPort
                );
            #endif
        }
    }

    return status;
}

Int32 NullSrcLink_networkRxFillData(NullSrcLink_Obj * pObj, UInt32 channelId,
                            System_Buffer *pBuffer)
{
    NullSrcLink_NetworkRxObj *pNetRxObj = &pObj->netRxObj;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_VideoDataFormat dataFormat;
    System_VideoFrameBuffer *videoFrame;
    System_BitstreamBuffer *bitstreamBuf;

    status = NullSrcLink_networkRxWaitConnect(pObj, pNetRxObj);

    if(pNetRxObj->state == NETWORK_RX_SERVER_CONNECTED)
    {
        NetworkRx_CmdHeader cmdHeader;
        UInt32 bufSize;

        memset(&cmdHeader, 0, sizeof(cmdHeader));

        cmdHeader.chNum = channelId;

        switch(pBuffer->bufType)
        {
            case SYSTEM_BUFFER_TYPE_BITSTREAM:
                bitstreamBuf = ((System_BitstreamBuffer *)pBuffer->payload);

                /* right now only MJPEG RX is supported */
                cmdHeader.payloadType = NETWORK_RX_TYPE_BITSTREAM_MJPEG;
                cmdHeader.dataSize = bitstreamBuf->bufSize; /* max buffer size */
                break;

            case SYSTEM_BUFFER_TYPE_VIDEO_FRAME:
                videoFrame = ((System_VideoFrameBuffer*)pBuffer->payload);
                dataFormat = (System_VideoDataFormat)
                 SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(videoFrame->chInfo.flags);

                if(dataFormat == SYSTEM_DF_YUV420SP_UV)
                {
                    bufSize =
                    (videoFrame->chInfo.pitch[0]*videoFrame->chInfo.height) +
                    (videoFrame->chInfo.pitch[1]*videoFrame->chInfo.height/2);

                    cmdHeader.payloadType = NETWORK_RX_TYPE_VIDEO_FRAME_YUV420SP_UV;
                }
                else
                if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
                {
                    bufSize =
                        videoFrame->chInfo.pitch[0]*videoFrame->chInfo.height;

                    cmdHeader.payloadType = NETWORK_RX_TYPE_VIDEO_FRAME_YUV422I_YUYV;
                }
                else
                {
                    status = SYSTEM_LINK_STATUS_EFAIL;
                }

                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    cmdHeader.dataSize = bufSize; /* max buffer size */
                    cmdHeader.width = videoFrame->chInfo.width;
                    cmdHeader.height = videoFrame->chInfo.height;
                    cmdHeader.pitch[0] = videoFrame->chInfo.pitch[0];
                    cmdHeader.pitch[1] = videoFrame->chInfo.pitch[1];
                }

                break;

            default:
                status = SYSTEM_LINK_STATUS_EFAIL;
                break;
        }

        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            status = NullSrcLink_networkRxWriteHeader(pObj, pNetRxObj, &cmdHeader);
        }
        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            status = NullSrcLink_networkRxReadHeader(pObj, pNetRxObj, &cmdHeader);
        }
        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            UInt32 numBuf;
            UInt8 *dataAddr[2];
            UInt32 dataSize[2];

            /* read payload data */
            switch(pBuffer->bufType)
            {
                case SYSTEM_BUFFER_TYPE_BITSTREAM:
                    numBuf = 1;
                    dataAddr[0] = bitstreamBuf->bufAddr;
                    dataSize[0] = cmdHeader.dataSize;
                    bitstreamBuf->fillLength = cmdHeader.dataSize;
                    break;

                case SYSTEM_BUFFER_TYPE_VIDEO_FRAME:

                    if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
                    {
                        numBuf = 1;
                        dataAddr[0] = videoFrame->bufAddr[0];
                        dataSize[0] = cmdHeader.pitch[0]*cmdHeader.height*2;
                        UTILS_assert( dataSize[0] == cmdHeader.dataSize );
                    }
                    else
                    if(dataFormat == SYSTEM_DF_YUV420SP_UV)
                    {
                        numBuf = 2;
                        dataAddr[0] = videoFrame->bufAddr[0];
                        dataAddr[1] = videoFrame->bufAddr[1];
                        dataSize[0] = cmdHeader.pitch[0]*cmdHeader.height;
                        dataSize[1] = (cmdHeader.pitch[1]*cmdHeader.height)/2;

                        UTILS_assert( dataSize[0]+dataSize[1] == cmdHeader.dataSize );
                    }
                    else
                    {
                        /* Can never reach here */
                        UTILS_assert(0);
                    }
                    break;

                default:
                    /* Can never reach here */
                    UTILS_assert(0);
                    break;
            }

            status = NullSrcLink_networkRxReadPayload(
                                pObj, pNetRxObj, numBuf, dataAddr, dataSize);

        }
    }

    return status;
}
