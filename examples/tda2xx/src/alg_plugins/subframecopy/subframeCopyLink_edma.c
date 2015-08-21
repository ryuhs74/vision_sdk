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
 *
 * \file subframeCopyLink_edma.c
 *
 * \brief Algorithm for subframe copy on EVE using local edma
 *
 *        This implementation is taken from framecopyAlgoLocalDma.c, with minor
          changes. Its only for demonstrative purpose and NOT product quality.
 *        This algorithm does a subframe copy.
 *        This implementation uses DMA to copy the data
 *        EDMA3LLD APIs are used to copy the subframe data.
 *        EDMA3LLD is initialized via Utils_dmaInit() during system init.
 *
 * \version 0.0 (Sept 2014) : [VT] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "src/utils_common/include/utils_dma.h"
#include "subframeCopyLink_priv.h"

/**
 *******************************************************************************
 *
 * \brief Implementation of create for subframe copy algo
 *
 * \param  AlgorithmLink_SubframeCopyObj    [IN] Subframe copy object
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_SubframeCopyEDMACreate
                                        (AlgorithmLink_SubframeCopyObj * pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    UInt32 paramPhyAddr;

    UTILS_assert(pObj != NULL);

    pObj->EDMA_subframeToDDR.hEdma     =
        Utils_dmaGetEdma3Hndl(UTILS_DMA_LOCAL_EDMA_INST_ID);

    UTILS_assert(pObj->EDMA_subframeToDDR.hEdma!=NULL);

    pObj->EDMA_subframeToDDR.tccId       = EDMA3_DRV_TCC_ANY;
    pObj->EDMA_subframeToDDR.edmaChId    = EDMA3_DRV_DMA_CHANNEL_ANY;

    edma3Result = EDMA3_DRV_requestChannel(
                                pObj->EDMA_subframeToDDR.hEdma,
                                (uint32_t*)&pObj->EDMA_subframeToDDR.edmaChId,
                                (uint32_t*)&pObj->EDMA_subframeToDDR.tccId,
                                (EDMA3_RM_EventQueue)0,
                                NULL, (void *)(&pObj->EDMA_subframeToDDR));

    if (edma3Result == EDMA3_DRV_SOK)
    {
        Vps_printf(" ALG_SUBFRAME DMA: Allocated CH (TCC) = %d (%d)\n",
                        pObj->EDMA_subframeToDDR.edmaChId,
                        pObj->EDMA_subframeToDDR.tccId);

        edma3Result = EDMA3_DRV_clearErrorBits(
                            pObj->EDMA_subframeToDDR.hEdma,
                            pObj->EDMA_subframeToDDR.edmaChId
                        );
    }
    else
    {
        Vps_printf(" ALG_SUBFRAME: DMA: ERROR in EDMA CH allocation\n");
    }

    if (edma3Result == EDMA3_DRV_SOK)
    {
        edma3Result = EDMA3_DRV_getPaRAMPhyAddr(
                                pObj->EDMA_subframeToDDR.hEdma,
                                pObj->EDMA_subframeToDDR.edmaChId,
                                &paramPhyAddr);

        pObj->EDMA_subframeToDDR.pParamSet = (EDMA3_DRV_PaRAMRegs*)paramPhyAddr;
    }

    UTILS_assert(edma3Result == EDMA3_DRV_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process for subframe copy algo
 *
 *        Supported formats are SYSTEM_DF_YUV422I_YUYV, SYSTEM_DF_YUV420SP_UV.
 *        It is assumed that the width of the image will
 *        be multiple of 4 and buffer pointers are 32-bit aligned.
 *
 * \param  AlgorithmLink_SubframeCopyObj [IN] Sub frame copy object handle
 * \param  inPtr[]      [IN] Array of input pointers
 *                           Index 0 - Pointer to Y data in case of YUV420SP,
 *                                   - Single pointer for YUV422IL or RGB
 *                           Index 1 - Pointer to UV data in case of YUV420SP
 * \param  outPtr[]     [IN] Array of output pointers. Indexing similar to
 *                           array of input pointers
 * \param  width        [IN] width of image
 * \param  height       [IN] height - number of vertical lines to be copied
 * \param  inPitch[]    [IN] Array of pitch of input image (Address offset
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  outPitch[]   [IN] Array of pitch of output image (Address offset
 *                           b.n. two  consecutive lines, interms of bytes)
 *                           Indexing similar to array of input pointers
 * \param  dataFormat   [IN] Different image data formats. Refer
 *                           System_VideoDataFormat
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_SubframeCopyEDMACopy
                        (AlgorithmLink_SubframeCopyObj * pObj,
                           UInt32            *inPtr[],
                           UInt32            *outPtr[],
                           UInt32             width,
                           UInt32             height,
                           UInt32             inPitch[],
                           UInt32             outPitch[],
                           UInt32             dataFormat)
{
    UInt32 numPlanes;
    UInt32 lineSizeInBytes;
    UInt32 opt;
    uint16_t tccStatus;
    AlgorithmLink_SubframeCopyEdma_Obj *pEDMA_subframeToDDR;

    pEDMA_subframeToDDR
        = (AlgorithmLink_SubframeCopyEdma_Obj *)(&pObj->EDMA_subframeToDDR);

    if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
    {
        numPlanes = 1;
        lineSizeInBytes = width*2;
    }
    else if(dataFormat == SYSTEM_DF_YUV420SP_UV)
    {
        numPlanes = 2;
        lineSizeInBytes = width;
    }
    else
    {
        return SYSTEM_LINK_STATUS_EFAIL;
    }

    opt  =
        (EDMA3_CCRL_OPT_ITCCHEN_ENABLE << EDMA3_CCRL_OPT_ITCCHEN_SHIFT)
           |
        ((pEDMA_subframeToDDR->tccId << EDMA3_CCRL_OPT_TCC_SHIFT)
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
    pEDMA_subframeToDDR->pParamSet->destAddr   = (UInt32)outPtr[0];
    pEDMA_subframeToDDR->pParamSet->srcAddr    = (UInt32)inPtr[0];
    pEDMA_subframeToDDR->pParamSet->srcBIdx    = inPitch[0];
    pEDMA_subframeToDDR->pParamSet->destBIdx   = outPitch[0];
    pEDMA_subframeToDDR->pParamSet->srcCIdx    = 0;
    pEDMA_subframeToDDR->pParamSet->destCIdx   = 0;
    pEDMA_subframeToDDR->pParamSet->aCnt       = lineSizeInBytes;
    pEDMA_subframeToDDR->pParamSet->bCnt       = height;
    pEDMA_subframeToDDR->pParamSet->cCnt       = 1;
    pEDMA_subframeToDDR->pParamSet->bCntReload = height;
    pEDMA_subframeToDDR->pParamSet->opt        = opt;
    pEDMA_subframeToDDR->pParamSet->linkAddr   = 0xFFFF;

    EDMA3_DRV_checkAndClearTcc(pEDMA_subframeToDDR->hEdma,
                               pEDMA_subframeToDDR->tccId,
                               &tccStatus);
    EDMA3_DRV_clearErrorBits(pEDMA_subframeToDDR->hEdma,
                            pEDMA_subframeToDDR->edmaChId);
    EDMA3_DRV_enableTransfer(pEDMA_subframeToDDR->hEdma,
                            pEDMA_subframeToDDR->edmaChId,
                            EDMA3_DRV_TRIG_MODE_MANUAL);
    EDMA3_DRV_waitAndClearTcc(pEDMA_subframeToDDR->hEdma,
                            pEDMA_subframeToDDR->tccId);

    /*
     * For chroma plane of 420SP
     */
    if(numPlanes == 2)
    {
        pEDMA_subframeToDDR->pParamSet->destAddr   = (UInt32)outPtr[1];
        pEDMA_subframeToDDR->pParamSet->srcAddr    = (UInt32)inPtr[1];
        pEDMA_subframeToDDR->pParamSet->srcBIdx    = inPitch[1];
        pEDMA_subframeToDDR->pParamSet->destBIdx   = outPitch[1];
        pEDMA_subframeToDDR->pParamSet->srcCIdx    = 0;
        pEDMA_subframeToDDR->pParamSet->destCIdx   = 0;
        pEDMA_subframeToDDR->pParamSet->aCnt       = lineSizeInBytes;
        pEDMA_subframeToDDR->pParamSet->bCnt       = height/2;
        pEDMA_subframeToDDR->pParamSet->cCnt       = 1;
        pEDMA_subframeToDDR->pParamSet->bCntReload = height/2;
        pEDMA_subframeToDDR->pParamSet->opt        = opt;
        pEDMA_subframeToDDR->pParamSet->linkAddr   = 0xFFFF;

        EDMA3_DRV_enableTransfer
            (pEDMA_subframeToDDR->hEdma,pEDMA_subframeToDDR->edmaChId,
                                           EDMA3_DRV_TRIG_MODE_MANUAL);
        EDMA3_DRV_waitAndClearTcc
            (pEDMA_subframeToDDR->hEdma,pEDMA_subframeToDDR->tccId);
    }

    EDMA3_DRV_clearErrorBits
        (pEDMA_subframeToDDR->hEdma,pEDMA_subframeToDDR->edmaChId);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete for subframe copy algo
 *
 * \param  AlgorithmLink_SubframeCopyObj    [IN] subframe copy object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_SubframeCopyEDMADelete
                                        (AlgorithmLink_SubframeCopyObj * pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    AlgorithmLink_SubframeCopyEdma_Obj *pEDMA_subframeToDDR;

    pEDMA_subframeToDDR =
            (AlgorithmLink_SubframeCopyEdma_Obj *)(&pObj->EDMA_subframeToDDR);

    edma3Result = EDMA3_DRV_freeChannel(
                        pEDMA_subframeToDDR->hEdma,
                        pEDMA_subframeToDDR->edmaChId
                  );

    return edma3Result;
}

/* Nothing beyond this point */
