/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _chains_vipStereoCameraDisplay_Defines_H_
#define _chains_vipStereoCameraDisplay_Defines_H_

#define ALIGN(a,b) ((((a) + ((1<<(b)) - 1)) >> (b)) <<(b))

/* The param this command takes is Stereo_ConfigurableCreateParams*/
#define SYSTEM_LINK_CMD_STEREO_SET_PARAM                        (0x7000)
#define DISPARITY_TI_LEFT_TO_RIGHT                              0
#define DISPARITY_TI_RIGHT_TO_LEFT                              1

/* The parameters set the initial values of the stereo post-processing algorithm and can be set at runtime using the network control tool
with command stereo_set_dynamic_params. Please refer to the VisionSDK_NetworkTools_UserGuide.docx for usage .
The effects of the command are instantaneous.
 */
#define POSTPROC_COST_MAX_THRESHOLD                             95
#define POSTPROC_CONF_MIN_THRSESHOLD                            98
#define POSTPROC_HOLEFILLING_STRENGTH                           0
#define POSTPROC_TEXTURE_LUMALOTHRESH                           0
#define POSTPROC_TEXTURE_LUMAHITHRESH                           100
#define POSTPROC_TEXTURE_THRESHOLD                              85
#define POSTPROC_LEFTRIGHT_MAXDIFF_THRESHOLD                    255
#define POSTPROC_MAX_DISP_DISSIMILARITY                         2
#define POSTPROC_MIN_CONFIDENT_N_SEG                            2
#define POSTPROC_TEMPORAL_FILTER_NUM_FRAMES                     3
#define POSTPROC_MIN_DISPARITY_DISPLAY                          0
#define POSTPROC_COLORMAP_INDEX                                 0 /* 0: multi-color map, 1: blue tone color map */

#define CAPTURE_SENSOR_WIDTH                                    1280
#define CAPTURE_SENSOR_HEIGHT                                   720
#define SOFTISP_CROPPED_INPUT_HEIGHT                            664
#define SOFTISP_OUTPUT_HEIGHT                                   660

/* The parameters set the initial values of the stereo census transform and disparity algorithms and can be set at runtime using the network control tool
with command stereo_set_params. The effect of the command will only be applied before a use case is started. Once a use case is started, the effec of the command
will only be applied if the use case is stopped and restarted.
Please refer to the VisionSDK_NetworkTools_UserGuide.docx for usage.
 */
#define NUM_DISPARITIES                                         128
#define DISPARITY_STEP_SIZE                                     4
#define DISPARITY_OUTPUT_ROI_WIDTH                              640
#define DISPARITY_OUTPUT_ROI_HEIGHT                             360
#define DISPARITY_INPUT_BIT_DEPTH                               32
#define DISPARITY_WIN_WIDTH                                     15
#define DISPARITY_WIN_HEIGHT                                    15
#define DISPARITY_SEARCH_DIR                                    DISPARITY_TI_LEFT_TO_RIGHT
#define DISPARITY_EXRA_RIGHT_LEFT_DISP_MAP                      0

#define CENSUS_INPUT_BIT_DEPTH                                  8
#define CENSUS_WIN_WIDTH                                        9
#define CENSUS_WIN_HEIGHT                                       9
#define CENSUS_WIN_HORZ_STEP                                    2
#define CENSUS_WIN_VERT_STEP                                    2

#define REMAP_OUTPUT_BLOCK_WIDTH                                128
#define REMAP_OUTPUT_BLOCK_HEIGHT                               8

#define STEREO_CALIB_LUT_QSPI_OFFSET                            (29U*1024U*1024U)
#define STEREO_CALIB_LUT_HEADER_SIZE                            (16U)
#define STEREO_CALIB_LUT_TAG_ID                                 (0x00CCAABBU)

/**
 ******************************************************************************
 *
 * \brief This structure gathers all the different image dimensions used throughout the
 *          stereo-vision processing. It is initialized by the function ChainsCommon_Stereo_initImageDims()
 *
 ******************************************************************************
 */
typedef struct {
    UInt32 disparityOutputRoiWidth; /* Input parameter */
    UInt32 disparityOutputRoiHeight; /* Input parameter */

    UInt32 disparityInputImageWidth;
    UInt32 disparityInputImageHeight;
    UInt32 disparityInputLeftImageStartX;
    UInt32 disparityInputLeftImageStartY;
    UInt32 disparityInputRightImageStartX;
    UInt32 disparityInputRightImageStartY;
    UInt32 censusOutputRoiWidth;
    UInt32 censusOutputRoiHeight;
    UInt32 censusInputImageWidth;
    UInt32 censusInputImageHeight;
    UInt32 censusInputImageStartX;
    UInt32 censusInputImageStartY;
    UInt32 remapImageWidth;
    UInt32 remapImageHeight;
    UInt32 origRoiStartX;
    UInt32 origRoiStartY;
}StereoImageDims;
extern StereoImageDims gStereoImDims;


/*NOTE: This struct is also defined in netwrork_ctrl tools in network_ctrl_handle_stereo_set_params.c .
It contains the parameters of the census transform and disparity computation algorithms that are set
once a use case is started. These parameters cannot be changed until the use case is stopped and then restarted.
Please make sure the two are matching, if any changes are made.
 */
typedef struct
{
    UInt32              numDisparities;
    UInt32              disparityStepSize;
    Uint32              disparitySearchDir;
    UInt32              disparitySupportWinWidth;
    UInt32              disparitySupportWinHeight;
    UInt32              leftRightCheckEna;
    UInt32              censusWinWidth;
    UInt32              censusWinHeight;
    UInt32              censusWinHorzStep;
    UInt32              censusWinVertStep;
    UInt32              postproc_colormap_index;
}Stereo_ConfigurableCreateParams;

extern Stereo_ConfigurableCreateParams gStereoParams;

/**
 ******************************************************************************
 *
 * \brief set default parameters for stereo. If user passes any parameters then these are 
 *          configured instead.
 *
 * \param  pPrm  [IN]  Stereo specific Params
 *
 ******************************************************************************
 */
static inline void Stereo_CreateParams_Init
(Stereo_ConfigurableCreateParams *pPrm)
{
    if(pPrm == NULL)
    {
        gStereoParams.numDisparities 
        = NUM_DISPARITIES;
        gStereoParams.disparitySearchDir
        = DISPARITY_SEARCH_DIR;
        gStereoParams.disparityStepSize
        = DISPARITY_STEP_SIZE;
        gStereoParams.disparitySupportWinWidth
        = DISPARITY_WIN_WIDTH;
        gStereoParams.disparitySupportWinHeight
        = DISPARITY_WIN_HEIGHT;
        gStereoParams.leftRightCheckEna
        = DISPARITY_EXRA_RIGHT_LEFT_DISP_MAP;
        gStereoParams.censusWinWidth
        = CENSUS_WIN_WIDTH;
        gStereoParams.censusWinHeight
        = CENSUS_WIN_HEIGHT;
        gStereoParams.censusWinHorzStep
        = CENSUS_WIN_HORZ_STEP;
        gStereoParams.censusWinVertStep
        = CENSUS_WIN_VERT_STEP;
        gStereoParams.postproc_colormap_index 
        = POSTPROC_COLORMAP_INDEX;
    }
    else
    {
        memcpy(&gStereoParams, pPrm, sizeof(Stereo_ConfigurableCreateParams));
    }
}

#endif /* _chains_vipStereoCameraDisplay_Defines_H_ */
