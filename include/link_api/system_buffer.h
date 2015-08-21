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
 *  \ingroup SYSTEM_LINK_INTER_LINK_API
 *
 *  @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \file system_buffer.h
 *
 *  \brief System Buffer Data Structure definition
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_BUFFER_H_
#define _SYSTEM_BUFFER_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Possible fields inside of System_BitstreamBuffer.flags
 *
 *   @{
 *
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *  \brief Type of bitstream
 *         Refer to System_BitstreamCodingType for values
 *
 *         Encoded in bit-field (0x000000FF)
 *
 *******************************************************************************
 */
#define SYSTEM_BITSTREAM_BUFFER_FLAG_GET_BITSTREAM_FORMAT(container)      \
            SystemUtils_unpack_bitfield(container, 0x000000FF, 0)

#define SYSTEM_BITSTREAM_BUFFER_FLAG_SET_BITSTREAM_FORMAT(container, value)\
           (container = SystemUtils_pack_bitfield((container),            \
                                                   (value),               \
                                                   0x000000FF,            \
                                                   0))

/**
 *******************************************************************************
 *
 *  \brief Key frame identifier
 *
 *         Encoded in bit-field (0x00000100)
 *
 *         0: Not a key-frame
 *         1: Key-frame, ex, I-frame
 *
 *******************************************************************************
 */
#define SYSTEM_BITSTREAM_BUFFER_FLAG_GET_IS_KEYFRAME(container)           \
            SystemUtils_unpack_bitfield(container, 0x00000100, 8)

#define SYSTEM_BITSTREAM_BUFFER_FLAG_SET_IS_KEYFRAME(container, value)    \
           (container = SystemUtils_pack_bitfield((container),            \
                                                   (value),               \
                                                   0x00000100,            \
                                                   8))

/**
 *******************************************************************************
 *
 *  \brief Flush Buffer identifier
 *
 *         Encoded in bit-field (0x00000200)
 *
 *         0: No Flush buffer identifier
 *         1: Flush buffer identifier
 *
 *******************************************************************************
 */
#define SYSTEM_BITSTREAM_BUFFER_FLAG_GET_IS_FLUSHBUF(container)           \
            SystemUtils_unpack_bitfield(container, 0x00000200, 9)

#define SYSTEM_BITSTREAM_BUFFER_FLAG_SET_IS_FLUSHBUF(container, value)    \
           (container = SystemUtils_pack_bitfield((container),            \
                                                   (value),               \
                                                   0x00000200,            \
                                                   9))

/* @} */

/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Bitstream buffer payload for System_Buffer
 *
 *******************************************************************************
 */
typedef struct {

    Void   *bufAddr;
    /**< when NULL bitstream data is not present,
     *      meta data could still be present */

    UInt32  bufSize;
    /**< Total buffer size */

    UInt32  fillLength;
    /**< size of valid data inside the buffer */

    Void   *metaBufAddr;
    /**< Metadata buffer address when NULL meta data is not present */

    UInt32  metaBufSize;
    /**< meta buffer size */

    UInt32  metaFillLength;
    /**< size of valid data inside the meta data buffer  */

    UInt32  flags;
    /**< see SYSTEM_BITSTREAM_BUFFER_FLAG_* */

    UInt32  width;
    /**< Width of frame encoded inside the bitstream */

    UInt32  height;
    /**< Height of frame encoded inside the bitstream */

} System_BitstreamBuffer;


/**
 *******************************************************************************
 *
 *  \brief Possible fields inside of System_VideoFrameBuffer.flags
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Field ID for interlaced video, set to 0 for progressive video
 *
 *         Encoded in bit-field (0x00000001)
 *
 *         0: even field
 *         1: odd field
 *
 *******************************************************************************
 */
#define SYSTEM_VIDEO_FRAME_GET_FLAG_FID(container)                        \
            SystemUtils_unpack_bitfield(container, 0x00000001, 0)

#define SYSTEM_VIDEO_FRAME_SET_FLAG_FID(container, value)                 \
           (container = SystemUtils_pack_bitfield((container),            \
                                                   (value),               \
                                                   0x00000001,            \
                                                   0))

/* @} */

/**
 *******************************************************************************
 *
 *  \brief Structure to hold video frame
 *
 *******************************************************************************
*/
typedef struct {

    Void              *bufAddr[SYSTEM_MAX_PLANES];
    /**< when NULL video frame data is not present,
     *   meta data could still be present
     *
     *   Index 0: Y-plane for YUV420 data format
     *   Index 1: UV-plane for YUV420 data format
     *
     *   Index 0: YUV422i, RGB565, RGB888 etc are all represented
     *           in a single plane */

    Void              *metaBufAddr;
    /**< when NULL meta data is not present */

    UInt32            metaBufSize;
    /**< Total buffer size */

    UInt32            metaFillLength;
    /**< size of valid data inside the buffer  */

    System_LinkChInfo chInfo;
    /**< Channel specific information */

    UInt32            flags;
    /**< flags here convey certain properties of the buffer, which typically
     * may change with each frame. Refer macros above to set / get values
     * into this flags.
     * flags present in System_LinkChInfo will not typically change for
     * every frame */

} System_VideoFrameBuffer;

/**
 *******************************************************************************
 *
 * \brief Max Meta data planes
 *
 *******************************************************************************
 */
#define SYSTEM_MAX_META_DATA_PLANES   (4)

/**
*******************************************************************************
*
*  \brief Meta data buffer payload for System_Buffer
*
*******************************************************************************
*/
typedef struct {

    UInt32 numMetaDataPlanes;
    /**< Number of valid Meta-data planes */

    Void   *bufAddr[SYSTEM_MAX_META_DATA_PLANES];
    /**< Metadata buffer address when NULL meta data is not present */

    UInt32  metaBufSize[SYSTEM_MAX_META_DATA_PLANES];
    /**< meta buffer size */

    UInt32  metaFillLength[SYSTEM_MAX_META_DATA_PLANES];
    /**< size of valid data inside the meta data buffer  */

    UInt32  flags;
    /**< see SYSTEM_METADATA_BUFFER_FLAG_* */

} System_MetaDataBuffer;

/**
 *******************************************************************************
 *
 * \brief Max frames that can be present in the composite video frame payload
 *
 *******************************************************************************
 */
#define SYSTEM_MAX_FRAMES_IN_COMPOSITE_BUFFER   (6)

/**
 *******************************************************************************
 *
 * \brief Composite video frame payload in System_Buffer
 *
 *******************************************************************************
 */
typedef struct {

    UInt32  numFrames;
    /**< Number of frames in this composite buffer */

    Void   *bufAddr[SYSTEM_MAX_PLANES][SYSTEM_MAX_FRAMES_IN_COMPOSITE_BUFFER];
    /**< Video frame address */

    Void   *metaBufAddr[SYSTEM_MAX_FRAMES_IN_COMPOSITE_BUFFER];
    /**< when NULL meta data is not present */

    UInt32  metaBufSize;
    /**< Total buffer size */

    UInt32  metaFillLength;
    /**< size of valid data inside the buffer  */

    System_LinkChInfo chInfo;
    /**<
     *   Channel information
     *
     *   Assumes that chInfo and meta data size and meta data buffer
     *   size is same for all frames in composite buffer */

    UInt32            flags;
    /**< NOT used - for future use */

} System_VideoFrameCompositeBuffer;


/**
 *******************************************************************************
 *
 * \brief System buffer information
 *
 *        A pointer to this structure is exchange between different links.
 *        This can have different payloads for different types of data.
 *
 *******************************************************************************
 */
typedef struct System_Buffer {

    System_BufferType     bufType;
    /**< Buffer type, this is used to identify
     *   the payload type associated with this buffer */

    UInt32                chNum;
    /**< Channel associated with this buffer. MUST be < 255 */

    UInt64                srcTimestamp;
    /**< Timestamp updated at the source. This is the timestamp at which the
     *   source buffer was available, in units of usec's
     */
    UInt64                linkLocalTimestamp;
    /**< Timestamp at which the buffer was available at the input of the local
     *   link, in units of usec's
     */

    UInt32                payloadSize;
    /**< Payload size in bytes, MUST be < 4KB */

    Void                 *payload;
    /**< Pointer to payload data */

    /** below fields are used by specific links internally
     *   and other links should not modify their values */

    UInt32                selectOrgChNum;
    /**< used by select link, keeps
     *   track of original channel num before the select operation */

    struct System_Buffer *pDupOrgFrame;
     /**< used by DUP link
     * Pointer to original System_buffer in case this is a dup of original
     * buffer */

    UInt32                dupCount;
    /**< Used by DUP link
     *   Dup count in case the original system_buffer is duplicated  using
     *   dup link */

    struct System_Buffer *pSplitOrgFrame;
     /**< used by SPLIT link
     * Pointer to original System_buffer in case this is a split of original
     * buffer */

    UInt32                splitCount;
    /**< Used by SPLIT link
     *   Split count in case the original system_buffer is duplicated  using
     *   dup link */

    UInt32                ipcInOrgQueElem;
    /**< Used by IPC IN link to keep track of original IPC queue element */

    Void                 *pCaptureOrgBufferPtr;
    /**< Used by capture link to hold driver/algorithm specific private info */

    Void                 *pSyncLinkOrgBufferPtr;
    /**< Used by Sync Link to keep track of the original buffers */

    Void                 *pVpeLinkPrivate;
    /**< Used by VPE link to hold driver/algorithm specific private info */

    Void                 *pEncDecLinkPrivate;
    /**< Used by VDEC link to hold driver/algorithm specific private info */

    UInt64                ipcPrfTimestamp64[2];
    /**< Used to measure IPC overheads */

} System_Buffer;

#endif

/* @} */
