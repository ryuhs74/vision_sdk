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
 * \file hdvicp2_config.c
 *
 * \brief  This file contains device level configuration information for IVA-HD.
 *         Used for HDVICP PRCM and Reset
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 * \version 0.1 (Jul 2014) : [SS] Second version
 *
 *******************************************************************************
 */

/****************************************************************
*  INCLUDE FILES
****************************************************************/
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <ti/sysbios/hal/Hwi.h>
#include <src/utils_common/include/utils.h>
#include <ti/ipc/remoteproc/Resource.h>

/****************************************************************
*  DEFINES
****************************************************************/

/* Enable this to get the debug prints for IVA PRCM */
//#define SYSTEM_DEBUG_IVA_PRCM_IVA_PRCM

#define IVAHD_REG(off)            (*(volatile unsigned int *)(0x4AE06F00 + (off)))

#define PM_IVAHD_PWRSTCTRL        IVAHD_REG(0x00)
#define RM_IVAHD_RSTCTRL          IVAHD_REG(0x10)
#define RM_IVAHD_RSTST            IVAHD_REG(0x14)

#define CM_IVAHD_CLKSTCTRL        (*(volatile unsigned int *)0x4A008F00)
#define CM_IVAHD_CLKCTRL          (*(volatile unsigned int *)0x4A008F20)
#define CM_IVAHD_SL2_CLKCTRL      (*(volatile unsigned int *)0x4A008F28)
#define CM_DIV_DPLL_IVA           (*(volatile unsigned int *)0x4A0051B0)
#define IVAHD_CONFIG_REG_BASE     (0x5A000000)

#define ICONT1_ITCM_BASE          (IVAHD_CONFIG_REG_BASE + 0x08000)
#define ICONT2_ITCM_BASE          (IVAHD_CONFIG_REG_BASE + 0x18000)

#define  HDVICP_NUM_RESOURCES     (1)


/*******************************************************************************
 *   Hex code to set for Stack setting, Interrupt vector setting
 *   and instruction to put ICONT in WFI mode.
 *   This shall be placed at TCM_BASE_ADDRESS of given IVAHD, which is
 *   0x0000 locally after reset.
 *******************************************************************************/

const unsigned int    icont_boot[] =
{
    0xEA000006,
    0xEAFFFFFE,
    0xEAFFFFFE,
    0xEAFFFFFE,
    0xEAFFFFFE,
    0xEAFFFFFE,
    0xEAFFFFFE,
    0xEAFFFFFE,
    0xE3A00000,
    0xEE070F9A,
    0xEE070F90,
    0xE3A00000,
    0xEAFFFFFE,
    0xEAFFFFF1
};

static void ivahd_boot(void)
{
    int                      i;
    volatile unsigned int   *icont1_itcm_base_addr =
        (unsigned int *)ICONT1_ITCM_BASE;
    volatile unsigned int   *icont2_itcm_base_addr =
        (unsigned int *)ICONT2_ITCM_BASE;

    /*
     * Reset IVA HD, SL2 and ICONTs
     */
#ifdef SYSTEM_DEBUG_IVA_PRCM
    Vps_printf(" IVA_PRCM: IVA_PRCM: Booting IVAHD...Start!!!");
#endif

    /* Sequence
     * Apply Reset (0xAA306F10 = 0x7)
     * Turn IVA power state to on (0xAA306F00 = 0x3)
     * Set CM to SW WKUP (0xAA008F00 = 0x2)
     * Set IVA CLK to Auto (0xAA008F20 = 0x1)
     * Set SL2 CLK to Auto (0xAA008F28 = 0x1)
     * Apply reset to ICONT1 and ICONT2 and remove SL2 reset (0xAA306F10 = 0x3)
     * Code load ICONTs
     * Release Reset for ICONT1 and ICONT2 (0xAA306F10 = 0x0)
    */

    RM_IVAHD_RSTCTRL = 0x00000007;
    BspOsal_sleep(10);

    /*POWERSTATE : IVAHD_PRM:PM_IVAHD_PWRSTCTRL*/
    PM_IVAHD_PWRSTCTRL = 0x00000003;
    BspOsal_sleep(10);

    /*IVAHD_CM2:CM_IVAHD_CLKSTCTRL = SW_WKUP*/
    CM_IVAHD_CLKSTCTRL = 0x00000002;
    BspOsal_sleep(10);

    /*IVAHD_CM2:CM_IVAHD_IVAHD_CLKCTRL*/
    CM_IVAHD_CLKCTRL= 0x00000001;
    BspOsal_sleep(10);

    /*IVAHD_CM2:CM_IVAHD_SL2_CLKCTRL*/
    CM_IVAHD_SL2_CLKCTRL = 0x00000001;
    BspOsal_sleep(10);

    /* put ICONT1 & ICONT2 in reset and remove SL2 reset */
#ifdef SYSTEM_DEBUG_IVA_PRCM
    Vps_printf(" IVA_PRCM:Putting [ICONT1 ICONT2]: RESET and SL2:OutOfRESET...");
#endif

    RM_IVAHD_RSTCTRL = 0x00000003;
    BspOsal_sleep(10);

    /* Copy boot code to ICONT1 & ICONT2 memory */
    for( i = 0; i < (sizeof(icont_boot) / sizeof(unsigned int)); i++ ) {
        *icont1_itcm_base_addr++ = icont_boot[i];
        *icont2_itcm_base_addr++ = icont_boot[i];
    }
    // *icont1_itcm_base_addr = 0xEAFFFFFE;
    // *icont2_itcm_base_addr = 0xEAFFFFFE;

    BspOsal_sleep(10);

    RM_IVAHD_RSTCTRL = 0x00000000;
    BspOsal_sleep(10);

    /*Read1 IVAHD_ROOT_CLK is running or gatng/ungating transition is on-going*/
#ifdef SYSTEM_DEBUG_IVA_PRCM
    Vps_printf(" IVA_PRCM:Waiting for IVAHD to go out of reset\n");
#endif

    while(((CM_IVAHD_CLKSTCTRL) & 0x100) & ~0x100 ) {
        ;
    }

#ifdef SYSTEM_DEBUG_IVA_PRCM
    Vps_printf(" IVA_PRCM:Booting IVAHD...Done!!!");
#endif

}

static int    ivahd_use_cnt = 0;

void ivahd_acquire(void)
{
    UInt    hwiKey = Hwi_disable();

    if( ++ivahd_use_cnt == 1 ) {
#ifdef SYSTEM_DEBUG_IVA_PRCM
        Vps_printf(" IVA_PRCM: ivahd acquire");
#endif
        /* switch SW_WAKEUP mode */
        CM_IVAHD_CLKSTCTRL = 0x00000002;
    } else {
#ifdef SYSTEM_DEBUG_IVA_PRCM
        Vps_printf(" IVA_PRCM: ivahd already acquired");
#endif
    }
    Hwi_restore(hwiKey);
}

void ivahd_release(void)
{
    UInt    hwiKey = Hwi_disable();

    if( ivahd_use_cnt-- == 1 ) {
#ifdef SYSTEM_DEBUG_IVA_PRCM
        Vps_printf(" IVA_PRCM: ivahd release");
#endif
        /* switch HW_AUTO mode */
        CM_IVAHD_CLKSTCTRL = 0x00000003;
    } else {
#ifdef SYSTEM_DEBUG_IVA_PRCM
        Vps_printf(" IVA_PRCM: ivahd still in use");
#endif
    }
    Hwi_restore(hwiKey);
}

Void HDVICP2_Init()
{
    ivahd_acquire();
    ivahd_boot();
    ivahd_release();
}

extern UInt ti_sdo_fc_ires_hdvicp_HDVICP2_interrupts[];

Void HDVICP2_ClearIVAInterrupts()
{
    Int i;

    for (i = 0; i < HDVICP_NUM_RESOURCES; i++)
    {
        Hwi_clearInterrupt(ti_sdo_fc_ires_hdvicp_HDVICP2_interrupts[i]);
    }
}

#ifdef A15_TARGET_OS_LINUX

static unsigned int SyslinkMemUtils_VirtToPhys(Ptr Addr)
{
    unsigned int    pa;

    if( !Addr || Resource_virtToPhys((unsigned int) Addr, &pa)) {
        return (0);
    }
    return (pa);
}

void *MEMUTILS_getPhysicalAddr(Ptr vaddr)
{
    unsigned int    paddr = SyslinkMemUtils_VirtToPhys(vaddr);

    return ((void *)paddr);
}

#endif

/* Nothing beyond this point */


