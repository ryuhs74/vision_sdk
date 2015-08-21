/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup UTILS_API
 * \defgroup UTILS_VIP_INTERRUPT_API APIs to register VIP interrupt from EVE
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_vip_interrupt.h
 *
 * \brief  APIs to register VIP interrupt from EVE
 *
 * \version 0.0 (Jul 2014) : [VT] First version
 *
 *
 *******************************************************************************
 */

#ifndef UTILS_VIP_INTERRUPT_PRV_H
#define UTILS_VIP_INTERRUPT_PRV_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include <ti/sysbios/family/shared/vayu/IntXbar.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief VIP Interrupt no. for frame and Subframe Completion
 *******************************************************************************
 */
#define UTILS_IRQ_XBAR_VIP1_IRQ_2                   (392)


/**
 *******************************************************************************
 * \brief EVE Interrupt no. for frame and Subframe Completion
 *******************************************************************************
 */
#define UTILS_IRQ_XBAR_EVE_VIP                      (3)


/**
 *******************************************************************************
 * \brief All the register locations and mask values for enabling and clearing
 * List complete and subframe interrupts from VIP
 *******************************************************************************
 */

#define UTILS_EVE_VIPTOP_LC_EVENT_ENABLE_ADDR       0x68970050
#define UTILS_EVE_VIPTOP_LC_EVENT_ENABLE_MASK       0x01

#define UTILS_EVE_VPDMA_LC_EVENT_ENABLE_ADDR        0x6897D0DC
#define UTILS_EVE_VPDMA_LC_EVENT_ENABLE_MASK        0x01

#define UTILS_EVE_VIPTOP_SUBFRAME_EVENT_ENABLE_ADDR 0x68970054
#define UTILS_EVE_VIPTOP_SUBFRAME_EVENT_ENABLE_MASK 0x80

/* Enable interrupts for VIP1_SLICE0_UP_Y (TDA2XX_EVM) & VIP1_SLICE1_UP_Y (TDA2XX_MC) */
#define UTILS_EVE_VPDMA_SUBFRAME_EVENT_ENABLE_ADDR  0x6897D0D4
#define UTILS_EVE_VPDMA_SUBFRAME_EVENT_ENABLE_MASK  0x110


#define UTILS_EVE_VIPTOP_LC_EVENT_CLEAR_ADDR        0x68970048
#define UTILS_EVE_VIPTOP_LC_EVENT_CLEAR_MASK        0x01

#define UTILS_EVE_VPDMA_LC_EVENT_CLEAR_ADDR         0x6897D0D8
#define UTILS_EVE_VPDMA_LC_EVENT_CLEAR_MASK         0x01

#define UTILS_EVE_VIPTOP_SUBFRAME_EVENT_CLEAR_ADDR  0x6897004C
#define UTILS_EVE_VIPTOP_SUBFRAME_EVENT_CLEAR_MASK  0x80

/* Check interrupts for VIP1_SLICE0_UP_Y (TDA2XX_EVM) & VIP1_SLICE1_UP_Y (TDA2XX_MC) */
#define UTILS_EVE_VPDMA_SUBFRAME_EVENT_CLEAR_ADDR   0x6897D0D0
#define UTILS_EVE_VPDMA_SUBFRAME_EVENT_CLEAR_MASK   0x110

#define UTILS_EVE_END_OF_INTERRUPT_ADDR             0x689700A0
#define UTILS_EVE_END_OF_INTERRUPT_MASK             0x01


/*******************************************************************************
 * \brief The macros to read and write from/to the memory
 *******************************************************************************
 */
#define WR_MEM_32(addr, data)    *(unsigned int*)(addr) =(unsigned int)(data)
#define RD_MEM_32(addr)          *(unsigned int*)(addr)

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */
BspOsal_IntrHandle Utils_EVE_RegisterInterrupts_FromVIP(UInt xbarSrc,
                UInt xbarDest, UInt32 intrId, Hwi_FuncPtr iSRfunc, void *pPtr);

void Utils_EVE_UnregisterInterrupts_FromVIP(BspOsal_IntrHandle *hwi);

void Utils_VIP_Interrupt_EndOfInterrupt();

Bool Utils_VIP_Interrupt_IsSubframe();

void Utils_VIP_Interrupt_ClearSubframe_Interrupts();

Bool Utils_VIP_Interrupt_IsFrame();

void Utils_VIP_Interrupt_Clearframe_Interrupts();

#ifdef __cplusplus
}
#endif

#endif /* UTILS_VIP_INTERRUPT_PRV_H */

/*@}*/
