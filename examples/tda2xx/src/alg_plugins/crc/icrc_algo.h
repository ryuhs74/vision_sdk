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
 * \file icrc_algo.h
 *
 * \brief Interface file for CRC algorithm on IPU1_0
 *
 *        This CRC algorithm runs on CRC HW of TDA3x SOC,
 *        So this is supported only on TDA3x
 *
 * \version 0.0 (May 2015) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ICRC_ALGO_H_
#define _ICRC_ALGO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <crc.h>
#include <hw_core_cm_core.h>
#include <soc.h>
#include <hw_types.h>
#include "src/utils_common/include/utils_dma.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing the CRC create time parameters
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                startX;
    /**< X-Coordinate position of the frame */
    UInt32                startY;
    /**< Y-Coordinate position of the frame  */
    UInt32                roiHeight;
    /**< ROI height of the frame */
    UInt32                roiWidth;
    /**< ROI width of the frame */
    UInt32                dataFormat;
    /**< ROI width of the frame */

} Alg_CrcCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure for CRC algoirthm object
 *
 *******************************************************************************
*/
typedef struct
{
    Alg_CrcCreateParams   crcPrms;
    UInt32                baseAddr;
    UInt32                watchdogPreloadVal;
    UInt32                blockPreloadVal;
    UInt32                sectCnt;
    UInt32                patternSize;
    crcChannel_t          chNumber;
    crcOperationMode_t    mode;
    crcSignature_t        sectSignVal;
    crcSignatureRegAddr_t psaSignRegAddr;

} Alg_Crc_Obj;

/**
 *******************************************************************************
 *
 *   \brief Structure for CRC & EDMA algoirthm object
 *
 *******************************************************************************
*/
typedef struct
{
    Alg_Crc_Obj crcObj;
    /**< Base CRC object */
    unsigned int edmaChId;
    /**< EDMA CH ID that is used for this EDMA */
    unsigned int tccId;
    /**< EDMA TCC ID that is used for this EDMA */
    EDMA3_DRV_Handle hEdma;
    /**< Handle to EDMA controller associated with this logical DMA channel */
    EDMA3_DRV_PaRAMRegs *pParamSet;
    /**< Pointer to physical area of PaRAM for this channel */

} Alg_CrcDma_Obj;

/**
 *******************************************************************************
 *
 *   \brief Structure containing the CRC control parameters
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                xxx;
    /**< Any parameters which can be used to alter the algorithm behavior */

} Alg_CrcControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32 Alg_CrcCreate (Alg_CrcDma_Obj *algHandle,
                     Alg_CrcCreateParams *pCreateParams);

Int32 Alg_CrcProcess (Alg_CrcDma_Obj *algHandle,
                          UInt32 *inPtr[],
                          UInt32  inWidth,
                          UInt32  inHeight,
                          UInt32  inPitch[],
                          UInt32  dataFormat
                         );

Int32 Alg_CrcControl (Alg_CrcDma_Obj *algHandle,
                      Alg_CrcControlParams *pControlParams);

Int32 Alg_CrcStop (Alg_CrcDma_Obj *algHandle);

Int32 Alg_CrcDelete (Alg_CrcDma_Obj *algHandle);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* Nothing beyond this point */
