/* ======================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
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
/**
 *   Component:         SBL
 *
 *   Filename:          sbl_tda2xx_soc_init.h
 *
 *   Description:       This file contains SoC init functions like configure
 *                      dpll, clock domain wake-up & PRCM module enable.
 */
#ifndef SBL_TDA2xx_SOC_INIT_H_
#define SBL_TDA2xx_SOC_INIT_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "stdint.h"
#include "hw_types.h"
#include "soc_defines.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */
enum dss_dpll
{
    dss_dpll_video1,
    dss_dpll_video2,
    dss_dpll_hdmi
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
UInt32 get_current_opp(void);
Int32 configure_dpll(void);
Int32 configure_clock_domains(void);
Int32 enable_vip_dss(void);
Int32 program_dss_video_pll(void);
Int32 enable_ocmc(void);
Int32 enable_serial_per(void);
Int32 enable_core(void);
Int32 enable_mem(void);
Int32 enable_interconnect(void);
Int32 enable_iva(void);
Int32 enable_ipc(void);
Int32 enable_timer(void);
Int32 enable_per(void);
Int32 SBL_UART_PRCM(UART_INST_t);
Int32 ti_tda2xx_evm_configure_pad();

#ifdef __cplusplus
}
#endif

#endif /*SBL_TDA2xx_SOC_INIT_H_*/

