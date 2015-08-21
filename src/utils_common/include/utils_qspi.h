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
 * \ingroup UTILS_API
 * \defgroup UTILS_QSPI_API QSPI related utilities
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_uart.h
 *
 * \brief  QSPI related utilities
 *
 * \version 0.0 (Dec 2013) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef UTILS_QSPI_H
#define UTILS_QSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
#define SYSTEM_QSPI_READ_WRITE_SIZE  (256)
#define SYSTEM_QSPI_FLASH_BLOCK_SIZE (64*1024)

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */
Void System_qspiInit(void);
/* Length must be multiple of 256 bytes */
Int32 System_qspiReadSector(UInt32 dstAddr, UInt32 srcOffsetAddr, Int32 length);
/* dstOffsetAddr must be 64KB aligned & length must be multiple of 64k bytes */
Int32 System_qspiWriteSector(UInt32 dstOffsetAddr, UInt32 srcAddr, Int32 length);
Void System_qspiEraseFull(void);
Int32 System_qspiEraseSector(UInt32 dstOffsetAddr, Int32 length);

#ifdef __cplusplus
}
#endif

#endif

/* @} */
