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
 *  \file utils_prf.c
 *
 *  \brief This file has implementation for UTILS PERF API
 *
 *
 *  \version 0.0 (Jun 2013) : [NN] First version
 *  \version 0.1 (Jul 2013) : [NN] Updates as per code review comments
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_idle.h>
#include <src/links_eve/system/system_priv_eve.h>
#include <src/utils_common/src/utils_link_stats_collector.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Convert float to int by rounding off
 *
 *******************************************************************************
*/
#define UTILS_FLOAT2INT_ROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

#ifdef BUILD_M4_0
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_IPU1_0_INST_NUM)
#endif

#ifdef BUILD_M4_1
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_IPU1_1_INST_NUM)
#endif

#ifdef BUILD_A15
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_A15_0_INST_NUM)
#endif

#ifdef BUILD_DSP_1
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_DSP1_INST_NUM)
#endif

#ifdef BUILD_DSP_2
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_DSP2_INST_NUM)
#endif

#ifdef BUILD_ARP32_1
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_EVE1_INST_NUM)
#endif

#ifdef BUILD_ARP32_2
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_EVE2_INST_NUM)
#endif

#ifdef BUILD_ARP32_3
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_EVE3_INST_NUM)
#endif

#ifdef BUILD_ARP32_4
#define UTILS_PRF_MAX_TASK_INST (UTILS_LINK_STATS_PRF_EVE4_INST_NUM)
#endif




/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

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
    BspOsal_TaskHandle pTsk;
    /**< Handle to the task */
    char name[32];
    /**< name of the task */
    uint64_t totalTskThreadTime;
    /**< Total time the task has spent */
} Utils_PrfTsHndl;

/**
 *******************************************************************************
 *
 *  \brief  Structure containing profiling information for all tasks.
 *
 *******************************************************************************
*/
typedef struct
{
    Utils_PrfTsHndl                tskHndlObj[UTILS_PRF_MAX_TASK_INST];
    /**< Array of Utils_PrfTsHndl objects */
    System_LinkStatsPrfLoadObj     *pPrfLoadObj;
    /**< Pointer to the Performance Load information for the tasks
         stored in shared location.
         This memory gets updated only when Copy command is received */
    System_LinkStatsAccPrfLoadObj   accPrfLoadObj;
    /**< Local Copy of the accumulated performance load object */

    System_LinkStatsCorePrfObj     *pCorePrfLoadObj;
    /**< Core performance object,
         will be updated when monitor thread sends command to copy
         performance/load parameters */
} Utils_PrfObj;


/**
 *******************************************************************************
 *
 *  \brief  Structure containing accumulated
 *          profiling information for all tasks.
 *
 *******************************************************************************
*/


/**
 *******************************************************************************
 * \brief An instance of Utils_PrfObj
 *******************************************************************************
 */
Utils_PrfObj gUtils_prfObj;

/**
 *******************************************************************************
 * \brief Varibale to enable or disable Load calculation
 *******************************************************************************
 */
uint32_t gUtils_startLoadCalc = 0;


/**
 *******************************************************************************
 *
 * \brief Call this function before using any peformance or Timestamp utils
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
int32_t Utils_prfInit()
{
    uint32_t maxTskHndl;

    memset(&gUtils_prfObj, 0, sizeof(gUtils_prfObj));

    /* Due to a bug in the load module, you will need to call Load_reset().
       If this not called, then the load is sometimes incorrect the very
       first time, This will be fixed in future Bios release */
    Load_reset();

    /* Getting Core Performance object at init time, so that
       it can be used by other APIs */
    gUtils_prfObj.pCorePrfLoadObj =
        Utils_linkStatsGetPrfLoadInst(System_getSelfProcId());

    UTILS_assert(NULL != gUtils_prfObj.pCorePrfLoadObj);

    /* Initalize core performance load object */
    memset(gUtils_prfObj.pCorePrfLoadObj,
        0x0, sizeof(System_LinkStatsCorePrfObj));

    gUtils_prfObj.pPrfLoadObj = Utils_linkStatsGetGetMaxPrfLoadInst(
        System_getSelfProcId(),
        &maxTskHndl);
    UTILS_assert(0 != maxTskHndl);
    UTILS_assert(NULL != gUtils_prfObj.pPrfLoadObj);
    UTILS_assert(maxTskHndl == UTILS_PRF_MAX_TASK_INST);

    /* Reset allocated Task Handle Object */
    memset(gUtils_prfObj.tskHndlObj, 0,
        sizeof(Utils_PrfTsHndl) * UTILS_PRF_MAX_TASK_INST);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief de-Init of the performance and timestamp utils
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
int32_t Utils_prfDeInit()
{
    gUtils_prfObj.pCorePrfLoadObj = NULL;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get the 64-bit timer ticks
 *
 * \return  64-bit timer ticks
 *
 *******************************************************************************
 */
uint64_t Utils_prfTsGet64()
{
    Types_Timestamp64 ts64;
    uint64_t curTs;

    Timestamp_get64(&ts64);

    curTs = ((uint64_t) ts64.hi << 32) | ts64.lo;

    return curTs;
}

/**
 *******************************************************************************
 *
 * \brief Register a task for load calculation
 *
 * \param  pTsk     [IN] Task Handle
 * \param  name     [IN] A name for the task
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
int32_t Utils_prfLoadRegister(BspOsal_TaskHandle pTsk, char *name)
{
    uint32_t hndlId;
    uint32_t cookie;
    int32_t status = FVID2_EFAIL;
    Utils_PrfTsHndl *pHndl;
    System_LinkStatsCorePrfObj *pCorePrfLoadObj;

    pCorePrfLoadObj = gUtils_prfObj.pCorePrfLoadObj;
    UTILS_assert(NULL != pCorePrfLoadObj);

    cookie = Hwi_disable();

    for (hndlId = 0; hndlId < UTILS_PRF_MAX_TASK_INST; hndlId++)
    {
        pHndl = &gUtils_prfObj.tskHndlObj[hndlId];

        if (pHndl->isAlloc == FALSE)
        {
            pHndl->isAlloc = TRUE;
            pHndl->pTsk = pTsk;
            strncpy(pHndl->name, name, (sizeof(pHndl->name) - 1U));
            /* The maximum size of name permitted is 31 characters now.
                In cases where the name was exactly 32 characters, there would
                be no space for the string null terminator.
                Also, if the name was > 32 characters and dosent have a null
                terminator. The destination pHndl->name will be a non-null
                terminated string. */
            pHndl->name[sizeof(pHndl->name) - 1U] = '\0';

            /* Copy the parameters in the load object from shared memory,
               Same number of instances are allocated in the shared memory,
               so cnt can be used directly  */
            gUtils_prfObj.pPrfLoadObj[hndlId].isAlloc = TRUE;
            /* This Alloc Flag can be used by the monitor thread to check
               which all instances are used */
            strncpy(gUtils_prfObj.pPrfLoadObj[hndlId].name,
                    name,
                    (sizeof(gUtils_prfObj.pPrfLoadObj[hndlId].name) - 1U));
            /* The maximum size of name permitted is 31 characters now.
                In cases where the name was exactly 32 characters, there would
                be no space for the string null terminator.
                Also, if the name was > 32 characters and dosent have a null
                terminator. The destination pHndl->name will be a non-null
                terminated string. */
            gUtils_prfObj.pPrfLoadObj[hndlId].name \
                [(sizeof(gUtils_prfObj.pPrfLoadObj[hndlId].name) - 1U)] = '\0';
            status = SYSTEM_LINK_STATUS_SOK;
            break;
        }
    }

    if (SYSTEM_LINK_STATUS_SOK != status)
    {
        Vps_printf(" UTILS: PRF: ##### Cannot allocate Object for %s ####\n",
            name);
    }

    Hwi_restore(cookie);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Un-Register the task for load caculation
 *
 * \param  pTsk    [IN] Task Handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
int32_t Utils_prfLoadUnRegister(BspOsal_TaskHandle pTsk)
{
    uint32_t hndlId;
    uint32_t cookie;
    int32_t status = FVID2_EFAIL;
    Utils_PrfTsHndl *pHndl;
    System_LinkStatsCorePrfObj *pCorePrfLoadObj;

    pCorePrfLoadObj = gUtils_prfObj.pCorePrfLoadObj;
    UTILS_assert(NULL != pCorePrfLoadObj);

    cookie = Hwi_disable();

    for (hndlId = 0; hndlId < UTILS_PRF_MAX_TASK_INST; hndlId++)
    {
        pHndl = &gUtils_prfObj.tskHndlObj[hndlId];

        if ((TRUE == pHndl->isAlloc) && (pHndl->pTsk == pTsk))
        {
            pHndl->isAlloc = FALSE;

            /* Dealloc this instance from the shared memory also,
               Same number of instances are allocated in the shared memory,
               so cnt can be used directly  */
            gUtils_prfObj.pPrfLoadObj[hndlId].isAlloc = FALSE;
            strncpy(gUtils_prfObj.pPrfLoadObj[hndlId].name,
                    "",
                    (sizeof(gUtils_prfObj.pPrfLoadObj[hndlId].name) - 1U));
            /* The maximum size of name permitted is 31 characters now.
            In cases where the name was exactly 32 characters, there would
            be no space for the string null terminator.
            Also, if the name was > 32 characters and dosent have a null
            terminator. The destination pMultiMbxTsk->tskName will be a non-null
            terminated string. */
            gUtils_prfObj.pPrfLoadObj[hndlId].name [\
                (sizeof(gUtils_prfObj.pPrfLoadObj[hndlId].name) - 1U)] = '\0';
            status = SYSTEM_LINK_STATUS_SOK;
            break;
        }
    }

    Hwi_restore(cookie);

    return status;
}


int32_t Utils_prfGetLoad(Utils_SystemLoadStats *pStats)
{
    int32_t cpuLoad;
    uint32_t cookie;
    uint64_t idlTime64, totalTime64, temp;

    cookie = Hwi_disable();

    idlTime64 = gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeLo & 0xFFFFFFFFU;
    temp = gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeHi & 0xFFFFFFFFU;
    idlTime64 |= (temp << 32);

    /* Local Call, so getting performance numbers from the local object */
    totalTime64 = gUtils_prfObj.accPrfLoadObj.totalTimeLo & 0xFFFFFFFFU;
    temp = gUtils_prfObj.accPrfLoadObj.totalTimeHi & 0xFFFFFFFFU;
    totalTime64 |= (temp << 32);

    cpuLoad = 1000 - ((idlTime64 * 1000) / totalTime64);

    Hwi_restore(cookie);

    pStats->totalLoadParams.integerValue    = cpuLoad/10;
    pStats->totalLoadParams.fractionalValue = cpuLoad%10;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Print loads for all the registered tasks
 *
 * \param  printTskLoad    [IN] print task load if true
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
int32_t Utils_prfLoadPrintAll(Bool printTskLoad)
{
    int32_t hwiLoad, swiLoad, tskLoad, hndlId, cpuLoad, totalLoad, otherLoad;
    Utils_PrfTsHndl *pHndl;
    System_LinkStatsAccPrfLoadObj *pAccPrfLoadObj;
    uint64_t totalTime, time64, temp;

    /* Reading from Local Object */
    pAccPrfLoadObj = &gUtils_prfObj.accPrfLoadObj;

    hwiLoad = swiLoad = tskLoad = -1;

    totalTime = pAccPrfLoadObj->totalTimeLo & 0xFFFFFFFFU;
    temp = pAccPrfLoadObj->totalTimeHi & 0xFFFFFFFFU;
    totalTime |= (temp << 32);

    time64 = pAccPrfLoadObj->totalHwiThreadTimeLo & 0xFFFFFFFFU;
    temp = pAccPrfLoadObj->totalHwiThreadTimeHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    hwiLoad = (time64 * 1000) / totalTime;

    time64 = pAccPrfLoadObj->totalSwiThreadTimeLo & 0xFFFFFFFFU;
    temp = pAccPrfLoadObj->totalSwiThreadTimeHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    swiLoad = (time64 * 1000) / totalTime;

    time64 = pAccPrfLoadObj->totalIdlTskTimeLo & 0xFFFFFFFFU;
    temp = pAccPrfLoadObj->totalIdlTskTimeHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    cpuLoad = 1000 - ((time64 * 1000) / totalTime);


    totalLoad = hwiLoad+swiLoad;

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        Vps_printf(" \n");
        Vps_printf(" #### EVE CLK = %4d.%-6d Mhz \n",
            Utils_getClkHz(UTILS_CLK_ID_EVE)/1000000,
            Utils_getClkHz(UTILS_CLK_ID_EVE)%1000000
            );
        Vps_printf(" #### DSP CLK = %4d.%-6d Mhz \n",
            Utils_getClkHz(UTILS_CLK_ID_DSP)/1000000,
            Utils_getClkHz(UTILS_CLK_ID_DSP)%1000000
            );
        Vps_printf(" #### IPU CLK = %4d.%-6d Mhz \n",
            Utils_getClkHz(UTILS_CLK_ID_IPU)/1000000,
            Utils_getClkHz(UTILS_CLK_ID_IPU)%1000000
            );
        Vps_printf(" #### A15 CLK = %4d.%-6d Mhz \n",
            Utils_getClkHz(UTILS_CLK_ID_A15)/1000000,
            Utils_getClkHz(UTILS_CLK_ID_A15)%1000000
            );
        Vps_printf(" \n");
    }

#if defined (BUILD_ARP32)
#ifdef CPU_IDLE_ENABLED
    {
        System_EveObj *pObj;
        pObj = &gSystem_objEve;
        /* When the EVE power mode is Auto clock gate do not calculate
         * CPU load as the task loads would not be accurate.
         */
        if (pObj->evePowerMode == UTILS_IDLE_EVE_AUTOCG)
            Vps_printf(" NOTE: EVE Auto Clock Gate Enabled. CPU Load is\
 not accurate in this mode");
    }
#endif
#endif
    Vps_printf(" \n");
    Vps_printf(" LOAD: CPU: %d.%d%% HWI: %d.%d%%, SWI:%d.%d%% \n",
                cpuLoad/10, cpuLoad%10,
                hwiLoad/10, hwiLoad%10,
                swiLoad/10, swiLoad%10
                );

    if (printTskLoad)
    {
        Vps_printf(" \n");

        for (hndlId = 0; hndlId < UTILS_PRF_MAX_TASK_INST; hndlId++)
        {
            pHndl = &gUtils_prfObj.tskHndlObj[hndlId];

            if (TRUE == pHndl->isAlloc)
            {
                tskLoad = -1;

                tskLoad = (pHndl->totalTskThreadTime * 1000) / totalTime;

                totalLoad += tskLoad;

                if (tskLoad)
                {
                    Vps_printf(" LOAD: TSK: %-20s: %d.%d%% \r\n",
                           pHndl->name,
                           tskLoad/10,
                           tskLoad%10
                        );
                }
            }
        }

        otherLoad = cpuLoad - totalLoad;

        Vps_printf(" LOAD: TSK: %-20s: %d.%d%% \r\n",
               "MISC",
               otherLoad/10,
               otherLoad%10
            );
    }
    Vps_printf(" \n");

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Start taking the performance load for all the registered tasks
 *
 *******************************************************************************
 */
Void Utils_prfLoadCalcStart()
{
    uint32_t cookie;

    cookie = Hwi_disable();
    gUtils_startLoadCalc = TRUE;
    Hwi_restore(cookie);
}

/**
 *******************************************************************************
 *
 * \brief Stop taking the load for all the registered tasks
 *
 *******************************************************************************
 */
Void Utils_prfLoadCalcStop()
{
    uint32_t cookie;

    cookie = Hwi_disable();
    gUtils_startLoadCalc = FALSE;
    Hwi_restore(cookie);
}

/**
 *******************************************************************************
 *
 * \brief Reset the load calculation mainly for next cycle of run
 *
 * \return  None
 *
 *******************************************************************************
 */
Void Utils_prfLoadCalcReset()
{
    uint32_t hndlId;
    uint32_t cookie;
    Utils_PrfTsHndl *pHndl;
    System_LinkStatsCorePrfObj *pCorePrfObj;

    /* Global Performance object */
    pCorePrfObj = gUtils_prfObj.pCorePrfLoadObj;
    UTILS_assert(NULL != pCorePrfObj);

    cookie = Hwi_disable();

    /* Reset Local Parameters */
    gUtils_prfObj.accPrfLoadObj.totalHwiThreadTimeHi = 0;
    gUtils_prfObj.accPrfLoadObj.totalSwiThreadTimeHi = 0;
    gUtils_prfObj.accPrfLoadObj.totalTimeHi = 0;
    gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeHi = 0;
    gUtils_prfObj.accPrfLoadObj.totalHwiThreadTimeLo = 0;
    gUtils_prfObj.accPrfLoadObj.totalSwiThreadTimeLo = 0;
    gUtils_prfObj.accPrfLoadObj.totalTimeLo = 0;
    gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeLo = 0;

    /* Reset the performace loads accumulator */
    for (hndlId = 0; hndlId < UTILS_PRF_MAX_TASK_INST; hndlId++)
    {
        pHndl = &gUtils_prfObj.tskHndlObj[hndlId];

        if ((TRUE == pHndl->isAlloc) &&
            (NULL != pHndl->pTsk))
        {
            pHndl->totalTskThreadTime = 0;
        }
    }

    Hwi_restore(cookie);
}

/**
 *******************************************************************************
 *
 * \brief This function processes link statistics commands
 *
 *        Link stat monitor thread sends different command to each link
 *        This function receives this command and processes it.
 *        Currently only command supported is RESET Link Statistics.
 *
 * \param  pCorePrfObj     [IN] Link Stats Instance handle
 *
 * \return None
 *
 *******************************************************************************
 */
static Void UtilsPrf_ProcessLinkStatsCommand(
    System_LinkStatsCorePrfObj *pCorePrfObj)
{
    int32_t status = 0;
    uint32_t cmd, hndlId;
    Utils_PrfTsHndl *pHndlLcl;
    System_LinkStatsPrfLoadObj *pHndlGbl;
    System_LinkStatsAccPrfLoadObj *pAccPrfLoadLcl, *pAccPrfLoadGbl;

    UTILS_assert(NULL != pCorePrfObj);

    status = Utils_linkStatsRecvCommand(&pCorePrfObj->srcToLinkCmdObj, &cmd);

    if (0 == status)
    {
        if (LINK_STATS_CMD_COPY_CORE_LOAD == cmd)
        {
            pAccPrfLoadLcl = &gUtils_prfObj.accPrfLoadObj;
            pAccPrfLoadGbl = &pCorePrfObj->accPrfLoadObj;

            /* Copy Global Parameters */
            pAccPrfLoadGbl->totalHwiThreadTimeHi =
                pAccPrfLoadLcl->totalHwiThreadTimeHi;
            pAccPrfLoadGbl->totalSwiThreadTimeHi =
                pAccPrfLoadLcl->totalSwiThreadTimeHi;
            pAccPrfLoadGbl->totalTimeHi =
                pAccPrfLoadLcl->totalTimeHi;
            pAccPrfLoadGbl->totalIdlTskTimeHi =
                pAccPrfLoadLcl->totalIdlTskTimeHi;

            pAccPrfLoadGbl->totalHwiThreadTimeLo =
                pAccPrfLoadLcl->totalHwiThreadTimeLo;
            pAccPrfLoadGbl->totalSwiThreadTimeLo =
                pAccPrfLoadLcl->totalSwiThreadTimeLo;
            pAccPrfLoadGbl->totalTimeLo =
                pAccPrfLoadLcl->totalTimeLo;
            pAccPrfLoadGbl->totalIdlTskTimeLo =
                pAccPrfLoadLcl->totalIdlTskTimeLo;

            /* Call the load updated function of each registered task one by one
             * along with the swiLoad, hwiLoad, and Task's own load */
            for (hndlId = 0; hndlId < UTILS_PRF_MAX_TASK_INST; hndlId++)
            {
                pHndlLcl = &gUtils_prfObj.tskHndlObj[hndlId];
                pHndlGbl = &gUtils_prfObj.pPrfLoadObj[hndlId];

                if ((TRUE == pHndlLcl->isAlloc) &&
                    (NULL != pHndlLcl->pTsk) &&
                    (0 == strncmp(pHndlGbl->name,
                        pHndlLcl->name,
                        sizeof(pHndlGbl->name))))
                {
                    pHndlGbl->totalTskThreadTimeHi =
                        (pHndlLcl->totalTskThreadTime >> 32) & 0xFFFFFFFFU;
                    pHndlGbl->totalTskThreadTimeLo =
                        (pHndlLcl->totalTskThreadTime) & 0xFFFFFFFFU;
                }
            }

            /* Get the Heap information */
            Utils_memGetHeapStats(UTILS_HEAPID_L2_LOCAL,
                &pCorePrfObj->heapStats[UTILS_HEAPID_L2_LOCAL]);
            Utils_memGetHeapStats(UTILS_HEAPID_DDR_CACHED_LOCAL,
                &pCorePrfObj->heapStats[UTILS_HEAPID_DDR_CACHED_LOCAL]);
            if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
            {
                Utils_memGetHeapStats(UTILS_HEAPID_OCMC_SR,
                    &pCorePrfObj->heapStats[UTILS_HEAPID_OCMC_SR]);
                Utils_memGetHeapStats(UTILS_HEAPID_DDR_CACHED_SR,
                    &pCorePrfObj->heapStats[UTILS_HEAPID_DDR_CACHED_SR]);
                Utils_memGetHeapStats(UTILS_HEAPID_DDR_NON_CACHED_SR0,
                    &pCorePrfObj->heapStats[UTILS_HEAPID_DDR_NON_CACHED_SR0]);
            }

            {
                BspOsal_StaticMemStatus staticMemStatus;

                BspOsal_getStaticMemStatus(&staticMemStatus);

                pCorePrfObj->maxSemaphoreObjs  = staticMemStatus.numMaxSemObjs;
                pCorePrfObj->freeSemaphoreObjs = staticMemStatus.numFreeSemObjs;
                pCorePrfObj->maxTaskObjs      = staticMemStatus.numMaxTaskObjs;
                pCorePrfObj->freeTaskObjs     = staticMemStatus.numFreeTaskObjs;
                pCorePrfObj->maxClockObjs       = staticMemStatus.numMaxClockObjs;
                pCorePrfObj->freeClockObjs      = staticMemStatus.numFreeClockObjs;
                pCorePrfObj->maxHwiObjs        = staticMemStatus.numMaxHwiObjs;
                pCorePrfObj->freeHwiObjs       = staticMemStatus.numFreeHwiObjs;
            }

            /* Copy is done so send copy_done command to monitor thread */
            cmd = LINK_STATS_CMD_COPY_CORE_LOAD_DONE;
            status = Utils_linkStatsSendCommand(
                &pCorePrfObj->linkToSrcCmdObj, cmd);
            /* This call will not fail as queue is empty as the reset
               command is just successfully read */
            if(status!=0)
            {
                Vps_printf(" UTILS_PRF: Unable to send command [0x%04x] !!!\n", cmd);
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief Function called by Loadupdate for each update cycle
 *
 *******************************************************************************
 */
Void Utils_prfLoadUpdate()
{
    Utils_PrfTsHndl *pHndlLcl;
    Load_Stat hwiLoadStat, swiLoadStat, tskLoadStat, idlTskLoadStat;
    Task_Handle idlTskHndl[2] = {NULL};
    UInt32 hndlId;
    System_LinkStatsAccPrfLoadObj *pAccPrfLoadLcl;
    uint64_t time64, temp;

#if defined (BUILD_ARP32)
#ifdef CPU_IDLE_ENABLED
    System_EveObj *pObj;
    pObj = &gSystem_objEve;
    /* When the EVE power mode is Auto clock gate do not calculate
     * CPU load as the task loads would not be accurate.
     */
    if (pObj->evePowerMode == UTILS_IDLE_EVE_AUTOCG)
        return;
#endif
#endif

    if (TRUE == gUtils_startLoadCalc)
    {
        idlTskHndl[0] = Task_getIdleTaskHandle(0);

        /* Get the Local Object */
        pAccPrfLoadLcl = &gUtils_prfObj.accPrfLoadObj;

        /* Get the all loads first */
        Load_getGlobalHwiLoad(&hwiLoadStat);
        Load_getGlobalSwiLoad(&swiLoadStat);
        Load_getTaskLoad(idlTskHndl[0], &idlTskLoadStat);

        time64 = pAccPrfLoadLcl->totalHwiThreadTimeLo & 0xFFFFFFFFU;
        temp = pAccPrfLoadLcl->totalHwiThreadTimeHi & 0xFFFFFFFFU;
        time64 |= (temp << 32);
        time64 += hwiLoadStat.threadTime;
        pAccPrfLoadLcl->totalHwiThreadTimeLo = time64 & 0xFFFFFFFFU;
        pAccPrfLoadLcl->totalHwiThreadTimeHi = (time64 >> 32) & 0xFFFFFFFFU;

        time64 = pAccPrfLoadLcl->totalSwiThreadTimeLo & 0xFFFFFFFFU;
        temp = pAccPrfLoadLcl->totalSwiThreadTimeHi & 0xFFFFFFFFU;
        time64 |= (temp << 32);
        time64 += swiLoadStat.threadTime;
        pAccPrfLoadLcl->totalSwiThreadTimeLo = time64 & 0xFFFFFFFFU;
        pAccPrfLoadLcl->totalSwiThreadTimeHi = (time64 >> 32) & 0xFFFFFFFFU;

        time64 = pAccPrfLoadLcl->totalTimeLo & 0xFFFFFFFFU;
        temp = pAccPrfLoadLcl->totalTimeHi & 0xFFFFFFFFU;
        time64 |= (temp << 32);
        time64 += hwiLoadStat.totalTime;
        pAccPrfLoadLcl->totalTimeLo = time64 & 0xFFFFFFFFU;
        pAccPrfLoadLcl->totalTimeHi = (time64 >> 32) & 0xFFFFFFFFU;

        time64 = pAccPrfLoadLcl->totalIdlTskTimeLo & 0xFFFFFFFFU;
        temp = pAccPrfLoadLcl->totalIdlTskTimeHi & 0xFFFFFFFFU;
        time64 |= (temp << 32);
        time64 += idlTskLoadStat.threadTime;
        pAccPrfLoadLcl->totalIdlTskTimeLo = time64 & 0xFFFFFFFFU;
        pAccPrfLoadLcl->totalIdlTskTimeHi = (time64 >> 32) & 0xFFFFFFFFU;

#ifdef DUAL_A15_SMP_BIOS_INCLUDE
        if (System_getSelfProcId()==SYSTEM_PROC_A15_0)
        {
            Load_Stat idlTskLoadStatCore1;
            idlTskHndl[1] = Task_getIdleTaskHandle(1);
            Load_getTaskLoad(idlTskHndl[1], &idlTskLoadStatCore1);

            time64 = pAccPrfLoadLcl->totalIdlTskTimeLo & 0xFFFFFFFFU;
            temp = pAccPrfLoadLcl->totalIdlTskTimeHi & 0xFFFFFFFFU;
            time64 |= (temp << 32);
            time64 += idlTskLoadStatCore1.threadTime;
            pAccPrfLoadLcl->totalIdlTskTimeLo = time64 & 0xFFFFFFFFU;
            pAccPrfLoadLcl->totalIdlTskTimeHi = (time64 >> 32) & 0xFFFFFFFFU;

            time64 = pAccPrfLoadLcl->totalTimeLo & 0xFFFFFFFFU;
            temp = pAccPrfLoadLcl->totalTimeHi & 0xFFFFFFFFU;
            time64 |= (temp << 32);
            time64 += hwiLoadStat.totalTime;
            pAccPrfLoadLcl->totalTimeLo = time64 & 0xFFFFFFFFU;
            pAccPrfLoadLcl->totalTimeHi = (time64 >> 32) & 0xFFFFFFFFU;
        }
#endif

        /* Call the load updated function of each registered task one by one
         * along with the swiLoad, hwiLoad, and Task's own load */
        for (hndlId = 0; hndlId < UTILS_PRF_MAX_TASK_INST; hndlId++)
        {
            /* Get the local object */
            pHndlLcl = &gUtils_prfObj.tskHndlObj[hndlId];

            if ((TRUE == pHndlLcl->isAlloc) &&
                (NULL != pHndlLcl->pTsk))
            {
                Load_getTaskLoad(pHndlLcl->pTsk, &tskLoadStat);
                pHndlLcl->totalTskThreadTime += tskLoadStat.threadTime;
            }
        }

        /* Check and process command received from monitor thread */
        UtilsPrf_ProcessLinkStatsCommand(gUtils_prfObj.pCorePrfLoadObj);
    }
}

/**
 *******************************************************************************
 *
 * \brief Reset latency stats
 *
 * \param  lStats    [OUT] latency statistics
 *
 *******************************************************************************
 */
Void Utils_resetLatency(Utils_LatencyStats *lStats)
{
    lStats->accumulatedLatencyHi = lStats->accumulatedLatencyLo = 0;
    lStats->minLatencyHi = lStats->minLatencyLo = 0xFFFFFFFF;
    lStats->maxLatencyHi = lStats->maxLatencyLo = 0x0;
    lStats->countHi = lStats->countLo = 0;
}

/**
 *******************************************************************************
 *
 * \brief Calculate latency
 *
 * \param  lStats    [OUT] latency statistics
 * \param  linkLocalTime     [IN]  time at which frame was received at the link
 *
 *******************************************************************************
 */
Void Utils_updateLatency(Utils_LatencyStats *lStats,
                         uint64_t linkLocalTime)
{
    uint64_t latency;
    uint64_t curTime = Utils_getCurGlobalTimeInUsec();
    uint64_t time64, temp;

    latency = curTime - linkLocalTime;

    time64 = lStats->minLatencyLo & 0xFFFFFFFFU;
    temp = lStats->minLatencyHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    if (time64 > latency)
    {
        lStats->minLatencyHi = (latency >> 32) & 0xFFFFFFFFU;
        lStats->minLatencyLo = (latency) & 0xFFFFFFFFU;
    }

    time64 = lStats->maxLatencyLo & 0xFFFFFFFFU;
    temp = lStats->maxLatencyHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    if (latency > time64)
    {
        lStats->maxLatencyHi = (latency >> 32) & 0xFFFFFFFFU;
        lStats->maxLatencyLo = (latency) & 0xFFFFFFFFU;
    }

    time64 = lStats->accumulatedLatencyLo & 0xFFFFFFFFU;
    temp = lStats->accumulatedLatencyHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    time64 += latency;
    lStats->accumulatedLatencyHi = (time64 >> 32) & 0xFFFFFFFFU;
    lStats->accumulatedLatencyLo = (time64) & 0xFFFFFFFFU;

    time64 = lStats->countLo & 0xFFFFFFFFU;
    temp = lStats->countHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    time64 ++;
    lStats->countHi = (time64 >> 32) & 0xFFFFFFFFU;
    lStats->countLo = (time64) & 0xFFFFFFFFU;

}

/**
 *******************************************************************************
 *
 * \brief Print the latency statistics
 *
 * \param  name              [IN] Name of module
 * \param  localLinkstats    [IN] local link latency
 * \param  srcToLinkstats    [IN] source to link latency
 * \param  resetStats        [IN] TRUE: reset counters to 0 after printing
 *
 *******************************************************************************
 */
Void Utils_printLatency(char *name,
                        Utils_LatencyStats *localLinkstats,
                        Utils_LatencyStats *srcToLinkstats,
                        Bool resetStats)
{
    uint64_t accLatency64, count64, temp;

    /* Divide by 1000 is done to convert micro second to millisecond */
    Vps_printf( " \n");
    Vps_printf( " [ %s ] LATENCY,\n", name);
    Vps_printf( " ********************\n", name);

    count64 = localLinkstats->countLo & 0xFFFFFFFFU;
    temp = localLinkstats->countHi & 0xFFFFFFFFU;
    count64 |= (temp << 32);
    if(count64)
    {
        accLatency64 = localLinkstats->accumulatedLatencyLo & 0xFFFFFFFFU;
        temp = localLinkstats->accumulatedLatencyHi & 0xFFFFFFFFU;
        accLatency64 |= (temp << 32);

        Vps_printf( " Local Link Latency     : Avg = %6d us, Min = %6d us, Max = %6d us, \r\n",
            (uint32_t)(accLatency64/count64),
            localLinkstats->minLatencyLo,
            localLinkstats->maxLatencyLo);
    }

    count64 = srcToLinkstats->countLo & 0xFFFFFFFFU;
    temp = srcToLinkstats->countHi & 0xFFFFFFFFU;
    count64 |= (temp << 32);
    if(count64)
    {
        accLatency64 = srcToLinkstats->accumulatedLatencyLo & 0xFFFFFFFFU;
        temp = srcToLinkstats->accumulatedLatencyHi & 0xFFFFFFFFU;
        accLatency64 |= (temp << 32);

        Vps_printf( " Source to Link Latency : Avg = %6d us, Min = %6d us, Max = %6d us, \r\n",
            (uint32_t)(accLatency64/count64),
            srcToLinkstats->minLatencyLo,
            srcToLinkstats->maxLatencyLo);
    }
    Vps_printf( " \n");

    if (resetStats)
    {
        Utils_resetLatency(localLinkstats);
        Utils_resetLatency(srcToLinkstats);
    }

}

/**
 *******************************************************************************
 *
 * \brief Divide between a count value and div value
 *
 *        Returns in units as explained below
 *        - 3000 = 30.00
 *        - 2997 = 29.97
 *
 * \param  count        [IN] count
 * \param  divValue     [IN] divisor
 *
 * \return FPS in units of XXX.DD
 *
 *******************************************************************************
 */

uint32_t Utils_calcFps(uint32_t count, uint32_t divValue)
{
    uint32_t fps, mult, div;

    /*
     * multiplier and divider is selected based on precision possible in a
     * 32-bit count value
     * i.e make sure count*mult does not overflow the 32-bit value
     *
     */
    if(count < 40000)
    {
        mult = 1000*100;
        div  = 1;
    }
    else
    if(count < 400000)
    {
        mult = 100*100;
        div  = 10;
    }
    else
    if(count < 4000000)
    {
        mult = 10*100;
        div  = 100;
    }
    else
    if(count < 40000000)
    {
        mult = 1*100;
        div  = 1000;
    }
    else
    if(count < 400000000)
    {
        mult = 1*10;
        div  = 10000;
    }
    else
    {
        mult = 1*1;
        div  = 100000;
    }

    div = divValue / div;

    if(div==0)
        fps = 0;
    else
        fps = (count * mult) / div;

    return fps;
}

/**
 *******************************************************************************
 *
 * \brief Reset the link statistics
 *
 * \param  pPrm         [OUT] link statistics
 * \param  numCh        [IN]  Number of channels for which statistics
 *                            is collected
 * \param  numOut       [IN]  Number of outputs associated with this channel
 *
 *******************************************************************************
 */
Void Utils_resetLinkStatistics(Utils_LinkStatistics *pPrm,
                                uint32_t numCh,
                                uint32_t numOut)
{
    Utils_LinkChStatistics *pChStats;
    uint32_t chId, outId;


    pPrm->numCh = numCh;

    pPrm->inBufErrorCount = 0;
    pPrm->outBufErrorCount = 0;
    pPrm->newDataCmdCount = 0;
    pPrm->releaseDataCmdCount = 0;
    pPrm->getFullBufCount = 0;
    pPrm->putEmptyBufCount = 0;
    pPrm->notifyEventCount = 0;

    for(chId=0; chId<pPrm->numCh; chId++)
    {
        pChStats = &pPrm->chStats[chId];

        pChStats->numOut = numOut;
        pChStats->inBufRecvCount = 0;
        pChStats->inBufDropCount = 0;
        pChStats->inBufUserDropCount = 0;
        pChStats->inBufProcessCount = 0;

        for(outId=0; outId<pChStats->numOut; outId++)
        {
            pChStats->outBufCount[outId] = 0;
            pChStats->outBufDropCount[outId] = 0;
            pChStats->outBufUserDropCount[outId] = 0;
        }
    }

    /* reset stats Start time */
    pPrm->statsStartTime = Utils_getCurGlobalTimeInMsec();
}

/**
 *******************************************************************************
 *
 * \brief Print the link statistics
 *
 * \param  pPrm         [IN] link statistics
 * \param  name         [IN] Link task name
 * \param  resetStats   [IN] TRUE, Reset stats after print
 *
 *******************************************************************************
 */
Void Utils_printLinkStatistics(Utils_LinkStatistics *pPrm, char *name,
                                Bool resetStats)
{
    uint32_t elaspedTime;
    uint32_t chId, outId;
    Utils_LinkChStatistics *pChStats;
    uint32_t value1, value2, value3, value4;

    elaspedTime = Utils_getCurGlobalTimeInMsec() - pPrm->statsStartTime;

    Vps_printf(" \r\n");
    Vps_printf(" [ %s ] Link Statistics,\r\n", name);
    Vps_printf(" ******************************\r\n");
    Vps_printf(" \r\n");
    Vps_printf(" Elapsed time       = %d msec\r\n", elaspedTime);
    Vps_printf(" \r\n");

    if(pPrm->inBufErrorCount)
        Vps_printf(" In Buf Error Count = %d frames\r\n", pPrm->inBufErrorCount);

    if(pPrm->outBufErrorCount)
        Vps_printf(" Out Buf Error Count = %d frames\r\n", pPrm->outBufErrorCount);

    if(pPrm->newDataCmdCount)
    {
        value1 = Utils_calcFps(pPrm->newDataCmdCount, elaspedTime);

        Vps_printf(" New data Recv      = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->releaseDataCmdCount)
    {
        value1 = Utils_calcFps(pPrm->releaseDataCmdCount, elaspedTime);

        Vps_printf(" Release data Recv  = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->getFullBufCount)
    {
        value1 = Utils_calcFps(pPrm->getFullBufCount, elaspedTime);

        Vps_printf(" Get Full Buf Cb    = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->putEmptyBufCount)
    {
        value1 = Utils_calcFps(pPrm->putEmptyBufCount, elaspedTime);

        Vps_printf(" Put Empty Buf Cb   = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->notifyEventCount)
    {
        value1 = Utils_calcFps(pPrm->notifyEventCount, elaspedTime);

        Vps_printf(" Driver/Notify Cb   = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->numCh)
    {
        Vps_printf(" \r\n");
        Vps_printf(" Input Statistics,\r\n");
        Vps_printf(" \r\n");
        Vps_printf(" CH | In Recv | In Drop | In User Drop | In Process \r\n");
        Vps_printf("    | FPS     | FPS     | FPS          | FPS        \r\n");
        Vps_printf(" -------------------------------------------------- \r\n");

        for(chId=0; chId<pPrm->numCh; chId++)
        {
            pChStats = &pPrm->chStats[chId];

            if(pChStats->inBufRecvCount ||
                pChStats->inBufDropCount ||
                pChStats->inBufUserDropCount ||
                pChStats->inBufProcessCount
                )
            {
                value1 = Utils_calcFps(pChStats->inBufRecvCount, elaspedTime);
                value2 = Utils_calcFps(pChStats->inBufDropCount, elaspedTime);
                value3 = Utils_calcFps(pChStats->inBufUserDropCount, elaspedTime);
                value4 = Utils_calcFps(pChStats->inBufProcessCount, elaspedTime);

                Vps_printf(" %2d | %3d.%2d    %3d.%2d    %3d.%2d         %3d.%2d \r\n",
                            chId,
                            value1/100, value1%100,
                            value2/100, value2%100,
                            value3/100, value3%100,
                            value4/100, value4%100);
            }
        }
    }

    if(pPrm->numCh && pPrm->chStats[0].numOut)
    {
        Vps_printf(" \r\n");
        Vps_printf(" Output Statistics,\r\n");
        Vps_printf(" \r\n");
        Vps_printf(" CH | Out | Out     | Out Drop | Out User Drop \r\n");
        Vps_printf("    | ID  | FPS     | FPS      | FPS           \r\n");
        Vps_printf(" --------------------------------------------- \r\n");

        for(chId=0; chId<pPrm->numCh; chId++)
        {
            pChStats = &pPrm->chStats[chId];

            for(outId=0; outId<pChStats->numOut; outId++)
            {
                if(pChStats->outBufCount[outId] ||
                    pChStats->outBufDropCount[outId] ||
                    pChStats->outBufUserDropCount[outId]
                    )
                {
                    value1 = Utils_calcFps(
                                pChStats->outBufCount[outId],
                                elaspedTime);

                    value2 = Utils_calcFps(
                                pChStats->outBufDropCount[outId],
                                elaspedTime);

                    value3 = Utils_calcFps(
                                pChStats->outBufUserDropCount[outId],
                                elaspedTime);

                    Vps_printf( " %2d | %2d    %3d.%2d   %3d.%2d    %3d.%2d \r\n",
                        chId,
                        outId,
                        value1/100, value1%100,
                        value2/100, value2%100,
                        value3/100, value3%100
                        );
                }
            }
        }
    }

    if(resetStats)
    {
        /* assume number of outputs = number of outputs of CH0 */
        Utils_resetLinkStatistics(pPrm, pPrm->numCh, pPrm->chStats[0].numOut);
    }
}

#if defined (BUILD_ARP32)
/**
 *******************************************************************************
 *
 * \brief Update the EVE CPU load values when the EVE Auto Clock Gating is
 *        enabled.
 *
 * \param  totalTime   [IN] The total Time calculated before going to Auto CG
 *
 *******************************************************************************
 */
Void Utils_prfUpdateEveLoadPreAutoCg(UInt64 totalTime)
{
    uint64_t time64, temp;
    time64 = gUtils_prfObj.accPrfLoadObj.totalTimeLo & 0xFFFFFFFFU;
    temp = gUtils_prfObj.accPrfLoadObj.totalTimeHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    time64 += totalTime;
    gUtils_prfObj.accPrfLoadObj.totalTimeLo = time64 & 0xFFFFFFFFU;
    gUtils_prfObj.accPrfLoadObj.totalTimeHi = (time64 >> 32) & 0xFFFFFFFFU;
    UtilsPrf_ProcessLinkStatsCommand(gUtils_prfObj.pCorePrfLoadObj);
}

/**
 *******************************************************************************
 *
 * \brief Update the EVE CPU load values when the EVE Auto Clock Gating is
 *        enabled.
 *
 * \param  totalTimeIdle  [IN] The total Time calculated after coming out of
 *                             Auto CG
 *
 *******************************************************************************
 */
Void Utils_prfUpdateEveLoadPostAutoCg(UInt64 totalTimeIdle)
{
    uint64_t time64, temp;
    time64 = gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeLo & 0xFFFFFFFFU;
    temp = gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    time64 += totalTimeIdle;
    gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeLo = time64 & 0xFFFFFFFFU;
    gUtils_prfObj.accPrfLoadObj.totalIdlTskTimeHi = (time64 >> 32) & 0xFFFFFFFFU;

    time64 = gUtils_prfObj.accPrfLoadObj.totalTimeLo & 0xFFFFFFFFU;
    temp = gUtils_prfObj.accPrfLoadObj.totalTimeHi & 0xFFFFFFFFU;
    time64 |= (temp << 32);
    time64 += totalTimeIdle;
    gUtils_prfObj.accPrfLoadObj.totalTimeLo = time64 & 0xFFFFFFFFU;
    gUtils_prfObj.accPrfLoadObj.totalTimeHi = (time64 >> 32) & 0xFFFFFFFFU;
}

#endif

