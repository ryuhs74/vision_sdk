

#ifndef _VIDEO_SENSOR_PRIV_H_
#define _VIDEO_SENSOR_PRIV_H_

#define VID_SENSOR_AWB_AVG_BUF_LENGTH    (12)
#define VID_SENSOR_NUM_RGB2RGB_MATRIXES  (10)

/* Currently this layer uses only 0th index of the AEWB output parameters,
   Need to change this layer, if multiple planes is required to be supported */
#define VID_SENSOR_AEWB_PLANE_ID         (0U)

typedef struct {
    Int32 steadySet;
    Int32 prevIdx;
    Int32 curSetIdx;
    Int32 curSetCnt;

    UInt32 historyIdx[VID_SENSOR_AWB_AVG_BUF_LENGTH];
    UInt32 awbCount;
    UInt32 prevRgb2RgbIdx;
} VidSensor_AewbPrivParams;

typedef struct {
    UInt32 colorTemp;
    vpsissIpipeRgb2RgbConfig_t rgb2rgb1;
    vpsissIpipeRgb2RgbConfig_t rgb2rgb2;
} VidSensor_Rgb2RgbParams;

Int32 VidSensor_Switch_rgb2rgb_matrixes(
    IssAewbAlgOutParams *pAlgOut,
    VidSensor_AewbPrivParams *prms,
    VidSensor_Rgb2RgbParams m[],
    UInt32 reset);

Void VidSensor_SetIssIspConfig_ar0132(IssIspConfigurationParameters *pIspConfig);
Void VidSensor_SetIssIspConfig_ar0140(IssIspConfigurationParameters *pIspConfig);
Void VidSensor_SetIssIspConfig_ov10640(IssIspConfigurationParameters *pIspConfig);
Void VidSensor_SetIssIspConfig_imx224(IssIspConfigurationParameters *pIspConfig);

Void VidSensor_SetAewbParams_ar0132(AlgorithmLink_IssAewbCreateParams *prms);
Void VidSensor_SetAewbParams_ar0140(AlgorithmLink_IssAewbCreateParams *prms,
    UInt32 isOnePassWdr);
Void VidSensor_SetAewbParams_ov10640(AlgorithmLink_IssAewbCreateParams *prms);
Void VidSensor_SetAewbParams_imx224(AlgorithmLink_IssAewbCreateParams *prms);

Void VidSensor_SetIssIspGlbceConfig_ar0140(
    IssIspConfigurationParameters *pIspConfig);
Void VidSensor_SetIssIspGlbceConfig_ar0132(
    IssIspConfigurationParameters *pIspConfig);

#endif /* _VIDEO_SENSOR_PRIV_H_ */
