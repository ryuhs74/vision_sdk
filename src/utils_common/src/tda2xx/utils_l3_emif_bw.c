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
 * \file utils_l3_emif_bw.c
 *
 * \brief This file has the implementation of the APIs to config bandwdith
 *        related controls at L3 and EMIF
 *
 * \version 0.0 (Dec 2013) : [KC] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_l3_emif_bw.h>

#define DISPC_GLOBAL_MFLAG_ATTRIBUTE              (volatile UInt32*)(0x5800185C)
#define DISPC_GFX_MFLAG_THRESHOLD                 (volatile UInt32*)(0x58001860)
#define DISPC_VID1_MFLAG_THRESHOLD                (volatile UInt32*)(0x58001864)
#define DISPC_VID2_MFLAG_THRESHOLD                (volatile UInt32*)(0x58001868)
#define DISPC_VID3_MFLAG_THRESHOLD                (volatile UInt32*)(0x5800186C)

#define DMM_EMERGENCY                             (volatile UInt32*)(0x4E000020)
#define DMM_PEG_PRIO_0_ADDR                       (volatile UInt32*)(0x4E000620)
#define CTRL_CORE_EMIF_INITIATOR_PRIORITY_1_ADDR  (volatile UInt32*)(0x4A002420)

Int32 Utils_setDssMflagMode(Utils_DssMflagMode mode)
{
    *DISPC_GLOBAL_MFLAG_ATTRIBUTE
        =  ((UInt32)mode & 0x3) /* MFLAG_CTRL */
          |
           (1<<2) /* MFLAG_START
                   * 0x1: Even at the beginning of the frame when the DMA
                   *      buffer is empty, MFLAG_CTRL bitfield is used to
                   *      determine how MFLAG signal for each pipeline shall be
                   *      driven.
                   */
           ;
    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_setDssMflagThreshold(System_DssDispcPipes displayPipeId,
                        UInt32 thresHigh,
                        UInt32 thresLow)
{
    UInt32 value;
    Int32 status=SYSTEM_LINK_STATUS_SOK;
    volatile UInt32 *pReg;

    value = (thresLow & 0xFFFF) | ((thresHigh & 0xFFFF)<<16);

    switch(displayPipeId)
    {
        case SYSTEM_DSS_DISPC_PIPE_VID1:
            pReg = DISPC_VID1_MFLAG_THRESHOLD;
            break;
        case SYSTEM_DSS_DISPC_PIPE_VID2:
            pReg = DISPC_VID2_MFLAG_THRESHOLD;
            break;
        case SYSTEM_DSS_DISPC_PIPE_VID3:
            pReg = DISPC_VID3_MFLAG_THRESHOLD;
            break;
        case SYSTEM_DSS_DISPC_PIPE_GFX1:
            pReg = DISPC_GFX_MFLAG_THRESHOLD;
            break;
        default:
            status = SYSTEM_LINK_STATUS_EFAIL;
            break;
    }

    if(status==SYSTEM_LINK_STATUS_SOK)
    {
        *pReg = value;
    }

    return status;
}

Int32 Utils_setDmmPri(Utils_DmmInitiatorId initiatorId, UInt32 priValue)
{
    volatile UInt32 *pPegPrioReg = DMM_PEG_PRIO_0_ADDR;
    UInt32 index;
    UInt32 shift;

    index = initiatorId/8;
    shift  = (initiatorId%8)*4;

    if(index>=8)
        return SYSTEM_LINK_STATUS_EFAIL;

    priValue = (0x8 | (priValue & 0x7));

    pPegPrioReg[index] = priValue << shift;

    Vps_printf(" DMM_PEG_PRIO_%d (0x%08x) = 0x%08x\n",
            index,
            &pPegPrioReg[index],
            pPegPrioReg[index]
        );

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_setDmmMflagEmergencyEnable(Bool enable)
{
    UInt32 value;

    value = *DMM_EMERGENCY;

    if(enable)
    {
        value |= 0x1;
    }
    else
    {
        value &= ~0x1;
    }


    *DMM_EMERGENCY = value;

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_setEmifPri(Utils_EmifInitiatorId initiatorId, UInt32 priValue)
{
    volatile UInt32 *pEmifPrioReg = CTRL_CORE_EMIF_INITIATOR_PRIORITY_1_ADDR;
    UInt32 index;
    UInt32 shift;

    index = initiatorId/8;
    shift  = (initiatorId%8)*4;

    if(index>=7)
        return SYSTEM_LINK_STATUS_EFAIL;

    priValue = (priValue & 0x7);

    /* clear field */
    pEmifPrioReg[index] &= ~(0x7 << shift);

    /* set field */
    pEmifPrioReg[index] |= (priValue << shift);

    Vps_printf(" CTRL_CORE_EMIF_INITIATOR_PRIORITY_%d (0x%08x) = 0x%08x\n",
            index+1,
            &pEmifPrioReg[index],
            pEmifPrioReg[index]
        );

    return SYSTEM_LINK_STATUS_SOK;
}

#define L3_BW_LIMITER_BANDWIDTH_FRACTIONAL_P1    (volatile UInt32*)(0x44805B08)
#define L3_BW_LIMITER_BANDWIDTH_INTEGER_P1       (volatile UInt32*)(0x44805B0C)
#define L3_BW_LIMITER_WATERMARK_0_P1             (volatile UInt32*)(0X44805B10)
#define L3_BW_LIMITER_CLEARHISTORY_P1            (volatile UInt32*)(0X44805B14)

#define L3_BW_LIMITER_BANDWIDTH_FRACTIONAL_P2    (volatile UInt32*)(0x44805C08)
#define L3_BW_LIMITER_BANDWIDTH_INTEGER_P2       (volatile UInt32*)(0x44805C0C)
#define L3_BW_LIMITER_WATERMARK_0_P2             (volatile UInt32*)(0X44805C10)
#define L3_BW_LIMITER_CLEARHISTORY_P2            (volatile UInt32*)(0X44805C14)

Int32 Utils_setBWLimiter(Utils_DmmInitiatorId initiatorId, UInt32 BW_valueInMBps)
{
   UInt32 BW;
   UInt32 BW_int;
   UInt32 BW_frac;

   BW = (UInt32) (BW_valueInMBps/8.3125);
   BW_int = (BW & 0xFFFFFFE0) >> 5;
   BW_frac = (BW & 0x1F);

   if (UTILS_DMM_INITIATOR_ID_GPU_P1 == initiatorId)
   {
       *L3_BW_LIMITER_BANDWIDTH_FRACTIONAL_P1 = BW_frac;
       *L3_BW_LIMITER_BANDWIDTH_INTEGER_P1 = BW_int;
       *L3_BW_LIMITER_WATERMARK_0_P1= 0;
       *L3_BW_LIMITER_CLEARHISTORY_P1 = 1;
   }

   if (UTILS_DMM_INITIATOR_ID_GPU_P2 == initiatorId)
   {
       *L3_BW_LIMITER_BANDWIDTH_FRACTIONAL_P2 = BW_frac;
       *L3_BW_LIMITER_BANDWIDTH_INTEGER_P2 = BW_int;
       *L3_BW_LIMITER_WATERMARK_0_P2 = 0;
       *L3_BW_LIMITER_CLEARHISTORY_P2 = 1;
   }

    return SYSTEM_LINK_STATUS_SOK;
}

