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
 * \ingroup IPC_OUT_LINK_API
 * \defgroup IPC_OUT_LINK_IMPL IPC Out Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file ipcOutLink_priv.h
 *
 * \brief IPC OUT Link Private Header File
 *
 *        This file has the structures, enums, function prototypes
 *        for IPC OUT link, which are not exposed to the application
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _IPC_OUT_LINK_PRIV_H_
#define _IPC_OUT_LINK_PRIV_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_ipc.h>
#include <include/link_api/ipcLink.h>
#include <src/utils_common/include/utils_ipc_que.h>
#include "ipcOutLink_cfg.h"


/**
 *******************************************************************************
 *
 *   \brief Link CMD: Command to tell IPC Out link to look at its
 *                    IPC IN->IPC OUT shared memory queue for buffers to be
 *                    released to previous link
 *
 *   \param None
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define IPC_OUT_LINK_CMD_RELEASE_FRAMES     (0x5100)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *  \brief  Channel specific information
 *
 *******************************************************************************
 */
typedef struct {

    Utils_BufSkipContext bufSkipContext;

} IpcOutLink_ChObj;


/**
 *******************************************************************************
 *
 *  \brief  IPC Out Link Object
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

    IpcLink_CreateParams  createArgs;
    /**< create time arguments */

    System_LinkInfo prevLinkInfo;
    /**< Previous Link channel info */

    System_LinkInfo linkInfo;
    /**< Current Link channel info */

    Utils_IpcQueHandle ipcOut2InQue;
    /**< IPC OUT to IPC IN queue */

    Utils_IpcQueHandle ipcIn2OutQue;
    /**< IPC IN to IPC OUT queue */

    Ptr ipcOut2InSharedMemBaseAddr;
    /**< Base address of IPC queue area */

    Ptr ipcIn2OutSharedMemBaseAddr;
    /**< Base address of IPC queue area */

    Utils_QueHandle localQue;
    /**< local queue handle */

    Ptr localQueMem[SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS];
    /**< Memory for local queue */

    IpcOutLink_ChObj    chObj[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< channel specific information */

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

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by this link */

} IpcOutLink_Obj;

extern IpcOutLink_Obj gIpcOutLink_obj[];

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32 IpcOutLink_drvProcessBuffers(IpcOutLink_Obj *pObj);

Int32 IpcOutLink_drvReleaseBuffers(IpcOutLink_Obj *pObj);

Int32 IpcOutLink_drvStop(IpcOutLink_Obj *pObj);

Int32 IpcOutLink_drvSetFrameRate(IpcOutLink_Obj *pObj,
                              IpcLink_FrameRateParams *pPrm);

Int32 IpcOutLink_drvPrintStatistics(IpcOutLink_Obj *pObj);

Int32 IpcOutLink_drvCreate(IpcOutLink_Obj *pObj, IpcLink_CreateParams *pPrm);

Int32 IpcOutLink_drvDelete(IpcOutLink_Obj *pObj);

Int32 IpcOutLink_getLinkInfo(Void *pTsk,
                             System_LinkInfo *info);

Void  IpcOutLink_drvNotifyCb(Utils_TskHndl * pTsk);

Void IpcOutLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

Int32 IpcOutLink_tskCreate(UInt32 instId);

#endif

/* @} */
