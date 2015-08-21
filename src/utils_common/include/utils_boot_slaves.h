/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup UTILS_API
 * \defgroup UTILS_BOOT_SLAVES_API Slave Boot APIs
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_boot_slaves.h
 *
 * \brief  APIs to use SBL LIB to load and run slaves
 *
 * \version 0.0 (Jun 2015) : [YM] First version
 *
 *******************************************************************************
 */

#ifndef UTILS_BOOT_SLAVES_H
#define UTILS_BOOT_SLAVES_H

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
/**
 *******************************************************************************
 *
 * \brief Default qspi offset for UcLate AppImages
 *
 * SUPPORTED on TDA3x
 *
 *******************************************************************************
*/
#define SBLLIB_APP_IMAGE_LATE_OFFSET_QSPI 0xA80000

/*******************************************************************************
 *  Structures
 *******************************************************************************
 */

typedef struct
{
    UInt32 offset;
    /**< offset of UcLate multicore appImage in the memory */

    Bool useEdma;
    /**< if not true uses memcpy to copy secions to DDR */

}Utils_BootSlaves_Params;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Reads a Multi-core AppImage from given offset, parses it for entry
 *        points and sections to be loaded, loads and runs slave core
 *        using SBL LIB
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 Utils_bootSlaves(Utils_BootSlaves_Params *params);

/**
 *******************************************************************************
 *
 * \brief Syncs up with all cores after boot up is complete and completes ipc
 *        set up
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Void Utils_syncSlaves();

/**
 *******************************************************************************
 *
 * \brief Initializes basic create time params
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
static inline Void Utils_bootSlaves_paramsInit(Utils_BootSlaves_Params *params)
{
    params->offset  = SBLLIB_APP_IMAGE_LATE_OFFSET_QSPI;
    params->useEdma = TRUE;
}


#ifdef __cplusplus
}
#endif

#endif

/* @} */
