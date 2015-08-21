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
 * \ingroup SYSTEM_IMPL
 *
 * \defgroup SYSTEM_EVE_IMPL System implementation for EVE Core 0
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file system_priv_eve.h EVE private file containing all the header files
 *                            util files required by EVE
 *
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
 */
#ifndef _SYSTEM_PRIV_EVE_H_
#define _SYSTEM_PRIV_EVE_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_common/system/system_priv_common.h>
#include <include/link_api/system.h>
#include <include/link_api/systemLink_eve.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/dupLink.h>
#include <include/link_api/gateLink.h>
#include <include/link_api/selectLink.h>
#include <include/link_api/syncLink.h>
#include <include/link_api/mergeLink.h>

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure to hold the EVE global objects.
 *          Any Link in the EVE core can use these objects.
 *
 *******************************************************************************
*/
typedef struct {
    UInt32 eveIdleInitDone;
    /* Eve cpu idle preparation done */
    UInt32 evePowerMode;
    /* EVE CPU Idle Power Mode */
    UInt32 evePreIdleTimeStampTaken;
    /* Flag to indicate that the pre Idle Time stamp has been taken */
    UInt32 reserved;

} System_EveObj;

extern System_EveObj gSystem_objEve;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
