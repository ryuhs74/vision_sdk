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
 * \defgroup DISPLAY_LINK_API Display Link API
 *
 * \brief  This module has the interface for using Display Link
 *
 *         Display Link is used to feed video frames over a video/graphics
 *         pipe to a connected VENC. The connection of input pipe to a VENC
 *         is done by Display Controller. This link deals with actually
 *         displaying the video/graphic frames from a previous link onto the
 *         display device.
 *
 *         Each input pipe needs to be a separate display link instance.
 *         System link create API is used to create a display link instance
 *         with the input pipe info to use be specified as create time
 *         parameters.
 *
 *         The display link can only take input for a single input queue.
 *         The single input queue can contain multiple channels but only
 *         one of the channel can be shown at a time.
 *
 *         By default CH0 is shown on the display.
 *
 *         Users can use the command DISPLAY_LINK_CMD_SWITCH_CH to switch
 *         the channel that is displayed on the display - This feature is
 *         NOT supported in this version.
 *
 *         Display link also supported an inline scalar. The scalar will be
 *         enabled automatically once the input image resolution is different
 *         than the target video window display resolution
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file displayLink.h
 *
 * \brief Display Link API
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _DISPLAY_LINK_H_
#define _DISPLAY_LINK_H_

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
 *   \brief Default value of maximum number of frames allowed to be
 *          queued into the display driver
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DISPLAY_LINK_MAX_DRIVER_QUEUE_LENGTH_DEFAULT (0)
/* @} */

/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup DISPLAY_LINK_API_CMD Display Link Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Switch input channel that is being displayed
 *
 *   SUPPORTED platforms - None of the platforms is supported this as of today
 *
 *   MUST be set by user
 *
 *   \param DisplayLink_SwitchChannelParams *pPrm [IN]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define DISPLAY_LINK_CMD_SWITCH_CH                   (0x4000)


/**
 *******************************************************************************
 *
 *   \brief Link CMD: Run time Command to get display performance statistics
 *
 *   \param DisplayLink_Statistics  [OUT]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define DISPLAY_LINK_CMD_GET_STATISTICS             (0x4003)

/* @} */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the display instances (Pipes) supported
 *
 *          List of the supported display instance IDs to be configured by
 *          user/App while creating a specific display link instance.
 *          Please note that SDK do not support multiple display link instances
 *          with same displayId. However Display link as such supported
 *          multiple instances with each configured with a unique displayId
 *******************************************************************************
*/
typedef enum
{
    DISPLAY_LINK_INST_DSS_VID1 = 0,
    /**< Video1 Pipeline. */
    DISPLAY_LINK_INST_DSS_VID2,
    /**< Video2 Pipeline. */
    DISPLAY_LINK_INST_DSS_VID3,
    /**< Video3 Pipeline. */
    DISPLAY_LINK_INST_DSS_GFX1,
    /**< GFX1 Pipeline. */
    DISPLAY_LINK_INST_DSS_MAX,
    /**< Should be the last value of this enumeration.
     *   Will be used by Link/driver for validating the input parameters. */
    DISPLAY_LINK_INST_DSS_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} DisplayLink_displayID;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Display link statistics
 *
 *          All count values are relative to last reset of the counter's
 *          Counter's are reset when 'resetStatistics' is TRUE or
 *          when 'Print Statistics' command is called.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 resetStatistics;
    /**< TRUE: Reset counter after getting the values */

    UInt32 elaspedTimeInMsec;
    /**< Time since since start last reset of statistics */

    UInt32 inBufRecvCount;
    /**< Number of frames received on active channel */

    UInt32 inBufDropCount;
    /**< Number of frames that were dropped and could not be displayed */

    UInt32 inBufDisplayCount;
    /**< Number of frames that were displayed on the screen */

    UInt32 displayIsrCount;
    /**< Number of display ISR's */

} DisplayLink_Statistics;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Display link real time parameters
 *
 *          Display link supports run time configuration of the output image
 *          resolution and the output window size. All the Below parameters
 *          need to be populated properly for any dynamic display resolution
 *          or window position update.
 *          Real time display update is not supported as of today
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   tarWidth;
    /**< Horizontal Size of the picture at output of video display
     *   Not used in case of Graphics instance */

    UInt32                   tarHeight;
    /**< vertical Size of picture at output of video display
     *   Not used in case of Graphics instance */

    UInt32                   posX;
    /**< X-Coordinate position of the frame in the output Video Window */

    UInt32                   posY;
    /**< Y-Coordinate position of the frame in the output Video Window */
} DisplayLink_RtParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Display link create time parameters
 *
 *          This structure is used to create and configure a Display link
 *          instance.
 *
 *******************************************************************************
*/
typedef struct
{
    System_LinkInQueParams   inQueParams;
    /**< Display link input queue information */
    UInt32                   displayId;
    /**< Used to select the display driver to be used in this link instance.
     *   See DisplayLink_displayID for the supported values */
    UInt32                   displayScanFormat;
    /**< Display device scanformat type. Display link support both
     *   progressive and interlaced display devices
     *   See System_VideoScanFormat for the supported values */
    DisplayLink_RtParams     rtParams;
    /**< Display link real time configuration parameters
     *   This needs to be configured at create time as well,
     *   IF not set, then assume tarWidth & tarHeight as input
     *   width & height and both posX & posY as 0 */
} DisplayLink_CreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Display link switch channel parameters
 *
 *          The display link can only take input for a single input queue.
 *          The single input queue can contain multiple channels, but only
 *          one of the channel can be shown at a time.
 *          Real time input channel switch is not supported as of today
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   activeChId;
    /**< Active chID from which frames should be displayed */
} DisplayLink_SwitchChannelParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Display link register and init function
 *
 *          For each display instance (VID1, VID2, VID3 or GRPX1)
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 DisplayLink_init();

/**
 *******************************************************************************
 *
 *   \brief Display link de-register and de-init function
 *
 *          For each display instance (VID1, VID2, VID3 or GRPX1)
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 DisplayLink_deInit();

/**
 *******************************************************************************
 *
 *   \brief Function to initialize the Display Link Create Params
 *
 *          Sets default values for Display link create time parameters
 *          User/App can override these default values later.
 *
 *   \param prm [IN] Display Link create parameters
 *
 *   \return void
 *
 *******************************************************************************
*/
static inline Void DisplayLink_CreateParams_Init(DisplayLink_CreateParams *prm)
{
    memset(prm, 0, sizeof(*prm));
    memset(&prm->inQueParams,0,sizeof(prm->inQueParams));
    memset(&prm->rtParams,0,sizeof(prm->rtParams));

    prm->displayId = DISPLAY_LINK_INST_DSS_VID1;
    prm->displayScanFormat = SYSTEM_SF_PROGRESSIVE;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
