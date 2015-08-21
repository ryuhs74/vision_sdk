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
 *   \addtogroup ISSM2MISP_LINK_API
 *
 *   This file defines structure for ISS ISP configuration.
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file issIspConfiguration.h
 *
 * \brief ISS Isp Configuration structure definition file
 *
 * \version 0.0 (Jun 2014) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ISSISPCONFIG_H_
#define _ISSISPCONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <vps/iss/vps_isscommon.h>
#include <vps/iss/vps_cfgcal.h>
#include <vps/iss/vps_cfgcnf.h>
#include <vps/iss/vps_cfgglbce.h>
#include <vps/iss/vps_cfgh3a.h>
#include <vps/iss/vps_cfgipipe.h>
#include <vps/iss/vps_cfgipipeif.h>
#include <vps/iss/vps_cfgisif.h>
#include <vps/iss/vps_cfgisp.h>
#include <vps/iss/vps_cfgldc.h>
#include <vps/iss/vps_cfgnsf3.h>
#include <vps/iss/vps_cfgrsz.h>
#include <vps/iss/vps_cfgsimcop.h>
#include <vps/iss/vps_cfgvtnf.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief IPIPE 3D Lut table Format
 *
 * There are two formats supported by the 3D Lut table, ie
 *******************************************************************************
*/
typedef enum
{
    ISS_ISP_IPIPE_3D_LUT_FORMAT_RGB = 0,
    /**< Lut contains RGB table for 3D,
         this table format is not directly supported by the IP, so it
         requires conversion, ISP link does this conversion. */
    ISS_ISP_IPIPE_3D_LUT_FORMAT_BANK
    /**< Lut contains table of four arrays for each bank,
         this table format is supported by the IP */
} IssIspIpipe3dLutFormat_t;


/*******************************************************************************
 *  Data structures
 *******************************************************************************
*/

/**
 *******************************************************************************
 * \brief Advanced parameters for re-sizer operation
 *
 *  These are advanced parameters for re-sizer operation. In create params init
 *  default values for these will be configured. If application is interested,
 *  then after init, values for these can be altered.
 *******************************************************************************
*/
typedef struct
{
    vpsissRszFlipCtrl_t flipCtrl;
    /**< Used to flip the output in horizontal and/or vertical direction */

    vpsissRszScaleMode_t scaleMode;
    /**< Rescaled Mode, could be either Normal or Downscale mode */

    vpsissRszFiltConfig_t filtCfg;
    /**< Structure containing Filter configuration for luma and chroma on
         horizontal and vertical side */

    vpsissRszIntsConfig_t intensityCfg;
    /**< Structure containing intensity configuration for luma and chroma on
         horizontal and vertical side */

    UInt32 alpha;
    /**< 8 Bit Alpha value (8-bits lsb is valid), in the case of SYSTEM_DF_ARGB32_8888 data format.
     *   For other formats, this parameter is don't care.
     */

} IssIspRszParams;

/**
 *******************************************************************************
 * \brief ISS ISP Configuration Parameters
 *******************************************************************************
*/
typedef struct
{
    UInt32 channelId;
    /**< Channel Id to which this configuration needs to be applied */

    /**< If the pointer is NULL: Indicates application is not interested in configuring values.
     *   If the pointer is non-NULL:  Indicates application is providing values to configure them.
     *   Note that, the structure (Memory) area pointed to these buffers are available for the link
     *   and they are not over-written, while the link is active.
     */
    vpsissCnfConfig_t             *cnfCfg;

    vpsissGlbceConfig_t           *glbceCfg;
    vpsissGlbcePerceptConfig_t    *glbceFwdPerCfg;
    vpsissGlbcePerceptConfig_t    *glbceRevPerCfg;
    vpsissGlbceWdrConfig_t        *glbceWdrCfg;

    vpsissH3aAewbConfig_t         *aewbCfg;
/*  vpsissH3aAfConfig_t           *afCfg; */

    vpsissIpipeInConfig_t         *ipipeInputCfg;
    vpsissIpipeYuvPhsConfig_t     *yuvPhsCfg;
    vpsissIpipeRgb2RgbConfig_t    *rgb2rgb1Cfg;
    vpsissIpipeRgb2RgbConfig_t    *rgb2rgb2Cfg;
    vpsissIpipeRgb2YuvConfig_t    *rgb2yuvCfg;
    vpsissIpipeWbConfig_t         *wbCfg;
    vpsissIpipeCfaConfig_t        *cfaCfg;
    vpsissIpipeDpcOtfConfig_t     *dpcOtfCfg;
    vpsissIpipeDpcLutConfig_t     *dpcLutCfg;
    vpsissIpipeGammaConfig_t      *gammaCfg;
    vpsissIpipe3DLutConfig_t      *lut3d;
    IssIspIpipe3dLutFormat_t       ipipe3dLutFormat;
    vpsissIpipeEeConfig_t         *eeCfg;
    vpsissIpipeGicConfig_t        *gicCfg;
    vpsissIpipeLscConfig_t        *lscCfg;
    vpsissIpipeNf2Config_t        *nf1Cfg;
    vpsissIpipeNf2Config_t        *nf2Cfg;

    vpsissIpipeifLutConfig_t      *ipipeifLut;
    vpsissIpipeifWdrCfg_t         *ipipeifWdrCfg;
    vpsissIpipeifDeCompandInsts_t *ipipeifCmpDecmpCfg;
/*  vpsissIpipeifDpcCfg_t         *ipipeifDpcCfg; */

/*  vpsissIsifVfdcConfig_t        *isifVfdcCfg; */
    vpsissIsifGainOfstConfig_t    *isifWbCfg;
    vpsissIsifBlackClampConfig_t  *isifBlkClampCfg;
    vpsissIsif2DLscConfig_t       *isif2DLscCfg;

    vpsissNsf3Config_t            *nsf3vCfg;

/*  IssIspRszParams               *rszCfg; */
} IssIspConfigurationParameters;

/*******************************************************************************
 *  \brief Structure of AEWB output parameters
 *******************************************************************************
 */
typedef struct {
    UInt32 channelId;
    /**< Channel Id for which this output params are valid */

    struct {
        UInt32 useAeCfg;
        /**< Flag to indicate whether to use exposureTime, analogGain
             and digital Gains or not */
        UInt32 exposureTime;
        /**< Exposure Time in micro seconds */
        UInt32 analogGain;
        /**< Analog Gains,
             Step size is same as Sensor gain step size */
        UInt32 digitalGain;
        /**< Digital Gains,
             Step size is same IPIPE gain step size */

        UInt32 useWbCfg;
        /**< Flag to indicate whether to use White balance gains or not */
        UInt32 gain[4];
        /**< [0] = r, [1] = Gr, [2] = Gb, [3] = B */
        UInt32 offset[4];
        /**< [0] = r, [1] = Gr, [2] = Gb, [3] = B */

        UInt32 useColorTemp;
        /* Flag to indicate whether to use color temp or not */
        UInt32 colorTemparature;
        /**< Color Temperature */
    } outPrms[2u];
    /**< For 2 PASS WDR. Currently applying for only 1 PASS */

    IssIspConfigurationParameters ispCfg;
    /**< ISP parameters from AEWB output */

    UInt32 numParams;
    /**< Number of valid entries in outPrms */

    UInt32 exposureRatio;
    /**< Exposure Ratio,
         Read from the sensor when Wdr mode is enabled
         ISP Merge parameters are calculated based on this ratio */
} IssAewbAlgOutParams;

typedef struct {
    UInt16 subSampleAcc[4];
    UInt16 saturatorAcc[4];
} IssAwebH3aOutSumModeOverlay;

typedef struct {
    UInt16 unsatCount[8];
} IssAwebH3aOutUnsatBlkCntOverlay;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
