/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "network_ctrl_priv.h"
#include <stdint.h>
#include "stdio.h"
#include "hw_types.h"
#include "pmhal_prcm.h"
#include "pm/pm_types.h"
#include "pm/pmhal/pmhal_rm.h"

Void NetworkCtrl_cmdHandlerSysReset(char *cmd, UInt32 prmSize)
{

    Vps_printf(" NETWORK_CTRL: Command Received %s: ", cmd);
    Vps_printf(" NETWORK_CTRL: System Getting RESET Now !");

    /* send response */
    NetworkCtrl_writeParams(NULL, 0, 0);

    PMHALResetAssertGlobal(PMHAL_PRCM_GLB_RG_GLOBAL_COLD_RST);
}

