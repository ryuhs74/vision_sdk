/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file utils_stat_collector.c
 *
 * \brief  This file has the implementataion of statCollector task.
 *
 *
 * \version 0.0 (Jan 2014) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "utils_stat_collector_prv.h"
#include "utils_stat_collector_cfg.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gUtilsStatColl_tskStack, 32)
#pragma DATA_SECTION(gUtilsStatColl_tskStack, ".bss:taskStackSection")
UInt8 gUtilsStatColl_tskStack[UTILS_STAT_COLL_TSK_STACK_SIZE];


Utils_StatCollCtrl gUtils_statCollCtrl;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief   Comuputes the average of stat collector values
 *
 *          Function caluclates the average values and updates them once in
 *          every SAMPLING_WINDOW_WIDTH interval.
 *
 *
 *******************************************************************************
 */
Void Utils_statColCompute(UInt32 statColType,
                               UInt32 diff,
                               UInt32 uc_idx,
                               UInt32 cfg_idx,
                               UInt64 elaspedTimeAvg,
                               UInt64 elaspedTimePeak)
{
    UInt32 i = 0;
    UInt32 oldIntState;
    UInt64 tmpPeak = 0;

    if(diff==0)
        return;

    if( statColType == SDRAM)
    {
        Utils_StatCollSdramCfg *pCfg;

        pCfg = &gUtils_statCollCtrl.sdramCfg[cfg_idx];

        for( i= 0;
             i < pCfg->data[uc_idx].numCounters;
             i++)
        {
            pCfg->total[uc_idx] += diff;

            if(elaspedTimePeak > 1000)
            {
                /* only compute peak if elasped time is > 1ms
                 * to avoid dvision errors
                 */
                tmpPeak = ((UInt64)diff*1000000)/elaspedTimePeak;

                if(tmpPeak > pCfg->data[uc_idx].peak[i])
                    pCfg->data[uc_idx].peak[i] = tmpPeak;
            }

            if(elaspedTimeAvg/1000 > SAMPLING_WINDOW_WIDTH)
            {
                oldIntState = Hwi_disable();

                pCfg->data[uc_idx].avg[i]
                    = (pCfg->total[uc_idx]*1000000)/elaspedTimeAvg;

                pCfg->data[uc_idx].peakRate[i]
                    = pCfg->data[uc_idx].peak[i];

                pCfg->total[uc_idx] = 0;
                pCfg->data[uc_idx].peak[i] = 0;

                Hwi_restore(oldIntState);
            }
        }
    }
    else if(statColType == MSTR)
    {
        Utils_StatCollMstrCfg *pCfg;

        pCfg = &gUtils_statCollCtrl.mstrCfg[cfg_idx];

        for( i= 0;
             i < pCfg->data[uc_idx].numCounters;
             i++)
        {
            pCfg->total[uc_idx] += diff;
            pCfg->count[uc_idx] ++;

            if(pCfg->cfg[uc_idx].cfg.usecase==SCI_MSTR_THROUGHPUT)
            {
                if(elaspedTimePeak > 1000)
                {
                    /* only compute peak if elasped time is > 1ms
                     * to avoid dvision errors
                     */

                    tmpPeak = ((UInt64)diff*1000000)/elaspedTimePeak;
                }
            }
            else
            {
                tmpPeak = diff;
            }

            if(tmpPeak > pCfg->data[uc_idx].peak[i])
                pCfg->data[uc_idx].peak[i] = tmpPeak;

            if(elaspedTimeAvg/1000 > SAMPLING_WINDOW_WIDTH)
            {
                oldIntState = Hwi_disable();

                if(pCfg->cfg[uc_idx].cfg.usecase==SCI_MSTR_THROUGHPUT)
                {
                    pCfg->data[uc_idx].avg[i]
                        = (pCfg->total[uc_idx]*1000000)/elaspedTimeAvg;

                    pCfg->data[uc_idx].peakRate[i]
                        = pCfg->data[uc_idx].peak[i];
                }
                else
                {
                    pCfg->data[uc_idx].avg[i]
                        = pCfg->total[uc_idx]/pCfg->count[uc_idx];

                    pCfg->data[uc_idx].peakRate[i]
                        = pCfg->data[uc_idx].peak[i];
                }

                pCfg->total[uc_idx] = 0;
                pCfg->count[uc_idx] = 0;
                pCfg->data[uc_idx].peak[i] = 0;

                Hwi_restore(oldIntState);
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Reset the stat collector values
 *
 *          Function itrates through a loop of number of statcol usecase and
 *          resets the values.
 *
 *
 *******************************************************************************
*/
Void Utils_statCollectorReset()
{
    gUtils_statCollCtrl.doReset = TRUE;
}


Void Utils_statCollectorResetCounters()
{
    UInt32 uc_idx, cfg_idx;
    UInt32 tmpNumCounters;
    UInt32 oldIntState;

    oldIntState = Hwi_disable();

    for(cfg_idx=0; cfg_idx<MAX_SDRAM_CFGS; cfg_idx++)
    {
        Utils_StatCollSdramCfg *pCfg;

        pCfg = &gUtils_statCollCtrl.sdramCfg[cfg_idx];

        for(uc_idx = 0; uc_idx < pCfg->num_uc; uc_idx++)
        {
            tmpNumCounters = pCfg->data[uc_idx].numCounters;

            memset( &pCfg->data[uc_idx],
                    0,
                    sizeof(pCfg->data[uc_idx])
                );

            memset( &pCfg->total[uc_idx],
                    0,
                    sizeof(pCfg->total[uc_idx])
                );

            pCfg->data[uc_idx].numCounters = tmpNumCounters;
        }
    }

    for(cfg_idx=0; cfg_idx<MAX_MSTR_CFGS; cfg_idx++)
    {
        Utils_StatCollMstrCfg *pCfg;

        pCfg = &gUtils_statCollCtrl.mstrCfg[cfg_idx];

        for(uc_idx = 0; uc_idx < pCfg->num_uc; uc_idx++)
        {
            tmpNumCounters = pCfg->data[uc_idx].numCounters;

            memset( &pCfg->data[uc_idx],
                    0,
                    sizeof(pCfg->data[uc_idx])
                );

            memset( &pCfg->total[uc_idx],
                    0,
                    sizeof(pCfg->total[uc_idx])
                );

            memset( &pCfg->count[uc_idx],
                    0,
                    sizeof(pCfg->count[uc_idx])
                );

            pCfg->data[uc_idx].numCounters = tmpNumCounters;
        }
    }

    Hwi_restore(oldIntState);
}


Void Utils_statCollectorPrintThroughPut(char *descr,
                        UInt64 avg,
                        UInt64 peak)
{
    Vps_printf(" %-20s | %6llu.%06d %6llu.%06d\n",
        descr,
        avg/1000000,
        avg%1000000,
        peak/1000000,
        peak%1000000
    );
}

Void Utils_statCollectorPrintLatency(char *descr,
                        UInt64 avg,
                        UInt64 peak)
{
    Vps_printf(" %-20s | %6llu        %6llu cycles\n",
        descr,
        avg,
        peak
    );
}

/**
 *******************************************************************************
 *
 * \brief   Prints the stat collector values
 *
 *          Function itrates through a loop of number of statcol usecase and
 *          prints the averaged values. These are been updated once in
 *          SAMPLING_WINDOW_WIDTH duration.
 *
 *
 *******************************************************************************
*/
Void Utils_statCollectorPrintCount()
{
    Int32 uc_idx = 0;
    Int32 num_counters = 0;
    UInt32 cfg_idx;

    Vps_printf(" \r\n");
    Vps_printf(" Statistics Collector,\r\n");
    Vps_printf(" \r\n");
    Vps_printf("       STATISTIC          Avg Data        Peak Data \r\n");
    Vps_printf("       COLLECTOR          MB/s            MB/s      \r\n");
    Vps_printf(" -------------------------------------------------- \r\n");

    for(cfg_idx=0; cfg_idx<MAX_SDRAM_CFGS; cfg_idx++)
    {
        Utils_StatCollSdramCfg *pCfg;

        pCfg = &gUtils_statCollCtrl.sdramCfg[cfg_idx];

        for(uc_idx = 0; uc_idx < pCfg->num_uc; uc_idx++)
        {
            num_counters = pCfg->data[uc_idx].numCounters;

            switch(num_counters)
            {
                case 1  :
                    Utils_statCollectorPrintThroughPut(
                        pCfg->cfg[uc_idx].descr,
                        pCfg->data[uc_idx].avg[0],
                        pCfg->data[uc_idx].peakRate[0]
                    );
                    break;

                case 2  :
                    Utils_statCollectorPrintThroughPut(
                        pCfg->cfg[uc_idx].descr,
                        pCfg->data[uc_idx].avg[0],
                        pCfg->data[uc_idx].peakRate[0]
                    );
                    Utils_statCollectorPrintThroughPut(
                        pCfg->cfg[uc_idx].descr,
                        pCfg->data[uc_idx].avg[1],
                        pCfg->data[uc_idx].peakRate[1]
                    );
                    break;
            }
        }
    }
    for(cfg_idx=0; cfg_idx<MAX_MSTR_CFGS; cfg_idx++)
    {
        Utils_StatCollMstrCfg *pCfg;

        pCfg = &gUtils_statCollCtrl.mstrCfg[cfg_idx];

        for(uc_idx = 0; uc_idx < pCfg->num_uc; uc_idx++)
        {
            num_counters = pCfg->data[uc_idx].numCounters;

            switch(num_counters)
            {
                case 1  :
                    if(pCfg->cfg[uc_idx].cfg.usecase==SCI_MSTR_THROUGHPUT)
                    {
                        Utils_statCollectorPrintThroughPut(
                            pCfg->cfg[uc_idx].descr,
                            pCfg->data[uc_idx].avg[0],
                            pCfg->data[uc_idx].peakRate[0]
                        );
                    }
                    else
                    {
                        Utils_statCollectorPrintLatency(
                            pCfg->cfg[uc_idx].descr,
                            pCfg->data[uc_idx].avg[0],
                            pCfg->data[uc_idx].peakRate[0]
                        );

                    }
                    break;

                case 2  :
                    if(pCfg->cfg[uc_idx].cfg.usecase==SCI_MSTR_THROUGHPUT)
                    {
                        Utils_statCollectorPrintThroughPut(
                            pCfg->cfg[uc_idx].descr,
                            pCfg->data[uc_idx].avg[0],
                            pCfg->data[uc_idx].peakRate[0]
                        );
                        Utils_statCollectorPrintThroughPut(
                            pCfg->cfg[uc_idx].descr,
                            pCfg->data[uc_idx].avg[1],
                            pCfg->data[uc_idx].peakRate[1]
                        );

                    }
                    else
                    {
                        Utils_statCollectorPrintLatency(
                            pCfg->cfg[uc_idx].descr,
                            pCfg->data[uc_idx].avg[0],
                            pCfg->data[uc_idx].peakRate[0]
                        );
                        Utils_statCollectorPrintLatency(
                            pCfg->cfg[uc_idx].descr,
                            pCfg->data[uc_idx].avg[1],
                            pCfg->data[uc_idx].peakRate[1]
                        );
                    }
                    break;
            }
        }
    }
}

Void Utils_statCollectorRegisterMstr(UInt32 cfg_idx)
{
    Int32 uc_idx;
    Int32 total_cntrs = 0;
    enum sci_err system_sci_err;

    struct sci_config_mstr *pSciCfg = NULL;
    Utils_StatCollMstrCfg *pCfg;

    pCfg = &gUtils_statCollCtrl.mstrCfg[cfg_idx];

    for(uc_idx = 0;
        uc_idx < pCfg->num_uc;
        uc_idx++ )
    {
        pSciCfg = &pCfg->cfg[uc_idx].cfg;

        system_sci_err = sci_reg_usecase_mstr(
                                    gUtils_statCollCtrl.psci_hdl,
                                    pSciCfg,
                                    &pCfg->uc_key[uc_idx]
                                    );

        UTILS_assert(system_sci_err == SCI_SUCCESS);

        sci_dump_info(gUtils_statCollCtrl.psci_hdl,
                        &pCfg->uc_key[uc_idx],
                        1,
                        &total_cntrs);

        pCfg->data[uc_idx].numCounters = total_cntrs;
    }
}

Void Utils_statCollectorRegisterSdram(UInt32 cfg_idx)
{
    Int32 uc_idx;
    Int32 total_cntrs = 0;
    enum sci_err system_sci_err;

    struct sci_config_sdram *pSciCfg = NULL;
    Utils_StatCollSdramCfg *pCfg;

    pCfg = &gUtils_statCollCtrl.sdramCfg[cfg_idx];

    for(uc_idx = 0;
        uc_idx < pCfg->num_uc;
        uc_idx++ )
    {
        pSciCfg = &pCfg->cfg[uc_idx].cfg;

        system_sci_err = sci_reg_usecase_sdram(
                                    gUtils_statCollCtrl.psci_hdl,
                                    pSciCfg,
                                    &pCfg->uc_key[uc_idx]
                                    );

        UTILS_assert(system_sci_err == SCI_SUCCESS);

        sci_dump_info(gUtils_statCollCtrl.psci_hdl,
                        &pCfg->uc_key[uc_idx],
                        1,
                        &total_cntrs);

        pCfg->data[uc_idx].numCounters = total_cntrs;
    }
}

Void Utils_statCollectorUnRegisterMstr(UInt32 cfg_idx)
{
    Int32 uc_idx;
    enum sci_err system_sci_err;
    Utils_StatCollMstrCfg *pCfg;

    pCfg = &gUtils_statCollCtrl.mstrCfg[cfg_idx];

    for(uc_idx = 0;
        uc_idx < pCfg->num_uc;
        uc_idx++ )
    {
        system_sci_err = sci_remove_usecase(
                                    gUtils_statCollCtrl.psci_hdl,
                                    &pCfg->uc_key[uc_idx]
                            );

        UTILS_assert(system_sci_err == SCI_SUCCESS);
    }
}

Void Utils_statCollectorUnRegisterSdram(UInt32 cfg_idx)
{
    Int32 uc_idx;
    enum sci_err system_sci_err;
    Utils_StatCollSdramCfg *pCfg;

    pCfg = &gUtils_statCollCtrl.sdramCfg[cfg_idx];

    for(uc_idx = 0;
        uc_idx < pCfg->num_uc;
        uc_idx++ )
    {
        system_sci_err = sci_remove_usecase(
                                    gUtils_statCollCtrl.psci_hdl,
                                    &pCfg->uc_key[uc_idx]
                            );

        UTILS_assert(system_sci_err == SCI_SUCCESS);
    }
}

/**
 *******************************************************************************
 *
 * \brief   Stat Collector Task Function
 *
 *          This functions executes on saperate thread
 *
 *          Stat Collector is initialized and started.
 *          Periodically the values are optamed avgrage is computed.
 *
 *
 *******************************************************************************
*/
Void Utils_statCollectorTskMain(UArg arg0, UArg arg1)
{
    struct sci_config system_sci_config;
    enum sci_err system_sci_err;
    Utils_StatCollSdramCfg *pSdramCfg;
    Utils_StatCollMstrCfg *pMstrCfg;

    UInt32 mstr_cfg_idx, sdram_cfg_idx, uc_idx;
    UInt64 elaspedTimeAvg, elaspedTimePeak;

    gUtils_statCollCtrl.mstrCfg[0].num_uc = UTILS_ARRAYSIZE(mstr_cfg_array_0);
    gUtils_statCollCtrl.mstrCfg[0].cfg    = mstr_cfg_array_0;

    gUtils_statCollCtrl.mstrCfg[1].num_uc = UTILS_ARRAYSIZE(mstr_cfg_array_1);
    gUtils_statCollCtrl.mstrCfg[1].cfg    = mstr_cfg_array_1;

#ifdef TDA3XX_FAMILY_BUILD
    gUtils_statCollCtrl.sdramCfg[0].num_uc = 0;
    gUtils_statCollCtrl.sdramCfg[0].cfg    = NULL;
#else
    gUtils_statCollCtrl.sdramCfg[0].num_uc = UTILS_ARRAYSIZE(sdram_cfg_array_0);
    gUtils_statCollCtrl.sdramCfg[0].cfg    = sdram_cfg_array_0;
#endif
    system_sci_config.errhandler = NULL;
    system_sci_config.data_options = 0;
    system_sci_config.sdram_msg_rate = 8192;
    system_sci_config.mstr_msg_rate = 256;
    system_sci_config.trigger_enable = false;
    system_sci_config.mode = SCI_MODE_DUMP;

    system_sci_err = sci_open(&gUtils_statCollCtrl.psci_hdl, &system_sci_config);
    UTILS_assert(system_sci_err == SCI_SUCCESS);

    mstr_cfg_idx = 0;
    sdram_cfg_idx = 0;

    Utils_statCollectorRegisterMstr(mstr_cfg_idx);
    Utils_statCollectorRegisterSdram(sdram_cfg_idx);

    system_sci_err = sci_global_enable(gUtils_statCollCtrl.psci_hdl);
    UTILS_assert(system_sci_err == SCI_SUCCESS);

    Utils_statCollectorReset();

    while(!gUtils_statCollCtrl.doStop)
    {
        pMstrCfg  = &gUtils_statCollCtrl.mstrCfg[mstr_cfg_idx];
        pSdramCfg = &gUtils_statCollCtrl.sdramCfg[sdram_cfg_idx];

        if(gUtils_statCollCtrl.doReset)
        {
            gUtils_statCollCtrl.doReset = FALSE;
            Utils_statCollectorResetCounters();
            sci_dump(gUtils_statCollCtrl.psci_hdl);
            gUtils_statCollCtrl.startTimeAvg = Utils_getCurGlobalTimeInUsec();
            gUtils_statCollCtrl.startTimePeak = Utils_getCurGlobalTimeInUsec();
        }

        sci_dump_cntrs(gUtils_statCollCtrl.psci_hdl,
                        pSdramCfg->uc_key,
                        pSdramCfg->num_uc,
                        pSdramCfg->stat_buf);

        sci_dump_cntrs(gUtils_statCollCtrl.psci_hdl,
                        pMstrCfg->uc_key,
                        pMstrCfg->num_uc,
                        pMstrCfg->stat_buf);

        sci_dump(gUtils_statCollCtrl.psci_hdl);

        elaspedTimeAvg = Utils_getCurGlobalTimeInUsec() - gUtils_statCollCtrl.startTimeAvg;
        elaspedTimePeak = Utils_getCurGlobalTimeInUsec() - gUtils_statCollCtrl.startTimePeak;

        gUtils_statCollCtrl.startTimePeak = Utils_getCurGlobalTimeInUsec();

        for( uc_idx = 0; uc_idx < pSdramCfg->num_uc; uc_idx++)
        {
            Utils_statColCompute(SDRAM,
                                pSdramCfg->stat_buf[uc_idx],
                                uc_idx,
                                sdram_cfg_idx,
                                elaspedTimeAvg,
                                elaspedTimePeak);
        }

        for( uc_idx = 0; uc_idx < pMstrCfg->num_uc; uc_idx++)
        {
            Utils_statColCompute(MSTR,
                                    pMstrCfg->stat_buf[uc_idx],
                                    uc_idx,
                                    mstr_cfg_idx,
                                    elaspedTimeAvg,
                                    elaspedTimePeak);
        }

        if(elaspedTimeAvg/1000 > SAMPLING_WINDOW_WIDTH)
        {
            system_sci_err = sci_global_disable(gUtils_statCollCtrl.psci_hdl);
            UTILS_assert(system_sci_err == SCI_SUCCESS);

            Utils_statCollectorUnRegisterSdram(sdram_cfg_idx);
            Utils_statCollectorUnRegisterMstr(mstr_cfg_idx);

            mstr_cfg_idx = (mstr_cfg_idx+1)%MAX_MSTR_CFGS;
            sdram_cfg_idx = (sdram_cfg_idx+1)%MAX_SDRAM_CFGS;

            Utils_statCollectorRegisterMstr(mstr_cfg_idx);
            Utils_statCollectorRegisterSdram(sdram_cfg_idx);

            system_sci_err = sci_global_enable(gUtils_statCollCtrl.psci_hdl);
            UTILS_assert(system_sci_err == SCI_SUCCESS);

            gUtils_statCollCtrl.startTimeAvg = Utils_getCurGlobalTimeInUsec();
            sci_dump(gUtils_statCollCtrl.psci_hdl);
        }
        BspOsal_sleep(CNTR_DUMP_INTRVL);
    }

    system_sci_err = sci_global_disable(gUtils_statCollCtrl.psci_hdl);
    UTILS_assert(system_sci_err == SCI_SUCCESS);

    Utils_statCollectorUnRegisterSdram(sdram_cfg_idx);
    Utils_statCollectorUnRegisterMstr(mstr_cfg_idx);

    gUtils_statCollCtrl.doStop = FALSE;
}

/**
 *******************************************************************************
 *
 * \brief   Stat Collector Init Function
 *
 *          This functions creates on saperate task which
 *          Stat Collector is initialized and started.
 *
 *******************************************************************************
*/
Void Utils_statCollectorInit()
{
    memset(&gUtils_statCollCtrl, 0, sizeof(gUtils_statCollCtrl));

    gUtils_statCollCtrl.doStop = FALSE;

    gUtils_statCollCtrl.tskHndl =
        BspOsal_taskCreate(
                (BspOsal_TaskFuncPtr)Utils_statCollectorTskMain,
                UTILS_STAT_COLL_TSK_PRI,
                gUtilsStatColl_tskStack,
                sizeof(gUtilsStatColl_tskStack),
                NULL
            );
    UTILS_assert(gUtils_statCollCtrl.tskHndl != NULL);

    Utils_prfLoadRegister(gUtils_statCollCtrl.tskHndl, "STAT_COLL");
}

/**
 *******************************************************************************
 *
 * \brief   Stat Collector deInit Function
 *
 *          DeInit And terminate the thread
 *
 *******************************************************************************
*/
Void Utils_statCollectorDeInit()
{
    gUtils_statCollCtrl.doStop = TRUE;

    Utils_prfLoadUnRegister(gUtils_statCollCtrl.tskHndl);

    BspOsal_taskDelete(&gUtils_statCollCtrl.tskHndl);
}
