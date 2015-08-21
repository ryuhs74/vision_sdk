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
 *  \file osa_prf.c
 *
 *  \brief This file has implementation for OSA PERF
 *
 *
 *  \version 0.0 (May 2014) : [YM] First version taken from bios side vision_sdk
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <osa_prf.h>


/**
 *******************************************************************************
 *
 * \brief Reset latency stats
 *
 * \param  lStats    [OUT] latency statistics
 *
 *******************************************************************************
 */
Void OSA_resetLatency(OSA_LatencyStats *lStats)
{
    lStats->accumulatedLatency = 0;
    lStats->minLatency = 0xFFFFFFFF;
    lStats->maxLatency = 0x0;
    lStats->count = 0;
}

/**
 *******************************************************************************
 *
 * \brief Calculate latency
 *
 * \param  localLinkstats    [OUT] latency statistics
 * \param  curTime           [IN]  current timestamp after link processing is
 *                                 complete
 * \param  linkLocalTime     [IN]  time at which frame was received at the link
 *                                 OR source timestamp
 * \param  firstTime         [IN]  TRUE, first time this function is called for
 *                                 this stats structure
 *
 *******************************************************************************
 */
Void OSA_updateLatency(OSA_LatencyStats *lStats,
                         UInt64 linkLocalTime)
{
    UInt64 latency;
    UInt64 curTime = OSA_getCurGlobalTimeInUsec();

    latency = curTime - linkLocalTime;

    if (lStats->minLatency > latency)
    {
        lStats->minLatency = latency;
    }
    if (latency > lStats->maxLatency)
    {
        lStats->maxLatency = latency;
    }
    lStats->accumulatedLatency += latency;
    lStats->count++;

}

/**
 *******************************************************************************
 *
 * \brief Print the latency statistics
 *
 * \param  localLinkstats    [IN] local link latency
 * \param  srcToLinkstats    [IN] source to link latency
 * \param  numFrames         [IN] number of frames over which this latency is
 *                                calculated
 *
 *******************************************************************************
 */
Void OSA_printLatency(char *name,
                        OSA_LatencyStats *localLinkstats,
                        OSA_LatencyStats *srcToLinkstats,
                        Bool resetStats)
{
    if(srcToLinkstats->count || localLinkstats->count)
    {
        /* Divide by 1000 is done to convert micro second to millisecond */
        Vps_printf( " \n");
        Vps_printf( " [ %s ] LATENCY,\n", name);
        Vps_printf( " ********************\n");
        if(localLinkstats->count)
        {
            Vps_printf( " Local Link Latency     : Avg = %6d us, Min = %6d us, Max = %6d us, \r\n",
                (UInt32)(localLinkstats->accumulatedLatency/localLinkstats->count),
                (UInt32)localLinkstats->minLatency,
                (UInt32)localLinkstats->maxLatency
                );
        }
        if(srcToLinkstats->count)
        {
            Vps_printf( " Source to Link Latency : Avg = %6d us, Min = %6d us, Max = %6d us, \r\n",
                (UInt32)(srcToLinkstats->accumulatedLatency/srcToLinkstats->count),
                (UInt32)srcToLinkstats->minLatency,
                (UInt32)srcToLinkstats->maxLatency
                );
        }
        Vps_printf( " \n");
    }
    OSA_resetLatency(localLinkstats);
    OSA_resetLatency(srcToLinkstats);
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
Void OSA_resetLinkStatistics(OSA_LinkStatistics *pPrm,
                                UInt32 numCh,
                                UInt32 numOut)
{
    OSA_LinkChStatistics *pChStats;
    UInt32 chId, outId;


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
    pPrm->statsStartTime = OSA_getCurGlobalTimeInMsec();
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

UInt32 OSA_calcFps(UInt32 count, UInt32 divValue)
{
    UInt32 fps, mult, div;

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
 * \brief Print the link statistics
 *
 * \param  pPrm         [IN] link statistics
 * \param  name         [IN] Link task name
 * \param  resetStats   [IN] TRUE, Reset stats after print
 *
 *******************************************************************************
 */
Void OSA_printLinkStatistics(OSA_LinkStatistics *pPrm, char *name,
                                Bool resetStats)
{
    UInt32 elaspedTime;
    UInt32 chId, outId;
    OSA_LinkChStatistics *pChStats;
    UInt32 value1, value2, value3, value4;

    elaspedTime = OSA_getCurGlobalTimeInMsec() - pPrm->statsStartTime;

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
        value1 = OSA_calcFps(pPrm->newDataCmdCount, elaspedTime);

        Vps_printf(" New data Recv      = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->releaseDataCmdCount)
    {
        value1 = OSA_calcFps(pPrm->releaseDataCmdCount, elaspedTime);

        Vps_printf(" Release data Recv  = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->getFullBufCount)
    {
        value1 = OSA_calcFps(pPrm->getFullBufCount, elaspedTime);

        Vps_printf(" Get Full Buf Cb    = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->putEmptyBufCount)
    {
        value1 = OSA_calcFps(pPrm->putEmptyBufCount, elaspedTime);

        Vps_printf(" Put Empty Buf Cb   = %3d.%d fps\r\n",
                        value1/100,
                        value1%100
                    );
    }

    if(pPrm->notifyEventCount)
    {
        value1 = OSA_calcFps(pPrm->notifyEventCount, elaspedTime);

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
                value1 = OSA_calcFps(pChStats->inBufRecvCount, elaspedTime);
                value2 = OSA_calcFps(pChStats->inBufDropCount, elaspedTime);
                value3 = OSA_calcFps(pChStats->inBufUserDropCount, elaspedTime);
                value4 = OSA_calcFps(pChStats->inBufProcessCount, elaspedTime);

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
                    value1 = OSA_calcFps(
                                pChStats->outBufCount[outId],
                                elaspedTime);

                    value2 = OSA_calcFps(
                                pChStats->outBufDropCount[outId],
                                elaspedTime);

                    value3 = OSA_calcFps(
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
        OSA_resetLinkStatistics(pPrm, pPrm->numCh, pPrm->chStats[0].numOut);
    }
}

