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
 * \file alg_control.c
 *
 * \brief ALG_control implementation.  This is common to all implementations
 *         of the ALG module.
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <xdc/std.h>
#include <src/links_ipu/iva/codec_utils/src/alg.h>
#include <ti/xdais/ialg.h>

/*
 *  ======== ALG_control ========
 */
Int ALG_control(ALG_Handle alg, IALG_Cmd cmd, IALG_Status * statusPtr)
{
    if (alg && alg->fxns->algControl)
    {
        return (alg->fxns->algControl(alg, cmd, statusPtr));
    }

    return (IALG_EFAIL);
}

/* Nothing beyond this point */

