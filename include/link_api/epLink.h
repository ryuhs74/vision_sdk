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
 * \file epLink.h
 *
 * \brief Endpoint Link API
 *
 * \version 0.0 (May 2015) : [SM] First version
 *
 *******************************************************************************
 */

#ifndef _EP_LINK_H_
#define _EP_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>

/**
 *******************************************************************************
 *
 * \brief Endpoint Link specific defines
 *
 *******************************************************************************
 */
#define VIVI_MAX_NAME (100)

/**
 *******************************************************************************
 *
 * \brief Endpoint Link specific commands
 *
 *******************************************************************************
 */

/**
 * Meant for the source eplink
 * TODO define parameters
 */
#define EP_CMD_PUT_BUF              ((SYSTEM_LINK_ID_EP_0<<4) + 1)

/**
 * This is to create a thread specific handle for the buffer que
 *
 * \param struct ep_buf_que: to let the link know about the que name and the
 *                           function pointer to invoke for posting buffers into
 *                           the queue.
 *
 * \return SYSTEM_STATUS_SOK: on success
 */
#define EP_CMD_CREATE_QUE_HANDLE    ((SYSTEM_LINK_ID_EP_0<<4) + 2)

/**
 * Meant for the source eplink
 * TODO define parameters
 */
#define EP_CMD_CONFIG_SOURCE        ((SYSTEM_LINK_ID_EP_0<<4) + 3)

/**
 *******************************************************************************
 *
 * \brief Endpoint Link create parameters
 *
 *******************************************************************************
 */
typedef struct {

} EpLink_ConfigSource;

typedef struct {
    System_LinkInQueParams   inQueParams;
    /**< Input queue information
     */

    System_LinkOutQueParams  outQueParams;
    /**< Output queue information
     */

    UInt32 chainId;
    /**< Chain id, it belongs to
     */

    char plugName[VIVI_MAX_NAME];
    /**< Plugin it belongs to
     */

    UInt16 epType;
    /**< Type of endpoint - source or sink
     */

    EpLink_ConfigSource srcConfig;
    /**< Configuration required when acting as source
     */

} EpLink_CreateParams;

/**
 *******************************************************************************
 *
 *   \brief Endpoint link register and init function
 *
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 EpLink_init();
Int32 EpLink_deInit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
