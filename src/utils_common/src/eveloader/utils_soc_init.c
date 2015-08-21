/**
 *  \file     sbl_tda2xx_soc_init.c
 *
 *  \brief    This file contains SoC init functions like configure
 *            dpll, clock domain wake-up & PRCM module enable.
 *
 *  \copyright Copyright (C) 2014 Texas Instruments Incorporated -
 *             http://www.ti.com/
 */

/* =============================================================================
 *   Copyright (C) 2014 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ========================================================================== */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <src/utils_common/include/utils_eveloader.h>
#include "sbl_tda2xx_prcmlib_dpll.h"
#include "pmhal_prcm.h"
#include "pm/pmhal/pmhal_mm.h"
#include "pm/pmhal/pmhal_pdm.h"
#include "pm/pmhal/pmhal_cm.h"
#include "pm/pmhal/pmhal_sd.h"
#include "pm/pm_types.h"
#include "soc.h"
#include "utils_tda2xx_platform.h"
#include "utils_tda2xx_soc_init.h"
#include "soc_defines.h"
#include "platform.h"
#include "sbl_tda2xx_prcm_dpll.h"

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                 Internal Function Definitions                              */
/* ========================================================================== */

/**
 * \brief       configure_dpll function program the ADPLLM & ADPLLJM for
 *              vayu SoC
 *
 * \param           None.
 *
 * \return      error status.If error has occured it returns a non zero value.
 *                  If no error has occured then return status will be zero.
 *
 **/
Int32 configure_dpll(void)
{
    Int32 retVal  = 0U;
    sblTda2xxPrcmDpllOpp_t opp;
    pmhalPrcmDpllConfig_t *dpllEveCfg  = 0U;
    pmhalPrcmSysClkVal_t   sysClkFreq = PMHAL_PRCM_SYSCLK_20_MHZ;

    PMHALCMSetCdClockMode(PMHAL_PRCM_CD_COREAON,
                          PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
                          PM_TIMEOUT_INFINITE);

    opp = DPLL_OPP_NOM;

    /*Configure prcm_dpll_eve*/

    getDpllStructure(PMHAL_PRCM_DPLL_EVE, sysClkFreq, opp,
                     &dpllEveCfg);
    retVal = PMHALCMDpllConfigure(PMHAL_PRCM_DPLL_EVE,
                                  dpllEveCfg,
                                  PM_TIMEOUT_INFINITE);
    if (retVal != PM_SUCCESS)
    {
        Vps_printf("UTILS : EVELOADER : config PMHAL_PRCM_DPLL_EVE - error  \n");
        return retVal;
    }

    return retVal;
}
