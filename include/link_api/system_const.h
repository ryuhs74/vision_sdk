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
 *  \ingroup SYSTEM_LINK_API
 *  \defgroup SYSTEM_CONST_API System constants
 *
 *  \brief This module lists the system wide common constants and enums
 *
 * @{
 *
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file system_const.h
 *
 * \brief System wide constants and enums.
 *
 * \version 0.0 (Jun 2013) : [HS] First version
 * \version 0.1 (Jul 2013) : [HS] Updates as per code review comments
 * \version 0.2 (Mar 2015) : [YM] Added SYSTEM_LINK_STATUS_EUNBLOCK
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_CONST_H_
#define _SYSTEM_CONST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 *
 * \brief Buffer alignment required across all links
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SYSTEM_BUFFER_ALIGNMENT         (16u)

/**
 *******************************************************************************
 *
 * \brief Maximum number of output queues
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SYSTEM_MAX_OUT_QUE              (6u)

/**
 *******************************************************************************
 *
 * \brief Maximum number of channels per output queue
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SYSTEM_MAX_CH_PER_OUT_QUE       (8u)

/**
 *******************************************************************************
 *
 * \brief Maximum number of planes with buffer
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SYSTEM_MAX_PLANES               (3u)


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
 * \brief Enum for the returns status of the link API.
 *
 *  Link API returns error codes based on failure scenario or success
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_LINK_STATUS_SOK = 0x0,
    /** \brief LINK API call successful. */

    SYSTEM_LINK_STATUS_EFAIL = -1,
    /**< LINK API call returned for general failure. Where failure
     *  code doesnt fall into any of the below category
     */

    SYSTEM_LINK_STATUS_ETIMEOUT = -2,
    /**< LINK API call returned with error as timed out. Typically API is
     *  waiting for some condition and returned as condition not happened
     *  in the timeout period. */

    SYSTEM_LINK_STATUS_EALLOC = -3,
    /**< LINK API call returned with error as allocation failure. Typically
     *  memory or resource allocation failure. */

    SYSTEM_LINK_STATUS_EAGAIN = -4,
    /**< LINK API call returned with error as try again. Momentarily API is
     *  not able to service request because of queue full or any other temporary
     *  reason. */

    SYSTEM_LINK_STATUS_EUNSUPPORTED_CMD = -5,
    /**< LINK API call returned with unsupported command. Typically when
     *  command is not supported by control API. */

    SYSTEM_LINK_STATUS_ENO_MORE_BUFFERS = -6,
    /**< LINK API call returned with error as no more buffers available.
     *  Typically when no buffers are available. */

    SYSTEM_LINK_STATUS_EINVALID_PARAMS = -7,
    /**< LINK API call returned with error as invalid parameters. Typically
     *  when parameters passed are not valid or out of range. */

    SYSTEM_LINK_STATUS_EUNBLOCK = -8,
    /**< Status used to force unblock the blocking thread.
      *  used by rpmsg based msgq with host communication */

    SYSTEM_LINK_STATUS_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}System_LinkApiReturnCodes;

/**
 *******************************************************************************
 * \brief Enums for data format.
 *
 *  All data formats may not be supported by all links. For supported data
 *  formats please look link header file.
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_DF_YUV422I_UYVY = 0x0000,
    /**< YUV 422 Interleaved format - UYVY. */
    SYSTEM_DF_YUV422I_YUYV,
    /**< YUV 422 Interleaved format - YUYV. */
    SYSTEM_DF_YUV422I_YVYU,
    /**< YUV 422 Interleaved format - YVYU. */
    SYSTEM_DF_YUV422I_VYUY,
    /**< YUV 422 Interleaved format - VYUY. */
    SYSTEM_DF_YUV422SP_UV,
    /**< YUV 422 Semi-Planar - Y separate, UV interleaved. */
    SYSTEM_DF_YUV422SP_VU,
    /**< YUV 422 Semi-Planar - Y separate, VU interleaved. */
    SYSTEM_DF_YUV422P,
    /**< YUV 422 Planar - Y, U and V separate. */
    SYSTEM_DF_YUV420SP_UV,
    /**< YUV 420 Semi-Planar - Y separate, UV interleaved. */
    SYSTEM_DF_YUV420SP_VU,
    /**< YUV 420 Semi-Planar - Y separate, VU interleaved. */
    SYSTEM_DF_YUV420P,
    /**< YUV 420 Planar - Y, U and V separate. */
    SYSTEM_DF_YUV444P,
    /**< YUV 444 Planar - Y, U and V separate. */
    SYSTEM_DF_YUV444I,
    /**< YUV 444 interleaved - YUVYUV... */
    SYSTEM_DF_RGB16_565,
    /**< RGB565 16-bit - 5-bits R, 6-bits G, 5-bits B. */
    SYSTEM_DF_ARGB16_1555,
    /**< ARGB1555 16-bit - 5-bits R, 5-bits G, 5-bits B, 1-bit Alpha (MSB). */
    SYSTEM_DF_RGBA16_5551,
    /**< RGBA5551 16-bit - 5-bits R, 5-bits G, 5-bits B, 1-bit Alpha (LSB). */
    SYSTEM_DF_ARGB16_4444,
    /**< ARGB4444 16-bit - 4-bits R, 4-bits G, 4-bits B, 4-bit Alpha (MSB). */
    SYSTEM_DF_RGBA16_4444,
    /**< RGBA4444 16-bit - 4-bits R, 4-bits G, 4-bits B, 4-bit Alpha (LSB). */
    SYSTEM_DF_RGBX16_4444,
        /**< RGBX4444 16-bit - 4-bits R, 4-bits G, 4-bits B, 4-bit Unused  . */
    SYSTEM_DF_ARGB24_6666,
    /**< ARGB6666 24-bit - 6-bits R, 6-bits G, 6-bits B, 6-bit Alpha (MSB). */
    SYSTEM_DF_RGBA24_6666,
    /**< RGBA6666 24-bit - 6-bits R, 6-bits G, 6-bits B, 6-bit Alpha (LSB). */
    SYSTEM_DF_RGB24_888,
    /**< RGB24 24-bit - 8-bits R, 8-bits G, 8-bits B. */
    SYSTEM_DF_BGRX_4444,
    /**<RGBx12-16bit- 4-bits R, 4-bits G, 4-bits B, 4-bits unused(LSB).*/
    SYSTEM_DF_XBGR_4444,
    /**<xRGB12-16bit- 4-bits R, 4-bits G, 4-bits B, 4-bits unused(MSB).*/
    SYSTEM_DF_ARGB32_8888,
    /**< ARGB32 32-bit - 8-bits R, 8-bits G, 8-bits B, 8-bit Alpha (MSB). */
    SYSTEM_DF_XRGB32_8888,
    /**< XRGB32 32-bit - 8-bits R, 8-bits G, 8-bits B, 8-bit unused . */
    SYSTEM_DF_RGBA32_8888,
    /**< RGBA32 32-bit - 8-bits R, 8-bits G, 8-bits B, 8-bit Alpha (LSB). */
    SYSTEM_DF_BGR16_565,
    /**< BGR565 16-bit -   5-bits B, 6-bits G, 5-bits R. */
    SYSTEM_DF_ABGR16_1555,
    /**< ABGR1555 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit Alpha (MSB). */
    SYSTEM_DF_ABGR16_4444,
    /**< ABGR4444 16-bit - 4-bits B, 4-bits G, 4-bits R, 4-bit Alpha (MSB). */
    SYSTEM_DF_BGRA16_5551,
    /**< BGRA5551 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit Alpha (LSB). */
    SYSTEM_DF_BGRA16_4444,
    /**< BGRA4444 16-bit - 4-bits B, 4-bits G, 4-bits R, 4-bit Alpha (LSB). */
    SYSTEM_DF_AGBR16_1555,
    /**< ABGR1555 16-bit - 5-bits G, 5-bits B, 5-bits R, 1-bit Alpha (MSB). */
    SYSTEM_DF_AGBR16_4444,
    /**< ABGR4444 16-bit - 4-bits G, 4-bits B, 4-bits R, 4-bit Alpha (MSB). */
    SYSTEM_DF_XGBR16_1555,
    /**< XGBR1555 16-bit - 5-bits G, 5-bits B, 5-bits R, 1-bit unused (MSB). */
    SYSTEM_DF_BGRX16_5551,
    /**< BGRX5551 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit unused (MSB). */
    SYSTEM_DF_ABGR24_6666,
    /**< ABGR6666 24-bit - 6-bits B, 6-bits G, 6-bits R, 6-bit Alpha (MSB). */
    SYSTEM_DF_BGR24_888,
    /**< BGR888 24-bit - 8-bits B, 8-bits G, 8-bits R. */
    SYSTEM_DF_XBGR24_8888,
    /**< xBGR888 24-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit unused(MSB) */
    SYSTEM_DF_RGBX24_8888,
    /**< xBGR888 24-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit unused(LSB) */
    SYSTEM_DF_BGRX24_8888,
    /**< xBGR888 24-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit unused(MSB) */
    SYSTEM_DF_ABGR32_8888,
    /**< ABGR8888 32-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit Alpha (MSB). */
    SYSTEM_DF_BGRA24_6666,
    /**< BGRA6666 24-bit - 6-bits B, 6-bits G, 6-bits R, 6-bit Alpha (LSB). */
    SYSTEM_DF_BGRA32_8888,
    /**< BGRA8888 32-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit Alpha  . */
    SYSTEM_DF_BGRX32_8888,
    /**< BGRX8888 32-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit unused. */
    SYSTEM_DF_BGRA16_1555,
    /**< BGRA1555 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit Alpha. */
    SYSTEM_DF_BGRX16_1555,
    /**< BGRX1555 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit unused. */
    SYSTEM_DF_BGRA32_1010102,
    /**< BGRA1010102 32-bit - 10-bits B, 10-bits G, 10-bits R, 2-bit Alpha.*/
    SYSTEM_DF_BGRX32_1010102,
    /**< BGRX1010102 32-bit - 10-bits B, 10-bits G, 10-bits R, 2-bit unused.*/
    SYSTEM_DF_RGBA32_1010102,
    /**< RGBA1010102 32-bit - 10-bits B, 10-bits G, 10-bits R, 2-bit Alpha.*/
    SYSTEM_DF_RGBX32_1010102,
    /**< RGBX1010102 32-bit - 10-bits B, 10-bits G, 10-bits R, 2-bit unused.*/
    SYSTEM_DF_BGRA64_16161616,
    /**< RGBA16161616 64-bit - 16-bits B, 16-bits G, 16-bits R, 16-bit Alpha.*/
    SYSTEM_DF_BGRX64_16161616,
    /**< BGRX16161616 64-bit - 16-bits B, 16-bits G, 16-bits R, 16-bit unused.*/
    SYSTEM_DF_ABGR64_16161616,
    /**< ABGR16161616 64-bit - 16-bits B, 16-bits G, 16-bits R, 16-bit Alpha.*/
    SYSTEM_DF_XBGR64_16161616,
    /**< XBGR16161616 64-bit - 16-bits B, 16-bits G, 16-bits R, 16-bit unused.*/
    SYSTEM_DF_BITMAP8,
    /**< BITMAP 8bpp. */
    SYSTEM_DF_BITMAP4_LOWER,
    /**< BITMAP 4bpp lower address in CLUT. */
    SYSTEM_DF_BITMAP4_UPPER,
    /**< BITMAP 4bpp upper address in CLUT. */
    SYSTEM_DF_BITMAP2_OFFSET0,
    /**< BITMAP 2bpp offset 0 in CLUT. */
    SYSTEM_DF_BITMAP2_OFFSET1,
    /**< BITMAP 2bpp offset 1 in CLUT. */
    SYSTEM_DF_BITMAP2_OFFSET2,
    /**< BITMAP 2bpp offset 2 in CLUT. */
    SYSTEM_DF_BITMAP2_OFFSET3,
    /**< BITMAP 2bpp offset 3 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET0,
    /**< BITMAP 1bpp offset 0 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET1,
    /**< BITMAP 1bpp offset 1 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET2,
    /**< BITMAP 1bpp offset 2 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET3,
    /**< BITMAP 1bpp offset 3 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET4,
    /**< BITMAP 1bpp offset 4 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET5,
    /**< BITMAP 1bpp offset 5 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET6,
    /**< BITMAP 1bpp offset 6 in CLUT. */
    SYSTEM_DF_BITMAP1_OFFSET7,
    /**< BITMAP 1bpp offset 7 in CLUT. */
    SYSTEM_DF_BITMAP8_BGRA32,
    /**< BITMAP 8bpp BGRA32. */
    SYSTEM_DF_BITMAP4_BGRA32_LOWER,
    /**< BITMAP 4bpp BGRA32 lower address in CLUT. */
    SYSTEM_DF_BITMAP4_BGRA32_UPPER,
    /**< BITMAP 4bpp BGRA32 upper address in CLUT. */
    SYSTEM_DF_BITMAP2_BGRA32_OFFSET0,
    /**< BITMAP 2bpp BGRA32 offset 0 in CLUT. */
    SYSTEM_DF_BITMAP2_BGRA32_OFFSET1,
    /**< BITMAP 2bpp BGRA32 offset 1 in CLUT. */
    SYSTEM_DF_BITMAP2_BGRA32_OFFSET2,
    /**< BITMAP 2bpp BGRA32 offset 2 in CLUT. */
    SYSTEM_DF_BITMAP2_BGRA32_OFFSET3,
    /**< BITMAP 2bpp BGRA32 offset 3 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET0,
    /**< BITMAP 1bpp BGRA32 offset 0 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET1,
    /**< BITMAP 1bpp BGRA32 offset 1 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET2,
    /**< BITMAP 1bpp BGRA32 offset 2 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET3,
    /**< BITMAP 1bpp BGRA32 offset 3 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET4,
    /**< BITMAP 1bpp BGRA32 offset 4 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET5,
    /**< BITMAP 1bpp BGRA32 offset 5 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET6,
    /**< BITMAP 1bpp BGRA32 offset 6 in CLUT. */
    SYSTEM_DF_BITMAP1_BGRA32_OFFSET7,
    /**< BITMAP 1bpp BGRA32 offset 7 in CLUT. */
    SYSTEM_DF_BAYER_RAW,
    /**< Bayer pattern. */
    SYSTEM_DF_BAYER_GRBG,
    /** < Raw bayer data color pattern
     * G R G R ...
     * B G B G ...
     */
    SYSTEM_DF_BAYER_RGGB,
    /** < Raw bayer data color pattern
     * R G G R ...
     * G B G B ...
     */
    SYSTEM_DF_BAYER_BGGR,
    /** < Raw bayer data color pattern
     * B G B G ...
     * G B G B ...
     */
    SYSTEM_DF_BAYER_GBRG,
    /** < Raw bayer data color pattern
     * G B G B ...
     * R R R G ...
     */
    SYSTEM_DF_RAW_VBI,
    /**< Raw VBI data. */
    SYSTEM_DF_RAW24,
    /**< 24 bit raw-data. */
    SYSTEM_DF_RAW16,
    /**< 16 bit raw-data. */
    SYSTEM_DF_RAW08,
    /**< 8 bit raw-data. */
    SYSTEM_DF_MISC,
    /**< For future purpose. */
    SYSTEM_DF_BITMAP4 = SYSTEM_DF_BITMAP4_LOWER,
    /** BITMAP 4bpp. */
    SYSTEM_DF_BITMAP2 = SYSTEM_DF_BITMAP2_OFFSET0,
    /** BITMAP 2bpp. */
    SYSTEM_DF_BITMAP1 = SYSTEM_DF_BITMAP1_OFFSET0,
    /** BITMAP 1bpp. */
    SYSTEM_DF_RAW06 = 0x5E,
    /**< 6 bit raw-data. */
    SYSTEM_DF_RAW07,
    /**< 7 bit raw-data. */
    SYSTEM_DF_RAW10,
    /**< 10 bit raw-data. */
    SYSTEM_DF_RAW12,
    /**< 12 bit raw-data. */
    SYSTEM_DF_RAW14,
    /**< 14 bit raw-data. */
    SYSTEM_DF_JPEG1_INTERCHANGE,
    /**< JPEG INTERCHANGE data. */
    SYSTEM_DF_JPEG2_JFIF,
    /**< JPEG2 JFIF data. */
    SYSTEM_DF_JPEG3_EXIF,
    /**< JPEG3 EXIF data. */
    SYSTEM_DF_DPCM_10_8_10_PRED1,
    /**< DPCM 10-8-10 PRED1 data. */
    SYSTEM_DF_DPCM_10_8_10_PRED2,
    /**< DPCM 10-8-10 PRED2 data. */
    SYSTEM_DF_DPCM_10_7_10_PRED1,
    /**< DPCM 10-7-10 PRED1 data. */
    SYSTEM_DF_DPCM_10_7_10_PRED2,
    /**< DPCM 10-7-10 PRED2 data. */
    SYSTEM_DF_DPCM_10_6_10_PRED1,
    /**< DPCM 10-6-10 PRED1 data. */
    SYSTEM_DF_DPCM_10_6_10_PRED2,
    /**< DPCM 10-6-10 PRED2 data. */
    SYSTEM_DF_DPCM_12_8_10_PRED1,
    /**< DPCM 12-8-10 PRED1 data. */
    SYSTEM_DF_DPCM_12_8_10_PRED2,
    /**< DPCM 12-8-10 PRED2 data. */
    SYSTEM_DF_DPCM_12_7_10_PRED1,
    /**< DPCM 12-7-10 PRED1 data. */
    SYSTEM_DF_DPCM_12_7_10_PRED2,
    /**< DPCM 12-7-10 PRED2 data. */
    SYSTEM_DF_DPCM_12_6_10_PRED1,
    /**< DPCM 12-6-10 PRED1 data. */
    SYSTEM_DF_DPCM_12_6_10_PRED2,
    /**< DPCM 12-6-10 PRED2 data. */
    SYSTEM_DF_BGR16_565_A8,
    /**< BGR565 16-bit - 5-bits B, 6-bits G, 5-bits R.
     *Alpha 8 -bits another plane */
    SYSTEM_DF_RGB16_565_A8,
    /**< RGB565 16-bit - 5-bits R, 6-bits G, 5-bits B.
     *Alpha 8 -bits another plane */
    SYSTEM_DF_INVALID,
    /**< Invalid data format. Could be used to initialize variables. */
    SYSTEM_DF_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
}System_VideoDataFormat;


/**
 *******************************************************************************
 * \brief Buffer formats enums.
 *
 * Is Buffer a field or frame in memory
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_BUF_FMT_FIELD = 0,
    /**< Buffers are captured/displayed as fields instead of frames */
    SYSTEM_BUF_FMT_FRAME,
    /**< Buffers are captured/displayed as frames instead of frames */
    SYSTEM_BUF_FMT_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_BufferFormat;

/**
 *******************************************************************************
 * \brief Bits per pixel for data formats.
 *
 * Bits per pixel for data formats.
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_BPP_BITS1 = 0,
    /**< 1 Bits per Pixel. */
    SYSTEM_BPP_BITS2,
    /**< 2 Bits per Pixel. */
    SYSTEM_BPP_BITS4,
    /**< 4 Bits per Pixel. */
    SYSTEM_BPP_BITS8,
    /**< 8 Bits per Pixel. */
    SYSTEM_BPP_BITS12,
    /**< 12 Bits per Pixel - used for YUV420 format. */
    SYSTEM_BPP_BITS16,
    /**< 16 Bits per Pixel. */
    SYSTEM_BPP_BITS24,
    /**< 24 Bits per Pixel. */
    SYSTEM_BPP_BITS32,
    /**< 32 Bits per Pixel. */
    SYSTEM_BPP_BITS10,
    /**< 10 Bits per Pixel. */
    SYSTEM_BPP_BITS7,
    /**< 7 Bits per Pixel. */
    SYSTEM_BPP_BITS9,
    /**< 9 Bits per Pixel. */
    SYSTEM_BPP_BITS11,
    /**< 11 Bits per Pixel. */
    SYSTEM_BPP_BITS13,
    /**< 13 Bits per Pixel. */
    SYSTEM_BPP_BITS14,
    /**< 14 Bits per Pixel. */
    SYSTEM_BPP_BITS15,
    /**< 15 Bits per Pixel. */
    SYSTEM_BPP_BITS20,
    /**< 20 Bits per Pixel. */
    SYSTEM_BPP_BITS6,
    /**< 6 Bits per Pixel. */
    SYSTEM_BPP_BITS17,
    /**< 17 Bits per Pixel. */
    SYSTEM_BPP_BITS18,
    /**< 18 Bits per Pixel. */
    SYSTEM_BPP_BITS19,
    /**< 19 Bits per Pixel. */

    SYSTEM_BPP_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
    SYSTEM_BPP_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_BitsPerPixel;

/**
 *******************************************************************************
 * \brief Enums for frames per second
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_FPS_85,
    /**< 85 frames per second. */
    SYSTEM_FPS_75,
    /**< 75 frames per second. */
    SYSTEM_FPS_72,
    /**< 72 frames per second. */
    SYSTEM_FPS_70,
    /**< 70 frames per second. */
    SYSTEM_FPS_60,
    /**< 60 frames per second. */
    SYSTEM_FPS_50,
    /**< 50 frames per second. */
    SYSTEM_FPS_30,
    /**< 30 frames per second. */
    SYSTEM_FPS_25,
    /**< 25 frames per second. */
    SYSTEM_FPS_24,
    /**< 24 frames per second. */
    SYSTEM_FPS_15,
    /**< 15 frames per second. */
    SYSTEM_FPS_10,
    /**< 10 frames per second. */
    SYSTEM_FPS_5,
    /**< 5 frames per second. */
    SYSTEM_FPS_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
    SYSTEM_FPS_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_VideoFrameRate;

/**
 *******************************************************************************
 * \brief Enum for Field/Format
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_SF_INTERLACED = 0,
    /**< Interlaced mode. */
    SYSTEM_SF_PROGRESSIVE,
    /**< Progressive mode. */
    SYSTEM_SF_MAX,
    /**< Should be the last value of this enumeration.
         Will be used by driver for validating the input parameters. */
    SYSTEM_SF_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_VideoScanFormat;

/**
 *******************************************************************************
 * \brief Enum for Tiled v/s Non Tiled memory
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_MT_NONTILEDMEM = 0,
    /**< 1D non-tiled memory. */
    SYSTEM_MT_TILEDMEM,
    /**< 2D tiled memory. */
    SYSTEM_MT_MAX,
    /**< Should be the last value of this enumeration.
         Will be used by driver for validating the input parameters. */
    SYSTEM_MT_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_MemoryType;

/**
 *******************************************************************************
 * \brief Enums for standard resolution
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_STD_NTSC = 0u,
    /**< 720x480 30FPS interlaced NTSC standard. */
    SYSTEM_STD_PAL,
    /**< 720x576 30FPS interlaced PAL standard. */

    SYSTEM_STD_480I,
    /**< 720x480 30FPS interlaced SD standard. */
    SYSTEM_STD_576I,
    /**< 720x576 30FPS interlaced SD standard. */

    SYSTEM_STD_CIF,
    /**< Interlaced, 360x120 per field NTSC, 360x144 per field PAL. */
    SYSTEM_STD_HALF_D1,
    /**< Interlaced, 360x240 per field NTSC, 360x288 per field PAL. */
    SYSTEM_STD_D1,
    /**< Interlaced, 720x240 per field NTSC, 720x288 per field PAL. */

    SYSTEM_STD_480P,
    /**< 720x480 60FPS progressive ED standard. */
    SYSTEM_STD_576P,
    /**< 720x576 60FPS progressive ED standard. */

    SYSTEM_STD_720P_60,
    /**< 1280x720 60FPS progressive HD standard. */
    SYSTEM_STD_720P_50,
    /**< 1280x720 50FPS progressive HD standard. */

    SYSTEM_STD_1080I_60,
    /**< 1920x1080 30FPS interlaced HD standard. */
    SYSTEM_STD_1080I_50,
    /**< 1920x1080 50FPS interlaced HD standard. */

    SYSTEM_STD_1080P_60,
    /**< 1920x1080 60FPS progressive HD standard. */
    SYSTEM_STD_1080P_50,
    /**< 1920x1080 50FPS progressive HD standard. */

    SYSTEM_STD_1080P_24,
    /**< 1920x1080 24FPS progressive HD standard. */
    SYSTEM_STD_1080P_30,
    /**< 1920x1080 30FPS progressive HD standard. */

    /* Vesa standards from here Please add all SMTPE and CEA standard enums
     * above this only. this is to ensure proxy Oses compatibility
     */
    SYSTEM_STD_VGA_60,
    /**< 640x480 60FPS VESA standard. */
    SYSTEM_STD_VGA_72,
    /**< 640x480 72FPS VESA standard. */
    SYSTEM_STD_VGA_75,
    /**< 640x480 75FPS VESA standard. */
    SYSTEM_STD_VGA_85,
    /**< 640x480 85FPS VESA standard. */

    SYSTEM_STD_WVGA_60,
    /**< 800x480 60PFS WVGA */

    SYSTEM_STD_SVGA_60,
    /**< 800x600 60FPS VESA standard. */
    SYSTEM_STD_SVGA_72,
    /**< 800x600 72FPS VESA standard. */
    SYSTEM_STD_SVGA_75,
    /**< 800x600 75FPS VESA standard. */
    SYSTEM_STD_SVGA_85,
    /**< 800x600 85FPS VESA standard. */

    SYSTEM_STD_WSVGA_70,
    /**< 1024x600 70FPS standard. */

    SYSTEM_STD_XGA_60,
    /**< 1024x768 60FPS VESA standard. */
    SYSTEM_STD_XGA_DSS_TDM_60,
    /**< 1024x768 60FPS VESA standard. Applicable for
      *  DSS in 8-bit TDM mode.*/
    SYSTEM_STD_XGA_70,
    /**< 1024x768 72FPS VESA standard. */
    SYSTEM_STD_XGA_75,
    /**< 1024x768 75FPS VESA standard. */
    SYSTEM_STD_XGA_85,
    /**< 1024x768 85FPS VESA standard. */

    SYSTEM_STD_1368_768_60,
    /**< 1368x768 60 PFS VESA. */
    SYSTEM_STD_1366_768_60,
    /**< 1366x768 60 PFS VESA. */
    SYSTEM_STD_1360_768_60,
    /**< 1360x768 60 PFS VESA. */

    SYSTEM_STD_WXGA_30,
    /**< 1280x800 30FPS VESA standard. */
    SYSTEM_STD_WXGA_60,
    /**< 1280x800 60FPS VESA standard. */
    SYSTEM_STD_WXGA_75,
    /**< 1280x800 75FPS VESA standard. */
    SYSTEM_STD_WXGA_85,
    /**< 1280x800 85FPS VESA standard. */

    SYSTEM_STD_1440_900_60,
    /**< 1440x900 60 PFS VESA>*/

    SYSTEM_STD_SXGA_60,
    /**< 1280x1024 60FPS VESA standard. */
    SYSTEM_STD_SXGA_75,
    /**< 1280x1024 75FPS VESA standard. */
    SYSTEM_STD_SXGA_85,
    /**< 1280x1024 85FPS VESA standard. */

    SYSTEM_STD_WSXGAP_60,
    /**< 1680x1050 60 PFS VESA>*/

    SYSTEM_STD_SXGAP_60,
    /**< 1400x1050 60FPS VESA standard. */
    SYSTEM_STD_SXGAP_75,
    /**< 1400x1050 75FPS VESA standard. */

    SYSTEM_STD_UXGA_60,
    /**< 1600x1200 60FPS VESA standard. */

    /* Multi channel standards from here Please add all VESA standards enums
     * above this only. this is to ensure proxy Oses compatibility
     */
    SYSTEM_STD_MUX_2CH_D1,
    /**< Interlaced, 2Ch D1, NTSC or PAL. */
    SYSTEM_STD_MUX_2CH_HALF_D1,
    /**< Interlaced, 2ch half D1, NTSC or PAL. */
    SYSTEM_STD_MUX_2CH_CIF,
    /**< Interlaced, 2ch CIF, NTSC or PAL. */
    SYSTEM_STD_MUX_4CH_D1,
    /**< Interlaced, 4Ch D1, NTSC or PAL. */
    SYSTEM_STD_MUX_4CH_CIF,
    /**< Interlaced, 4Ch CIF, NTSC or PAL. */
    SYSTEM_STD_MUX_4CH_HALF_D1,
    /**< Interlaced, 4Ch Half-D1, NTSC or PAL. */
    SYSTEM_STD_MUX_8CH_CIF,
    /**< Interlaced, 8Ch CIF, NTSC or PAL. */
    SYSTEM_STD_MUX_8CH_HALF_D1,
    /**< Interlaced, 8Ch Half-D1, NTSC or PAL. */

    SYSTEM_STD_WXGA_5x3_30,
    /**< WXGA standard (1280x768) with the aspect ratio 5:3 at 30FPS. */
    SYSTEM_STD_WXGA_5x3_60,
    /**< WXGA resolution (1280x768) with the aspect ratio 5:3 at 60FPS. */
    SYSTEM_STD_WXGA_5x3_75,
    /**< WXGA resolution (1280x768) with the aspect ratio 5:3 at 75FPS. */

    /* Auto detect and Custom standards Please add all multi channel standard
     * enums above this only. this is to ensure proxy Oses compatibility
     */
    SYSTEM_STD_AUTO_DETECT,
    /**< Auto-detect standard. Used in capture mode. */
    SYSTEM_STD_CUSTOM,
    /**< Custom standard used when connecting to external LCD etc...
     *   The video timing is provided by the application.
     *   Used in display mode. */
    SYSTEM_STD_INVALID,

    SYSTEM_STD_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_Standard;

/**
 *******************************************************************************
 * \brief Enums for how many bits are interfaced with VIP capture
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_VIFW_8BIT = 0,
    /**< 8-bit interface. */
    SYSTEM_VIFW_10BIT,
    /**< 10-bit interface. */
    SYSTEM_VIFW_12BIT,
    /**< 12-bit interface. */
    SYSTEM_VIFW_14BIT,
    /**< 14-bit interface. */
    SYSTEM_VIFW_16BIT,
    /**< 16-bit interface. */
    SYSTEM_VIFW_18BIT,
    /**< 18-bit interface. */
    SYSTEM_VIFW_20BIT,
    /**< 20-bit interface. */
    SYSTEM_VIFW_24BIT,
    /**< 24-bit interface. */
    SYSTEM_VIFW_30BIT,
    /**< 30-bit interface. */
    SYSTEM_VIFW_1LANES,
    /**< CSI2 specific - 1 data lanes */
    SYSTEM_VIFW_2LANES,
    /**< CSI2 specific - 2 data lanes */
    SYSTEM_VIFW_3LANES,
    /**< CSI2 specific - 3 data lanes */
    SYSTEM_VIFW_4LANES,
    /**< CSI2 / LVDS specific - 4 data lanes */
    SYSTEM_VIFW_MAX,
    /**< Maximum modes */
    SYSTEM_VIFW_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_VideoIfWidth;

/**
 *******************************************************************************
 * \brief How sensor/decoder is interfaced with VIP capture port
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_VIFM_SCH_ES = 0,
    /**< Single Channel non multiplexed mode. */
    SYSTEM_VIFM_MCH_LINE_MUX_ES,
    /**< Multi-channel line-multiplexed mode. */
    SYSTEM_VIFM_MCH_PIXEL_MUX_ES,
    /**< Multi-channel pixel muxed. */
    SYSTEM_VIFM_SCH_DS_HSYNC_VBLK,
    /**< Single Channel non multiplexed discrete sync mode with HSYNC and
     *   VBLK as control signals. */
    SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC,
    /**< Single Channel non multiplexed discrete sync mode with HSYNC and
     *   VSYNC as control signals. */
    SYSTEM_VIFM_SCH_DS_AVID_VBLK,
    /**< Single Channel non multiplexed discrete sync mode with AVID and
     *   VBLK as control signals. */
    SYSTEM_VIFM_SCH_DS_AVID_VSYNC,
    /**< Single Channel non multiplexed discrete sync mode with AVID and
     *   VBLK as control signals. */
    SYSTEM_VIFM_MCH_LINE_MUX_SPLIT_LINE_ES,
    /**< Multi-channel line-multiplexed mode - split line mode. */
    SYSTEM_VIFM_SCH_CSI2,
    /**< Single channel capture via CSI2 interface */
    SYSTEM_VIFM_SCH_LVDS,
    /**< Single channel capture via LVDS interface */
    SYSTEM_VIFM_SCH_CPI,
    /**< Single channel capture via Parallel interface */
    SYSTEM_VIFM_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
    SYSTEM_VIFM_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_VideoIfMode;


/**
 *******************************************************************************
 * \brief Capture mode of buffer. How buffers will be looping around between
 *        driver and application.
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_CAPT_BCM_FRM_DROP = 0,
    /**< In this mode the driver will stop capturing data when there are
     *   no more buffers at the input queue.
     *   The driver will not hold any buffer with it and the last buffer
     *   will be returned to the application through dequeue call.
     *   For this mode, the driver makes use of the VPDMA drop data feature. */
    SYSTEM_CAPT_BCM_LAST_FRM_REPEAT,
    /**< In this mode the driver will keep capturing the data to the last
     *   queued buffer when there are no more buffers at the input queue.
     *   The driver will hold the last buffer with it till the application
     *   queues any new buffer or the capture is stopped. */
    SYSTEM_CAPT_BCM_CIRCULAR_FRM_REPEAT,
    /**< In this mode the driver will keep reusing all the sets of buffer
     *   with it in a circular fashion.
     *   Application cannot get back any buffer from the driver when streaming
     *   is on and dequeue call will result in error. */
    SYSTEM_CAPT_BCM_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
    SYSTEM_CAPT_BCM_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_CaptBufferCaptMode;

/**
 *******************************************************************************
 * \brief Enum for buffer Field ID
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_FID_TOP = 0,
    /**< Top field. */
    SYSTEM_FID_BOTTOM,
    /**< Bottom field. */
    SYSTEM_FID_FRAME,
    /**< Frame mode - Contains both the fields or a progressive frame. */
    SYSTEM_FID_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
    SYSTEM_FID_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} System_Fid;

/**
 *******************************************************************************
 * \brief Enum for signal polarity
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_POL_LOW = 0,
    /**< Low Polarity. */
    SYSTEM_POL_HIGH,
    /**< High Polarity. */
    SYSTEM_POL_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
     SYSTEM_POL_FORCE32BITS = 0x7FFFFFFF
     /**< This should be the last value after the max enumeration value.
      *   This is to make sure enum size defaults to 32 bits always regardless
      *   of compiler.
      */
} System_Polarity;

/**
 *******************************************************************************
 * \brief  Enum for sampling edge for signal.
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_EDGE_POL_RISING = 0,
    /**< Rising Edge. */
    SYSTEM_EDGE_POL_FALLING,
    /**< Falling Edge. */
    SYSTEM_EDGE_POL_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
     SYSTEM_EDGE_POL_FORCE32BITS = 0x7FFFFFFF
     /**< This should be the last value after the max enumeration value.
      *   This is to make sure enum size defaults to 32 bits always regardless
      *   of compiler.
      */
} System_EdgePolarity;

/**
 *******************************************************************************
 *  \brief This defines the standard coefficient sets available for
 *  different scaling ratios.
 *
 *  IMP: Do not assign numerical values to enum here.
 *
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_SC_DS_SET_ANTI_FLICKER,
    /**< Coefficient for anti-flicker effect */
    SYSTEM_SC_DS_SET_3_16,
    /**< Coefficient for down sampling 0.1875(3/16) <Factor<= 0.25(4/16). */
    SYSTEM_SC_DS_SET_4_16,
    /**< Coefficient for down sampling 0.25(4/16) <Factor<= 0.3125(5/16). */
    SYSTEM_SC_DS_SET_5_16,
    /**< Coefficient for down sampling 0.3125(5/16) <Factor<= 0.375(6/16). */
    SYSTEM_SC_DS_SET_6_16,
    /**< Coefficient for down sampling 0.375(6/16) <Factor<= 0.4375(7/16). */
    SYSTEM_SC_DS_SET_7_16,
    /**< Coefficient for down sampling 0.4375(7/16) <Factor<= 0.5(8/16). */
    SYSTEM_SC_DS_SET_8_16,
    /**< Coefficient for down sampling 0.5(8/16) <Factor<= 0.5625(9/16). */
    SYSTEM_SC_DS_SET_9_16,
    /**< Coefficient for down sampling 0.5625(9/16) <Factor<= 0.625(10/16). */
    SYSTEM_SC_DS_SET_10_16,
    /**< Coefficient for down sampling 0.625(10/16) <Factor<= 0.6875(11/16). */
    SYSTEM_SC_DS_SET_11_16,
    /**< Coefficient for down sampling 0.6875(11/16) <Factor<= 0.75(12/16). */
    SYSTEM_SC_DS_SET_12_16,
    /**< Coefficient for down sampling 0.75(12/16) <Factor<= 0.8125(13/16). */
    SYSTEM_SC_DS_SET_13_16,
    /**< Coefficient for down sampling 0.8125(13/16) <Factor<= 0.875(14/16). */
    SYSTEM_SC_DS_SET_14_16,
    /**< Coefficient for down sampling 0.875(14/16) <Factor<= 0.9375(15/16). */
    SYSTEM_SC_DS_SET_15_16,
    /**< Coefficient for down sampling 0.9375(15/16) < Factor< 1(16/16) */
    SYSTEM_SC_US_SET,
    /**< Coefficient set for the upsampling.  Includes horizontal, vertical
     *   and both chroma and luma up sampling. */
    SYSTEM_SC_SET_1_1,
    /**< Coefficient set for one-to-one scenario, when scaler is not in
     *   bypass. */
    SYSTEM_SC_SET_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
     SYSTEM_SC_SET_FORCE32BITS = 0x7FFFFFFF
    /**< Should be the last value of this enumeration.
     *   Will be used by driver for validating the input parameters. */
} System_ScCoeffSet;


/**
 *******************************************************************************
 * \brief Type of System buffer
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum  {

    SYSTEM_BUFFER_TYPE_VIDEO_FRAME,
    /**< Video frame buffer */

    SYSTEM_BUFFER_TYPE_BITSTREAM,
    /**< Bitstream buffer */

    SYSTEM_BUFFER_TYPE_METADATA,
    /**< Metadata buffer */

    SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER,
    /**< Group of 'synced' video frames buffer across N channels */

    SYSTEM_BUFFER_TYPE_FORCE32BITS = 0x7FFFFFFF
    /**< to force enum as 32-bit size */

} System_BufferType;

/**
 *******************************************************************************
 * \brief Type of bitstream
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum {

    SYSTEM_BITSTREAM_CODING_TYPE_MJPEG,
    /**< MJPEG coding */

    SYSTEM_BITSTREAM_CODING_TYPE_H264,
    /**< H264 coding */

    SYSTEM_BITSTREAM_CODING_TYPE_FORCE32BITS = 0x7FFFFFFF
    /**< to force enum as 32-bit size */

} System_BitstreamCodingType;

/**
 *******************************************************************************
 * \brief Video type for encoder / decoder links
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum {
    SYSTEM_IVIDEO_MPEG1 = 1,
    /**< Video format is Mpeg1 stream */
    SYSTEM_IVIDEO_MPEG2SP = 2,
    /**< Video format is Mpeg2/H.262 stream, Simple Profile */
    SYSTEM_IVIDEO_MPEG2MP = 3,
    /**< Video format is Mpeg2/H.262 stream, Main Profile */
    SYSTEM_IVIDEO_MPEG2HP = 4,
    /**< Video format is Mpeg2/H.262 stream, High Profile */
    SYSTEM_IVIDEO_MPEG4SP = 5,
    /**< Video format is Mpeg4 stream, Simple Profile */
    SYSTEM_IVIDEO_MPEG4ASP = 6,
    /**< Video format is Mpeg4 stream, Advanced Simple Profile */
    SYSTEM_IVIDEO_H264BP = 7,
    /**< Video format is H.264 stream, Base Profile */
    SYSTEM_IVIDEO_H264MP = 8,
    /**< Video format is H.264 stream, Main Profile */
    SYSTEM_IVIDEO_H264HP = 9,
    /**< Video format is H.264 stream, High Profile */
    SYSTEM_IVIDEO_VC1SP = 10,
     /**< Video format is VC1/WMV9 stream, Simple Profile */
    SYSTEM_IVIDEO_VC1MP = 11,
    /**< Video format is VC1/WMV9 stream, Main Profile */
    SYSTEM_IVIDEO_VC1AP = 12,
    /**< Video format is VC1 stream, Advanced Profile */
    SYSTEM_IVIDEO_H264RCDO = 13,
    /**< Video format is H.264 stream, Fast profile/RCDO */
    SYSTEM_IVIDEO_RV8 = 14,
    /**< Video format is Real Video 8 stream */
    SYSTEM_IVIDEO_RV9 = 15,
    /**< Video format is Real Video 9 stream */
    SYSTEM_IVIDEO_RV10 = SYSTEM_IVIDEO_RV9,
    /**< Video format is Real Video 10 stream, same as RV9 */
    SYSTEM_IVIDEO_ON2VP6 = 16,
    /**< Video format is ON2, VP6.x */
    SYSTEM_IVIDEO_ON2VP7 = 17,
    /**< Video format is ON2, VP7.x */
    SYSTEM_IVIDEO_AVS10 = 18,
    /**< Video format is AVS 1.0 */
    SYSTEM_IVIDEO_SORENSONSPARK = 19,
    /**< Video format is SorensonSpark V0/V1 */
    SYSTEM_IVIDEO_H263_PROFILE0 = 20,
    /**< Video format is H263 Base line profile */
    SYSTEM_IVIDEO_H263_PROFILE3 = 21,
    /**< Video format is H263 and Annex IJKT */
    SYSTEM_IVIDEO_H264SVC = 22,
    /**< Video format is SVC */
    SYSTEM_IVIDEO_MULTIVIEW = 23,
    /**< Video format is Multiview coding */
    SYSTEM_IVIDEO_MJPEG = 24,
    /**< Video format is motion JPEG */
    SYSTEM_IVIDEO_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} System_IVideoFormat;

/**
 *******************************************************************************
 * \brief Video frame types for encoder / decoder links
 *
 *        For the various @c IVIDEO_xy_FRAME values, this frame type is
 *        interlaced where both top and bottom fields are
 *        provided in a single frame.  The first field is an "x"
 *        frame, the second field is "y" field.
 *
 *******************************************************************************
*/

typedef enum {
    SYSTEM_IVIDEO_NA_FRAME = -1,
    /**< Frame type not available. */
    SYSTEM_IVIDEO_I_FRAME = 0,
    /**< Intra coded frame. */
    SYSTEM_IVIDEO_P_FRAME = 1,
    /**< Forward inter coded frame. */
    SYSTEM_IVIDEO_B_FRAME = 2,
    /**< Bi-directional inter coded frame. */
    SYSTEM_IVIDEO_IDR_FRAME = 3,
    /**< Intra coded frame that can be used for
     *   refreshing video content.
     */
    SYSTEM_IVIDEO_II_FRAME = 4,
    /**< Interlaced Frame, both fields are I frames
      */
    SYSTEM_IVIDEO_IP_FRAME = 5,
    /**< Interlaced Frame, first field is an I frame,
      *   second field is a P frame.
      */
    SYSTEM_IVIDEO_IB_FRAME = 6,
    /**< Interlaced Frame, first field is an I frame,
      *   second field is a B frame.
      */
    SYSTEM_IVIDEO_PI_FRAME = 7,
    /**< Interlaced Frame, first field is a P frame,
      *   second field is a I frame.
      */
    SYSTEM_IVIDEO_PP_FRAME = 8,
    /**< Interlaced Frame, both fields are P frames.
      */
    SYSTEM_IVIDEO_PB_FRAME = 9,
    /**< Interlaced Frame, first field is a P frame,
      *   second field is a B frame.
      */
    SYSTEM_IVIDEO_BI_FRAME = 10,
    /**< Interlaced Frame, first field is a B frame,
      *   second field is an I frame.
      */
    SYSTEM_IVIDEO_BP_FRAME = 11,
    /**< Interlaced Frame, first field is a B frame,
      *   second field is a P frame.
      */
    SYSTEM_IVIDEO_BB_FRAME = 12,
    /**< Interlaced Frame, both fields are B frames.
     */
    SYSTEM_IVIDEO_MBAFF_I_FRAME = 13,
    /**< Intra coded MBAFF frame.
     */
    SYSTEM_IVIDEO_MBAFF_P_FRAME = 14,
    /**< Forward inter coded MBAFF frame.
     */
    SYSTEM_IVIDEO_MBAFF_B_FRAME = 15,
    /**< Bi-directional inter coded MBAFF frame.
     */
    SYSTEM_IVIDEO_MBAFF_IDR_FRAME = 16,
    /**< Intra coded MBAFF frame that can be used
      *   for refreshing video content.
      */
    /** Default setting. */
    SYSTEM_IVIDEO_FRAMETYPE_DEFAULT = SYSTEM_IVIDEO_I_FRAME,

    SYSTEM_IVIDEO_FRAMETYPE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} System_IVideoFrameType;

/**
 *******************************************************************************
 * \brief Video content types for encoder / decoder links
 *******************************************************************************
*/

typedef enum {
    SYSTEM_IVIDEO_CONTENTTYPE_NA = -1,
    /**< Frame type is not available. */
    SYSTEM_IVIDEO_PROGRESSIVE = 0,
    /**< Progressive frame. */
    SYSTEM_IVIDEO_PROGRESSIVE_FRAME = SYSTEM_IVIDEO_PROGRESSIVE,
    /**< Progressive Frame. */
    SYSTEM_IVIDEO_INTERLACED = 1,
    /**< Interlaced frame. */
    SYSTEM_IVIDEO_INTERLACED_FRAME = SYSTEM_IVIDEO_INTERLACED,
    /**< Interlaced frame. */
    SYSTEM_IVIDEO_INTERLACED_TOPFIELD = 2,
    /**< Interlaced picture, top field. */
    SYSTEM_IVIDEO_INTERLACED_BOTTOMFIELD = 3,
    /**< Interlaced picture, bottom field. */
    /**Default setting. */
    SYSTEM_IVIDEO_CONTENTTYPE_DEFAULT = SYSTEM_IVIDEO_PROGRESSIVE,
    SYSTEM_IVIDEO_CONTENTTYPE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} System_IVideoContentType;

/**
 *******************************************************************************
 * \brief Encoding Preset for the enclink
 *******************************************************************************
*/

typedef enum {
    SYSTEM_XDM_DEFAULT = 0,
    /**< Default setting of encoder.
     *   See codec specific documentation for its encoding behaviour.
     */
    SYSTEM_XDM_HIGH_QUALITY = 1,
    /**< High quality encoding. */
    SYSTEM_XDM_HIGH_SPEED = 2,
    /**< High speed encoding. */
    SYSTEM_XDM_USER_DEFINED = 3,
    /**< User defined configuration, using advanced parameters.*/
    SYSTEM_XDM_HIGH_SPEED_MED_QUALITY = 4,
     /**< High speed, medium quality encoding */
    SYSTEM_XDM_MED_SPEED_MED_QUALITY = 5,
    /**< Medium speed, medium quality encoding*/
    SYSTEM_XDM_MED_SPEED_HIGH_QUALITY = 6,
    /**< Medium speed, high quality encoding */
    SYSTEM_XDM_ENCODING_PRESET_MAX  = 7,
    SYSTEM_XDM_PRESET_DEFAULT = SYSTEM_XDM_MED_SPEED_MED_QUALITY
    /**< Default setting of encoder.
     *   See codec specific documentation for its encoding behaviour.
     */
} System_XDMEncodingPreset;

/**
 *******************************************************************************
 * \brief Rate control preset for the encoder
 *******************************************************************************
*/

typedef enum {
    SYSTEM_IVIDEO_LOW_DELAY = 1,
    /**< CBR rate control for video conferencing. */
    SYSTEM_IVIDEO_STORAGE = 2,
    /**< VBR rate control for local storage (DVD) recording */
    SYSTEM_IVIDEO_TWOPASS = 3,
    /**< Two pass rate control for non real time applications. */
    SYSTEM_IVIDEO_NONE = 4,
    /**< No configurable video rate control mechanism. */
    SYSTEM_IVIDEO_USER_DEFINED = 5,
    /**< User defined configuration using extended parameters. */
    SYSTEM_IVIDEO_RATECONTROLPRESET_DEFAULT = SYSTEM_IVIDEO_LOW_DELAY
    /** Default setting. */
} System_IVideoRateControlPreset;

/**
 *******************************************************************************
 * \brief Motion Vector accuracy for the encoder link
 *******************************************************************************
*/

typedef enum {
    SYSTEM_IVIDENC2_MOTIONVECTOR_PIXEL = 0,
    /**< Motion vectors accuracy is only integer pel. */
    SYSTEM_IVIDENC2_MOTIONVECTOR_HALFPEL = 1,
    /**< Motion vectors accuracy is half pel. */
    SYSTEM_IVIDENC2_MOTIONVECTOR_QUARTERPEL = 2,
    /**< Motion vectors accuracy is quarter pel. */
    SYSTEM_IVIDENC2_MOTIONVECTOR_EIGHTHPEL = 3,
    /**< Motion vectors accuracy is one-eighth pel. */
    SYSTEM_IVIDENC2_MOTIONVECTOR_MAX = 4
    /**< Motion vectors accuracy is not defined */
} System_IVidenc2MotionVectorAccuracy;


/**
 *******************************************************************************
 *  \brief CSI2 Data types.
 *******************************************************************************
 */
typedef enum
{
    SYSTEM_CSI2_YUV420_8B = 0x18,
    /**< YUV 4:2:0 with 8bit for each Y/U/V */
    SYSTEM_CSI2_YUV420_10B = 0x19,
    /**< YUV 4:2:0 with 10bit for each Y/U/V */
    SYSTEM_CSI2_YUV420_8B_LEGACY = 0x1A,
    /**< YUV 4:2:0 with 8bit for each Y/U/V */
    SYSTEM_CSI2_YUV420_8B_CHROMA_SHIFT = 0x1C,
    /**< YUV 4:2:0 with 8bit for each Y/U/V with
     *   with phase shifted chroma */
    SYSTEM_CSI2_YUV420_10B_CHROMA_SHIFT = 0x1D,
    /**< YUV 4:2:0 with 10bit for each Y/U/V with
     *   with phase shifted chroma */
    SYSTEM_CSI2_YUV422_8B = 0x1E,
    /**< YUV 4:2:2 with 8bit for each Y/U/V */
    SYSTEM_CSI2_YUV422_10B = 0x1F,
    /**< YUV 4:2:2 with 10bit for each Y/U/V */
    SYSTEM_CSI2_RGB444 = 0x20,
    /**< RGB888 - 4-bits B, 4-bits G, 4-bits R */
    SYSTEM_CSI2_RGB555 = 0x21,
    /**< RGB888 - 5-bits B, 5-bits G, 5-bits R */
    SYSTEM_CSI2_RGB565 = 0x22,
    /**< RGB888 - 5-bits B, 6-bits G, 5-bits R */
    SYSTEM_CSI2_RGB666 = 0x23,
    /**< RGB888 - 6-bits B, 6-bits G, 6-bits R */
    SYSTEM_CSI2_RGB888 = 0x24,
    /**< RGB888 - 8-bits B, 8-bits G, 8-bits R */
    SYSTEM_CSI2_RAW6 = 0x28,
    /**< 6 bit raw-data. */
    SYSTEM_CSI2_RAW7 = 0x29,
    /**< 7 bit raw-data. */
    SYSTEM_CSI2_RAW8 = 0x2A,
    /**< 8 bit raw-data. */
    SYSTEM_CSI2_RAW10 = 0x2B,
    /**< 10 bit raw-data. */
    SYSTEM_CSI2_RAW12 = 0x2C,
    /**< 12 bit raw-data. */
    SYSTEM_CSI2_RAW14 = 0x2D,
    /**< 14 bit raw-data. */
    SYSTEM_CSI2_ANY = 0x01,
    /**< Allow any data type for capture */
    SYSTEM_CSI2_DISABLE_CONTEXT = 0x00,
    /**< Disable context */
    SYSTEM_CSI2_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
      *   This is to make sure enum size defaults to 32 bits always regardless
      *   of compiler.
      */
} System_Csi2DataFormat;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
