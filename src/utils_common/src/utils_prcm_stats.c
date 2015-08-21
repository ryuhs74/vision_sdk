/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file utils_prcm_stats.c
 *
 * \brief  APIs to execute PRCM Stats , Dpll states, Junction temperature,
 *         Voltage.
 *
 * \version 0.0 (May 2015) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_temperature.h>
#include "soc_defines.h"
#include "pmhal_bgap.h"
#include "pmhal_prcm.h"
#include "hw_pmhal_data_names.h"
#include "pm/pmhal/pmhal_vm.h"
#include "pm/pmhal/pmhal_cm.h"
#include "pm/pmhal/pmhal_prcm_stats.h"
#include "pm/pmlib/pmlib_clkrate.h"
#include "pmlib_boardconfig.h"
#include "pmlib_sysconfig.h"
#include "pm/pmhal/pmhal_pmic.h"
#include "platform.h"
#include "pmhal_tps65917.h"
#include "pmhal_tps659039.h"
#include "pmhal_l8731.h"

/**
 *******************************************************************************
 * \brief Macro
 *******************************************************************************
 */

#define RD_MEM_32(addr)          *(unsigned int*)(addr)

/**
 *******************************************************************************
 * \brief Structure and Enum
 *******************************************************************************
 */
/**
 * \brief Structure contains ModuleId list clocklId list
 *
 */
typedef struct
{
    char               *moduleCharId;
    /**< module Id for debugging */
    pmhalPrcmModuleId_t modId;
    /**< module Id */
    char               *clkCharId;
    /**< clock id for debugging*/
    pmhalPrcmClockId_t  clkId;
    /**< clock Id */
}cpuClockList_t;


/**
 * \brief Module Power State names
 */
const char *moduleState_names[] =
{
    "DISABLED",
    "AUTO_CLOCKGATED",
    "ENABLED"
};

/**
 * \brief Module SIDLE State names
 */
const char *moduleSIdleState_names[] =
{
    "FUNCTIONAL",
    "IN_TRANSITION",
    "INTERFACE_IDLE",
    "DISABLED",
    "DETERMINED BY CLK STATE"
};

/**
 * \brief clock Activity State names
 */
const char *clockActivityState_names[] =
{
    "CLOCK_GATED",
    "CLOCK_ENABLED"
};

/**
 * \brief Power Domain Activity State names
 */
const char *powerDoaminState_names[] =
{
    "OFF",
    "RETENTION",
    "ON_INACTIVE",
    "ON_ACTIVE",
};

/**
 * \brief Structure to define the clocks that one wants to set. The frequency is
 *        given in Hz. Generic clock is used when ever the user does not want to
 *        set any specific clock.
 *        One can look at the generic clocks list to know which module's clock
 *        is defined as generic. To program a specific clock refer
 *        pmhalPrcmClockId_t enum in starterware.
 */
static cpuClockList_t           cpuClockList[] =
{
#ifdef PROC_A15_0_INCLUDE
    {
        "MPU",
        PMHAL_PRCM_MOD_MPU,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC
    },
#endif
#ifdef PROC_IPU1_0_INCLUDE
    {
        "IPU1_0",
        PMHAL_PRCM_MOD_IPU1,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC
    },
#endif
#ifdef PROC_IPU1_1_INCLUDE
    {
        "IPU1_1",
        PMHAL_PRCM_MOD_IPU1,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC
    },
#endif
#ifdef PROC_DSP1_INCLUDE
    {
        "DSP1",
        PMHAL_PRCM_MOD_DSP1,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC
    },
#endif
#ifdef PROC_DSP2_INCLUDE
    {
        "DSP2",
        PMHAL_PRCM_MOD_DSP2,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC
    },
#endif
#ifdef PROC_EVE1_INCLUDE
    {
        "EVE1",
        PMHAL_PRCM_MOD_EVE1,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC,
    },
#endif
#ifdef PROC_EVE2_INCLUDE
    {
        "EVE2",
        PMHAL_PRCM_MOD_EVE2,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC,
    },
#endif
#ifdef PROC_EVE3_INCLUDE
    {
        "EVE3",
        PMHAL_PRCM_MOD_EVE3,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC,
    },
#endif
#ifdef PROC_EVE4_INCLUDE
    {
        "EVE4",
        PMHAL_PRCM_MOD_EVE4,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC,
    },
#endif
#ifdef IVAHD_INCLUDE
    {
        "IVA",
        PMHAL_PRCM_MOD_IVA,
        "PMHAL_PRCM_CLK_GENERIC",
        PMHAL_PRCM_CLK_GENERIC
    },
#endif
};

/**
 * \brief Structure to define the clocks that one wants to set. The frequency is
 *        given in Hz. Generic clock is used when ever the user does not want to
 *        set any specific clock.
 *        One can look at the generic clocks list to know which module's clock
 *        is defined as generic. To program a specific clock refer
 *        pmhalPrcmClockId_t enum in starterware.
 */
static cpuClockList_t           PeripheralClockList[] =
{
    {
        "PMHAL_PRCM_MOD_QSPI",
        PMHAL_PRCM_MOD_QSPI,
        "PMHAL_PRCM_CLK_QSPI_GFCLK",
        PMHAL_PRCM_CLK_QSPI_GFCLK
    }
};

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Print the temperature value for Available Temperature
 *        Sensor for given Voltage Id.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintTempValues(UInt32 voltId)
{
    pmhalBgapRange_t currTempRange = {0, 0};
    int32_t retVal = PM_SUCCESS;

    retVal = PMHALBgapGetCurrTemperature((pmhalPrcmVdId_t)voltId, &currTempRange);

    if(PM_SUCCESS == retVal)
    {
        Vps_printf("  %20s ||     [%d.%3d , %d.%3d]    \n",
               pmhalVoltageDomain_t_names[voltId],
               currTempRange.minTemp/1000, (abs(currTempRange.minTemp) % 1000),
               currTempRange.maxTemp/1000, (abs(currTempRange.maxTemp) % 1000));
    }
    else
    {
        Vps_printf(" UTILS_PRCM_STATS: Get Temperature Failed!!\n");
    }
}

/**
 *******************************************************************************
 *
 * \brief Print all the temperature value for Available Temperature Sensor.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllVDTempValues()
{
    UInt32 voltId;

    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  Current Temperature,\r\n");
    Vps_printf(" \r\n");
    Vps_printf("  Voltage Rail         ||   Curr Temp Min - Max   \r\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
    for(voltId = PMHAL_PRCM_VD_MIN;voltId < (pmhalPrcmVdId_t)PMHAL_BGAP_NUM_FSM;voltId++)
    {
        Utils_prcmPrintTempValues(voltId);
    }
    Vps_printf("\n");

}

/**
 *******************************************************************************
 *
 * \brief Print the Voltage value for Available Voltage rails corrosponding to
 *         given voltage id
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintVoltageValues(UInt32 voltId)
{
    int32_t retVal = PM_SUCCESS;
    UInt32 voltage = 0U;

    retVal = PMHALVMGetCurrentVoltage((pmhalPrcmVdId_t)voltId, &voltage);

    if(PM_SUCCESS == retVal)
    {
        Vps_printf("  %20s     ||      %d mV  \n",pmhalVoltageDomain_t_names[voltId],voltage);
    }
    else
    {
        Vps_printf(" UTILS_PRCM_STATS: Get Voltage Failed!! \n");
    }

}

/**
 *******************************************************************************
 *
 * \brief Print all the Voltage value for available Voltage Rails.
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllVoltageValues()
{
    UInt32 voltId;
    Vps_printf(" --------------------------------------------------------- \r\n");
    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  Current Volatge          \r\n");
    Vps_printf(" \r\n");
    Vps_printf("     Voltage Rail          ||      Voltage  \r\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
    for(voltId = PMHAL_PRCM_VD_MIN;voltId < PMHAL_PRCM_VD_MAX;voltId++)
    {
        Utils_prcmPrintVoltageValues(voltId);
    }
    Vps_printf("\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
}

/**
 *******************************************************************************
 *
 * \brief Print all PRCM Register data and the current state .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmDumpRegisterData()
{
    Int32 status;
    pmhalPrcmPdId_t pdIdx;
    pmhalPrcmCdId_t cdIdx;
    pmhalPrcmModuleId_t modIdx;
    pmhalPDStats_t pdStatus;
    pmhalCDStats_t cdStatus;
    pmhalModuleStats_t modStatus;

    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  PRCM DUMP Statistics         \r\n");
    Vps_printf(" \r\n");
    Vps_printf(" -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* \r\n");
    for(pdIdx = (pmhalPrcmPdId_t)0U; pdIdx < PMHAL_PRCM_PD_COUNT; pdIdx++)
    {
        Vps_printf(" \r\n");
        Vps_printf(" POWER DOMAIN    %20s \r\n", pmhalPowerDomain_t_names[pdIdx]);
        Vps_printf(" -------------------------------------------------------------------- \r\n");
        status = PMHALPrcmStatsGetPdRegData(pdIdx,&pdStatus);
        if(status == PM_SUCCESS)
        {
            Vps_printf("                                         |  Reg. Address  |  Value ");
            Vps_printf(" -------------------------------------------------------------------- \r\n");
            Vps_printf("                               PWRSTCTRL |  0x%8x    |  %08x        \n",
                        pdStatus.pwrStateCtrlRegAddr,
                        pdStatus.pwrStateCtrlRegVal);

            Vps_printf("                                 PWRSTST |  0x%8x    |  %08x      \n",
                        pdStatus.pwrStateStatusRegAddr,
                        pdStatus.pwrStateStatusRegVal);
        }
        else
        {
            Vps_printf(" PWRSTCTRL and PWRSTST Not Available        ");
        }

        if(pdStatus.numClockDomain)
        {
            for(cdIdx = (pmhalPrcmCdId_t)0U; cdIdx < pdStatus.numClockDomain; cdIdx++)
            {
                Vps_printf(" -------------------------------------------------------------------- \r\n");
                Vps_printf("        CLOCK DOMAIN      %s \r\n", pmhalClkDomain_t_names[pdStatus.prcmCdList[cdIdx]]);
                Vps_printf(" -------------------------------------------------------------------- \r\n");
                status = PMHALPrcmStatsGetCdRegData(pdStatus.prcmCdList[cdIdx],&cdStatus);

                if(status == PM_SUCCESS)
                {
                    Vps_printf("                                         |  Reg. Address  |   Value ");
                    Vps_printf(" -------------------------------------------------------------------- \r\n");
                    if(0x00 != cdStatus.clkStCtrlAddr)
                    {
                        Vps_printf("                               CLKSTCTRL |  0x%8x    |  %08x        \n",
                                cdStatus.clkStCtrlAddr,
                                cdStatus.clkStCtrlRegVal);
                    }
                    else
                    {
                        Vps_printf("                               CLKSTCTRL |  CLKSTCTRL REG NOT PRESENT \n");
                    }

                    if(0x00 != cdStatus.staticDepAddr)
                    {
                        Vps_printf("                       STATIC DEPENDENCY |  0x%8x    |  %08x      \n",
                                cdStatus.staticDepAddr,
                                cdStatus.staticDepRegVal);
                    }
                    else
                    {
                        Vps_printf("                       STATIC DEPENDENCY |    NOT PRESENT \n");
                    }
                }

                if(cdStatus.numModulesInCd)
                {
                    Vps_printf(" -------------------------------------------------------------------- \r\n");
                    Vps_printf("            MODULE  NAME                 | CLKCTRL Address|    Value   ");
                    Vps_printf(" -------------------------------------------------------------------- \r\n");
                    for(modIdx = (pmhalPrcmModuleId_t)0U; modIdx < cdStatus.numModulesInCd; modIdx++)
                    {
                        modStatus.clkCtrlRegAddr = 0U;
                        PMHALPrcmStatsGetModRegData(
                                    cdStatus.prcmModList[modIdx],&modStatus);
                        if(0x00 != modStatus.clkCtrlRegAddr)
                        {
                            Vps_printf("%40s |  0x%8x    |  %08x        ""\r\n",
                            pmhalNodeList_names[(cdStatus.prcmModList[modIdx])],
                            modStatus.clkCtrlRegAddr,
                            modStatus.clkCtrlRegVal);
                        }
                        else
                        {
                            Vps_printf("%40s |  CLKCTRL  REG NOT PRESENT \n",
                            pmhalNodeList_names[(cdStatus.prcmModList[modIdx])]);
                        }
                        BspOsal_sleep(50U);
                    }
                }
            }
            Vps_printf(" \r\n");
            Vps_printf(" -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* \r\n");
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief Print all Dpll Register data and the current state .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllDpllValues()
{
    Int32 status;
    pmhalPrcmNodeId_t idx;
    UInt32 mulValue,divValue,rootFreq,lockFreq,maxPostDiv,postDivCount,clkSpeed;
    pmhalPrcmClockActivityState_t clkStatus;
    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  DPLL Statistics         \r\n");
    Vps_printf(" \r\n");

    for (idx = PMHAL_PRCM_DPLL_MIN; idx < PMHAL_PRCM_DPLL_MAX; idx++)
    {
        pmhalPrcmPllPostDivValue_t divConfig[PMHAL_PRCM_DPLL_POST_DIV_COUNT];
        Vps_printf(" ----------------------------------------------- \r\n");
        Vps_printf(" %s          \n",pmhalNodeList_names[idx]);
        Vps_printf(" ----------------------------------------------- \r\n");
        Vps_printf(" \r\n");
        status = PMHALCMDpllLockStatus(idx);
        if(0U == status)
        {
            Vps_printf(" NOT LOCKED \n");
            Vps_printf(" \r\n");
        }
        else
        {
            mulValue = PMHALCMDpllGetMultiplier(idx);
            divValue = PMHALCMDpllGetDivider(idx);
            maxPostDiv = PMHALCMDpllGetAllPostDivStatus(idx,divConfig);
            rootFreq = 20U;
            lockFreq = (2U * rootFreq * mulValue)/(divValue + 1U);
            Vps_printf(" LOCKED with \n");
            Vps_printf(" Ref. Frequency (MHz) = %d \n",rootFreq);
            Vps_printf(" M Multiplier Factor  = %d \n",mulValue);
            Vps_printf(" N Divider Factor     = %d \n",divValue);
            Vps_printf(" DCOLDO Freq    (MHz) = %d \n",lockFreq);
            Vps_printf(" \r\n");
            Vps_printf(" Supported Number of Post Dividers = %d \n",maxPostDiv);
            Vps_printf(" \r\n");
            for(postDivCount = 0 ;
                postDivCount < maxPostDiv;
                postDivCount++)
            {
                PMHALCMDpllGetPostDivClkStatus(idx,
                                            divConfig[postDivCount].postDivId,
                                            &clkStatus);
                Vps_printf(" %s \n",
                    pmhalPostDivList_names[divConfig[postDivCount].postDivId]);
                if(clkStatus == PMHAL_PRCM_MODULE_CLOCKACTIVITY_CLK_GATED)
                {
                    Vps_printf(" Gated \n");
                }
                else
                {
                    Vps_printf(" Enabled \n");
                }
                Vps_printf(" Clock Divider = %d \n",
                    divConfig[postDivCount].configValue);

                if(divConfig[postDivCount].postDivId == PMHAL_PRCM_DPLL_POST_DIV_M2)
                {
                    clkSpeed = lockFreq /(2*divConfig[postDivCount].configValue);
                }
                else
                {
                    clkSpeed = lockFreq /(divConfig[postDivCount].configValue);
                }
                Vps_printf(" Clock Speed (MHz) = %d \n",
                    clkSpeed);
                Vps_printf(" \r\n");
            }
            Vps_printf(" \r\n");
        }
    }
    Vps_printf(" --------------------------------------------------------- \r\n");
    Vps_printf("\n");
}

/**
 *******************************************************************************
 *
 * \brief Init PMLIB ClockRate. Initialize during system init
 *
 * \return None
 *
 *******************************************************************************
 */
Int32 Utils_prcmClockRateInit()
{
    static UInt32                *rootClkFreqList;
    static pmlibBoardVdRailShare_t *vdRailShareList;
    Int32  retVal = PM_SUCCESS;

#ifdef PMIC_I2C_ENABLE

#ifdef TDA3XX_BUILD
    UInt32  siliconPackageType;
#endif

    const pmhalPmicOperations_t *pmicOps;
#ifdef TDA2EX_BUILD
    pmicOps = PMHALTps65917GetPMICOps();
#elif defined TDA2XX_BUILD
    pmicOps = PMHALTps659039GetPMICOps();
#elif defined TDA3XX_BUILD
    siliconPackageType = PlatformGetSiliconPackageType();
    if (PLATFORM_SILICON_PACKAGE_TYPE_15X15 == siliconPackageType)
    {
        pmicOps = PMHALTps65917GetPMICOps();
    }
    else
    {
        pmicOps = PMHALL8731GetPMICOps();
    }
#endif

    retVal = (pmErrCode_t) PMHALPmicRegister(pmicOps);
#endif
    rootClkFreqList = PMLIBBoardConfigGetRootClks();
    vdRailShareList = PMLIBBoardConfigGetVdRailShareInfo();
    retVal         += PMLIBClkRateInit(rootClkFreqList, vdRailShareList);

    return retVal;
}

/**
 *******************************************************************************
 *
 * \brief Print all CPU Frequency .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllCPUFrequency()
{
    Int32 status;
    UInt32 idx;
    UInt32    size = sizeof(cpuClockList)/sizeof(cpuClockList_t);
    Vps_printf(" --------------------------------------------------------- \r\n");
    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  CPU Frequency                          \r\n");
    Vps_printf(" \r\n");
    Vps_printf("     Core Name          ||      Freq in Mhz  \r\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
    for (idx = 0U; idx < size; idx++)
    {
        UInt32 clkGetRate = 0U;;
        status = PMLIBClkRateGet(cpuClockList[idx].modId,
                                 cpuClockList[idx].clkId,
                                 &clkGetRate);

        if (status == PM_SUCCESS)
        {
            Vps_printf("  %20s  ||      %d.%2d    \n",
                cpuClockList[idx].moduleCharId,
                clkGetRate/1000000,
                clkGetRate%1000000);
        }
    }
    Vps_printf("\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
}

/**
 *******************************************************************************
 *
 * \brief Print all the Peripheral Frequency .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllPeripheralsFrequency()
{
    Int32 status;
    UInt32 idx;
    UInt32    size = sizeof(PeripheralClockList)/sizeof(cpuClockList_t);
    Vps_printf(" --------------------------------------------------------- \r\n");
    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  Peripheral Frequency                   \r\n");
    Vps_printf(" \r\n");
    Vps_printf("     Peripheral Name           ||      Freq in Mhz  \r\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
    for (idx = 0U; idx < size; idx++)
    {
        UInt32 clkGetRate = 0U;;
        status = PMLIBClkRateGet(PeripheralClockList[idx].modId,
                                 PeripheralClockList[idx].clkId,
                                 &clkGetRate);

        if (status == PM_SUCCESS)
        {
            Vps_printf("  %20s     ||      %d.%2d    \n",
                PeripheralClockList[idx].moduleCharId,
                clkGetRate/1000000,
                clkGetRate%1000000);
        }
    }
    Vps_printf("\n");
    Vps_printf(" --------------------------------------------------------- \r\n");
}

/**
 *******************************************************************************
 *
 * \brief Print all Module current state .
 *
 * \return None
 *
 *******************************************************************************
 */
Void Utils_prcmPrintAllModuleState()
{

    pmhalPrcmModuleId_t modIdx;
    pmlibSysConfigDetailedState_t detailedState;
    pmlibSysConfigPowerState_t    moduleState;

    Vps_printf(" \r\n");
    Vps_printf(" UTILS_PRCM_STATS:  PRCM Module Statistics         \r\n");
    Vps_printf(" \r\n");

    Vps_printf(" ---------------------------------------------------------- \r\n");
    Vps_printf("            MODULE  NAME                 |  Module State    \n");

    for(modIdx = (pmhalPrcmModuleId_t)((UInt32)PMHAL_PRCM_MOD_LOWER_BOUND + 1U);
        modIdx < PMHAL_PRCM_MOD_COUNT;
        modIdx++)
    {
        moduleState = PMLIB_SYS_CONFIG_INVALID;
        detailedState.moduleMode = PMHAL_PRCM_MODULE_IDLEST_MIN;
        detailedState.clockMode = PMHAL_PRCM_MODULE_CLOCKACTIVITY_CLK_MIN;
        detailedState.pdMode = PMHAL_PRCM_PD_STATUS_MIN;
        PMLIBSysConfigGetPowerState(modIdx, &moduleState, &detailedState);

        if(moduleState != PMLIB_SYS_CONFIG_INVALID)
        {
            Vps_printf(" ---------------------------------------------------------- \r\n");
            Vps_printf("%40s | %15s    \r\n",
                            pmhalNodeList_names[modIdx],
                            moduleState_names[moduleState]);
            Vps_printf("                      Module SIDLE State | %15s \n",
                    moduleSIdleState_names[detailedState.moduleMode]);
            Vps_printf("                    Clock Activite State | %15s \n",
                            clockActivityState_names[detailedState.clockMode]);
            Vps_printf("                      Power Domain State | %15s \n",
                            powerDoaminState_names[detailedState.pdMode]);
        }
        else
        {
            Vps_printf(" ---------------------------------------------------------- \r\n");
            Vps_printf("%40s \r\n",
                            pmhalNodeList_names[modIdx]);
            Vps_printf(" moduleState returned PMLIB_SYS_CONFIG_INVALID \n");

            Vps_printf("                      Module SIDLE State | %15s \n",
                    moduleSIdleState_names[detailedState.moduleMode]);
            Vps_printf("                    Clock Activite State | %15s \n",
                            clockActivityState_names[detailedState.clockMode]);
            Vps_printf("                      Power Domain State | %15s \n",
                            powerDoaminState_names[detailedState.pdMode]);
        }
        BspOsal_sleep(50U);
    }
}
