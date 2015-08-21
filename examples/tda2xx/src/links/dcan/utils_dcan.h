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
 * \defgroup UTILS_DCAN_API Utility functions for DCAN
 *
 * \brief This module define APIs for DCAn
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_dcan.h
 *
 * \brief Utility functions for DCAN
 *
 *******************************************************************************
 */

#ifndef _UTILS_DCAN_H_
#define _UTILS_DCAN_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <include/link_api/system.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Text.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/utils/Load.h>
#include <ti/sysbios/hal/Cache.h>


#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_remote_log_if.h>

#include <include/link_api/system_inter_link_api.h>
#include <include/link_api/systemLink_common.h>
#include "dcan.h"


typedef struct dcanMsg_tag
{
    UInt32 msgId;
    dcanMsgParams_t  appMsgPrms;
} dcanMsg_t;

typedef Void (*dcanMsgCallback)(dcanMsg_t *msg);


typedef struct dcanConfig_tag
{
    UInt32 enableLoopback;  /*!< Enable DCAN internal loopback */
    UInt32 enablePeriodicTx;/*!< Enable DCAN Tx thread         */
    UInt32 enableSendRxAck; /*!< Enable sending of Ack on DCAN Rx processing */
    UInt32 txMsgId;         /*!< DCAN Tx Msg Id */
    UInt32 rxMsgId;         /*!< DCAN Rx Msg id */
    UInt32 txAckMsgId;      /*!< MsgId of Tx  Ack Msg */
    UInt32 txMsgPeriod_ms;  /*!< Duration in ms when Tx msg needs to be xmitted*/
    UInt32 dcanCntrlIntrId; /*!< IRQ number to use for DCAN controller interrupt */
    UInt32 dcanRxTskPri;    /*!< Priority of the DCAN Rx thread */
    UInt32 dcanTxTskPri;    /*!< Priority of the DCAN Tx thread */
    UInt32 dcanBaudRate_hz; /*!< DCAN bus baud rate */
    UInt32 dcanInputClk_hz; /*!< DCAN input clock */
    UInt32 enableTxMsgCycle;/*!< Enable cycling of msg bytes in dcan tx msg */
    dcanMsgParams_t dcanTxPrdMsg;/*!< DCAN periodic Tx Msg */
    dcanMsgCallback rxMsgCb;/*!< Handler of DCAN Rx Msgs */
    dcanMsgCallback rxAckMsgCb;/*!< Handler of DCAN Rx Ack Msgs */
} dcanConfig_t;


Void Utils_dcanInit(dcanConfig_t *dcanCfg);
Void Utils_dcanDeInit(void);
void Utils_dcanWriteMsg(dcanMsg_t *msg);



#endif /* ifndef _UTILS_H_ */

/* @} */
