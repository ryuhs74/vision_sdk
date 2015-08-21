/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"
#include <include/link_api/algorithmLink_stereoPostProcess.h>

Void NetworkCtrl_cmdHandlerStereoSetDynamicParams(char *cmd, UInt32 prmSize)
{
    UInt32 linkId; 
    AlgorithmLink_StereoPostProcessDynamicParams stereoParams;
    AlgorithmLink_StereoPostProcessControlParams stereoCtlParams;
    Int32 status;

    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(AlgorithmLink_StereoPostProcessDynamicParams))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8*)(&stereoParams), sizeof(AlgorithmLink_StereoPostProcessDynamicParams));
        linkId = DSP1_LINK (SYSTEM_LINK_ID_ALG_0);
        
        memcpy(&stereoCtlParams.stereoParams, &stereoParams, sizeof(AlgorithmLink_StereoPostProcessDynamicParams));
        stereoCtlParams.baseClassControl.controlCmd= STEREO_POSTPROCESS_LINK_CMD_SET_DYNAMIC_PARAMS;
        stereoCtlParams.baseClassControl.size= sizeof(stereoCtlParams);
                
        status= System_linkControl(
            linkId,
            ALGORITHM_LINK_CMD_CONFIG,
            &stereoCtlParams,
            sizeof(stereoCtlParams),
            TRUE
        );
        
        UTILS_assert(0 == status);
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
    
    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);
}
