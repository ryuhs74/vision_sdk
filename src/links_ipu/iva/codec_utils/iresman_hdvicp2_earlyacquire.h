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
 * \file iresman_hdvicp2_earlyacquire.h
 *
 * \brief  HDVICP TiledMemory config functions are defined
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _IRESMAN_HDVICP2_EARLY_ACQUIRE_H_
#define _IRESMAN_HDVICP2_EARLY_ACQUIRE_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/ires.h>

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */
IRES_Status IRESMAN_TiledMemoryForceDisableTileAlloc_Register(IALG_Handle algHandle);
IRES_Status IRESMAN_TiledMemoryForceDisableTileAlloc_UnRegister(IALG_Handle algHandle);
Int32 Utils_encdec_checkResourceAvail(IALG_Handle alg, IRES_Fxns * resFxns,
                                      FVID2_Format *pFormat, UInt32 numFrames,
                                      IRES_ResourceDescriptor resDesc[]);

#endif

/* Nothing beyond this point */


