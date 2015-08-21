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
 * \ingroup SYSTEM_LINK_API
 *
 *
 * @{
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file   system_link_info.h
 *
 * \brief  System CH Information Data structure
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_CH_INFO_H_
#define _SYSTEM_CH_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

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
 * \brief Possible fields inside of System_LinkChInfo.flags
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Utility macros to get and set the bit-fields in 32-bit value.
 *
 *******************************************************************************
 */
#define SystemUtils_unpack_bitfield(container, mask, offset)              \
                   ((container & mask) >> offset)
#define SystemUtils_pack_bitfield(container, value, mask, offset)         \
                   (((container) & ~(mask)) | (((value) << (offset)) & (mask)))

/**
 *******************************************************************************
 *
 * \brief Utility macros to get/set buffer type in flag field of buffer
 *
 *        Refer Enum System_BufferType for different possible values
 *
 *        Encoded in bit-field (0x0000000F)
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_CH_INFO_GET_FLAG_BUF_TYPE(container)                  \
            SystemUtils_unpack_bitfield(container, 0x0000000F, 0)

#define SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(container, value)           \
           (container = SystemUtils_pack_bitfield((container),            \
                                                   (value),               \
                                                   0x0000000F,            \
                                                   0))
/**
 *******************************************************************************
 *
 *  \brief Utility macro to get/set the coding type in bit stream buffers.
 *
 *         Refer Enum System_BitstreamCodingType for different possible values
 *
 *         Encoded in bit-field (0x000000F0)
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_CH_INFO_GET_FLAG_BITSTREAM_FORMAT(container)          \
            SystemUtils_unpack_bitfield(container, 0x000000F0, 4)

#define SYSTEM_LINK_CH_INFO_SET_FLAG_BITSTREAM_FORMAT(container, value)   \
           (container = SystemUtils_pack_bitfield((container),            \
                                                  (value),                \
                                                  0x000000F0,             \
                                                  4))
/**
 *******************************************************************************
 *
 * \brief Utility function to get/set the scan format in buffer
 *
 *        Refer Enum System_VideoScanFormat for values
 *
 *        Encoded in bit-field (0x00000100)
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_CH_INFO_GET_FLAG_SCAN_FORMAT(container)               \
            SystemUtils_unpack_bitfield(container, 0x00000100, 8)

#define SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(container, value)        \
           (container = SystemUtils_pack_bitfield((container),            \
                                                  (value),                \
                                                  0x00000100,             \
                                                  8))
/**
 *******************************************************************************
 *
 *  \brief Utility macro to set/get the memtype fields from buffer.
 *
 *         Refer Enum System_MemoryType for values
 *
 *         Encoded in bit-field (0x00000200)
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_CH_INFO_GET_FLAG_MEM_TYPE(container)                  \
            SystemUtils_unpack_bitfield(container, 0x00000200, 9)

#define SYSTEM_LINK_CH_INFO_SET_FLAG_MEM_TYPE(container, value)           \
           (container = SystemUtils_pack_bitfield((container),            \
                                                  (value),                \
                                                  0x00000200,             \
                                                  9))

/**
 *******************************************************************************
 *
 *   \brief Macro to set/get run-time params fields in buffer
 *
 *         1: params changed, do run time parameter update
 *         0: No parameter update
 *
 *         Ignored during create time
 *
 *         Encoded in bit-field (0x00000400)
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_CH_INFO_GET_FLAG_IS_RT_PRM_UPDATE(container)          \
            SystemUtils_unpack_bitfield(container, 0x00000400, 10)

#define SYSTEM_LINK_CH_INFO_SET_FLAG_IS_RT_PRM_UPDATE(container, value)   \
           (container = SystemUtils_pack_bitfield((container),            \
                                                  (value),                \
                                                  0x00000400,             \
                                                  10))

/**
 *******************************************************************************
 *
 *  \brief Macro to update data format in system_buffer flag field
 *
 *         Refer Enum System_VideoDataFormat for different possible values
 *
 *         Encoded in bit-field (0x0FFFF000)
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(container)               \
            SystemUtils_unpack_bitfield((container), 0x0FFFF000, 12)

#define SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(container, value)        \
           (container = SystemUtils_pack_bitfield((container),            \
                                                  (value),                \
                                                  0x0FFFF000,             \
                                                  12))

/* @} */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Channel specific info
 *
 *         Used by a link to set itself up based on channel info of previous
 *         link
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32 flags;
    /**< Refer macros above to set / get values into / from flags */
    UInt32 pitch[SYSTEM_MAX_PLANES];
    /**< Pitch for various formats / planes */
    UInt32 startX;
    /**< Start x position */
    UInt32 startY;
    /**< Start y position */
    UInt32 width;
    /**< channel resolution - width */
    UInt32 height;
    /**< channel resolution - height */
} System_LinkChInfo;

/**
 *******************************************************************************
 *
 *  \brief  In queue params
 *
 *          This structure contains input queue parameters from which a
 *          link gets it's data
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 prevLinkId;
    /**< Previous link ID to which current link will be connected */

    UInt32 prevLinkQueId;
    /**< Previous link Que ID, with which current link
     *   will exchange frames */
} System_LinkInQueParams;

/**
 *******************************************************************************
 *
 * \brief  Out queue params
 *
 *         This structure contains output queue parameters to which a
 *         link is connected to.
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 nextLink;
    /**< Next link ID to which current link will be connected */
} System_LinkOutQueParams;

/**
 *******************************************************************************
 *
 *  \brief LINKs output queue information
 *         Specifies a place holder that describe the output information
 *         of the LINK
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32              numCh;
    /**< No of channel that would be sent out */
    System_LinkChInfo   chInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Each channels configurations */

} System_LinkQueInfo;

/**
 *******************************************************************************
 *
 * \brief LINKs information
 *        Specifies a place holder that describe the LINK information
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32              numQue;
    /**< Number of output queue that a LINK supports */
    System_LinkQueInfo  queInfo[SYSTEM_MAX_OUT_QUE];
    /**< Each queue configurations */
} System_LinkInfo;

#ifdef  __cplusplus
}
#endif

#endif

/*@}*/
