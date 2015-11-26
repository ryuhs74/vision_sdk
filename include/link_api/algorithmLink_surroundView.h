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
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_SURROUND_VIEW_API Algorithm Plugin: SURROUND VIEW \
 *                                        API
 *
 * \brief  This module has the interface for the algorithm plugin which uses
 *         DMA to perform the function of Software mosacing
 *
 *         This plugin can run on DSP, A15 and IPU1
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_surroundView.h
 *
 * \brief Algorithm Plugin: SURROUND VIEW API
 *
 * \version 0.0 (Aug 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_SURROUND_VIEW_H_
#define _ALGORITHM_LINK_SURROUND_VIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <src/utils_common/include/utils_lut.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Maximum number of windows in Mosaic layout
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_SURROUND_VIEW_MAX_WINDOWS    (8)

/**
 *******************************************************************************
 *
 *   \brief Invalid channel ID
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_SURROUND_VIEW_INVALID_CH_ID  ((UInt32)-1)
/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup ALGORITHM_LINK_SURROUND_VIEW_API_CMD Algorithm Plugin: SURROUND VIEW Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Alg Link Config CMD: Set SW Mosaic Layout parameters
 *
 *   \param AlgorithmLink_SurroundViewLayoutParams [IN]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *z
 *******************************************************************************
 */
#define ALGORITHM_LINK_SURROUND_VIEW_CONFIG_CMD_SET_LAYOUT_PARAMS     (0x0001)

/**
 *******************************************************************************
 *
 *   \brief Alg Link Config CMD: Get SW Mosaic Layout parameters
 *
 *   \param AlgorithmLink_SurroundViewLayoutParams [OUT]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_SURROUND_VIEW_CONFIG_CMD_GET_LAYOUT_PARAMS     (0x0002)

/* @} */


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

typedef enum
{
	LUT_VIEW_INFO_FULL_VIEW,
	LUT_VIEW_INFO_FULL_VIEW_LUT,
	LUT_VIEW_INFO_SIDE_VIEW,
	LUT_VIEW_INFO_SIDE_VIEW_LUT,
	LUT_VIEW_INFO_TOP_VIEW,
	LUT_VIEW_INFO_TOP_A00,
	LUT_VIEW_INFO_TOP_A01,
	LUT_VIEW_INFO_TOP_A02,
	LUT_VIEW_INFO_TOP_A03,
	LUT_VIEW_INFO_TOP_A04,
	LUT_VIEW_INFO_TOP_A05,
	LUT_VIEW_INFO_TOP_A06,
	LUT_VIEW_INFO_TOP_A07,
	LUT_VIEW_INFO_MAX
}LUT_VIEW_INFO;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief SW Mosaic window information
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 startX;
    /**< X-position in input frame from where to copy */

    UInt32 startY;
    /**< Y-position in input frame from where to copy */

    UInt32 width;
    /**< Window width,
     *   if window width < input width, cropped input is copied
     *   if window width > input width, rest of window is filled with blank data
     */

    UInt32 height;
    /**< Window height,
     *   if window height < input height, cropped input is copied
     *   if window height > input height, rest of window is filled with blank data
     */
    UInt32 pitch;


} AlgorithmLink_SurroundViewLutInfo;

/**
 *******************************************************************************
 *
 *   \brief SW Mosaic layout parameters
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    UInt32 numWin;
    /**< Number of windows in the SW Mosaic */

    AlgorithmLink_SurroundViewLutInfo lutViewInfo
                [LUT_VIEW_INFO_MAX];
    /**< Information of individual window's in the output */

    UInt32 outBufWidth;
    /**< MUST be <= AlgorithmLink_SurroundViewCreateParams.maxOutBufWidth */

    UInt32 outBufHeight;
    /**< MUST be <= AlgorithmLink_SurroundViewCreateParams.maxOutBufHeight */

	UInt32* Basic_frontView;
	UInt32* Basic_rearView;
	UInt32* Basic_leftSideView;
	UInt32* Basic_rightSideView;
	UInt32* Basic_frontNT;
	UInt32* Basic_rearNT;
	UInt32* Basic_leftNT;
	UInt32* Basic_rightNT;
	UInt32* cmaskNT;
	UInt32* Basic_frontFullView;
	UInt32* Basic_rearFullView;

    UInt32* psingleViewLUT;
    UInt32 singleViewInputChannel;
    AlgorithmLink_SurroundViewLutInfo* psingleViewLUTInfo;
    AlgorithmLink_SurroundViewLutInfo* psingleViewInfo;

} AlgorithmLink_SurroundViewLayoutParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for DMA SW Mosaic
 *
 *******************************************************************************
 */
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params */

    System_LinkInQueParams    inQueParams;
    /**< Input queue information */

    System_LinkOutQueParams   outQueParams;
    /**< Output queue information */

    AlgorithmLink_SurroundViewLayoutParams initLayoutParams;
    /**< Initial window layout parameters */

    UInt32 maxOutBufWidth;
    /**< Max possible output buffer width */

    UInt32 maxOutBufHeight;
    /**< Max possible output buffer height */

    UInt32 numOutBuf;
    /**< Number of buffer to allocate for the output */

    UInt32 useLocalEdma;
    /**< Flag to control if system DMA controller should be used or local DMA
     *   controller should be used
     *   FALSE, use system DMA controller
     *   TRUE, use local EDMA controller
     *
     *   NOTE: local EDMA controller is available for DSP only
     *         for M4, A15 always system DMA controller will be used
     */
    ViewMode					sViewmode;

} AlgorithmLink_SurroundViewCreateParams;



/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Set default values for Layout parameters
 *
 *******************************************************************************
 */
static inline Void AlgorithmLink_SurroundViewLayoutParams_Init(
                    AlgorithmLink_SurroundViewLayoutParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassControl.size = sizeof(*pPrm);
    pPrm->baseClassControl.controlCmd
            = ALGORITHM_LINK_SURROUND_VIEW_CONFIG_CMD_SET_LAYOUT_PARAMS;

    pPrm->numWin = 0;
    pPrm->outBufWidth  = 1920;
    pPrm->outBufHeight = 1080;
}

/**
 *******************************************************************************
 *
 * \brief Set default values for create parameters
 *
 *******************************************************************************
 */
static inline Void AlgorithmLink_SurroundViewCreateParams_Init(
                    AlgorithmLink_SurroundViewCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_IPU_ALG_SURROUND_VIEW;
    pPrm->maxOutBufWidth = 1920;
    pPrm->maxOutBufHeight = 1080;
    pPrm->numOutBuf = 4;
    pPrm->useLocalEdma = FALSE;

    AlgorithmLink_SurroundViewLayoutParams_Init(&pPrm->initLayoutParams);
}

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of frame copy algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_SurroundView_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
