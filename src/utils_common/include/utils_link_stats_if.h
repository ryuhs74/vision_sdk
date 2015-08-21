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
 * \ingroup UTILS_API
 * \defgroup UTILS_LINK_STATS_IF_API Link Statistics Interface
 *
 * \brief Link statistics layer allows to keep the all the statistics in
 *        a common shared location.
 *
 * @{
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \file utils_link_stats_if.h
 *
 * \brief  Link Statistics Layer interface file, keeps definitaion of the
 *         data structures accessible by each link.
 *
 *******************************************************************************
 */

#ifndef _LINK_STATS_IF_
#define _LINK_STATS_IF_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <src/utils_common/include/utils_stat_collector.h>
#include <src/utils_common/include/utils_mem.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Tag used to identify Link Statistics object
 *        Used to check log object is valid or not
 *
 *******************************************************************************
*/
#define LINK_STATS_HEADER_TAG               (0xFFABCDEFU)

/**
 *******************************************************************************
 *
 * \brief Maximum number of Link Stats object supported
 *
 *******************************************************************************
*/
#define LINK_STATS_MAX_STATS_INST           (130U)

/**
 *******************************************************************************
 *
 * \brief Maximum number of Utils perf handles allowed
 *
 *******************************************************************************
*/
#define LINK_STATS_PRF_MAX_TSK               (336U)

/**
 *******************************************************************************
 *
 * \brief Command to Reset the Link Statistics.
 *        When link gets this command, it resets the link statistics
 *        information.
 *
 *******************************************************************************
*/
#define LINK_STATS_CMD_RESET_STATS          (0x1000)

/**
 *******************************************************************************
 *
 * \brief Command to inform that the Reset is done.
 *        This is sent by the link to the monitor thread that the
 *        reset is completed.
 *
 *******************************************************************************
*/
#define LINK_STATS_CMD_RESET_STATS_DONE     (0x1001)

/**
 *******************************************************************************
 *
 * \brief Command to copy core load information to the shared memory.
 *        This is sent by the monitor thread to copy the core load
 *        into share memory.
 *
 *******************************************************************************
*/
#define LINK_STATS_CMD_COPY_CORE_LOAD       (0x1002)

/**
 *******************************************************************************
 *
 * \brief Command to inform that core loading/performance data is copied
 *        into shared memory.
 *
 *******************************************************************************
*/
#define LINK_STATS_CMD_COPY_CORE_LOAD_DONE  (0x1003)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure for Link Statistics command object.
 *
 *          This structure is implement a queue of depth 1 between
 *          monitor thread and each link thread. Monitor thread posts
 *          command into this queue and when each link link sees
 *          this command, it performs actions based on the command.
 *          There is one command object between each link and monitor.
 *
 *******************************************************************************
*/
typedef struct
{
  volatile uint32_t cmd;
  /**< variable containing command value*/
  volatile uint32_t rdIdx;
  /**< Command Read Index */
  volatile uint32_t wrIdx;
  /**< Command Write Index */
} System_LinkStatsCmdObj;

/**
 *******************************************************************************
 *
 *  \brief  Structure for storing load information for the core.
 *
 *          This structure stores the complete load information
 *          of the core. It keeps time spent on Hwi, Swi etc.
 *
 *******************************************************************************
*/
typedef struct
{
    uint32_t totalSwiThreadTimeHi;
    /**< Upper 32bits of Total time spent in swi */
    uint32_t totalSwiThreadTimeLo;
    /**< Lower 32bits of Total time spent in swi */
    uint32_t totalHwiThreadTimeHi;
    /**< Upper 32bits of Total time spent in hwi */
    uint32_t totalHwiThreadTimeLo;
    /**< Lower 32bits of Total time spent in hwi */
    uint32_t totalTimeHi;
    /**< Upper 32bits of Total time spent */
    uint32_t totalTimeLo;
    /**< Lower 32bits of Total time spent */
    uint32_t totalIdlTskTimeHi;
    /**< Upper 32bits of Total time spent in idle task */
    uint32_t totalIdlTskTimeLo;
    /**< Lower 32bits of Total time spent in idle task */
} System_LinkStatsAccPrfLoadObj;

/**
 *******************************************************************************
 *
 *  \brief  Structure containing profiling information for a task.
 *
 *******************************************************************************
*/
typedef struct
{
    uint32_t isAlloc;
    /**< Flag to indicate if this instance is allocated or not */
    char name[32];
    /**< name of the task */
    uint32_t totalTskThreadTimeHi;
    /**< Upper 32bits of Total time the task has spent */
    uint32_t totalTskThreadTimeLo;
    /**< Lower 32bits of Total time the task has spent */
} System_LinkStatsPrfLoadObj;



/**
 *******************************************************************************
 *
 *  \brief  Structure for link statistics.
 *
 *          This structure is used by each link on all core for storing link
 *          statistics. Each link gets the pointer to this structure
 *          and updates link statistics
 *******************************************************************************
*/
typedef struct
{
    uint32_t             isAlloc;
    /**< Flag to indicate if this instance is allocated or not */

    uint32_t             linkId;
    /**< Storing link id locally so that each statistics instance
         can be identified. */
    char                 linkName[32];
    /**< String to hold link name */

    Utils_LinkStatistics linkStats;
    /**< links statistics like frames captured/processed, dropped etc
     */
    Utils_LatencyStats   linkLatency;
    /**< Structure to find out min, max and average latency of the link */
    Utils_LatencyStats   srcToLinkLatency;
    /**< Structure to find out min, max and average latency from
     *   source to this link
     */
    System_LinkStatsCmdObj srcToLinkCmdObj;
    /**< Link Stats Command Queue, used to send/receive command from
         monitor thread to Link */
    System_LinkStatsCmdObj linkToSrcCmdObj;
    /**< Link Stats Command Queue, used to send/receive command from
         Link to Monitor thread */
} System_LinkStatistics;

typedef struct
{
    System_LinkStatsAccPrfLoadObj  accPrfLoadObj;
    /**< Array of Accumulated performance load objects,
         one for each core,
         Stores core load information like
         time spent on executing interrup or software interrupt etc. */
    Utils_MemHeapStats             heapStats[UTILS_HEAPID_MAXNUMHEAPS];
    /**< System Heap Status */

    UInt32                         maxSemaphoreObjs;
    /**< Maximum statically allocated semaphore objects */

    UInt32                         freeSemaphoreObjs;
    /**< Number of free semaphore objects */

    UInt32                         maxClockObjs;
    /**< Maximum statically allocated Clock objects */

    UInt32                         freeClockObjs;
    /**< Number of free Clock objects */

    UInt32                         maxTaskObjs;
    /**< Maximum statically allocated Task objects */

    UInt32                         freeTaskObjs;
    /**< Number of free Task objects */

    UInt32                         maxHwiObjs;
    /**< Maximum statically allocated Hwi objects */

    UInt32                         freeHwiObjs;
    /**< Number of free Hwi objects */

    System_LinkStatsCmdObj         srcToLinkCmdObj;
    /**< Link Stats Command Queue, used to send/receive command from
         monitor thread to Link */
    System_LinkStatsCmdObj         linkToSrcCmdObj;
    /**< Link Stats Command Queue, used to send/receive command from
         Link to Monitor thread */
} System_LinkStatsCorePrfObj;

/**
 *******************************************************************************
 *
 *  \brief  Structure for link Statistics core Object.
 *
 *          This structure contains link statistics object instaces for all
 *          links on all cores, Currently, it does not contains any additional
 *          information, but it can be used to store any additional/common
 *          statistics information.
 *******************************************************************************
*/
typedef struct
{
    System_LinkStatsPrfLoadObj     prfLoadObj[LINK_STATS_PRF_MAX_TSK];
    /* Performance Task Object,
       Stores task loading information,
       There are #LINK_STATS_PRF_MAX_TSK tasks supported */

    System_LinkStatsCorePrfObj     corePrfObj[SYSTEM_PROC_MAX];
    /**< Core Performance Object
         Contains accumulated performance and each tasks
         performance for each core. */

    System_LinkStatistics          linkStats[LINK_STATS_MAX_STATS_INST];
    /**< Link statistics for all links on all cores */
} System_LinkStatsCoreObj;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Function to allocate link state object.
 *          Collector keeps a pool of link state object for each core.
 *          This API is used to allocates link state instance from this pool.
 *          Each link calls this api at create time to get link state object.
 *
 *  \return Pointer to link state object
 *          NULL in case there is no free instance in the pool
 *
 *******************************************************************************
 */
System_LinkStatistics *Utils_linkStatsCollectorAllocInst(uint32_t linkId, char *linkName);

/**
 *******************************************************************************
 *
 *  \brief  Function to De-allocate link state object.
 *          Function deallocates the link state object and returns it
 *          to the pool.
 *
 *  \param  linkStats   Pointer to the link state object
 *
 *  \return Pointer to link state object
 *          NULL in case there is no free instance in the pool
 *
 *******************************************************************************
 */
Int32 Utils_linkStatsCollectorDeAllocInst(System_LinkStatistics *linkStats);

/**
 *******************************************************************************
 *
 *  \brief  Function to print link statistics given a link ID
 *
 *******************************************************************************
 */
Void Utils_linkStatsPrintLinkStatistics(uint32_t linkId);

/**
 *******************************************************************************
 *
 *  \brief  Function to reset link stats based on received command
 *
 *******************************************************************************
 */
Void  Utils_linkStatsCollectorProcessCmd(System_LinkStatistics *linkStatsInfo);

/**
 *******************************************************************************
 *
 *  \brief  Function to send command.
 *
 *          Function sets command in the queue of the list stat
 *          object of the link. This is usually used by the monitor thread
 *          to send the command to the link.
 *
 *  \param  pCmdObj     Pointer to command Object
 *  \param  cmd         Command id
 *
 *  \return 0       on success
 *         -1       if command cannot be written to queue
 *
 *******************************************************************************
 */
Int32 Utils_linkStatsSendCommand(System_LinkStatsCmdObj *pCmdObj, uint32_t cmd);

/**
 *******************************************************************************
 *
 *  \brief  Function to receive command.
 *
 *          Function gets command in the queue of the list stat
 *          object of the link. This is used by the each link to
 *          get the command and process it accordingly in the link.
 *
 *  \param  pCmdObj     Pointer to command Object
 *  \param  cmd         Command Id
 *
 *  \return 0       on success
 *         -1       if command queue is empty
 *
 *******************************************************************************
 */
Int32 Utils_linkStatsRecvCommand(System_LinkStatsCmdObj *pCmdObj, uint32_t *cmd);

/**
 *******************************************************************************
 *
 *  \brief  Function to get pointer to link stat instance for the
 *          given Link Id and Core Id.
 *
 *  \param  linkId   Id of the Link
 *
 *  \return 0       on success
 *         -1       if command queue is empty
 *
 *******************************************************************************
 */
System_LinkStatistics *Utils_linkStatsGetLinkStatInst(uint32_t linkId);

/**
 *******************************************************************************
 *
 *  \brief  Function to allocate Performance Load Instance on the give core.
 *
 *          It is used by the utils_prf utility when a new task is created.
 *          The performance load instance stores the task load on this core.
 *
 *  \param  coreId   Id of the Core
 *
 *  \return pPrfLoadObj     Pointer to Performance Load Object
 *          NULL            If there are no free performance load instances
 *
 *******************************************************************************
 */
System_LinkStatsCorePrfObj *Utils_linkStatsGetPrfLoadInst(uint32_t coreId);

/**
 *******************************************************************************
 *
 *  \brief  Function returns performance load object for the task and also
 *          max instances supported for this core.
 *
 *          It is used by the utils_prf utility to create local tasks objects.
 *          The performance load instance stores the task load on this core.
 *
 *  \param  coreId          Id of the Core
 *  \param  maxLoadInst     Max Load/Task Instances supported for this core,
 *                          returned by the core.
 *
 *  \return pPrfLoadObj     Start pointer to performance load/task instancs
 *          NULL            if core id is wrong or maxLoadInst pointer is null.
 *
 *******************************************************************************
 */
System_LinkStatsPrfLoadObj *Utils_linkStatsGetGetMaxPrfLoadInst(
    uint32_t coreId,
    uint32_t *maxLoadInst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
