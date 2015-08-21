/**
 *  \file     sbl_tda2xx_prcm_dpll.c
 *
 *  \brief    This file contains the structure for all dpll divider elements
 *            This also contains some related macros.
 *
 *  \copyright Copyright (C) 2014 Texas Instruments Incorporated -
 *             http://www.ti.com/
 */

/* ======================================================================
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
 * ==================================================================== */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include "pmhal_prcm.h"
#include "utils_eve_prcmlib_dpll.h"
#include "utils_tda2xx_prcm_dpll.h"
#include <src/utils_common/include/utils_eveloader.h>

/**
 *  \file     sbl_tda2xx_prcm_dpll.c
 *
 *  \brief    This file contains the structure for all dpll divider elements
 *            This also contains some related macros.
 *
 *  \copyright Copyright (C) 2014 Texas Instruments Incorporated -
 *             http://www.ti.com/
 */

/* ======================================================================
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
 * ==================================================================== */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*********************************************************************/
/* 20 MHZ */
/*********************************************************************/

pmhalPrcmPllPostDivValue_t dpllEvePostDivCfgOpp100_20[] =
{
    {PMHAL_PRCM_DPLL_POST_DIV_M2, 2} /* div_m2_clkcfg */
};

pmhalPrcmPllPostDivValue_t dpllEvePostDivCfgOppOd_20[] =
{
    {PMHAL_PRCM_DPLL_POST_DIV_M2, 2}, /* div_m2_clkcfg */
};

pmhalPrcmPllPostDivValue_t dpllEvePostDivCfgOppHigh_20[] =
{
    {PMHAL_PRCM_DPLL_POST_DIV_M2, 2}, /* div_m2_clkcfg */
};

pmhalPrcmDpllConfig_t      dpllEveCfgOpp100_20 =
{
    214,                       /* multiplier */
    3,                         /* divider */
    0,                         /* dutyCycleCorrector */
    dpllEvePostDivCfgOpp100_20,
    (sizeof (dpllEvePostDivCfgOpp100_20) / sizeof (pmhalPrcmPllPostDivValue_t))
};


pmhalPrcmDpllConfig_t      dpllEveCfgOppOd_20 =
{
    240,                      /* multiplier */
    3,                        /* divider */
    0,                        /* dutyCycleCorrector */
    dpllEvePostDivCfgOppOd_20,
    (sizeof (dpllEvePostDivCfgOppOd_20) / sizeof (pmhalPrcmPllPostDivValue_t))
};

pmhalPrcmDpllConfig_t      dpllEveCfgOppHigh_20 =
{
    260,                        /* multiplier */
    3,                          /* divider */
    0,                          /* dutyCycleCorrector */
    dpllEvePostDivCfgOppHigh_20,
    (sizeof (dpllEvePostDivCfgOppHigh_20) / sizeof (pmhalPrcmPllPostDivValue_t))
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void getDpllStructure(pmhalPrcmNodeId_t       dpllId,
                      pmhalPrcmSysClkVal_t    sysClk,
                      sblTda2xxPrcmDpllOpp_t  opp,
                      pmhalPrcmDpllConfig_t **dpllCfg)
{
    /* This function is ONLY for EVE and sys clock = 20Mhz and OPP_NOM */
    *dpllCfg = &dpllEveCfgOpp100_20;
}

#ifdef __cplusplus
}
#endif

