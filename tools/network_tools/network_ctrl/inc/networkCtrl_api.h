 /*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _NETWORK_CTRL_API_H_
#define _NETWORK_CTRL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <osa.h>
#include <networkCtrl_if.h>


Int32 NetworkCtrl_init();
Int32 NetworkCtrl_deInit();

Int32 NetworkCtrl_readParams(UInt8 *pPrm, UInt32 prmSize);
Int32 NetworkCtrl_writeParams(UInt8 *pPrm, UInt32 prmSize);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


