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
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup MERGE_LINK_API Merge Link API
 *
 *
 *   Merge link is a connector link. What it does is taking input buffers from
 *   multiple input links and merge them to single output link. Release buffers
 *   to its previous links when the link connected to this merge link release
 *   the corresponding input buffers
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file mergeLink.h
 *
 * \brief Merge link API public header file.
 *
 * \version 0.0 (Aug 2013) : [SL] First version
 *
 *******************************************************************************
 */

#ifndef _MERGE_LINK_H_
#define _MERGE_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /* @{ */

/**
 *******************************************************************************
 *
 * \brief Max input queues supported by Merge Link
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define MERGE_LINK_MAX_IN_QUE	(6)

/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
*/


/**
 *******************************************************************************
 * \brief MERGE link configuration parameters.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 numInQue;
    /**< Number of inputs queue's */

    System_LinkInQueParams   inQueParams[MERGE_LINK_MAX_IN_QUE];
    /**< Input queue information */

    System_LinkOutQueParams   outQueParams;
    /**< Output queue information */

    UInt32  notifyNextLink;
    /**< TRUE: send command to next link notifying that new data is ready in que */

} MergeLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
*******************************************************************************
*
* \brief Init function for MERGE link. This function does the following for
*   each MERGE link,
*  - Creates a task for the link
*  - Registers this link with the system
*
* \return  SYSTEM_LINK_STATUS_SOK
*
*******************************************************************************
*/
Int32 MergeLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for MERGE link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 MergeLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Merge link set default parameters for create time params
 *   This function does the following
 *      - memset create params object
 *      - Sets notifyNextLink as TRUE
 * \param  pPrm  [OUT]  MergeLink Create time Params
 *
 *******************************************************************************
 */
static inline void MergeLink_CreateParams_Init(MergeLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
    pPrm->notifyNextLink = TRUE;
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/
