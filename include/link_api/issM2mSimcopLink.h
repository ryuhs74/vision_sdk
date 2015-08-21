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
 *   \defgroup ISSM2MSIMCOP_LINK_API ISS M2M SIMCOP (LDC+VTNF) Link API
 *
 *   ISS M2M SIMCOP Link is used for SIMCOP operations available in ISS
 *   This link operates in M2M mode (Input Data read from memory, operation by
 *   SIMCOP (LDC and VTNF) and Output Data written back to memory)
 *
 *   This link can be operated in three primary modes
 *   - LDC only: Lens Distortion correction
 *   - VTNF only: Temporal noise filter
 *   - LDC + VTNF only: Both LDC and VTNF are active
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

#ifndef _ISSM2MSIMCOP_LINK_H_
#define _ISSM2MSIMCOP_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <fvid2/fvid2_dataTypes.h>
#include <vps/iss/vps_cfgldc.h>
#include <vps/iss/vps_cfgvtnf.h>


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
#define ISSM2MSIMCOP_LINK_MAX_CH     (4)

/**
 *******************************************************************************
 * \brief Indicates number of output buffers to be set to default
 *         value by the iss M2m isp link
 *******************************************************************************
*/
#define ISSM2MSIMCOP_LINK_NUM_BUFS_PER_CH_DEFAULT (3)


/* @} */

/* Control Command's    */

/**
    \ingroup LINK_API_CMD
    \addtogroup ISSM2MSIMCOP_LINK_API_CMD  ISS M2M SIMCOP Link Control Commands

    @{
*/

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To set configuration for LDC, VNTF
 *
 *          Needs be set by user. If not set then default values will be used.
 *          Use IssM2mSimcopLink_ConfigParams_Init() to set default params
 *
 *   \param IssM2mSimcopLink_ConfigParams *pConfig
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_CMD_SET_SIMCOPCONFIG           (0x5001)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To save/dump simcop frame into extra frame
 *                    Dumps the frame allocated at CreateTime
 *                    If extra frame buffer is not allocated at create time
 *                      returns Error
 *
 *      Can be used to save frame only when capture is running
 *
 *   \param None
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_CMD_SAVE_FRAME             (0x5002)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Return's pointer to saved frame
 *
 *   \param IssM2mSimcopLink_GetSaveFrameStatus   [OUT]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_CMD_GET_SAVE_FRAME_STATUS      (0x5003)

/* @} */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *  \brief  Enumerations for operating modes of ISS M2M Simcop Link
 *******************************************************************************
*/
typedef enum
{
    ISSM2MSIMCOP_LINK_OPMODE_LDC = 0,
    /**< LDC operation only */

    ISSM2MSIMCOP_LINK_OPMODE_VTNF,
    /**< VTNF operation only */

    ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF,
    /**< Both LDC and VTNF operations are required */

    ISSM2MSIMCOP_LINK_OPMODE_MAXNUM,
    /**< Maximum number of operating modes for this link */

    ISSM2MSIMCOP_LINK_OPMODE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} IssM2mSimcopLink_OperatingMode;


/*******************************************************************************
 *  Data structures
 *******************************************************************************
*/

typedef struct
{
    UInt32  chNum;
    /**< Channel ID for which these parameters are valid */

    /**< If the pointer is NULL: Indicates application is not interested
     *   in configuring or changing values.
     *   If the pointer is non-NULL:  Indicates application is providing new
     *   configuration.
     *   Note that, the structure (Memory) area pointed to these buffers are
     *   available for the link and they are not over-written,
     *   while the link is active.
     */

    vpsissldcConfig_t *ldcConfig;
    /**< Only Valid if
     *  IssM2mSimcopLink_CreateParams.channelParams
     *       [IssM2mSimcopLink_ConfigParams.chNum].operatingMode
     *           = ISSM2MSIMCOP_LINK_OPMODE_LDC
     *              or
     *             ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF
     *
     *  Following field in ldcConfig, need not be set by user
     *   and link implementation will over-ride user set values,
     *    vpsissldcConfig_t.isAdvCfgValid
     *    vpsissldcConfig_t.advCfg
     *    vpsissldcConfig_t.inputFrameWidth
     *    vpsissldcConfig_t.inputFrameHeight
     *
     *  Other fields in vpsissldcConfig_t MUST be set by user
     *
     */

    vpsissvtnfConfig_t *vtnfConfig;
    /**< Only Valid if
     *  IssM2mSimcopLink_CreateParams.channelParams
     *       [IssM2mSimcopLink_ConfigParams.chNum].operatingMode
     *           = ISSM2MSIMCOP_LINK_OPMODE_VTNF
     *              or
     *             ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF
     *
     *  Following fields in vtnfConfig, need not be set by user
     *   and link implementation will over-ride user set values,
     *    vpsissvtnfConfig_t.outDataFormat
     *    vpsissvtnfConfig_t.isAdvCfgValid
     *    vpsissvtnfConfig_t.advCfg.blockWidth
     *    vpsissvtnfConfig_t.advCfg.blockHeight
     *    vpsissvtnfConfig_t.advCfg.triggerSource
     *    vpsissvtnfConfig_t.advCfg.intrEnable
     *
     *  Below MUST be set by user,
     *   vpsissvtnfConfig_t.advCfg.roundBitCount
     *   vpsissvtnfConfig_t.advCfg.colorWeight1
     *   vpsissvtnfConfig_t.advCfg.colorWeight2
     *   vpsissvtnfConfig_t.advCfg.lut1[]
     *   vpsissvtnfConfig_t.advCfg.lut2[]
     */

} IssM2mSimcopLink_ConfigParams;

/**
 *******************************************************************************
 *
 * \brief ISS M2m Simcop link channel parameters
 *
 *        Note, for M2M SIMCOP link output data format, width, height
 *        is same as input data format, width, height
 *
 *        For LDC+VNTF mode  , input MUST be YUV420SP data format
 *        For VNTF ONLY mode , input MUST be YUV420SP data format
 *        For LDC ONLY mode  , input MUST be YUV420SP data format
 *
 *******************************************************************************
 */
typedef struct
{
    IssM2mSimcopLink_OperatingMode     operatingMode;
    /**< Refer IssM2mSimcopLink_OperatingMode for details */

    UInt32                             numBuffersPerCh;
    /**< Number of image buffers per output channel */

} IssM2mSimcopLink_ChannelParams;

/**
 *******************************************************************************
 *  \brief Information of saved data frame
 *******************************************************************************
 */
typedef struct
{
    UInt32  chId;
    /**< Channel id for which buffer information is requested */

    UInt32 isSaveFrameComplete;
    /**< TRUE: Frame is saved at address mentioned in 'bufAddr'
     *   FALSE: Frame is not yet saved, try after some time
     */

    UInt32 bufAddr;
    /**< Address where frame is saved */

    UInt32 bufSize;
    /**< Size of buffer where frame is saved */

} IssM2mSimcopLink_GetSaveFrameStatus;

/**
 *******************************************************************************
 * \brief ISS M2m Simcop link create time parameters
 *******************************************************************************
 */
typedef struct
{
    System_LinkInQueParams       inQueParams;
    /**< Input queue information */

    System_LinkOutQueParams      outQueParams;
    /**< Output queue information */

    IssM2mSimcopLink_ChannelParams  channelParams[ISSM2MSIMCOP_LINK_MAX_CH];
    /**< Parameters for each channel */

    UInt32                  allocBufferForDump;
    /**< [IN] Flag to allocate extra frame buffer for RAW dump
              1, extra frame buffer is allocated
              0, extra frame buffer is not allocated, so RAW frames
                 cannot be dumped */

    System_LinkMemAllocInfo memAllocInfo;
    /**< Memory alloc region info, used to pass user alloc memory address */

} IssM2mSimcopLink_CreateParams;

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
Int32 IssM2mSimcopLink_init();

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
Int32 IssM2mSimcopLink_deInit();

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
static inline void IssM2mSimcopLink_CreateParams_Init(IssM2mSimcopLink_CreateParams *pPrm)
{
    UInt32 chId;

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->allocBufferForDump = 1;

    for(chId = 0; chId < ISSM2MSIMCOP_LINK_MAX_CH; chId++)
    {
        pPrm->channelParams[chId].operatingMode   = ISSM2MSIMCOP_LINK_OPMODE_LDC_VTNF;
        pPrm->channelParams[chId].numBuffersPerCh = ISSM2MSIMCOP_LINK_NUM_BUFS_PER_CH_DEFAULT;
    }
}

static inline void IssM2mSimcopLink_ConfigParams_Init(IssM2mSimcopLink_ConfigParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
