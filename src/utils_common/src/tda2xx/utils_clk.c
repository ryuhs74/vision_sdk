/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_clk.c
 *
 * \brief Utility functions implementation for gettin DPLL clock settings
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <src/utils_common/include/utils.h>
#include <pm/pmlib/pmlib_clkrate.h>
#include <pm/pmlib/pmlib_boardconfig.h>

#define UTILS_SYS_CLK1           (20*1000*1000)

/* Various register address definitions */
#define IPU_CM_CORE_AON              (0x4a005500)
#define CKGEN_CM_CORE_AON            (0x4a005100)

#define CM_CLKMODE_DPLL_CORE         (CKGEN_CM_CORE_AON + 0x020)
#define CM_CLKMODE_DPLL_MPU          (CKGEN_CM_CORE_AON + 0x060)
#define CM_CLKMODE_DPLL_ABE          (CKGEN_CM_CORE_AON + 0x0E0)
#define CM_CLKMODE_DPLL_DSP          (CKGEN_CM_CORE_AON + 0x134)
#define CM_CLKMODE_DPLL_EVE          (CKGEN_CM_CORE_AON + 0x184)

#define CM_IPU1_IPU1_CLKCTRL         (IPU_CM_CORE_AON   + 0x020)

#define CM_CLKSEL_DPLL(x)       *(volatile UInt32*)(x + 0x0C)
#define CM_DIV_M2_DPLL(x)       *(volatile UInt32*)(x + 0x10)
#define CM_DIV_H22_DPLL(x)      *(volatile UInt32*)(x + 0x34)

/**
 *******************************************************************************
 * \brief DPLL clkout info
 *******************************************************************************
 */
typedef struct {

    UInt32 f_dpll;
    /**< DPLL clock output value */

    UInt32 clkout_M2;
    /**< M2 clock output value */

    UInt32 clkoutX2_M2;
    /**< X2_M2 clock output value */

    UInt32 isEnableClkout_M2;
    /**< TRUE, M2 clk out is enabled, else disabled */

} Utils_DpllClkOutInfo;

/**
 *******************************************************************************
 * \brief Get DPLL Clock out info
 *******************************************************************************
 */
Void Utils_getDpllClkOutInfo(UInt32 base_address, Utils_DpllClkOutInfo *pPrm)
{
    UInt32 temp, dpll_div_n, dpll_div_m;
    UInt32 dpll_div_m2;

    temp = CM_CLKSEL_DPLL(base_address);
    dpll_div_n = temp & 0x7F;
    dpll_div_m = (temp >> 8) & 0x7FF;
    pPrm->f_dpll = (UTILS_SYS_CLK1/(dpll_div_n+1))*dpll_div_m;

    temp = CM_DIV_M2_DPLL(base_address);
    dpll_div_m2 = temp & 0xF;
    pPrm->isEnableClkout_M2 = (temp >> 9) & 0x1;

    pPrm->f_dpll = (2*UTILS_SYS_CLK1/(dpll_div_n+1))*dpll_div_m;
    pPrm->clkout_M2 = pPrm->f_dpll/(dpll_div_m2*2);
    pPrm->clkoutX2_M2 = pPrm->f_dpll/dpll_div_m2;
}

/**
 *******************************************************************************
 * \brief Get DPLL Clock out info for A15 DPLL
 *******************************************************************************
 */
Void Utils_getDpllClkOutInfoA15(UInt32 base_address,
                Utils_DpllClkOutInfo *pPrm)
{
    UInt32 temp,dpll_div_n,dpll_div_m,dpll_div_m2,dcc_en;

    temp = CM_CLKSEL_DPLL(base_address);
    dpll_div_n = temp & 0x7F;
    dpll_div_m = (temp >> 8) & 0x7FF;
    dcc_en = (temp >> 22 ) & 0x1;

    temp = CM_DIV_M2_DPLL(base_address);
    dpll_div_m2 = temp & 0xF;
    pPrm->isEnableClkout_M2 = (temp >> 9) & 0x1;

    if(dcc_en == 1)
	{
	    pPrm->f_dpll = (UTILS_SYS_CLK1/(dpll_div_n+1))*dpll_div_m;
	    pPrm->clkout_M2 = pPrm->f_dpll/dpll_div_m2;
	}
	else
	{
	    pPrm->f_dpll = (2*UTILS_SYS_CLK1/(dpll_div_n+1))*dpll_div_m;
	    pPrm->clkout_M2 = pPrm->f_dpll/(dpll_div_m2*2);
	}
}

/**
 *******************************************************************************
 * \brief Get DPLL Clock out info for ABE DPLL
 *******************************************************************************
 */
Void Utils_getDpllClkOutInfoAbe(UInt32 base_address,
                Utils_DpllClkOutInfo *pPrm)
{
    UInt32 temp,dpll_div_n,dpll_div_m;
    UInt32 dpll_div_m2;

    temp = CM_CLKSEL_DPLL(base_address);
    dpll_div_n = temp & 0x7F;
    dpll_div_m = (temp >> 8) & 0x7FF;
    pPrm->f_dpll = (UTILS_SYS_CLK1/(dpll_div_n+1))*dpll_div_m;

    temp = CM_DIV_M2_DPLL(base_address);
    dpll_div_m2 = temp & 0xF;
    pPrm->isEnableClkout_M2 = (temp >> 9) & 0x1;

    pPrm->f_dpll = (2*UTILS_SYS_CLK1/(dpll_div_n+1))*dpll_div_m;
    pPrm->clkout_M2 = pPrm->f_dpll/(dpll_div_m2*2);
    pPrm->clkoutX2_M2 = pPrm->f_dpll/dpll_div_m2;
}

/**
 *******************************************************************************
 * \brief Get IPU Clk Hz
 *******************************************************************************
 */
UInt32 Utils_getDpllClkOutInfoIpu()
{
    Utils_DpllClkOutInfo coreDpll;
    Utils_DpllClkOutInfo abeDpll;

    UInt32 temp,ipu_clksel,divhs, clkHz;
    UInt32 isEnableClkout_M2;

    clkHz = 0;

    Utils_getDpllClkOutInfo(CM_CLKMODE_DPLL_CORE, &coreDpll);
    Utils_getDpllClkOutInfoAbe(CM_CLKMODE_DPLL_ABE, &abeDpll);

    temp = *(volatile UInt32*)(CM_IPU1_IPU1_CLKCTRL);
    ipu_clksel = (temp >> 24) & 0x1;

    if(ipu_clksel == 0)
    {
        if(abeDpll.isEnableClkout_M2)
            clkHz = abeDpll.clkoutX2_M2;
    }
    else
    {
        temp = CM_DIV_H22_DPLL(CM_CLKMODE_DPLL_CORE);
        divhs = temp & 0x3F;
        isEnableClkout_M2 = (temp >> 9) & 0x1;
        if(isEnableClkout_M2)
            clkHz = coreDpll.f_dpll/(divhs*2);
    }

    return clkHz;
}

UInt32 Utils_getClkHz(Utils_ClkId clkId)
{
    UInt32 clkHz = 0;
    pmErrCode_t status = PM_FAIL;

    /**
     * \brief Holds the root clock frequencies specific for a given platform
     */
    UInt32 *rootClkFreqList;

    /**
     * \brief Holds the voltage domains information whose rails are shared
     */
    pmlibBoardVdRailShare_t *vdRailShareList;

    rootClkFreqList = PMLIBBoardConfigGetRootClks();
    vdRailShareList = PMLIBBoardConfigGetVdRailShareInfo();
    status = PMLIBClkRateInit(rootClkFreqList,vdRailShareList);

    if (PM_SUCCESS == status)
    {
        switch(clkId)
        {
            case UTILS_CLK_ID_EVE:
                status = PMLIBClkRateGet(PMHAL_PRCM_MOD_EVE1, PMHAL_PRCM_CLK_GENERIC, &clkHz);
                break;
            case UTILS_CLK_ID_DSP:
                status = PMLIBClkRateGet(PMHAL_PRCM_MOD_DSP1, PMHAL_PRCM_CLK_GENERIC, &clkHz);
                break;
            case UTILS_CLK_ID_IPU:
                status = PMLIBClkRateGet(PMHAL_PRCM_MOD_IPU1, PMHAL_PRCM_CLK_GENERIC, &clkHz);
                break;
            case UTILS_CLK_ID_A15:
                status = PMLIBClkRateGet(PMHAL_PRCM_MOD_MPU, PMHAL_PRCM_CLK_GENERIC, &clkHz);
                break;
        }
    }
    if (PM_SUCCESS != status)
    {
        clkHz = 0U;
    }


    return clkHz;
}

