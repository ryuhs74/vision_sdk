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
 * \file utils_link_stats_collector.c
 *
 * \brief  This file currently just keeps a global variable in
 *         a shared memory, This variable is used for storing link
 *         statistics information.
 *
 * \version 0.0 (Mar 2015) : First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/src/utils_link_stats_collector.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* None */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

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
typedef struct Utils_LinkStatsIndexInfo
{
    uint32_t startIdx;
    /**< Start Index */
    uint32_t numInst;
    /**< Number of instances */
} Utils_LinkStatsIndexInfo_t;

/**
 *******************************************************************************
 *
 *  \brief  Structure for link Statistics Collector Object.
 *
 *          It stores link state colletors local information like which
 *          list state object is allocated for this core.
 *******************************************************************************
*/
typedef struct Utils_LinkStateCollectorObj
{
    uint32_t  coreId;
    /**< Core Id, storing it locally */
    BspOsal_SemHandle semHandle;
    /**< Semaphore Handle to protect allocFlags */
    Utils_LinkStatsIndexInfo_t  linkStatsIdxInfo[SYSTEM_PROC_MAX];
    /**< Index information for link stats for each core */
    Utils_LinkStatsIndexInfo_t  prfLoadInstIdxInfo[SYSTEM_PROC_MAX];
    /**< Index information for performance objects for each core */
} Utils_LinkStateCollectorObj_t;



/**
 *******************************************************************************
 *
 *  \brief  Global array for Link Statistics information.
 *
 *
 *******************************************************************************
 */

#ifndef BUILD_A15
#pragma DATA_SECTION(gSystemLinkStatsCoreObj,".bss:extMemNonCache:linkStats");
#pragma DATA_ALIGN(gSystemLinkStatsCoreObj, 128);
#endif
System_LinkStatsCoreObj gSystemLinkStatsCoreObj
#ifdef BUILD_A15
__attribute__ ((section(".bss:extMemNonCache:linkStats")))
__attribute__ ((aligned(128)))
#endif
    ;

/*******************************************************************************
 *  Global structures
 *******************************************************************************
 */


static Utils_LinkStateCollectorObj_t gUtilsLinkStateCollectorObj =
{
    0, NULL,
    {
        {UTILS_LINK_STATS_IPU1_0_INST_START, UTILS_LINK_STATS_IPU1_0_INST_NUM},
        {UTILS_LINK_STATS_IPU1_1_INST_START, UTILS_LINK_STATS_IPU1_1_INST_NUM},
        {UTILS_LINK_STATS_A15_0_INST_START, UTILS_LINK_STATS_A15_0_INST_NUM},
        {UTILS_LINK_STATS_DSP1_INST_START, UTILS_LINK_STATS_DSP1_INST_NUM},
        {UTILS_LINK_STATS_DSP2_INST_START, UTILS_LINK_STATS_DSP2_INST_NUM},
        {UTILS_LINK_STATS_EVE1_INST_START, UTILS_LINK_STATS_EVE1_INST_NUM},
        {UTILS_LINK_STATS_EVE2_INST_START, UTILS_LINK_STATS_EVE2_INST_NUM},
        {UTILS_LINK_STATS_EVE3_INST_START, UTILS_LINK_STATS_EVE3_INST_NUM},
        {UTILS_LINK_STATS_EVE4_INST_START, UTILS_LINK_STATS_EVE4_INST_NUM}
    },
    {
        {UTILS_LINK_STATS_PRF_IPU1_0_INST_START,
            UTILS_LINK_STATS_PRF_IPU1_0_INST_NUM},
        {UTILS_LINK_STATS_PRF_IPU1_1_INST_START,
            UTILS_LINK_STATS_PRF_IPU1_1_INST_NUM},
        {UTILS_LINK_STATS_PRF_A15_0_INST_START,
            UTILS_LINK_STATS_PRF_A15_0_INST_NUM},
        {UTILS_LINK_STATS_PRF_DSP1_INST_START,
            UTILS_LINK_STATS_PRF_DSP1_INST_NUM},
        {UTILS_LINK_STATS_PRF_DSP2_INST_START,
            UTILS_LINK_STATS_PRF_DSP2_INST_NUM},
        {UTILS_LINK_STATS_PRF_EVE1_INST_START,
            UTILS_LINK_STATS_PRF_EVE1_INST_NUM},
        {UTILS_LINK_STATS_PRF_EVE2_INST_START,
            UTILS_LINK_STATS_PRF_EVE2_INST_NUM},
        {UTILS_LINK_STATS_PRF_EVE3_INST_START,
            UTILS_LINK_STATS_PRF_EVE3_INST_NUM},
        {UTILS_LINK_STATS_PRF_EVE4_INST_START,
            UTILS_LINK_STATS_PRF_EVE4_INST_NUM}
    }
};


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/* None */


/**
 *******************************************************************************
 *
 *  \brief  Function to initialize link stats collector.
 *
 *          This api initializes Link stats collector, it resets the flags,
 *          counter for each core
 *
 *  \returns    0: Collector is initialized and ready to be used
 *              any other number: collector is not initialized and
 *                                return the error
 *******************************************************************************
 */
int32_t Utils_linkStatsCollectorInit()
{
    uint32_t cnt;
    uint32_t coreId;
    uint32_t startIdx, numInst;

    gUtilsLinkStateCollectorObj.semHandle = BspOsal_semCreate(1U, TRUE);
    UTILS_assert(gUtilsLinkStateCollectorObj.semHandle != NULL);

    /* Locally store the core id of this core,
       so that dont need to get coreId on every alloc/dealloc call */
    gUtilsLinkStateCollectorObj.coreId = System_getSelfProcId();

    /* Initialize all link stats object on the IPU1_0 core */
    if (SYSTEM_PROC_IPU1_0 == gUtilsLinkStateCollectorObj.coreId)
    {
        memset(&gSystemLinkStatsCoreObj, 0x0, sizeof(System_LinkStatsCoreObj));
    }

    coreId = gUtilsLinkStateCollectorObj.coreId;
    startIdx = gUtilsLinkStateCollectorObj.prfLoadInstIdxInfo[coreId].startIdx;
    numInst = gUtilsLinkStateCollectorObj.prfLoadInstIdxInfo[coreId].numInst;
    for (cnt = startIdx; cnt < (numInst + startIdx); cnt ++)
    {
        gSystemLinkStatsCoreObj.linkStats[cnt].isAlloc = FALSE;
    }

    return (0);
}


/**
 *******************************************************************************
 *
 *  \brief  Function to Deinitialize collector.
 *          checks if all link stats objects are de-allocated or not,
 *          Asserts if this is not the case.
 *          This API should be called after deinit of all links
 *
 *******************************************************************************
 */
Void Utils_linkStatsCollectorDeInit()
{
    uint32_t cnt;
    uint32_t startIdx, numInst;
    uint32_t coreId;

    /* Check to see if all link stat instances are deallocated */
    coreId = gUtilsLinkStateCollectorObj.coreId;
    startIdx = gUtilsLinkStateCollectorObj.prfLoadInstIdxInfo[coreId].startIdx;
    numInst = gUtilsLinkStateCollectorObj.prfLoadInstIdxInfo[coreId].numInst;
    for (cnt = startIdx; cnt < (numInst + startIdx); cnt ++)
    {
        if (TRUE == gSystemLinkStatsCoreObj.linkStats[cnt].isAlloc)
        {
            UTILS_assert(FALSE);
        }
    }

    if (NULL != gUtilsLinkStateCollectorObj.semHandle)
    {
        BspOsal_semDelete(&gUtilsLinkStateCollectorObj.semHandle);
    }
}

/**
 *******************************************************************************
 *
 *  \brief  Function to allocate link state object.
 *          Collector keeps a pool of link state object for each core.
 *          This API is used to allocates link state instance from this pool.
 *          Each link calls this api at create time to get link state object.
 *
 *
 *
 *  \return Pointer to link state object
 *          NULL in case there is no free instance in the pool
 *
 *******************************************************************************
 */
System_LinkStatistics *Utils_linkStatsCollectorAllocInst(uint32_t linkId, char *linkName)
{
    uint32_t cnt;
    uint32_t coreId;
    uint32_t dummyCmd;
    System_LinkStatistics *linkStats = NULL;
    Utils_LinkStatsIndexInfo_t *idxInfo = NULL;

    BspOsal_semWait(gUtilsLinkStateCollectorObj.semHandle, BSP_OSAL_WAIT_FOREVER);

    coreId = gUtilsLinkStateCollectorObj.coreId;
    idxInfo = &gUtilsLinkStateCollectorObj.linkStatsIdxInfo[coreId];

    for (cnt = idxInfo->startIdx;
            cnt < (idxInfo->numInst + idxInfo->startIdx);
            cnt ++)
    {
        if (FALSE == gSystemLinkStatsCoreObj.linkStats[cnt].isAlloc)
        {
            linkStats = &gSystemLinkStatsCoreObj.linkStats[cnt];

            /* Initialize command object */
            memset(linkStats, 0x0, sizeof(System_LinkStatistics));

            Utils_resetLinkStatistics(
                &linkStats->linkStats,
                SYSTEM_MAX_CH_PER_OUT_QUE,
                SYSTEM_MAX_OUT_QUE);

            Utils_resetLatency(&linkStats->linkLatency);
            Utils_resetLatency(&linkStats->srcToLinkLatency);

            strncpy(linkStats->linkName, linkName,
                (sizeof(linkStats->linkName) - 1U));
            /* The maximum size of name permitted is 31 characters now.
                In cases where the name was exactly 32 characters, there would
                be no space for the string null terminator.
                Also, if the name was > 32 characters and dosent have a null
                terminator. The destination linkStats->linkName will be a 
                non-null terminated string. */
            linkStats->linkName[sizeof(linkStats->linkName) - 1U] = '\0';

            /* Dummy Read to clear command queue */
            Utils_linkStatsRecvCommand(
                &linkStats->srcToLinkCmdObj, &dummyCmd);
            Utils_linkStatsRecvCommand(
                &linkStats->linkToSrcCmdObj, &dummyCmd);

            linkStats->linkId = linkId;

            linkStats->isAlloc = TRUE;
            break;
        }
    }

    BspOsal_semPost(gUtilsLinkStateCollectorObj.semHandle);

    return (linkStats);
}

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
int32_t Utils_linkStatsCollectorDeAllocInst(System_LinkStatistics *linkStats)
{
    int32_t status = -1;
    uint32_t cnt;
    uint32_t coreId;
    Utils_LinkStatsIndexInfo_t *idxInfo = NULL;

    BspOsal_semWait(gUtilsLinkStateCollectorObj.semHandle, BSP_OSAL_WAIT_FOREVER);

    coreId = gUtilsLinkStateCollectorObj.coreId;
    idxInfo = &gUtilsLinkStateCollectorObj.linkStatsIdxInfo[coreId];

    for (cnt = idxInfo->startIdx;
            cnt < (idxInfo->numInst + idxInfo->startIdx); cnt ++)
    {
        if (linkStats == &gSystemLinkStatsCoreObj.linkStats[cnt])
        {
            /* Alloc flag must be set to TRUE */
            UTILS_assert(TRUE ==
                linkStats->isAlloc);

            linkStats->isAlloc = FALSE;

            linkStats = NULL;

            status = 0;
            break;
        }
    }

    BspOsal_semPost(gUtilsLinkStateCollectorObj.semHandle);

    return (status);
}

/**
 *******************************************************************************
 *
 *  \brief  Function to send command.
 *
 *          Function sets command in the queue of the list stat
 *          object of the link. This is usually used by the monitor thread
 *          to send the command to the link.
 *
 *  \param  pCmdObj     Pointer to command object
 *  \param  cmd         Command id
 *
 *  \return 0       on success
 *         -1       if command cannot be written to queue
 *
 *******************************************************************************
 */
int32_t Utils_linkStatsSendCommand(System_LinkStatsCmdObj *pCmdObj, uint32_t cmd)
{
    int32_t status = -1;
    volatile uint32_t tmpValue;

    /* Error checking */
    if (NULL != pCmdObj)
    {
        if(pCmdObj->rdIdx == pCmdObj->wrIdx)
        {
            /* new command can be written */
            pCmdObj->cmd = cmd;

            /* flip write index */
            pCmdObj->wrIdx ^= 1;

            /* to make sure earlier write's are written to memory */
            tmpValue = pCmdObj->wrIdx;

            status = 0;
        }
        else
        {
            /* old command is not yet read, so new command
                 cannot be written */
            status = -1;
        }
    }

    return (status);
}


/**
 *******************************************************************************
 *
 *  \brief  Function to receive command.
 *
 *          Function gets command in the queue of the list stat
 *          object of the link. This is used by the each link to
 *          get the command and process it accordingly in the link.
 *
 *  \param  pCmdObj     Pointer to the link state object
 *  \param  cmd         Command Id
 *
 *  \return 0       on success
 *         -1       if command queue is empty
 *
 *******************************************************************************
 */
int32_t Utils_linkStatsRecvCommand(System_LinkStatsCmdObj *pCmdObj, uint32_t *cmd)
{
    int32_t status = -1;
    volatile uint32_t tmpValue;

    if ((NULL != pCmdObj) && (NULL != cmd))
    {
        if(pCmdObj->rdIdx != pCmdObj->wrIdx)
        {
              /* new command is written and can be read */
              *cmd = pCmdObj->cmd;

              /* flip read index */
              pCmdObj->rdIdx ^= 1;

              /* to make sure earlier write's are written to memory */
              tmpValue = pCmdObj->rdIdx;

              status = 0;
        }
        else
        {
              /* no new command available */
              status = -1;
        }
    }

    return status;
}

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
System_LinkStatistics *Utils_linkStatsGetLinkStatInst(uint32_t linkId)
{
    uint32_t linkCnt, coreId;
    System_LinkStatistics *tmpLinkStats = NULL;
    System_LinkStatistics *linkStats = NULL;
    Utils_LinkStatsIndexInfo_t *idxInfo = NULL;

    coreId = SYSTEM_GET_PROC_ID(linkId);
    idxInfo = &gUtilsLinkStateCollectorObj.linkStatsIdxInfo[coreId];

    for (linkCnt = idxInfo->startIdx;
            linkCnt < (idxInfo->numInst + idxInfo->startIdx); linkCnt ++)
    {
        tmpLinkStats = &gSystemLinkStatsCoreObj.linkStats[linkCnt];
        if ((TRUE == tmpLinkStats->isAlloc) &&
            (SYSTEM_GET_LINK_ID(linkId) == SYSTEM_GET_LINK_ID(
                tmpLinkStats->linkId)))
        {
            linkStats = tmpLinkStats;
            break;
        }
    }

    return (linkStats);
}

/**
 *******************************************************************************
 *
 *  \brief  Function to Pointer to accumulated performance load object.
 *
 *          Accumulated performance object keeps load information for each core.
 *          There is one object created for each core. This function is used to
 *          get pointer to this object.
 *          This is used by utils_prf.c utility to print the total load
 *          and also to store the load statistics in shared memory.
 *
 *  \param  coreId          Id of the Core
 *
 *  \return pAccPrfLoadObj  Pointer to Accumulated performance load
 *          NULL            If core Id is invalid.
 *
 *******************************************************************************
 */
System_LinkStatsCorePrfObj *Utils_linkStatsGetPrfLoadInst(uint32_t coreId)
{
    int32_t status = 0;
    System_LinkStatsCorePrfObj *pCoreLoadObj = NULL;

    /* Check for errors */
    if (coreId >= SYSTEM_PROC_MAX)
    {
        status = -1;
    }

    if (0 == status)
    {
        pCoreLoadObj = &gSystemLinkStatsCoreObj.corePrfObj[coreId];
    }

    return (pCoreLoadObj);
}


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
    uint32_t *maxLoadInst)
{
    uint32_t startIdx;
    System_LinkStatsPrfLoadObj *pPrfLoadObjStart = NULL;

    /* Check for errors */
    if ((coreId >= SYSTEM_PROC_MAX) || (NULL == maxLoadInst))
    {
        pPrfLoadObjStart = NULL;
    }
    else
    {
        startIdx = gUtilsLinkStateCollectorObj.prfLoadInstIdxInfo[coreId].
                        startIdx;
        *maxLoadInst = gUtilsLinkStateCollectorObj.prfLoadInstIdxInfo
                            [coreId].numInst;
        pPrfLoadObjStart = &gSystemLinkStatsCoreObj.prfLoadObj[startIdx];
    }

    return (pPrfLoadObjStart);
}

Void Utils_linkStatsPrintLinkStatistics(uint32_t linkId)
{
    System_LinkStatistics *pLinkStats;
    Utils_LinkStatistics linkStats;
    Utils_LatencyStats   linkLatency;
    Utils_LatencyStats   srcToLinkLatency;
    UInt32 cmd;

    pLinkStats = Utils_linkStatsGetLinkStatInst(linkId);
    if(pLinkStats==NULL)
    {
        Vps_printf(" CPU [%8s], LinkID [%3d], Link Statistics not available !\n",
                System_getProcName(SYSTEM_GET_PROC_ID(linkId)),
                SYSTEM_GET_LINK_ID(linkId)
            );
    }
    else
    {
        /* take a local copy */
        linkStats = pLinkStats->linkStats;
        linkLatency = pLinkStats->linkLatency;
        srcToLinkLatency = pLinkStats->srcToLinkLatency;

        Vps_printf(" \n");
        Vps_printf(" ### CPU [%6s], LinkID [%3d],\n",
                System_getProcName(SYSTEM_GET_PROC_ID(linkId)),
                SYSTEM_GET_LINK_ID(linkId)
            );
        Utils_printLinkStatistics(&linkStats, pLinkStats->linkName, FALSE);

        Utils_printLatency(pLinkStats->linkName,
                           &linkLatency,
                           &srcToLinkLatency,
                           FALSE);

        /* reset link stats */
        Utils_linkStatsSendCommand(
                    &pLinkStats->srcToLinkCmdObj,
                    LINK_STATS_CMD_RESET_STATS);

        /* clear any pending commands */
        Utils_linkStatsRecvCommand(&pLinkStats->linkToSrcCmdObj, &cmd);
    }
}

Void  Utils_linkStatsCollectorProcessCmd(System_LinkStatistics *linkStatsInfo)
{
    Int32 status = 0;
    UInt32 cmd;

    UTILS_assert(NULL != linkStatsInfo);


    if(linkStatsInfo->isAlloc)
    {
        status = Utils_linkStatsRecvCommand(&linkStatsInfo->srcToLinkCmdObj, &cmd);

        if (0 == status)
        {
            if (LINK_STATS_CMD_RESET_STATS == cmd)
            {
                Utils_resetLinkStatistics(
                    &linkStatsInfo->linkStats,
                    SYSTEM_MAX_CH_PER_OUT_QUE,
                    1);

                Utils_resetLatency(&linkStatsInfo->linkLatency);
                Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);

                /* Reset is done so send reset_done command to monitor thread */
                cmd = LINK_STATS_CMD_RESET_STATS_DONE;
                Utils_linkStatsSendCommand(
                    &linkStatsInfo->linkToSrcCmdObj, cmd);
            }
        }
    }
}


/*******************************************************************************
 *  Local Functions
 *******************************************************************************
 */

/* None */
