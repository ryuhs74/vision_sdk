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
 * \ingroup EP_LINK_API
 * \defgroup EP_LINK_IMPL EP Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file epLink_priv.h
 *
 * \brief Endpoint(EP) Link Private Header File
 *
 *        This file has the structures, enums, function prototypes
 *        for EP link, which are not exposed to the application
 *
 * \version 0.0 (May 2015) : [SM] First version
 *
 *******************************************************************************
 */

#ifndef _EP_LINK_PRIV_H_
#define _EP_LINK_PRIV_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/epLink.h>
#include <linux/src/osa/include/osa_tsk.h>
#include <linux/src/osa/include/osa_prf.h>
#include <vivi_plugin.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Maximum number of EP links to be used and stack sizes
 *          to be used.
 *
 *******************************************************************************
 */

#define EP_LINK_OBJ_MAX        (2)

/**
 *******************************************************************************
 * \brief size of EP link thread stack
 *******************************************************************************
 */
#define EP_LINK_TSK_STACK_SIZE (OSA_TSK_STACK_SIZE_DEFAULT)

/**
 *******************************************************************************
 * \brief size of EP link thread priority
 *******************************************************************************
 */
#define EP_LINK_TSK_PRI (OSA_THR_PRI_DEFAULT)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  EP Link Object
 *
 *******************************************************************************
*/
typedef struct {
    UInt32 linkId;
    /**< Link ID of this Link Obj */

    UInt32 instId;
    /**< Instance index of this link */

    OSA_TskHndl tsk;
    /**< Link task handle */

    System_LinkInfo linkInfo;
    /**< Current Link channel info */

    EpLink_CreateParams  createArgs;
    /**< create time arguments */

    struct ep_plugin_ctx ep_ctx;
    /**< endpoint's plugin context */

    int32_t (*post_buf) (struct ep_plugin_ctx*, System_Buffer*);
    /**< function pointer to post buffers to the vivi framework */

    OSA_LatencyStats  srcToEpSinkLatency;
    /**< Structure to find out min, max and average latency from
     *   source to this link
     */
} EpLink_obj;

extern EpLink_obj gEpLink_obj[];

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/* Common across source and sink */
Int32 EpLink_getLinkInfo(Void *pTsk, System_LinkInfo *info);
Int32 EpLink_drvPutEmptyBuffers(EpLink_obj *pObj, System_BufferList *pBufList);
Int32 EpLink_drvGetFullBuffers(EpLink_obj *pObj, System_BufferList *pBufList);
Int32 EpLink_drvCreateQueHandle(EpLink_obj *pObj, OSA_MsgHndl *pMsg);

/* Sink specific */
Int32 EpLink_drvSinkCreate(EpLink_obj *pObj);
Int32 EpLink_drvSinkProcessBuffers(EpLink_obj *pObj);
Int32 EpLink_drvSinkPutEmptyBuffers(EpLink_obj *pObj, System_BufferList *pBufList);
/* Source specific */

#endif

/* @} */
