/**
 *                                                                            
 * Copyright (c) 2015 CAMMSYS - http://www.cammsys.net/                       
 *                                                                            
 * All rights reserved.                                                       
 *                                                                            
 * @file	error_monitor_tsk.c
 * @author  Raven
 * @date	Sep 21, 2015
 * @brief	                                                                     
 */

#include <examples/tda2xx/include/chains.h>
#include <examples/tda2xx/include/chains_common.h>
#include <src/utils_common/include/utils_prcm_stats.h>
#include <src/utils_common/include/utils_uart.h>
#include "error_monitor_priv.h"
#include <common/bsp_GpioDefineAVM_E500.h>
#include <devices/bsp_device.h>

#pragma DATA_ALIGN(Error_MonitorStack, 32)
#pragma DATA_SECTION(Error_MonitorStack, ".bss:taskStackSection")
static UInt8 Error_MonitorStack[1024*4];

static BspOsal_TaskHandle tsk;

static int8_t des_status_lflt[5] = {0};
static int8_t des_status_lflt_bak[5] = {0};
static int8_t des_status_error[5] = {0};
static int8_t des_status_error_bak[5] = {0};
static int8_t des_status_lock[5] = {0};
static int8_t des_status_lock_bak[5] = {0};


static Void Error_Monitor_main(UArg arg0, UArg arg1)
{

	for(;;)
	{	Uint32 i=0;
		for(i=0; i< BSP_DEVICE_ISX016_INST_ID_4+1; i++)
		{
			Uint8 state = 0;
			des_status_lflt[i] = Bsp_boardGetDesStatusLFLT(i,&state);
			if(des_status_lflt_bak[i]!=des_status_lflt[i])
			{

				Vps_printf("LFLT Pin Changed ID[%d] [%d]-> [%d] State[%X]\n", i, des_status_lflt_bak[i], des_status_lflt[i],state);
				des_status_lflt_bak[i]=des_status_lflt[i];
			}
		}
		for(i=0; i< BSP_DEVICE_ISX016_INST_ID_4+1; i++)
		{
			des_status_error[i] = Bsp_boardGetDesStatusERROR(i);
			if(des_status_error_bak[i]!=des_status_error[i])
			{

				Vps_printf("ERROR Pin Changed ID[%d] [%d]-> [%d]\n", i, des_status_error_bak[i], des_status_error[i]);
				des_status_error_bak[i]=des_status_error[i];
			}
		}
		for(i=0; i< BSP_DEVICE_ISX016_INST_ID_4+1; i++)
		{
			des_status_lock[i] = Bsp_boardGetDesStatusLOCK(i);
			if(des_status_lock_bak[i]!=des_status_lock[i])
			{

				Vps_printf("lock Pin Changed ID[%d] [%d]-> [%d]\n", i, des_status_lock_bak[i], des_status_lock[i]);
				des_status_lock_bak[i]=des_status_lock[i];
			}
		}
        BspOsal_sleep(200);
	}
}


Int32 Error_Monitor_init()
{
	tsk = BspOsal_taskCreate(
			(BspOsal_TaskFuncPtr)Error_Monitor_main,
			1,
			Error_MonitorStack,
			sizeof(Error_MonitorStack),
			NULL);

    return 0;
}


Int32 Error_Monitor_deInit()
 {
	BspOsal_taskDelete(tsk);
    return 0;
 }
