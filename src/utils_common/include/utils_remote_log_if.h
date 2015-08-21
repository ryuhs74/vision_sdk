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
 * \ingroup UTILS_API
 * \defgroup UTILS_REMOTE_LOG_IF_API Remote Log Interface
 *
 *         Remote log allows debugging a core by exporting it's logs to
 *         any other core dynamically.
 *
 * @{
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \file utils_remote_log_if.h Remote Log Interface
 *
 * \brief  Remote Log Interface
 *
 *******************************************************************************
 */

#ifndef _REMOTE_LOG_IF_
#define _REMOTE_LOG_IF_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Max allowed parameter buffer size
 *
 *******************************************************************************
*/
#define REMOTE_LOG_PARAM_BUF_SIZE   (1024)

/**
 *******************************************************************************
 *
 * \brief Size of log buffer
 *
 *******************************************************************************
*/
#define REMOTE_LOG_LOG_BUF_SIZE     (158*1024U)

/**
 *******************************************************************************
 *
 * \brief Tag used to identify remote log object
 *        Used to check log object is valid or not
 *
 *******************************************************************************
*/
#define REMOTE_LOG_HEADER_TAG       (0)

/**
 *******************************************************************************
 *
 * \brief Flag to indicate a character is requested or returned
 *
 *******************************************************************************
*/
#define REMOTE_LOG_FLAG_TYPE_CHAR    (0x01000000)

/**
 *******************************************************************************
 *
 * \brief Flag to indicate a string is requested or returned
 *
 *******************************************************************************
*/
#define REMOTE_LOG_FLAG_TYPE_STRING  (0x10000000)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 ********************************************************************************
 *
 *  \brief  Enumerations for the core's initialization states
 *
 ********************************************************************************
*/
typedef enum
{
    CORE_APP_INITSTATUS_PRE_INIT = 0,
    /**< Before init */
    CORE_APP_INITSTATUS_DO_INIT,
    /**< Trigger from host to start initialization */
    CORE_APP_INITSTATUS_INIT_DONE,
    /**< Update from core to host that application initialization is complete */
    CORE_APP_INITSTATUS_DO_EXIT,
    /**< Trigger from host to exit app */
    CORE_APP_INITSTATUS_EXIT_DONE
    /**< State to indicate app exit is complete */
} CoreApp_InitializationState;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

typedef struct
{
    volatile unsigned int headerTag;
    /**< Tag used to identify remote log object
     *    Used to check log object is valid or not */
    volatile unsigned int serverIdx;
    /**< Write pointer. As the core logs this keeps incrementing */
    volatile unsigned int clientIdx;
    /**< Read pointer. As other core reads the log this keeps incrementing */
    volatile unsigned int appInitState;
    /** < Flag indicating application initialization status */
} RemoteLog_MemInfo;

 /**
 *******************************************************************************
 *
 *  \brief  Structure for remote log object.
 *
 *          This structure is used by a remote core to store information about
 *          log buffer. This log buffer is a shared memory, through which other
 *          cores can see logs of this core
 *******************************************************************************
*/
typedef struct
{
    RemoteLog_MemInfo memInfo[SYSTEM_PROC_MAX];
    /**< Shared memory information for each core */
    volatile unsigned char serverLogBuf[REMOTE_LOG_LOG_BUF_SIZE];
    /** < log buffer into which all logs from the core go into */
} RemoteLog_CoreObj;

/**
 *******************************************************************************
 *
 *  \brief  Structure stores start index and size information.
 *
 *          The number of link stats objects and performance stats objects
 *          are different for each core. There is a big array of
 *          link stats and performance objects allocated in
 *          #System_LinkStatsCoreObj. This structure is used to get
 *          start index and number of instances into this big array
 *          for each core.
 *******************************************************************************
*/
typedef struct
{
    uint32_t startIdx;
    /**< Start Index */
    uint32_t size;
    /**< Number of instances */
} RemoteLog_ServerIndexInfo;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Initializes remote log client.
 *
 * Creates periodic task for printing the logs from remote server.
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
Int32 Utils_remoteLogClientInit();

/**
 *******************************************************************************
 *
 * \brief This function is used for initiating remote log server.
 *
 * \return  returns 0 on success
 *
 *******************************************************************************
 */
int RemoteLog_init();

/**
 *******************************************************************************
 *
 * \brief Return Index information for the given core
 *
 * \param coreId    [IN]  Id of the core
 *
 * \return returns 0 on success
 *
 *******************************************************************************
 */
RemoteLog_ServerIndexInfo *RemoteLog_getCoreIdxInfo(int coreId);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
