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

    if ( PM_SUCCESS == status )
    {
        switch(clkId)
        {
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

