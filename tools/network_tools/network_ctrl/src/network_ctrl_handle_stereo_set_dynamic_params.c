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
    unsigned int postproc_cost_max_threshold;
    unsigned int postproc_conf_min_thrseshold;
    unsigned int postproc_texture_lumalothresh;
    unsigned int postproc_texture_lumahithresh;
    unsigned int postproc_texture_threshold;
    unsigned int postproc_lrmaxdiff_threshold;
    unsigned int postproc_maxdisp_dissimilarity;
    unsigned int postproc_minconf_nseg_threshold;
}Stereo_ConfigurableDynamicParams;


void handleStereoSetDynamicParams()
{
    Stereo_ConfigurableDynamicParams stereoParams;
    unsigned int prmSize;

    stereoParams.postproc_cost_max_threshold
                                    = atoi(gNetworkCtrl_obj.params[0]);
    stereoParams.postproc_conf_min_thrseshold 
                                    = atoi(gNetworkCtrl_obj.params[1]);;
    stereoParams.postproc_texture_lumalothresh
                                    = atoi(gNetworkCtrl_obj.params[2]);
    stereoParams.postproc_texture_lumahithresh
                                    = atoi(gNetworkCtrl_obj.params[3]);
    stereoParams.postproc_texture_threshold
                                    = atoi(gNetworkCtrl_obj.params[4]);
    stereoParams.postproc_lrmaxdiff_threshold
                                    = atoi(gNetworkCtrl_obj.params[5]);
    stereoParams.postproc_maxdisp_dissimilarity
                                    = atoi(gNetworkCtrl_obj.params[6]);
    stereoParams.postproc_minconf_nseg_threshold
                                    = atoi(gNetworkCtrl_obj.params[7]);
    SendCommand(gNetworkCtrl_obj.command, &stereoParams,
                                    sizeof(stereoParams));
    RecvResponse(gNetworkCtrl_obj.command, &prmSize);
}

