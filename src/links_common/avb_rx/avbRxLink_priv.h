/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 ********************************************************************************
 *
 * \ingroup AVB_RX_LINK_API
 * \defgroup AVB_RX_LINK_IMPL AVB Recieve Link Implementation
 *
 * @{
 */


/**
 *******************************************************************************
 *
 * \file avbRxLink_priv.h
 *
 * \brief This file is a private header file for AvbRx link implementation.
 *
 *        This file lists the data structures, function prototypes which are
 *        implemented and used as a part of AvbRx link.
 *
 * \version 0.0 (Nov 2013) : [KRB] First version
 *
 *******************************************************************************
 */

#ifndef _AVB_RX_LINK_PRIV_H_
#define _AVB_RX_LINK_PRIV_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_1.h>
#include <ti/avbtp/inc/avbtp.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 ********************************************************************************
 *
 * \brief Maximum number of output queues that capture link supports.
 *
 * SUPPORTED in ALL platforms
 *
 ********************************************************************************
 */
#define AVB_RX_LINK_MAX_OUT_QUE         (1)


/* @} */

/* Control Command's    */

/**
    \ingroup LINK_API_CMD
    \addtogroup AVB_SOURCE_LINK_API_CMD  Capture Link Control Commands

    @{
*/


/* @} */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief configuration for VIP instnace.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    struct app_buff_details avbBufDetails;
    /**<  */

    uint8_t  avbBufState[AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT];
    /**<  */

    UInt32 avbBufValidLen[AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT];
    /**< */

    uint8_t * avbBufPtr[AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT];
    /**< */
} AvbRxLink_avbBuffDesc;

/**
 *******************************************************************************
 *
 * \brief Forward declaration of AvbRx object.
 *
 *******************************************************************************
 */

typedef struct AvbRxLink_Obj_t AvbRxLink_Obj;


/**
 *******************************************************************************
 * \brief Structure for setting input stream parameters for capture link
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 chID;
    /**< AVBRx Talker channel ID */

    avbtp_listener_handle_t listenerHandle;
    /**< AVBRx Handle */

    uint8_t streamId[AVB_RX_LINK_STREAM_ID_LENGTH];
    /**< AVBRx StreamId */

    uint8_t talkerMacAddr[AVB_RX_LINK_MAC_ID_LENGTH];
    /**< AVBRx Talker MAC Addr */

    AvbRxLink_avbBuffDesc avbBuffDescriptor;
    /**< AVBRx Buffer Descriptor */

    System_Buffer buffers[AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT];
    /**< System buffer data structure to exchange buffers between links */

    System_BitstreamBuffer bitstreamBuf[AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT];
    /**< Payload for System buffers */

    UInt32 numBufs;
    /**< Number of buffers allocated for this instance */

    UInt32  bufferSize;
    /**< Width of the buffers allocated */

    UInt32 dataReadyIndex;
    /**< Index into the buffer descriptor that is ready to be consumed */

} AvbRxLink_ChObj;

/**
 *******************************************************************************
 * \brief Structure containing Link object information
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
struct AvbRxLink_Obj_t
{
    UInt32 tskId;
    /**< Link ID for this link */

    Utils_TskHndl tsk;
    /**< Handle to AvbRx link task */

    AvbRxLink_CreateParams createArgs;
    /**< Create params for AvbRx link */

    Utils_BufHndl bufQue;
    /**< Handle to buffer queue */

    avbtp_handle_t avbHandle;
    /**< Global AVB handle */

    AvbRxLink_ChObj avbSrcObj[AVB_RX_LINK_MAX_TALKERS_DEFAULT];
    /**< Array of AvbRx link instance objects. Fields specific to instance */

    System_LinkInfo info;
    /**< AvbRx link information */

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by AvbRx link */

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
};

/******************************************************************************
 *  Functions
 *******************************************************************************
 */

extern AvbRxLink_Obj gAvbRxLink_obj;


Int32 AvbRxLink_drvCreate(AvbRxLink_Obj * pObj,
                                    AvbRxLink_CreateParams * pPrm);

Int32 AvbRxLink_drvStart(AvbRxLink_Obj * pObj);
Int32 AvbRxLink_drvStop(AvbRxLink_Obj * pObj);
Int32 AvbRxLink_drvDelete(AvbRxLink_Obj * pObj);
Int32 AvbRxLink_drvPrintStatus(AvbRxLink_Obj * pObj);
Int32 AvbRxLink_drvPutEmptyBuffers(AvbRxLink_Obj * pObj, System_BufferList * pBufList);
Int32 AvbRxLink_printBufferStatus(AvbRxLink_Obj * pObj);
Int32 AvbRxLink_getFullBuffers(Void * ptr, UInt16 queId, System_BufferList * pBufList);
Int32 AvbRxLink_putEmptyBuffers(Void * ptr, UInt16 queId, System_BufferList * pBufList);
Int32 AvbRxLink_getInfo(Void * ptr, System_LinkInfo * info);
#endif


/*@}*/
