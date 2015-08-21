/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *
 * \file chains_main_srv_calibration.h
 *
 * \brief This file contains common utility functions used by all use-cases
 *
 *******************************************************************************
 */

#ifndef _CHAIN_MAIN_SRV_CALIBRATION_H_
#define _CHAIN_MAIN_SRV_CALIBRATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <include/link_api/algorithmLink_geometricAlignment.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief GA ouput calibration table and perspective matrix sizes and
 *        their respective offsets in the flash memory
 *        Use a 4 byte Magic sequence in the begining of the both these
 *        tables to check the validity of tables before use
 *******************************************************************************
 */


#define GA_OUTPUT_LUT_FLASHMEM_OFFSET         (01*1024*1024)
#define GA_OUTPUT_LUT_SIZE                    (10*1024*1024)
#define GA_OUTPUT_LUT_SIZE_MEM_ALLOC          (20*1024*1024)

#define GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET (GA_OUTPUT_LUT_FLASHMEM_OFFSET + GA_OUTPUT_LUT_SIZE)
#define GA_PERSPECTIVE_MATRIX_SIZE            (64*1024)

#define GA_MAGIC_PATTERN_SIZE_IN_BYTES        (4)
#define GA_OUTPUT_LUT_MAGIC_SEQUENCE          (0x1234ABCD)
#define GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE  (0xABCD1234)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Enum for various GA calibration modes (Calibration ON/OFF)
 *
 *******************************************************************************
*/
typedef enum
{
    CHAIN_COMMON_SRV_GA_CALIBRATION_NO = 0,
    CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE,
    CHAIN_COMMON_SRV_GA_ERASE_ENTIRE_TABLE,
    CHAIN_COMMON_SRV_GA_CALIBRATION_MAXNUM,
    CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */
}Chain_Common_SRV_GACalibrationType;

/**
 *******************************************************************************
 * \brief Enum for various GA calibration states
 *
 *******************************************************************************
*/
typedef enum
{
    CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_CREATETIME = 0,
    CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_RUNTIME,
    CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_DELETETIME,
    CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_MAXNUM,
    CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */
}Chain_Common_SRV_GACalibrationStates;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Geometric Alignment
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    Chain_Common_SRV_GACalibrationStates calibState;
    /**< variable to pass various GA calibration states */
    UInt32 startWithCalibration;
    /**< flag to specify the resetState in create time */
    AlgorithmLink_GAlignCalibrationMode calMode;
    /**< Calibration mode to be used */
    UInt32 gaLUTAddr;
    /**< DDR pointer for GALUT */
    UInt32 persMatAddr;
    /**< DDR pointer for perspective matrix */
    Chain_Common_SRV_GACalibrationType  calType;
    /**< Calibration mode to be used */

} Chain_Common_SRV_CalibParams;

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */
Void Chain_Common_SRV_Calibration(Chain_Common_SRV_CalibParams * calInfo);

/* Is this path valid for both NFS and SD as storage media? */
#define CALIBDATA_FILENAME "/home/root/.calibtable"

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

/* @} */

/**
 *******************************************************************************
 *
 *   \defgroup EXAMPLES_API Example code implementation
 *
 *******************************************************************************
 */

