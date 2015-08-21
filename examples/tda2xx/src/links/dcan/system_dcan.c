/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file system_dcan.c
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "include/link_api/grpxSrcLink.h"
#include "include/link_api/dcanCtrl_api.h"
#include "utils_dcan.h"

#define SYSTEM_DCAN_TX_PRD_MS            (10000)
#define SYSTEM_DCAN_TX_TSK_PRI           (13)
#define SYSTEM_DCAN_RX_TSK_PRI           (13)
#define SYSTEM_DCAN_INTR_ID              (28)
/** \brief DCAN input clock - 20MHz */
#define SYSTEM_DCAN_INPUT_CLK            (20000000U)
/** \brief DCAN output bit rate - 1MHz */
#define SYSTEM_DCAN_BIT_RATE             (1000000U)

static Void System_dcanRxMsgHandler(dcanMsg_t *rxMsg);
static Void System_dcanRxAckMsgHandler(dcanMsg_t *rxMsg);

dcanConfig_t System_dcanConfig;

static Void System_dcanInitCfgStruct(dcanConfig_t * dcanConfig)
{
    Int i;
    UInt8 msgData[] = {0xAA,0xEE,0xBB,0xFF,0xCC,0xDD,0x55,0xAA};

    dcanConfig->enableLoopback   = TRUE;
    dcanConfig->enablePeriodicTx = TRUE;
    dcanConfig->rxMsgId         = 0xC2;
    dcanConfig->txMsgId         = 0xC1;
    dcanConfig->enableSendRxAck = TRUE;
    dcanConfig->txAckMsgId      = 0xC3;
    dcanConfig->rxMsgCb         = System_dcanRxMsgHandler;
    dcanConfig->rxAckMsgCb      = System_dcanRxAckMsgHandler;
    dcanConfig->txMsgPeriod_ms  = SYSTEM_DCAN_TX_PRD_MS;
    dcanConfig->dcanCntrlIntrId = SYSTEM_DCAN_INTR_ID;
    dcanConfig->dcanTxTskPri    = SYSTEM_DCAN_TX_TSK_PRI;
    dcanConfig->dcanRxTskPri    = SYSTEM_DCAN_RX_TSK_PRI;
    dcanConfig->dcanInputClk_hz = SYSTEM_DCAN_INPUT_CLK;
    dcanConfig->dcanBaudRate_hz  = SYSTEM_DCAN_BIT_RATE;
    dcanConfig->enableTxMsgCycle = TRUE;
    dcanConfig->dcanTxPrdMsg.dataLength = UTILS_ARRAYSIZE(msgData);
    for ( i = 0 ; i < UTILS_ARRAYSIZE(msgData); i++)
    {
        dcanConfig->dcanTxPrdMsg.msgData[i] = msgData[i];
    }
}

static Void System_dcanRxMsgHandler(dcanMsg_t *rxMsg)
{
    GrpxSrcLink_StringRunTimePrintParams printPrms;

    if (rxMsg->appMsgPrms.dataLength)
    {
        UTILS_COMPILETIME_ASSERT(UTILS_ARRAYSIZE(printPrms.stringInfo.string) >
                              0);
        Vps_printf("DCAN MSG Received:Id[0x%X] , Length : [%d], Data [0 .. 7]"
                   "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
                    rxMsg->msgId,rxMsg->appMsgPrms.dataLength,
                    rxMsg->appMsgPrms.msgData[0],
                    rxMsg->appMsgPrms.msgData[1],
                    rxMsg->appMsgPrms.msgData[2],
                    rxMsg->appMsgPrms.msgData[3],
                    rxMsg->appMsgPrms.msgData[4],
                    rxMsg->appMsgPrms.msgData[5],
                    rxMsg->appMsgPrms.msgData[6],
                    rxMsg->appMsgPrms.msgData[7]);
        snprintf(printPrms.stringInfo.string,
                 sizeof(printPrms.stringInfo.string) - 1,
                 "DCAN MSG ID:[0x%X]\n"
                 "DCAN MSG:0x%2X, 0x%2X, 0x%2X, 0x%2X, 0x%2X, 0x%2X, 0x%2X, 0x%2X",
                 rxMsg->msgId,
                 rxMsg->appMsgPrms.msgData[0],
                 rxMsg->appMsgPrms.msgData[1],
                 rxMsg->appMsgPrms.msgData[2],
                 rxMsg->appMsgPrms.msgData[3],
                 rxMsg->appMsgPrms.msgData[4],
                 rxMsg->appMsgPrms.msgData[5],
                 rxMsg->appMsgPrms.msgData[6],
                 rxMsg->appMsgPrms.msgData[7]);
        printPrms.stringInfo.string[sizeof(printPrms.stringInfo.string) - 1] = 0;
        printPrms.duration_ms = SYSTEM_DCAN_CTRL_DISPLAY_DURATION_MS;
        printPrms.stringInfo.fontType = SYSTEM_DCAN_CTRL_DISPLAY_FONTID;
        printPrms.stringInfo.startX  = SYSTEM_DCAN_CTRL_DISPLAY_STARTX;
        printPrms.stringInfo.startY  = SYSTEM_DCAN_CTRL_DISPLAY_STARTY;
        System_linkControl(IPU1_0_LINK(SYSTEM_LINK_ID_GRPX_SRC_0),
                           GRPX_SRC_LINK_CMD_PRINT_STRING,
                           &printPrms,
                           sizeof(printPrms),
                           TRUE);
    }
}


static Void System_dcanRxAckMsgHandler(dcanMsg_t *rxMsg)
{
    Vps_printf("DCAN TX ACk MsgId:%d,Length:%d,Payload[0]:0x%2x",
               rxMsg->msgId,
               rxMsg->appMsgPrms.dataLength,
               rxMsg->appMsgPrms.msgData[0]);
}


Void System_dcanInit()
{
    System_dcanInitCfgStruct(&System_dcanConfig);
    Utils_dcanInit(&System_dcanConfig);
}

Void System_dcanDeInit()
{
    Utils_dcanDeInit();
}



