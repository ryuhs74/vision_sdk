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
 * \ingroup UTILS_API
 * \defgroup UTILS_UART_API UART related utilities
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_uart.h
 *
 * \brief  UART related utilities
 *
 * \version 0.0 (Jun 2013) : [NN] First version
 * \version 0.1 (Jul 2013) : [NN] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef UART_INIT_H
#define UART_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

typedef enum
{
	UART_CH_MICOM =	0,
	UART_CH_DEBUG =	1,
	UART_CH_MAX	  =	2
}Utils_UART_Channel;

 /**
 *******************************************************************************
 *
 * \brief Size of UART Buffer
 *
 *******************************************************************************
*/
#define UART_BUFFER_SIZE   3000

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Void System_uartInit();
Bool System_isUartInitDone();
Void uartPrint(char *string);
Void uartRead(Int8 *pOption);

Int32 UtillUartRead(Utils_UART_Channel channel, uint8_t* rdBuf, size_t* nLen);
Int32 UtillUartWrite(Utils_UART_Channel channel, uint8_t* wrBuf, size_t* nLen);

#ifdef __cplusplus
}
#endif

#endif

/* @} */
