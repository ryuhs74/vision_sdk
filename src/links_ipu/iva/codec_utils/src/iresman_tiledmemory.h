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
 * \file iresman_tiledmemory.h
 *
 * \brief  HDVICP resource register/Unregister with RAMN functions are defined
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _IRESMAN_TILEDMEMORY_H_
#define _IRESMAN_TILEDMEMORY_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/ires.h>

IRES_Status IRESMAN_TiledMemoryResourceRegister();
IRES_Status IRESMAN_TiledMemoryResourceUnregister();
IRES_Status IRESMAN_TILEDMEMORY_checkResourceAvail(IALG_Handle alg, UInt32 *size,
                                            IRES_Fxns * resFxns,
                                            IRES_ResourceDescriptor resDesc[]);

#endif

/* Nothing beyond this point */

