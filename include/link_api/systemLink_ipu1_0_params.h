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
 *  \ingroup PROCESSOR_LINK_API
 *  \defgroup SYSTEM_IPU1_0_LINK_API Processor Link API: IPU1 Core0
 *
 *  This module defines the control commands that are applicable to
 *  IPU1 Core0 processor.
 *
 *   @{
*/

/**
 *******************************************************************************
 *
 *  \file systemLink_ipu1_0_params.h
 *  \brief Processor Link API: IPU1 Core0
 *
 *******************************************************************************
*/

#ifndef _SYSTEM_LINK_IPU1_0_PARAMS_H_
#define _SYSTEM_LINK_IPU1_0_PARAMS_H_

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/systemLink_common.h>

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Configuration for sub-frame level processing at create time.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32 subFrameEnable;
    /**< TRUE : SubFrame level capture/processing is enabled.
     *   FALSE: SubFrame level capture/processing is disabled.
     *   Must be FALSE for multi-channel capture mode. */
    UInt32 numLinesPerSubFrame;
    /**< Number of lines per subframes.
     *
     *   MUST be multiple of the output size.
     *   Not valid, ignored for ancillary data capture.
     *
     *   In case of capture,
     *   SubFrame callback gets called after every numLinesPerSubFrame
     *   for every output stream, except ancillary data stream.
     *
     *   Ignored when subFrameEnable = FALSE */
} System_SubFrameParams;

/**
 *******************************************************************************
 *
 *  \brief Input frame Configuration parameters for DSS WB capture &
 *         processing at create time.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32                 wbInSourceWidth;
    /**< Input width of overlay feeding Writeback pipeline */
    UInt32                 wbInSourceHeight;
    /**< Input height of overlay feeding Writeback pipeline */
    UInt32                 wbInWidth;
    /**< input width for wb pipeline. Same as wbInSourceWidth if crop not
     *required */
    UInt32                 wbInHeight;
    /**< input height for wb pipeline. Same as wbInSourceHeight if crop not
     *required */
    UInt32                 wbPosx;
    /**< Input position x for WB pipeline */
    UInt32                 wbPosy;
    /**< Input position y  for WB pipeline */
    System_VideoDataFormat wbInSourceDataFmt;
    /**< output data format of Overlay which feeds wb pipeline */
    System_VideoScanFormat wbScanFormat;
    /**< output Scan format of Overlay which feeds wb pipeline */
    UInt32                 wbFieldMerge;
    /**< Field merge or field separated */
} System_DssWbInputParams;

/**
 *******************************************************************************
 *
 *  \brief Output frame Configuration parameters for DSS WB capture &
 *         processing at create time.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32                 wbWidth;
    /**< Output width in pixels for WB Pipeline.
         For TDA3xx, Must be output 32 pixels aligned */
    UInt32                 wbHeight;
    /**< Output height in pixels for WB Pipeline. */
    System_VideoDataFormat wbDataFmt;
    /**< YUV or RGB data format of wb pipeline output. */
    System_VideoScanFormat wbScanFormat;
    /**< Output Scan Format. For valid values see Fvid2_ScanFormat in
     *   starterware. */
    UInt32                 wbFieldMerge;
    /**< Field merge or field separated */
} System_DssWbOutputParams;

/**
 *******************************************************************************
 *
 *  \brief Structure containing crop configuration - used in Scaler and VCOMP.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32 cropStartX;
    /**< Horizontal offset from which picture needs to be cropped. */
    UInt32 cropStartY;
    /**< Vertical offset from which picture needs to be cropped. */
    UInt32 cropWidth;
    /**< Width of the picture to be cropped. */
    UInt32 cropHeight;
    /**< Height of the picture to be cropped. */
} System_CropConfig;

/**
 *******************************************************************************
 *
 *  \brief These are all scaler parameters exposed to the application.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32           bypass;
    /**< Scaler should be bypassed or not. */

    UInt32           nonLinear;
    /**< Flag to indicate whether linear or non-linear scaling is used for
     *   horizontal scaler. Non-linear scaling is available for polyphase
     *   filter only
     *   Example: Scaling from 16/9 aspect ratio to 4/3 aspect ratio.*/

    UInt32           stripSize;
    /**< Size of left and right strip for nonlinear horizontal scaling in terms
     *   of pixel. It must be set to zero for linear horz scaling. */
} System_ScConfig;

/**
 *******************************************************************************
 *
 *  \brief Application interface structure for programming the coefficients.
 *  Structure will be used for all drivers involving scalers.
 *
 *******************************************************************************
 */
typedef struct
{
    UInt32 hScalingSet;
    /**< Horizontal scaling coefficient set.
     *   For valid values see #System_ScCoeffSet. */
    UInt32 vScalingSet;
    /**< Vertical scaling coefficient set.
     *   For valid values see #System_ScCoeffSet. */
} System_ScCoeffParams;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
