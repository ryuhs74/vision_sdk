/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _ALG_IVISION_H_
#define _ALG_IVISION_H_

#include <include/link_api/system.h>
#include <ivision.h>

Void *AlgIvision_create(const IVISION_Fxns *fxns, IALG_Params *pAlgPrms);

Int32 AlgIvision_delete(Void *algHandle);

Int32 AlgIvision_process(Void *algHandle,
        IVISION_InBufs *inBufs,
        IVISION_OutBufs *outBufs,
        IVISION_InArgs *inArgs,
        IVISION_OutArgs *outArgs);

Int32 AlgIvision_control(Void *algHandle,
                    IALG_Cmd cmd,
                    const IALG_Params *inParams,
                    IALG_Params *outParams);

#endif /* _ALG_IVISION_H_ */

