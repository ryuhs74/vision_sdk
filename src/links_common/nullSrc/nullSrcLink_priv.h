/*
*******************************************************************************
*
* Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*
*******************************************************************************
*/

/**
 ******************************************************************************
 *
 * \ingroup NULLSRC_LINK_API
 * \defgroup NULLSRC_LINK_IMPL Null Link Implementation
 *
 * @{
 */

/**
 ******************************************************************************
 *
 * \file nullSrcLink_priv.h Null Source Link private API/Data structures
 *
 * \brief  This file is a private header file for null source link
 * implementation. It lists the data structures, function prototypes which are
 * implemented and used as a part of null source link.
 *
 * \version 0.0 (Dec 2013) : [VT] First version
 *
 ******************************************************************************
 */

#ifndef _NULL_SRC_LINK_PRIV_H_
#define _NULL_SRC_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  Include files
 ******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/nullSrcLink.h>
#include <src/utils_common/include/network_api.h>

/******************************************************************************
 *  Defines
 ******************************************************************************
 */

/**
 ******************************************************************************
 *
 * \brief Maximum number of output buffers in null source link
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_MAX_OUT_BUFFERS   (NULL_SRC_LINK_MAX_OUT_BUFS_PER_CH \
                                            * NULL_SRC_LINK_MAX_CH)

/******************************************************************************
 *  Data structure's
 ******************************************************************************
 */

/**
 ******************************************************************************
 *
 * \brief Structure to hold all Null Source link related to network receive
 *
 ******************************************************************************
 */
typedef struct {

    Network_SockObj sockObj;
    /**< Information about network socket */

    UInt32 state;
    /**< State of server socket */

} NullSrcLink_NetworkRxObj;

/**
 ******************************************************************************
 *
 * \brief Structure to hold all Null Source link related information
 *
 ******************************************************************************
 */

typedef struct {
    UInt32 tskId;
    /**< Placeholder to store null link source task id */

    Utils_TskHndl tsk;
    /**< Handle to null source link task */

    NullSrcLink_CreateParams createArgs;
    /**< Create params for null source link */

    Utils_QueHandle fullOutBufQue;
    /**< Null Source link output side full buffer queue */

    System_Buffer *pBufferOnFullQ[NULL_SRC_LINK_MAX_OUT_BUFFERS];
    /** Holds full buffers */

    Utils_QueHandle emptyOutBufQue[NULL_SRC_LINK_MAX_CH];
    /**< Null Source link output side empty buffer queue */

    System_Buffer *pBufferOnEmptyQ[NULL_SRC_LINK_MAX_CH]
                                  [NULL_SRC_LINK_MAX_OUT_BUFS_PER_CH];
   /** Holds individual channel empty buffers */

    System_Buffer buffers[NULL_SRC_LINK_MAX_CH]
                         [NULL_SRC_LINK_MAX_OUT_BUFS_PER_CH];
    /**< System buffer data structure to exchange buffers between links */

    System_VideoFrameBuffer videoFrames[NULL_SRC_LINK_MAX_CH]
                                       [NULL_SRC_LINK_MAX_OUT_BUFS_PER_CH];
    /**< Payload for System buffers in case of YUV data*/

    System_BitstreamBuffer bitstreamBuf[NULL_SRC_LINK_MAX_CH]
                                       [NULL_SRC_LINK_MAX_OUT_BUFS_PER_CH];
    /*< Payload for System buffers in case of compressed data*/

    BspOsal_ClockHandle timer;
    /**<Timer used to generate new data at a set interval*/

    System_LinkInfo linkInfo;
    /* Null source link info that is returned when queried by next link */

    FILE *fpDataStream[NULL_SRC_LINK_MAX_CH];
    /**< Binary File containing the stream data.*/

    FILE *fpIndexFile[NULL_SRC_LINK_MAX_CH];
    /**< File used to index into fpDataStream.
    * It will contain frame sizes in bytes.
    */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool isFirstBufferSent;
    /**< flag indicates if Null Src Link has sent any buffers to next link yet*/

    UInt32 numPendingCmds;
    /**< Number of pending NEW_DATA_CMD that are yet to handled */

    NullSrcLink_NetworkRxObj netRxObj;
    /**< Information related to receiving data over network */

} NullSrcLink_Obj;


Int32 NullSrcLink_networkRxCreate(NullSrcLink_Obj *pObj);
Int32 NullSrcLink_networkRxDelete(NullSrcLink_Obj *pObj);
Int32 NullSrcLink_networkRxFillData(NullSrcLink_Obj * pObj, UInt32 channelId,
                            System_Buffer *pBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/
