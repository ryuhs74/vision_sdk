/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <examples/tda2xx/include/chains.h>
#include <examples/tda2xx/include/chains_common.h>
#include <examples/tda2xx/include/chains_common_iss.h>

/*******************************************************************************
 *  Structure declaration
 *******************************************************************************
 */

/**
 *  \brief Structure containins Sensor Information, used for ISS usecases
 */
typedef struct {
    Chains_CaptureSrc captSrc;
    /**< Source of the capture */
    UInt32 dccId;
    /**< DCC Id of the sensor */
    UInt32 qSpiOffset;
    /**< QSPI Offset for this sensor */
} ChainsCommon_IssSensorInfo;


/*******************************************************************************
 *  Globals
 *******************************************************************************
 */

ChainsCommon_IssSensorInfo gChainsCommonIssSensorInfo[] =
{
    {CHAINS_CAPTURE_SRC_OV10640_CSI2, 10640,
        CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_OV10640},
    {CHAINS_CAPTURE_SRC_OV10640_PARALLEL, 10640,
        CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_OV10640},
    {CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL, 140,
        CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_AR0140},
    {CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL, 132,
        CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_AR0132},
    {CHAINS_CAPTURE_SRC_IMX224_CSI2, 224,
        CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_IMX224},
    {CHAINS_CAPTURE_SRC_UB960_TIDA00262, 140,
        CHAINS_COMMON_ISS_DCC_QSPI_OFFSET_AR0140},
};

/**
 *******************************************************************************
 *
 * \brief   callback to handle user defined commands reaching system link
 *
 *  \param  cmd [IN] Command that needs to be handled
 *               pPrm [IN/OUT] Parameters for this command
 *
 *******************************************************************************
 */
Void ChainsCommon_Iss_CmdHandler(UInt32 cmd, Void *pPrm)
{
    Int32 status;
    UInt32 qSpiOffset, dccCameraId;
    AlgorithmLink_IssAewbDccControlParams *dccCtrlPrms;
    VidSensor_RegRdWrParams vidSensorReg;

    if (NULL != pPrm)
    {
        if (SYSTEM_LINK_CMD_SAVE_DCC_FILE == cmd)
        {
            dccCtrlPrms = (AlgorithmLink_IssAewbDccControlParams *)pPrm;

            /* Get the Sensor DCC Id from the header */
            dccCameraId = *(((UInt32 *)dccCtrlPrms->dccBuf) +
                CHAINS_COMMON_ISS_DCC_BIN_DCC_ID_OFFSET);

            /* Get the Sensor QSPI offset for the given sensors DCC ID */
            qSpiOffset = ChainsCommon_issGetQspiOffsetFromDccId(dccCameraId);

            /* Offset 0 cannot be used, so it is used as error value here */
            if (0 != qSpiOffset)
            {
                /* Write complete bin file */
                System_qspiWriteSector(qSpiOffset,
                                       (UInt32)dccCtrlPrms->dccBuf,
                                       ALGORITHM_AEWB1_DCC_IN_BUF_SIZE);
            }
        }
        if (SYSTEM_LINK_CMD_CLEAR_DCC_QSPI_MEM == cmd)
        {
            dccCameraId = *(UInt32 *)pPrm;

            /* Get the Sensor QSPI offset for the given sensors DCC ID */
            qSpiOffset = ChainsCommon_issGetQspiOffsetFromDccId(dccCameraId);

            /* Offset 0 cannot be used, so it is used as error value here */
            if (0 != qSpiOffset)
            {
                /* Write complete bin file */
                System_qspiEraseSector(qSpiOffset,
                                       ALGORITHM_AEWB1_DCC_IN_BUF_SIZE);
            }
        }
        if (SYSTEM_LINK_CMD_WRITE_SENSOR_REG == cmd)
        {
            UInt32 *prms = (UInt32 *)pPrm;
            vidSensorReg.chanNum = *prms;
            prms ++;
            vidSensorReg.regAddr = *prms;
            prms ++;
            vidSensorReg.regValue = *prms;

            status = VidSensor_control(
                ChainsCommon_GetSensorCreateParams(),
                VID_SENSOR_CMD_WRITE_REG,
                &vidSensorReg,
                NULL);

            UTILS_assert(0 == status);
        }
        if (SYSTEM_LINK_CMD_READ_SENSOR_REG == cmd)
        {
            UInt32 *prms = (UInt32 *)pPrm;
            vidSensorReg.chanNum = *prms;
            prms ++;
            vidSensorReg.regAddr = *prms;

            status = VidSensor_control(
                ChainsCommon_GetSensorCreateParams(),
                VID_SENSOR_CMD_READ_REG,
                &vidSensorReg,
                NULL);

            UTILS_assert(0 == status);

            prms ++;
            *prms = vidSensorReg.regValue;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   ChainsCommon_Iss_Init
 *
 *          This function registers command handler in system
 *          link for iss comands.
 *
 * \param   NULL
 *
  *******************************************************************************
 */
Void ChainsCommon_Iss_Init()
{
    SystemLink_registerHandler(ChainsCommon_Iss_CmdHandler);
}

Void ChainsCommon_Iss_DeInit()
{
    SystemLink_unregisterHandler(ChainsCommon_Iss_CmdHandler);
}

/**
 *******************************************************************************
 *
 * \brief   Set Capture Create Parameters for single camera capture mode
 *          with ISS
 *
 *******************************************************************************
*/
Void ChainsCommon_SingleCam_SetIssCapturePrms(
                        IssCaptureLink_CreateParams *pPrm,
                        Chains_CaptureSrc captureSrc,
                        IssM2mIspLink_OperatingMode ispOpMode)
{
    pPrm->videoIfMode = SYSTEM_VIFM_SCH_CPI;
    pPrm->videoIfWidth = SYSTEM_VIFW_12BIT;
    pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_BGGR;
    pPrm->outParams[0U].width = 1280;
    pPrm->outParams[0U].height = 720;

    if(captureSrc==CHAINS_CAPTURE_SRC_OV10640_CSI2)
    {
        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CSI2;
        pPrm->videoIfWidth = SYSTEM_VIFW_4LANES;
        pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_BGGR;
        pPrm->outParams[0U].width = 1280;
        pPrm->outParams[0U].height = 720;
        pPrm->outParams[0U].inCsi2DataFormat            = SYSTEM_CSI2_RAW12;
        pPrm->outParams[0U].maxWidth = 1280;
        pPrm->outParams[0U].maxHeight = 720;
        pPrm->outParams[0U].inCsi2VirtualChanNum = 0U;
        pPrm->csi2Params.cmplxIoCfg.clockLane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.clockLane.position  = 2U;
        pPrm->csi2Params.cmplxIoCfg.data1Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data1Lane.position  = 1U;
        pPrm->csi2Params.cmplxIoCfg.data2Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data2Lane.position  = 3U;
        pPrm->csi2Params.cmplxIoCfg.data3Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data3Lane.position  = 4U;
        pPrm->csi2Params.cmplxIoCfg.data4Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data4Lane.position  = 5U;
    }
    if(captureSrc==CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CSI2;
        pPrm->videoIfWidth = SYSTEM_VIFW_4LANES;
        pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_RGGB;
        if ((ISSM2MISP_LINK_OPMODE_2PASS_WDR == ispOpMode) ||
            (ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED == ispOpMode))
        {
            pPrm->outParams[0U].width = 1312;
            pPrm->outParams[0U].height = 2164;
        }
        else
        {
            pPrm->outParams[0U].width = 1280;
            pPrm->outParams[0U].height = 960;
        }
        pPrm->outParams[0U].inCsi2DataFormat            = SYSTEM_CSI2_RAW12;
        pPrm->outParams[0U].maxWidth = pPrm->outParams[0U].width;
        pPrm->outParams[0U].maxHeight = pPrm->outParams[0U].height;
        pPrm->outParams[0U].inCsi2VirtualChanNum = 0U;

        pPrm->csi2Params.cmplxIoCfg.clockLane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.clockLane.position  = 2U;
        pPrm->csi2Params.cmplxIoCfg.data1Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data1Lane.position  = 1U;
        pPrm->csi2Params.cmplxIoCfg.data2Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data2Lane.position  = 3U;
        pPrm->csi2Params.cmplxIoCfg.data3Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data3Lane.position  = 4U;
        pPrm->csi2Params.cmplxIoCfg.data4Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data4Lane.position  = 5U;
    }
    if(captureSrc==CHAINS_CAPTURE_SRC_OV10640_PARALLEL)
    {
        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CPI;
        pPrm->videoIfWidth = SYSTEM_VIFW_12BIT;
        pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_BGGR;
        pPrm->outParams[0U].width = 1280;
        pPrm->outParams[0U].height = 720;
    }
    if(captureSrc==CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL)
    {
        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CPI;
        pPrm->videoIfWidth = SYSTEM_VIFW_12BIT;
        pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_GRBG;
        pPrm->outParams[0U].width = 1280;
        pPrm->outParams[0U].height = 720;
    }
    if(captureSrc==CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL)
    {
        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CPI;
        pPrm->videoIfWidth = SYSTEM_VIFW_12BIT;
        pPrm->outParams[0U].dataFormat = SYSTEM_DF_RAW12;
        pPrm->outParams[0U].width = 1280;
        pPrm->outParams[0U].height = 720;
    }
    if((captureSrc==CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL) ||
       (captureSrc==CHAINS_CAPTURE_SRC_UB960_TIDA00262))
    {
        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CPI;
        pPrm->videoIfWidth = SYSTEM_VIFW_12BIT;
        pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_GRBG;
        pPrm->outParams[0U].width = 1280;
        pPrm->outParams[0U].height = 800;
    }

    pPrm->bufCaptMode = SYSTEM_CAPT_BCM_LAST_FRM_REPEAT;
    pPrm->outParams[0U].maxWidth = pPrm->outParams[0U].width;
    pPrm->outParams[0U].maxHeight = pPrm->outParams[0U].height;
    pPrm->outParams[0U].numOutBuf = 3;
}

Void ChainsCommon_SetIssAlgAewbPrms(
                        AlgorithmLink_IssAewbCreateParams *pPrm,
                        Chains_CaptureSrc captureSrc,
                        IssM2mIspLink_OperatingMode ispOpMode)
{
    UInt32 isOnePassWdr = 0;
    IssIspConfigurationParameters ispConfig;

    IssM2mIspLink_ConfigParams_Init(&ispConfig);
    /* get ISP config */
    ChainsCommon_SetIssIspConfig(&ispConfig, captureSrc);

    UTILS_assert(NULL != ispConfig.aewbCfg);

    /* udpate alg config based on ISP config */
    pPrm->h3aParams.winCountH = ispConfig.aewbCfg->winCfg.horzCount;
    pPrm->h3aParams.winCountV = ispConfig.aewbCfg->winCfg.vertCount;
    pPrm->h3aParams.winSizeH  = ispConfig.aewbCfg->winCfg.width;
    pPrm->h3aParams.winSizeV  = ispConfig.aewbCfg->winCfg.height;
    pPrm->h3aParams.winSkipH  = ispConfig.aewbCfg->winCfg.horzIncr;
    pPrm->h3aParams.winSkipV  = ispConfig.aewbCfg->winCfg.vertIncr;
    pPrm->numH3aPlanes = 1u;

    if ((captureSrc==CHAINS_CAPTURE_SRC_OV10640_PARALLEL) ||
        (captureSrc==CHAINS_CAPTURE_SRC_OV10640_CSI2))
    {
        pPrm->dataFormat = SYSTEM_DF_BAYER_BGGR;

        /* AE is supported by the Sensor, so only AWB is enabled */
        pPrm->mode = ALGORITHMS_ISS_AEWB_MODE_AWB;
    }
    if (((captureSrc==CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL) ||
         (captureSrc==CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL)) ||
        (captureSrc==CHAINS_CAPTURE_SRC_UB960_TIDA00262))
    {
        pPrm->dataFormat = SYSTEM_DF_BAYER_GRBG;
    }
    if (captureSrc==CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        pPrm->dataFormat = SYSTEM_DF_BAYER_RGGB;

        /* AE is not supported */
        pPrm->mode = ALGORITHMS_ISS_AEWB_MODE_AWB;
    }

    if ((ISSM2MISP_LINK_OPMODE_2PASS_WDR == ispOpMode) ||
        (ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED == ispOpMode))
    {
        pPrm->isWdrEnable = TRUE;
    }
    else
    {
        pPrm->isWdrEnable = FALSE;
    }

    if (ISSM2MISP_LINK_OPMODE_1PASS_WDR == ispOpMode)
    {
        isOnePassWdr = 1;
    }

    /* Set the AE and AWB sensor specific parameters */
    VidSensor_SetAewbParams(
        ChainsCommon_GetSensorCreateParams(),
        pPrm,
        isOnePassWdr);

    pPrm->numSteps = 6;
}


Void ChainsCommon_SetIssIspPrms(
                        IssM2mIspLink_CreateParams *pPrm,
                        Chains_CaptureSrc captureSrc,
                        UInt16 outWidthRszA,
                        UInt16 outHeightRszA,
                        UInt16 outWidthRszB,
                        UInt16 outHeightRszB,
                        IssM2mIspLink_OperatingMode ispOpMode,
                        IssM2mIspLink_WdrOffsetParams_t *wdrOffsetPrms)
{
    UInt32 chId;

    for(chId = 0; chId < ISSM2MISP_LINK_MAX_CH; chId++)
    {
        pPrm->channelParams[chId].operatingMode   = ispOpMode;
        pPrm->channelParams[chId].inBpp           = SYSTEM_BPP_BITS12;

        if (NULL != wdrOffsetPrms)
        {
            pPrm->channelParams[chId].wdrOffsetPrms = *wdrOffsetPrms;
        }

        pPrm->channelParams[chId].numBuffersPerCh = 2;

        pPrm->channelParams[chId].outParams.widthRszA  = outWidthRszA;
        pPrm->channelParams[chId].outParams.heightRszA = outHeightRszA;
        pPrm->channelParams[chId].outParams.widthRszB  = outWidthRszB;
        pPrm->channelParams[chId].outParams.heightRszB = outHeightRszB;
        pPrm->channelParams[chId].outParams.winWidthH3a = 16;
        pPrm->channelParams[chId].outParams.winHeightH3a = 16;
        pPrm->channelParams[chId].outParams.dataFormat = SYSTEM_DF_YUV420SP_UV;

        pPrm->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A]  = 1;
        pPrm->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B]  = 0;
        pPrm->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_H3A]  = 1;

        if(outWidthRszB && outHeightRszB)
        {
            pPrm->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B]  = 1;
        }
    }
}

Void ChainsCommon_SetIssSimcopPrms(
                        IssM2mSimcopLink_CreateParams *pPrm,
                        IssM2mSimcopLink_OperatingMode opMode)
{
    int chId;

    if(System_isFastBootEnabled())
    {
        pPrm->allocBufferForDump = 0;
    }

    for(chId=0; chId<ISSM2MSIMCOP_LINK_MAX_CH; chId++)
    {
        pPrm->channelParams[chId].operatingMode
            = opMode;

        pPrm->channelParams[chId].numBuffersPerCh
            = ISSM2MSIMCOP_LINK_NUM_BUFS_PER_CH_DEFAULT;
    }
}

Void ChainsCommon_SetIssSimcopLdcVtnfRtConfig(
                        vpsissldcConfig_t *ldcCfg,
                        vpsissvtnfConfig_t *vtnfCfg,
                        Bool bypssVtnf,
                        Bool bypssLdc)
{
    UInt32 i;

    /* LDC and VTNF Config pointers must not be null */
    UTILS_assert(NULL != ldcCfg);
    UTILS_assert(NULL != vtnfCfg);

    if(bypssLdc)
    {
        ldcCfg->enableBackMapping = FALSE;
    }
    else
    {
        ldcCfg->enableBackMapping = TRUE;
    }

    if (bypssVtnf)
    {
        vtnfCfg->outDataFormat          = SYSTEM_DF_YUV420SP_UV;
        vtnfCfg->isAdvCfgValid          = TRUE;
        vtnfCfg->advCfg.blockWidth      = 32; /* NEED NOT be set by USER */
        vtnfCfg->advCfg.blockHeight     = 36; /* NEED NOT be set by USER */
        vtnfCfg->advCfg.roundBitCount   = 3;
        vtnfCfg->advCfg.colorWeight1    = 0;
        vtnfCfg->advCfg.colorWeight2    = 0;

        i=0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 176;
        vtnfCfg->advCfg.lut1[i++]       = 160;
        vtnfCfg->advCfg.lut1[i++]       = 144;
        vtnfCfg->advCfg.lut1[i++]       = 128;
        vtnfCfg->advCfg.lut1[i++]       = 112;
        vtnfCfg->advCfg.lut1[i++]       = 96;
        vtnfCfg->advCfg.lut1[i++]       = 80;

        vtnfCfg->advCfg.lut1[i++]       = 72;
        vtnfCfg->advCfg.lut1[i++]       = 64;
        vtnfCfg->advCfg.lut1[i++]       = 56;
        vtnfCfg->advCfg.lut1[i++]       = 48;
        vtnfCfg->advCfg.lut1[i++]       = 32;
        vtnfCfg->advCfg.lut1[i++]       = 24;
        vtnfCfg->advCfg.lut1[i++]       = 16;
        vtnfCfg->advCfg.lut1[i++]       = 8;

        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;

        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;

        i=0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 176;
        vtnfCfg->advCfg.lut2[i++]       = 160;
        vtnfCfg->advCfg.lut2[i++]       = 144;
        vtnfCfg->advCfg.lut2[i++]       = 128;
        vtnfCfg->advCfg.lut2[i++]       = 112;
        vtnfCfg->advCfg.lut2[i++]       = 96;
        vtnfCfg->advCfg.lut2[i++]       = 80;

        vtnfCfg->advCfg.lut2[i++]       = 72;
        vtnfCfg->advCfg.lut2[i++]       = 64;
        vtnfCfg->advCfg.lut2[i++]       = 56;
        vtnfCfg->advCfg.lut2[i++]       = 48;
        vtnfCfg->advCfg.lut2[i++]       = 32;
        vtnfCfg->advCfg.lut2[i++]       = 24;
        vtnfCfg->advCfg.lut2[i++]       = 16;
        vtnfCfg->advCfg.lut2[i++]       = 8;

        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;

        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;

        /* NEED NOT be s->t by USER */
        vtnfCfg->advCfg.triggerSource   = ISSHAL_VTNF_TRG_SRC_HWSEQ;
        vtnfCfg->advCfg.intrEnable      = TRUE; /* NEED NOT be set by USER */
    }
    else
    {
        vtnfCfg->advCfg.roundBitCount   = 5;
        vtnfCfg->advCfg.colorWeight1    = 4;
        vtnfCfg->advCfg.colorWeight2    = 4;

        i=0;
        vtnfCfg->advCfg.lut1[i++]       = 192;
        vtnfCfg->advCfg.lut1[i++]       = 176;
        vtnfCfg->advCfg.lut1[i++]       = 160;
        vtnfCfg->advCfg.lut1[i++]       = 144;
        vtnfCfg->advCfg.lut1[i++]       = 128;
        vtnfCfg->advCfg.lut1[i++]       = 112;
        vtnfCfg->advCfg.lut1[i++]       = 96;
        vtnfCfg->advCfg.lut1[i++]       = 80;

        vtnfCfg->advCfg.lut1[i++]       = 72;
        vtnfCfg->advCfg.lut1[i++]       = 64;
        vtnfCfg->advCfg.lut1[i++]       = 56;
        vtnfCfg->advCfg.lut1[i++]       = 48;
        vtnfCfg->advCfg.lut1[i++]       = 32;
        vtnfCfg->advCfg.lut1[i++]       = 24;
        vtnfCfg->advCfg.lut1[i++]       = 16;
        vtnfCfg->advCfg.lut1[i++]       = 8;

        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;

        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;

        i=0;
        vtnfCfg->advCfg.lut2[i++]       = 192;
        vtnfCfg->advCfg.lut2[i++]       = 176;
        vtnfCfg->advCfg.lut2[i++]       = 160;
        vtnfCfg->advCfg.lut2[i++]       = 144;
        vtnfCfg->advCfg.lut2[i++]       = 128;
        vtnfCfg->advCfg.lut2[i++]       = 112;
        vtnfCfg->advCfg.lut2[i++]       = 96;
        vtnfCfg->advCfg.lut2[i++]       = 80;

        vtnfCfg->advCfg.lut2[i++]       = 72;
        vtnfCfg->advCfg.lut2[i++]       = 64;
        vtnfCfg->advCfg.lut2[i++]       = 56;
        vtnfCfg->advCfg.lut2[i++]       = 48;
        vtnfCfg->advCfg.lut2[i++]       = 32;
        vtnfCfg->advCfg.lut2[i++]       = 24;
        vtnfCfg->advCfg.lut2[i++]       = 16;
        vtnfCfg->advCfg.lut2[i++]       = 8;

        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;

        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
    }
}

Void ChainsCommon_SetIssSimcopConfig(
                        IssM2mSimcopLink_ConfigParams *pPrm,
                        Bool bypassVtnf,
                        Bool bypassLdc,
                        UInt32 ldcTableIdx)
{
    vpsissldcConfig_t  *ldcCfg;
    vpsissvtnfConfig_t *vtnfCfg;

    /* LDC and VTNF Config pointers must not be null */
    UTILS_assert(NULL != pPrm->ldcConfig);
    UTILS_assert(NULL != pPrm->vtnfConfig);
    ldcCfg = pPrm->ldcConfig;
    vtnfCfg = pPrm->vtnfConfig;

    ldcCfg->isAdvCfgValid             = TRUE;
    ldcCfg->pixelPad                  = 8;
    ldcCfg->advCfg.outputBlockWidth   = 32;
    ldcCfg->advCfg.outputBlockHeight  = 16;
    ldcCfg->advCfg.outputStartX       = 0;
    ldcCfg->advCfg.outputStartY       = 0;
    ldcCfg->advCfg.enableCircAddrMode = FALSE;
    ldcCfg->advCfg.circBuffSize       = 0;
    ldcCfg->advCfg.enableConstOutAddr = TRUE;

    /* set LDC LUT config */
    ChainsCommon_SetIssLdcLutConfig(&ldcCfg->lutCfg, ldcTableIdx);

    if(bypassLdc)
    {
        ldcCfg->enableBackMapping = FALSE;
    }
    else
    {
        ldcCfg->enableBackMapping = TRUE;
    }

    if(bypassVtnf)
    {
        /* No need to change VTNF params since by default it
         * is set for bypass mode
         */
    }
    else
    {
        int i;

        vtnfCfg->advCfg.roundBitCount   = 5;
        vtnfCfg->advCfg.colorWeight1    = 4;
        vtnfCfg->advCfg.colorWeight2    = 4;

        i=0;
        vtnfCfg->advCfg.lut1[i++]       = 192;
        vtnfCfg->advCfg.lut1[i++]       = 176;
        vtnfCfg->advCfg.lut1[i++]       = 160;
        vtnfCfg->advCfg.lut1[i++]       = 144;
        vtnfCfg->advCfg.lut1[i++]       = 128;
        vtnfCfg->advCfg.lut1[i++]       = 112;
        vtnfCfg->advCfg.lut1[i++]       = 96;
        vtnfCfg->advCfg.lut1[i++]       = 80;

        vtnfCfg->advCfg.lut1[i++]       = 72;
        vtnfCfg->advCfg.lut1[i++]       = 64;
        vtnfCfg->advCfg.lut1[i++]       = 56;
        vtnfCfg->advCfg.lut1[i++]       = 48;
        vtnfCfg->advCfg.lut1[i++]       = 32;
        vtnfCfg->advCfg.lut1[i++]       = 24;
        vtnfCfg->advCfg.lut1[i++]       = 16;
        vtnfCfg->advCfg.lut1[i++]       = 8;

        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;

        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;
        vtnfCfg->advCfg.lut1[i++]       = 0;

        i=0;
        vtnfCfg->advCfg.lut2[i++]       = 192;
        vtnfCfg->advCfg.lut2[i++]       = 176;
        vtnfCfg->advCfg.lut2[i++]       = 160;
        vtnfCfg->advCfg.lut2[i++]       = 144;
        vtnfCfg->advCfg.lut2[i++]       = 128;
        vtnfCfg->advCfg.lut2[i++]       = 112;
        vtnfCfg->advCfg.lut2[i++]       = 96;
        vtnfCfg->advCfg.lut2[i++]       = 80;

        vtnfCfg->advCfg.lut2[i++]       = 72;
        vtnfCfg->advCfg.lut2[i++]       = 64;
        vtnfCfg->advCfg.lut2[i++]       = 56;
        vtnfCfg->advCfg.lut2[i++]       = 48;
        vtnfCfg->advCfg.lut2[i++]       = 32;
        vtnfCfg->advCfg.lut2[i++]       = 24;
        vtnfCfg->advCfg.lut2[i++]       = 16;
        vtnfCfg->advCfg.lut2[i++]       = 8;

        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;

        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
        vtnfCfg->advCfg.lut2[i++]       = 0;
    }
}

Void ChainsCommon_SetIssCreatePrms(
        IssCaptureLink_CreateParams *pCapturePrm,
        IssM2mIspLink_CreateParams *pIspPrm,
        IssM2mSimcopLink_CreateParams *pSimcopPrm,
        AlgorithmLink_IssAewbCreateParams *pAlgAewbPrm,
        Chains_CaptureSrc captureSrc,
        UInt16 outWidthRszA,
        UInt16 outHeightRszA,
        UInt16 outWidthRszB,
        UInt16 outHeightRszB,
        IssM2mSimcopLink_OperatingMode opMode,
        IssM2mIspLink_OperatingMode ispOpMode,
        IssM2mIspLink_WdrOffsetParams_t *wdrOffsetPrms)
{
    if(pCapturePrm)
    {
        ChainsCommon_SingleCam_SetIssCapturePrms(
            pCapturePrm,
            captureSrc,
            ispOpMode);
    }
    if(pIspPrm)
    {
        ChainsCommon_SetIssIspPrms(
            pIspPrm,
            captureSrc,
            outWidthRszA,
            outHeightRszA,
            outWidthRszB,
            outHeightRszB,
            ispOpMode,
            wdrOffsetPrms);
    }
    if(pSimcopPrm)
    {
        ChainsCommon_SetIssSimcopPrms(
            pSimcopPrm,
            opMode);
    }
    if(pAlgAewbPrm)
    {
        ChainsCommon_SetIssAlgAewbPrms(
            pAlgAewbPrm,
            captureSrc,
            ispOpMode);
    }
}

Void ChainsCommon_UpdateAewbParams(IssAewbAlgOutParams *pAewbAlgOut)
{
    if (pAewbAlgOut->outPrms[0].useAeCfg)
    {
        VidSensor_UpdateAewbParams(
            ChainsCommon_GetSensorCreateParams(),
            pAewbAlgOut);
    }
}

Void ChainsCommon_GetIssIspConfig(
    Chains_CaptureSrc captureSrc,
    UInt32 linkId,
    IssM2mIspLink_OperatingMode ispOpMode,
    IssIspConfigurationParameters *pIspConfig,
    IssM2mSimcopLink_ConfigParams *pSimcopCfg)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 *header = NULL;
    UInt32 qSpiOffset = 0U, isDccProfileAvail = 0;
    VidSensor_CreateParams *createPrms = NULL;
    VidSensor_DccInfo dccInfo;
    AlgorithmLink_IssAewbDccControlParams dccCtrlPrms;

    createPrms = ChainsCommon_GetSensorCreateParams();

    UTILS_assert(NULL != createPrms);
    UTILS_assert(NULL != pIspConfig);

    /* Get the default config from ISP Layer */
    IssM2mIspLink_ConfigParams_Init(pIspConfig);

    pIspConfig->channelId = 0;

    /* Get the default configuration from the video sensor */
    VidSensor_SetIssIspConfig(
        createPrms,
        pIspConfig);

    /* For SinglePass WDR or monochrome mode, additional
       configuration is set by the video sensor layer.
       This configuration includes GLBCE WDR settings, which
       are as of now, not supported by DCC */
    if ((ISSM2MISP_LINK_OPMODE_1PASS_WDR == ispOpMode) ||
        (ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME == ispOpMode))
    {
        VidSensor_SetIssIspExtraConfig(
            createPrms,
            pIspConfig);
    }

    memset((void*) &dccInfo, 0, sizeof(dccInfo));
    VidSensor_control(
        ChainsCommon_GetSensorCreateParams(),
        VID_SENSOR_CMD_GET_DCC_INFO,
        &dccInfo,
        NULL);

    /* As of now, DCC is not supported for AR0132 monochrom sensor */
    if (CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL == captureSrc)
    {
        Vps_printf(" CHAINS: For AR0132 Monochrome Sensor,");
        Vps_printf(" using ISP settings from Video Sensor layer !!!\n");
        Vps_printf(" CHAINS: No Support for DCC for AR0132 monochrome sensor !!!\n");
        return ;
    }

    /* DCC is supported only if this flag is set in the driver,
       even dcc profile from qspi is supported only if this flag is set.
       Otherwise DCC parameters will not be used at all and configuration
       from the video sensor laye will be used */
    if (TRUE == dccInfo.isDccCfgSupported)
    {
        memset(&dccCtrlPrms, 0x0, sizeof(dccCtrlPrms));

        /* Get the DCC Buffer */
        dccCtrlPrms.baseClassControl.controlCmd =
            ALGORITHM_AEWB_LINK_CMD_GET_DCC_BUF_PARAMS;
        dccCtrlPrms.baseClassControl.size = sizeof(dccCtrlPrms);

        status = System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &dccCtrlPrms,
            sizeof(dccCtrlPrms),
            TRUE);
        UTILS_assert(0 == status);

        /* Get QSPI offst for this sensor */
        qSpiOffset = ChainsCommon_issGetQspiOffsetFromDccId(dccInfo.cameraId);
        UTILS_assert(0 != qSpiOffset);

        /* Check if the binary file in QSPI is valid, then use it instead
           of binary file form the driver */

        /* Read the Header first */
        System_qspiReadSector((UInt32)dccCtrlPrms.dccBuf,
                              qSpiOffset,
                              SystemUtils_align(
                                CHAINS_COMMON_ISS_DCC_BIN_HEADER_SIZE,
                                SYSTEM_QSPI_READ_WRITE_SIZE));

        header = (UInt32 *)dccCtrlPrms.dccBuf;
        if (CHAINS_COMMON_ISS_DCC_BIN_FILE_TAG_ID == *header)
        {
            /* Read bin file size */
            header ++;
            dccCtrlPrms.dccBufSize = *header;

            /* Read the binary file */
            System_qspiReadSector(
                (UInt32)dccCtrlPrms.dccBuf,
                qSpiOffset + CHAINS_COMMON_ISS_DCC_BIN_HEADER_SIZE,
                SystemUtils_align(dccCtrlPrms.dccBufSize,
                                  SYSTEM_QSPI_READ_WRITE_SIZE));
            isDccProfileAvail = 1U;
            Vps_printf(" CHAINS: Using DCC Profile from QSPI \n");
        }
        else
        {
            Vps_printf(" CHAINS: DCC Tag ID check failed for QSPI \n");
            Vps_printf(" CHAINS: Using DCC Profile from Driver \n");
            if ((TRUE == dccInfo.isDccCfgSupported) &&
                (NULL != dccInfo.pDccCfg) &&
                (0 != dccInfo.dccCfgSize))
            {
                memcpy(
                    dccCtrlPrms.dccBuf,
                    dccInfo.pDccCfg,
                    dccInfo.dccCfgSize);
                dccCtrlPrms.dccBufSize = dccInfo.dccCfgSize;
                isDccProfileAvail = 1U;
            }
        }

        if (1U == isDccProfileAvail)
        {
            dccCtrlPrms.baseClassControl.controlCmd =
                ALGORITHM_AEWB_LINK_CMD_PARSE_AND_SET_DCC_PARAMS;
            dccCtrlPrms.baseClassControl.size = sizeof(dccCtrlPrms);
            dccCtrlPrms.pIspCfg = pIspConfig;
            dccCtrlPrms.pSimcopCfg = pSimcopCfg;

            status = System_linkControl(
                linkId,
                ALGORITHM_LINK_CMD_CONFIG,
                &dccCtrlPrms,
                sizeof(dccCtrlPrms),
                TRUE);
            UTILS_assert(0 == status);
        }
    }
}

/* Used only for getting H3A Configuration to set to AEWB */
Void ChainsCommon_SetIssIspConfig(
    IssIspConfigurationParameters *ispConfig,
    Chains_CaptureSrc captureSrc)
{
    /* Get the default configuration from the video sensor */
    VidSensor_SetIssIspConfig(
        ChainsCommon_GetSensorCreateParams(),
        ispConfig);
}

/**
 *******************************************************************************
 *
 * \brief   Update Capture Create Parameters for multiple channels
 *
 *******************************************************************************
*/
Void ChainsCommon_MultipleCam_UpdateIssCapturePrms(
                        IssCaptureLink_CreateParams *pPrm,
                        Chains_CaptureSrc captureSrc,
                        IssM2mIspLink_OperatingMode ispOpMode,
                        UInt32 width, UInt32 height)
{

    if (captureSrc==CHAINS_CAPTURE_SRC_UB960_TIDA00262)
    {

        pPrm->numCh = 4U;

        pPrm->videoIfMode = SYSTEM_VIFM_SCH_CSI2;
        pPrm->videoIfWidth = SYSTEM_VIFW_4LANES;

        pPrm->csi2Params.cmplxIoCfg.clockLane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.clockLane.position  = 1U;
        pPrm->csi2Params.cmplxIoCfg.data1Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data1Lane.position  = 2U;
        pPrm->csi2Params.cmplxIoCfg.data2Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data2Lane.position  = 3U;
        pPrm->csi2Params.cmplxIoCfg.data3Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data3Lane.position  = 4U;
        pPrm->csi2Params.cmplxIoCfg.data4Lane.pol       = FALSE;
        pPrm->csi2Params.cmplxIoCfg.data4Lane.position  = 5U;
        pPrm->csi2Params.csi2PhyClk = 800U;

        pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_GRBG;
        pPrm->outParams[0U].width = width;
        pPrm->outParams[0U].height = height;
        pPrm->outParams[0U].inCsi2DataFormat = SYSTEM_CSI2_RAW12;
        pPrm->outParams[0U].maxWidth = width;
        pPrm->outParams[0U].maxHeight = height;
        pPrm->outParams[0U].inCsi2VirtualChanNum = 0U;
        pPrm->outParams[0U].numOutBuf = 3;

        pPrm->outParams[1U].dataFormat = SYSTEM_DF_BAYER_GRBG;
        pPrm->outParams[1U].width = width;
        pPrm->outParams[1U].height = height;
        pPrm->outParams[1U].inCsi2DataFormat = SYSTEM_CSI2_RAW12;
        pPrm->outParams[1U].maxWidth = width;
        pPrm->outParams[1U].maxHeight = height;
        pPrm->outParams[1U].inCsi2VirtualChanNum = 1U;
        pPrm->outParams[1U].numOutBuf = 3;

        pPrm->outParams[2U].dataFormat = SYSTEM_DF_BAYER_GRBG;
        pPrm->outParams[2U].width = width;
        pPrm->outParams[2U].height = height;
        pPrm->outParams[2U].inCsi2DataFormat = SYSTEM_CSI2_RAW12;
        pPrm->outParams[2U].maxWidth = width;
        pPrm->outParams[2U].maxHeight = height;
        pPrm->outParams[2U].inCsi2VirtualChanNum = 2U;
        pPrm->outParams[2U].numOutBuf = 3;

        pPrm->outParams[3U].dataFormat = SYSTEM_DF_BAYER_GRBG;
        pPrm->outParams[3U].width = width;
        pPrm->outParams[3U].height = height;
        pPrm->outParams[3U].inCsi2DataFormat = SYSTEM_CSI2_RAW12;
        pPrm->outParams[3U].maxWidth = width;
        pPrm->outParams[3U].maxHeight = height;
        pPrm->outParams[3U].inCsi2VirtualChanNum = 3U;
        pPrm->outParams[3U].numOutBuf = 3;
    }
    else
    {
        /* Un Recognized Capture source */
        UTILS_assert(FALSE);
    }
}
/*  \brief Function to get the QSPI offset for give sensor DCC ID.
 *
 *  params sensorDccIf      DCC Id of the sensor
 *
 *  returns qspiOffset      Qspi offset for this give sensor
 *          0               If sensor is not supported. Offset 0 cannot be
 *                          used for writing, so using this for
 *                          unsupported sensor
 */
UInt32 ChainsCommon_issGetQspiOffsetFromDccId(UInt32 sensorDccId)
{
    UInt32 cnt, maxSensors;
    UInt32 qSpiOffset = 0U;

    maxSensors = sizeof(gChainsCommonIssSensorInfo) /
                    sizeof(ChainsCommon_IssSensorInfo);

    for (cnt = 0U; cnt < maxSensors; cnt ++)
    {
        if (sensorDccId == gChainsCommonIssSensorInfo[cnt].dccId)
        {
            qSpiOffset = gChainsCommonIssSensorInfo[cnt].qSpiOffset;
            break;
        }
    }

    return (qSpiOffset);
}

