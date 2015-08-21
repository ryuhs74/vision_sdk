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
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup AVB_RX_LINK_API AVB Receive Link API
 *
 *   \brief AVB Receive Link captures MJPEG video frames from AVB IEEE 1722 compliant
 *   sources (talker). AVB Rx link supports capture of MJPEG frames from multiple
 *   AVB cameras connected over ethernet. In a typical ethernet surround view
 *   based system, these MJPEG frames are then decoded & used for analytics and
 *   stitching. A unique channel ID is assigned to each AVB camera.
 *
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file avbRxLink.h
 *
 * \brief AVB Receive link API public header file.
 *
 * \version 0.0 (Nov 2013) : [KRB] First version
 * \version 0.1 (feb 2014) : [CM] AVB Link up
 *
 *******************************************************************************
 */

#ifndef _AVB_RX_LINK_H_
#define _AVB_RX_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <include/link_api/system.h>
#include <include/link_api/systemLink_ipu1_0_params.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Max AVB talkers supported
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define AVB_RX_LINK_MAX_TALKERS_DEFAULT (5)


/**
 *******************************************************************************
 *
 * \brief Indicates the length of the stream id in bytes
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define AVB_RX_LINK_STREAM_ID_LENGTH (8)

/**
 *******************************************************************************
 *
 * \brief Indicates the length of the mac id in bytes
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define AVB_RX_LINK_MAC_ID_LENGTH (8)

/**
 *******************************************************************************
 *
 * \brief Indicates the minimum number of buffers required per channel
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT (6)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *  \brief Enumerations for AVB Src notification types
 *
 *  SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
typedef enum
{
    AVB_RX_LINK_BUF_NOTIFY_MJPEG_FRAME,
    /**< Enable AVB to invoke callback once a complete MJPEG frame is detected */

    AVB_RX_LINK_BUF_NOTIFY_FULL,
    /**< Enable AVB to invoke callback when a particular buffer has been filled up
     *  but not necessarily a complete MJPEG frame
     */

    AVB_RX_LINK_BUF_USE_NEW_BUFFER_FOR_MJPEG,
    /**< Inform AVB to use a new buffer after previously detecting an MJPEG frame
     * instead of filling up the current buffer.
     */

    AVB_RX_LINK_BUF_NOTIFY_ERROR,
     /**< Enable AVB to invoke callback when an error occurs */

    AVB_RX_LINK_FRAME_NOTIFICATION_TYPE_FORCE32BITS = 0x7FFFFFFF
     /**< This should be the last value after the max enumeration value.
      *   This is to make sure enum size defaults to 32 bits always regardless
      *   of compiler.
      */
} AvbRxLink_FrameNotifcationType;

/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
*/

/**
 *******************************************************************************
 * \brief AVBSrc link configuration parameters.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    System_LinkOutQueParams             outQueParams;
    /**< output queue information */

    UInt8                               streamId[AVB_RX_LINK_MAX_TALKERS_DEFAULT][AVB_RX_LINK_STREAM_ID_LENGTH];
    /**< streamId for the different AVB cameras connected */

    UInt8                               srcMacId[AVB_RX_LINK_MAX_TALKERS_DEFAULT][AVB_RX_LINK_MAC_ID_LENGTH];
    /**< Source MAC Address - Same MAC should be present in the stream ID */

    UInt8                               dstMacId[AVB_RX_LINK_MAC_ID_LENGTH];
    /**< Destination MAC Address */

    AvbRxLink_FrameNotifcationType      bufferNotificationType;
    /**< frame notification type */

    UInt32                              numCameras;
    /**< number of AVB cameras connected */

    UInt32                              numBufs;
    /**< Number of buffers to be allocated for the AVB Rx link. Minimum
     *   number of buffers required is 3 for AVB Rx link to capture without
     *   frame drops
     */
    UInt32                              height;
    /**< Input source height,
     */

    UInt32                              width;
    /**< Input source width,
     */

    UInt32                              buffSize;
    /**< Number of buffers to be allocated for the AVB Rx link. Minimum
     *   number of buffers required is 3 for AVB Rx link to capture without
     *   frame drops
     */

} AvbRxLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Init function for AVB Src link. This function does the following for
 *   AVB Src link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 AvbRxLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for AVB Rx link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 AvbRxLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Set default parameters for AVB Rx Link create time params
 *   This function does the following
 *      - memset create params object
 *      - Sets bufferNotificationType as AVBRX_LINK_BUF_NOTIFY_MJPEG_FRAME
 *      - Sets numCameras as AVB_RX_LINK_MAX_TALKERS_DEFAULT
 *      - Sets numBufs as AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT
 * \param  pPrm  [OUT]  avbRxLink Create time Params
 *
 *******************************************************************************
 */
static inline void AvbRxLink_CreateParams_Init(AvbRxLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
    pPrm->bufferNotificationType = AVB_RX_LINK_BUF_NOTIFY_MJPEG_FRAME;
    pPrm->numCameras = AVB_RX_LINK_MAX_TALKERS_DEFAULT;
    pPrm->numBufs = AVB_RX_LINK_NUM_BUFS_PER_TALKER_DEFAULT;
    return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
