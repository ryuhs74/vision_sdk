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
 *   \defgroup SELECT_LINK_API Select Link API
 *
 *
 *   SELECT Link can be used to select expected channel(s) data
 *   from input channels.It takes input buffers and selectively passes to the
 *   next link.
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file selectLink.h
 *
 * \brief Select link API public header file.
 *
 * \version 0.0 (Nov 2013) : [CM] First version
 *
 *******************************************************************************
 */


#ifndef _SELECT_LINK_H_
#define _SELECT_LINK_H_

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
 * \brief Max output queues to which a given SELECT link can connect
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
#define SELECT_LINK_MAX_OUT_QUE     (4)

/* @} */

/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup SELECT_LINK_API_CMD Select Link Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Link CMD: SELECT_LINK_CMD_SET_OUT_QUE_CH_INFO
 *
 *  Sets Information about which channels from input to be set at the output
 *
 * \param SelectLink_drvSetOutQueChInfo
 *
 *******************************************************************************
*/
#define SELECT_LINK_CMD_SET_OUT_QUE_CH_INFO         (0xB000)

/**
 *******************************************************************************
 *
 * \brief Link CMD: SELECT_LINK_CMD_GET_OUT_QUE_CH_INFO
 *
 * Information about which channels from input are selected at the output
 *
 * \param SelectLink_drvGetOutQueChInfo
 *
 *******************************************************************************
*/
#define SELECT_LINK_CMD_GET_OUT_QUE_CH_INFO         (0xB001)

/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 *  \brief SELECT_LINK_CMD_GET_OUT_QUE_CH_INFO command params
 *  Information about which channels from input are selected at the output
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
/**

*/
typedef struct
{
    UInt32 outQueId;
    /**< Que ID for which this is applicable
            - IGNORED when used with SelectLink_CreateParams */
    UInt32 numOutCh;
    /**< number of output channels in this output queue */

    UInt32 inChNum[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< input channel number which maps to this output queue */

} SelectLink_OutQueChInfo;

/**
 *******************************************************************************
 * \brief Selecet link configuration parameters.
 *
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 numOutQue;
    /**< Number of outPutQue queue's */

    System_LinkInQueParams     inQueParams;
    /**< Input queue information. */

    System_LinkOutQueParams   outQueParams[SELECT_LINK_MAX_OUT_QUE];
    /**< Output queue information */

    SelectLink_OutQueChInfo   outQueChInfo[SELECT_LINK_MAX_OUT_QUE];
    /**< Information about which channels from input are selected at the output */

    UInt32  notifyNextLink;
    /**< TRUE: send command to next link notifying that new data is ready in que */

} SelectLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
*******************************************************************************
*
* \brief Init function for SELECT link. This function does the following for
*   each SELECT link,
*  - Creates a task for the link
*  - Registers this link with the system
*
* \return  SYSTEM_LINK_STATUS_SOK
*
*******************************************************************************
*/
Int32 SelectLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for SELECT link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 SelectLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Select link set default parameters for create time params
 *   This function does the following
 *      - memset create params object
 *      - Sets notifyNextLink as TRUE
 * \param  pPrm  [OUT]  Select Link Create time Params
 *
 *******************************************************************************
 */
static inline void SelectLink_CreateParams_Init(SelectLink_CreateParams *pPrm)
{
    Int32 queId;

    memset(pPrm, 0, sizeof(*pPrm));
    pPrm->notifyNextLink = TRUE;

    /* Init Out Queue ID in order as this is the most likely way of using */
    for (queId = 0; queId < SELECT_LINK_MAX_OUT_QUE; queId++)
    {
        pPrm->outQueChInfo[queId].outQueId = queId;
    }
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/
