/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file ultrasonicCaptureLink_hw.c
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <xdc/std.h>
#include <stdio.h>
#include <string.h>
#include <hw/hw_types.h>
#include <hw/hw_ctrl_wkup.h>
#include <hw/hw_ctrl_core_pad_io.h>
#include <hw/hw_wkupaon_cm.h>
#include <soc.h>
#include <soc_defines.h>

#define PRCM_ENABLE                     ((UInt32) 2U)
#define DEFAULT                         ((UInt32) 0xFF)
#define PINMUX_MODE_3                   ((UInt32) 3U)


Int32 UltrasonicCaptureLink_setPinmuxRegs
                (UInt32 mode_index, UInt32 offset, UInt32 pupd_info)
{
    UInt32 muxVal;

    if (offset != (UInt32) 0xffff)
    {
        muxVal  = HW_RD_REG32(SOC_CORE_PAD_IO_REGISTERS_BASE + offset);
        muxVal &= ~(0x0FU);
        muxVal |= (mode_index & 0x0000000FU);

        if (pupd_info != (UInt32) DEFAULT)
        {
            muxVal &= ~(0x70000U);
            muxVal |= ((pupd_info & 0x07U) << 16U);
        }
        HW_WR_REG32(SOC_CORE_PAD_IO_REGISTERS_BASE + offset, muxVal);
    }

    return (0);
}

/**
 * \brief Setup pin mux and power UART used for ultrasonic comm.
 */
void UltrasonicCaptureLink_hwSetup()
{
    #ifdef TDA2XX_FAMILY_BUILD
    /* UART10 mux
     * PAD=gpio6_14, PIN=uart10_rxd
     * PAD=gpio6_15, PIN=uart10_txd */

    UltrasonicCaptureLink_setPinmuxRegs(PINMUX_MODE_3, (UInt32) CTRL_CORE_PAD_GPIO6_14, DEFAULT);
    UltrasonicCaptureLink_setPinmuxRegs(PINMUX_MODE_3, (UInt32) CTRL_CORE_PAD_GPIO6_15, DEFAULT);

    /* Enable UART10 module */
    HW_WR_REG32(
        SOC_WKUPAON_CM_BASE + CM_WKUPAON_UART10_CLKCTRL, PRCM_ENABLE);
    while ((HW_RD_REG32(SOC_WKUPAON_CM_BASE +
                CM_WKUPAON_UART10_CLKCTRL) & ((UInt32) (0x00030000)))
                    != 0x0)
    {
        /* Do nothing - Busy wait */
    }

    #endif
}
