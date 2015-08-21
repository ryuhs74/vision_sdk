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
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup GATE_LINK_API Gate Link API
 *
 *
 *   Gate link allows usecase writer to have runtime control on part of a data
 *   flow. It acts as a switch which can be turned on/off. Based on the state
 *   of the link it decides to either forward or return data.
 *
 *   This link can have many applications, some of them are as mentioned below
 *   1. Power Management - Selectively turn off and turn on cores based on their
 *      requirement for the usecase at run time.
 *   2. Boot time optimization - Usecase can be devided in to UcEarly and UcLate
 *      Basically UcEarly is created & started with gateLinks off, UcLate is
 *      created later in the course and then gateLinks can be switch on.
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file gateLink.h
 *
 * \brief Gate link API public header file.
 *
 * \version 0.0 (Apr 2015) : [YM] First version
 *
 *******************************************************************************
 */

#ifndef _GATE_LINK_H_
#define _GATE_LINK_H_

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
 *   \brief Link CMD: Run time Command to get to switch operation mode to ON
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define GATE_LINK_CMD_SET_OPERATION_MODE_ON                       (0x8000)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Run time Command to get to switch operation mode to OFF
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define GATE_LINK_CMD_SET_OPERATION_MODE_OFF                      (0x8001)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Run time Command to get to value of bufCount, application
 *                    essentially polls using this command before deleteing
 *                    instance of previous or next link. When bufCount is zero
 *                    it is safe to delete the prev / next link instance
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define GATE_LINK_CMD_GET_BUFFER_FORWARD_COUNT                    (0x8002)


/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
*/

/**
 *******************************************************************************
 * \brief GATE link configuration parameters.
 *
 *******************************************************************************
*/
typedef struct
{
    System_LinkInQueParams        inQueParams;
    /**< Input queue information */

    System_LinkOutQueParams       outQueParams;
    /**< output queue information */

    UInt32                        prevLinkIsCreated;
    /**< TRUE: no need to set 'prevLinkInfo'
        FALSE: user needs to set 'prevLinkInfo'
      */

    System_LinkInfo               prevLinkInfo;
    /**< Previous link info of the link which cab be instantiated later
         in the course of execution or can be existing at time of creation
         of the data flow */

} GateLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Init function for GATE link. This function does the following for each
 *   GATE link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 GateLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for GATE link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 GateLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Gate link set default parameters for create time params
 *
 * \param  pPrm  [OUT]  GateLink Create time Params
 *
 *******************************************************************************
 */
static inline void GateLink_CreateParams_Init(GateLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->prevLinkIsCreated = TRUE;

    return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/

