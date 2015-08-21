
#ifndef _ISS_AEWB1_ALG_PRIV_H
#define _ISS_AEWB1_ALG_PRIV_H

#include <include/link_api/system_const.h>
#include <include/link_api/issIspConfiguration.h>
#include <include/link_api/algorithmLink_issAewb.h>

#define TI_AEW_WIN_HZ_CNT               (64u)
#define TI_AEW_WIN_VT_CNT               (64u)

#define ALG_AEWB_OFF            0
#define ALG_AEWB_AE             1
#define ALG_AEWB_AWB            2
#define ALG_AEWB_AEWB           3

#define ALG_VIDEO_MODE_NTSC   0
#define ALG_VIDEO_MODE_PAL    1

#define ALG_AWB_MODE_INDOOR   0
#define ALG_AWB_MODE_OUTDOOR  1

#define ALG_AE_MODE_DAY      0
#define ALG_AE_MODE_NIGHT    1

#define IMAGE_TUNE_AWB_RGB_SIZE    1024

enum {
        VIDEO_NTSC=0,
        VIDEO_PAL
};

enum {
        AEW_DISABLE = 0,
        AEW_ENABLE
};

enum {
        AE_NIGHT = 0,
        AE_DAY
};

enum {
        BACKLIGHT_LOW = 0,
        BACKLIGHT_LOW2,
        BACKLIGHT_NORMAL,
        BACKLIGHT_NORMAL2,
        BACKLIGHT_HIGH,
        BACKLIGHT_HIGH2
};

enum {

        INDOOR = 0,
        OUTDOOR,
        AWB_AUTO
};


typedef enum {
    TI2A_WB_SCENE_MODE_AUTO                = 0,
    TI2A_WB_SCENE_MODE_D65                 = 1,
    TI2A_WB_SCENE_MODE_D55                 = 2,
    TI2A_WB_SCENE_MODE_FLORESCENT          = 3,
    TI2A_WB_SCENE_MODE_INCANDESCENT        = 4
}ti2aWBSceneMode;


typedef struct _aewDataEntry {
        unsigned short window_data[8][8];
        unsigned short unsat_block_ct[8];
}aewDataEntry;

typedef struct
{
    int AutoIris;
    int saturation;
    int sharpness;
    int brightness;
    int contrast;
    int blc;
    int AWBMode;
    int AEMode;
    int Env;
    int Binning;
    int FrameRate;
    int sensorMode;

    AlgorithmLink_IssAewbMode mode;

    unsigned char  *dcc_Default_Param;
    int  dcc_init_done;
    unsigned int dccSize;
    unsigned int AFValue;

    int *g_flickerMem;
    IAEWB_Rgb *rgbData;
    aewDataEntry *aew_data;
    int aewbFrames;
    int reduceShutter, saldre, env_50_60Hz;

    unsigned char weightingMatrix[TI_AEW_WIN_HZ_CNT * TI_AEW_WIN_VT_CNT];
    unsigned char weightingMatrixSpot[TI_AEW_WIN_HZ_CNT * TI_AEW_WIN_VT_CNT];
    unsigned char weightingMatrixCenter[TI_AEW_WIN_HZ_CNT * TI_AEW_WIN_VT_CNT];

    IAE_InArgs    AE_InArgs;
    IAE_OutArgs   AE_OutArgs;
    IAE_DynamicParams aeDynamicParams;

    IAWB_InArgs   AWB_InArgs;
    IAWB_OutArgs  AWB_OutArgs;

    IALG_Handle   handle_ae;
    IALG_Handle   handle_awb;

    IALG_MemRec   memTab_ae[4];
    IALG_MemRec   memTab_awb[4];

    unsigned char *weight;

    IAEWB_StatMat IAEWB_StatMatdata;

    awb_calc_data_t calbData;
    /**< Sensor Specific Calibration Data */

    /* Configurable Parameters */
    int update;
    int flicker_sel; /* <TBR: Uday>Sel between 50/60Hz flicker*/
    int flickerFreq; /*Select custum flicker*/
    int minExposure; /*Mininum Sensor exposure*/
    int maxExposure; /*Maximum Sensor exposure*/
    int stepSize;    /*step size for the exposure variation */
    int aGainMin;    /*Minimum analog Gain*/
    int aGainMax;    /*Maximum Analog gain */
    int dGainMin;    /*Minimum digital gain*/
    int dGainMax;    /*Maximum Digital gain */
    int targetBrightnessMin; /*Minimum target bright ness */
    int targetBrightnessMax; /*Maximum target bright ness */
    int targetBrightness;    /*target brightness to be achieved*/

    int day_night;
    int wbSceneMode;

    int accValue[4];

    System_VideoDataFormat dataFormat;
    int numSteps;

    int isAwbAllocated;
    int isAeAllocated;

    int aewbNumWinH;
    int aewbNumWinV;
    int aewbNumPix;

    int aeDynParamsChanged;
    int awbCalbDataChanged;
} ALG_aewbObj;

Void Alg_setDefaults();


#endif
