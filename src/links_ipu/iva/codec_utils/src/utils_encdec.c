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
 * \file utils_encdec.c
 *
 * \brief  IVA Encode/Decode link utils functions are implemented
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <xdc/std.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/sdo/fc/rman/rman.h>
#include <ti/xdais/dm/ividenc2.h>
#include <ti/xdais/dm/ividdec3.h>
#include <ti/sysbios/gates/GateMutexPri.h>
#include <src/links_ipu/iva/codec_utils/utils_encdec.h>
#include <src/links_ipu/iva/codec_utils/utils_encdec_prf.h>
#include <src/links_ipu/iva/codec_utils/hdvicp2_config.h>
#include <src/utils_common/include/utils_mem.h>
#include "iresman_tiledmemory.h"
#include <ti/sdo/codecs/h264enc/ih264enc.h>
#include <ti/sdo/codecs/h264vdec/ih264vdec.h>


Int initDone = FALSE;

HDVICP_logTbl g_HDVICP_logTbl[UTILS_ENCDEC_MAXNUMOFHDVICP2_RESOUCES];


EncDec_ResolutionClass Utils_encdecGetResolutionClass(UInt32 width,
                                                    UInt32 height)
{
    EncDec_ResolutionClass resClass = UTILS_ENCDEC_RESOLUTION_CLASS_16MP;

    UTILS_assert((width <= UTILS_ENCDEC_RESOLUTION_CLASS_16MP_WIDTH)
                 && (height <= UTILS_ENCDEC_RESOLUTION_CLASS_16MP_HEIGHT));

    if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_9MP_WIDTH) ||
        (height > UTILS_ENCDEC_RESOLUTION_CLASS_9MP_HEIGHT))
    {
        resClass = UTILS_ENCDEC_RESOLUTION_CLASS_16MP;
    }
    else
    {
        if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_5MP_WIDTH) ||
            (height > UTILS_ENCDEC_RESOLUTION_CLASS_5MP_HEIGHT))
        {
            resClass = UTILS_ENCDEC_RESOLUTION_CLASS_9MP;
        }
        else
        {
            if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_4MP_WIDTH) ||
                (height > UTILS_ENCDEC_RESOLUTION_CLASS_4MP_HEIGHT))
            {
                   resClass = UTILS_ENCDEC_RESOLUTION_CLASS_5MP;
            }
            else
            {
                if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_1080P_WIDTH) ||
                    (height > UTILS_ENCDEC_RESOLUTION_CLASS_1080P_HEIGHT))
                {
                    resClass = UTILS_ENCDEC_RESOLUTION_CLASS_4MP;
                }
                else
                {
                    if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_720P_WIDTH) ||
                        (height > UTILS_ENCDEC_RESOLUTION_CLASS_720P_HEIGHT))
                    {
                        resClass = UTILS_ENCDEC_RESOLUTION_CLASS_1080P;
                    }
                    else
                    {
                        if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_D1_WIDTH) ||
                            (height > UTILS_ENCDEC_RESOLUTION_CLASS_D1_HEIGHT))
                        {
                            resClass = UTILS_ENCDEC_RESOLUTION_CLASS_720P;
                        }
                        else
                        {
                            if ((width > UTILS_ENCDEC_RESOLUTION_CLASS_CIF_WIDTH) ||
                                (height > UTILS_ENCDEC_RESOLUTION_CLASS_CIF_HEIGHT))
                            {
                                resClass = UTILS_ENCDEC_RESOLUTION_CLASS_D1;
                            }
                            else
                            {
                                resClass = UTILS_ENCDEC_RESOLUTION_CLASS_CIF;
                            }
                        }
                    }
                }
            }
        }
    }
    return resClass;
}

Int Utils_encdecGetCodecLevel(UInt32 codingFormat,
                              UInt32 maxWidth,
                              UInt32 maxHeight,
                              UInt32 maxFrameRate,
                              UInt32 maxBitRate, Int32 * pLevel,
                              Bool   isEnc)
{
    Int retVal = UTILS_ENCDEC_S_SUCCESS;

    (Void) maxWidth;
    (Void) maxHeight;
    (Void) maxFrameRate;
    (Void) maxBitRate;

    switch (codingFormat)
    {
        case SYSTEM_IVIDEO_H264BP:
        case SYSTEM_IVIDEO_H264MP:
        case SYSTEM_IVIDEO_H264HP:
            switch (Utils_encdecGetResolutionClass(maxWidth, maxHeight))
            {
                case UTILS_ENCDEC_RESOLUTION_CLASS_16MP:
                case UTILS_ENCDEC_RESOLUTION_CLASS_9MP:
                case UTILS_ENCDEC_RESOLUTION_CLASS_5MP:
                case UTILS_ENCDEC_RESOLUTION_CLASS_4MP:
                    if (isEnc)
                        *pLevel = IH264_LEVEL_51;
                    else
                        *pLevel = IH264VDEC_LEVEL51;
                    break;
                case UTILS_ENCDEC_RESOLUTION_CLASS_1080P:
                    if (isEnc)
                        *pLevel = IH264_LEVEL_41;
                    else
                        *pLevel = IH264VDEC_LEVEL41;
                    break;
                case UTILS_ENCDEC_RESOLUTION_CLASS_CIF:
                    if (isEnc)
                        *pLevel = IH264_LEVEL_30;
                    else
                        *pLevel = IH264VDEC_LEVEL3;
                    break;
                case UTILS_ENCDEC_RESOLUTION_CLASS_720P:
                case UTILS_ENCDEC_RESOLUTION_CLASS_D1:
                    if (isEnc)
                        *pLevel = IH264_LEVEL_31;
                    else
                        *pLevel = IH264VDEC_LEVEL31;
                    break;
                default:
                    if (isEnc)
                        *pLevel = IH264_LEVEL_41;
                    else
                        *pLevel = IH264VDEC_LEVEL41;
                    break;
            }
            break;

        default:
            *pLevel = IVIDENC2_DEFAULTLEVEL;
            retVal = UTILS_ENCDEC_E_UNKNOWNCODINGTFORMAT;
            break;
    }

    return retVal;
}

Int Utils_encdecInit()
{
    IRES_Status iresStatus;

    if (FALSE == initDone)
    {
        HDVICP2_ClearIVAInterrupts();
        HDVICP2_Init();
        iresStatus = RMAN_init();
        UTILS_assert(iresStatus == IRES_OK);

        iresStatus = IRESMAN_TiledMemoryResourceRegister();
        UTILS_assert(iresStatus == IRES_OK);

        initDone = TRUE;
    }
    return 0;
}

Int Utils_encdecDeInit()
{
    IRES_Status iresStatus;

    if (TRUE == initDone)
    {
        iresStatus = IRESMAN_TiledMemoryResourceUnregister();
        UTILS_assert(iresStatus == IRES_OK);

        iresStatus = RMAN_exit();
        UTILS_assert(iresStatus == IRES_OK);
        initDone = FALSE;
        Utils_encdecHdvicpPrfInit();
    }

    return 0;
}

/**
********************************************************************************
 *  @func   Utils_encdecGetIVAID()
 *  @brief  Framework Componnet callable APIs, It serves as interface between
 *          codec and Iva scheduler.
 *
 * @param [in] algHandle      : codec handle
 * @param [in] hdvicpHandle   : hdvicp handle from framework component
 * @param [out] id            : returned ivahd id  pointer
 *
 *  @return
 *  0 = Successful
 *
 *
********************************************************************************
*/
/* =========================================================================== */

Int32 Utils_encdecGetIVAID(Int32 * id, Ptr algHandle, Ptr hdvicpHandle)
{
    (void)algHandle;
    (void)hdvicpHandle;

    *id = 0; /*assume only one IVA-HD, multiple IVA-HDs not supported */
    return (UTILS_ENCDEC_S_SUCCESS);
}

/**
********************************************************************************
 *  @func   Utils_encdecReleaseIVAID()
 *  @brief  Framework Componnet callable APIs, It serves as interface between
 *          codec and Iva scheduler, currently a placeholder for enhancement
 *
 * @param [in] algHandle      : codec handle
 * @param [in] hdvicpHandle   : hdvicp handle from framework component
 * @param [out] id            : ivahd id
 *
 *  @return
 *  0 = Successful
 *
 *  Other_value = Failed (Error code is returned)
 *
********************************************************************************
*/
/* =========================================================================== */

Int32 Utils_encdecReleaseIVAID(Int32 id, Ptr algHandle, Ptr hdvicpHandle)
{
    return (UTILS_ENCDEC_S_SUCCESS);
}

/* Yield function to be used when yielding context and reacquiring it to run
 * again */
Void Utils_encdecDummyContextRelease(IRES_YieldResourceType resource,
                                     IRES_YieldContextHandle algYieldContext,
                                     IRES_YieldArgs yieldArgs)
{
    (Void) resource;
    (Void) algYieldContext;
    (Void) yieldArgs;
    /* Do nothing */

}

Void Utils_encdecDummyContextAcquire(IRES_YieldResourceType resource,
                                     IRES_YieldContextHandle algYieldContext,
                                     IRES_YieldArgs yieldArgs)
{
    (Void) resource;
    (Void) algYieldContext;
    (Void) yieldArgs;
    /* Do nothing */
}


Int32 Utils_encdec_checkResourceAvail(IALG_Handle alg, IRES_Fxns * resFxns,
                                      FVID2_Format *pFormat, UInt32 numFrames,
                                      IRES_ResourceDescriptor resDesc[])
{
    Int32 status = UTILS_ENCDEC_S_SUCCESS;
    UInt32 size, bufsize, codecSize, cOffset, totalFreeSpace;
    IRES_Status codecStatus = IRES_OK;
    Int32 bufStatus = SYSTEM_LINK_STATUS_SOK;
    UInt32 paddingOffset = 10*1024;
    Utils_MemHeapStats memStats;

    size = 0;
    bufsize = 0;
    codecSize = 0;
    totalFreeSpace = 0;
    /* align height to multiple of 2 */
    pFormat->height = VpsUtils_align(pFormat->height, 2);
    /* get frame size for given pFormat */
    bufStatus = Utils_memFrameGetSize(pFormat, &bufsize, &cOffset, 0);
    UTILS_assert (bufsize >= cOffset);
    bufsize = bufsize * numFrames;
    bufsize = VpsUtils_align(bufsize, VPS_BUFFER_ALIGNMENT);

    if (bufStatus == SYSTEM_LINK_STATUS_SOK)
    {
        size += bufsize;
    }
    codecStatus = IRESMAN_TILEDMEMORY_checkResourceAvail(alg, &codecSize,
                                                         resFxns, resDesc);
    if (codecStatus == IRES_OK)
    {
        size += codecSize;
    }

    Utils_memGetHeapStats(UTILS_HEAPID_DDR_CACHED_SR, &memStats);

    totalFreeSpace += memStats.freeSize;

    if (totalFreeSpace < (size + paddingOffset))
    {
        status = UTILS_ENCDEC_E_FAIL;
        Vps_printf("ENCDECUTIL: ERROR! DECODER Creation: Insufficient Memory; "
                   "Required = %d, Free Memory Available = %d \n ",
                   (size + paddingOffset), totalFreeSpace);
    }

    if((bufStatus != SYSTEM_LINK_STATUS_SOK) || (codecStatus != IRES_OK))
    {
        status = UTILS_ENCDEC_E_FAIL;
        Vps_printf("ENCDECUTIL: ERROR! DECODER Resource Availability Check "
                   "FAILED \n");
    }

    return (status);
}

/* Nothing beyond this point */

