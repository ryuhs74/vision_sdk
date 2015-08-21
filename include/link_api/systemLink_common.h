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
 *  \ingroup FRAMEWORK_MODULE_API
 *  \defgroup PROCESSOR_LINK_API Processor Link API
 *
 *   Processor Link is a link which is created on every processor to handle
 *   generic commands which do not fall into any specific pre-defined link
 *   category like capture or display. This interface allows user/developer to
 *   extend new commands in the system without having to create a new link
 *   everytime. Normally this link does not exchange frames and usually
 *   handles control commands only.
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 *   \ingroup PROCESSOR_LINK_API
 *   \defgroup SYSTEM_COMMON_LINK_API Processor Link API: Common Interfaces
 *
 *   This module defines the control commands that are applicable to
 *   all processors running in the system.
 *
 *   @{
*/

/**
 *
 *******************************************************************************
 *
 *   \file systemLink_common.h
 *   \brief Processor Link API: Common Interfaces
 *
 *******************************************************************************
*/


#ifndef _SYSTEM_LINK_COMMON_H_
#define _SYSTEM_LINK_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */

#include <include/link_api/system.h>

/**
 *******************************************************************************
 *
 *  \ingroup LINK_API_CMD
 *  \addtogroup SYSTEM_COMMON_CMD Processor Link API: Common Control Commands
 *
 *   @{
*/

/**
 *******************************************************************************
 *
 *  \brief System CMD: Start CPU load computation
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START       (0x9000)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Stop CPU load computation
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_STOP       (0x9001)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Reset CPU load computation
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET      (0x9002)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Print CPU load information
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_CPU_LOAD_CALC_PRINT      (0x9003)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Print CPU load, Task Load and Heap status information
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_PRINT_STATUS             (0x9004)

/**
 *******************************************************************************
 *
 *  \brief Check's if core is active or not
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_CORE_STATUS               (0x9005)

/**
 *******************************************************************************
 *
 *  \brief Gets Load on this core
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_GET_LOAD                  (0x9006)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Run EDMA LLD standalone tests
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_RUN_DMA_TEST              (0x9007)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Print Statistics standalone tests
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_PRINT_STAT_COLL              (0x9008)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Reset Statistics standalone tests
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_RESET_STAT_COLL              (0x9009)


/**
 *******************************************************************************
 *
 *  \brief System CMD: Get IP address of the system, if enabled
 *
 *  \param SystemCommon_IpAddr  [OUT] System IP address
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_GET_IP_ADDR                 (0x900A)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Allocate buffer from heaps associated with System_HeapId
 *
 *  \param SystemCommon_AllocBuffer  [OUT] bufferPtr physical address
 *  \param SystemCommon_AllocBuffer  [IN]  System_heapId
 *  \param SystemCommon_AllocBuffer  [IN]  size
 *  \param SystemCommon_AllocBuffer  [IN]  align
 *
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_ALLOC_BUFFER                (0x900B)
/**
 *******************************************************************************
 *
 *  \brief System CMD: Free buffer from heaps associated with System_HeapId
 *
 *  \param SystemCommon_FreeBuffer  [IN] bufferPtr physical address
 *  \param SystemCommon_FreeBuffer  [IN] System_heapId
 *  \param SystemCommon_FreeBuffer  [IN] size
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_FREE_BUFFER                 (0x900C)


/**
 *******************************************************************************
 *
 *  \brief System CMD: Get proc Id where NDK is running
 *
 *  \param SystemCommon_NetProcId  [OUT] procId
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_CMD_NET_PROC_ID                  (0x900D)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Set the EVE Auto clock gate mode
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_EVE_AUTOCG                        (0x900E)

/**
 *******************************************************************************
 *
 *  \brief System CMD: Set the EVE Idle Power mode
 *
 *  \param NONE
 *
 *******************************************************************************
*/
#define SYSTEM_COMMON_EVE_IDLE                          (0x900F)


/* @} */

/*******************************************************************************
 *  Define's
 *******************************************************************************
 */
#define SYSTEM_COMMON_MAX_CMD_HANDLERS                (4)

/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Heap ID
 *******************************************************************************
*/
typedef enum
{
    SYSTEM_HEAPID_DDR_NON_CACHED_SR0,
    /**< Heap ID of heap in DDR
     *   - This is non-cached memory
     *   - This is shared across all CPUs
     *   - Recommended to be used for small information structures, that need to be
     *     shared across CPUs
     */

    SYSTEM_HEAPID_DDR_CACHED_SR1,
    /**< Heap ID of heap in DDR
     *   - This is cached memory
     *   - This is shared across all CPUs
     *   - Recommended to be used for big buffer allocations, that need to be shared
     *     across CPUs
     */

    SYSTEM_HEAPID_OCMC_SR2,
    /**< Heap ID of heap in DDR
     *   - This is cached or non-cached memory depending on CPU MMU setting
     *   - This is shared across all CPUs
     *   - Recommended to be used as fast access temporary memory by algorithms
     */

    SYSTEM_HEAPID_RESERVED1,
    /**< Heap ID of heap in DDR
     *   - This is cached memory
     *   - This is only visible to the local CPUs
     *   - Recommended to be used for small information, that is needed by the
     *     local algorithm
     */

    SYSTEM_HEAPID_RESERVED2,
    /**< Heap ID of heap in L2 Memory
     *   - This is cached or non-cached memory depending on CPU MMU setting
     *   - This is only visible to the local CPU
     *   - Recommended to be used as fast access temporary memory by algorithms
     */

    SYSTEM_HEAPID_MAXNUMHEAPS,
    /**< max Heap ID */

    SYSTEM_HEAPID_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} System_HeapId;

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *  \brief Callback to handle user defined commands
 *
 *  \param cmd [IN] Command that needs to be handled
 *  \param pPrm [IN/OUT] Parameters for this command
 *
 *******************************************************************************
 */
typedef Void (*SystemLink_CmdHandler)(UInt32 cmd, Void *pPrm);

/**
 *******************************************************************************
 *
 *  \brief System Common: Print CPU load, Task Load and Heap Staus..
 *
 *   Defines System Common parameters that are passed when system statistics
 *   print is requested..
 *
 *******************************************************************************
*/

typedef struct {

    UInt32 printCpuLoad;
    /**< [IN] Print CPU load of each core */

    UInt32 printTskLoad;
    /**< [IN] Print CPU loading of each Task*/

    UInt32 printHeapStatus;
    /**< [IN] Print Current Heap status of core*/

} SystemCommon_PrintStatus;


/**
 *******************************************************************************
 *
 *  \brief IP address of system
 *
 *******************************************************************************
 */
typedef struct {

    char ipAddr[32];
    /**< IP address of the system as string */

} SystemCommon_IpAddr;

/**
 *******************************************************************************
 *
 *  \brief SystemCommon_AllocBuffer defines parameters used to
 *         allocates buffer from IPU managed heap
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 bufferPtr;
    /**< [OUT] pointer to allocated buffer */
    UInt32 heapId;
    /**< [IN] Heap id of the heap to allocate from */
    UInt32 size;
    /**< [IN] Size of the buffer to be allocated */
    UInt32 align;
    /**< [IN] Alignment of the allocated buffer */
} SystemCommon_AllocBuffer;

/**
 *******************************************************************************
 *
 *  \brief SystemCommon_AllocBuffer defines parameters used to
 *         free buffer from IPU managed heap
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 bufferPtr;
    /**< [IN] pointer to allocated buffer */
    UInt32 heapId;
    /**< [IN] Heap id of the heap to allocate from */
    UInt32 size;
    /**< [IN] Size of the buffer to be allocated */
} SystemCommon_FreeBuffer;

/**
 *******************************************************************************
 *
 *  \brief IP address of system
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 procId;
    /**< procId where NDK is runnning */

} SystemCommon_NetProcId;


/*******************************************************************************
 *  Function's
 *******************************************************************************
 */




/**
 *******************************************************************************
 *  \brief Register user defined command handler in System Link
 *
 *         Multiple hanlder can be registered
 *         All handlers are aclled when a command is received
 *         Unsupported command should be ignored in handler
 *
 *  \param handler [IN] User defined handler
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 SystemLink_registerHandler(SystemLink_CmdHandler handler);

/**
 *******************************************************************************
 *  \brief Un-Register user defined command handler in System Link
 *
 *  \param handler [IN] User defined handler to unregister
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 SystemLink_unregisterHandler(SystemLink_CmdHandler handler);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/


