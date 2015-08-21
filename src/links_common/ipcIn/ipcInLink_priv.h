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
 * \ingroup IPC_IN_LINK_API
 * \defgroup IPC_IN_LINK_IMPL IPC In Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file ipcInLink_priv.h
 *
 * \brief IPC IN Link Private Header File
 *
 *        This file has the structures, enums, function prototypes
 *        for IPC IN link, which are not exposed to the application
 *
 * \version 0.0 (Aug 2013) : [CM] First version
 *
 *******************************************************************************
 */

#ifndef _IPC_IN_LINK_PRIV_H_
#define _IPC_IN_LINK_PRIV_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_ipc.h>
#include <include/link_api/ipcLink.h>
#include <src/utils_common/include/utils_ipc_que.h>
#include "ipcInLink_cfg.h"

/**
 *******************************************************************************
 * \brief Max number of elemtns in IPC queue
 *******************************************************************************
 */
#define IPC_IN_LINK_IPC_QUE_MAX_ELEMENTS        (10)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  More precise latency stats specific to IPC link
 *
 *******************************************************************************
 */
typedef struct {

    UInt64 count;
    UInt64 totalIpcLatency; /**< in usecs */
    UInt64 totalNotifyLatency; /**< in usecs */
    UInt64 maxIpcLatency; /**< in usecs */
    UInt64 minIpcLatency; /**< in usecs */
    UInt64 maxNotifyLatency; /**< in usecs */
    UInt64 minNotifyLatency; /**< in usecs */

} IpcInLink_LatencyStats;

/**
 *******************************************************************************
 *
 *  \brief  IPC In Link Object
 *
 *******************************************************************************
*/
typedef struct {

    UInt32 linkId;
    /**< Link ID of this Link Obj */

    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */

    UInt32 linkInstId;
    /**< Instance index of this link */

    Utils_TskHndl tsk;
    /**< Link task handle */

    System_LinkInfo prevLinkInfo;
    /**< Link channel info */

    System_LinkInfo linkInfo;
    /**< Current Link channel info */

    IpcLink_CreateParams  createArgs;
    /**< create time arguments */

    Utils_IpcQueHandle ipcOut2InQue;
    /**< IPC OUT to IPC IN queue */

    Utils_IpcQueHandle ipcIn2OutQue;
    /**< IPC IN to IPC OUT queue */

    System_Buffer buffers[IPC_IN_LINK_IPC_QUE_MAX_ELEMENTS];
    /**< Placeholder to store the incoming buffers */

    UInt32  payload[IPC_IN_LINK_IPC_QUE_MAX_ELEMENTS]
                        [SYSTEM_MAX_PAYLOAD_SIZE/sizeof(UInt32)];
    /**<  array of payload elements*/

    Utils_BufHndl outBufQue;
    /**< Handle to buffer queue */

    BspOsal_SemHandle lock;
    /**< Link level lock, used while updating the link params */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */

    IpcInLink_LatencyStats ipcLatencyStats;
    /**< IPC specific latency stats */

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by this link */

} IpcInLink_obj;

extern IpcInLink_obj gIpcInLink_obj[];

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32 IpcInLink_drvProcessBuffers(IpcInLink_obj *pObj);

Int32 IpcInLink_drvStop(IpcInLink_obj *pObj);

Int32 IpcInLink_drvPrintStatistics(IpcInLink_obj *pObj);

Int32 IpcInLink_drvCreate(IpcInLink_obj *pObj, IpcLink_CreateParams *pPrm);

Int32 IpcInLink_drvDelete(IpcInLink_obj *pObj);

Int32 IpcInLink_getLinkInfo(Void *pTsk, System_LinkInfo *info);

Int32 IpcInLink_drvPutEmptyBuffers(IpcInLink_obj *pObj,
                                    System_BufferList *pBufList);

Int32 IpcInLink_drvGetFullBuffers(IpcInLink_obj *pObj,
                                    System_BufferList *pBufList);

Int32 IpcInLink_drvPrdStart(IpcInLink_obj *pObj);

Int32 IpcInLink_drvPrdStop(IpcInLink_obj *pObj);

Void  IpcInLink_drvNotifyCb(Utils_TskHndl * pTsk);

Void IpcInLink_latencyStatsReset(IpcInLink_obj *pObj);

Void IpcInLink_latencyStatsUpdate(IpcInLink_obj *pObj,
                                UInt64 ipcLatency,
                                UInt64 notifyLatency );

Void IpcInLink_latencyStatsPrint(IpcInLink_obj *pObj, Bool resetStats);

Int32 IpcInLink_tskCreate(UInt32 instId);

Void IpcInLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

#endif

/* @} */
