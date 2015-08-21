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
 * \ingroup MERGE_LINK_API
 * \defgroup MERGE_LINK_IMPL Merge Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file mergeLink_priv.h Merge Link private API/Data structures
 *
 * \brief  This file is a private header file for merge link implementation
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of merge link.
 *         Merge Link is used in cases where the output buffers from multiple
 *         links are to be sent to single link. Merge link simply collects the
 *         buffers and send these buffers to single output queue.
 *         Merge link implements the logic to release buffers to it's previous
 *         link only when the link connected to it's output queue release the
 *         buffers.
 *
 * \version 0.0 (Aug 2013) : [SL] First version
 *
 *******************************************************************************
 */

#ifndef _MERGE_LINK_PRIV_H_
#define _MERGE_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/mergeLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Maximum number of merge link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define MERGE_LINK_OBJ_MAX   (5)

/**
 *******************************************************************************
 *
 * \brief Maximum channels an input queue can support
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define MERGE_LINK_MAX_CH_PER_IN_QUE    (SYSTEM_MAX_CH_PER_OUT_QUE)


/**
 *******************************************************************************
 *
 * \brief Stat structure that stores information about the number of
 *  - Recevied buffers
 *  - Forwarded buffers ( Sent to next link )
 *  - Released buffers ( Sent back to the previous link )
 *
 *******************************************************************************
 */
typedef struct MergeLink_StatsObj {
    UInt32 recvCount[MERGE_LINK_MAX_IN_QUE];
    UInt32 forwardCount;
    UInt32 releaseCount[MERGE_LINK_MAX_IN_QUE];
} MergeLink_StatsObj;

/**
 *******************************************************************************
 *
 * \brief Structure to hold all Merge link related information
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 tskId;
    /**< Placeholder to store merge link task id */

    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */

    Utils_TskHndl tsk;
    /**< Handle to merge link task */

    MergeLink_CreateParams createArgs;
    /**< Create params for merge link */

    System_LinkInfo inTskInfo[MERGE_LINK_MAX_IN_QUE];
    /**< Output queue information of previous link */

    System_LinkInfo info;
    /**< Output queue information of this link */

    BspOsal_SemHandle lock;
    /**< Link level lock, used while updating the link params */

    Utils_BufHndl outBufQue;
    /**< Handles to the output queue */

    System_BufferList inBufList;
    System_BufferList freeBufList[MERGE_LINK_MAX_IN_QUE];

    /* max channel number possible in a input queue */
    UInt32 inQueMaxCh[MERGE_LINK_MAX_IN_QUE];

    /* incoming channel number to outgoing channel number map */
    UInt32 inQueChNumMap[MERGE_LINK_MAX_IN_QUE][MERGE_LINK_MAX_CH_PER_IN_QUE];

    /* outgoing channel number to input que ID map */
    UInt32 outQueChToInQueMap[MERGE_LINK_MAX_IN_QUE *
                              MERGE_LINK_MAX_CH_PER_IN_QUE];

    /* outgoing channel number to incoming channel number map, reverse of
     * inQueChNumMap[] */
    UInt32 outQueChMap[MERGE_LINK_MAX_IN_QUE * MERGE_LINK_MAX_CH_PER_IN_QUE];

    MergeLink_StatsObj stats;
    /**< To store statistics of the buffers */
} MergeLink_Obj;

extern MergeLink_Obj gMergeLink_obj[];

Void MergeLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

Int32 MergeLink_tskCreate(UInt32 instId);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

