/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "network_ctrl_priv.h"
#include <examples/tda2xx/src/usecases/common/chains_common_stereo_defines.h>

Void NetworkCtrl_cmdHandlerStereoSetParams(char *cmd, UInt32 prmSize)
{
    UInt32 linkId; 
    Stereo_ConfigurableCreateParams stereoParams;

    /* alloc tmp buffer for parameters */
    if(prmSize == sizeof(Stereo_ConfigurableCreateParams))
    {
        /* read parameters */
        NetworkCtrl_readParams((UInt8*)(&stereoParams), sizeof(Stereo_ConfigurableCreateParams));
        linkId = SYSTEM_LINK_ID_IPU1_0;
        System_linkControl(
            linkId,
            SYSTEM_LINK_CMD_STEREO_SET_PARAM,
            &stereoParams,
            sizeof(stereoParams),
            TRUE
        );
    }
    else
    {
        Vps_printf(" NETWORK_CTRL: %s: Insufficient parameters (%d bytes) specified !!!\n", cmd, prmSize);
    }
    
    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);
}
