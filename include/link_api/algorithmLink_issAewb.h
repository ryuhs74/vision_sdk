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
 * \defgroup ALGORITHM_LINK_ISS_AEWB Algorithm Plugin: ISS AEWB API
 *
 * \brief  This module has the interface for using ISS AEWB algorithm
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_issAewb.h
 *
 * \brief Algorithm Link API specific to ISS AEWB algorithm
 *
 * \version 0.0 (Feb 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_ISS_AEWB_H_
#define _ALGORITHM_LINK_ISS_AEWB_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
/* Making AEWB algorithm dependent on ISP configuration to avoid
   multiple copies of RGB2RGB structure and also by doing this,
   AEWB output structure can be accessible to Alg plugin, ISP link
   and to the VidSensor */
#include <include/link_api/issIspConfiguration.h>
#include <include/link_api/issM2mSimcopLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* Max number of H3A Planes/buffers and thus output parameters supported */
#define ALGORITHM_AEWB1_MAX_PLANES              (2U)

/* Maximum number of dynamic parameter configuration supported
   for Auto Exposure Algorithm */
#define ALGORITHM_AEWB1_MAX_AE_DYN_PARAMS       (10u)

/* maximal number of reference 1, actual use 17 */
#define ALGORITHM_AEWB1_MAX_REF1        (30u)

/* maximal number of reference 2, actual use 7 */
#define ALGORITHM_AEWB1_MAX_REF2        (15u)

/* maximal number of gray 2 used, actual use 4 */
#define ALGORITHM_AEWB1_MAX_GRAY        (4u)

/* DCC input buffer Size */
#define ALGORITHM_AEWB1_DCC_IN_BUF_SIZE (128U*1024U)

/**
    \ingroup LINK_API_CMD
    \addtogroup ALGORITHM_AEWB_LINK_API_CMD  Algorithm Plugin: ISS AEWB Control Commands

    @{
*/

/**
 *******************************************************************************
 * \brief Link CMD: Command to set the Auto Exposure Dynamic Params
 *
 *   \param AlgorithmLink_IssAewbAeDynamicParams *pPrm [IN] AE Dynamic Parameters.
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_SET_AE_DYNAMIC_PARAMS   (0x1000)

/**
 *******************************************************************************
 * \brief Link CMD: Command to set AWB Calibration Data
 *
 *   \param AlgorithmLink_IssAewbAwbCalbData *pPrm [IN] AWB Calibration Parameters.
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_SET_AWB_CALB_DATA       (0x1001)

/**
 *******************************************************************************
 * \brief Link CMD: Command to parse DCC bin file and set parameters in ISP
 *
 *   \param AlgorithmLink_IssAewbDccParams *pPrm [IN] DCC Parameters,
 *          contains pointer to DCC file and size of the DCC file.
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_PARSE_AND_SET_DCC_PARAMS (0x1002)

/**
 *******************************************************************************
 * \brief Link CMD: Command to get the DCC buffer parameters
 *        Used by the network tool kit to get the DCC input buffer, where
 *        DCC bin file is stored.
 *
 *   \param AlgorithmLink_IssAewbDccParams *pPrm [IN] DCC Parameters,
 *          contains pointer to DCC file and size of the DCC file.
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_GET_DCC_BUF_PARAMS (0x1003)

/**
 *******************************************************************************
 * \brief Link CMD: Command to set DCC camera Information
 *        Used for setting up DCC camera Information like camera ID
 *
 *   \param AlgorithmLink_IssAewbDccCameraInfo *pPrm [IN] DCC Parameters,
 *          contains pointer to Dcc Camera id.
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_SET_CAMERA_INFO    (0x1004)

/**
 *******************************************************************************
 * \brief Link CMD: Command to get 2A Parameters
 *                  Used for setting AE/AWB manual or Auto mode.
 *                  When in manual mode, fixed output parameters for AE/AWB
 *                  can be specified.
 *
 *        This ioctl can be used only if AEWB is enabled at create time
 *
 *   \param AlgorithmLink_IssAewb2AParams *pPrm [IN] 2A Parameters
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_SET_2A_PARAMS      (0x1005)

/**
 *******************************************************************************
 * \brief Link CMD: Command to get 2A Parameters
 *
 *  This ioctl can be used only if AEWB is enabled at create time
 *
 *   \param AlgorithmLink_IssAewb2AParams *pPrm [OUT] 2A Parameters
 *
 *   Supported only on TDA3xx Iss UseCase
 *
 *******************************************************************************
*/
#define ALGORITHM_AEWB_LINK_CMD_GET_2A_PARAMS      (0x1006)

/* @} */


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief Algorithm Callback function
 *******************************************************************************
 */

/**< Callback function for applying AEWB output to the ISP */
typedef Void (*AlgorithmLink_IssAewbConfig) (
            IssAewbAlgOutParams *algoOutPrms,
            Void *appData);

/**< Callback function for applying ISP Configuration after DCC Update */
typedef Void (*AlgorithmLink_IssAewbDccUpdate) (
    IssIspConfigurationParameters *ispCfgPrms,
    IssM2mSimcopLink_ConfigParams *simcopCfgPrm,
    Void *appData);

/*******************************************************************************
 *  \brief AEWB Algorithm Mode
 *******************************************************************************
 */
typedef enum {
    ALGORITHMS_ISS_AEWB_MODE_AWB       = 0,
    /**< Auto White Balance Mode only */
    ALGORITHMS_ISS_AEWB_MODE_AE,
    /**< Auto Exposure Mode only */
    ALGORITHMS_ISS_AEWB_MODE_AEWB,
    /**< Auto Exposure and Auto White Balance Mode */
    ALGORITHMS_ISS_AEWB_MODE_NONE,
    /**< None of AEWB Mode,
         Used when DCC Functionality is required, but not AEWB
         Also used when AEWB is dynamically enabled/disabled using DCC */
    ALGORITHMS_ISS_AEWB_MODE_MAX
    /**< Max mode value, used for error checking */
} AlgorithmLink_IssAewbMode;

/*******************************************************************************
 *  \brief H3A params that are used by ISP
 *******************************************************************************
 */
typedef struct {

    UInt32 winCountH;
    /**< Number of H3A windows in H-direction */
    UInt32 winCountV;
    /**< Number of H3A windows in V-direction */
    UInt32 winSizeH;
    /**< Width of each H3A window in H-direction */
    UInt32 winSizeV;
    /**< Height of each H3A window in H-direction */
    UInt32 winSkipH;
    /**< Number of pixels skipped inside each H3A window in H-direction */
    UInt32 winSkipV;
    /**< Number of pixels skipped inside each H3A window in V-direction */

} AlgorithmLink_IssAewbH3aParams;

/*******************************************************************************
 *  \brief Min/Max Range, used in AWB calibration data
 *******************************************************************************
 */
typedef struct {
    UInt32 min;
    /**< Min Value */
    UInt32 max;
    /**< Max Value */
} AlgorithmLink_IssAewbRange;

/*******************************************************************************
 *  \brief Sensor Specific Auto Exposure Dynamic Parameters
 *******************************************************************************
 */
typedef struct {
    AlgorithmLink_IssAewbRange exposureTimeRange[ALGORITHM_AEWB1_MAX_AE_DYN_PARAMS];
    /**< range of exposure time, unit is same with exposure time */
    AlgorithmLink_IssAewbRange apertureLevelRange[ALGORITHM_AEWB1_MAX_AE_DYN_PARAMS];
    /**< range of aperture level, unit is same with aperture level */
    AlgorithmLink_IssAewbRange sensorGainRange[ALGORITHM_AEWB1_MAX_AE_DYN_PARAMS];
    /**< range of sensor gain, unit is same with sensor gain */
    AlgorithmLink_IssAewbRange ipipeGainRange[ALGORITHM_AEWB1_MAX_AE_DYN_PARAMS];
    /**< range of IPIPE gain, unit is same with IPIPE gain */
    UInt32                     numAeDynParams;
    /**< Number of Valid Entries in above arrays */
    AlgorithmLink_IssAewbRange targetBrightnessRange;
    /**< range of target brightnes */
    UInt32                     targetBrightness;
    /**< target brightness value */
    UInt32                     threshold;
    /**< threshold for not using history brightness information */
    UInt32                     exposureTimeStepSize;
    /**< step size of exposure time adjustmnent */
    UInt32                     enableBlc;
    /**< TRUE enables Backlight compensation, disabled otherwise */
} AlgorithmLink_IssAewbAeDynamicParams;

/*******************************************************************************
 *  \brief 2A Parameters, Used by the DCC to control 2A parameters
 *         It could be used to enable/disable AE/AWB or used to fix
 *         output of AE/AWB
 *******************************************************************************
 */
typedef struct {
    UInt32                aeMode;
    /**< AE Mode, used to enable/disable AE.
         It could be either AUTO (0) or manual (1),
         Auto mode is default and runs the AE algorithms
         Manual mode uses below AE parameters value as AE output */
    UInt32                digitalGain;
    /**< Digital Gain for manual AE mode */
    UInt32                analogGain;
    /**< Analog Gain for manual AE mode */
    UInt32                expTime;
    /**< Exposure time for manual AE mode */
    UInt32                awbMode;
    /**< AWB Mode, used to enable/disable AWB.
         It could be either AUTO (0) or manual (1),
         Auto mode is default and runs the AWB algorithms
         Manual mode uses below AWB parameters value as AE output */
    UInt32                rGain;
    /**< Red color gain for manual AWB mode*/
    UInt32                gGain;
    /**< Green color gain for manual AWB mode*/
    UInt32                bGain;
    /**< Blue color gain for manual AWB mode*/

    UInt32                colorTemp;
    /**< Color Temparature for manual AWB mode*/
} AlgorithmLink_IssAewb2AParams;

/*******************************************************************************
 *  \brief Sensor Specific AWB calibration Data
 *         Caution: Do not change size of any array as this will be
 *         internally by the algorithm
 *******************************************************************************
 */
typedef struct {
    UInt32    numRef1;
    UInt32    numRef2;
    UInt32    numGray;

    UInt32    colorTemp1[ALGORITHM_AEWB1_MAX_REF1];

    Int32     wbReferenceCb[ALGORITHM_AEWB1_MAX_REF1][ALGORITHM_AEWB1_MAX_GRAY];
    Int32     wbReferenceCr[ALGORITHM_AEWB1_MAX_REF1][ALGORITHM_AEWB1_MAX_GRAY];

    UInt32    refGrayR1[ALGORITHM_AEWB1_MAX_REF1][ALGORITHM_AEWB1_MAX_GRAY];
    UInt32    refGrayG1[ALGORITHM_AEWB1_MAX_REF1][ALGORITHM_AEWB1_MAX_GRAY];
    UInt32    refGrayB1[ALGORITHM_AEWB1_MAX_REF1][ALGORITHM_AEWB1_MAX_GRAY];

    UInt32    refIndex2[ALGORITHM_AEWB1_MAX_REF2];
    UInt32    colorTemp2[ALGORITHM_AEWB1_MAX_REF2];

    UInt32    imgRef[ALGORITHM_AEWB1_MAX_REF2 * 1120];

    Int32     refCb2[ALGORITHM_AEWB1_MAX_REF2][ALGORITHM_AEWB1_MAX_GRAY];
    Int32     refCr2[ALGORITHM_AEWB1_MAX_REF2][ALGORITHM_AEWB1_MAX_GRAY];

    UInt32    refGrayR2[ALGORITHM_AEWB1_MAX_REF2][ALGORITHM_AEWB1_MAX_GRAY];
    UInt32    refGrayG2[ALGORITHM_AEWB1_MAX_REF2][ALGORITHM_AEWB1_MAX_GRAY];
    UInt32    refGrayB2[ALGORITHM_AEWB1_MAX_REF2][ALGORITHM_AEWB1_MAX_GRAY];


    /* Sensor specific tuning paramaters */
    UInt32    radius;

    Int32     lumaAwbMin;
    Int32     lumaAwbMax;

    UInt32    lowColorTempThreshold;

    UInt32    applyRgbAdjust;

    Int32     redAdjust;
    Int32     blueAdjust;

    UInt32    sb1;
    UInt32    sb2;

    UInt32    sbLowBound;

    UInt32    defaultTH;
    UInt32    defaultTMH;
    UInt32    defaultTML;
    UInt32    defaultTL;

    UInt32    defaultTHIndex;
    UInt32    defaultTMHIndex;
    UInt32    defaultTMLIndex;
    UInt32    defaultTLIndex;

    UInt32    bestGrayIndexDefault;
} AlgorithmLink_IssAewbAwbCalbData;

/*******************************************************************************
 *  \brief Alg Link create params
 *******************************************************************************
 */
typedef struct
{
    AlgorithmLink_CreateParams  baseClassCreate;
    /**< Base class create params. This structure should be first element */
    UInt32                      channelId;
    /**< Channel Id for which this AEWB algo is created */
    AlgorithmLink_IssAewbMode   mode;
    /**< Algorithm Mode */
    System_LinkInQueParams      inQueParams;
    /**< Input queue information */
    UInt32                      numOutBuffers;
    /**< Number of output Buffers */
    UInt32                      numH3aPlanes;
    /**< Number of H3A buffer, which will provided in a single new request */
    AlgorithmLink_IssAewbH3aParams h3aParams;
    /**< H3A params used by ISP. This MUST match the ISP setting */
    System_VideoDataFormat      dataFormat;
    /**< H3A input Bayer format, Must match with the ISP settings */
    UInt32                      numSteps;
    /**< AWB Algorithm Step size, After each numSteps, algo will run */

    /* AE Parameters */
    AlgorithmLink_IssAewbAeDynamicParams aeDynParams;
    /**< Array of Auto Exposure Dynamic Parameters */

    AlgorithmLink_IssAewbAwbCalbData *calbData;
    /**< Sensor Specific Calibration data for the AutoWhite Balance
         If set to NULL, default calibration data will be used */

    Void                        *appData;
    /**< Private Application data */
    AlgorithmLink_IssAewbConfig cfgCbFxn;
    /**< Callback function, AEWB algorithm calls this callback with the
         output parameters, this callback should apply output parameters to
         the appropriate hardware modules/sensors */

    UInt32                      isWdrEnable;
    /**< Flag to indicate whether WDR Merge is enabled or not,
         This flag is just used to call mergeCbFxn callback function and
         configure ISP */
    AlgorithmLink_IssAewbConfig mergeCbFxn;
    /**< Callback function to get the exposure ratio from sensor and
         apply merge parameters in ISP based on exposure ratio,
         used only when isWdrMergeEnable flag is set
         Called on every #numSteps frames processing,
         It could be set to null if fixed exposure ratio is used
         in sensor/isp */

    AlgorithmLink_IssAewbDccUpdate dccIspCfgFxn;
    /**< Callback function to update DCC Configuration in ISP,
         After reading ISP configuration from file, it will be updated
         in ISP using this function. */
    UInt32                         dccCameraId;
    /**< FVID2 ID of the Sensor/Camera, Used by the DCC Parser.
         DCC bin file will be used only if this id matches with the
         id in the bin file. */

    System_LinkMemAllocInfo memAllocInfo;
    /**< Memory alloc region info, used to pass user alloc memory address */

    UInt32                         enableDcc;
    /**< Flag to enable DCC support for this AEWB instance */
} AlgorithmLink_IssAewbCreateParams;

/**
 *******************************************************************************
 *   \brief Structure containing control parameters for AE Dynamic Parameters
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    AlgorithmLink_IssAewbAeDynamicParams *aeDynParams;
    /**< Auto Exposure Dynamic Parameters */

} AlgorithmLink_IssAewbAeControlParams;

/*******************************************************************************
 *  \brief Structure containing control parameters for AWB Calibration data
 *******************************************************************************
 */
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    AlgorithmLink_IssAewbAwbCalbData *calbData;
    /**< Sensor Specific Calibration data for the AutoWhite Balance */
} AlgorithmLink_IssAewbAwbControlParams;

/**
 *******************************************************************************
 *   \brief Structure containing control parameters for DCC
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    UInt8       *dccBuf;
    /**< Pointer to the DCC File */

    UInt32      dccBufSize;
    /**< DCC Buffer Size */

    IssIspConfigurationParameters *pIspCfg;
    /**< Pointer to ISP configuration */

    IssM2mSimcopLink_ConfigParams *pSimcopCfg;
    /**< Pointer to Simcop Config Parameters */
} AlgorithmLink_IssAewbDccControlParams;

/**
 *******************************************************************************
 *   \brief Structure containing control parameters for DCC Camera Information
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    UInt32 cameraId;
    /**< DCC Camera Id */

    UInt32 width;
    /**< Capture frame width */
    UInt32 height;
    /**< Capture frame height */
} AlgorithmLink_IssAewbDccCameraInfo;

/**
 *******************************************************************************
 *   \brief Structure containing control parameters for ISP Configuration
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    IssIspConfigurationParameters ispCfg;
    /**< Structure containing pointer to isp configuration */
} AlgorithmLink_IssAewbDccInitIspCfg;

/**
 *******************************************************************************
 *   \brief Structure containing control parameters for 2A State Information
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams     baseClassControl;
    /**< Base class control params */
    AlgorithmLink_IssAewb2AParams   aewb2APrms;
    /**< 2A Parameters */
} AlgorithmLink_IssAewb2AControlParams;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Set defaults for plugin create parameters
 *
 * \param pPrm  [OUT] plugin create parameters
 *
 *******************************************************************************
 */
static inline void AlgorithmLink_IssAewb_Init(
    AlgorithmLink_IssAewbCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_IPU_ALG_ISS_AEWB;

    pPrm->channelId = 0U;
    pPrm->appData = NULL;

    pPrm->inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->numH3aPlanes = 1;
    pPrm->numOutBuffers = 3;

    pPrm->h3aParams.winCountH = 16;
    pPrm->h3aParams.winCountV = 20;
    pPrm->h3aParams.winSizeH  = 32;
    pPrm->h3aParams.winSizeV  = 16;
    pPrm->h3aParams.winSkipH  = 3;
    pPrm->h3aParams.winSkipV  = 3;

    pPrm->dataFormat = SYSTEM_DF_BAYER_GRBG;
    pPrm->numSteps = 5u;
    pPrm->mode = ALGORITHMS_ISS_AEWB_MODE_AWB;
    pPrm->cfgCbFxn = NULL;

    pPrm->isWdrEnable = FALSE;
    pPrm->mergeCbFxn = NULL;
    pPrm->dccIspCfgFxn = NULL;

    pPrm->enableDcc = 1;
}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of AEWB algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_issAewb1_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
