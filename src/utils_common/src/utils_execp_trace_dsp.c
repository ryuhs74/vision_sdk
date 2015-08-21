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
 * \file utils_execp_trace_dsp.c
 *
 * \brief  This file has the implementation for Exception Trace for all
 *         remote DSP cores
 *
 *         Refer Utils_dspExceptionHookFxn() for details
 *
 * \version 0.0 (Jul 2013) : [HS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
*/
#include <src/utils_common/include/utils.h>
#include <ti/sysbios/family/c64p/Exception.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Implementation for Exception Trace
 *
 *        This function catches the exception and prints onto shared buffer.
 *        Finally shared buffer will be printed by host core onto uart.
 *        So remote core exception will be printed on host core.
 *
 *
 *
 *******************************************************************************
*/
Void Utils_dspExceptionHookFxn()
{
    Exception_Status status;
    Exception_Context *excp;

    Exception_getLastStatus(&status);

    /* set exception context */
    excp = status.excContext;

    /* Force MAIN threadtype So we can safely call Vps_printf */
    BIOS_setThreadType(BIOS_ThreadType_Main);

    Vps_printf("Utils_dspExceptionHookFxn:\n");
    Vps_printf("  efr=0x%x\n", status.efr);
    Vps_printf("  nrp=0x%x\n", status.nrp);
    Vps_printf("  ntsr=0x%x\n", status.ntsr);
    Vps_printf("  ierr=0x%x\n", status.ierr);
    Vps_printf("  excContext=0x%x\n", status.excContext);

    Vps_printf("A0=0x%x A1=0x%x\n", excp->A0, excp->A1);
    Vps_printf("A2=0x%x A3=0x%x\n", excp->A2, excp->A3);
    Vps_printf("A4=0x%x A5=0x%x\n", excp->A4, excp->A5);
    Vps_printf("A6=0x%x A7=0x%x\n", excp->A6, excp->A7);
    Vps_printf("A8=0x%x A9=0x%x\n", excp->A8, excp->A9);
    Vps_printf("A10=0x%x A11=0x%x\n", excp->A10, excp->A11);
    Vps_printf("A12=0x%x A13=0x%x\n", excp->A12, excp->A13);
    Vps_printf("A14=0x%x A15=0x%x\n", excp->A14, excp->A15);
    Vps_printf("A16=0x%x A17=0x%x\n", excp->A16, excp->A17);
    Vps_printf("A18=0x%x A19=0x%x\n", excp->A18, excp->A19);
    Vps_printf("A20=0x%x A21=0x%x\n", excp->A20, excp->A21);
    Vps_printf("A22=0x%x A23=0x%x\n", excp->A22, excp->A23);
    Vps_printf("A24=0x%x A25=0x%x\n", excp->A24, excp->A25);
    Vps_printf("A26=0x%x A27=0x%x\n", excp->A26, excp->A27);
    Vps_printf("A28=0x%x A29=0x%x\n", excp->A28, excp->A29);
    Vps_printf("A30=0x%x A31=0x%x\n", excp->A30, excp->A31);
    Vps_printf("B0=0x%x B1=0x%x\n", excp->B0, excp->B1);
    Vps_printf("B2=0x%x B3=0x%x\n", excp->B2, excp->B3);
    Vps_printf("B4=0x%x B5=0x%x\n", excp->B4, excp->B5);
    Vps_printf("B6=0x%x B7=0x%x\n", excp->B6, excp->B7);
    Vps_printf("B8=0x%x B9=0x%x\n", excp->B8, excp->B9);
    Vps_printf("B10=0x%x B11=0x%x\n", excp->B10, excp->B11);
    Vps_printf("B12=0x%x B13=0x%x\n", excp->B12, excp->B13);
    Vps_printf("B14=0x%x B15=0x%x\n", excp->B14, excp->B15);
    Vps_printf("B16=0x%x B17=0x%x\n", excp->B16, excp->B17);
    Vps_printf("B18=0x%x B19=0x%x\n", excp->B18, excp->B19);
    Vps_printf("B20=0x%x B21=0x%x\n", excp->B20, excp->B21);
    Vps_printf("B22=0x%x B23=0x%x\n", excp->B22, excp->B23);
    Vps_printf("B24=0x%x B25=0x%x\n", excp->B24, excp->B25);
    Vps_printf("B26=0x%x B27=0x%x\n", excp->B26, excp->B27);
    Vps_printf("B28=0x%x B29=0x%x\n", excp->B28, excp->B29);
    Vps_printf("B30=0x%x B31=0x%x\n", excp->B30, excp->B31);
    Vps_printf("NTSR=0x%x\n", excp->NTSR);
    Vps_printf("ITSR=0x%x\n", excp->ITSR);
    Vps_printf("IRP=0x%x\n", excp->IRP);
    Vps_printf("SSR=0x%x\n", excp->SSR);
    Vps_printf("AMR=0x%x\n", excp->AMR);
    Vps_printf("RILC=0x%x\n", excp->RILC);
    Vps_printf("ILC=0x%x\n", excp->ILC);
    Vps_printf ("Terminating Execution...");

}


