/** ==================================================================
 *  @file   TI_aewb.h
 *
 *  @path    /proj/vsi/users/venu/DM812x/IPNetCam_rel_1_8/ti_tools/iss_02_bkup/packages/ti/psp/iss/alg/aewb/ti2a/awb/inc/
 *
 *  @desc   This  File contains.
 * ===================================================================
 *  Copyright (c) Texas Instruments Inc 2011, 2012
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * ===================================================================*/
#ifndef AEWB_TI_
#define AEWB_TI_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned int useWbCfg;
    unsigned int rGain, bGain, grGain, gbGain, dGain;
    unsigned int rOffset, bOffset, grOffset, gbOffset;

    unsigned int useColorTemp;
    unsigned int colorTemparature;

    unsigned int useAeCfg;
    unsigned int exposureTime, analogGain, digitalGain;
} ALG_Output;


void *ALG_aewbCreate(AlgorithmLink_IssAewbMode mode,
    int aewbNumWinH, int aewbNumWinV, int aewbNumPix,
    System_VideoDataFormat dataFormat, int numSteps,
    AlgorithmLink_IssAewbAeDynamicParams *aeDynamicPrms,
    AlgorithmLink_IssAewbAwbCalbData *calbData,
    System_LinkMemAllocInfo *pMemAllocInfo);
int ALG_aewbRun(void *hndl, void *h3aDataVirtAddr, ALG_Output *output);
int ALG_aewbDelete(void *hndl, System_LinkMemAllocInfo *pMemAllocInfo);
int ALG_aewbSetAeDynParams(void *hndl,
    AlgorithmLink_IssAewbAeDynamicParams *aeDynamicPrms);
int ALG_aewbSetAwbCalbData(void *hndl,
    AlgorithmLink_IssAewbAwbCalbData *calbData);


#ifdef __cplusplus
}
#endif

#endif
