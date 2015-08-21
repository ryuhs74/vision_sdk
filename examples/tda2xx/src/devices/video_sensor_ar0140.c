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
 * \file video_sensor_ar140.c
 *
 * \brief  This file implemented AR140 sensor specific controls
 *
 *         This implements APIs and Helper functions for configuring
 *         color correction and exposure for AR140 sensor.
 *
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <examples/tda2xx/include/video_sensor.h>

#include <fvid2/fvid2.h>
#include <vps/vps.h>

#include "video_sensor_priv.h"

#define VID_SENSOR_AR140_RGB2RGB_SWITCH_LIMIT       (2u)

//#define _USE_DIGITAL_GAIN_

/**
 * \brief Structure for ISP H3A AEWB engine parameters.
 */
static vpsissH3aAewbConfig_t aewbConfig_ar140 =
{
    FALSE, /* enableALowComp */
    FALSE, /* enableMedFilt */
    0xFFF, /* midFiltThreshold */
    {   /* vpsissH3aPaxelConfig_t      winCfg */
        {32u, 16u}, /* Fvid2_PosConfig pos */
        64u,     /* width */
        24u,     /* height */
        16u,     /* horzCount */
        32u,     /* vertCount */
        4,      /* horzIncr */
        4,      /* vertIncr */
    },
    798U,       /* Black Line Vertical Start */
    2U,         /* Black Line Height */
    VPS_ISS_H3A_OUTPUT_MODE_SUM_ONLY, /* vpsissH3aOutputMode_t outMode; */
    0,  /* sumShift */
    1023u, /* satLimit */
    VPS_ISS_H3A_MODE_NORMAL /* vpsissH3aMode_t mode */

} ;

static vpsissIsifGainOfstConfig_t isifWbCfg_ar140 = {0};

static vpsissIpipeInConfig_t ipipeInputCfg_ar140 =
        {VPS_ISS_IPIPE_DATA_PATH_RAW_YUV422};

static vpsissIpipeRgb2RgbConfig_t    rgb2rgb1Cfg = {{ \
                    {338, -90, 8}, \
                    {-122, 275, 103}, \
                    {-62, -320, 638}}, {0, 0, 0}};

static vpsissIpipeRgb2RgbConfig_t    rgb2rgb2Cfg = {{ \
                    {249, 0, 7}, \
                    {-2, 264, -6}, \
                    {-13, -2, 271}}, {0, 0, 0}};

static vpsissGlbceWdrConfig_t glbceWdrCfg_ar0140 =
{
    TRUE,
    {
        0U,24U,64U,114U,172U,237U,307U,383U,464U,549U,638U,731U,828U,928U,
        1031U,1138U,1248U,1361U,1477U,1596U,1717U,1841U,1967U,2096U,2228U,
        2361U,2498U,2636U,2777U,2919U,3064U,3211U,3360U,3511U,3664U,3819U,
        3976U,4134U,4295U,4457U,4622U,4787U,4955U,5125U,5296U,5468U,5643U,
        5819U,5997U,6176U,6357U,6539U,6723U,6908U,7095U,7284U,7474U,7665U,
        7858U,8052U,8248U,8445U,8644U,8843U,9045U,9247U,9451U,9656U,9863U,
        10071U,10280U,10490U,10702U,10915U,11129U,11345U,11561U,11779U,11998U,
        12219U,12440U,12663U,12887U,13112U,13338U,13566U,13794U,14024U,14255U,
        14487U,14720U,14954U,15189U,15426U,15663U,15902U,16142U,16382U,16624U,
        16867U,17111U,17356U,17602U,17849U,18097U,18346U,18596U,18847U,19099U,
        19353U,19607U,19862U,20118U,20375U,20633U,20892U,21152U,21413U,21675U,
        21938U,22202U,22467U,22732U,22999U,23267U,23535U,23805U,24075U,24346U,
        24618U,24891U,25165U,25440U,25716U,25993U,26270U,26549U,26828U,27108U,
        27389U,27671U,27954U,28238U,28522U,28807U,29094U,29381U,29669U,29957U,
        30247U,30537U,30829U,31121U,31414U,31707U,32002U,32297U,32593U,32890U,
        33188U,33487U,33786U,34086U,34387U,34689U,34992U,35295U,35599U,35904U,
        36210U,36516U,36823U,37132U,37440U,37750U,38060U,38371U,38683U,38996U,
        39309U,39623U,39938U,40254U,40570U,40887U,41205U,41523U,41843U,42163U,
        42483U,42805U,43127U,43450U,43774U,44098U,44423U,44749U,45075U,45403U,
        45731U,46059U,46389U,46719U,47049U,47381U,47713U,48046U,48379U,48714U,
        49048U,49384U,49720U,50057U,50395U,50733U,51072U,51412U,51752U,52093U,
        52435U,52777U,53121U,53464U,53809U,54154U,54499U,54846U,55193U,55540U,
        55889U,56238U,56587U,56938U,57289U,57640U,57992U,58345U,58699U,59053U,
        59408U,59763U,60119U,60476U,60833U,61191U,61550U,61909U,62269U,62629U,
        62990U,63352U,63714U,64077U,64441U,64805U,65170U,65535U
    }
};


Void VidSensor_SetIssIspConfig_ar0140(IssIspConfigurationParameters *pIspConfig)
{
    UInt32 outCnt, colorCnt;

    /* Override common settings for specific sensor */
    pIspConfig->aewbCfg = &aewbConfig_ar140;

    /* isifWbCfg */
    pIspConfig->isifWbCfg = &isifWbCfg_ar140;
    /* Enable Gains and Offsets for all three outputs */
    for(outCnt = 0u; outCnt < VPS_ISS_ISIF_MAX_OUTPUT; outCnt++)
    {
        pIspConfig->isifWbCfg->gainEnable[outCnt] = TRUE;
        pIspConfig->isifWbCfg->offsetEnable[outCnt] = TRUE;
    }
    for(colorCnt = 0u; colorCnt < FVID2_BAYER_COLOR_COMP_MAX; colorCnt++)
    {
        pIspConfig->isifWbCfg->gain[colorCnt] = 512;
    }
    /* Setting Offset to 0 */
    pIspConfig->isifWbCfg->offset = 0U;

    pIspConfig->ipipeInputCfg = &ipipeInputCfg_ar140;

    ipipeInputCfg_ar140.dataPath = VPS_ISS_IPIPE_DATA_PATH_RAW_YUV422;
    ipipeInputCfg_ar140.procWin.cropStartX = 0;
    ipipeInputCfg_ar140.procWin.cropStartY = 2;
    ipipeInputCfg_ar140.procWin.cropWidth  = 1280;
    ipipeInputCfg_ar140.procWin.cropHeight
            =  800 - ipipeInputCfg_ar140.procWin.cropStartY;

    pIspConfig->rgb2rgb1Cfg = &rgb2rgb1Cfg;
    pIspConfig->rgb2rgb2Cfg = &rgb2rgb2Cfg;
}

Void VidSensor_SetIssIspGlbceConfig_ar0140(
    IssIspConfigurationParameters *pIspConfig)
{
    UTILS_assert(NULL != pIspConfig);

    /* WDR Config */
    pIspConfig->glbceWdrCfg = &glbceWdrCfg_ar0140;
}

Void VidSensor_SetAewbParams_ar0140(AlgorithmLink_IssAewbCreateParams *prms,
    UInt32 isOnePassWdr)
{
    AlgorithmLink_IssAewbAeDynamicParams *dynPrms = NULL;

    /* Enable Auto Exposure */
    prms->mode = ALGORITHMS_ISS_AEWB_MODE_AEWB;
    prms->dccCameraId = 0;

    dynPrms = &prms->aeDynParams;

    if (isOnePassWdr)
    {
        dynPrms->targetBrightnessRange.min = 18;
        dynPrms->targetBrightnessRange.max = 22;
        dynPrms->targetBrightness = 20;
    }
    else
    {
        dynPrms->targetBrightnessRange.min = 35;
        dynPrms->targetBrightnessRange.max = 45;
        dynPrms->targetBrightness = 40;
    }
    dynPrms->threshold = 5;

    dynPrms->exposureTimeRange[0].min = 100;
    dynPrms->exposureTimeRange[0].max = 8333;
    dynPrms->apertureLevelRange[0].min = 1;
    dynPrms->apertureLevelRange[0].max = 1;
    dynPrms->sensorGainRange[0].min = 1000;
    dynPrms->sensorGainRange[0].max = 1000;
    dynPrms->ipipeGainRange[0].min = 512;
    dynPrms->ipipeGainRange[0].max = 512;

    dynPrms->exposureTimeRange[1].min = 8333;
    dynPrms->exposureTimeRange[1].max = 8333;
    dynPrms->apertureLevelRange[1].min = 1;
    dynPrms->apertureLevelRange[1].max = 1;
    dynPrms->sensorGainRange[1].min = 1000;
    dynPrms->sensorGainRange[1].max = 2000;
    dynPrms->ipipeGainRange[1].min = 512;
    dynPrms->ipipeGainRange[1].max = 512;


//Limiting exposure time to 16.6 ms for 16x WDR mode
    dynPrms->exposureTimeRange[2].min = 16666;
    dynPrms->exposureTimeRange[2].max = 16666;
    dynPrms->apertureLevelRange[2].min = 1;
    dynPrms->apertureLevelRange[2].max = 1;
    dynPrms->sensorGainRange[2].min = 1000;
    dynPrms->sensorGainRange[2].max = 1150;
    dynPrms->ipipeGainRange[2].min = 512;
    dynPrms->ipipeGainRange[2].max = 512;

    dynPrms->exposureTimeRange[3].min = 16666;
    dynPrms->exposureTimeRange[3].max = 16666;
    dynPrms->apertureLevelRange[3].min = 1;
    dynPrms->apertureLevelRange[3].max = 1;
    dynPrms->sensorGainRange[3].min = 1000;
    dynPrms->sensorGainRange[3].max = 12000;
    dynPrms->ipipeGainRange[3].min = 512;
    dynPrms->ipipeGainRange[3].max = 512;

#ifdef _USE_DIGITAL_GAIN_
    dynPrms->exposureTimeRange[4].min = 16666;
    dynPrms->exposureTimeRange[4].max = 16666;
    dynPrms->apertureLevelRange[4].min = 1;
    dynPrms->apertureLevelRange[4].max = 1;
    dynPrms->sensorGainRange[4].min = 12000;
    dynPrms->sensorGainRange[4].max = 12000;
    dynPrms->ipipeGainRange[4].min = 512;
    dynPrms->ipipeGainRange[4].max = 4095;

    dynPrms->numAeDynParams = 5;
#else
    dynPrms->numAeDynParams = 4;
#endif
    dynPrms->exposureTimeStepSize = 1;
    dynPrms->enableBlc = TRUE;

    prms->calbData = NULL;
}
