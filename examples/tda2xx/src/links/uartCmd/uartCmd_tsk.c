/**
 *                                                                            
 * Copyright (c) 2015 CAMMSYS - http://www.cammsys.net/                       
 *                                                                            
 * All rights reserved.                                                       
 *                                                                            
 * @file	uartInput_tsk.c
 * @author  Raven
 * @date	Sep 21, 2015
 * @brief	                                                                     
 */

#include <examples/tda2xx/include/chains.h>
#include <examples/tda2xx/include/chains_common.h>
#include <src/utils_common/include/utils_prcm_stats.h>
#include <src/utils_common/include/utils_uart.h>

#pragma DATA_ALIGN(UartCmd_tskStack, 32)
#pragma DATA_SECTION(UartCmd_tskStack, ".bss:taskStackSection")
UInt8 UartCmd_tskStack[1024*4];


BspOsal_TaskHandle tsk;
uint8_t rxBuf[128];

Void UartCmd_tsk_main(UArg arg0, UArg arg1)
{
	for(;;)
	{
		//UtillUartRead(UART_CH_MICOM,rxBuf,1);
		size_t len = 9;
		UtillUartWrite(UART_CH_MICOM,"testMSG\r\n",&len);
        BspOsal_sleep(100);
	}
}


Int32 UartCmd_tsk_init()
{
	tsk = BspOsal_taskCreate(
			(BspOsal_TaskFuncPtr)UartCmd_tsk_main,
			1,
			UartCmd_tskStack,
			sizeof(UartCmd_tskStack),
			NULL);

    return 0;
}


Int32 UartCmd_tsk_deInit()
 {

    return 0;
 }
