/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup ISSM2MISP_LINK_API ISS M2M ISP Link API
 *
 *
 *   ISS M2M Isp Link is used for ISP operations available in ISS
 *   This link operates in M2M mode (Input Data read from memory, operation by
 *   ISP and Output Data written back to memory)
 *
 *   This link can be operated in two main modes
 *   - ISSM2MISP_LINK_OPMODE_12BITLINEAR: This
 *     mode is typically used for RAW Bayear to YUV format conversion with
 *     relevant image signal processing.
 *   - ISSM2MISP_LINK_OPMODE_20BITWDR: This
 *     mode is typically used for Wide Dynamic Range operations.
 *
 *   This link can operate on multiple channels.
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file issM2mIspLink.h
 *
 * \brief ISS M2M Isp link API public header file.
 *
 * \version 0.0 (Jun 2014) : [PS] First version
 * \version 0.1 (Aug 2014) : [PS] Addressed review comments given by team
 *
 *******************************************************************************
 */

#ifndef _ISSM2MISP_LINK_H_
#define _ISSM2MISP_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <include/link_api/issIspConfiguration.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 * \brief Max Channels of operation
 *******************************************************************************
*/
#define ISSM2MISP_LINK_MAX_CH     (4U)

/**
 *******************************************************************************
 *
 * \brief Indicates number of output buffers to be set to default
 *         value by the link
 *******************************************************************************
*/
#define ISSM2MISP_LINK_NUM_BUFS_PER_CH_DEFAULT (3U)


/* @} */

/* Control Command's    */

/**
    \ingroup LINK_API_CMD
    \addtogroup ISSM2MISP_LINK_API_CMD  ISS M2M ISP Link Control Commands

    @{
*/

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To set configuration for different modules of ISP
 *
 *          Needs be set by user before calling System_linkStart()
 *
 *   \param IssIspConfigurationParameters *pIspConfig
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSM2MISP_LINK_CMD_SET_ISPCONFIG             (0x5001)


/**
 *******************************************************************************
 *
 *   \brief Link CMD: To set AE and AWB parameters and set the ISP
 *          configuration accordingly
 *
 *          AEWB algorithm calls this ioctl through Vid_Sensor layer
 *
 *   \param IssAewbAlgOutParams *pAewbConfig
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSM2MISP_LINK_CMD_SET_AEWB_PARAMS           (0x5002)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To set ISP merge parameters based on the exposure ratio
 *
 *          AEWB algorithm calls this ioctl through Vid_Sensor layer
 *
 *   \param IssAewbAlgOutParams *pAewbConfig
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSM2MISP_LINK_CMD_SET_WDR_MERGE_PARAMS      (0x5003)


/* @} */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for operating modes of ISS M2M Link
 *
 *          List of operating modes
 *
 *******************************************************************************
*/
typedef enum
{
    ISSM2MISP_LINK_OPMODE_12BIT_LINEAR = 0,
    /**< upto 12-bit linear mode of operation */

    ISSM2MISP_LINK_OPMODE_1PASS_WDR,
    /**< Single Pass WDR mode of operation */

    ISSM2MISP_LINK_OPMODE_2PASS_WDR,
    /**< Two Pass WDR mode of operation */

    ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED,
    /**< Two Pass WDR Line Interleaved mode */

    ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME,
    /**< 12Bit Mononchroma mode,
         In this mode, ISP reads 12bit data,
            does few processing like DPC/NSF3/Black Level subtraction/GLBCE
            and outputs 8bit monochroma data from resizer */

    ISSM2MISP_LINK_OPMODE_MAXNUM,
    /**< Maximum number of operating modes for this link */

    ISSM2MISP_LINK_OPMODE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} IssM2mIspLink_OperatingMode;

/**
 *******************************************************************************
 *  \brief  Enumerations for output queue IDs
 *******************************************************************************
*/
typedef enum
{
    ISSM2MISP_LINK_INPUTQUE_RAW_IMAGE = 0,
    /**< Input queue for raw image input to this link */

    ISSM2MISP_LINK_INPUTQUE_MAXNUM,
    /**< Number of Input queues for this link */

    ISSM2MISP_LINK_INPUTQUE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} IssM2mIspLink_InputQueId;

/**
 *******************************************************************************
 *  \brief  Enumerations for output queue IDs
 *******************************************************************************
*/
typedef enum
{
    ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A = 0,
    /**< Output queue for image output of re-sizer A */

    ISSM2MISP_LINK_OUTPUTQUE_H3A,
    /**< Output queue for H3A statistics */

    ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B,
    /**< Output queue for image output of re-sizer B */

    ISSM2MISP_LINK_OUTPUTQUE_MAXNUM,
    /**< Number of Output queues for this link */

    ISSM2MISP_LINK_OUTPUTQUE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} IssM2mIspLink_OutputQueId;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
*/

/**
 *******************************************************************************
 * \brief ISS M2m Isp link output image format
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                           heightRszA;
    /**< Height of the output frame for Re-sizer.
     *   Don't care if  enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A] = 0
     */

    UInt32                           widthRszA;
    /**< Width of the output frame for Re-sizer.
     *   Don't care if  enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A] = 0
     */

    UInt32                           heightRszB;
    /**< Height of the output frame for Re-sizer.
     *   Don't care if  enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B] = 0
     */

    UInt32                           widthRszB;
    /**< Width of the output frame for Re-sizer.
     *   Don't care if  enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B] = 0
     */

    UInt32                           winWidthH3a;
    /**< Width of H3A window, this parameter is mainly used to
     *    allocate output buffer for H3A
     *
     *   Actually H3A info is provided
     *   via IssIspConfigurationParameters->aewbCfg
     */

    UInt32                           winHeightH3a;
    /**< Height of H3A window, this parameter is mainly used to
     *    allocate output buffer for H3A
     *
     *   Actually H3A info is provided
     *   via IssIspConfigurationParameters->aewbCfg
     */


    System_VideoDataFormat           dataFormat;
    /**< Output Frame data Format.
     *   Only following output data formats are supported
     *       SYSTEM_DF_YUV422I_YUYV
     *       SYSTEM_DF_YUV420SP_UV,
     */

} IssM2mIspLink_OutputParams;

typedef struct IssM2mIspLink_WdrOffsetParams
{
    UInt32 longLineOffset;
    /**< In case of WDR Line Interleaved offset, this is used to get
         the start offset for the long channel
         Not used for other operating modes */
    UInt32 shortLineOffset;
    /**< In case of WDR Line Interleaved offset, this is used to get
         the start offset for the long channel
         Not used for other operating modes */
    UInt32 longPixelOffset;
    /**< In WDR Line Interleaved mode, this is used to get the the
         start pixel offset for the long exposure channel. This is used
         in addition to #longLineOffset.
         Not used for other operating modes */
    UInt32 shortPixelOffset;
    /**< In WDR Line Interleaved mode, this is used to get the the
         start pixel offset for the short exposure channel. This is used
         in addition to #shortLineOffset.
         Not used for other operating modes */
    UInt32 width;
    /**< In case of WDR Line Interleaved offset,
         this is used to set the ISP frame size
         This is added because input frame from capture could
         have big frame */
    UInt32 height;
    /**< In case of WDR Line Interleaved offset,
         this is used to set the ISP frame size
         This is added because input frame from capture could
         have big frame */
} IssM2mIspLink_WdrOffsetParams_t;

/**
 *******************************************************************************
 * \brief ISS M2m Isp link output image format
 *
 *******************************************************************************
*/
typedef struct
{
    IssM2mIspLink_OperatingMode        operatingMode;
    /**< Refer IssM2mIspLink_OperatingMode for details */

    IssM2mIspLink_WdrOffsetParams_t    wdrOffsetPrms;
    /**< WDR Offset parameters for Line Interleaved WDR mode */

    System_BitsPerPixel                inBpp;
    /**< Input Bits per pixel
     *   - As output by image censor
     *   - In case of 16-bit WDR and 20-bit WDR mode
     *     - This specifies BPP after companding
     */

    System_BitsPerPixel                decmpBpp;
    /**< Input Bits per pixel after decompaning
     *   - Only valid in 16-bit WDR and 20-bit WDR mode
     *   - Ignored in 12-bit linear mode
     */

    UInt32                             numBuffersPerCh;
    /**< Number of image buffers per output channel */

    UInt32                             enableOut[ISSM2MISP_LINK_OUTPUTQUE_MAXNUM];
    /**< enableOut[x] = 1, enable the corresponding output
     *   enableOut[x] = 0, disable the corresponding output.
     *   For possible value of x, refer enum IssM2mIspLink_OutputQueId
     */

    IssM2mIspLink_OutputParams         outParams;
    /**< Parameters for output queue */

} IssM2mIspLink_ChannelParams;

/**
 *******************************************************************************
 * \brief ISS M2m Isp link create time parameters
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    System_LinkInQueParams       inQueParams[ISSM2MISP_LINK_INPUTQUE_MAXNUM];
    /**< Input queue information */

    System_LinkOutQueParams      outQueParams[ISSM2MISP_LINK_OUTPUTQUE_MAXNUM];
    /**< Output queue information */

    IssM2mIspLink_ChannelParams  channelParams[ISSM2MISP_LINK_MAX_CH];
    /**< Parameters for each channel */

    System_LinkMemAllocInfo memAllocInfo;
    /**< Memory alloc region info, used to pass user alloc memory address */

} IssM2mIspLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Iss M2m Isp Link register and init
 *
 * Creates the tasks for the link. Registers Link within System with
 * unique link ID and callback functions.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_init();

/**
 *******************************************************************************
 *
 * \brief Iss M2m Isp Link de-register and de-init
 *
 * Delete the tasks and de-registers itself from the system.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Set defaults for creation time parameters
 *
 *  Currently defaults are set for 12 bit linear processing with output format of
 *  SYSTEM_DF_YUV420SP_UV.
 *  For any other use case, this function needs to be called
 *  and then change the required parameter accordingly.
 *
 * \param  pPrm [OUT] Create parameters for this link.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline void IssM2mIspLink_CreateParams_Init(IssM2mIspLink_CreateParams *pPrm)
{
    UInt32 chId, queId;
    IssM2mIspLink_ChannelParams *pChPrm;

    memset(pPrm, 0, sizeof(*pPrm));

    for(queId=0; queId<ISSM2MISP_LINK_INPUTQUE_MAXNUM; queId++)
    {
        pPrm->inQueParams[queId].prevLinkId = SYSTEM_LINK_ID_INVALID;
    }

    for(queId=0; queId<ISSM2MISP_LINK_OUTPUTQUE_MAXNUM; queId++)
    {
        pPrm->outQueParams[queId].nextLink = SYSTEM_LINK_ID_INVALID;
    }


    for(chId = 0; chId < ISSM2MISP_LINK_MAX_CH; chId++)
    {
        pChPrm = &pPrm->channelParams[chId];

        pChPrm->inBpp = SYSTEM_BPP_BITS12;
        pChPrm->decmpBpp = SYSTEM_BPP_BITS20;
        pChPrm->operatingMode = ISSM2MISP_LINK_OPMODE_12BIT_LINEAR;
        pChPrm->numBuffersPerCh = ISSM2MISP_LINK_NUM_BUFS_PER_CH_DEFAULT;
        pChPrm->enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A]  = 1;
        pChPrm->outParams.heightRszA = 720;
        pChPrm->outParams.widthRszA  = 1280;
        pChPrm->outParams.winWidthH3a = 16;
        pChPrm->outParams.winHeightH3a = 16;
        pChPrm->outParams.dataFormat = SYSTEM_DF_YUV420SP_UV;
    }
}

static inline void IssM2mIspLink_ConfigParams_Init(IssIspConfigurationParameters *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
