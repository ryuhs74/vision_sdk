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
 * \file frameCopyAlgoLocalDma.c
 *
 * \brief Algorithm for Alg_FrameCopy on DSP/EVE using Local EDMA
 *
 *        This Alg_FrameCopy algorithm is only for demonstrative purpose.
 *        It is NOT product quality.
 *        This algorithm does a frame copy. Height and width gets decided during
 *        Create. If height / width needs to be altered, then control call
 *        needs to be done.
 *
 *        This implementation uses CPU or DMA to copy the frames
 *        EDMA3LLD APIs are used to copy the frames.
 *        EDMA3LLD is initialized via Utils_dmaInit() during system init.
 *
 * \version 0.0 (Sept 2013) : [KC] First version
 * \version 0.1 (Sept 2013) : [PS] Merged CPU copy version also
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "iFrameCopyAlgo.h"
#include "src/utils_common/include/utils_dma.h"

/**
 *******************************************************************************
 *
 *   \brief Structure for frame copy algoirthm object
 *
 *******************************************************************************
*/
typedef struct
{
    Alg_FrameCopy_Obj     frameCopyObj;
    /**< Base Frame copy object */

    unsigned int edmaChId;
    /**< EDMA CH ID that is used for this EDMA */

    unsigned int tccId;
    /**< EDMA TCC ID that is used for this EDMA */

    EDMA3_DRV_Handle hEdma;
    /**< Handle to EDMA controller associated with this logical DMA channel */

    EDMA3_DRV_PaRAMRegs *pParamSet;
    /**< Pointer to physical area of PaRAM for this channel */

} Alg_FrameCopyDma_Obj;

/**
 *******************************************************************************
 *
 * \brief Implementation of create for frame copy algo
 *
 * \param  pCreateParams    [IN] Creation parameters for frame copy Algorithm
 *
 * \return  Handle to algorithm
 *
 *******************************************************************************
 */
Alg_FrameCopy_Obj * Alg_FrameCopyCreate(
                        Alg_FrameCopyCreateParams *pCreateParams)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    UInt32 paramPhyAddr;
    Alg_FrameCopyDma_Obj * pAlgHandle;

    pAlgHandle = (Alg_FrameCopyDma_Obj *) malloc(sizeof(Alg_FrameCopyDma_Obj));

    UTILS_assert(pAlgHandle != NULL);

    pAlgHandle->frameCopyObj.maxHeight   = pCreateParams->maxHeight;
    pAlgHandle->frameCopyObj.maxWidth    = pCreateParams->maxWidth;

    pAlgHandle->hEdma     =
        Utils_dmaGetEdma3Hndl(UTILS_DMA_LOCAL_EDMA_INST_ID);

    UTILS_assert(pAlgHandle->hEdma!=NULL);

    pAlgHandle->tccId       = EDMA3_DRV_TCC_ANY;
    pAlgHandle->edmaChId    = EDMA3_DRV_DMA_CHANNEL_ANY;

    edma3Result = EDMA3_DRV_requestChannel(
                                    pAlgHandle->hEdma,
                                    (uint32_t*)&pAlgHandle->edmaChId,
                                    (uint32_t*)&pAlgHandle->tccId,
                                    (EDMA3_RM_EventQueue)0,
                                    NULL, (void *)pAlgHandle);

    if (edma3Result == EDMA3_DRV_SOK)
    {
        Vps_printf(" ALG_FRAMECOPY: DMA: Allocated CH (TCC) = %d (%d)\n",
                        pAlgHandle->edmaChId,
                        pAlgHandle->tccId);

        edma3Result = EDMA3_DRV_clearErrorBits(
                            pAlgHandle->hEdma,
                            pAlgHandle->edmaChId
                        );
    }
    else
    {
        Vps_printf(" ALG_FRAMECOPY: DMA: ERROR in EDMA CH allocation\n");
    }

    if (edma3Result == EDMA3_DRV_SOK)
    {
        edma3Result = EDMA3_DRV_getPaRAMPhyAddr(
                                pAlgHandle->hEdma,
                                pAlgHandle->edmaChId,
                                &paramPhyAddr);

        pAlgHandle->pParamSet = (EDMA3_DRV_PaRAMRegs*)paramPhyAddr;
    }

    UTILS_assert(edma3Result == EDMA3_DRV_SOK);

    return &pAlgHandle->frameCopyObj;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process for frame copy algo
 *
 *        Supported formats are SYSTEM_DF_YUV422I_YUYV, SYSTEM_DF_YUV420SP_UV.
 *        It is assumed that the width of the image will
 *        be multiple of 4 and buffer pointers are 32-bit aligned.
 *
 * \param  algHandle    [IN] Algorithm object handle
 * \param  inPtr[]      [IN] Array of input pointers
 *                           Index 0 - Pointer to Y data in case of YUV420SP,
 *                                   - Single pointer for YUV422IL or RGB
 *                           Index 1 - Pointer to UV data in case of YUV420SP
 * \param  outPtr[]     [IN] Array of output pointers. Indexing similar to
 *                           array of input pointers
 * \param  width        [IN] width of image
 * \param  height       [IN] height of image
 * \param  inPitch[]    [IN] Array of pitch of input image (Address offset
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  outPitch[]   [IN] Array of pitch of output image (Address offset
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  dataFormat   [IN] Different image data formats. Refer
 *                           System_VideoDataFormat
 * \param  copyMode     [IN] 0 - copy by CPU, 1 - copy by DMA
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FrameCopyProcess(Alg_FrameCopy_Obj *algHandle,
                           UInt32            *inPtr[],
                           UInt32            *outPtr[],
                           UInt32             width,
                           UInt32             height,
                           UInt32             inPitch[],
                           UInt32             outPitch[],
                           UInt32             dataFormat,
                           Uint32             copyMode
                          )
{
    Int32 rowIdx;
    Int32 colIdx;
    UInt32 *inputPtr;
    UInt32 *outputPtr;
    UInt32 numPlanes;
    UInt32 wordWidth;
    UInt32 lineSizeInBytes;
    UInt32 opt;
    uint16_t tccStatus;
    Alg_FrameCopyDma_Obj * pAlgHandle;

    pAlgHandle = (Alg_FrameCopyDma_Obj *)algHandle;

    if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
    {
        numPlanes = 1;
        lineSizeInBytes = width*2;
        wordWidth = (width*2)>>2;
    }
    else if(dataFormat == SYSTEM_DF_YUV420SP_UV)
    {
        numPlanes = 2;
        lineSizeInBytes = width;
        wordWidth = (width)>>2;
    }
    else
    {
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    if(copyMode == 0)
    {
        /*
         * For Luma plane of 420SP OR RGB OR 422IL
         */
        inputPtr  = inPtr[0];
        outputPtr = outPtr[0];

        for(rowIdx = 0; rowIdx < height ; rowIdx++)
        {
            for(colIdx = 0; colIdx < wordWidth ; colIdx++)
            {
                *(outputPtr+colIdx) = *(inputPtr+colIdx);
            }
            inputPtr += (inPitch[0] >> 2);
            outputPtr += (outPitch[0] >> 2);
        }

        /*
         * For chroma plane of 420SP
         */
        if(numPlanes == 2)
        {
            inputPtr  = inPtr[1];
            outputPtr = outPtr[1];
            for(rowIdx = 0; rowIdx < (height >> 1) ; rowIdx++)
            {
                for(colIdx = 0; colIdx < wordWidth ; colIdx++)
                {
                    *(outputPtr+colIdx) = *(inputPtr+colIdx);
                }
                inputPtr += (inPitch[1] >> 2);
                outputPtr += (outPitch[1] >> 2);
            }
        }

    }
    else
    {
        opt  =
            (EDMA3_CCRL_OPT_ITCCHEN_ENABLE << EDMA3_CCRL_OPT_ITCCHEN_SHIFT)
               |
            ((pAlgHandle->tccId << EDMA3_CCRL_OPT_TCC_SHIFT)
                        & EDMA3_CCRL_OPT_TCC_MASK
             )
               |
            (EDMA3_CCRL_OPT_SYNCDIM_ABSYNC << EDMA3_CCRL_OPT_SYNCDIM_SHIFT)
               |
            (EDMA3_CCRL_OPT_TCINTEN_ENABLE
                        << EDMA3_CCRL_OPT_TCINTEN_SHIFT)
            ;

        /*
         * For Luma plane of 420SP OR RGB OR 422IL
         */
        pAlgHandle->pParamSet->destAddr   = (UInt32)outPtr[0];
        pAlgHandle->pParamSet->srcAddr    = (UInt32)inPtr[0];
        pAlgHandle->pParamSet->srcBIdx    = inPitch[0];
        pAlgHandle->pParamSet->destBIdx   = outPitch[0];
        pAlgHandle->pParamSet->srcCIdx    = 0;
        pAlgHandle->pParamSet->destCIdx   = 0;
        pAlgHandle->pParamSet->aCnt       = lineSizeInBytes;
        pAlgHandle->pParamSet->bCnt       = height;
        pAlgHandle->pParamSet->cCnt       = 1;
        pAlgHandle->pParamSet->bCntReload = height;
        pAlgHandle->pParamSet->opt        = opt;
        pAlgHandle->pParamSet->linkAddr   = 0xFFFF;

        EDMA3_DRV_checkAndClearTcc(pAlgHandle->hEdma,
                                   pAlgHandle->tccId,
                                   &tccStatus);
        EDMA3_DRV_clearErrorBits (pAlgHandle->hEdma,pAlgHandle->edmaChId);
        EDMA3_DRV_enableTransfer (pAlgHandle->hEdma,pAlgHandle->edmaChId,
                                               EDMA3_DRV_TRIG_MODE_MANUAL);
        EDMA3_DRV_waitAndClearTcc(pAlgHandle->hEdma,pAlgHandle->tccId);

        /*
         * For chroma plane of 420SP
         */
        if(numPlanes == 2)
        {
            pAlgHandle->pParamSet->destAddr   = (UInt32)outPtr[1];
            pAlgHandle->pParamSet->srcAddr    = (UInt32)inPtr[1];
            pAlgHandle->pParamSet->srcBIdx    = inPitch[1];
            pAlgHandle->pParamSet->destBIdx   = outPitch[1];
            pAlgHandle->pParamSet->srcCIdx    = 0;
            pAlgHandle->pParamSet->destCIdx   = 0;
            pAlgHandle->pParamSet->aCnt       = lineSizeInBytes;
            pAlgHandle->pParamSet->bCnt       = height/2;
            pAlgHandle->pParamSet->cCnt       = 1;
            pAlgHandle->pParamSet->bCntReload = height/2;
            pAlgHandle->pParamSet->opt        = opt;
            pAlgHandle->pParamSet->linkAddr   = 0xFFFF;

            EDMA3_DRV_enableTransfer (pAlgHandle->hEdma,pAlgHandle->edmaChId,
                                               EDMA3_DRV_TRIG_MODE_MANUAL);
            EDMA3_DRV_waitAndClearTcc(pAlgHandle->hEdma,pAlgHandle->tccId);
        }

        EDMA3_DRV_clearErrorBits (pAlgHandle->hEdma,pAlgHandle->edmaChId);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Control for frame copy algo
 *
 * \param  algHandle             [IN] Algorithm object handle
 * \param  pControlParams        [IN] Pointer to Control Params
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FrameCopyControl(Alg_FrameCopy_Obj          *pAlgHandle,
                           Alg_FrameCopyControlParams *pControlParams)
{
    /*
     * Any alteration of algorithm behavior
     */
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop for frame copy algo
 *
 * \param  algHandle    [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FrameCopyStop(Alg_FrameCopy_Obj *algHandle)
{
      return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for frame copy algo
 *
 * \param  algHandle    [IN] Algorithm object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 Alg_FrameCopyDelete(Alg_FrameCopy_Obj *algHandle)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    Alg_FrameCopyDma_Obj * pAlgHandle;

    pAlgHandle = (Alg_FrameCopyDma_Obj *)algHandle;

    edma3Result = EDMA3_DRV_freeChannel(
                        pAlgHandle->hEdma,
                        pAlgHandle->edmaChId
                  );

    free(pAlgHandle);

    return edma3Result;
}

/* Nothing beyond this point */
