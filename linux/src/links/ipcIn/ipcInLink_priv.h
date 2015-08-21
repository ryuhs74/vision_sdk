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
#include <linux/src/system/system_priv_ipc.h>
#include <include/link_api/ipcLink.h>
#include <linux/src/osa/include/osa_ipc_que.h>
#include <linux/src/osa/include/osa_prf.h>
#include <linux/src/osa/include/osa_mutex.h>
#include <linux/src/osa/include/osa_tsk.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Maximum number of IPC links to be used and stack sizes
 *          to be used.
 *
 *          It is defined based on the processor core, thus giving
 *          flexibility of different number of links / stack sizes for different
 *          processor cores. However for different links on the same processor
 *          core, stack size is kept same.
 *
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */

#define IPC_IN_LINK_OBJ_MAX        (7)

/**
 *******************************************************************************
 * \brief size of IPC_IN link thread stack
 *******************************************************************************
 */
#define IPC_IN_LINK_TSK_STACK_SIZE (OSA_TSK_STACK_SIZE_DEFAULT)


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

    OSA_TskHndl tsk;
    /**< Link task handle */

    System_LinkInfo prevLinkInfo;
    /**< Link channel info */

    System_LinkInfo linkInfo;
    /**< Current Link channel info */

    IpcLink_CreateParams  createArgs;
    /**< create time arguments */

    OSA_IpcQueHandle ipcOut2InQue;
    /**< IPC OUT to IPC IN queue */

    OSA_IpcQueHandle ipcIn2OutQue;
    /**< IPC IN to IPC OUT queue */

    System_Buffer buffers[SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS];
    /**< Placeholder to store the incoming buffers */

    UInt32  payload[SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS]
                        [SYSTEM_MAX_PAYLOAD_SIZE/sizeof(UInt32)];
    /**<  array of payload elements*/

    OSA_BufHndl outBufQue;
    /**< Handle to buffer queue */

    OSA_MutexHndl lock;
    /**< Link level lock, used while updating the link params */

    OSA_LatencyStats  linkLatency;
    /**< Structure to find out min, max and average latency of the link */

    OSA_LatencyStats  srcToLinkLatency;
    /**< Structure to find out min, max and average latency from
     *   source to this link
     */

    OSA_LinkStatistics linkStats;
    /**< Statistics related to this link */

    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */

    IpcInLink_LatencyStats ipcLatencyStats;
    /**< IPC specific latency stats */

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

Void  IpcInLink_drvNotifyCb(OSA_TskHndl * pTsk);

Void IpcInLink_latencyStatsReset(IpcInLink_obj *pObj);

Void IpcInLink_latencyStatsUpdate(IpcInLink_obj *pObj,
                                UInt64 ipcLatency,
                                UInt64 notifyLatency );

Void IpcInLink_latencyStatsPrint(IpcInLink_obj *pObj, Bool resetStats);

#endif

/* @} */
