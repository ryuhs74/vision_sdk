/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup UTILS_API
 * \defgroup UTILS_CBUF_OCMC OCMC CBUF APIs
 *
 * \brief This module define APIs for allocating memory from ocmc region as
 * circular buffers
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_cbuf_ocmc.h
 *
 * \brief OCMC CBUF APIs
 *
 * \version 0.0 (July 2014) : [VT] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_CBUF_OCMC_H_
#define _UTILS_CBUF_OCMC_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <src/utils_common/include/utils.h>
#include <ocmc_ecc_l1.h>
#include <ocmc_ecc_l2.h>



/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */


/**
 *******************************************************************************
 *  \brief OCMC Instance ID - There are three OCMC RAMS available
 * The OCM subsystem consists of three OCM Controllers (OCMC)
 * The RAM associated controllers are as follows:
 * OCMC_RAM1 - 512 KiB
 * OCMC_RAM2 - 1024 KiB
 * OCMC_RAM3 - 1024 KiB
 *******************************************************************************
 */
typedef enum
{
    UTILS_OCMC_RAM1 = 0,

    UTILS_OCMC_RAM2,

    UTILS_OCMC_RAM3
}Utils_OcmcInstanceId;


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32 Utils_cbufOcmcInit(Utils_OcmcInstanceId ocmcInstId);

Ptr Utils_cbufOcmcAlloc(Utils_OcmcInstanceId ocmcInstId,
                UInt32 bpp,UInt32 width, UInt32 height, UInt32 numLinesPerSlice,
                UInt32 numSlicesPerCbuf);

Int32 Utils_cbufOcmcFree(Utils_OcmcInstanceId ocmcInstId, Ptr addr);

Int32 Utils_cbufOcmcDeInit(Utils_OcmcInstanceId ocmcInstId);

#endif /* ifndef _UTILS_CBUF_OCMC_H_ */

/* @} */
