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
 * \file alg_create.c
 *
 * \brief This file contains a simple implementation of the
 *        ALG_create API operation
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
#include <stdlib.h>

#include <src/links_ipu/iva/codec_utils/src/_alg.h>

/*
 *  ======== ALG_create ========
 */
ALG_Handle ALG_create(IALG_Fxns * fxns, IALG_Handle p, IALG_Params * params)
{
    IALG_MemRec *memTab;
    Int n;
    ALG_Handle alg;
    IALG_Fxns *fxnsPtr;

    if (fxns != NULL)
    {
        n = fxns->algNumAlloc != NULL ? fxns->algNumAlloc() : IALG_DEFMEMRECS;

        if ((memTab = (IALG_MemRec *) malloc(n * sizeof(IALG_MemRec))))
        {

            n = fxns->algAlloc(params, &fxnsPtr, memTab);
            if (n <= 0)
            {
                return (NULL);
            }

            if (_ALG_allocMemory(memTab, n))
            {
                alg = (IALG_Handle) memTab[0].base;
                alg->fxns = fxns;
                if (fxns->algInit(alg, memTab, p, params) == IALG_EOK)
                {
                    free(memTab);
                    return (alg);
                }
                fxns->algFree(alg, memTab);
                _ALG_freeMemory(memTab, n);
            }

            free(memTab);
        }
    }

    return (NULL);
}

/*
 *  ======== ALG_delete ========
 */
Void ALG_delete(ALG_Handle alg)
{
    IALG_MemRec *memTab;
    Int n;
    IALG_Fxns *fxns;

    if (alg != NULL && alg->fxns != NULL)
    {
        fxns = alg->fxns;
        n = fxns->algNumAlloc != NULL ? fxns->algNumAlloc() : IALG_DEFMEMRECS;

        if ((memTab = (IALG_MemRec *) malloc(n * sizeof(IALG_MemRec))))
        {
            memTab[0].base = alg;
            n = fxns->algFree(alg, memTab);
            _ALG_freeMemory((IALG_MemRec *) memTab, n);

            free(memTab);
        }
    }
}

/* Nothing beyond this point */

