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
 * \ingroup FRAMEWORK_MODULE_API
 * \defgroup NULL_SOURCE_LINK_API Null Source Link API
 *
 * Null Src Link is a source link which provides input to the next links
 * It can be used to integrate other links when capture link is not available
 * or not used. It can also be used for supplying pre-defined data to the chain
 * for testing purpose.
 * The Link currently supports - YUV420sp, YUV422i and bitstream data.
 * The link has a single output queue and supports multiple channel.
 * For testing purpose in conjunction with CCS, the link can be used to
 * data from file in a chain.
 *
 * @{
 ******************************************************************************
 */

/**
 ******************************************************************************
 *
 * \file nullSrcLink.h
 *
 * \brief Null source link API public header file.
 *
 * \version 0.0 (Dec 2013) : [VT] First version
 *
 ******************************************************************************
 */

#ifndef _NULL_SRC_LINK_H_
#define _NULL_SRC_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Include's    */

#include <include/link_api/networkCtrl_if.h>
#include <include/link_api/system.h>

/**
 ******************************************************************************
 *
 * \brief Maximum number of null source link objects
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_OBJ_MAX                       (1)

/**
 ******************************************************************************
 *
 * \brief Maximum number of output queues that null source link supports.
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_MAX_OUT_QUE                   (1)

/******************************************************************************
 *
 * \brief Max channels per output queues supported by Null Src Link
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
#define NULL_SRC_LINK_MAX_CH                        (6)

/**
 ******************************************************************************
 *
 * \brief Default number of channels in null source link
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_NUM_CHANNELS_DEFAULT          (1)

/**
 ******************************************************************************
 *
 * \brief Maximum number of output buffers per channel in null source link
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_MAX_OUT_BUFS_PER_CH       (SYSTEM_LINK_MAX_FRAMES_PER_CH)

/**
 ******************************************************************************
 *
 * \brief Default number of output buffers per channel in null source link
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_NUM_BUFS_PER_CH_DEFAULT       (4)

/**
 ******************************************************************************
 *
 * \brief Default time interval between buffers sent by Null Src Link
 * 16 is set considering a throughput rate of 60fps.
 *
 * SUPPORTED in ALL platforms
 *
 ******************************************************************************
 */
#define NULL_SRC_LINK_BUF_TIME_INTERVAL_DEFAULT     (16)


/**
 *******************************************************************************
 * \brief  Data receive mode
 *******************************************************************************
*/
typedef enum
{
    NULLSRC_LINK_DATA_RX_MODE_FILE = 0,
    /**< Data is read from a file based on
     *   NullSrcLink_ChannelSpecificParams.NullSrcLink_ApiFileReadMode
     */

    NULLSRC_LINK_DATA_RX_MODE_NETWORK,
    /**< Data is received over network,
     *   NullSrcLink_CreateParams.serverPort needs to be set
     */

    NULLSRC_LINK_DATA_RX_MODE_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

} NullSrcLink_DataRxMode;

/**
 *******************************************************************************
 * \brief Enum for the File read mode
 *
 * This mode will indicate whether Null Src link will read data from a file.
 * And if does then whether its read at create time or run time.
 * By default file read is disabled. Only for the purpose of debugging using CCS
 * should other modes be enabled. File read option will work ONLY with CCS.
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    NULLSRC_LINK_FILEREAD_DISABLE = 0,
    /**<File read is disabled. Dummy data is circulated. */

    NULLSRC_LINK_FILEREAD_CREATE_TIME,
    /**< Buffers are filled with data from file in create state.
    * These are then circulated till the link is deleted.
    */

    NULLSRC_LINK_FILEREAD_RUN_TIME,
    /**< Buffers are filled with data from file in run state.
    * Everytime New Data has to be processed data is read from file and buffer
    * is filled.
    */

    NULLSRC_LINK_FILEREAD_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}NullSrcLink_ApiFileReadMode;

/******************************************************************************
 *
 * Data structures
 *
*******************************************************************************
*/

/******************************************************************************
 *
 * brief Null source link Channel specific create parameters
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32                   numBuffers;
    /**< Number of buffers per channel to be allocated for the source link.
    * This can be set based on the throughput rate the chain requires.
    */

    NullSrcLink_ApiFileReadMode fileReadMode;
    /**< This mode will take values from NullSrcLink_ApiFileReadMode
     *
     *   ONLY needs to be set when dataRxMode is
     *     NULLSRC_LINK_DATA_RX_MODE_FILE
     */

    char                    nameDataFile[260];
    /**< File containing the stream data. This is a binary file.
     * 260 is filename size limit set by WIndows 7
     * This file resides on local machine and used only for the purpose of
     * debugging with CCS
     *
     *   ONLY needs to be set when dataRxMode is
     *     NULLSRC_LINK_DATA_RX_MODE_FILE
     */

    char                    nameIndexFile[260];
    /**< File used to index into binary Data file. This is a text file.
     * This file resides on local machine and used in tandem with nameDataFile
     * only for the purpose of debugging with CCS
     * It contains frame sizes in bytes for every frame on every line.
     * e.g. contents of index file for 3 frames of YUV420,image size 200x100
     *
     * 30000
     * 30000
     * 30000
     *
     *
     *   ONLY needs to be set when dataRxMode is
     *     NULLSRC_LINK_DATA_RX_MODE_FILE
     */
} NullSrcLink_ChannelSpecificParams;

/******************************************************************************
 *
 * brief Null source link create parameters
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/

typedef struct
{
    System_LinkOutQueParams   outQueParams;
    /**< output queue parameters. contains next links ID */

    System_LinkQueInfo        outQueInfo;
    /**< Output queue information : This ontains channel info which further
        * details the buffer format and resolutions
        */

    UInt32                    timerPeriodMilliSecs;
    /**< Time period at which output data has to be generated
    * Note that for all CHannels - Throughput rate is common
    */

    NullSrcLink_ChannelSpecificParams
                                channelParams[NULL_SRC_LINK_MAX_CH];
    /**< Channel Specific Parameters contain file read mode, num buffers etc.*/

    UInt32 networkServerPort;
    /**< Server port ot use when dataRxMode is
     *   NULLSRC_LINK_DATA_RX_MODE_NETWORK
     */

    NullSrcLink_DataRxMode dataRxMode;
    /**< Recevied data via File or network */

} NullSrcLink_CreateParams;

/******************************************************************************
*
*  Functions
*
*******************************************************************************
*/

/**
*******************************************************************************
 *
 * \brief Null Source link register and init
 *
 *  - Creates link task
 *  - Registers as a link with the system API
 *
 * \return SYSTEM_LINK_STATUS_SOK
 *
 ******************************************************************************
 */
Int32 NullSrcLink_init();

/**
*******************************************************************************
 *
 * \brief Null Source link de-register and de-init
 *
 *  - Deletes link task
 *  - De-registers as a link with the system API
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 ******************************************************************************
 */
Int32 NullSrcLink_deInit();

/**
 ******************************************************************************
 *
 * \brief Null Source link set default parameters for create time params
 *
 * \param  pPrm  [IN]  NullSourceLink Create time Params
 *
 ******************************************************************************
 */
static inline void NullSrcLink_CreateParams_Init
                                            (NullSrcLink_CreateParams *pPrm)
{
    UInt32 chId;
    System_LinkChInfo *pChInfo;

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->timerPeriodMilliSecs = NULL_SRC_LINK_BUF_TIME_INTERVAL_DEFAULT;
    pPrm->outQueInfo.numCh = NULL_SRC_LINK_NUM_CHANNELS_DEFAULT;
    pPrm->dataRxMode = NULLSRC_LINK_DATA_RX_MODE_FILE;

    pPrm->networkServerPort = NETWORK_RX_SERVER_PORT;

    /*
    * For YUV422i parameters are Data format = SYSTEM_DF_YUV422I_YUYV,
    * Scan Format = SYSTEM_SF_PROGRESSIVE,
    * Buffert Type = SYSTEM_BUFFER_TYPE_VIDEO_FRAME
    * Width and Height need to be set.
    * Pitch should be equal to width * 2
    */

    /*
    * For Bitstream parameters are Buf Type = SYSTEM_BUFFER_TYPE_BITSTREAM,
    * BITSTREAM_FORMAT = SYSTEM_BITSTREAM_CODING_TYPE_MJPEG,
    * Width and Height need to be set. Pitch is irrelevant.
    */

    for (chId = 0; chId < pPrm->outQueInfo.numCh; chId++)
    {
        pPrm->channelParams[chId].fileReadMode = NULLSRC_LINK_FILEREAD_DISABLE;

        pChInfo = &pPrm->outQueInfo.chInfo[chId];

        SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(pChInfo->flags,
                                                    SYSTEM_DF_YUV420SP_UV);
        SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(pChInfo->flags,
                                                    SYSTEM_SF_PROGRESSIVE);
        SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(pChInfo->flags,
                                               SYSTEM_BUFFER_TYPE_VIDEO_FRAME);
        pChInfo->width = 800;
        pChInfo->height = 480;
        pChInfo->startX = 0;
        pChInfo->startY = 0;
        pChInfo->pitch[0] = SystemUtils_align(pChInfo->width, 32);
        pChInfo->pitch[1] = SystemUtils_align(pChInfo->width, 32);

        pPrm->channelParams[chId].numBuffers
                                        = NULL_SRC_LINK_NUM_BUFS_PER_CH_DEFAULT;
    }

    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/
