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
 *   \ingroup SAMPLE_MODULE_API
 *   \defgroup GRPX_SRC_LINK_API Grpx Source Link API
 *
 *   Grpx source Link can be used for the following  purposes
 *     1. For testing a link that doesnot take capture data
 *     2. For generating logo and sending it to Display
 *     3. For printing stats on the display
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file grpxSrcLink.h
 *
 * \brief Grpx Src link API public header file.
 *
 * \version 0.0 (Oct 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _GRPX_SRC_LINK_H_
#define _GRPX_SRC_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <include/link_api/system.h>

/* @{ */

/**
 *******************************************************************************
 *
 * \brief Max input queues supported by Grpx Src Link
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define GRPX_SRC_LINK_MAX_IN_QUE        (1)

/**
 *******************************************************************************
 *
 * \brief Max output queues supported by Grpx Src Link
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define GRPX_SRC_LINK_MAX_OUT_QUE        (1)

/* @} */

/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup GRPXSRC_LINK_API_CMD Grpx Src Link Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *   \brief Link CMD: Print the passed string on display
 *
 *   Command used to print arbitrary string on display using GrpxSrc link
 *
 *   \param GrpxSrcLink_StringRunTimePrintParams * [IN] String to be printed
 *******************************************************************************
 */
#define GRPX_SRC_LINK_CMD_PRINT_STRING (0x5001)

/* @} */



/******************************************************************************
 *
 *  Data structures
 *
*******************************************************************************
*/

/**
*******************************************************************************
 * \brief Grpx src link create parameters.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32 dataFormat;
    /**< Valid values of type System_VideoDataFormat
     *   Currently below data formats are supported
     *   - SYSTEM_DF_BGR16_565
     *   - SYSTEM_DF_BGRA16_4444
     */

    UInt32 width;
    /**< channel resolution - width */

    UInt32 height;
    /**< channel resolution - height */

} GrpxSrcLink_BufferInfo;

/**
*******************************************************************************
 * \brief Grpx src logo parameters.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32 startX;
    /**< X position where the logo starts */

    UInt32 startY;
    /**< Y position where the logo starts */
} GrpxSrcLink_LogoParameters;

/**
*******************************************************************************
 * \brief Grpx src Optical Flow Legend parameters.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32 lutId;
    /**< LUT bitmap to use */

    UInt32 fps;
    /**< Optical flow frame-rate */

} GrpxSrcLink_OpticalFlowParameters;


/**
*******************************************************************************
 * \brief Grpx src Stats Print parameters.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32 startX;
    /**< X position where the stats print starts */

    UInt32 startY;
    /**< Y position where the stats print starts */

} GrpxSrcLink_StatsPrintParams;

/**
*******************************************************************************
 * \brief Grpx src String properties
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32 startX;
    /**< X position where the stats print starts */

    UInt32 startY;
    /**< Y position where the stats print starts */

    UInt32 fontType;
    /**< font type */

    char string[128];
    /**< String to be displayed */

} GrpxSrcLink_StringInfo;

/**
*******************************************************************************
 * \brief Grpx src string runtime print cmd params.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    Int32 duration_ms;
    /**< Duration in ms string should be displayed */

    GrpxSrcLink_StringInfo stringInfo;
    /**< Strings to be displayed */

} GrpxSrcLink_StringRunTimePrintParams;

/**
*******************************************************************************
 * \brief Grpx src link create parameters.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/

typedef struct
{
    UInt32 enable;
    /**< TRUE: draw ultrasonic results generated by link ID,
               ultrasonicResultsLinkId
     */

    UInt32 windowStartX;
    /**< Display window start position */

    UInt32 windowStartY;
    /**< Display window start position */

    UInt32 windowWidth;
    /**< Width of ultrasonic overlay data */

    UInt32 windowHeight;
    /**< Width of ultrasonic overlay data */

} GrpxSrcLink_UltrasonicParams;

typedef struct
{
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */

    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */

    GrpxSrcLink_BufferInfo      grpxBufInfo;
    /**< Instance of channel information */

    GrpxSrcLink_LogoParameters   logoParams;
    /**< Instance of Logo Parameters */

    GrpxSrcLink_StatsPrintParams   statsPrintParams;
    /**< Instance of stats Parameters */

    GrpxSrcLink_OpticalFlowParameters opticalFlowParams;
    /**< Instance of OF Legend Parameters */

    GrpxSrcLink_UltrasonicParams ultrasonicParams;
    /**< Ultraosnic drawing params */

    UInt32                     logoDisplayEnable;
    /**< Flag to enable/disable logo */

    UInt32                     statsDisplayEnable;
    /**< Flag to enable/disable stats */

    UInt32                     surroundViewEdgeDetectLayoutEnable;
    /**< Flag to enable/disable surround view layout */

    UInt32                     surroundViewPdTsrLayoutEnable;
    /**< Flag to enable/disable surround view + PD/TSR layout */

    UInt32                     surroundViewLdLayoutEnable;
    /**< Flag to enable/disable surround view + LD layout */

    UInt32                     enableJeepOverlay;
    /**< Enable for 2D SRV, Disable for 3D SRV  */

    UInt32                     surroundViewDOFLayoutEnable;
    /**< Flag to enable/disable surround view + dof layout */

    UInt32                     opticalFlowLayoutEnable;
    /**< Flag to enable/disable of Legend */

    UInt32                     pdTsrLdLayoutEnable;
    /**< Flag to enable/disable of specific layout */

    UInt32                     pdTsrLdSofLayoutEnable;
    /**< Flag to enable/disable of specific layout */

    UInt32                     pdTsrLdSofStereoLayoutEnable;
    /**< Flag to enable/disable of specific layout */

    UInt32                     stereoDisparityLayoutEnable;
    /**< Flag to enable/disable stereo view layout */

    UInt32                     tda3xxSvFsRotLayoutEnable;
    /**< Flag to enable/disable surround view + SOF layout */

    UInt32                     surroundViewStandaloneLayoutEnable;
    /**< Flag to enable/disable 2D/3D surround view standaone layout */

} GrpxSrcLink_CreateParams;

/******************************************************************************
*
*  Functions
*
*******************************************************************************
*/

/**
*******************************************************************************
 *
 * \brief Grpx Src link register and init
 *
 *    - Creates link task
 *    - Registers as a link with the system API
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 GrpxSrcLink_init();

/**
*******************************************************************************
 *
 * \brief Grpx Src link de-register and init
 *
 *    - deletes link task
 *    - De-registers as a link with the system API
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 GrpxSrcLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Grpx src link set default parameters for create time params
 *   This function does the following
 *      - memset create params object
 * \param  pPrm  [OUT]  GrpxLink Src Create time Params
 *
 *******************************************************************************
 */
static inline void GrpxSrcLink_CreateParams_Init(GrpxSrcLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
    pPrm->enableJeepOverlay = TRUE;
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/
