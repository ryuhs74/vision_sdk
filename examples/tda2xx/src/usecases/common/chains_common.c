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
#include <src/utils_common/include/utils_temperature.h>
#include <src/utils_common/include/utils_prcm_stats.h>
#include <examples/tda2xx/include/link_stats_monitor.h>

#define AVB_TALKER_MAX_FRAME_SIZE 300000

/**
 *******************************************************************************
 *
 *  \brief  Common information related to all use-case
 *
 *******************************************************************************
*/
typedef struct {

    UInt32  lcdInstId;
    UInt32  displayCtrlLinkId;

    VidSensor_CreateParams                  vidSensorPrm;
    HdmiRecvr_CreateParams                  hdmiRecvrPrm;
    Lcd_CreateParams                        lcdPrm;
    HdmiTx_CreateParams                     hdmiTxPrm;
	Component_CreateParams 					componentPrm;

    DisplayCtrlLink_ConfigParams            dctrlCfgPrms;
    DisplayCtrlLink_OvlyPipeParams  pipeParams[4];
    DisplayCtrlLink_OvlyParams      ovlyParams;

    UInt32                          issWdrMode;
} Chains_CommonObj;

Chains_CommonObj gChains_commonObj;


/**
 *******************************************************************************
 *
 * \brief   Do common system init
 *
 *******************************************************************************
*/
Void ChainsCommon_Init()
{

#if A15_TARGET_OS_BIOS
    Board_init();
    Lcd_init();
#endif

#ifdef ENABLE_UART
    System_uartInit();

    /* let the pending prints from other processor's finish */
    if(!System_isFastBootEnabled())
    {
        Task_sleep(1000);
    }
#endif
    Utils_prcmClockRateInit();

    if(!System_isFastBootEnabled())
    {
        SplitLink_init();
        UltrasonicCaptureLink_init();
        System_qspiInit();
        ChainsCommon_Stereo_Init();
    }

    GrpxSrcLink_init();
    ChainsCommon_SetSystemL3DmmPri();
    ChainsCommon_Iss_Init();
    Chains_linkStatsMonitorInit();
}

/**
 *******************************************************************************
 *
 * \brief   Do common system init
 *
 *******************************************************************************
*/
Void ChainsCommon_DeInit()
{
    GrpxSrcLink_deInit();

    if(!System_isFastBootEnabled())
    {
        SplitLink_deInit();
        UltrasonicCaptureLink_deInit();
    }

#if A15_TARGET_OS_BIOS
    Lcd_deInit();
    Board_deInit();
#endif

    ChainsCommon_Iss_DeInit();

    Chains_linkStatsMonitorDeInit();
}

/**
 *******************************************************************************
 *
 * \brief   Set Capture Create Parameters for single camera capture mode
 *
 * \param   pPrm         [IN]  CaptureLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SingleCam_SetCapturePrms(
                        CaptureLink_CreateParams *pPrm,
                        UInt32 captureInWidth,
                        UInt32 captureInHeight,
                        UInt32 captureOutWidth,
                        UInt32 captureOutHeight,
                        Chains_CaptureSrc captureSrc
                        )
{
    UInt32 i, streamId;
    CaptureLink_VipInstParams *pInstPrm;
    CaptureLink_InParams *pInprms;
    CaptureLink_OutParams *pOutprms;
    CaptureLink_VipScParams *pScPrms;
    CaptureLink_VipPortConfig    *pPortCfg;

    memset(pPrm, 0, sizeof(*pPrm));
    /* For SCV usecase number of camera is always 1 */
    pPrm->numVipInst = 1;
    pPrm->numDssWbInst = 0;

    for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
    {
        pInstPrm = &pPrm->vipInst[i];
        pInprms = &pInstPrm->inParams;
        pInstPrm->vipInstId     =   i;
#ifdef TDA2EX_BUILD
        //This change is done for supporting tda2ex
        pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
#endif
        if(captureSrc == CHAINS_CAPTURE_SRC_OV10635)
        {

            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_8BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   captureInWidth;
            pInprms->height     =   captureInHeight;
            pInprms->dataFormat =   SYSTEM_DF_YUV422P;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;

        }
         if(captureSrc == CHAINS_CAPTURE_SRC_DM388)
        {
            pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_AVID_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_24BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   captureInWidth;
            pInprms->height     =   captureInHeight;
            pInprms->dataFormat =   SYSTEM_DF_RGB24_888;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;

            /* Without clk inversion, saturation artifacts are seen on the screen when DM388
             * captures and tda2xx displays on HDMI. This is board specific fix and needed
             * only for MONSTERCAM.
             */
            Bsp_platformSetVipClkInversion(pInstPrm->vipInstId, TRUE);

        }
       else if (captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P)
        {


            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_AVID_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_16BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   1280;
            pInprms->height     =    720;
            pInprms->dataFormat =   SYSTEM_DF_YUV422P;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;

        }
        else if (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P)
        {


            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_AVID_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_16BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   1920;
            pInprms->height     =   1080;
            pInprms->dataFormat =   SYSTEM_DF_YUV422P;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;
        }
        else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
        {
            pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_16BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   captureInWidth;
            pInprms->height     =   captureInHeight;
            pInprms->dataFormat =   SYSTEM_DF_YUV422I_UYVY;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;
        }
        else if((captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL) ||
                (captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL))
        {
            pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_16BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   captureInWidth;
            pInprms->height     =   captureInHeight;
            pInprms->dataFormat =   SYSTEM_DF_BAYER_GRBG;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;
        }
        else
        {
            /* Nothing here. To avoid MISRA C warnings */
        }

        for (streamId = 0; streamId < CAPTURE_LINK_MAX_OUTPUT_PER_INST;
                streamId++)
        {
            pOutprms = &pInstPrm->outParams[streamId];
            pOutprms->width         =   captureOutWidth;
            pOutprms->height        =   captureOutHeight;
            pOutprms->dataFormat    =   SYSTEM_DF_YUV420SP_UV;
            pOutprms->maxWidth      =   pOutprms->width;
            pOutprms->maxHeight     =   pOutprms->height;
            if((pInprms->width != pOutprms->width) ||
                (pInprms->height != pOutprms->height))
            {
                pOutprms->scEnable      =   TRUE;
            }
            else
            {
                pOutprms->scEnable      =   FALSE;
            }
            pOutprms->subFrmPrms.subFrameEnable = FALSE;
            pOutprms->subFrmPrms.numLinesPerSubFrame = 0;

            if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
            {
                pOutprms->dataFormat    =   SYSTEM_DF_YUV422I_UYVY;
            }
            if((captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL) ||
               (captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL))
            {
                /* Keeping the bayer output format, so that next link (ISP)
                   can understand dataformat */
                pOutprms->dataFormat    =   SYSTEM_DF_BAYER_GRBG;
            }

            if(captureSrc==CHAINS_CAPTURE_SRC_HDMI_720P
                ||
               captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P
                ||
               captureSrc==CHAINS_CAPTURE_SRC_DM388
            )
            {
                /* skip alternate frame to make it 30fps output */
                pOutprms->frameSkipMask
                    = 0x2AAAAAAA;
            }
        }
        pScPrms = &pInstPrm->scPrms;
        pScPrms->inCropCfg.cropStartX = 0;
        pScPrms->inCropCfg.cropStartY = 0;
        pScPrms->inCropCfg.cropWidth = pInprms->width;
        pScPrms->inCropCfg.cropHeight = pInprms->height;

        pScPrms->scCfg.bypass       = FALSE;
        pScPrms->scCfg.nonLinear    = FALSE;
        pScPrms->scCfg.stripSize    = 0;

        pScPrms->userCoeff = FALSE;

        /* pScPrms->scCoeffCfg is not reuquired in case
         * pScPrms->userCoeff is FALSE
         */
        pPortCfg = &pInstPrm->vipPortCfg;
        pPortCfg->syncType          =   SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_YUV;
        pPortCfg->ancCropEnable     =   FALSE;



        pPortCfg->intfCfg.clipActive    =   FALSE;
        pPortCfg->intfCfg.clipBlank     =   FALSE;
        pPortCfg->intfCfg.intfWidth     =   SYSTEM_VIFW_16BIT;

        pPortCfg->disCfg.fidSkewPostCnt     =   0;
        pPortCfg->disCfg.fidSkewPreCnt      =   0;
        pPortCfg->disCfg.lineCaptureStyle   =
                                SYSTEM_VIP_LINE_CAPTURE_STYLE_ACTVID;
        pPortCfg->disCfg.fidDetectMode      =   SYSTEM_VIP_FID_DETECT_MODE_PIN;
        pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.discreteBasicMode  =   TRUE;

        pPortCfg->comCfg.ctrlChanSel        =   SYSTEM_VIP_CTRL_CHAN_SEL_7_0;
        pPortCfg->comCfg.ancChSel8b         =
                                SYSTEM_VIP_ANC_CH_SEL_8B_LUMA_SIDE;
        pPortCfg->comCfg.pixClkEdgePol      =   SYSTEM_EDGE_POL_RISING;
        pPortCfg->comCfg.invertFidPol       =   FALSE;
        pPortCfg->comCfg.enablePort         =   FALSE;
        pPortCfg->comCfg.expectedNumLines   =   pInprms->height;
        pPortCfg->comCfg.expectedNumPix     =   pInprms->width;
        pPortCfg->comCfg.repackerMode       =   SYSTEM_VIP_REPACK_CBA_TO_CBA;
        pPortCfg->actCropEnable             =   TRUE;

        if ((captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P) ||
            (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P))
        {
            pPortCfg->actCropEnable         =   FALSE;
            if(Bsp_platformIsTda3xxFamilyBuild())
            {
                pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_HIGH;
                pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_LOW;
                pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_LOW;
            }
        }

        pPortCfg->actCropCfg.srcNum                     =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->actCropCfg.cropCfg.cropWidth          =   pInprms->width;
        pPortCfg->actCropCfg.cropCfg.cropHeight         =   pInprms->height;

        pPortCfg->ancCropCfg.srcNum                     =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropWidth          =   0;
        pPortCfg->ancCropCfg.cropCfg.cropHeight         =   0;

        if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
        {
            pPortCfg->disCfg.lineCaptureStyle   =   SYSTEM_VIP_LINE_CAPTURE_STYLE_HSYNC;
            pPortCfg->disCfg.fidDetectMode      =   SYSTEM_VIP_FID_DETECT_MODE_VSYNC;
            pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_LOW;
            pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_LOW;
            pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_HIGH;
            pPortCfg->comCfg.ancChSel8b         =   SYSTEM_VIP_ANC_CH_SEL_DONT_CARE;
            pPortCfg->comCfg.pixClkEdgePol      =   SYSTEM_EDGE_POL_FALLING;
        }

        pInstPrm->numBufs = CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Capture Create Parameters for Stereo camera capture mode
 *
 * \param   pPrm         [IN]  CaptureLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_StereoCam_SetCapturePrms(
                        CaptureLink_CreateParams *pPrm,
                        UInt32 captureInWidth,
                        UInt32 captureInHeight,
                        UInt32 captureOutWidth,
                        UInt32 captureOutHeight,
                        Chains_CaptureSrc captureSrc
                        )
{
    UInt32 i, streamId;

    CaptureLink_VipInstParams *pInstPrm;
    CaptureLink_InParams *pInprms;
    CaptureLink_OutParams *pOutprms;
    CaptureLink_VipScParams *pScPrms;
    CaptureLink_VipPortConfig    *pPortCfg;

    memset(pPrm, 0, sizeof(*pPrm));
    /* For SCV usecase number of camera is always 1 */
    pPrm->numVipInst = 2;
    pPrm->numDssWbInst = 0;

    for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
    {
        pInstPrm = &pPrm->vipInst[i];
        pInprms = &pInstPrm->inParams;
        if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
        {
            if (i==0)
            {
                pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
            }
            else
            {
                pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
            }
            pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC;
            pInstPrm->videoIfWidth  =   SYSTEM_VIFW_16BIT;
            pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
            pInstPrm->numStream     =   1;

            pInprms->width      =   captureInWidth;
            pInprms->height     =   captureInHeight;
            pInprms->dataFormat =   SYSTEM_DF_YUV422I_UYVY;
            pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;
        }
        else
        {
            /* Nothing here. To avoid MISRA C warnings */
        }

        for (streamId = 0; streamId < CAPTURE_LINK_MAX_OUTPUT_PER_INST;
                streamId++)
        {
            pOutprms = &pInstPrm->outParams[streamId];
            pOutprms->width         =   captureOutWidth;
            pOutprms->height        =   captureOutHeight;
            pOutprms->dataFormat    =   SYSTEM_DF_YUV420SP_UV;
            pOutprms->maxWidth      =   pOutprms->width;
            pOutprms->maxHeight     =   pOutprms->height;
            if((pInprms->width != pOutprms->width) ||
                (pInprms->height != pOutprms->height))
            {
                pOutprms->scEnable      =   TRUE;
            }
            else
            {
                pOutprms->scEnable      =   FALSE;
            }
            pOutprms->subFrmPrms.subFrameEnable = FALSE;
            pOutprms->subFrmPrms.numLinesPerSubFrame = 0;

            if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
            {
                pOutprms->dataFormat    =   SYSTEM_DF_YUV422I_UYVY;
            }
        }
        pScPrms = &pInstPrm->scPrms;
        pScPrms->inCropCfg.cropStartX = 0;
        pScPrms->inCropCfg.cropStartY = 0;
        pScPrms->inCropCfg.cropWidth = pInprms->width;
        pScPrms->inCropCfg.cropHeight = pInprms->height;

        pScPrms->scCfg.bypass       = FALSE;
        pScPrms->scCfg.nonLinear    = FALSE;
        pScPrms->scCfg.stripSize    = 0;

        pScPrms->userCoeff = FALSE;

        /* pScPrms->scCoeffCfg is not reuquired in case
         * pScPrms->userCoeff is FALSE
         */
        pPortCfg = &pInstPrm->vipPortCfg;
        pPortCfg->syncType          =   SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_YUV;
        pPortCfg->ancCropEnable     =   FALSE;



        pPortCfg->intfCfg.clipActive    =   FALSE;
        pPortCfg->intfCfg.clipBlank     =   FALSE;
        pPortCfg->intfCfg.intfWidth     =   SYSTEM_VIFW_16BIT;

        pPortCfg->disCfg.fidSkewPostCnt     =   0;
        pPortCfg->disCfg.fidSkewPreCnt      =   0;
        pPortCfg->disCfg.lineCaptureStyle   =
                                SYSTEM_VIP_LINE_CAPTURE_STYLE_ACTVID;
        pPortCfg->disCfg.fidDetectMode      =   SYSTEM_VIP_FID_DETECT_MODE_PIN;
        pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.discreteBasicMode  =   TRUE;

        pPortCfg->comCfg.ctrlChanSel        =   SYSTEM_VIP_CTRL_CHAN_SEL_7_0;
        pPortCfg->comCfg.ancChSel8b         =
                                SYSTEM_VIP_ANC_CH_SEL_8B_LUMA_SIDE;
        pPortCfg->comCfg.pixClkEdgePol      =   SYSTEM_EDGE_POL_RISING;
        pPortCfg->comCfg.invertFidPol       =   FALSE;
        pPortCfg->comCfg.enablePort         =   FALSE;
        pPortCfg->comCfg.expectedNumLines   =   pInprms->height;
        pPortCfg->comCfg.expectedNumPix     =   pInprms->width;
        pPortCfg->comCfg.repackerMode       =   SYSTEM_VIP_REPACK_CBA_TO_CBA;
        pPortCfg->actCropEnable             =   TRUE;

        if ((captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P) ||
            (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P))
        {
            pPortCfg->actCropEnable         =   FALSE;
        }

        pPortCfg->actCropCfg.srcNum                     =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->actCropCfg.cropCfg.cropWidth          =   pInprms->width;
        pPortCfg->actCropCfg.cropCfg.cropHeight         =   pInprms->height;

        pPortCfg->ancCropCfg.srcNum                     =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropWidth          =   0;
        pPortCfg->ancCropCfg.cropCfg.cropHeight         =   0;

        if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
        {
            pPortCfg->disCfg.lineCaptureStyle   =   SYSTEM_VIP_LINE_CAPTURE_STYLE_HSYNC;
            pPortCfg->disCfg.fidDetectMode      =   SYSTEM_VIP_FID_DETECT_MODE_VSYNC;
            pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_LOW;
            pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_LOW;
            pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_HIGH;
            pPortCfg->comCfg.ancChSel8b         =   SYSTEM_VIP_ANC_CH_SEL_DONT_CARE;
            pPortCfg->comCfg.pixClkEdgePol      =   SYSTEM_EDGE_POL_FALLING;
        }

        pInstPrm->numBufs = CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT;
    }
}


/**
 *******************************************************************************
 *
 * \brief   Set Capture Create Parameters for multi camera capture mode
 *
 * \param   pPrm         [IN]  CaptureLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_MultiCam_SetCapturePrms(
                        CaptureLink_CreateParams *pPrm,
                        UInt32 captureInWidth,
                        UInt32 captureInHeight,
                        UInt32 portId[],
                        UInt32 numLvdsCh
                        )
{
    UInt32 i, streamId;

    CaptureLink_VipInstParams *pInstPrm;
    CaptureLink_InParams *pInprms;
    CaptureLink_OutParams *pOutprms;
    CaptureLink_VipScParams *pScPrm;
    CaptureLink_VipPortConfig    *pPortCfg;

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->numVipInst = numLvdsCh;
    pPrm->numDssWbInst = 0;

    for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
    {
        pInstPrm = &pPrm->vipInst[i];

        if(BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
        {
            if (i == 0)
            {
                pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
            }
            else if(i == 1)
            {
                pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP2_SLICE1_PORTA;
            }
            else
            {
                pInstPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
            }
        }
        else
        {
            UTILS_assert(portId != NULL);
            pInstPrm->vipInstId     =   portId[i];
        }

        pInstPrm->videoIfMode   =   SYSTEM_VIFM_SCH_DS_HSYNC_VSYNC;
        pInstPrm->videoIfWidth  =   SYSTEM_VIFW_8BIT;
        pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_FRM_DROP;
        pInstPrm->numStream     =   1;

        pInprms = &pInstPrm->inParams;

        pInprms->width      =   captureInWidth;
        pInprms->height     =   captureInHeight;
        pInprms->dataFormat =   SYSTEM_DF_YUV422P;
        pInprms->scanFormat =   SYSTEM_SF_PROGRESSIVE;

        for (streamId = 0; streamId < CAPTURE_LINK_MAX_OUTPUT_PER_INST;
                streamId++)
        {
            pOutprms = &pInstPrm->outParams[streamId];
            pOutprms->width         =   pInprms->width;
            pOutprms->height        =   pInprms->height;
            pOutprms->dataFormat    =   SYSTEM_DF_YUV420SP_UV;
            pOutprms->maxWidth      =   pOutprms->width;
            pOutprms->maxHeight     =   pOutprms->height;
            pOutprms->scEnable      =   FALSE;

            /* sub-frame not supported, set to FALSE */
            pOutprms->subFrmPrms.subFrameEnable = FALSE;
            pOutprms->subFrmPrms.numLinesPerSubFrame = 0;

        }
        pScPrm = &pInstPrm->scPrms;
        pScPrm->inCropCfg.cropStartX = 0;
        pScPrm->inCropCfg.cropStartY = 0;
        pScPrm->inCropCfg.cropWidth = pInprms->width;
        pScPrm->inCropCfg.cropHeight = pInprms->height;

        pScPrm->scCfg.bypass       = FALSE;
        pScPrm->scCfg.nonLinear    = FALSE;
        pScPrm->scCfg.stripSize    = 0;

        pScPrm->userCoeff = FALSE;

        /* pScPrm->scCoeffCfg is not reuquired in case
         * pScPrm->userCoeff is FALSE
         */
        pPortCfg = &pInstPrm->vipPortCfg;
        pPortCfg->syncType          =   SYSTEM_VIP_SYNC_TYPE_DIS_SINGLE_YUV;
        pPortCfg->ancCropEnable     =   FALSE;

        pPortCfg->intfCfg.clipActive    =   FALSE;
        pPortCfg->intfCfg.clipBlank     =   FALSE;
        pPortCfg->intfCfg.intfWidth     =   SYSTEM_VIFW_16BIT;

        pPortCfg->disCfg.fidSkewPostCnt     =   0;
        pPortCfg->disCfg.fidSkewPreCnt      =   0;
        pPortCfg->disCfg.lineCaptureStyle   =
                                SYSTEM_VIP_LINE_CAPTURE_STYLE_ACTVID;
        pPortCfg->disCfg.fidDetectMode      =   SYSTEM_VIP_FID_DETECT_MODE_PIN;
        pPortCfg->disCfg.actvidPol          =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.vsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.hsyncPol           =   SYSTEM_POL_HIGH;
        pPortCfg->disCfg.discreteBasicMode  =   TRUE;

        pPortCfg->comCfg.ctrlChanSel        =   SYSTEM_VIP_CTRL_CHAN_SEL_7_0;
        pPortCfg->comCfg.ancChSel8b         =
                            SYSTEM_VIP_ANC_CH_SEL_8B_LUMA_SIDE;
        pPortCfg->comCfg.pixClkEdgePol      =   SYSTEM_EDGE_POL_RISING;
        pPortCfg->comCfg.invertFidPol       =   FALSE;
        pPortCfg->comCfg.enablePort         =   FALSE;
        pPortCfg->comCfg.expectedNumLines   =   pInprms->height;
        pPortCfg->comCfg.expectedNumPix     =   pInprms->width;
        pPortCfg->comCfg.repackerMode       =   SYSTEM_VIP_REPACK_CBA_TO_CBA;

        pPortCfg->actCropEnable                         =   TRUE;
        pPortCfg->actCropCfg.srcNum                     =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->actCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->actCropCfg.cropCfg.cropWidth          =   pInprms->width;
        pPortCfg->actCropCfg.cropCfg.cropHeight         =   pInprms->height;

        pPortCfg->ancCropCfg.srcNum                     =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartX         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropStartY         =   0;
        pPortCfg->ancCropCfg.cropCfg.cropWidth          =   0;
        pPortCfg->ancCropCfg.cropCfg.cropHeight         =   0;

        pInstPrm->numBufs = CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT + 2;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Decode Create Parameters
 *
 *          It is called in Create function.
 *          All decoder parameters are set.
 *
 * \param   pPrm         [IN]    DecodeLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SetAvbRxDecodePrm(
                                     AvbRxLink_CreateParams *pAvbPrm,
                                     DecLink_CreateParams *pDecPrm,
                                     UInt32 maxWidth,
                                     UInt32 maxHeight,
                                     UInt32 numCh)
{
    UInt32 chId;
    DecLink_ChCreateParams *decPrm;
    UInt32 nIdx;
    UInt8 stream_ID[][8] =
    {
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x01},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x02},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x03},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x04}
    };
    UInt8 SrcMACAdd[][6] =
    {
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}
    };

    for (nIdx = 0; nIdx < numCh; nIdx++)
    {
        memcpy(pAvbPrm->streamId[nIdx], stream_ID[nIdx],sizeof(stream_ID[nIdx]));
        memcpy(pAvbPrm->srcMacId[nIdx], SrcMACAdd[nIdx],sizeof(SrcMACAdd[nIdx]));
    }
    pAvbPrm->numCameras = numCh;
    pAvbPrm->numBufs = 6;
    pAvbPrm->buffSize = AVB_TALKER_MAX_FRAME_SIZE;
    pAvbPrm->width  = maxWidth;
    pAvbPrm->height = maxHeight;

    for (chId = 0; chId<numCh; chId++)
    {
        decPrm = &pDecPrm->chCreateParams[chId];

        decPrm->dpbBufSizeInFrames  = DEC_LINK_DPB_SIZE_IN_FRAMES_DEFAULT;
        decPrm->algCreateStatus     = DEC_LINK_ALG_CREATE_STATUS_CREATE;
        decPrm->decodeFrameType     = DEC_LINK_DECODE_ALL;

        decPrm->format              = SYSTEM_IVIDEO_MJPEG;
        decPrm->processCallLevel    = DEC_LINK_FRAMELEVELPROCESSCALL;
        decPrm->targetMaxWidth      = maxWidth;
        decPrm->targetMaxHeight     = maxHeight;
        decPrm->numBufPerCh         = DEC_LINK_NUM_BUFFERS_PER_CHANNEL;
        decPrm->defaultDynamicParams.targetBitRate = 10 * 1000 * 1000;
        decPrm->defaultDynamicParams.targetFrameRate = 30;
        decPrm->fieldMergeDecodeEnable = FALSE;

        switch (decPrm->format)
        {
            case SYSTEM_IVIDEO_MJPEG: /* MJPEG */
                decPrm->profile = 0;
                decPrm->displayDelay = 0;
                break;
            default:
                Vps_printf(" CHAINS: ERROR: Un-supported codec type: %d !!! \n", decPrm->format);
                UTILS_assert(FALSE);
                break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set NullSrc and Decode Create Parameters
 *******************************************************************************
*/
Void ChainsCommon_SetNetworkRxPrms(
                        NullSrcLink_CreateParams *pPrm,
                        DecLink_CreateParams *pDecPrm,
                        UInt32 maxWidth,
                        UInt32 maxHeight,
                        UInt32 numCh,
                        UInt32 fps /* in msecs */
                        )
{
    UInt32 chId;
    DecLink_ChCreateParams *decPrm;
    System_LinkChInfo *pChInfo;

    pPrm->outQueInfo.numCh = numCh;

    pPrm->timerPeriodMilliSecs = 1000/fps;

    for (chId = 0; chId < pPrm->outQueInfo.numCh; chId++)
    {
        pPrm->channelParams[chId].numBuffers = 5;

        pChInfo = &pPrm->outQueInfo.chInfo[chId];

        if(pDecPrm)
        {
            SYSTEM_LINK_CH_INFO_SET_FLAG_BITSTREAM_FORMAT(pChInfo->flags,
                                                    SYSTEM_BITSTREAM_CODING_TYPE_MJPEG);
            SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(pChInfo->flags,
                                               SYSTEM_BUFFER_TYPE_BITSTREAM);
        }
        else
        {
            SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(pChInfo->flags,
                                                    SYSTEM_DF_YUV420SP_UV);

            SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(pChInfo->flags,
                                               SYSTEM_BUFFER_TYPE_VIDEO_FRAME);
        }

        SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(pChInfo->flags,
                                                    SYSTEM_SF_PROGRESSIVE);
        pChInfo->width = maxWidth;
        pChInfo->height = maxHeight;
        pChInfo->startX = 0;
        pChInfo->startY = 0;
        pChInfo->pitch[0] = SystemUtils_align(pChInfo->width, 32);
        pChInfo->pitch[1] = SystemUtils_align(pChInfo->width, 32);

    }

    pPrm->networkServerPort = NETWORK_RX_SERVER_PORT;
    pPrm->dataRxMode = NULLSRC_LINK_DATA_RX_MODE_NETWORK;

    if(pDecPrm)
    {
        for (chId = 0; chId<numCh; chId++)
        {
            decPrm = &pDecPrm->chCreateParams[chId];

            decPrm->dpbBufSizeInFrames  = DEC_LINK_DPB_SIZE_IN_FRAMES_DEFAULT;
            decPrm->algCreateStatus     = DEC_LINK_ALG_CREATE_STATUS_CREATE;
            decPrm->decodeFrameType     = DEC_LINK_DECODE_ALL;

            decPrm->format              = SYSTEM_IVIDEO_MJPEG;
            decPrm->processCallLevel    = DEC_LINK_FRAMELEVELPROCESSCALL;
            decPrm->targetMaxWidth      = SystemUtils_align(maxWidth, 32);
            decPrm->targetMaxHeight     = SystemUtils_align(maxHeight, 32);
            decPrm->numBufPerCh         = 4;
            decPrm->defaultDynamicParams.targetBitRate = 10 * 1000 * 1000;
            decPrm->defaultDynamicParams.targetFrameRate = 30;
            decPrm->fieldMergeDecodeEnable = FALSE;

            switch (decPrm->format)
            {
                case SYSTEM_IVIDEO_MJPEG: /* MJPEG */
                    decPrm->profile = 0;
                    decPrm->displayDelay = 0;
                    break;
                default:
                    Vps_printf(" CHAINS: ERROR: Un-supported codec type: %d !!! \n", decPrm->format);
                    UTILS_assert(FALSE);
                    break;
            }
        }
    }
}

/**
 *******************************************************************************
 * \brief   Set create parameters for NullSrc
 *******************************************************************************
*/
Void chains_networkRxDecDisplay_SetNullSrcPrms(
                                     UInt32 maxWidth,
                                     UInt32 maxHeight,
                                     UInt32 numCh)
{
}

/**
 *******************************************************************************
 *
 * \brief   Return W x H of a given display type
 *
 *******************************************************************************
*/
Void ChainsCommon_GetDisplayWidthHeight(
        Chains_DisplayType displayType,
        UInt32 *displayWidth,
        UInt32 *displayHeight
        )
{
    switch(displayType)
    {
        case CHAINS_DISPLAY_TYPE_CH7026_480P:
            *displayWidth = 720;
            *displayHeight = 480;
            break;
        case CHAINS_DISPLAY_TYPE_LCD_10_INCH:
            *displayWidth = 1280;
            *displayHeight = 800;
            break;
        case CHAINS_DISPLAY_TYPE_HDMI_720P:
            *displayWidth = 1280;
            *displayHeight = 720;
            break;
        case CHAINS_DISPLAY_TYPE_HDMI_1080P:
            *displayWidth = 1920;
            *displayHeight = 1080;
            break;
        case CHAINS_DISPLAY_TYPE_SDTV_NTSC:
            *displayWidth = 720;
            *displayHeight = 480;
            break;
        case CHAINS_DISPLAY_TYPE_SDTV_PAL:
            *displayWidth = 720;
            *displayHeight = 576;
            break;
        case CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM:
            *displayWidth = 1024;
            *displayHeight = 768;
            break;
        default:
            UTILS_assert(0);
            break;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Display Controller Create Parameters
 *
 * \param   pPrm         [IN]    DisplayCtrlLink_ConfigParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SetDctrlConfig(
                                DisplayCtrlLink_ConfigParams *pPrm,
                                DisplayCtrlLink_OvlyParams *ovlyPrms,
                                DisplayCtrlLink_OvlyPipeParams *pipeOvlyPrms,
                                UInt32 displayType,
                                UInt32 displayWidth,
                                UInt32 displayHeight
                                )
{
    DisplayCtrlLink_VencInfo *pVInfo;

    pPrm->numVencs = 1;
    pPrm->tiedVencs = 0;

    pVInfo = &pPrm->vencInfo[0];

    pVInfo->tdmMode = DISPLAYCTRL_LINK_TDM_DISABLE;
    if(displayType == CHAINS_DISPLAY_TYPE_CH7026_480P)
    {

        pPrm->deviceId = DISPLAYCTRL_LINK_USE_LCD;
        pVInfo->vencId = SYSTEM_DCTRL_DSS_VENC_LCD1;
        pVInfo->outputPort = SYSTEM_DCTRL_DSS_DPI1_OUTPUT;
        pVInfo->vencOutputInfo.vsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
        pVInfo->vencOutputInfo.hsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_LOW;

        /* Below are of dont care for EVM LCD */
        pVInfo->vencOutputInfo.fidPolarity      =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
        pVInfo->vencOutputInfo.actVidPolarity   =   SYSTEM_DCTRL_POLARITY_ACT_LOW;

        pVInfo->vencOutputInfo.dataFormat       =   SYSTEM_DF_RGB24_888;
        pVInfo->vencOutputInfo.dvoFormat        =
                                    SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
        pVInfo->vencOutputInfo.videoIfWidth     =   SYSTEM_VIFW_24BIT;

        pVInfo->vencOutputInfo.pixelClkPolarity =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
        pVInfo->vencOutputInfo.aFmt             =   SYSTEM_DCTRL_A_OUTPUT_MAX;
        pVInfo->mInfo.standard                  =   SYSTEM_STD_CUSTOM;


        //{FVID2_STD_480P,           720,  480,  FVID2_SF_PROGRESSIVE, 27000,  60, 16, 60, 62, 9, 30, 6},
        pVInfo->mInfo.width                     =   displayWidth;
        pVInfo->mInfo.height                    =   displayHeight;
        pVInfo->mInfo.scanFormat                =   SYSTEM_SF_PROGRESSIVE;
        pVInfo->mInfo.pixelClock                =   27000;

       // UInt32              hFrontPorch;		16
       // UInt32              hBackPorch;		60
       // UInt32              hSyncLen;			62
       // UInt32              vFrontPorch;		9
       // UInt32              vBackPorch;		30
       // UInt32              vSyncLen;			6

        pVInfo->mInfo.fps                       =   60U;
        pVInfo->mInfo.hSyncLen                  =   62U;
        pVInfo->mInfo.hFrontPorch               =   16U;
        pVInfo->mInfo.hBackPorch                =   60U;

        pVInfo->mInfo.vFrontPorch               =   9U;
        pVInfo->mInfo.vSyncLen                  =   6U;
        pVInfo->mInfo.vBackPorch                =   30U;

        pVInfo->vencDivisorInfo.divisorLCD      =   1;
        pVInfo->vencDivisorInfo.divisorPCD      =   1;

        /* Configure overlay params */

        ovlyPrms->vencId                        = SYSTEM_DCTRL_DSS_VENC_LCD1;


    }
    if(displayType == CHAINS_DISPLAY_TYPE_LCD_10_INCH)
    {
        pPrm->deviceId = DISPLAYCTRL_LINK_USE_LCD;
        pVInfo->vencId = SYSTEM_DCTRL_DSS_VENC_LCD1;
        pVInfo->outputPort = SYSTEM_DCTRL_DSS_DPI1_OUTPUT;
        pVInfo->vencOutputInfo.vsPolarity    =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
        pVInfo->vencOutputInfo.hsPolarity    =   SYSTEM_DCTRL_POLARITY_ACT_LOW;

        /* Below are of dont care for EVM LCD */
        pVInfo->vencOutputInfo.fidPolarity      =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
        pVInfo->vencOutputInfo.actVidPolarity   =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

        pVInfo->mInfo.standard                  =   SYSTEM_STD_CUSTOM;
        pVInfo->mInfo.width                     =   displayWidth;
        pVInfo->mInfo.height                    =   displayHeight;
        pVInfo->mInfo.scanFormat                =   SYSTEM_SF_PROGRESSIVE;
        pVInfo->mInfo.pixelClock                =   74500U;
        pVInfo->mInfo.fps                       =   60U;
        pVInfo->mInfo.hBackPorch                =   80U;
        pVInfo->mInfo.hSyncLen                  =   62U;
        pVInfo->mInfo.hFrontPorch               =   48U;
        pVInfo->mInfo.vBackPorch                =   12U;
        pVInfo->mInfo.vSyncLen                  =   35U;
        pVInfo->mInfo.vFrontPorch               =   6U;
        pVInfo->vencDivisorInfo.divisorLCD      =   1;
        pVInfo->vencDivisorInfo.divisorPCD      =   1;

        pVInfo->vencOutputInfo.dataFormat       =   SYSTEM_DF_RGB24_888;
        pVInfo->vencOutputInfo.dvoFormat        =
                                    SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
        pVInfo->vencOutputInfo.videoIfWidth     =   SYSTEM_VIFW_24BIT;

        pVInfo->vencOutputInfo.pixelClkPolarity =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
        pVInfo->vencOutputInfo.aFmt             =   SYSTEM_DCTRL_A_OUTPUT_MAX;

        /* Configure overlay params */

        ovlyPrms->vencId                       = SYSTEM_DCTRL_DSS_VENC_LCD1;
    }
    else if(displayType == CHAINS_DISPLAY_TYPE_HDMI_720P
            || displayType == CHAINS_DISPLAY_TYPE_HDMI_1080P)
    {
        if(Bsp_platformIsTda2xxFamilyBuild())
        {
            pPrm->deviceId = DISPLAYCTRL_LINK_USE_HDMI;
            pVInfo->vencId = SYSTEM_DCTRL_DSS_VENC_HDMI;
            pVInfo->outputPort = SYSTEM_DCTRL_DSS_HDMI_OUTPUT;
            pVInfo->vencOutputInfo.vsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.hsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            /* Below are of dont care for EVM LCD */
            pVInfo->vencOutputInfo.fidPolarity      =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
            pVInfo->vencOutputInfo.actVidPolarity   =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            pVInfo->vencOutputInfo.dataFormat       =   SYSTEM_DF_RGB24_888;
            pVInfo->vencOutputInfo.dvoFormat        =
                                        SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
            pVInfo->vencOutputInfo.videoIfWidth     =   SYSTEM_VIFW_24BIT;

            pVInfo->vencOutputInfo.pixelClkPolarity =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.aFmt             =   SYSTEM_DCTRL_A_OUTPUT_MAX;

            /* Configure overlay params */

            ovlyPrms->vencId                        = SYSTEM_DCTRL_DSS_VENC_HDMI;
        }
        if(Bsp_platformIsTda3xxFamilyBuild())
        {
            pPrm->deviceId = DISPLAYCTRL_LINK_USE_LCD;
            pVInfo->vencId = SYSTEM_DCTRL_DSS_VENC_LCD1;
            pVInfo->outputPort = SYSTEM_DCTRL_DSS_DPI1_OUTPUT;
            pVInfo->vencOutputInfo.vsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.hsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            /* Below are of dont care for EVM LCD */
            pVInfo->vencOutputInfo.fidPolarity      =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
            pVInfo->vencOutputInfo.actVidPolarity   =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            pVInfo->vencOutputInfo.dataFormat       =   SYSTEM_DF_RGB24_888;
            pVInfo->vencOutputInfo.dvoFormat        =
                                        SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
            pVInfo->vencOutputInfo.videoIfWidth     =   SYSTEM_VIFW_24BIT;

            pVInfo->vencOutputInfo.pixelClkPolarity =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.aFmt             =   SYSTEM_DCTRL_A_OUTPUT_MAX;

            pVInfo->vencDivisorInfo.divisorLCD      =   1;
            pVInfo->vencDivisorInfo.divisorPCD      =   1;

            /* Configure overlay params */

            ovlyPrms->vencId                        = SYSTEM_DCTRL_DSS_VENC_LCD1;
        }
    }
    else if((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
            (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
    {
        if(Bsp_platformIsTda3xxFamilyBuild())
        {
            pPrm->deviceId = DISPLAYCTRL_LINK_USE_LCD;
            pVInfo->vencId = SYSTEM_DCTRL_DSS_VENC_SDTV;
            pVInfo->outputPort = SYSTEM_DCTRL_DSS_DPI1_OUTPUT;

            pVInfo->vencOutputInfo.vsPolarity    =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.hsPolarity    =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            /* Below are of dont care for EVM LCD */
            pVInfo->vencOutputInfo.fidPolarity      =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.actVidPolarity   =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            pVInfo->vencDivisorInfo.divisorLCD      =   1;
            pVInfo->vencDivisorInfo.divisorPCD      =   1;

            pVInfo->vencOutputInfo.dataFormat       =   SYSTEM_DF_RGB24_888;
            pVInfo->vencOutputInfo.dvoFormat        =
                                    SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
            pVInfo->vencOutputInfo.videoIfWidth     =   SYSTEM_VIFW_24BIT;

            pVInfo->vencOutputInfo.pixelClkPolarity =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.aFmt             =   SYSTEM_DCTRL_A_OUTPUT_MAX;

            ovlyPrms->vencId = SYSTEM_DCTRL_DSS_VENC_LCD1;
        }
        else
        {
            UTILS_assert(0);
        }
    }
    else if(displayType == CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM)
    {
        if(Bsp_platformIsTda3xxFamilyBuild())
        {
            pPrm->deviceId = DISPLAYCTRL_LINK_USE_LCD;
            pVInfo->vencId = SYSTEM_DCTRL_DSS_VENC_LCD1;
            pVInfo->outputPort = SYSTEM_DCTRL_DSS_DPI1_OUTPUT;
            pVInfo->vencOutputInfo.vsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.hsPolarity       =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            /* Below are of dont care for EVM LCD */
            pVInfo->vencOutputInfo.fidPolarity      =   SYSTEM_DCTRL_POLARITY_ACT_LOW;
            pVInfo->vencOutputInfo.actVidPolarity   =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;

            pVInfo->vencOutputInfo.dataFormat       =   SYSTEM_DF_RGB24_888;
            pVInfo->vencOutputInfo.dvoFormat        =
                                        SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
            pVInfo->vencOutputInfo.videoIfWidth     =   SYSTEM_VIFW_24BIT;

            pVInfo->vencOutputInfo.pixelClkPolarity =   SYSTEM_DCTRL_POLARITY_ACT_HIGH;
            pVInfo->vencOutputInfo.aFmt             =   SYSTEM_DCTRL_A_OUTPUT_MAX;

            pVInfo->vencDivisorInfo.divisorLCD      =   1;
            pVInfo->vencDivisorInfo.divisorPCD      =   1;

            /* Configure overlay params */

            ovlyPrms->vencId                        = SYSTEM_DCTRL_DSS_VENC_LCD1;

            pVInfo->tdmMode = DISPLAYCTRL_LINK_TDM_24BIT_TO_8BIT;
        }
        else
        {
            UTILS_assert(0);
        }
    }

    if(displayType == CHAINS_DISPLAY_TYPE_HDMI_720P)
    {
        pVInfo->mInfo.standard                  =   SYSTEM_STD_720P_60;

    }
    else if (displayType == CHAINS_DISPLAY_TYPE_HDMI_1080P)
    {
        pVInfo->mInfo.standard                  =   SYSTEM_STD_1080P_60;
    }
    else if(displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC)
    {
        pVInfo->mInfo.standard                  =   SYSTEM_STD_NTSC;
        pVInfo->mInfo.scanFormat                =   SYSTEM_SF_INTERLACED;
    }
    else if(displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL)
    {
        pVInfo->mInfo.standard                  =   SYSTEM_STD_PAL;
        pVInfo->mInfo.scanFormat                =   SYSTEM_SF_INTERLACED;
    }
    else if(displayType == CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM)
    {
        pVInfo->mInfo.standard                  =   SYSTEM_STD_XGA_DSS_TDM_60;
    }

    /* TODO Dont know what to set here */
    pVInfo->mode = 0;
    pVInfo->isInputPipeConnected[0] =
                    TRUE;
    pVInfo->isInputPipeConnected[1] =
                    TRUE;
    if(Bsp_platformIsTda2xxFamilyBuild())
    {
        pVInfo->isInputPipeConnected[2] =
                    TRUE;
    }
    else
    {
        pVInfo->isInputPipeConnected[2] =
                    FALSE;
    }
    pVInfo->isInputPipeConnected[3] =
                    TRUE;
    pVInfo->writeBackEnabledFlag = FALSE;


    /* Setting other overlay parameters common to both Venc */

    ovlyPrms->deltaLinesPerPanel   = 0;
    ovlyPrms->alphaBlenderEnable   = 0;
    ovlyPrms->backGroundColor      = 0x10;
    ovlyPrms->colorKeyEnable       = 1;
    ovlyPrms->colorKeySel          = SYSTEM_DSS_DISPC_TRANS_COLOR_KEY_SRC;
    ovlyPrms->ovlyOptimization     = SYSTEM_DSS_DISPC_OVLY_FETCH_ALLDATA;
    ovlyPrms->transColorKey        = DRAW2D_TRANSPARENT_COLOR;

    /* Setting overlay pipe parameters */

    pipeOvlyPrms[0].pipeLine = SYSTEM_DSS_DISPC_PIPE_VID1;
    pipeOvlyPrms[0].globalAlpha = 0xFF;
    pipeOvlyPrms[0].preMultiplyAlpha = 0;
    pipeOvlyPrms[0].zorderEnable = TRUE;
    pipeOvlyPrms[0].zorder = SYSTEM_DSS_DISPC_ZORDER0;

    pipeOvlyPrms[1].pipeLine = SYSTEM_DSS_DISPC_PIPE_VID2;
    pipeOvlyPrms[1].globalAlpha = 0xFF;
    pipeOvlyPrms[1].preMultiplyAlpha = 0;
    pipeOvlyPrms[1].zorderEnable = TRUE;
    pipeOvlyPrms[1].zorder = SYSTEM_DSS_DISPC_ZORDER1;

    if(Bsp_platformIsTda2xxFamilyBuild())
    {
        pipeOvlyPrms[2].pipeLine = SYSTEM_DSS_DISPC_PIPE_VID3;
        pipeOvlyPrms[2].globalAlpha = 0xFF;
        pipeOvlyPrms[2].preMultiplyAlpha = 0;
        pipeOvlyPrms[2].zorderEnable = TRUE;
        pipeOvlyPrms[2].zorder = SYSTEM_DSS_DISPC_ZORDER2;
    }

    pipeOvlyPrms[3].pipeLine = SYSTEM_DSS_DISPC_PIPE_GFX1;
    pipeOvlyPrms[3].globalAlpha = 0xFF;
    pipeOvlyPrms[3].preMultiplyAlpha = 0;
    pipeOvlyPrms[3].zorderEnable = TRUE;
    pipeOvlyPrms[3].zorder = SYSTEM_DSS_DISPC_ZORDER3;
    if(Bsp_platformIsTda3xxFamilyBuild())
    {
        pipeOvlyPrms[3].zorder = SYSTEM_DSS_DISPC_ZORDER2;
    }

}

/**
 *******************************************************************************
 *
 * \brief   Set Sensor Create Parameters
 *
 *          This function is used to set the Video Sensor params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipSingleCameraView_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *          Video sensor Id , Instance Id data format are set.
 *
 * \param   pPrm      [IN]    VidSensor_CreateParams
 * \param   sensorId  [IN]    Sensor ID
 * \param   portId    [OUT]   Ports used by this sensor,
 *                            only valid when sensorId is
 *                            VID_SENSOR_MULDES_OV1063X
 * \param   numCh     [IN]    Num of channels used
 *                            only valid when sensorId is
 *                            VID_SENSOR_MULDES_OV1063X or captureSrc is
 *                            CHAINS_CAPTURE_SRC_UB960_TIDA00262
 *
 *******************************************************************************
*/
static Void ChainsCommon_SetVidSensorPrms(
                                    VidSensor_CreateParams *pPrm,
                                    VidSensor_Id sensorId,
                                    UInt32 portId[],
                                    UInt32 numCh,
                                    Chains_CaptureSrc captureSrc
                                    )
{
    if(sensorId==VID_SENSOR_OV10635)
    {
        pPrm->sensorId      = VID_SENSOR_OV10635;
        pPrm->vipInstId[0]     = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
#ifdef TDA2EX_BUILD
       //This change is done to support tda2ex VIP1_SLICE2_PORTA
        pPrm->vipInstId[0]     = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
#endif
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_YUV422I_UYVY;
        pPrm->videoIfWidth  = SYSTEM_VIFW_8BIT;
        pPrm->fps           = SYSTEM_FPS_30;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 1;
    }
    if(sensorId==VID_SENSOR_IMX224_CSI2)
    {
        pPrm->sensorId      = VID_SENSOR_IMX224_CSI2;
        if (NULL != portId)
        {
            pPrm->vipInstId[0]  = portId[0U];
        }
        else
        {
            pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
        }
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_BAYER_BGGR;
        pPrm->videoIfWidth  = SYSTEM_VIFW_4LANES;
        pPrm->fps           = SYSTEM_FPS_30;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 1;
        pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CSI2;

    }
    if(sensorId==VID_SENSOR_OV10640)
    {
        pPrm->sensorId      = VID_SENSOR_OV10640;
        if (NULL != portId)
        {
            pPrm->vipInstId[0]  = portId[0U];
        }
        else
        {
            pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
        }
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_BAYER_BGGR;
        pPrm->videoIfWidth  = SYSTEM_VIFW_4LANES;
        pPrm->fps           = SYSTEM_FPS_30;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 1;
        pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CSI2;


        if(captureSrc==CHAINS_CAPTURE_SRC_OV10640_PARALLEL)
        {
            if (NULL != portId)
            {
                pPrm->vipInstId[0]  = portId[0U];
            }
            else
            {
                pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
            }
            pPrm->videoIfWidth  = SYSTEM_VIFW_12BIT;
            pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CPI;
        }
    }
    if(sensorId==VID_SENSOR_AR0132_BAYER)
    {
        pPrm->sensorId      = VID_SENSOR_AR0132_BAYER;
        if (NULL != portId)
        {
            pPrm->vipInstId[0]  = portId[0U];
        }
        else
        {
            pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
        }
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_BAYER_GRBG;
        pPrm->videoIfWidth  = SYSTEM_VIFW_12BIT;
        pPrm->fps           = SYSTEM_FPS_60;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 1;
        pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CPI;
    }
    if(sensorId==VID_SENSOR_AR0132_MONOCHROME)
    {
        pPrm->sensorId      = VID_SENSOR_AR0132_MONOCHROME;
        if (NULL != portId)
        {
            pPrm->vipInstId[0]  = portId[0U];
        }
        else
        {
            pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
        }
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_RAW12;
        pPrm->videoIfWidth  = SYSTEM_VIFW_12BIT;
        pPrm->fps           = SYSTEM_FPS_60;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 1;
        pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CPI;
    }
    if(sensorId==VID_SENSOR_AR0140_BAYER)
    {
        pPrm->sensorId      = VID_SENSOR_AR0140_BAYER;
        if (NULL != portId)
        {
            pPrm->vipInstId[0]  = portId[0U];
        }
        else
        {
            pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
        }
        pPrm->standard      = SYSTEM_STD_WXGA_30;
        pPrm->dataformat    = SYSTEM_DF_BAYER_GRBG;
        pPrm->videoIfWidth  = SYSTEM_VIFW_12BIT;
        pPrm->fps           = SYSTEM_FPS_30;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 1;
        pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CPI;

        if (CHAINS_CAPTURE_SRC_UB960_TIDA00262==captureSrc)
        {
            pPrm->vipInstId[0]  = VPS_CAPT_INST_ISS_CAL_A;
            pPrm->vipInstId[1]  = VPS_CAPT_INST_ISS_CAL_A;
            pPrm->vipInstId[2]  = VPS_CAPT_INST_ISS_CAL_A;
            pPrm->vipInstId[3]  = VPS_CAPT_INST_ISS_CAL_A;

            pPrm->standard      = SYSTEM_STD_720P_60;
            pPrm->dataformat    = SYSTEM_DF_BAYER_GRBG;
            pPrm->videoIfWidth  = SYSTEM_VIFW_4LANES;
            pPrm->fps           = SYSTEM_FPS_30;
            pPrm->isLVDSCaptMode = FALSE;
            pPrm->numChan       = numCh;
            pPrm->videoIfMode   = SYSTEM_VIFM_SCH_CSI2;
        }
    }
    if(sensorId==VID_SENSOR_MULDES_OV1063X)
    {
        pPrm->sensorId      = VID_SENSOR_MULDES_OV1063X;
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_YUV422I_UYVY;
        pPrm->videoIfWidth  = SYSTEM_VIFW_8BIT;
        pPrm->fps           = SYSTEM_FPS_30;
        pPrm->isLVDSCaptMode = TRUE;
        pPrm->numChan       = numCh;

        if(BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
        {
            pPrm->vipInstId[0] = SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
            pPrm->vipInstId[1] = SYSTEM_CAPTURE_INST_VIP2_SLICE1_PORTA;
            pPrm->vipInstId[2] = SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
        }
        else
        {
            pPrm->vipInstId[0] = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
            pPrm->vipInstId[1] = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
            pPrm->vipInstId[2] = SYSTEM_CAPTURE_INST_VIP2_SLICE1_PORTA;
            pPrm->vipInstId[3] = SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;
            pPrm->vipInstId[4] = SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
            pPrm->vipInstId[5] = SYSTEM_CAPTURE_INST_VIP2_SLICE2_PORTB;

#ifdef TDA2EX_BUILD
            pPrm->vipInstId[0] = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
            pPrm->vipInstId[1] = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
            pPrm->vipInstId[2] = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTB;
            pPrm->vipInstId[3] = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTB;
#endif

            if(Bsp_platformIsTda3xxFamilyBuild())
            {
                pPrm->vipInstId[0] = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
                pPrm->vipInstId[1] = SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTB;
                pPrm->vipInstId[2] = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
                pPrm->vipInstId[3] = SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTB;
            }
            /* copy VIP inst ID to portId[] so that this can be during setting up
             * of VIP capture parameters
             */
            UTILS_assert(portId != NULL);
            memcpy(portId,
                    pPrm->vipInstId,
                    sizeof(pPrm->vipInstId)
                );
         }

    }
    if(sensorId==VID_SENSOR_MULDES_AR0132RCCC)
    {
        pPrm->sensorId      = VID_SENSOR_MULDES_AR0132RCCC;
        pPrm->standard      = SYSTEM_STD_720P_60;
        pPrm->dataformat    = SYSTEM_DF_YUV422I_UYVY;
        pPrm->videoIfWidth  = SYSTEM_VIFW_16BIT;
        pPrm->fps           = SYSTEM_FPS_60;
        pPrm->isLVDSCaptMode = FALSE;
        pPrm->numChan       = 2;

        pPrm->vipInstId[0] = SYSTEM_CAPTURE_INST_VIP3_SLICE1_PORTA;

        pPrm->vipInstId[1] = SYSTEM_CAPTURE_INST_VIP3_SLICE2_PORTA;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Hdmi receiver Create Parameters
 *
 *          This function is used to set the Video Sensor params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipSingleCameraView_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *          Video sensor Id , Instance Id data format are set.
 *
 * \param   pPrm      [IN]    HdmiRecvr_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SetHdmiRecvrPrms(
                                    HdmiRecvr_CreateParams *pPrm)
{
    Bsp_BoardRev     dcBoardRev;

    if(Bsp_platformIsTda3xxFamilyBuild())
    {
        pPrm->hdmiRecvrId        =    HDMI_RECVR_ADV_7611;
    }
    if(Bsp_platformIsTda2xxFamilyBuild())
    {
        /* SIL9127 not present in REV D VISION cards */
        pPrm->hdmiRecvrId       =    HDMI_RECVR_ADV_7611;
        dcBoardRev = Bsp_boardGetDcRev();
        if (dcBoardRev < BSP_BOARD_REV_D)
        {
            pPrm->hdmiRecvrId   =    HDMI_RECVR_SII_9127;
        }
    }
    pPrm->vipInstId         =    SYSTEM_CAPTURE_INST_VIP1_SLICE1_PORTA;
#ifdef TDA2EX_BUILD
        //This change is done for supporting tda2ex
        pPrm->vipInstId     =   SYSTEM_CAPTURE_INST_VIP1_SLICE2_PORTA;
#endif
    pPrm->standard          =    SYSTEM_STD_CUSTOM;
    pPrm->dataformat        =    SYSTEM_DF_YUV422P;
    pPrm->videoIfWidth      =    SYSTEM_VIFW_16BIT;
    pPrm->videoIfMode        =    SYSTEM_VIFM_SCH_DS_AVID_VSYNC;
    pPrm->numChan           =    1;
}

/**
 *******************************************************************************
 *
 * \brief   Set LCD Create Parameters
 *
 *          It is called in Create function.
 *          Lcd parameters like the device id and brightnessvalue are set here.
 *
 * \param   pPrm    [IN]    Lcd_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SetLcdPrms(Lcd_CreateParams *pPrm)
{
    pPrm->drvId = LCD_CNTR_DRV;
    pPrm->brightnessValue = 100;
}


/**
 *******************************************************************************
 *
 * \brief   Setups Display Controller to dual display on On-chip HDMI and
 *          LCD
 *
 *          Configure such that VID2 and VID3 pipe goto LCD
 *          And VID1 and GRPX pipe goes to HDMI
 *
 *          HDMI is hardcoded to 1080p60 resolution
 *          LCD is hardcoded to 800x480 resolution
 *
 *          NOTE, this is just a sample config use by example use-cases
 *          User's can configure it different in their use-case
 *
 *******************************************************************************
*/
Int32 ChainsCommon_DualDisplay_StartDisplayCtrl(
Chains_DisplayType lcdType, UInt32 displayLCDWidth, UInt32 displayLCDHeight)
{
    Int32 status;
    DisplayCtrlLink_ConfigParams *pPrm = &gChains_commonObj.dctrlCfgPrms;
    DisplayCtrlLink_VencInfo *pVInfo;
    DisplayCtrlLink_OvlyParams *pOvlyPrms;
    DisplayCtrlLink_OvlyPipeParams *pPipeOvlyPrms;
    DisplayCtrlLink_OvlyParams ovlyPrms[2];

    /* Set the link id */
    gChains_commonObj.displayCtrlLinkId = SYSTEM_LINK_ID_DISPLAYCTRL;

    /* Number of valid entries in vencInfo array */
    pPrm->numVencs = 2;
    /* Bitmask of tied vencs. Two vencs, which uses same pixel clock and whose vsync are synchronized, can be tied together. */
    pPrm->tiedVencs = 0;
    /* Activate the HDMI BSP layer in the Dctrl link. This is not required if there is no HDMI display in use. */
    pPrm->deviceId = DISPLAYCTRL_LINK_USE_HDMI;

    /* Configure HDMI display */
    pVInfo                                  = &pPrm->vencInfo[0];
    pVInfo->vencId                          = SYSTEM_DCTRL_DSS_VENC_HDMI;
    pVInfo->outputPort                      = SYSTEM_DCTRL_DSS_HDMI_OUTPUT;
    pVInfo->vencOutputInfo.vsPolarity       = SYSTEM_DCTRL_POLARITY_ACT_HIGH;
    pVInfo->vencOutputInfo.hsPolarity       = SYSTEM_DCTRL_POLARITY_ACT_HIGH;

    /* Below are of dont care for EVM LCD */
    pVInfo->vencOutputInfo.fidPolarity      = SYSTEM_DCTRL_POLARITY_ACT_LOW;
    pVInfo->vencOutputInfo.actVidPolarity   = SYSTEM_DCTRL_POLARITY_ACT_HIGH;

    /* We use 1080p60 - set to SYSTEM_STD_720P_60 for 720p! */
    pVInfo->mInfo.standard                  = SYSTEM_STD_1080P_60;

    /* Configure HDMI overlay parameters */
    pVInfo->mode                            = 0;
    pVInfo->isInputPipeConnected[0]         = TRUE;
    pVInfo->isInputPipeConnected[1]         = FALSE;
    pVInfo->isInputPipeConnected[2]         = FALSE;
    pVInfo->isInputPipeConnected[3]         = TRUE;
    pVInfo->writeBackEnabledFlag            = FALSE;

    pVInfo->vencOutputInfo.dataFormat       = SYSTEM_DF_RGB24_888;
    pVInfo->vencOutputInfo.dvoFormat        = SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
    pVInfo->vencOutputInfo.videoIfWidth     = SYSTEM_VIFW_24BIT;

    pVInfo->vencOutputInfo.pixelClkPolarity = SYSTEM_DCTRL_POLARITY_ACT_HIGH;
    pVInfo->vencOutputInfo.aFmt             = SYSTEM_DCTRL_A_OUTPUT_MAX;

    pOvlyPrms                               = &ovlyPrms[0];
    pOvlyPrms->vencId                       = pVInfo->vencId;
    pOvlyPrms->deltaLinesPerPanel           = 0;
    pOvlyPrms->alphaBlenderEnable           = 0;
    pOvlyPrms->backGroundColor              = 0x10;
    pOvlyPrms->colorKeyEnable               = 1;
    pOvlyPrms->colorKeySel                  = SYSTEM_DSS_DISPC_TRANS_COLOR_KEY_SRC;
    pOvlyPrms->ovlyOptimization             = SYSTEM_DSS_DISPC_OVLY_FETCH_ALLDATA;
    pOvlyPrms->transColorKey                = DRAW2D_TRANSPARENT_COLOR;

/* Configure LCD */
    pVInfo                                  = &pPrm->vencInfo[1];
    pVInfo->vencId                          = SYSTEM_DCTRL_DSS_VENC_LCD1;
    pVInfo->outputPort                      = SYSTEM_DCTRL_DSS_DPI1_OUTPUT;
    pVInfo->vencOutputInfo.vsPolarity       = SYSTEM_DCTRL_POLARITY_ACT_LOW;
    pVInfo->vencOutputInfo.hsPolarity       = SYSTEM_DCTRL_POLARITY_ACT_LOW;

    /* Below are of dont care for EVM LCD */
    pVInfo->vencOutputInfo.fidPolarity      = SYSTEM_DCTRL_POLARITY_ACT_HIGH;
    pVInfo->vencOutputInfo.actVidPolarity   = SYSTEM_DCTRL_POLARITY_ACT_HIGH;

    pVInfo->mInfo.standard                  = SYSTEM_STD_CUSTOM;
    pVInfo->mInfo.scanFormat                = SYSTEM_SF_PROGRESSIVE;
    pVInfo->mInfo.width                     = displayLCDWidth;
    pVInfo->mInfo.height                    = displayLCDHeight;

    if(lcdType == CHAINS_DISPLAY_TYPE_CH7026_480P)
    {
        pVInfo->mInfo.pixelClock                =   27000;
        pVInfo->mInfo.fps                       =   60U;
        pVInfo->mInfo.hSyncLen                  =   62U;
        pVInfo->mInfo.hFrontPorch               =   16U;
        pVInfo->mInfo.hBackPorch                =   60U;

        pVInfo->mInfo.vFrontPorch               =   9U;
        pVInfo->mInfo.vSyncLen                  =   6U;
        pVInfo->mInfo.vBackPorch                =   30U;

		pVInfo->vencDivisorInfo.divisorLCD      = 1;
		pVInfo->vencDivisorInfo.divisorPCD      = 1;
    }
    else if(lcdType == CHAINS_DISPLAY_TYPE_LCD_10_INCH)
    {
        pVInfo->mInfo.pixelClock                =   74500U;
        pVInfo->mInfo.fps                       =   60U;
        pVInfo->mInfo.hBackPorch                =   80U;
        pVInfo->mInfo.hSyncLen                  =   62U;
        pVInfo->mInfo.hFrontPorch               =   48U;
        pVInfo->mInfo.vBackPorch                =   12U;
        pVInfo->mInfo.vSyncLen                  =   35U;
        pVInfo->mInfo.vFrontPorch               =   6U;
        pVInfo->vencDivisorInfo.divisorLCD      =   1;
        pVInfo->vencDivisorInfo.divisorPCD      =   1;
    }
    else
        UTILS_assert(NULL);

    /* Configure LCD overlay params */
    pVInfo->mode = 0;
    pVInfo->isInputPipeConnected[0]         = FALSE;
    pVInfo->isInputPipeConnected[1]         = TRUE;
    pVInfo->isInputPipeConnected[2]         = TRUE;
    pVInfo->isInputPipeConnected[3]         = FALSE;
    pVInfo->writeBackEnabledFlag            = FALSE;

    pVInfo->vencOutputInfo.dataFormat       = SYSTEM_DF_RGB24_888;
    pVInfo->vencOutputInfo.dvoFormat        = SYSTEM_DCTRL_DVOFMT_GENERIC_DISCSYNC;
    pVInfo->vencOutputInfo.videoIfWidth     = SYSTEM_VIFW_24BIT;

    pVInfo->vencOutputInfo.pixelClkPolarity = SYSTEM_DCTRL_POLARITY_ACT_LOW;
    pVInfo->vencOutputInfo.aFmt             = SYSTEM_DCTRL_A_OUTPUT_MAX;

    pOvlyPrms                               = &ovlyPrms[1];
    pOvlyPrms->vencId                       = pVInfo->vencId;
    pOvlyPrms->deltaLinesPerPanel           = 0;
    pOvlyPrms->alphaBlenderEnable           = 0;
    pOvlyPrms->backGroundColor              = 0x10;
    pOvlyPrms->colorKeyEnable               = 1;
    pOvlyPrms->colorKeySel                  = SYSTEM_DSS_DISPC_TRANS_COLOR_KEY_SRC;
    pOvlyPrms->ovlyOptimization             = SYSTEM_DSS_DISPC_OVLY_FETCH_ALLDATA;
    pOvlyPrms->transColorKey                = DRAW2D_TRANSPARENT_COLOR;

    /* Setting HDMI overlay pipe parameters */
    pPipeOvlyPrms                           = &gChains_commonObj.pipeParams[0];
    pPipeOvlyPrms->pipeLine                 = SYSTEM_DSS_DISPC_PIPE_VID1;
    pPipeOvlyPrms->globalAlpha              = 0xFF;
    pPipeOvlyPrms->preMultiplyAlpha         = 0;
    pPipeOvlyPrms->zorderEnable             = TRUE;
    pPipeOvlyPrms->zorder                   = SYSTEM_DSS_DISPC_ZORDER0;

    /* Setting LCD overlay pipe parameters */
    pPipeOvlyPrms                           = &gChains_commonObj.pipeParams[1];
    pPipeOvlyPrms->pipeLine                 = SYSTEM_DSS_DISPC_PIPE_VID2;
    pPipeOvlyPrms->globalAlpha              = 0xFF;
    pPipeOvlyPrms->preMultiplyAlpha         = 0;
    pPipeOvlyPrms->zorderEnable             = TRUE;
    pPipeOvlyPrms->zorder                   = SYSTEM_DSS_DISPC_ZORDER0;

    /* Setting PIP overlay pipe parameters on LCD */
    pPipeOvlyPrms                           = &gChains_commonObj.pipeParams[2];
    pPipeOvlyPrms->pipeLine                 = SYSTEM_DSS_DISPC_PIPE_VID3;
    pPipeOvlyPrms->globalAlpha              = 0xFF;
    pPipeOvlyPrms->preMultiplyAlpha         = 0;
    pPipeOvlyPrms->zorderEnable             = TRUE;
    pPipeOvlyPrms->zorder                   = SYSTEM_DSS_DISPC_ZORDER2;

    /* Configure graphics overlay for HDMI */
    pPipeOvlyPrms                           = &gChains_commonObj.pipeParams[3];
    pPipeOvlyPrms->pipeLine                 = SYSTEM_DSS_DISPC_PIPE_GFX1;
    pPipeOvlyPrms->globalAlpha              = 0xFF;
    pPipeOvlyPrms->preMultiplyAlpha         = 0;
    pPipeOvlyPrms->zorderEnable             = TRUE;
    pPipeOvlyPrms->zorder                   = SYSTEM_DSS_DISPC_ZORDER3;

/* Create, configure and start the Display Ctrl link */
    status = System_linkCreate(gChains_commonObj.displayCtrlLinkId,
                                NULL,
                                0);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_CONFIG,
                                &gChains_commonObj.dctrlCfgPrms,
                                sizeof(DisplayCtrlLink_ConfigParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PARAMS,
                                &ovlyPrms[0],
                                sizeof(DisplayCtrlLink_OvlyParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PARAMS,
                                &ovlyPrms[1],
                                sizeof(DisplayCtrlLink_OvlyParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[0],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[1],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[2],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[3],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Set display link parameters in the case of single video pipe
 *          and one graphics pipe
 *
 *******************************************************************************
*/
Void ChainsCommon_SetGrpxSrcPrms(
                                GrpxSrcLink_CreateParams *pPrm,
                                UInt32 displayWidth,
                                UInt32 displayHeight
                                )
{
    pPrm->grpxBufInfo.dataFormat  = SYSTEM_DF_BGR16_565;
    pPrm->grpxBufInfo.height   = displayHeight;
    pPrm->grpxBufInfo.width    = displayWidth;

    pPrm->logoDisplayEnable = TRUE;
    pPrm->logoParams.startX = 40;
    pPrm->logoParams.startY = 40;

    pPrm->statsDisplayEnable = TRUE;
    pPrm->statsPrintParams.startX = pPrm->logoParams.startX;
    pPrm->statsPrintParams.startY = displayHeight - 134 - pPrm->logoParams.startY;
}

/**
 *******************************************************************************
 *
 * \brief   Set display link parameters in the case of single video pipe
 *          and one graphics pipe
 *
 *******************************************************************************
*/
Void ChainsCommon_SetDisplayPrms(
                                DisplayLink_CreateParams *pPrm_Video,
                                DisplayLink_CreateParams *pPrm_Grpx,
                                Chains_DisplayType displayType,
                                UInt32 displayWidth,
                                UInt32 displayHeight
                                )
{
    if(pPrm_Video)
    {
        if((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
          (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
        {
            pPrm_Video->displayScanFormat = SYSTEM_SF_INTERLACED;
        }

        pPrm_Video->rtParams.tarWidth         = displayWidth;
        pPrm_Video->rtParams.tarHeight        = displayHeight;
        pPrm_Video->displayId                 = DISPLAY_LINK_INST_DSS_VID1;
    }

    if(pPrm_Grpx)
    {
        pPrm_Grpx->displayId                  = DISPLAY_LINK_INST_DSS_GFX1;

        if((displayType == CHAINS_DISPLAY_TYPE_SDTV_NTSC) ||
          (displayType == CHAINS_DISPLAY_TYPE_SDTV_PAL))
        {
            pPrm_Grpx->displayScanFormat = SYSTEM_SF_INTERLACED;
        }

    }
}


/**
 *******************************************************************************
 *
 * \brief   Configure and start display controller
 *
 *******************************************************************************
*/
Int32 ChainsCommon_StartDisplayCtrl(UInt32 displayType,
                                UInt32 displayWidth,
                                UInt32 displayHeight
)
{
    Int32 status=0;

    gChains_commonObj.displayCtrlLinkId = SYSTEM_LINK_ID_DISPLAYCTRL;

    ChainsCommon_SetDctrlConfig(
        &gChains_commonObj.dctrlCfgPrms,
        &gChains_commonObj.ovlyParams,
        &gChains_commonObj.pipeParams[0],
        displayType,
        displayWidth,
        displayHeight
        );

    /* There are no createtime Params for display controller */
    status = System_linkCreate(gChains_commonObj.displayCtrlLinkId,
                                NULL,
                                0);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_CONFIG,
                                &gChains_commonObj.dctrlCfgPrms,
                                sizeof(DisplayCtrlLink_ConfigParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PARAMS,
                                &gChains_commonObj.ovlyParams,
                                sizeof(DisplayCtrlLink_OvlyParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[0],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[1],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    if(Bsp_platformIsTda2xxFamilyBuild())
    {
        status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[2],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_SET_OVLY_PIPELINE_PARAMS,
                                &gChains_commonObj.pipeParams[3],
                                sizeof(DisplayCtrlLink_OvlyPipeParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);


    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Stop and Delete display controller
 *
 *******************************************************************************
*/
Int32 ChainsCommon_StopDisplayCtrl()
{
    Int32 status=0;

    status = System_linkControl(gChains_commonObj.displayCtrlLinkId,
                                DISPLAYCTRL_LINK_CMD_CLR_CONFIG,
                                &gChains_commonObj.dctrlCfgPrms,
                                sizeof(DisplayCtrlLink_ConfigParams),
                                TRUE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_linkDelete(gChains_commonObj.displayCtrlLinkId);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

Void ChainsCommon_InitCaptureDevice(Chains_CaptureSrc captureSrc)
{
    Vps_printf(" CHAINS: Sensor init in progress !!!\n");

    VidSensor_CreateParams_Init(&gChains_commonObj.vidSensorPrm);
    HdmiRecvr_CreateParams_Init(&gChains_commonObj.hdmiRecvrPrm);

    if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_MULDES_AR0132RCCC,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10635)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10635,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_CSI2)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10640,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10640,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_IMX224_CSI2,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0132_BAYER,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0132_MONOCHROME,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0140_BAYER,
                NULL,
                1,
                captureSrc
                );
    }
    else if((captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P) ||
        (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P))
    {
        ChainsCommon_SetHdmiRecvrPrms(&gChains_commonObj.hdmiRecvrPrm);
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0140_BAYER,
                NULL,
                4U,
                captureSrc
                );
    }
    else
    {
        /* Un Recognized Capture source */
        UTILS_assert(FALSE);
    }

    Vps_printf(" CHAINS: Sensor init ... DONE !!!\n");
}

/**
 *******************************************************************************
 *
 * \brief   Configure and start single channel capture device
 *
 *******************************************************************************
*/
Int32 ChainsCommon_StartCaptureDevice(Chains_CaptureSrc captureSrc,
                        UInt32 captureOutWidth,
                        UInt32 captureOutHeight
                        )
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VidSensorVar_Id sensorVariant = VID_SENSOR_VAR_NONE;
    VidSensor_CreateStatus vidSensorStatus;
    HdmiRecvr_CreateStatus hdmiRecvrStatus;

    Vps_printf(" CHAINS: Sensor breate in progress !!!\n");

    VidSensor_CreateParams_Init(&gChains_commonObj.vidSensorPrm);
    HdmiRecvr_CreateParams_Init(&gChains_commonObj.hdmiRecvrPrm);

    if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_MULDES_AR0132RCCC,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10635)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10635,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_CSI2)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10640,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10640,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_IMX224_CSI2,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0132_BAYER,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0132_MONOCHROME,
                NULL,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0140_BAYER,
                NULL,
                1,
                captureSrc
                );
    }
    else if((captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P) ||
        (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P))
    {
        ChainsCommon_SetHdmiRecvrPrms(&gChains_commonObj.hdmiRecvrPrm);
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0140_BAYER,
                NULL,
                4U,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_DM388)
    {
        /* Nothing to be done for MonsterCam */
    }
    else
    {
        /* Un Recognized Capture source */
        UTILS_assert(FALSE);
    }

    if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10635
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10640_CSI2
        ||
       captureSrc == CHAINS_CAPTURE_SRC_IMX224_CSI2
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10640_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262
        )
    {
        if (captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
        {
            sensorVariant = VID_SENSOR_VAR_AR0140_BAYER_TIDA00262;
        }
        if ((captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL) &&
                (CHAINS_ISS_WDR_MODE_SINGLE_PASS == gChains_commonObj.issWdrMode))
        {
            sensorVariant = VID_SENSOR_VAR_AR0140_BAYER_SINGLE_PASS;
        }

        status = VidSensor_create(&gChains_commonObj.vidSensorPrm,
                                    sensorVariant,
                                    &vidSensorStatus);

        status = VidSensor_control(&gChains_commonObj.vidSensorPrm,
                                    VID_SENSOR_CMD_START,
                                    NULL,
                                    NULL);

        status = VidSensor_control(&gChains_commonObj.vidSensorPrm,
                                    VID_SENSOR_CMD_SET_WDR_MODE,
                                    &gChains_commonObj.issWdrMode,
                                    NULL);
    }
    else if((captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P) ||
        (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P))
    {
        status = HdmiRecvr_create(&gChains_commonObj.hdmiRecvrPrm, &hdmiRecvrStatus);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        status = HdmiRecvr_control(&gChains_commonObj.hdmiRecvrPrm,
                                   HDMI_RECVR_CMD_START,
                                   &gChains_commonObj.issWdrMode,
                                   NULL);
    }

    Vps_printf(" CHAINS: Sensor create ... DONE !!!\n");

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Configure Sensor for WDR or non WDR mode
 *
 *******************************************************************************
*/
Void ChainsCommon_SetWdrMode(Chains_IssWdrMode issWdrMode)
{
    gChains_commonObj.issWdrMode = issWdrMode;
}


/**
 *******************************************************************************
 *
 * \brief   Configure and start multi-channel capture device
 *
 *******************************************************************************
*/
Int32 ChainsCommon_MultiCam_StartCaptureDevice( Chains_CaptureSrc captureSrc,
                                    UInt32 portId[],
                                    UInt32 numLvdsCh
                                    )
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VidSensor_CreateStatus vidSensorStatus;

    UTILS_assert(captureSrc==CHAINS_CAPTURE_SRC_OV10635);

    ChainsCommon_SetVidSensorPrms(
            &gChains_commonObj.vidSensorPrm,
            VID_SENSOR_MULDES_OV1063X,
            portId,
            numLvdsCh,
            captureSrc
            );

    status = VidSensor_create(&gChains_commonObj.vidSensorPrm,
                                VID_SENSOR_VAR_NONE,
                                &vidSensorStatus);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    VidSensor_control(&gChains_commonObj.vidSensorPrm,
                                VID_SENSOR_CMD_START,
                                NULL,
                                NULL);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Configure and start Sensor Devices for the ISS Usecase
 *
 *******************************************************************************
*/
Int32 ChainsCommon_IssStartCaptureDevice(Chains_CaptureSrc captureSrc,
                                         UInt32 captureOutWidth,
                                         UInt32 captureOutHeight,
                                         UInt32 instId)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    VidSensorVar_Id sensorVariant = VID_SENSOR_VAR_NONE;
    VidSensor_CreateStatus vidSensorStatus;

    Vps_printf(" CHAINS: Sensor create in progress !!!\n");

    VidSensor_CreateParams_Init(&gChains_commonObj.vidSensorPrm);
    HdmiRecvr_CreateParams_Init(&gChains_commonObj.hdmiRecvrPrm);

    if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_CSI2)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10640,
                &instId,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_OV10640,
                &instId,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_IMX224_CSI2)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_IMX224_CSI2,
                &instId,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0132_BAYER,
                &instId,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0132_MONOCHROME,
                &instId,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0140_BAYER,
                &instId,
                1,
                captureSrc
                );
    }
    else if(captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
    {
        ChainsCommon_SetVidSensorPrms(
                &gChains_commonObj.vidSensorPrm,
                VID_SENSOR_AR0140_BAYER,
                NULL,
                4U,
                captureSrc
                );
    }
    else
    {
        /* Un Recognized Capture source */
        UTILS_assert(FALSE);
    }

    if(captureSrc == CHAINS_CAPTURE_SRC_OV10640_CSI2
        ||
       captureSrc == CHAINS_CAPTURE_SRC_IMX224_CSI2
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10640_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262
        )
    {
        if (captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
        {
            sensorVariant = VID_SENSOR_VAR_AR0140_BAYER_TIDA00262;
        }

        status = VidSensor_create(&gChains_commonObj.vidSensorPrm,
                                    sensorVariant,
                                    &vidSensorStatus);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        status = VidSensor_control(&gChains_commonObj.vidSensorPrm,
                                    VID_SENSOR_CMD_START,
                                    NULL,
                                    NULL);
    }

    Vps_printf(" CHAINS: Sensor create DONE !!!\n");

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}


/**
 *******************************************************************************
 *
 * \brief   Stop capture device
 *
 *******************************************************************************
*/
Int32 ChainsCommon_StopCaptureDevice(Chains_CaptureSrc captureSrc)
{
    Int32 status=0;
    VidSensorVar_Id sensorVariant = VID_SENSOR_VAR_NONE;

    if(captureSrc == CHAINS_CAPTURE_SRC_AR0132RCCC
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10635
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10640_CSI2
        ||
       captureSrc == CHAINS_CAPTURE_SRC_OV10640_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_IMX224_CSI2
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL
        ||
       captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262
        )
    {
        if (captureSrc == CHAINS_CAPTURE_SRC_UB960_TIDA00262)
        {
            sensorVariant = VID_SENSOR_VAR_AR0140_BAYER_TIDA00262;
        }

        VidSensor_control(&gChains_commonObj.vidSensorPrm,
                                    VID_SENSOR_CMD_STOP,
                                    NULL,
                                    NULL);

        VidSensor_delete(&gChains_commonObj.vidSensorPrm, sensorVariant, NULL);
    }
    else if((captureSrc == CHAINS_CAPTURE_SRC_HDMI_720P) ||
        (captureSrc == CHAINS_CAPTURE_SRC_HDMI_1080P))
    {
        status = HdmiRecvr_control(&gChains_commonObj.hdmiRecvrPrm,
                                    HDMI_RECVR_CMD_STOP,
                                    NULL,
                                    NULL);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        HdmiRecvr_delete(&gChains_commonObj.hdmiRecvrPrm, NULL);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Start Display device
 *
 *******************************************************************************
*/
Int32 ChainsCommon_StartDisplayDevice(UInt32 displayType)
{
    Int32 status=0;

    if (displayType == CHAINS_DISPLAY_TYPE_LCD_10_INCH )
    {
        ChainsCommon_SetLcdPrms(&gChains_commonObj.lcdPrm);

        gChains_commonObj.lcdInstId = gChains_commonObj.lcdPrm.drvId;
        status = Lcd_turnOn(gChains_commonObj.lcdInstId, &gChains_commonObj.lcdPrm);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }else
    if(displayType == CHAINS_DISPLAY_TYPE_HDMI_720P
        ||
      displayType == CHAINS_DISPLAY_TYPE_HDMI_1080P
        ||
      displayType == CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM
        )
    {
        if(Bsp_platformIsTda3xxFamilyBuild())
        {
            HdmiTx_CreateStatus hdmiTxStatus;

            HdmiTx_CreateParams_Init(&gChains_commonObj.hdmiTxPrm);

            if(displayType == CHAINS_DISPLAY_TYPE_HDMI_1080P)
            {
                gChains_commonObj.hdmiTxPrm.standard = SYSTEM_STD_1080P_60;
            }
            if(displayType == CHAINS_DISPLAY_TYPE_HDMI_720P)
            {
                gChains_commonObj.hdmiTxPrm.standard = SYSTEM_STD_720P_60;
            }
            if(displayType == CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM)
            {
                gChains_commonObj.hdmiTxPrm.standard = SYSTEM_STD_XGA_DSS_TDM_60;
                gChains_commonObj.hdmiTxPrm.boardMode = BSP_BOARD_MODE_VIDEO_8BIT_TDM;
            }

            status = HdmiTx_create(&gChains_commonObj.hdmiTxPrm, &hdmiTxStatus);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            status = HdmiTx_control(&gChains_commonObj.hdmiTxPrm,
                                    HDMI_TX_CMD_START,
                                    NULL,
                                    NULL);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }else
    if (displayType == CHAINS_DISPLAY_TYPE_CH7026_480P)
	{
		Component_CreateStatus componentStatus;
		gChains_commonObj.componentPrm.standard        =  SYSTEM_STD_480P;
		gChains_commonObj.componentPrm.boardMode       =  BSP_BOARD_MODE_VIDEO_24BIT;

		status = Component_create(&gChains_commonObj.componentPrm, &componentStatus);
		UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
	}

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Start Display device
 *
 *******************************************************************************
*/
Int32 ChainsCommon_StopDisplayDevice(UInt32 displayType)
{
    Int32 status=0;

    if ( displayType == CHAINS_DISPLAY_TYPE_LCD_10_INCH )
    {
        Lcd_turnOff(gChains_commonObj.lcdInstId);
    }
    if(displayType == CHAINS_DISPLAY_TYPE_HDMI_720P
        ||
      displayType == CHAINS_DISPLAY_TYPE_HDMI_1080P
        ||
      displayType == CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM
        )
    {
        if(Bsp_platformIsTda3xxFamilyBuild())
        {
            status = HdmiTx_control(&gChains_commonObj.hdmiTxPrm,
                                    HDMI_TX_CMD_STOP,
                                    NULL,
                                    NULL);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

            status = HdmiTx_delete(&gChains_commonObj.hdmiTxPrm,
                                    NULL);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief   Set L3/DMM priority related registers
 *
 *******************************************************************************
*/
Void ChainsCommon_SetSystemL3DmmPri()
{
    /* Assert Mflag of DSS to give DSS highest priority */
    Utils_setDssMflagMode(UTILS_DSS_MFLAG_MODE_FORCE_ENABLE);

    /* enable usage of Mflag at DMM */
    Utils_setDmmMflagEmergencyEnable(TRUE);

    /* Set DMM as higest priority at DMM and EMIF */
    Utils_setDmmPri(UTILS_DMM_INITIATOR_ID_DSS, 0);
}

/**
 *******************************************************************************
 *
 * \brief   Print Statistics
 *
 *******************************************************************************
*/
Void ChainsCommon_PrintStatistics()
{
    System_linkControl(
        SYSTEM_LINK_ID_IPU1_0,
        SYSTEM_LINK_CMD_PRINT_CORE_PRF_LOAD,
        NULL,
        0,
        TRUE);

    Utils_prcmPrintAllVDTempValues();
    Chains_statCollectorPrint();
}

/**
 *******************************************************************************
 *
 * \brief   Load Calculation enable/disable
 *
 *          This functions enables load profiling. A control command
 *          SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START is passed to chianed links.
 *          If parameter Enable is set true Load profiling is enabled.
 *          If printStatus is set true a System CMD to Print CPU load,
 *          Task Laod and Heap status information is sent
 *          While creating enable = TRUE , printStatus & printTskLoad = FALSE
 *          While deleting enable = FALSE , printStatus & printTskLoad = TRUE
 *
 * \param   enable               [IN]   is set true Load profiling
 *
 * \param   printStatus          [IN] true a System CMD
 *
 * \param   printTskLoad         [IN]  true a Print CPU load
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad)
{
    UInt32 procId, linkId;


    for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
    {
        if(System_isProcEnabled(procId)==FALSE)
            continue;

        linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);

        if(enable)
        {
            System_linkControl(
                linkId,
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START,
                NULL,
                0,
                TRUE
            );
        }
        else
        {
            System_linkControl(
                linkId,
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_STOP,
                NULL,
                0,
                TRUE
            );
            if(printStatus)
            {
                SystemCommon_PrintStatus printStatus;

                memset(&printStatus, 0, sizeof(printStatus));

                printStatus.printCpuLoad = TRUE;
                printStatus.printTskLoad = printTskLoad;
                System_linkControl(
                    linkId,
                    SYSTEM_COMMON_CMD_PRINT_STATUS,
                    &printStatus,
                    sizeof(printStatus),
                    TRUE
                );
                Task_sleep(100);
            }
            System_linkControl(
                linkId,
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET,
                NULL,
                0,
                TRUE
            );
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   Print Load Calculation.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_prfCpuLoadPrint()
{
    UInt32 procId, linkId;
    SystemCommon_PrintStatus printStatus;

    memset(&printStatus, 0, sizeof(printStatus));

    printStatus.printCpuLoad = TRUE;
    printStatus.printTskLoad = TRUE;

    for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
    {
        if(System_isProcEnabled(procId)==FALSE)
            continue;

        linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);

        System_linkControl(
            linkId,
            SYSTEM_COMMON_CMD_PRINT_STATUS,
            &printStatus,
            sizeof(printStatus),
            TRUE
        );
        Task_sleep(100);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   Print Memory Heap Statistics
 *
 *          This function send a system control message
 *          SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START to all cores.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_memPrintHeapStatus()
{
    UInt32 procId, linkId;
    SystemCommon_PrintStatus printStatus;

    memset(&printStatus, 0, sizeof(printStatus));

    printStatus.printHeapStatus = TRUE;

    for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
    {
        if(System_isProcEnabled(procId)==FALSE)
            continue;

        linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);

        System_linkControl(
                linkId,
                SYSTEM_COMMON_CMD_PRINT_STATUS,
                &printStatus,
                sizeof(printStatus),
                TRUE
            );
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   This function Print the statCollector output
 *          SYSTEM_COMMON_CMD_PRINT_STAT_COLL send to only IPU1_0 core.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_statCollectorPrint()
{
    UInt32 linkId;

    linkId = SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_IPU1_0, SYSTEM_LINK_ID_PROCK_LINK_ID);

        System_linkControl(
            linkId,
            SYSTEM_COMMON_CMD_PRINT_STAT_COLL,
            NULL,
            0,
            TRUE
        );
        Task_sleep(100);


    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   This function Reset the statCollector registers
 *          SYSTEM_COMMON_CMD_RESET_STAT_COLL send to only IPU1_0 core.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_statCollectorReset()
{
    UInt32 linkId;

    linkId = SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_IPU1_0, SYSTEM_LINK_ID_PROCK_LINK_ID);

        System_linkControl(
            linkId,
            SYSTEM_COMMON_CMD_RESET_STAT_COLL,
            NULL,
            0,
            TRUE
        );

    return SYSTEM_LINK_STATUS_SOK;
}


VidSensor_CreateParams *ChainsCommon_GetSensorCreateParams()
{
    return (&gChains_commonObj.vidSensorPrm);
}

