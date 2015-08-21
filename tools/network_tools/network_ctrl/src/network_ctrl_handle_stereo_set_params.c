/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"

/*NOTE: This struct is also defined in chains_common_stereo_defines.h
* Please make sure the two are matching, if any changes are made.
*/
typedef struct
{
    unsigned int numDisparities;
    unsigned int disparityStepSize;
    unsigned int disparitySearchDir;
    unsigned int disparitySupportWinWidth;
    unsigned int disparitySupportWinHeight;
    unsigned int leftRightCheckEna;
    unsigned int censusWinWidth;
    unsigned int censusWinHeight;
    unsigned int censusWinHorzStep;
    unsigned int censusWinVertStep;
    unsigned int postproc_colormap_index;
}Stereo_ConfigurableCreateParams;


void handleStereoCalibSetParams()
{
    Stereo_ConfigurableCreateParams stereoParams;
    unsigned int prmSize;

    stereoParams.numDisparities
                                    = atoi(gNetworkCtrl_obj.params[0]);
    stereoParams.disparityStepSize 
                                    = atoi(gNetworkCtrl_obj.params[1]);
    stereoParams.disparitySearchDir
                                    = atoi(gNetworkCtrl_obj.params[2]);
    stereoParams.disparitySupportWinWidth
                                    = atoi(gNetworkCtrl_obj.params[3]);
    stereoParams.disparitySupportWinHeight
                                    = atoi(gNetworkCtrl_obj.params[4]);
    stereoParams.leftRightCheckEna
                                    = atoi(gNetworkCtrl_obj.params[5]);
    stereoParams.censusWinWidth
                                    = atoi(gNetworkCtrl_obj.params[6]);
    stereoParams.censusWinHeight
                                    = atoi(gNetworkCtrl_obj.params[7]);
    stereoParams.censusWinHorzStep
                                    = atoi(gNetworkCtrl_obj.params[8]);
    stereoParams.censusWinVertStep
                                    = atoi(gNetworkCtrl_obj.params[9]);
    stereoParams.postproc_colormap_index
                                    = atoi(gNetworkCtrl_obj.params[10]);

    SendCommand(gNetworkCtrl_obj.command, &stereoParams,
                                    sizeof(stereoParams));
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);
}

