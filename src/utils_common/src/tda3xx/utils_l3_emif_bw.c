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
    Vps_printf(" UTILS: DMM: API NOT supported in TDA3xx !!! \n");
    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_setDmmMflagEmergencyEnable(Bool enable)
{
    Vps_printf(" UTILS: DMM: API NOT supported in TDA3xx !!! \n");
    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_setEmifPri(Utils_EmifInitiatorId initiatorId, UInt32 priValue)
{
    Vps_printf(" UTILS: EMIF: API NOT supported in TDA3xx !!! \n");
    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_setBWLimiter(Utils_DmmInitiatorId initiatorId, UInt32 BW_valueInMBps)
{
    Vps_printf(" UTILS: SGX: API NOT supported in TDA3xx !!! \n");
    return SYSTEM_LINK_STATUS_SOK;
}
