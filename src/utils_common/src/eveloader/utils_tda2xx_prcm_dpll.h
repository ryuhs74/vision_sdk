/**
 *  \file     sbl_tda2xx_prcm_dpll.h
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

#ifndef UTILS_TDA2XX_PRCM_DPLL_H_
#define UTILS_TDA2XX_PRCM_DPLL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "pmhal_prcm.h"

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

/**
 * \brief Index for System Clock Frequency values.
 */
typedef enum sblTda2xxPrcmDpllOpp
{
    DPLL_OPP_LOW = 0U,
    /**< DPLL Operating Point Low */
    DPLL_OPP_MIN = DPLL_OPP_LOW,
    /**< DPLL Operating Point Min */
    DPLL_OPP_NOM = 1U,
    /**< DPLL Operating Point Nom */
    DPLL_OPP_OD = 2U,
    /**< DPLL Operating Point OD */
    DPLL_OPP_HIGH = 3U,
    /**< DPLL Operating Point HIGH */
    DPLL_OPP_MAX = (DPLL_OPP_HIGH + 1)
                   /**< Max system clock frequency index */
} sblTda2xxPrcmDpllOpp_t;

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void getDpllStructure(pmhalPrcmNodeId_t       dpllId,
                      pmhalPrcmSysClkVal_t    sysClk,
                      sblTda2xxPrcmDpllOpp_t  opp,
                      pmhalPrcmDpllConfig_t **dpllCfg);

#ifdef __cplusplus
}
#endif

#endif /* SBL_TDA2XX_PRCM_DPLL_H_ */

