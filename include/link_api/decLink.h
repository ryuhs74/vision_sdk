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
 * \ingroup FRAMEWORK_MODULE_API
 * \defgroup DECODE_LINK_API Video Decode Link API
 *
 * \brief  This module has the interface for using Decode Link
 *
 *         Decode Link integrated HDVICP codec, support multi channel decode.
 *         Video Decode Link can be used to decode a bitstream of different
 *         codec types such as
 *           - MJPEG
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file decLink.h
 *
 * \brief Decode Link API
 *
 * \version 0.0 (Jan 2013) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _DEC_LINK_H_
#define _DEC_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 *
 *   \brief Default value of maximum number of output queues the dec link
 *          supported
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DEC_LINK_MAX_OUT_QUE                  (1)

/**
 *******************************************************************************
 *
 *   \brief Max number of DEC channels per link supported
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DEC_LINK_MAX_CH                       (8)

/**
 *******************************************************************************
 *
 *   \brief Maximum number of output buffers supported per channel
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DEC_LINK_MAX_NUM_OUT_BUF_PER_CH       (16)

/**
 *******************************************************************************
 *
 *   \brief Default value for DPB size in frames
 *          This is valid only for H264 decode
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DEC_LINK_DPB_SIZE_IN_FRAMES_DEFAULT   (-1)

/**
 *******************************************************************************
 *
 *   \brief Default value for Number of buffers per channel request
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DEC_LINK_NUM_BUFFERS_PER_CHANNEL      (4)

/* @} */


/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup DEC_LINK_API_CMD DECODE Link Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Dec command to enable channel
 *          Application can use this command to enable an alreday
 *          disabled channel
 *
 *   SUPPORTED in ALL platforms
 *
 *   \param DecLink_ChannelInfo
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define DEC_LINK_CMD_ENABLE_CHANNEL           (0x2002)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Dec command to disable channel
 *          Application can use this command to disable an alreday
 *          enabled channel
 *
 *   SUPPORTED in ALL platforms
 *
 *   \param DecLink_ChannelInfo
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define DEC_LINK_CMD_DISABLE_CHANNEL          (0x2003)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Dec command to return buffer statistics
            Application can use this command to return the buffer statistics
            Mainly used for debuging purpose
 *
 *   SUPPORTED in ALL platforms
 *
 *   \param DecLink_BufferStats
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define DEC_LINK_CMD_GET_BUFFER_STATISTICS    (0x2004)

/* @} */


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the decode process type (I/IP/All)
 *
 *          This enum is used to request decoder to decode only I, IP or ALL
 *          frame types
 *******************************************************************************
 */
typedef enum
{
  DEC_LINK_DECODE_ALL = 0,
  /**<
   * Indicates that all type of frames decoding is enabled
   * Applicable only for H264 and MPEG4 decode types
   */
  DEC_LINK_DECODE_IP_ONLY = 1,
  /**<
   * Indicates that only I/IDR and P frames decoding is enabled
   * Applicable only for H264 decode type
   */
  DEC_LINK_DECODE_I_ONLY = 2
  /**<
   * Indicates that only I/IDR frames decoding is enabled
   * Applicable only for H264 and MPEG4 decode types
   */
} DecLink_ChDecodeFrameType;

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the decode process level (filed/frame)
 *
 *          This enum indicates whether process call is done at a field
 *          level or frame level
 *******************************************************************************
 */
typedef enum
{
  DEC_LINK_FIELDLEVELPROCESSCALL = 0,
  /**
  * Indicates that process call should be at field level
  */
  DEC_LINK_FRAMELEVELPROCESSCALL = 1
  /**
  * Indicates that process call should be at frame level
  */
} DecLink_ChProcessCallLevel;

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the decode channel codec create status
 *
 *          This enum indicates whether create channel really create the
 *          codec or only open the channel and neither codec instance
 *          will be craeted nor output buffers are allocated
 *          DEC_LINK_ALG_CREATE_STATUS_CREATE is the only option valiadted
 *******************************************************************************
 */
typedef enum
{
  DEC_LINK_ALG_CREATE_STATUS_DONOT_CREATE = 0,
  /**< Only the partial channel get created, neither codec instance
   *   nor the output buffers will get created.
   */
  DEC_LINK_ALG_CREATE_STATUS_CREATE = 1,
  /**< Fully functional channel will be created with Both codec
   *   instance and the output buffers created, reday to operate
   */
  DEC_LINK_ALG_CREATE_STATUS_CREATE_DONE = 2,
  /**< Not for Application use, used by Link internals */
  DEC_LINK_ALG_CREATE_STATUS_DELETE = 3
  /**< Not for Application use, used by Link internals */
} DecLink_ChDecodeAlgCreateStatus;


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Dec link channel info (channel number)
 *
 *          Defines the channel number of the DEC link for any channel
 *          specific dec link configutaion/settings
 *
 *******************************************************************************
 */
typedef struct DecLink_ChannelInfo
{
    UInt32 chId;
    /**< Decoder channel number */
} DecLink_ChannelInfo;

/**
 *******************************************************************************
 *
 *   \brief DecLink buffer statistics structure
 *
 *          Defines the structure returned to application having the
 *          buffer statistics of one channel for the decoder link.
 *
 *******************************************************************************
 */
typedef struct DecLink_ChBufferStats
{
    UInt32 numInBufQueCount;
    /**< Number of input buffers queued-in */
    UInt32 numOutBufQueCount;
    /**< Number of output buffers queued-in */
}DecLink_ChBufferStats;

/**
 *******************************************************************************
 *
 *   \brief DecLink buffer statistics structure
 *
 *          Defines the structure returned to application having the
 *          buffer statistics for a specific list of channels of the
 *          decoder link.
 *
 *******************************************************************************
 */
typedef struct DecLink_BufferStats
{
    UInt32  numCh;
    /**< Number of channels for buffer statistics requested */
    UInt32  chId[DEC_LINK_MAX_CH];
    /**< List of channels for which buffer statistics requested */
    DecLink_ChBufferStats stats[DEC_LINK_MAX_CH];
    /**< Buffer statistics of one channel for the decoder link */
} DecLink_BufferStats;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Decode link real time parameters
 *
 *          Defines those parameters that can be changed dynamically
 *          on a per channel basis for the decode link
 *
 *******************************************************************************
 */
typedef struct DecLink_ChDynamicParams
{
    Int32 targetFrameRate;
    /**< Target frame rate of the decoder channel */
    Int32 targetBitRate;
    /**< Target bitrate of the decoder channel */
} DecLink_ChDynamicParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Decode link create time channel parameters
 *
 *          Defines those parameters that can be configured/set during the
 *          create time on a per channel basis for the decode link
 *
 *******************************************************************************
 */
typedef struct DecLink_ChCreateParams
{
    UInt32 format;
    /**< Video Codec format/type */
    Int32 profile;
    /**< Video coding profile */
    Int32 targetMaxWidth;
    /**< Target frame width of the decoder */
    Int32 targetMaxHeight;
    /**< Target frame height of the decoder */
    Int32 displayDelay;
    /**< Max number of frames delayed by decoder */
    DecLink_ChDynamicParams defaultDynamicParams;
    /**< Default dynamic params for decoder */
    UInt32 processCallLevel;
    /**< Specifies if process call is done frame level or field level
     *   See DecLink_ChProcessCallLevel for supported values
     *   For progressive mode,processCallLevel = DEC_LINK_FRAMELEVELPROCESSCALL;
     *   For interlaced  mode,processCallLevel = DEC_LINK_FIELDLEVELPROCESSCALL;
     */
    UInt32 fieldMergeDecodeEnable;
    /**< This option is applicale only for field vise (interlaced) encoding.
     *   Enable this option if the same input bit stream buffer
     *   will have both TOP and BOTTOM field data
     */
    UInt32 numBufPerCh;
    /**< Number of buffer to allocate per channel-wise */
    Int32 dpbBufSizeInFrames;
    /**< Size of the decoder picture buffer.If application
     *   knows the max number of reference frames in the
     *   stream to be fed to the decoder, it can set this
     *   value to enable reduced memory consumption.
     *   Application should set this value to default
     *   if it doesn't care about this parameter
     */
     UInt32 algCreateStatus;
    /**< See DecLink_ChDecodeAlgCreateStatus for supported values
    *    App can only configure this variable with two below values
     *         DEC_LINK_ALG_CREATE_STATUS_DONOT_CREATE -> 0
     *         DEC_LINK_ALG_CREATE_STATUS_CREATE -> 1
     *   If 0: Only the partial channel get created, neither codec instance
     *         nor the output buffers will be get created. App will be
     *         able to make this channel fully functional dynamically by
     *         calling DEC_LINK_CMD_CREATE_CHANNEL API
     *   If 1: Fully functional channel will be created with Both codec
     *         instance and the output buffers created, reday to operate
     */
    UInt32 decodeFrameType;
    /**< see DecLink_ChDecodeFrameType for supported values
    *    decodeFrameType is a Flag to decoder from application to
     *   request decoding of only I & IDR or IP or all frame types.
     *   The intention of this parameter is to have create time
     *   indication to codec for lesser memory foot print request.
     */
    UInt32 enableFrameSkipDue2AccumuInNextLink;
    /**< enableFrameSkipDue2AccumuInNextLink is a Flag to decoder
     *   from application to enable frame skip of the output frames
     *   immediately after decode and not send to next link if all
     *   output bufs are alreday queueed in the input side of next link.
     */
    UInt32 extractJpegAppMarker;
    /**< TRUE: Extract JPEG APP marker data and place in System_Buffer
     *          as metadata
     *  FALSE: Dont extract JPEG App Marker
     */

} DecLink_ChCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Decode link create time parameters
 *
 *          This structure is used to create and configure a Decode link
 *          instance.
 *
 *******************************************************************************
 */
typedef struct DecLink_CreateParams
{
    System_LinkInQueParams      inQueParams;
    /**< Input queue information */
    System_LinkOutQueParams     outQueParams;
    /**< Output queue information */
    DecLink_ChCreateParams      chCreateParams[DEC_LINK_MAX_CH];
    /**< Decoder link channel create params */
} DecLink_CreateParams;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Decode link register and init function
 *
 *          For each Decode instance
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 DecLink_init();

/**
 *******************************************************************************
 *
 *   \brief Decode link de-register and de-init function
 *
 *          For each Decode instance
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 DecLink_deInit();

/**
 *******************************************************************************
 *
 *   \brief Function to initialize the Decode Link Create Params
 *
 *          Sets default values for Decode link create time
 *          link & channel parameters.
 *          User/App can override these default values later.
 *
 *   \param pPrm [OUT] Default information
 *
 *   \return void
 *
 *******************************************************************************
 */
static inline void DecLink_CreateParams_Init(DecLink_CreateParams *pPrm)
{
    UInt32 i;

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->outQueParams.nextLink = SYSTEM_LINK_ID_INVALID;

    /* when set 0, decoder will take default value based on system
       defined default on BIOS side */
    for (i=0; i<DEC_LINK_MAX_CH;i++)
    {
        pPrm->chCreateParams[i].dpbBufSizeInFrames =
                                DEC_LINK_DPB_SIZE_IN_FRAMES_DEFAULT;
        pPrm->chCreateParams[i].numBufPerCh = 0;
        pPrm->chCreateParams[i].displayDelay = 0;
        pPrm->chCreateParams[i].algCreateStatus =
                                DEC_LINK_ALG_CREATE_STATUS_CREATE;
        pPrm->chCreateParams[i].decodeFrameType = DEC_LINK_DECODE_ALL;
        pPrm->chCreateParams[i].enableFrameSkipDue2AccumuInNextLink = FALSE;
        pPrm->chCreateParams[i].processCallLevel=
                                DEC_LINK_FRAMELEVELPROCESSCALL;
        pPrm->chCreateParams[i].fieldMergeDecodeEnable = FALSE;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
