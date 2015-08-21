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
 * \file video_sensor_ar132.c
 *
 * \brief  This file implemented AR132 sensor specific controls
 *
 *         This implements APIs and Helper functions for configuring
 *         color correction and exposure for AR132 sensor.
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

#define VID_SENSOR_AR132_RGB2RGB_SWITCH_LIMIT       (10u)

/**
 * \brief Structure for ISP H3A AEWB engine parameters.
 */
static vpsissH3aAewbConfig_t aewbConfig_ar132 =
{
    FALSE, /* enableALowComp */
    FALSE, /* enableMedFilt */
    0xFFF, /* midFiltThreshold */
    {   /* vpsissH3aPaxelConfig_t      winCfg */
        {32u, 16u}, /* Fvid2_PosConfig pos */
        64u,     /* width */
        24u,     /* height */
        16u,     /* horzCount */
        28u,     /* vertCount */
        4,      /* horzIncr */
        4,      /* vertIncr */
    },
    718U,       /* Black Line Vertical Start */
    2U,         /* Black Line Height */
    VPS_ISS_H3A_OUTPUT_MODE_SUM_ONLY, /* vpsissH3aOutputMode_t outMode; */
    0,  /* sumShift */
    1023u, /* satLimit */
    VPS_ISS_H3A_MODE_NORMAL /* vpsissH3aMode_t mode */

} ;

static vpsissIpipeCfaConfig_t cfaCfg_ar132 =
{
    VPS_ISS_IPIPE_CFA_MODE_2DIRAC,
    600U,
    57U,
    100U,
    10U,
    16U,
    10U,
    16U,

    {
        16U,
        20U,
        7U,
        20U,
        200U,
        20U,
        32U
    }
};

static vpsissIpipeRgb2RgbConfig_t    rgb2rgb1Cfg_ar132 =
{
    {
        {348, 0xffffffb0, 0xfffffff4},
        {0xffb9, 345, 0xffffffee},
        {7, 0xffffff25, 468}
    },
    {0U,      0U,      0U     }
};
static vpsissIpipeRgb2RgbConfig_t    rgb2rgb2Cfg_ar132 =
{
    {
        {256, 0, 0},
        {0, 256, 0},
        {0, 0, 256}
    },
    {0U,      0U,      0U     }
};

static vpsissIsifGainOfstConfig_t isifWbCfg_ar132 = {0};
static vpsissIpipeInConfig_t ipipeInputCfg_ar132 =
        {VPS_ISS_IPIPE_DATA_PATH_RAW_YUV422};
static vpsissIpipeGammaConfig_t gammaCfg_ar132 = {
    VPS_ISS_IPIPE_GAMMA_TBL_RAM,
    VPS_ISS_IPIPE_GAMMA_TBL_SIZE_512,
    TRUE,
    TRUE,
    TRUE,
    NULL,
    NULL,
    NULL
};

static uint32_t gamma_lut_ar132[] = {
0       ,9,
8       ,9,
16      ,9,
24      ,9,
36      ,9,
44      ,9,
52      ,9,
60      ,9,
72      ,9,
80      ,9,
88      ,8,
96      ,8,
104     ,8,
112     ,7,
120     ,7,
128     ,7,
132     ,7,
140     ,6,
148     ,6,
152     ,6,
160     ,6,
164     ,6,
172     ,5,
176     ,6,
180     ,5,
188     ,5,
192     ,5,
196     ,5,
200     ,5,
208     ,5,
212     ,4,
216     ,5,
220     ,4,
224     ,5,
228     ,4,
232     ,4,
236     ,5,
244     ,4,
248     ,4,
252     ,4,
256     ,4,
260     ,4,
264     ,4,
268     ,4,
272     ,3,
272     ,4,
276     ,4,
280     ,3,
284     ,4,
288     ,4,
292     ,3,
296     ,4,
300     ,3,
304     ,4,
308     ,3,
308     ,3,
312     ,4,
316     ,3,
320     ,3,
324     ,3,
324     ,4,
328     ,3,
332     ,3,
336     ,3,
340     ,3,
340     ,3,
344     ,3,
348     ,3,
352     ,3,
352     ,3,
356     ,3,
360     ,3,
364     ,3,
364     ,3,
368     ,3,
372     ,3,
376     ,2,
376     ,3,
380     ,3,
384     ,3,
384     ,2,
388     ,3,
392     ,3,
392     ,3,
396     ,2,
400     ,3,
400     ,3,
404     ,2,
408     ,3,
408     ,2,
412     ,3,
416     ,2,
416     ,3,
420     ,2,
420     ,3,
424     ,2,
428     ,3,
428     ,2,
432     ,3,
436     ,2,
436     ,3,
440     ,2,
440     ,3,
444     ,2,
448     ,2,
448     ,3,
452     ,2,
452     ,2,
456     ,3,
460     ,2,
460     ,2,
464     ,3,
464     ,2,
468     ,2,
468     ,2,
472     ,3,
476     ,2,
476     ,2,
480     ,2,
480     ,2,
484     ,3,
484     ,2,
488     ,2,
488     ,2,
492     ,2,
492     ,2,
496     ,3,
500     ,2,
500     ,2,
504     ,2,
504     ,2,
508     ,2,
508     ,2,
512     ,2,
512     ,2,
516     ,2,
516     ,2,
520     ,2,
520     ,2,
524     ,3,
524     ,2,
528     ,2,
528     ,2,
532     ,2,
532     ,2,
536     ,1,
536     ,2,
540     ,2,
540     ,2,
544     ,2,
544     ,2,
548     ,2,
548     ,2,
552     ,2,
552     ,2,
556     ,2,
556     ,2,
560     ,2,
560     ,2,
564     ,1,
564     ,2,
564     ,2,
568     ,2,
568     ,2,
572     ,2,
572     ,2,
576     ,1,
576     ,2,
580     ,2,
580     ,2,
584     ,2,
584     ,1,
584     ,2,
588     ,2,
588     ,2,
592     ,2,
592     ,1,
596     ,2,
596     ,2,
600     ,2,
600     ,1,
600     ,2,
604     ,2,
604     ,2,
608     ,1,
608     ,2,
612     ,2,
612     ,2,
616     ,1,
616     ,2,
616     ,2,
620     ,1,
620     ,2,
624     ,2,
624     ,1,
624     ,2,
628     ,2,
628     ,2,
632     ,1,
632     ,2,
636     ,1,
636     ,2,
636     ,2,
640     ,1,
640     ,2,
644     ,2,
644     ,1,
644     ,2,
648     ,2,
648     ,1,
652     ,2,
652     ,1,
652     ,2,
656     ,2,
656     ,1,
660     ,2,
660     ,1,
660     ,2,
664     ,2,
664     ,1,
668     ,2,
668     ,1,
668     ,2,
672     ,1,
672     ,2,
676     ,2,
676     ,1,
676     ,2,
680     ,1,
680     ,2,
684     ,1,
684     ,2,
684     ,1,
688     ,2,
688     ,1,
688     ,2,
692     ,1,
692     ,2,
696     ,1,
696     ,2,
696     ,1,
700     ,2,
700     ,1,
700     ,2,
704     ,1,
704     ,2,
708     ,1,
708     ,2,
708     ,1,
712     ,2,
712     ,1,
712     ,2,
716     ,1,
716     ,2,
720     ,1,
720     ,1,
720     ,2,
724     ,1,
724     ,2,
724     ,1,
728     ,2,
728     ,1,
728     ,2,
732     ,1,
732     ,1,
732     ,2,
736     ,1,
736     ,2,
740     ,1,
740     ,1,
740     ,2,
744     ,1,
744     ,2,
744     ,1,
748     ,1,
748     ,2,
748     ,1,
752     ,2,
752     ,1,
752     ,1,
756     ,2,
756     ,1,
756     ,1,
760     ,2,
760     ,1,
760     ,2,
764     ,1,
764     ,1,
764     ,2,
768     ,1,
768     ,1,
768     ,2,
772     ,1,
772     ,1,
772     ,2,
776     ,1,
776     ,1,
776     ,2,
780     ,1,
780     ,1,
780     ,2,
784     ,1,
784     ,1,
784     ,2,
788     ,1,
788     ,1,
788     ,2,
792     ,1,
792     ,1,
792     ,2,
796     ,1,
796     ,1,
796     ,1,
800     ,2,
800     ,1,
800     ,1,
804     ,2,
804     ,1,
804     ,1,
808     ,1,
808     ,2,
808     ,1,
812     ,1,
812     ,2,
812     ,1,
816     ,1,
816     ,1,
816     ,2,
820     ,1,
820     ,1,
820     ,1,
820     ,2,
824     ,1,
824     ,1,
824     ,1,
828     ,2,
828     ,1,
828     ,1,
832     ,1,
832     ,2,
832     ,1,
836     ,1,
836     ,1,
836     ,2,
840     ,1,
840     ,1,
840     ,1,
840     ,2,
844     ,1,
844     ,1,
844     ,1,
848     ,1,
848     ,2,
848     ,1,
852     ,1,
852     ,1,
852     ,1,
852     ,2,
856     ,1,
856     ,1,
856     ,1,
860     ,1,
860     ,2,
860     ,1,
864     ,1,
864     ,1,
864     ,1,
864     ,2,
868     ,1,
868     ,1,
868     ,1,
872     ,1,
872     ,2,
872     ,1,
876     ,1,
876     ,1,
876     ,1,
876     ,1,
880     ,2,
880     ,1,
880     ,1,
884     ,1,
884     ,1,
884     ,1,
884     ,2,
888     ,1,
888     ,1,
888     ,1,
892     ,1,
892     ,1,
892     ,1,
892     ,2,
896     ,1,
896     ,1,
896     ,1,
900     ,1,
900     ,1,
900     ,1,
900     ,2,
904     ,1,
904     ,1,
904     ,1,
908     ,1,
908     ,1,
908     ,1,
908     ,1,
912     ,2,
912     ,1,
912     ,1,
916     ,1,
916     ,1,
916     ,1,
916     ,1,
920     ,1,
920     ,2,
920     ,1,
924     ,1,
924     ,1,
924     ,1,
924     ,1,
928     ,1,
928     ,1,
928     ,1,
928     ,1,
932     ,2,
932     ,1,
932     ,1,
936     ,1,
936     ,1,
936     ,1,
936     ,1,
940     ,1,
940     ,1,
940     ,1,
940     ,1,
944     ,1,
944     ,2,
944     ,1,
948     ,1,
948     ,1,
948     ,1,
948     ,1,
952     ,1,
952     ,1,
952     ,1,
952     ,1,
956     ,1,
956     ,1,
956     ,1,
956     ,1,
960     ,2,
960     ,1,
960     ,1,
964     ,1,
964     ,1,
964     ,1,
964     ,1,
968     ,1,
968     ,1,
968     ,1,
968     ,1,
972     ,1,
972     ,1,
972     ,1,
972     ,1,
976     ,1,
976     ,1,
976     ,1,
976     ,1,
980     ,1,
980     ,2,
980     ,1,
984     ,1,
984     ,1,
984     ,1,
984     ,1,
988     ,1,
988     ,1,
988     ,1,
988     ,1,
992     ,1,
992     ,1,
992     ,1,
992     ,1,
996     ,1,
996     ,1,
996     ,1,
996     ,1,
1000    ,1,
1000    ,1,
1000    ,1,
1000    ,1,
1004    ,1,
1004    ,1,
1004    ,1,
1004    ,1,
1008    ,1,
1008    ,1,
1008    ,1,
1008    ,1,
1012    ,1,
1012    ,1,
1012    ,1,
1012    ,1,
1016    ,1,
1016    ,1,
1016    ,1,
1016    ,1,
1020    ,0,
1020    ,0,
1020    ,0,
1020    ,0
};

static vpsissGlbceConfig_t glbceCfg_ar0132 =
{
    TRUE,           /* ENABLE */
    0,              /* IR Strength */
    0,              /* blackLevel */
    65535,          /* White Level */
    12,             /* Intensity variance */
    7,              /* Spacial variance */
    6,              /* Bright Amplification Limit */
    6,              /* Dark Amplification Limit */
    VPS_ISS_GLBCE_DITHER_FOUR_BIT,
    64,             /* MAX Slope Limit */
    72,             /* MIN Slope Limit */
    {0,5377,10218,14600,18585,22224,25561,28631,31466,34092,36530,38801,40921,42904,44764,46511,48156,49706,51171,52557,53870,55116,56299,57425,58498,59520,60497,61429,62322,63176,63995,64781,65535}
};

static vpsissGlbcePerceptConfig_t glbceFwbPerCfg_ar0132 =
{
    FALSE,
    {0,4622,8653,11684,14195,16380,18335,20118,21766,23304,24751,26119,27422,28665,29857,31003,32108,33176,34209,35211,36185,37132,38055,38955,39834,40693,41533,42355,43161,43951,44727,45488,46236,46971,47694,48405,49106,49795,50475,51145,51805,52456,53099,53733,54360,54978,55589,56193,56789,57379,57963,58539,59110,59675,60234,60787,61335,61877,62414,62946,63473,63996,64513,65026,65535}
};

static vpsissGlbcePerceptConfig_t glbceRevPerCfg_ar0132 =
{
    FALSE,
    {0,228,455,683,910,1138,1369,1628,1912,2221,2556,2916,3304,3717,4158,4626,5122,5645,6197,6777,7386,8024,8691,9387,10113,10869,11654,12471,13317,14194,15103,16042,17012,18014,19048,20113,21210,22340,23501,24696,25922,27182,28475,29800,31159,32552,33977,35437,36930,38458,40019,41615,43245,44910,46609,48343,50112,51916,53755,55630,57539,59485,61466,63482,65535}
};

static vpsissGlbceWdrConfig_t glbceWdrCfg_ar0132 =
{
    TRUE,
    {
        0U   ,
        255U ,
        361U ,
        442U ,
        510U ,
        570U ,
        625U ,
        675U ,
        721U ,
        765U ,
        806U ,
        846U ,
        883U ,
        919U ,
        954U ,
        988U ,
        1020U,
        1051U,
        1082U,
        1112U,
        1140U,
        1169U,
        1196U,
        1223U,
        1249U,
        1275U,
        1300U,
        1325U,
        1349U,
        1373U,
        1397U,
        1420U,
        1442U,
        1465U,
        1487U,
        1509U,
        1530U,
        1551U,
        1572U,
        1592U,
        1613U,
        1633U,
        1653U,
        1672U,
        1691U,
        1711U,
        1729U,
        1748U,
        1767U,
        1785U,
        1803U,
        1821U,
        1839U,
        1856U,
        1874U,
        1891U,
        1908U,
        1925U,
        1942U,
        1959U,
        1975U,
        1992U,
        2008U,
        2024U,
        2040U,
        2056U,
        2072U,
        2087U,
        2103U,
        2118U,
        2133U,
        2149U,
        2164U,
        2179U,
        2194U,
        2208U,
        2223U,
        2238U,
        2252U,
        2266U,
        2281U,
        2295U,
        2309U,
        2323U,
        2337U,
        2351U,
        2365U,
        2378U,
        2392U,
        2406U,
        2419U,
        2433U,
        2446U,
        2459U,
        2472U,
        2485U,
        2498U,
        2511U,
        2524U,
        2537U,
        2550U,
        2563U,
        2575U,
        2588U,
        2600U,
        2613U,
        2625U,
        2638U,
        2650U,
        2662U,
        2674U,
        2687U,
        2699U,
        2711U,
        2723U,
        2735U,
        2746U,
        2758U,
        2770U,
        2782U,
        2793U,
        2805U,
        2817U,
        2828U,
        2840U,
        2851U,
        2862U,
        2874U,
        2885U,
        2896U,
        2907U,
        2919U,
        2930U,
        2941U,
        2952U,
        2963U,
        2974U,
        2985U,
        2996U,
        3006U,
        3017U,
        3028U,
        3039U,
        3049U,
        3060U,
        3071U,
        3081U,
        3092U,
        3102U,
        3113U,
        3123U,
        3133U,
        3144U,
        3154U,
        3164U,
        3175U,
        3185U,
        3195U,
        3205U,
        3215U,
        3226U,
        3236U,
        3246U,
        3256U,
        3266U,
        3276U,
        3285U,
        3295U,
        3305U,
        3315U,
        3325U,
        3335U,
        3344U,
        3354U,
        3364U,
        3373U,
        3383U,
        3393U,
        3402U,
        3412U,
        3421U,
        3431U,
        3440U,
        3450U,
        3459U,
        3468U,
        3478U,
        3487U,
        3496U,
        3506U,
        3515U,
        3524U,
        3533U,
        3543U,
        3552U,
        3561U,
        3570U,
        3579U,
        3588U,
        3597U,
        3606U,
        3615U,
        3624U,
        3633U,
        3642U,
        3651U,
        3660U,
        3669U,
        3678U,
        3686U,
        3695U,
        3704U,
        3713U,
        3722U,
        3730U,
        3739U,
        3748U,
        3756U,
        3765U,
        3774U,
        3782U,
        3791U,
        3799U,
        3808U,
        3816U,
        3825U,
        3833U,
        3842U,
        3850U,
        3859U,
        3867U,
        3876U,
        3884U,
        3892U,
        3901U,
        3909U,
        3917U,
        3926U,
        3934U,
        3942U,
        3950U,
        3959U,
        3967U,
        3975U,
        3983U,
        3991U,
        4000U,
        4008U,
        4016U,
        4024U,
        4032U,
        4040U,
        4048U,
        4056U,
        4064U,
        4072U,
        4080U
    }
};


static vpsissNsf3Config_t nsf3vCfg_ar0132 =
{
    FALSE,                           //bypass
    VPS_ISS_NSF3_OP_MODE_BAYER,
    TRUE,
    FALSE,      // chroma desaturation
    {
        /* hPos */
        {
            {0, 64, 256, 1024},
            {0, 64, 256, 1024},
            {0, 64, 256, 1024},
            {0, 64, 256, 1024}
        },
        /* vpos */
        {
            {2, 3, 6, 12},
            {2, 3, 6, 12},
            {2, 3, 6, 12},
            {2, 3, 6, 12}
        },
        /* Slope */
        {
            {32, 26, 18, 11},
            {32, 26, 18, 11},
            {32, 26, 18, 11},
            {32, 26, 18, 11}
        },
        64u,    16u,
        {
            32u, 64u, 80u
        }
    },
    {
        {128u, 128u, 128u},
        {128u, 128u, 128u},
        {128u, 128u, 128u},
        {128u, 128u, 128u}
    },
    {
        {128u, 128u, 128u},
        {128u, 128u, 128u},
        {128u, 128u, 128u},
        {128u, 128u, 128u}
    },
    {
        FALSE,  // ee_enable
        {
            {64u, 64u, 64u},
            {64u, 64u, 64u},
            {64u, 64u, 64u},
            {64u, 64u, 64u}
        },
        {
            {64u, 64u, 64u},
            {64u, 64u, 64u},
            {64u, 64u, 64u},
            {64u, 64u, 64u}
        },
        0u, 0u, 0u, 0u
    },
    {
        FALSE,  // shading gain enable
        {0u, 0u},
        {0u, 0u},
        {0u, 0u},
        0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u, 99u
    },
};

static vpsissIpipeDpcOtfConfig_t   dpcOtfCfg_ar0132 = {0};

static vpsissIsifBlackClampConfig_t isifBlkClmpCfg_ar0132 = {0};


Void VidSensor_SetIssIspConfig_ar0132(IssIspConfigurationParameters *pIspConfig)
{
    UInt32 outCnt, colorCnt;

    /* Override common settings for specific sensor */

    pIspConfig->aewbCfg = &aewbConfig_ar132;

    /* CFA Configuration */
    pIspConfig->cfaCfg = &cfaCfg_ar132;

    /* isifWbCfg */
    pIspConfig->isifWbCfg = &isifWbCfg_ar132;
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
    pIspConfig->isifWbCfg->offset = (UInt32)(0);

    pIspConfig->ipipeInputCfg = &ipipeInputCfg_ar132;

    ipipeInputCfg_ar132.dataPath = VPS_ISS_IPIPE_DATA_PATH_RAW_YUV422;
    ipipeInputCfg_ar132.procWin.cropStartX = 0;
    ipipeInputCfg_ar132.procWin.cropStartY = 2;
    ipipeInputCfg_ar132.procWin.cropWidth  = 1280;
    ipipeInputCfg_ar132.procWin.cropHeight
            =  720 - ipipeInputCfg_ar132.procWin.cropStartY;

    /* Override common settings for specific sensor */
    pIspConfig->rgb2rgb1Cfg = &rgb2rgb1Cfg_ar132;
    pIspConfig->rgb2rgb2Cfg = &rgb2rgb2Cfg_ar132;

    /* gammaCfg */
    pIspConfig->gammaCfg = &gammaCfg_ar132;
    pIspConfig->gammaCfg->enableRed   = TRUE;
    pIspConfig->gammaCfg->enableGreen = TRUE;
    pIspConfig->gammaCfg->enableBlue  = TRUE;
    pIspConfig->gammaCfg->lutRed      = gamma_lut_ar132;
    pIspConfig->gammaCfg->lutGreen    = gamma_lut_ar132;
    pIspConfig->gammaCfg->lutBlue     = gamma_lut_ar132;
    pIspConfig->gammaCfg->tbl         = VPS_ISS_IPIPE_GAMMA_TBL_RAM;
    pIspConfig->gammaCfg->tblSize     = VPS_ISS_IPIPE_GAMMA_TBL_SIZE_512;

    /* glbceCfg */
    pIspConfig->glbceCfg = &glbceCfg_ar0132;

    /* fwdPerCfg */
    pIspConfig->glbceFwdPerCfg = &glbceFwbPerCfg_ar0132;

    /* revPerCfg */
    pIspConfig->glbceRevPerCfg = &glbceRevPerCfg_ar0132;

    /* nsf3vCfg */
    pIspConfig->nsf3vCfg = &nsf3vCfg_ar0132;

    /* dpcOtfCfg */
    pIspConfig->dpcOtfCfg = &dpcOtfCfg_ar0132;
    pIspConfig->dpcOtfCfg->enableOtfDpc = TRUE;
    pIspConfig->dpcOtfCfg->method       = VPS_ISS_IPIPE_DPC_OTF_METHOD_1;
    pIspConfig->dpcOtfCfg->dThr       = 0x0u;
    pIspConfig->dpcOtfCfg->grThr      = 0x1u;
    pIspConfig->dpcOtfCfg->gbThr      = 0x0u;
    pIspConfig->dpcOtfCfg->bThr       = 800u;
    pIspConfig->dpcOtfCfg->rCor       = 0x0u;
    pIspConfig->dpcOtfCfg->grCor      = 0x1u;
    pIspConfig->dpcOtfCfg->gbCor      = 0x0u;
    pIspConfig->dpcOtfCfg->bCor       = 1023u;
    pIspConfig->dpcOtfCfg->shiftValue = 0x2u;

    isifBlkClmpCfg_ar0132.dcOffset = 0U;
    pIspConfig->isifBlkClampCfg = &isifBlkClmpCfg_ar0132;
}

Void VidSensor_SetIssIspGlbceConfig_ar0132(
    IssIspConfigurationParameters *pIspConfig)
{
    UTILS_assert(NULL != pIspConfig);

    /* WDR Config */
    pIspConfig->glbceWdrCfg = &glbceWdrCfg_ar0132;
}

Void VidSensor_SetAewbParams_ar0132(AlgorithmLink_IssAewbCreateParams *prms)
{
    AlgorithmLink_IssAewbAeDynamicParams *dynPrms = NULL;

    /* Enable Auto Exposure */
    prms->mode = ALGORITHMS_ISS_AEWB_MODE_AE;
    prms->dccCameraId = 0;
    prms->calbData = NULL;

    dynPrms = &prms->aeDynParams;

    dynPrms->targetBrightnessRange.min = 35;
    dynPrms->targetBrightnessRange.max = 45;
    dynPrms->targetBrightness = 40;
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
    dynPrms->sensorGainRange[1].max = 1000;
    dynPrms->ipipeGainRange[1].min = 512;
    dynPrms->ipipeGainRange[1].max = 512;

    dynPrms->exposureTimeRange[2].min = 16666;
    dynPrms->exposureTimeRange[2].max = 16666;
    dynPrms->apertureLevelRange[2].min = 1;
    dynPrms->apertureLevelRange[2].max = 1;
    dynPrms->sensorGainRange[2].min = 1000;
    dynPrms->sensorGainRange[2].max = 1000;
    dynPrms->ipipeGainRange[2].min = 512;
    dynPrms->ipipeGainRange[2].max = 512;

    dynPrms->exposureTimeRange[3].min = 16666;
    dynPrms->exposureTimeRange[3].max = 16666;
    dynPrms->apertureLevelRange[3].min = 1;
    dynPrms->apertureLevelRange[3].max = 1;
    dynPrms->sensorGainRange[3].min = 1000;
    dynPrms->sensorGainRange[3].max = 4000;
    dynPrms->ipipeGainRange[3].min = 512;
    dynPrms->ipipeGainRange[3].max = 512;

    dynPrms->numAeDynParams = 4;
    dynPrms->exposureTimeStepSize = 1;
    dynPrms->enableBlc = TRUE;
}
