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
#include "nullLink_priv.h"

#define NETWORK_TX_SERVER_CLOSED        (0)
#define NETWORK_TX_SERVER_LISTEN        (1)
#define NETWORK_TX_SERVER_CONNECTED     (2)

#define NETWORK_TX_SERVER_POLL_TIMEOUT  (10)

Int32 NullLink_networkTxCreate(NullLink_Obj *pObj)
{
    NullLink_NetworkTxObj *pNetTxObj = &pObj->netTxObj;
    Int32 status;

    Network_sessionOpen(NULL);

    status = Network_open(
                    &pNetTxObj->sockObj,
                    pObj->createArgs.networkServerPort
                    );
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    pNetTxObj->state = NETWORK_TX_SERVER_LISTEN;

    Vps_printf(" NULL: NETWORK_TX: Server listening (port=%d) !!!\n",
        pObj->createArgs.networkServerPort);

    return status;
}

Int32 NullLink_networkTxDelete(NullLink_Obj *pObj)
{
    NullLink_NetworkTxObj *pNetTxObj = &pObj->netTxObj;
    Int32 status;

    status = Network_close(&pNetTxObj->sockObj, TRUE);
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    Network_sessionClose(NULL);

    pNetTxObj->state = NETWORK_TX_SERVER_CLOSED;

    Vps_printf(" NULL: NETWORK_TX: Server Closed (port=%d) !!!\n",
        pObj->createArgs.networkServerPort);

    return status;
}

Int32 NullLink_networkTxWaitConnect(NullLink_Obj *pObj, NullLink_NetworkTxObj *pNetTxObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool isConnected;

    if(pNetTxObj->state == NETWORK_TX_SERVER_LISTEN)
    {
        isConnected = Network_waitConnect(
                    &pNetTxObj->sockObj,
                    NETWORK_TX_SERVER_POLL_TIMEOUT);

        if(isConnected)
        {
            /* connected to client */
            pNetTxObj->state = NETWORK_TX_SERVER_CONNECTED;
            Vps_printf(" NULL: NETWORK_TX: Connected to client (port=%d) !!!\n",
                pObj->createArgs.networkServerPort);
        }
        else
        {
            status = SYSTEM_LINK_STATUS_EFAIL;
        }
    }

    return status;
}

Int32 NullLink_networkTxWriteHeader(NullLink_Obj *pObj,
        NullLink_NetworkTxObj *pNetTxObj,
        NetworkRx_CmdHeader *pHeader)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    pHeader->header = NETWORK_TX_HEADER;

    /* write header to client, if write failed, then client
     * disconnected, so go back to listening
     */
    status = Network_write(&pNetTxObj->sockObj,
                (UInt8*)pHeader,
                sizeof(*pHeader)
                );

    if(status!=SYSTEM_LINK_STATUS_SOK)
    {
        Vps_printf(" NULL: NETWORK_TX: Disconnected from client "
                   "while writing header (port=%d)!!!\n",
                    pObj->createArgs.networkServerPort
                   );

        Network_close(&pNetTxObj->sockObj, FALSE);
        pNetTxObj->state = NETWORK_TX_SERVER_LISTEN;
    }

    return status;
}

Int32 NullLink_networkTxWritePayload(NullLink_Obj *pObj,
                        NullLink_NetworkTxObj *pNetTxObj,
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
            Cache_inv(
                      (Ptr)SystemUtils_floor((UInt32)bufAddr[i], 128),
                      SystemUtils_align(bufSize[i]+128, 128),
                      Cache_Type_ALLD,
                      TRUE
                    );

            status = Network_write(&pNetTxObj->sockObj,
                        bufAddr[i],
                        bufSize[i]
                        );

            if(status!=SYSTEM_LINK_STATUS_SOK)
            {
                Vps_printf(" NULL: NETWORK_TX: Disconnected from client "
                           "while writing payload (port=%d)!!!\n",
                            pObj->createArgs.networkServerPort
                           );
                Network_close(&pNetTxObj->sockObj, FALSE);
                pNetTxObj->state = NETWORK_TX_SERVER_LISTEN;
                break;
            }

            #if 0
            Vps_printf(" NULL: NETWORK_TX: BUF%d: %d bytes sent (port=%d)!!!\n",
                i,
                bufSize[i],
                pObj->createArgs.networkServerPort
                );
            #endif
        }
    }

    return status;
}

Int32 NullLink_networkTxSendData(NullLink_Obj * pObj, UInt32 queId, UInt32 channelId,
                            System_Buffer *pBuffer)
{
    NullLink_NetworkTxObj *pNetTxObj = &pObj->netTxObj;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_VideoDataFormat dataFormat;
    System_VideoFrameBuffer *videoFrame;
    System_BitstreamBuffer *bitstreamBuf;
    System_MetaDataBuffer *metaBuf;
    System_LinkChInfo *pChInfo;
    UInt16 i;

    status = NullLink_networkTxWaitConnect(pObj, pNetTxObj);

    if(pNetTxObj->state == NETWORK_TX_SERVER_CONNECTED)
    {
        NetworkRx_CmdHeader cmdHeader;
        UInt32 bufSize;

        memset(&cmdHeader, 0, sizeof(cmdHeader));

        cmdHeader.chNum = channelId + queId*pObj->createArgs.numInQue;

        pChInfo = &pObj->inQueInfo[queId].chInfo[channelId];

        switch(pBuffer->bufType)
        {
            case SYSTEM_BUFFER_TYPE_BITSTREAM:
                bitstreamBuf = ((System_BitstreamBuffer *)pBuffer->payload);

                /* right now only MJPEG TX is supported */
                cmdHeader.payloadType = NETWORK_RX_TYPE_BITSTREAM_MJPEG;
                cmdHeader.dataSize = bitstreamBuf->fillLength; /* max buffer size */
                break;

            case SYSTEM_BUFFER_TYPE_METADATA:
                metaBuf = ((System_MetaDataBuffer *)pBuffer->payload);

                /* right now only MJPEG TX is supported */
                cmdHeader.payloadType = NETWORK_RX_TYPE_META_DATA;
                cmdHeader.dataSize = 0; /* max buffer size */

                for(i=0; i<metaBuf->numMetaDataPlanes; i++)
                {
                    cmdHeader.dataSize += metaBuf->metaFillLength[i];
                }
                break;

            case SYSTEM_BUFFER_TYPE_VIDEO_FRAME:
                videoFrame = ((System_VideoFrameBuffer*)pBuffer->payload);
                dataFormat = (System_VideoDataFormat)
                 SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pChInfo->flags);

                if(dataFormat == SYSTEM_DF_YUV420SP_UV)
                {
                    bufSize =
                    (pChInfo->pitch[0]*pChInfo->height) +
                    (pChInfo->pitch[1]*pChInfo->height/2);

                    cmdHeader.payloadType = NETWORK_RX_TYPE_VIDEO_FRAME_YUV420SP_UV;
                }
                else
                {
                    bufSize =
                        pChInfo->pitch[0]*pChInfo->height;

                    cmdHeader.payloadType = NETWORK_RX_TYPE_VIDEO_FRAME_YUV422I_YUYV;
                }

                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    cmdHeader.dataSize = bufSize; /* max buffer size */
                    cmdHeader.width = pChInfo->width;
                    cmdHeader.height = pChInfo->height;
                    cmdHeader.pitch[0] = pChInfo->pitch[0];
                    cmdHeader.pitch[1] = pChInfo->pitch[1];
                }

                break;

            default:
                status = SYSTEM_LINK_STATUS_EFAIL;
                break;
        }

        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            status = NullLink_networkTxWriteHeader(pObj, pNetTxObj, &cmdHeader);
        }
        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            UInt32 numBuf;
            UInt8 *dataAddr[SYSTEM_MAX_META_DATA_PLANES];
            UInt32 dataSize[SYSTEM_MAX_META_DATA_PLANES];

            /* read payload data */
            switch(pBuffer->bufType)
            {
                case SYSTEM_BUFFER_TYPE_BITSTREAM:
                    numBuf = 1;
                    dataAddr[0] = bitstreamBuf->bufAddr;
                    dataSize[0] = cmdHeader.dataSize;
                    break;

                case SYSTEM_BUFFER_TYPE_METADATA:
                    numBuf = metaBuf->numMetaDataPlanes;

                    for(i=0; i<metaBuf->numMetaDataPlanes; i++)
                    {
                        dataAddr[i] = metaBuf->bufAddr[i];
                        dataSize[i] = metaBuf->metaFillLength[i];
                    }
                    break;

                case SYSTEM_BUFFER_TYPE_VIDEO_FRAME:

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
                        numBuf = 1;
                        dataAddr[0] = videoFrame->bufAddr[0];
                        dataSize[0] = cmdHeader.pitch[0]*cmdHeader.height*2;
                        UTILS_assert( dataSize[0] == cmdHeader.dataSize );
                    }
                    break;

                default:
                    /* Can never reach here */
                    UTILS_assert(0);
                    break;
            }

            status = NullLink_networkTxWritePayload(
                                pObj, pNetTxObj, numBuf, dataAddr, dataSize);
        }
    }

    return status;
}
