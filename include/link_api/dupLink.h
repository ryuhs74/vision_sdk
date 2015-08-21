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
 *   \defgroup DUP_LINK_API Dup Link API
 *
 *
 *   Dup link is a connector link. What it all does is takes an input buffer
 *   and duplicate it across 'N' output links. Release buffers to its previous
 *   link only when all 'N' links connected to this link release the duplicated
 *   buffers
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file dupLink.h
 *
 * \brief Dup link API public header file.
 *
 * \version 0.0 (Jul 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _DUP_LINK_H_
#define _DUP_LINK_H_

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
 * \brief Max output queues supported by DUP Link
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define DUP_LINK_MAX_OUT_QUE    (SYSTEM_MAX_OUT_QUE)

/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
*/


/**
 *******************************************************************************
 * \brief DUP link configuration parameters.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */

    UInt32  numOutQue;
    /**< Number of output queues */

    System_LinkOutQueParams   outQueParams[DUP_LINK_MAX_OUT_QUE];
    /**< output queue information */

    UInt32  notifyNextLink;
    /**< TRUE: send command to next link notifying that new data is ready in que */

} DupLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Init function for DUP link. This function does the following for each
 *   DUP link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 DupLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for DUP link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 DupLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Dup link set default parameters for create time params
 *   This function does the following
 *      - memset create params object
 *      - Sets notifyNextLink as TRUE
 * \param  pPrm  [OUT]  DupLink Create time Params
 *
 *******************************************************************************
 */
static inline void DupLink_CreateParams_Init(DupLink_CreateParams *pPrm)
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

