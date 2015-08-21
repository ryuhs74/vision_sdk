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
 *   Filename:         utils_tda2xx_platform.h
 *
 *   Description:      This file contain sbl specific tda2xx platform's macro
 *                     definition
 */
#ifndef UTILS_TDA2xx_PLATFORM_H_
#define UTILS_TDA2xx_PLATFORM_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include "hw_types.h"
#include "soc.h"
#include "pm/pm_types.h"
#include "pm/pmhal/pmhal_vm.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Core Id Enum Definition */
#define CORE_A8 0
#define CORE_DSP 1
#define CORE_M3VIDEO 2
#define CORE_M3VPSS 3
#define CORE_ARP32 4

#define DEVICE_CENTEVE 8
#define DEVICE_CENTAURUS 3

#define QSPI_OFFSET_SI          0x80000

#define NOR_BASE_ADDRESS 0x8000000
#define NOR_OFFSET_SI       0x80000

#define SBL_DEBUG_LOGGER_BFR  0x40511000

#define MPU_DSP1_L2_RAM         0x40800000
#define MPU_DSP1_L1P_CACHE      0x40E00000
#define MPU_DSP1_L1D_CACHE      0x40F00000
#define MPU_DSP2_L2_RAM         0x41000000
#define MPU_DSP2_L1P_CACHE      0x41600000
#define MPU_DSP2_L1D_CACHE      0x41700000

#define MPU_IPU1_ROM                                (SOC_IPU1_TARGET_BASE)
#define MPU_IPU1_RAM                                (SOC_IPU1_TARGET_BASE + \
                                                     0x20000)
#define MPU_IPU1_UNICACHE_MMU_CONFIG_REGS       (SOC_IPU1_TARGET_BASE + 0x80000)
#define MPU_IPU1_WUGEN                              (SOC_IPU1_TARGET_BASE + \
                                                     0x81000)
#define MPU_IPU1_MMU                                (SOC_IPU1_TARGET_BASE + \
                                                     0x82000)

#define MPU_IPU2_ROM                                (SOC_IPU2_TARGET_BASE)
#define MPU_IPU2_RAM                                (SOC_IPU2_TARGET_BASE + \
                                                     0x20000)
#define MPU_IPU2_UNICACHE_MMU_CONFIG_REGS       (SOC_IPU2_TARGET_BASE + 0x80000)
#define MPU_IPU2_WUGEN                              (SOC_IPU2_TARGET_BASE + \
                                                     0x81000)
#define MPU_IPU2_MMU                                (SOC_IPU2_TARGET_BASE + \
                                                     0x82000)

#define MPU_EVE1_DMEM_BASE      0x62020000
#define MPU_EVE1_WBUF_BASE      0x62040000
#define MPU_EVE1_IBUF_LA_BASE   0x62050000
#define MPU_EVE1_IBUF_HA_BASE   0x62054000
#define MPU_EVE1_IBUF_LB_BASE   0x62070000
#define MPU_EVE1_IBUF_HB_BASE   0x62074000

#define MPU_EVE2_DMEM_BASE      0x62120000
#define MPU_EVE2_WBUF_BASE      0x62140000
#define MPU_EVE2_IBUF_LA_BASE   0x62150000
#define MPU_EVE2_IBUF_HA_BASE   0x62154000
#define MPU_EVE2_IBUF_LB_BASE   0x62170000
#define MPU_EVE2_IBUF_HB_BASE   0x62174000

#define MPU_EVE3_DMEM_BASE      0x62220000
#define MPU_EVE3_WBUF_BASE      0x62240000
#define MPU_EVE3_IBUF_LA_BASE   0x62250000
#define MPU_EVE3_IBUF_HA_BASE   0x62254000
#define MPU_EVE3_IBUF_LB_BASE   0x62270000
#define MPU_EVE3_IBUF_HB_BASE   0x62274000

#define MPU_EVE4_DMEM_BASE      0x62320000
#define MPU_EVE4_WBUF_BASE      0x62340000
#define MPU_EVE4_IBUF_LA_BASE   0x62350000
#define MPU_EVE4_IBUF_HA_BASE   0x62354000
#define MPU_EVE4_IBUF_LB_BASE   0x62370000
#define MPU_EVE4_IBUF_HB_BASE   0x62374000


#define SOC_EVE1_MMU0_BASE      0x62081000
#define SOC_EVE1_MMU1_BASE      0x62082000
#define SOC_EVE1_TPTC0_BASE     0x62086000
#define SOC_EVE1_TPTC1_BASE     0x62087000


#define SOC_EVE2_MMU0_BASE      0x62181000
#define SOC_EVE2_MMU1_BASE      0x62182000
#define SOC_EVE2_TPTC0_BASE     0x62186000
#define SOC_EVE2_TPTC1_BASE     0x62187000

#define SOC_EVE3_MMU0_BASE      0x62281000
#define SOC_EVE3_MMU1_BASE      0x62282000
#define SOC_EVE3_TPTC0_BASE     0x62286000
#define SOC_EVE3_TPTC1_BASE     0x62287000

#define SOC_EVE4_MMU0_BASE      0x62381000
#define SOC_EVE4_MMU1_BASE      0x62382000
#define SOC_EVE4_TPTC0_BASE     0x62386000
#define SOC_EVE4_TPTC1_BASE     0x62387000



#define SOC_DSP_L2_BASE                            0x800000
#define SOC_DSP_L1P_BASE                         0xe00000
#define SOC_DSP_L1D_BASE                         0xf00000

//#define SOC_IPU1_BOOT_SPACE_BASE                                        0x0
//#define SOC_IPU1_ROM_BASE \
    0x55000000
//#define SOC_IPU1_RAM_BASE \
    0x55020000

#define SOC_EVE_DMEM_BASE \
    0x40020000
#define SOC_EVE_WBUF_BASE \
    0x40040000
#define SOC_EVE_IBUF_LA_BASE \
    0x40050000
#define SOC_EVE_IBUF_HA_BASE \
    0x40054000
#define SOC_EVE_IBUF_LB_BASE \
    0x40070000
#define SOC_EVE_IBUF_HB_BASE \
    0x40074000

#define SOC_OCMC_RAM1_SIZE          0x80000     /*OCMC1 512KB*/
#define SOC_OCMC_RAM2_SIZE          0x100000    /*OCMC2 1MB   */
#define SOC_OCMC_RAM3_SIZE          0x100000    /*OCMC3  1MB   */

/* PRM_VC_VAL_BYPASS */
#define PRM_VC_I2C_CHANNEL_FREQ_KHZ 400

/*DSS PLL Registers*/
#define PLL_CONTROL                 0x00000000
#define PLL_STATUS                      0x00000004
#define PLL_GO                                  0x00000008
#define PLL_CONFIGURATION1      0x0000000C
#define PLL_CONFIGURATION2      0x00000010
#define PLL_CONFIGURATION3      0x00000014

/*DSI1 Registers */
#define DSI_CLK_CTRL                    0x58004054
#define DSI_PLL_STATUS              0x58004304

/*DSI1 Registers */
#define DSI2_CLK_CTRL                   0x58009054
#define DSI2_PLL_STATUS             0x58009304

/*Memory Subsystem*/
#define DMM_LISA_MAP                        0x40

/* Memory Adapter */
#define DMM_LISA_MA_BASE                0x482AF040

#define DUAL_EMIF_2X512MB       (0)
#define DUAL_EMIF_1GB_512MB     (1)
#define SINGLE_EMIF_256MB           (2)

/* OPP Configuration*/
#define OPP_LOW                  (0)
#define OPP_NOM                  (1)
#define OPP_OD                   (2)
#define OPP_HIGH                 (3)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */
typedef enum cpu_core_id
{
    MPU_CPU0_ID = 0,
    MPU_CPU1_ID,
    IPU1_CPU0_ID,
    IPU1_CPU1_ID,
    IPU1_CPU_SMP_ID,
    IPU2_CPU0_ID,
    IPU2_CPU1_ID,
    IPU2_CPU_SMP_ID,
    DSP1_ID,
    DSP2_ID,
    EVE1_ID,
    EVE2_ID,
    EVE3_ID,
    EVE4_ID
}cpu_core_id_t;

typedef enum {
    TRACE_REG    = 0, /*Regression Mode*/
    TRACE_HIGH   = 1,
    TRACE_MEDIUM = 2,
    TRACE_LOW    = 3
}traceLevel_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
void  SBL_PRINTF(traceLevel_t level, const char *ptr);
void  BootAbort(void);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif /*SBL_TDA2xx_PLATFORM_H_*/
