/* =================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and
 *    conditionsstipulated in the agreement under which this program
 *     has been supplied.
 * =================================================================*/
/**
 *  @Component      sbl
 *
 *  @Filename       sbl_tda2xx_prcmlib_dpll.h
 *
 *  @Description    Contains vayu dpll structure declaration for different
 *                  sysclk
 *//*======================================================================== */

#ifndef SBL_TDA2XX_PRCMLIB_DPLL_H_
#define SBL_TDA2XX_PRCMLIB_DPLL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables Declarations                   */
/* ========================================================================== */
// BEGIN : AVATAR-PRCM-HAL-2012.03.30

// Modification : Mon 30 Apr 2012 17:28:43

// Component    : extern dpll configs defined in prcmlib_dpll.c
// Change       : Adding extern definitions for New PLLs in Vayu like DDR/GMAC.

// END : AVATAR-PRCM-HAL-2012.03.30

/******************************************************************************/
/********************    CLK CONFIGS SYSTEM CLOCK 38.4MHZ *********************/
/******************************************************************************/
extern struct dpll_config_t  dpll_core_cfg_opp100_384;
extern struct dpll_config_t  dpll_core_cfg_opp50_384;
extern struct dpll_config_t  dpll_core_cfg_oppaudio_384;
extern struct dpll_config_t  dpll_core_cfg_omap4430gls_384;
extern struct dpll_config_t  dpll_core_cfg_opp100_gls_384;
extern struct dpll_config_t  dpll_core_cfg_opp50_gls_384;

extern struct dpll_config_t  dpll_ddr_cfg_opp100_384;
extern struct dpll_config_t  dpll_ddr_cfg_opp50_384;

extern struct dpll_config_t  dpll_gmac_cfg_opp100_384;
extern struct dpll_config_t  dpll_gmac_cfg_opp50_384;

extern struct dpll_config_t  dpll_eve_cfg_opp100_384;
extern struct dpll_config_t  dpll_eve_cfg_opp50_384;

extern struct dpll_config_t  dpll_dsp_cfg_opp100_384;
extern struct dpll_config_t  dpll_dsp_cfg_opp50_384;

extern struct dpll_config_t  dpll_gpu_cfg_opp100_384;
extern struct dpll_config_t  dpll_gpu_cfg_opp50_384;

extern struct dpll_config_t  dpll_pcie_ref_cfg_opp100_384;

extern struct dpll_config_t  dpll_mpu_cfg_oppspeedbin_384;
extern struct dpll_config_t  dpll_mpu_cfg_opphigh_384;
extern struct dpll_config_t  dpll_mpu_cfg_opp100_384;
extern struct dpll_config_t  dpll_mpu_cfg_opp50_384;

extern struct dpll_config_t  dpll_per_cfg_opp100_384;
extern struct dpll_config_t  dpll_per_cfg_opp50_384;
extern struct dpll_config_t  dpll_per_cfg_oppaudio_384;
extern struct dpll_config_t  dpll_per_cfg_omap4430gls_384;

extern struct dpll_config_t  dpll_iva_cfg_opphigh_384;
extern struct dpll_config_t  dpll_iva_cfg_opp119_384;
extern struct dpll_config_t  dpll_iva_cfg_opp100_384;
extern struct dpll_config_t  dpll_iva_cfg_opp50_384;

extern struct dpll_config_t  dpll_abe_cfg_allopp_384;
extern struct dpll_config_t  dpll_abe_cfg_allopp_32k_384;

extern struct dpll_config_t  dpll_usb_cfg_allopp_384;

extern struct dpll_config_t  dpll_unipro_cfg_allopp_384;
extern struct dpll_config_t  dpll_unipro_cfg_hs_384;
extern struct dpll_config_t  dpll_unipro_cfg_hs_gls_384;
extern struct dpll_config_t *dpll_unipro_cfg_hs_384_ptr;
extern struct dpll_config_t  dpll_dsp_cfg_oppod_384;
extern struct dpll_config_t  dpll_eve_cfg_oppod_384;
extern struct dpll_config_t  dpll_iva_cfg_oppod_384;
extern struct dpll_config_t  dpll_gpu_cfg_oppod_384;
extern struct dpll_config_t  dpll_dsp_cfg_opphigh_384;
extern struct dpll_config_t  dpll_eve_cfg_opphigh_384;
extern struct dpll_config_t  dpll_iva_cfg_opphigh_384;
extern struct dpll_config_t  dpll_gpu_cfg_opphigh_384;

/******************************************************************************/
/********************    CLK CONFIGS SYSTEM CLOCK 12MHZ ***********************/
/******************************************************************************/

extern struct dpll_config_t  dpll_core_cfg_opp100_12;
extern struct dpll_config_t  dpll_core_cfg_opp50_12;
extern struct dpll_config_t  dpll_core_cfg_oppaudio_12;
extern struct dpll_config_t  dpll_core_cfg_omap4430gls_12;
extern struct dpll_config_t  dpll_core_cfg_opp100_gls_12;
extern struct dpll_config_t  dpll_core_cfg_opp50_gls_12;

extern struct dpll_config_t  dpll_ddr_cfg_opp100_12;
extern struct dpll_config_t  dpll_ddr_cfg_opp50_12;

extern struct dpll_config_t  dpll_gmac_cfg_opp100_12;
extern struct dpll_config_t  dpll_gmac_cfg_opp50_12;

extern struct dpll_config_t  dpll_eve_cfg_opp100_12;
extern struct dpll_config_t  dpll_eve_cfg_opp50_12;

extern struct dpll_config_t  dpll_dsp_cfg_opp100_12;
extern struct dpll_config_t  dpll_dsp_cfg_opp50_12;

extern struct dpll_config_t  dpll_gpu_cfg_opp100_12;
extern struct dpll_config_t  dpll_gpu_cfg_opp50_12;

extern struct dpll_config_t  dpll_pcie_ref_cfg_opp100_12;

extern struct dpll_config_t  dpll_mpu_cfg_oppspeedbin_12;
extern struct dpll_config_t  dpll_mpu_cfg_opphigh_12;
extern struct dpll_config_t  dpll_mpu_cfg_opp100_12;
extern struct dpll_config_t  dpll_mpu_cfg_opp50_12;

extern struct dpll_config_t  dpll_per_cfg_opp100_12;
extern struct dpll_config_t  dpll_per_cfg_opp50_12;
extern struct dpll_config_t  dpll_per_cfg_omap4430gls_12;
extern struct dpll_config_t  dpll_per_cfg_oppaudio_12;

extern struct dpll_config_t  dpll_iva_cfg_opphigh_12;
extern struct dpll_config_t  dpll_iva_cfg_opp119_12;
extern struct dpll_config_t  dpll_iva_cfg_opp100_12;
extern struct dpll_config_t  dpll_iva_cfg_opp50_12;

extern struct dpll_config_t  dpll_abe_cfg_allopp_12;
extern struct dpll_config_t  dpll_abe_cfg_allopp_32k_12;

extern struct dpll_config_t  dpll_usb_cfg_allopp_12;

extern struct dpll_config_t  dpll_unipro_cfg_allopp_12;
extern struct dpll_config_t  dpll_unipro_cfg_hs_12;
extern struct dpll_config_t  dpll_unipro_cfg_hs_gls_12;
extern struct dpll_config_t *dpll_unipro_cfg_hs_12_ptr;
extern struct dpll_config_t  dpll_dsp_cfg_oppod_12;
extern struct dpll_config_t  dpll_eve_cfg_oppod_12;
extern struct dpll_config_t  dpll_iva_cfg_oppod_12;
extern struct dpll_config_t  dpll_gpu_cfg_oppod_12;
extern struct dpll_config_t  dpll_dsp_cfg_opphigh_12;
extern struct dpll_config_t  dpll_eve_cfg_opphigh_12;
extern struct dpll_config_t  dpll_iva_cfg_opphigh_12;
extern struct dpll_config_t  dpll_gpu_cfg_opphigh_12;

/******************************************************************************/
/******************************************************************************/
/********************    CLK CONFIGS SYSTEM CLOCK 19.2MHZ
 ************************/
/******************************************************************************/

extern struct dpll_config_t  dpll_core_cfg_opp100_192;
extern struct dpll_config_t  dpll_core_cfg_opp50_192;
extern struct dpll_config_t  dpll_core_cfg_oppaudio_192;
extern struct dpll_config_t  dpll_core_cfg_opp100_gls_192;
extern struct dpll_config_t  dpll_core_cfg_opp50_gls_192;

extern struct dpll_config_t  dpll_ddr_cfg_opp100_192;
extern struct dpll_config_t  dpll_ddr_cfg_opp50_192;

extern struct dpll_config_t  dpll_gmac_cfg_opp100_192;
extern struct dpll_config_t  dpll_gmac_cfg_opp50_192;

extern struct dpll_config_t  dpll_eve_cfg_opp100_192;
extern struct dpll_config_t  dpll_eve_cfg_opp50_192;

extern struct dpll_config_t  dpll_dsp_cfg_opp100_192;
extern struct dpll_config_t  dpll_dsp_cfg_opp50_192;

extern struct dpll_config_t  dpll_gpu_cfg_opp100_192;
extern struct dpll_config_t  dpll_gpu_cfg_opp50_192;

extern struct dpll_config_t  dpll_pcie_ref_cfg_opp100_192;

extern struct dpll_config_t  dpll_mpu_cfg_oppspeedbin_192;
extern struct dpll_config_t  dpll_mpu_cfg_opphigh_192;
extern struct dpll_config_t  dpll_mpu_cfg_opp100_192;
extern struct dpll_config_t  dpll_mpu_cfg_opp50_192;

extern struct dpll_config_t  dpll_per_cfg_opp100_192;
extern struct dpll_config_t  dpll_per_cfg_opp50_192;
extern struct dpll_config_t  dpll_per_cfg_oppaudio_192;

extern struct dpll_config_t  dpll_iva_cfg_opphigh_192;
extern struct dpll_config_t  dpll_iva_cfg_opp119_192;
extern struct dpll_config_t  dpll_iva_cfg_opp100_192;
extern struct dpll_config_t  dpll_iva_cfg_opp50_192;

extern struct dpll_config_t  dpll_abe_cfg_allopp_192;
extern struct dpll_config_t  dpll_abe_cfg_allopp_32k_192;
extern struct dpll_config_t  dpll_usb_cfg_allopp_192;
extern struct dpll_config_t  dpll_unipro_cfg_allopp_192;
extern struct dpll_config_t  dpll_unipro_cfg_hs_192;

extern struct dpll_config_t  dpll_unipro_cfg_hs_gls_192;
extern struct dpll_config_t *dpll_unipro_cfg_hs_192_ptr;
extern struct dpll_config_t  dpll_dsp_cfg_oppod_192;
extern struct dpll_config_t  dpll_eve_cfg_oppod_192;
extern struct dpll_config_t  dpll_iva_cfg_oppod_192;
extern struct dpll_config_t  dpll_gpu_cfg_oppod_192;
extern struct dpll_config_t  dpll_dsp_cfg_opphigh_192;
extern struct dpll_config_t  dpll_eve_cfg_opphigh_192;
extern struct dpll_config_t  dpll_iva_cfg_opphigh_192;
extern struct dpll_config_t  dpll_gpu_cfg_opphigh_192;

/********************************************************************************/
/********************    CLK CONFIGS SYSTEM CLOCK 26MHZ
 **************************/
/********************************************************************************/

extern struct dpll_config_t  dpll_core_cfg_opp100_26;
extern struct dpll_config_t  dpll_core_cfg_opp50_26;
extern struct dpll_config_t  dpll_core_cfg_oppaudio_26;

extern struct dpll_config_t  dpll_ddr_cfg_opp100_26;
extern struct dpll_config_t  dpll_ddr_cfg_opp50_26;

extern struct dpll_config_t  dpll_ddr2_cfg_opp100_26;
extern struct dpll_config_t  dpll_ddr2_cfg_opp50_26;

extern struct dpll_config_t  dpll_gmac_cfg_opp100_26;
extern struct dpll_config_t  dpll_gmac_cfg_opp50_26;

extern struct dpll_config_t  dpll_eve_cfg_opp100_26;
extern struct dpll_config_t  dpll_eve_cfg_opp50_26;

extern struct dpll_config_t  dpll_dsp_cfg_opp100_26;
extern struct dpll_config_t  dpll_dsp_cfg_opp50_26;

extern struct dpll_config_t  dpll_gpu_cfg_opp100_26;
extern struct dpll_config_t  dpll_gpu_cfg_opp50_26;

extern struct dpll_config_t  dpll_pcie_ref_cfg_opp100_26;

extern struct dpll_config_t  dpll_mpu_cfg_oppspeedbin_26;
extern struct dpll_config_t  dpll_mpu_cfg_opphigh_26;
extern struct dpll_config_t  dpll_mpu_cfg_opp100_26;
extern struct dpll_config_t  dpll_mpu_cfg_opp50_26;

extern struct dpll_config_t  dpll_per_cfg_opp100_26;
extern struct dpll_config_t  dpll_per_cfg_opp50_26;
extern struct dpll_config_t  dpll_per_cfg_oppaudio_26;

extern struct dpll_config_t  dpll_iva_cfg_opphigh_26;
extern struct dpll_config_t  dpll_iva_cfg_opp119_26;
extern struct dpll_config_t  dpll_iva_cfg_opp100_26;
extern struct dpll_config_t  dpll_iva_cfg_opp50_26;

extern struct dpll_config_t  dpll_abe_cfg_allopp_26;
extern struct dpll_config_t  dpll_abe_cfg_allopp_32k_26;

extern struct dpll_config_t  dpll_usb_cfg_allopp_26;

extern struct dpll_config_t  dpll_unipro_cfg_allopp_26;
extern struct dpll_config_t  dpll_unipro_cfg_hs_26;
extern struct dpll_config_t  dpll_unipro_cfg_hs_gls_26;
extern struct dpll_config_t *dpll_unipro_cfg_hs_26_ptr;
extern struct dpll_config_t  dpll_dsp_cfg_oppod_26;
extern struct dpll_config_t  dpll_eve_cfg_oppod_26;
extern struct dpll_config_t  dpll_iva_cfg_oppod_26;
extern struct dpll_config_t  dpll_gpu_cfg_oppod_26;
extern struct dpll_config_t  dpll_dsp_cfg_opphigh_26;
extern struct dpll_config_t  dpll_eve_cfg_opphigh_26;
extern struct dpll_config_t  dpll_iva_cfg_opphigh_26;
extern struct dpll_config_t  dpll_gpu_cfg_opphigh_26;

/********************************************************************************/
/********************    CLK CONFIGS SYSTEM CLOCK 20MHZ
 **************************/
/********************************************************************************/
extern struct dpll_config_t  dpll_mpu_cfg_oppspeedbin_20;
extern struct dpll_config_t  dpll_mpu_cfg_opphigh_20;
extern struct dpll_config_t  dpll_mpu_cfg_opp100_20;
extern struct dpll_config_t  dpll_mpu_cfg_opp50_20;
extern struct dpll_config_t  dpll_core_cfg_opp100_20;
extern struct dpll_config_t  dpll_core_cfg_opp50_20;
extern struct dpll_config_t  dpll_per_cfg_opp100_20;
extern struct dpll_config_t  dpll_per_cfg_opp50_20;
extern struct dpll_config_t  dpll_dsp_cfg_opp100_20;
extern struct dpll_config_t  dpll_dsp_cfg_opp50_20;
extern struct dpll_config_t  dpll_eve_cfg_opp100_20;
extern struct dpll_config_t  dpll_eve_cfg_opp50_20;
extern struct dpll_config_t  dpll_iva_cfg_opp100_20;
extern struct dpll_config_t  dpll_iva_cfg_opp50_20;
extern struct dpll_config_t  dpll_gpu_cfg_opp100_20;
extern struct dpll_config_t  dpll_gpu_cfg_opp50_20;
extern struct dpll_config_t  dpll_ddr_cfg_opp100_20;
extern struct dpll_config_t  dpll_ddr_cfg_opp50_20;
extern struct dpll_config_t  dpll_gmac_cfg_opp100_20;
extern struct dpll_config_t  dpll_gmac_cfg_opp50_20;
extern struct dpll_config_t  dpll_abe_cfg_allopp_20;
extern struct dpll_config_t  dpll_usb_cfg_allopp_20;
extern struct dpll_config_t  dpll_pcie_ref_cfg_opp100_20;
extern struct dpll_config_t  dpll_video1_cfg_opp100_20;
extern struct dpll_config_t  dpll_video2_cfg_opp100_20;
extern struct dpll_config_t  dpll_hdmi_cfg_opp100_20;
extern struct dpll_config_t  dpll_dsp_cfg_oppod_20;
extern struct dpll_config_t  dpll_eve_cfg_oppod_20;
extern struct dpll_config_t  dpll_iva_cfg_oppod_20;
extern struct dpll_config_t  dpll_gpu_cfg_oppod_20;
extern struct dpll_config_t  dpll_dsp_cfg_opphigh_20;
extern struct dpll_config_t  dpll_eve_cfg_opphigh_20;
extern struct dpll_config_t  dpll_iva_cfg_opphigh_20;
extern struct dpll_config_t  dpll_gpu_cfg_opphigh_20;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif /*SBL_TDA2XX_PRCMLIB_DPLL_H_*/
