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
 * \file utils_uart.c
 *
 * \brief  This file has the implementataion for UART
 *
 * \version 0.0 (Jun 2013) : [NN] First version
 * \version 0.1 (Jul 2013) : [NN] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <xdc/std.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/io/GIO.h>
#include <uart/bsp_uart.h>
#include <platforms/bsp_platform.h>
#include <boards/bsp_board.h>
#include <src/utils_common/include/utils_uart.h>

/**
 *******************************************************************************
 * \brief Buffer to read from UART console
 *******************************************************************************
 */
#pragma DATA_ALIGN(uartReadBuffer, 128);
static char       uartReadBuffer[UART_BUFFER_SIZE];

/**
 *******************************************************************************
 * \brief UART handle for input stream
 *******************************************************************************
 */
GIO_Handle       uartRxHandle;

/**
 *******************************************************************************
 * \brief UART handle for output stream
 *******************************************************************************
 */
GIO_Handle       uartTxHandle;

/**
 *******************************************************************************
 * \brief Global to check whether UART is initialized or not
 *******************************************************************************
 */
static Bool    InitDone  = FALSE;

/**
 *******************************************************************************
 *
 * \brief Return TRUE if UART init is done
 *
 * \return  TRUE if UART init is done, else FALSE
 *
 *******************************************************************************
 */

Bool System_isUartInitDone()
{
    return InitDone;
}

/**
 *******************************************************************************
 *
 * \brief Initializes the UART and sets the GIO handles for the Tx and Rx
 *
 * \return  None
 *
 *******************************************************************************
 */
Void System_uartInit()
{
    Uart_ChanParams chanParams;
    Error_Block eb;
    GIO_Params  ioParams;
    static char uartName[16]; /* device name MUST be global or static variable */
    Uart_Params   uartParams;
    int devId;

    Error_init(&eb);

    /*
     * Initialize channel attributes.
     */
    GIO_Params_init(&ioParams);

    Uart_init();

#ifdef AVM_E500_BUILD		///craven@150901
    strcpy(uartName, "/uart1");
    devId = 1;
#else
    if(Bsp_platformIsTda2xxFamilyBuild())
    {
        if(Bsp_boardGetId() == BSP_BOARD_MONSTERCAM)
        {
            strcpy(uartName, "/uart2");
            devId = 2;
        }
        else
        {
            strcpy(uartName, "/uart0");
            devId = 0;
        }
    }
    else
    if(Bsp_platformIsTda3xxFamilyBuild())
    {
        strcpy(uartName, "/uart2");
        devId = 2;
    }
    else
    {
        /* default */
        strcpy(uartName, "/uart0");
        devId = 0;
    }
#endif

    uartParams              = Uart_PARAMS;
    uartParams.opMode       = UART_OPMODE_INTERRUPT;
    uartParams.hwiNumber    = 8u;
    uartParams.rxThreshold  = UART_RXTRIGLVL_8;
    uartParams.txThreshold  = UART_TXTRIGLVL_56;
    uartParams.baudRate     = UART_BAUDRATE_115_2K;
    uartParams.prcmDevId    = 0;

    if(uartParams.opMode == UART_OPMODE_POLLED)
    {
        printf(" SYSTEM: UART: POLLED Mode is Selected \n");
    }
    else if(uartParams.opMode == UART_OPMODE_INTERRUPT)
    {
        printf(" SYSTEM: UART: INTERRUPT Mode is Selected \n");
    }
    else
    {
        /* MISRA WARNING */
    }
    uartParams.enableCache = FALSE;

    /* initialise the edma library and get the EDMA handle */
    chanParams.hEdma = NULL;

    /* If cross bar events are being used then make isCrossBarIntEn = TRUE and
     * choose appropriate interrupt number to be mapped (assign it to
     * intNumToBeMapped)
     */
    chanParams.crossBarEvtParam.isCrossBarIntEn = FALSE;

    chanParams.crossBarEvtParam.intNumToBeMapped = 0xFF;

    ioParams.chanParams = (Ptr)&chanParams;

    GIO_addDevice(uartName, (Ptr)&Uart_IOMFXNS, NULL, devId, &uartParams);

    /* create the required channels(TX/RX) for the UART demo */
    uartTxHandle = GIO_create(uartName, GIO_OUTPUT, &ioParams, &eb);
    uartRxHandle = GIO_create(uartName, GIO_INPUT, &ioParams, &eb);

    if ((NULL == uartRxHandle) || (NULL == uartTxHandle))
    {
        printf(" SYSTEM: UART: ERROR: GIO_create(%s) Failed !!!\n", uartName);
    }
    else
    {
        InitDone = TRUE;
    }
}

/**
 *******************************************************************************
 *
 * \brief Prints the string in UART console , System_uartInit must succeed
 *        Before a call to this function
 *
 * \param  string     [IN] Input string
 *
 * \return  None
 *
 *******************************************************************************
 */
void uartPrint(char *string)
{
    size_t len;
    Int32    status  = IOM_COMPLETED;
    if(InitDone == TRUE)
    {
        len = strlen(string);

        /* Transmit the string*/
        status = GIO_write(uartTxHandle, string, &len);

        if (IOM_COMPLETED != status)
        {
            printf(" SYSTEM: UART: ERROR: GIO_write failed (status = %d) !!! \n",status);
        }
    }
    else
    {
        /*
        printf("\n System_uartInit needs to be called prior to any uartprint");
        */
    }
}

/**
 *******************************************************************************
 *
 * \brief Reads the Input message from the UART console
 *        System_uartInit needs to be called prior to any uarRead
 *
 * \param  pOption     [OUT] Character read from UART
 *
 * \return  None
 *
 *******************************************************************************
 */
Void uartRead(Int8 *pOption)
{
    Int32   nStatus  = IOM_COMPLETED;
    size_t  nLen    = 1u ;

    nStatus = GIO_read(uartRxHandle, &uartReadBuffer, &nLen);
    if (IOM_COMPLETED != nStatus)
    {
        printf(" SYSTEM: UART: ERROR: GIO_read failed (status = %d) !!! \n",nStatus);
    }

    /* copy only one char */
    *pOption = uartReadBuffer[nLen -1];
}
