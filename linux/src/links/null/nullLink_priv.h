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
 * \ingroup NULL_LINK_API
 * \defgroup NULL_LINK_IMPL Null Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file nullLink_priv.h Null Link private API/Data structures
 *
 * \brief  This file is a private header file for null link implementation
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of null link.
 *
 * \version 0.0 (Jun 2014) : [NN] First version ported to Linux
 *
 *******************************************************************************
 */

#ifndef _NULL_LINK_PRIV_H_
#define _NULL_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <linux/src/osa/include/osa.h>
#include <linux/src/osa/include/osa_tsk.h>
#include <linux/src/osa/include/osa_mem.h>
#include <include/link_api/nullLink.h>
#include <linux/src/system/system_priv_common.h>
#include <stdio.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Size NULL link stack
 *******************************************************************************
 */
#define NULL_LINK_TSK_STACK_SIZE (OSA_TSK_STACK_SIZE_DEFAULT)


 /**
 *******************************************************************************
 *
 * \brief Maximum number of null link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define NULL_LINK_OBJ_MAX    (2)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Structure to hold all Dup link related information
 *
 *******************************************************************************
 */

typedef struct {
    UInt32 tskId;
    /**< Placeholder to store null link task id */

    OSA_TskHndl tsk;
    /**< Handle to null link task */

    NullLink_CreateParams createArgs;
    /**< Create params for null link */

    System_LinkQueInfo  inQueInfo[NULL_LINK_MAX_IN_QUE];
    /**< Input Queue info */

    UInt32 recvCount;
    /**< Count of buffers received */

    FILE *fpDataStream;
    /**< Binary File for the bitsream stream data.*/

} NullLink_Obj;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
