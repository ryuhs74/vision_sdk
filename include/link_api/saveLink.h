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

#ifndef _SAVE_LINK_H_
#define _SAVE_LINK_H_

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
#define SAVE_LINK_MAX_OUT_QUE    (1)
#define SAVE_LINK_MAX_IN_QUE	(1)

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
typedef struct //ryuhs74@20151027 - 정의 해야 한다.
{
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */

    //UInt32  numOutQue;
    /**< Number of output queues */

    //System_LinkOutQueParams   outQueParams[DUP_LINK_MAX_OUT_QUE]; //Save Link는 Next Link 없기 때문에 OutQue 필요 없음
    /**< output queue information */

    //UInt32  notifyNextLink;
    /**< TRUE: send command to next link notifying that new data is ready in que */

} SaveLink_CreateParams;


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
Int32 SaveLink_init();

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
Int32 SaveLink_deInit();

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
static inline void SaveLink_CreateParams_Init(SaveLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));
    memset(&pPrm->inQueParams, 0, sizeof(pPrm->inQueParams));
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/

