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
 * \ingroup SYNC_LINK_API
 * \defgroup SYNC_LINK_IMPL Sync Link Implementation
 *
 * @{
 */

 /**
 *******************************************************************************
 *
 * \file syncLink_priv.h
 *
 * \brief Sync Link Private Header File
 *
 *        This file has the structures, enums, function prototypes
 *        for sync link, which are not exposed to the application
 *
 * \version 0.0 (Aug 2013) : [NN] First version
 *
 *******************************************************************************
 */

 #ifndef _SYNC_LINK_PRIV_H
 #define _SYNC_LINK_PRIV_H

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_common.h>
#include <include/link_api/syncLink.h>

#define SYNC_DEBUG 1


/**
 *******************************************************************************
 *
 * \brief Beyond this value thresholds are not effective
 *
 *******************************************************************************
 */
#define SYNC_DROP_THRESHOLD_MAX      (0xFFF)

/**
 *******************************************************************************
 *
 * \brief Maximum number of sync link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SYNC_LINK_OBJ_MAX    (4)

/**
 *******************************************************************************
 * \brief Max number of elemtns in sync link local queue
 *******************************************************************************
 */
#define SYNC_LINK_LOCAL_QUE_MAX_ELEMENTS        (10)

/**
 *******************************************************************************
 * \brief Sync link logs information for dropped buffers. It logs for upto 1000
 *         buffers after which a wrap around occurs
 *******************************************************************************
 */
#define SYNC_LINK_MAX_DROP_BUFFER_STATS         (20)

/**
 *******************************************************************************
 * \brief Sync link max frames per out queue.
 *
 *******************************************************************************
 */
#define SYNC_LINK_MAX_FRAMES_PER_OUT_QUE        (8)

/**
 *******************************************************************************
 * \brief Sync link max frames per out queue.
 *
 *******************************************************************************
 */
#define SYNC_LINK_INVALID_TIMESTAMP        (0)


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
    Utils_QueHandle    localQueHandle;
    /**< Handle to the queue for this channel */
    System_Buffer      *queMem[SYNC_LINK_LOCAL_QUE_MAX_ELEMENTS];
    /**< Queue memory */
    UInt32             dropCountNoSync;
    /**< Buffers dropped in this channel because this channel is out of sync */
    UInt32             dropCountNoBuffers;
    /**< Buffers dropped in this channel because of absence of buffers in other
         channels */
    UInt32             forwardCount;
    /**< Buffers forwarded from this channel */
} SyncLink_ChObj;


/**
 *******************************************************************************
 * \brief Sync link dropped buffer stats structure.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32    masterTimestamp;
    /**< Reference timestamp. It might be the average of buffer timestamps from
         all channels or simply the master channel buffer timestamp */

    UInt32    bufferTimestamp;
    /**< Actual timestamp of the buffer */

    UInt32    curTimestamp;
    /**< Time at which this frame was dropped */

    UInt32    chNum;
    /**< Channel number from which the buffer is dropped */
} SyncLink_DropBufferStats;

/**
 *******************************************************************************
 * \brief Sync link stats structure.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32    totalDropCount;
    /**< Total dropped buffers in the sync link across all channels */

    SyncLink_DropBufferStats dropStats[SYNC_LINK_MAX_DROP_BUFFER_STATS];
    /**< stats object to store dropped buffer statistics */

    UInt32  totalSyncDelta;
    /**< Total timestamp sync delta */
    UInt32 syncDeltaCount;
    /**< Used with totalSyncDelta to find avergae sync delta */

} SyncLink_BufferStats;

/**
 *******************************************************************************
 * \brief Structure to hold the original System Buffer pointers when a
 *        composite frame is constructed
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 numChannels;
    System_Buffer *bufPtr[SYNC_LINK_MAX_CHANNELS];
} SyncLink_OrigBufferPtr;

typedef struct
{
    Int32 flag;
    UInt32 drop;
} SyncLink_ChDecisionParams;


/**
 *******************************************************************************
 *
 *  \brief  Sync Link Object
 *
 *******************************************************************************
*/
typedef struct {

    UInt32 tskId;
    /**< Placeholder to store sync link task id */

    Utils_TskHndl tsk;
    /**< Link task handle */

    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */

    UInt32 linkInstId;
    /**< Instance Id of the link */

    SyncLink_CreateParams  createArgs;
    /**< create time arguments */

    System_LinkInfo prevLinkInfo;
    /**< Previous link information */

    System_LinkInfo linkInfo;
    /**< Current link information */

    SyncLink_ChObj  chObj[SYNC_LINK_MAX_CHANNELS];
    /**< Instances for each channel */

    Utils_BufHndl outFrameQue;
    /**< Handle for the output queue */

    SyncLink_BufferStats stats;
    /**< Sync Link Stats object */

    System_VideoFrameCompositeBuffer compBuf[SYNC_LINK_MAX_FRAMES_PER_OUT_QUE];
    /**< Composite buffers instances */

    System_Buffer outBuf[SYNC_LINK_MAX_FRAMES_PER_OUT_QUE];
    /**< Output System Buffer instances */

    System_BufferList dropBufList;
    /**< Holds all dropped frames while sync logic iterates through */

    UInt32 masterTimeStamp;
    /**< Current master timestamp, which may be the average of all timestamps
         of buffers in the channel or master channel timestamp */

    SyncLink_ChDecisionParams chDecParams[SYNC_LINK_MAX_CHANNELS];
    /**< Structure which holds decision to retain or drop frame when
         buffers go out of sync */

    SyncLink_OrigBufferPtr origBufPtr[SYNC_LINK_MAX_FRAMES_PER_OUT_QUE];
    /**< Structure to hold the original System Buffer pointers when a composite
         frame is constructed */

    SyncLink_LatestSyncDelta latestSyncDelta;
    /**< Structure to hold the timestamps of the latest buffers that synced.
         Typically this info is needed for sensor for tuning */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool isFirstFrame;
    /**< Flag to indicate first frame is received */

    BspOsal_ClockHandle timer;
    /**<Timer used to generate new data at a set interval*/

} SyncLink_Obj;

extern SyncLink_Obj gSyncLink_obj[];

Void SyncLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

Int32 SyncLink_tskCreate(UInt32 instId);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

