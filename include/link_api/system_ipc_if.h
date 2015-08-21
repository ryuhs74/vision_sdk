/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_IPC_IF_H_
#define _SYSTEM_IPC_IF_H_

 /**
 *******************************************************************************
 *
 * \file system_ipc_if.h IPC Data structure interface
 *
 * \brief  Data structures that are exchanged across CPUs
 *         during IPC
 *
 *******************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/system_inter_link_api.h>

/*******************************************************************************
 *  Define's
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Max size of message send via System_linkControl API
 *******************************************************************************
 */
#define SYSTEM_IPC_MSG_SIZE_MAX     (4*1024)

/**
 *******************************************************************************
 * \brief Max possible IPC OUT link instances on a given CPU
 *******************************************************************************
 */
#define SYSTEM_IPC_OUT_LINK_MAX     (10)

/**
 *******************************************************************************
 * \brief Max number of elements in IPC Out Link queue
 *******************************************************************************
 */
#define SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS        (10)

/**
 *******************************************************************************
 * \brief Strcuture that is exchange across CPUs when sending message via
 *        System_linkControl API
 *******************************************************************************
 */
typedef struct {

    uint32_t payload[SYSTEM_IPC_MSG_SIZE_MAX/sizeof(uint32_t)];
    /**< Payload of message being sent to a given CPU */
} System_IpcMsg;

/**
 *******************************************************************************
 * \brief Shared memory structure that holds message structures for all CPUs
 *******************************************************************************
 */
typedef struct {

    System_IpcMsg procMsg[SYSTEM_PROC_MAX];
    /**< Per CPU message structure */

} System_IpcMsgSharedMemObj;

/**
 *******************************************************************************
 * \brief Data structure representing IPC Queue header in shared memory
 *
 *        Typically user does not need to know internals of this
 *        data structure
 *******************************************************************************
*/
typedef struct {

  volatile uint32_t curRd;
  /**< Current read index */

  volatile uint32_t curWr;
  /**< Current write index  */

  volatile uint32_t elementSize;
  /**< Size of individual element in units of bytes */

  volatile uint32_t maxElements;
  /**< Max elements that be present in the queue  */

} System_IpcQueHeader;

/**
 *******************************************************************************
 * \brief Data structure representing IPC Queue in shared memory including its
 *        queue memory
 *******************************************************************************
*/
typedef struct {

    System_IpcQueHeader queHeader;
    /**< IPC Queue header */

    uint32_t            queMem[SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS];
    /**< Memory associated with this queue */

} System_IpcQueObj;

/**
 *******************************************************************************
 * \brief Data structure all IPC Queue's on a given CPU including the memory
 *        associated with System_IpcBuffers that are exchanged
 *******************************************************************************
*/
typedef struct {

    System_IpcQueObj    queOut2InObj[SYSTEM_IPC_OUT_LINK_MAX];
    /**< IPC Queue objects for each IPC Out link on a given CPU */

    System_IpcQueObj    queIn2OutObj[SYSTEM_IPC_OUT_LINK_MAX];
    /**< IPC Queue objects for each IPC Out link on a given CPU */

    System_IpcBuffer    queElements[SYSTEM_IPC_OUT_LINK_MAX][SYSTEM_IPC_OUT_LINK_IPC_QUE_MAX_ELEMENTS];
    /**< IPC Buffer elements for each IPC Out link on a given CPU */

} System_IpcQueProcObj;

/**
 *******************************************************************************
 * \brief Data structure representing all IPC Queue's on a all CPU's
 *******************************************************************************
*/
typedef struct {

    System_IpcQueProcObj    ipcQueProcObj[SYSTEM_PROC_MAX];

} System_IpcQueSharedMemObj;


/**
 *******************************************************************************
 * \brief Data structure representing all IPC information across all CPUs
 *******************************************************************************
*/
typedef struct {

    System_IpcQueSharedMemObj ipcQueObj;
    System_IpcMsgSharedMemObj ipcMsgObj;

} System_IpcSharedMemObj;

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief Get pointer to message payload for a given procId
 *
 *  \param procId [IN] Processor ID
 *******************************************************************************
 */
System_IpcMsg *System_ipcGetMsg(uint32_t procId);

/*******************************************************************************
 *  \brief Get pointer to IPC Out link IPC Que object for OUT to IN Que
 *
 *  \param ipcOutLinkId [IN] Link ID of IPC Out link
 *
 *******************************************************************************
 */
System_IpcQueObj *System_ipcGetIpcOut2InQue(uint32_t ipcOutLinkId);

/*******************************************************************************
 *  \brief Get pointer to IPC Out link IPC Que object for IN to OUT Que
 *
 *  \param ipcOutLinkId [IN] Link ID of IPC Out link
 *
 *******************************************************************************
 */
System_IpcQueObj *System_ipcGetIpcIn2OutQue(uint32_t ipcOutLinkId);

/*******************************************************************************
 *  \brief Get pointer to IPC System buffer given IPC link ID and element index
 *
 *  \param ipcOutLinkId [IN] Link ID of IPC Out link
 *  \param index        [IN] Index of element
 *
 *******************************************************************************
 */
System_IpcBuffer *System_ipcGetIpcBuffer(uint32_t ipcOutLinkId, uint32_t index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


