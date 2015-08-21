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
 * \file utils_execp_trace_remote_m4.c
 *
 * \brief  This file has the implementataion for Exception Trace for all
 *         remote cores
 *
 *         Refer Utils_exceptionHookFxn() for details
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
#define  ti_sysbios_family_arm_m3_Hwi__nolocalnames
#define  ti_sysbios_hal_Hwi__nolocalnames
/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
*/
#include <src/utils_common/include/utils.h>
#include <elf_linkage.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#ifdef MIN
#undef MIN
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/**
 *******************************************************************************
 *
 * \brief Implementation for EXception Trace
 *
 *        By default, when an exception occurs, an ExcContext structure is
 *        allocated on the ISR stack and filled in within the exception handler
 *        If excContextBuffer is initialized by the user, the ExcContext
 *        structure will be placed at that address instead. The buffer must be
 *        large enough to contain an ExcContext structure.
 *
 *        By default, when an exception occurs, a pointer to the base address
 *        of the stack being used by the thread causing the exception is used.
 *        If excStackBuffer is initialized by the user, the stack contents of
 *        the thread causing the exception will be copied to that address
 *        instead. The buffer must be large enough to contain the largest task
 *        stack or ISR stack defined in the application.
 *
 * \param   excCtx [IN]
 *
 * \return  None
 *
 *******************************************************************************
*/

Void Utils_m4ExceptionHookFxn(ti_sysbios_family_arm_m3_Hwi_ExcContext *excCtx)
{

    Vps_printf ("Unhandled Exception:");
    if (excCtx->threadType == BIOS_ThreadType_Hwi){
            Vps_printf ("Exception occurred in ThreadType_HWI");
    } else if (excCtx->threadType == BIOS_ThreadType_Swi){
            Vps_printf ("Exception occurred in ThreadType_SWI");
    } else if (excCtx->threadType == BIOS_ThreadType_Task){
            Vps_printf ("Exception occurred in ThreadType_Task");
    } else if (excCtx->threadType == BIOS_ThreadType_Main){
            Vps_printf ("Exception occurred in ThreadType_Main");
    }

    Vps_printf ("handle: 0x%x.\n", excCtx->threadHandle);
    Vps_printf ("stack base: 0x%x.\n", excCtx->threadStack);
    Vps_printf ("stack size: 0x%x.\n", excCtx->threadStackSize);

    Vps_printf ("R0 = 0x%08x  R8  = 0x%08x\n", excCtx->r0, excCtx->r8);
    Vps_printf ("R1 = 0x%08x  R9  = 0x%08x\n", excCtx->r1, excCtx->r9);
    Vps_printf ("R2 = 0x%08x  R10 = 0x%08x\n", excCtx->r2, excCtx->r10);
    Vps_printf ("R3 = 0x%08x  R11 = 0x%08x\n", excCtx->r3, excCtx->r11);
    Vps_printf ("R4 = 0x%08x  R12 = 0x%08x\n", excCtx->r4, excCtx->r12);
    Vps_printf ("R5 = 0x%08x  SP(R13) = 0x%08x\n", excCtx->r5, excCtx->sp);
    Vps_printf ("R6 = 0x%08x  LR(R14) = 0x%08x\n", excCtx->r6, excCtx->lr);
    Vps_printf ("R7 = 0x%08x  PC(R15) = 0x%08x\n", excCtx->r7, excCtx->pc);
    Vps_printf ("PSR = 0x%08x\n", excCtx->psr);
    Vps_printf ("ICSR = 0x%08x\n", ti_sysbios_family_arm_m3_Hwi_nvic.ICSR);
    Vps_printf ("MMFSR = 0x%02x\n", ti_sysbios_family_arm_m3_Hwi_nvic.MMFSR);
    Vps_printf ("BFSR = 0x%02x\n", ti_sysbios_family_arm_m3_Hwi_nvic.BFSR);
    Vps_printf ("UFSR = 0x%04x\n", ti_sysbios_family_arm_m3_Hwi_nvic.UFSR);
    Vps_printf ("HFSR = 0x%08x\n", ti_sysbios_family_arm_m3_Hwi_nvic.HFSR);
    Vps_printf ("DFSR = 0x%08x\n", ti_sysbios_family_arm_m3_Hwi_nvic.DFSR);
    Vps_printf ("MMAR = 0x%08x\n", ti_sysbios_family_arm_m3_Hwi_nvic.MMAR);
    Vps_printf ("BFAR = 0x%08x\n", ti_sysbios_family_arm_m3_Hwi_nvic.BFAR);
    Vps_printf ("AFSR = 0x%08x\n", ti_sysbios_family_arm_m3_Hwi_nvic.AFSR);
    Vps_printf ("Terminating Execution...");
}



/* Nothing beyond this point */
