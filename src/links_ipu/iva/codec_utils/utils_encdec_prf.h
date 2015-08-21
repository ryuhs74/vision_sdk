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
 * \file utils_encdec_prf.h
 *
 * \brief  HDVICP frofile functions are defined
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_ENCDEC_PRF_H_
#define _UTILS_ENCDEC_PRF_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <string.h>
#include <xdc/std.h>
#include <xdc/runtime/Timestamp.h>
#include <include/link_api/system_trace.h>

/*******************************************************************************
 *  \brief Flag to enable/disable HDVICP profiling option
 *******************************************************************************
 */
#define UTILS_ENCDEC_HDVICP_PROFILE

/*******************************************************************************
 *  \brief Max number of HDVICP resources available in the decvice
 *******************************************************************************
 */
#define UTILS_ENCDEC_MAXNUMOFHDVICP2_RESOUCES                               (1)

typedef struct HDVICP_logTbl {
    UInt32 totalAcquire2wait;
    UInt32 totalWait2Isr;
    UInt32 totalIsr2Done;
    UInt32 totalWait2Done;
    UInt32 totalDone2Release;
    UInt32 totalAcquire2Release;
    UInt32 totalAcq2acqDelay;
    UInt32 tempPrevTime;
    UInt32 tempAcquireTime;
    UInt32 tempWaitTime;
    UInt32 startTime;
    UInt32 endTime;
    UInt32 numAccessCnt;
    UInt32 totalIVAHDActivateTime;
} HDVICP_logTbl;

extern HDVICP_logTbl g_HDVICP_logTbl[UTILS_ENCDEC_MAXNUMOFHDVICP2_RESOUCES];


static inline UInt32 Utils_encdecGetTime(Void)
{
    return (Utils_getCurGlobalTimeInMsec());
}

static Void Utils_encdecHdvicpPrfInit(Void)
{
    memset(&g_HDVICP_logTbl, 0, sizeof(g_HDVICP_logTbl));
    return;
}

static Void Utils_encdecHdvicpPrfPrint(void)
{
    int ivaId;
    UInt32 totalElapsedTime = 0, perCentTotalWait2Isr = 0;


#ifdef UTILS_ENCDEC_HDVICP_PROFILE
    for (ivaId = 0; ivaId < UTILS_ENCDEC_MAXNUMOFHDVICP2_RESOUCES; ivaId++)
    {

        Vps_printf(" HDVICP-ID: %d\n", ivaId);

        totalElapsedTime = (g_HDVICP_logTbl[ivaId].endTime -
                            g_HDVICP_logTbl[ivaId].startTime);
        if(totalElapsedTime)
        {
            perCentTotalWait2Isr = (g_HDVICP_logTbl[ivaId].totalWait2Isr * 100) /
                                    totalElapsedTime;
            Vps_printf (" All percentage figures are based off totalElapsedTime\n");
            Vps_printf("\n\t\t totalAcquire2wait :%d %%"
                       "\n\t\t totalWait2Isr :%d %%"
                       "\n\t\t totalIsr2Done :%d %%"
                       "\n\t\t totalWait2Done :%d %%"
                       "\n\t\t totalDone2Release :%d %%"
                       "\n\t\t totalAcquire2Release :%d %%"
                       "\n\t\t totalAcq2acqDelay :%d %%"
                       "\n\t\t totalElapsedTime in msec :%8d"
                       "\n\t\t numAccessCnt:%8d\n",
                       (g_HDVICP_logTbl[ivaId].totalAcquire2wait * 100) / totalElapsedTime ,
                       (g_HDVICP_logTbl[ivaId].totalWait2Isr * 100) / totalElapsedTime,
                       (g_HDVICP_logTbl[ivaId].totalIsr2Done * 100) / totalElapsedTime,
                       (g_HDVICP_logTbl[ivaId].totalWait2Done * 100) / totalElapsedTime,
                       (g_HDVICP_logTbl[ivaId].totalDone2Release * 100) / totalElapsedTime,
                       (g_HDVICP_logTbl[ivaId].totalAcquire2Release * 100 ) / totalElapsedTime,
                       (g_HDVICP_logTbl[ivaId].totalAcq2acqDelay * 100) / totalElapsedTime,
                       totalElapsedTime, g_HDVICP_logTbl[ivaId].numAccessCnt
                       );

            Vps_printf(" IVA-FPS :%8d\n",
                   ((g_HDVICP_logTbl[ivaId].numAccessCnt) /
                    (totalElapsedTime/1000)));

        }

        if(g_HDVICP_logTbl[ivaId].numAccessCnt)
        {
            Vps_printf(" Average time spent per frame in microsec:%8d\n",
                   ((totalElapsedTime * 1000/ g_HDVICP_logTbl[ivaId].numAccessCnt) * perCentTotalWait2Isr)/ 100);
        }
        Vps_printf(" \n");
    }
#endif
    return;
}

#endif

/* Nothing beyond this point */


