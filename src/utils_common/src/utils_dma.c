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
 * \file utils_dma.c
 *
 * \brief This file has the implementation of the system DMA APIs
 *
 * \version 0.0 (Aug 2013) : [KC] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_dma.h>
#include <src/utils_common/src/dma_cfg/utils_dma_edma3cc.h>

/*******************************************************************************
 *  Defines - private to DMA API implementation
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Compile time flag to control PaRAM parameter checking
 *******************************************************************************
 */
#define SYSTEM_UTILS_DMA_PARAM_CHECK





/**
 *******************************************************************************
 *
 * \brief Init DMA sub-system. MUST be called once before using the DMA APIs
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaInit()
{
    UInt32 edma3InstId;
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;

    if(System_getSelfProcId()==SYSTEM_PROC_DSP1
        ||
       System_getSelfProcId()==SYSTEM_PROC_DSP2
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE1
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE2
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE3
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE4
    )
    {
        edma3InstId = UTILS_DMA_LOCAL_EDMA_INST_ID;

        edma3Result = Utils_edma3Init(edma3InstId);
        if (edma3Result != EDMA3_DRV_SOK)
        {
            Vps_printf(
                   " UTILS: DMA: Utils_dmaInit() for instance %d "
                   "... FAILED (%d) \n",
                    edma3InstId,
                    edma3Result
            );
        }
    }

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0
        ||
       System_getSelfProcId()==SYSTEM_PROC_IPU1_1
        ||
       System_getSelfProcId()==SYSTEM_PROC_A15_0
        ||
       System_getSelfProcId()==SYSTEM_PROC_DSP1
        ||
       System_getSelfProcId()==SYSTEM_PROC_DSP2
    )
    {
        edma3InstId = UTILS_DMA_SYSTEM_EDMA_INST_ID;

        edma3Result = Utils_edma3Init(edma3InstId);
        if (edma3Result != EDMA3_DRV_SOK)
        {
            Vps_printf(
                   " UTILS: DMA: Utils_dmaInit() for instance %d "
                   "... FAILED (%d) \n",
                    edma3InstId,
                    edma3Result
            );
        }
    }

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief De-Init DMA sub-system.
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaDeInit()
{
    UInt32 edma3InstId;
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;

    if(System_getSelfProcId()==SYSTEM_PROC_DSP1
        ||
       System_getSelfProcId()==SYSTEM_PROC_DSP2
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE1
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE2
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE3
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE4
    )
    {
        edma3InstId = UTILS_DMA_LOCAL_EDMA_INST_ID;

        edma3Result = Utils_edma3DeInit(edma3InstId);
        if (edma3Result != EDMA3_DRV_SOK)
        {
            Vps_printf(
                   " UTILS: DMA: Utils_dmaDeInit() for instance %d "
                   "... FAILED (%d) \n",
                    edma3InstId,
                    edma3Result
            );
        }
    }

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0
        ||
       System_getSelfProcId()==SYSTEM_PROC_IPU1_1
        ||
       System_getSelfProcId()==SYSTEM_PROC_A15_0
        ||
       System_getSelfProcId()==SYSTEM_PROC_DSP1
        ||
       System_getSelfProcId()==SYSTEM_PROC_DSP2
    )
    {
        edma3InstId = UTILS_DMA_SYSTEM_EDMA_INST_ID;

        edma3Result = Utils_edma3DeInit(edma3InstId);
        if (edma3Result != EDMA3_DRV_SOK)
        {
            Vps_printf(
                   " UTILS: DMA: Utils_dmaDeInit() for instance %d "
                   "... FAILED (%d) \n",
                    edma3InstId,
                    edma3Result
            );
        }
    }
    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Create semaphore required for logical DMA channel
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaCreateSemaphores(Utils_DmaChObj *pObj)
{
    pObj->semComplete = BspOsal_semCreate(0u, TRUE);
    UTILS_assert(pObj->semComplete != NULL);

    pObj->semLock = BspOsal_semCreate(1u, TRUE);
    UTILS_assert(pObj->semLock != NULL);

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Cllback that is called with EDMA transfer completes
 *
 * \param tcc           [IN] TCC for which the tranfer completed
 * \param status        [IN] transfer completion status
 * \param appData       [IN] App data pointer set by user
 *                           during channel request
 *
 *******************************************************************************
 */
void Utils_dmaCallback(uint32_t tcc, EDMA3_RM_TccStatus status,
                            void *appData)
{
    (void)tcc;
    Utils_DmaChObj *pObj = (Utils_DmaChObj *)appData;

    switch (status)
    {
        case EDMA3_RM_XFER_COMPLETE:
            BspOsal_semPost(pObj->semComplete);
            break;
        case EDMA3_RM_E_CC_DMA_EVT_MISS:

            break;
        case EDMA3_RM_E_CC_QDMA_EVT_MISS:

            break;
        default:
            break;
    }
}

/**
 *******************************************************************************
 *
 * \brief Alloc EDMA channel for a logical DMA channel
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaAllocDmaCh(Utils_DmaChObj *pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    EDMA3_RM_TccCallback tccCb;

    pObj->tccId        = EDMA3_DRV_TCC_ANY;
    pObj->edmaChId    = EDMA3_DRV_DMA_CHANNEL_ANY;

    if(pObj->enableIntCb)
        tccCb = (EDMA3_RM_TccCallback)&Utils_dmaCallback;
    else
        tccCb = NULL;

    edma3Result = EDMA3_DRV_requestChannel(
                                    pObj->hEdma,
                                    (uint32_t*)&pObj->edmaChId,
                                    (uint32_t*)&pObj->tccId,
                                    (EDMA3_RM_EventQueue)pObj->eventQ,
                                    tccCb, (void *)pObj);

    if (edma3Result == EDMA3_DRV_SOK)
    {
        Vps_printf(" UTILS: DMA: Allocated CH (TCC) = %d (%d)\n",
                        pObj->edmaChId,
                        pObj->tccId);

        edma3Result = EDMA3_DRV_clearErrorBits(
                            pObj->hEdma,
                            pObj->edmaChId
                        );
    }
    else
    {
        Vps_printf(" UTILS: DMA: ERROR in EDMA CH allocation\n");
    }
    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Alloc PaRAM entry for a logical DMA channel
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaAllocParam(Utils_DmaChObj *pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    UInt32 paramId, paramPhyAddr, edmaChId, tccId;
    Utils_DmaTxObj  *pTxObj;

    /* allocate PaRAM entries */
    for(paramId=0; paramId<pObj->maxTransfers; paramId++)
    {
        pTxObj = &pObj->txObj[paramId];

        edma3Result = EDMA3_DRV_SOK;

        if(paramId==0)
        {
            /* first PaRAM used is equal to EDMA CH ID no need to allcaote it */
            pTxObj->paramId = pObj->edmaChId;
        }
        else
        {
            edmaChId = EDMA3_DRV_LINK_CHANNEL;
            tccId    = EDMA3_DRV_TCC_ANY;

            edma3Result = EDMA3_DRV_requestChannel (
                                    pObj->hEdma,
                                    &edmaChId,
                                    &tccId,
                                    (EDMA3_RM_EventQueue)pObj->eventQ,
                                    NULL,
                                    NULL
                                  );

            pTxObj->paramId = edmaChId;
        }

        if (edma3Result == EDMA3_DRV_SOK)
        {
            edma3Result = EDMA3_DRV_getPaRAMPhyAddr(
                                    pObj->hEdma,
                                    pTxObj->paramId,
                                    &paramPhyAddr);

            pTxObj->pParamSet = (EDMA3_DRV_PaRAMRegs*)paramPhyAddr;
        }

        pTxObj->pParamSet->destAddr   = 0;
        pTxObj->pParamSet->srcAddr    = 0;
        pTxObj->pParamSet->srcBIdx    = 0;
        pTxObj->pParamSet->destBIdx   = 0;
        pTxObj->pParamSet->srcCIdx    = 0;
        pTxObj->pParamSet->destCIdx   = 0;
        pTxObj->pParamSet->aCnt       = 0;
        pTxObj->pParamSet->bCnt       = 0;
        pTxObj->pParamSet->cCnt       = 1;
        pTxObj->pParamSet->bCntReload = 0;
        pTxObj->pParamSet->opt        = 0;
        pTxObj->pParamSet->linkAddr   = 0xFFFF;

        if (edma3Result == EDMA3_DRV_SOK)
        {
            Vps_printf(" UTILS: DMA: %d of %d: Allocated PaRAM = %d (0x%08X)\n",
                paramId,
                pObj->maxTransfers,
                pTxObj->paramId,
                pTxObj->pParamSet);
        }
        else
        {
            Vps_printf(" UTILS: DMA: %d of %d: ERROR in PaRAM allocation\n",
                paramId,
                pObj->maxTransfers);
            break;
        }
    }

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Free a allocated EDMACH and PaRAM entry for a logical DMA channel
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaFreeDmaChParam(Utils_DmaChObj *pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    UInt32 paramId;

    for(paramId=0; paramId<pObj->maxTransfers; paramId++)
    {
        edma3Result = EDMA3_DRV_freeChannel(
                            pObj->hEdma,
                            pObj->txObj[paramId].paramId
                      );
    }

    return edma3Result;
}


/**
 *******************************************************************************
 *
 * \brief Create a logical DMA channel
 *
 *        - Allcoates 1 EDMA CH + 1 PaRAM entry by default
 *        - Allocates more PaRAM entries depending on value of
 *          Utils_DmaChCreateParams.maxTransfers
 *
 * \param pObj      [OUT] Logical DMA channel handle
 * \param pPrm      [IN]  create time parameters
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaCreateCh(Utils_DmaChObj *pObj, Utils_DmaChCreateParams *pPrm)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    UInt32 maxTransfers, edmaInstId;

    memset(pObj, 0, sizeof(*pObj));

    maxTransfers = pPrm->maxTransfers;
    edmaInstId   = pPrm->edmaInstId;

    if(maxTransfers<1)
        maxTransfers = 1;

    if(maxTransfers>UTILS_DMA_MAX_TX_OBJ)
        maxTransfers = UTILS_DMA_MAX_TX_OBJ;

    if(edmaInstId>=UTILS_DMA_MAX_EDMA_INST)
        edmaInstId = UTILS_DMA_SYSTEM_EDMA_INST_ID;

    pObj->maxTransfers= maxTransfers;
    pObj->eventQ      = pPrm->eventQ;

    if(Utils_dmaIsIntrSupported(edmaInstId))
    {
        pObj->enableIntCb = pPrm->enableIntCb;
    }
    else
    {
        /* force dsiable interrupt mode if not supported */
        pObj->enableIntCb = FALSE;
    }

    pObj->hEdma       = Utils_dmaGetEdma3Hndl(edmaInstId);

    UTILS_assert(pObj->hEdma!=NULL);

    memset(pObj->txObj, 0, sizeof(pObj->txObj));

    if (edma3Result == EDMA3_DRV_SOK)
        edma3Result |= Utils_dmaCreateSemaphores(pObj);

    if (edma3Result == EDMA3_DRV_SOK)
        edma3Result |= Utils_dmaAllocDmaCh(pObj);

    if (edma3Result == EDMA3_DRV_SOK)
        edma3Result |= Utils_dmaAllocParam(pObj);

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Delete a logical DMA channel
 *
 *        - Free's DMA's CH, PaRAM entries allocated during create
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaDeleteCh(Utils_DmaChObj *pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;

    edma3Result = Utils_dmaFreeDmaChParam(pObj);

    BspOsal_semDelete(&pObj->semComplete);
    BspOsal_semDelete(&pObj->semLock);

    Vps_printf(" UTILS: DMA: Free'ed CH (TCC) = %d (%d)\n",
                    pObj->edmaChId,
                    pObj->tccId);

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Polling based wait for completion
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaPollWaitComplete(Utils_DmaChObj *pObj)
{
    uint16_t tccStatus;

    EDMA3_DRV_waitAndClearTcc(pObj->hEdma,pObj->tccId);
    EDMA3_DRV_checkAndClearTcc(pObj->hEdma,pObj->tccId, &tccStatus);
    EDMA3_DRV_clearErrorBits (pObj->hEdma,pObj->edmaChId);

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Trigger EDMA channel and wait for completion
 *
 * \param pObj      [IN] Logical DMA channel handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaTriggerAndWait(Utils_DmaChObj *pObj)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    uint16_t tccStatus;

    edma3Result |= EDMA3_DRV_checkAndClearTcc(pObj->hEdma,
                                    pObj->tccId,
                                    &tccStatus);

    edma3Result |= EDMA3_DRV_clearErrorBits (pObj->hEdma,pObj->edmaChId);

    edma3Result |= EDMA3_DRV_enableTransfer (pObj->hEdma,pObj->edmaChId,
                                           EDMA3_DRV_TRIG_MODE_MANUAL);

    if (edma3Result != EDMA3_DRV_SOK)
    {
        Vps_printf(
            " UTILS: DMA: Utils_dmaTrigger() ... FAILED (CH=%d) \n",
                pObj->edmaChId, edma3Result
            );
    }

    if (edma3Result == EDMA3_DRV_SOK)
    {
        if(pObj->enableIntCb)
        {
            BspOsal_semWait(pObj->semComplete, BSP_OSAL_WAIT_FOREVER);
        }
        else
        {
            Utils_dmaPollWaitComplete(pObj);
        }
    }

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Check if PaRAM set values are valid
 *
 * \param pParamSet      [IN] PaRAM set pointer
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaParamSetCheck(EDMA3_DRV_PaRAMRegs *pParamSet)
{
    if(pParamSet->aCnt == 0
        ||
       pParamSet->bCnt == 0
        ||
       pParamSet->cCnt == 0
        ||
       pParamSet->destAddr == 0
        ||
       pParamSet->srcAddr == 0
        ||
       pParamSet->bCntReload == 0
    )
    {
        return FVID2_EFAIL;
    }

    return FVID2_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Link PaRAM entires to numTx transfers
 *
 * \param pObj      [IN] Logical DMA channel handle
 * \param numTx     [IN] Number of transfers to perform
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaLinkDma(Utils_DmaChObj *pObj, UInt32 numTx)
{
    UInt32 i;
    EDMA3_DRV_PaRAMRegs *pParamSet;
    UInt32 opt;

    /* link the EDMAs */
    for(i=0; i<numTx; i++)
    {
        pParamSet   = pObj->txObj[i].pParamSet;

        opt  =
                 (EDMA3_CCRL_OPT_ITCCHEN_ENABLE << EDMA3_CCRL_OPT_ITCCHEN_SHIFT)
               | ((pObj->tccId << EDMA3_CCRL_OPT_TCC_SHIFT)
                        & EDMA3_CCRL_OPT_TCC_MASK
                 )
               | (EDMA3_CCRL_OPT_SYNCDIM_ABSYNC << EDMA3_CCRL_OPT_SYNCDIM_SHIFT)
                  ;

        if( i== (numTx-1) )
        {
            /* last DMA */

            /* enable interrupt for last Transfer ONLY */
            opt |= (EDMA3_CCRL_OPT_TCINTEN_ENABLE
                        << EDMA3_CCRL_OPT_TCINTEN_SHIFT);

            /* do not link DMA */
            pParamSet->linkAddr = 0xFFFF;
        }
        else
        {
            /* enable chaining after transfer complete */
            opt |= (EDMA3_CCRL_OPT_TCCHEN_ENABLE
                        << EDMA3_CCRL_OPT_TCCHEN_SHIFT);

            /* link current DMA to next DMA */
            pParamSet->linkAddr = (UInt32)pObj->txObj[i+1].pParamSet;
        }

        pParamSet->opt = opt;
    }

    return 0;
}

/**
 *******************************************************************************
 *
 * \brief Link PaRAM entires, trigger EDMA and wait fior completion
 *
 * \param pObj      [IN] Logical DMA channel handle
 * \param numTx     [IN] Number of transfer's to perform \n
 *                  [OUT] Number of transfer's remaining
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaRun(Utils_DmaChObj *pObj, UInt32 *numTx)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;

    if(*numTx==0)
        return edma3Result;

    Utils_dmaLinkDma(pObj, *numTx);

    edma3Result = Utils_dmaTriggerAndWait(pObj);

    *numTx = 0;

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Copy data using EDMA
 *
 *        The API takes a mutual exclusion lock to allow the same API on the
 *        same logical channel be called from multiple thread contexts
 *
 * \param pObj             [IN] Logical DMA channel handle
 * \param pPrm             [IN] parameters for copy- srcAddr, dstAddr, length
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaCopy1D(Utils_DmaChObj *pObj, Utils_DmaCopy1D *pPrm)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    EDMA3_DRV_PaRAMRegs *pParamSet;
    UInt32 opt;
    UInt32          bCntValue = 1U;
    UInt32          remBytes  = 0;
    UInt32          length    = pPrm->length;
    UInt32          aCntValue = length;
    UInt32          addr      = (UInt32) (pPrm->destAddr);
    UInt32          maxAcnt   = 0x7FFFU;

    BspOsal_semWait(pObj->semLock, BSP_OSAL_WAIT_FOREVER);

    if (length > maxAcnt)
    {
        bCntValue = (length / maxAcnt);
        remBytes  = (length % maxAcnt);
        aCntValue = maxAcnt;
    }

    /* Compute QSPI address and size */
    pParamSet             = pObj->txObj[0].pParamSet;
    pParamSet->opt        = 0;
    pParamSet->srcAddr    = (UInt32)pPrm->srcAddr;
    pParamSet->destAddr   = addr;
    pParamSet->aCnt       = aCntValue;
    pParamSet->bCnt       = bCntValue;
    pParamSet->cCnt       = 1;
    pParamSet->srcBIdx    = aCntValue;
    pParamSet->destBIdx   = aCntValue;
    pParamSet->srcCIdx    = 0;
    pParamSet->destCIdx   = 0;
    pParamSet->linkAddr   = 0xFFFF;
    opt  =
             (EDMA3_CCRL_OPT_ITCCHEN_ENABLE << EDMA3_CCRL_OPT_ITCCHEN_SHIFT)
           | ((pObj->tccId << EDMA3_CCRL_OPT_TCC_SHIFT)
                    & EDMA3_CCRL_OPT_TCC_MASK
             )
           | (EDMA3_CCRL_OPT_SYNCDIM_ABSYNC << EDMA3_CCRL_OPT_SYNCDIM_SHIFT)
              ;
    /* enable interrupt for last Transfer ONLY */
    opt |= (EDMA3_CCRL_OPT_TCINTEN_ENABLE
                << EDMA3_CCRL_OPT_TCINTEN_SHIFT);

    pParamSet->opt = opt;
    edma3Result = Utils_dmaTriggerAndWait(pObj);
    if (edma3Result != EDMA3_DRV_SOK)
    {
        Vps_printf(
            " UTILS: DMA: Utils_dmaTrigger() ... FAILED (CH=%d) \n",
                pObj->edmaChId, edma3Result
            );
    }

    if (remBytes != 0)
    {
        /* Compute QSPI address and size */
        pParamSet             = pObj->txObj[0].pParamSet;
        pParamSet->opt        = 0;
        pParamSet->srcAddr    = (UInt32)((UInt32)pPrm->srcAddr + (bCntValue * aCntValue));
        pParamSet->destAddr   = (UInt32)(addr + (aCntValue * bCntValue));
        pParamSet->aCnt       = remBytes;
        pParamSet->bCnt       = 1;
        pParamSet->cCnt       = 1;
        pParamSet->srcBIdx    = remBytes;
        pParamSet->destBIdx   = remBytes;
        pParamSet->srcCIdx    = 0;
        pParamSet->destCIdx   = 0;
        pParamSet->linkAddr   = 0xFFFF;
        opt  =
                 (EDMA3_CCRL_OPT_ITCCHEN_ENABLE << EDMA3_CCRL_OPT_ITCCHEN_SHIFT)
               | ((pObj->tccId << EDMA3_CCRL_OPT_TCC_SHIFT)
                        & EDMA3_CCRL_OPT_TCC_MASK
                 )
               | (EDMA3_CCRL_OPT_SYNCDIM_ABSYNC << EDMA3_CCRL_OPT_SYNCDIM_SHIFT)
                  ;
        /* enable interrupt for last Transfer ONLY */
        opt |= (EDMA3_CCRL_OPT_TCINTEN_ENABLE
                    << EDMA3_CCRL_OPT_TCINTEN_SHIFT);

        pParamSet->opt = opt;
        edma3Result = Utils_dmaTriggerAndWait(pObj);
        if (edma3Result != EDMA3_DRV_SOK)
        {
            Vps_printf(
                " UTILS: DMA: Utils_dmaTrigger() ... FAILED (CH=%d) \n",
                    pObj->edmaChId, edma3Result
                );
        }

    }

    BspOsal_semPost(pObj->semLock);

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Copy or fill data using EDMA
 *
 *        The API takes a mutual exclusion lock to allow the same API on the
 *        same logical channel be called from multiple thread contexts
 *
 * \param pObj          [IN] Logical DMA channel handle
 * \param pInfo         [IN] Array of DMA transfer parameter's
 * \param numTransfers  [IN] Number of transfers to perform
 * \param fillData      [IN] TRUE: Fill data in destination buffer, \n
 *                           FALSE: Copy data to destination buffer
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaCopyFill2D(Utils_DmaChObj *pObj, Utils_DmaCopyFill2D *pInfo,
                        UInt32 numTransfers, Bool fillData)
{
    EDMA3_DRV_Result edma3Result = EDMA3_DRV_SOK;
    EDMA3_DRV_PaRAMRegs *pParamSet;
    UInt32 bpp; /* bytes per pixel */
    UInt32 i, numTx;

    BspOsal_semWait(pObj->semLock, BSP_OSAL_WAIT_FOREVER);

    numTx = 0;

    for(i=0; i<numTransfers; i++)
    {
        pParamSet   = pObj->txObj[numTx].pParamSet;

        if(pInfo->dataFormat==SYSTEM_DF_RAW24)
            bpp = 3;
        else
        if(pInfo->dataFormat==SYSTEM_DF_RAW16)
            bpp = 2;
        else
        if(pInfo->dataFormat==SYSTEM_DF_RAW08)
            bpp = 1;
        else
        if(pInfo->dataFormat==SYSTEM_DF_YUV420SP_UV)
            bpp = 1;
        else
            bpp = 1;

        pParamSet->destAddr   = (UInt32)pInfo->destAddr[0]
                              + pInfo->destPitch[0]*pInfo->destStartY
                              + pInfo->destStartX * bpp;

        if(fillData)
        {
            pParamSet->srcAddr   = (UInt32)pInfo->srcAddr[0];
            pParamSet->srcBIdx   = 0;
        }
        else
        {
            pParamSet->srcAddr   = (UInt32)pInfo->srcAddr[0]
                                  + pInfo->srcPitch[0]*pInfo->srcStartY
                                  + pInfo->srcStartX * bpp;
            pParamSet->srcBIdx    = pInfo->srcPitch[0];
        }

        pParamSet->destBIdx   = pInfo->destPitch[0];

        pParamSet->aCnt       = (pInfo->width*bpp);
        pParamSet->bCnt       = pInfo->height;
        pParamSet->cCnt       = 1;

        pParamSet->bCntReload = pParamSet->bCnt;

        #ifdef SYSTEM_UTILS_DMA_PARAM_CHECK
        UTILS_assert(
            Utils_dmaParamSetCheck(pParamSet) == FVID2_SOK
        );
        #endif

        if(pInfo->dataFormat==SYSTEM_DF_YUV420SP_UV)
        {
            numTx++;

            if(numTx>=pObj->maxTransfers)
            {
                edma3Result |= Utils_dmaRun(pObj, &numTx);
            }

            /* setup PaRAM for UV plane */
            pParamSet   = pObj->txObj[numTx].pParamSet;

            bpp = 1;

            /*
             * For chroma plane when input or output is tiled we need to set
             * pitch as -32KB since other wise pitch value will overflow
             * the 16-bit register in EDMA.
             * Hence all for all Chroma TX's we set pitch as -pitch and DMA
             * from last line to first line
             *
             */

            pParamSet->destAddr   = (UInt32)pInfo->destAddr[1]
                                  + pInfo->destPitch[1]
                                    *((pInfo->destStartY+pInfo->height)/2-1)
                                  + pInfo->destStartX * bpp;

            if(fillData)
            {
                pParamSet->srcAddr   = (UInt32)pInfo->srcAddr[1];
                pParamSet->srcBIdx   = 0;
            }
            else
            {
                pParamSet->srcAddr   = (UInt32)pInfo->srcAddr[1]
                                      + pInfo->srcPitch[1]
                                        *((pInfo->srcStartY+pInfo->height)/2-1)
                                      + pInfo->srcStartX * bpp;
                pParamSet->srcBIdx   = -pInfo->srcPitch[1];
            }

            pParamSet->destBIdx   = -pInfo->destPitch[1];

            pParamSet->aCnt       = (pInfo->width*bpp);
            pParamSet->bCnt       = pInfo->height/2;
            pParamSet->cCnt       = 1;

            pParamSet->bCntReload = pParamSet->bCnt;

            #ifdef SYSTEM_UTILS_DMA_PARAM_CHECK
            UTILS_assert(
                Utils_dmaParamSetCheck(pParamSet) == FVID2_SOK
            );
            #endif
        }

        /* goto next tranfer information */
        pInfo++;
        numTx++;

        if(numTx>=pObj->maxTransfers)
        {
            edma3Result |= Utils_dmaRun(pObj, &numTx);
        }
    }

    if(numTx)
    {
        edma3Result |= Utils_dmaRun(pObj, &numTx);
    }

    BspOsal_semPost(pObj->semLock);

    return edma3Result;
}

/**
 *******************************************************************************
 *
 * \brief Fill data using EDMA
 *
 *        The API takes a mutual exclusion lock to allow the same API on the
 *        same logical channel be called from multiple thread contexts
 *
 * \param pObj          [IN] Logical DMA channel handle
 * \param pInfo         [IN] Array of DMA transfer parameter's
 * \param numTransfers  [IN] Number of transfers to perform
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaFill2D(Utils_DmaChObj *pObj, Utils_DmaCopyFill2D *pInfo,
                        UInt32 numTransfers)
{
    return Utils_dmaCopyFill2D(pObj, pInfo, numTransfers, TRUE);
}

/**
 *******************************************************************************
 *
 * \brief Copy data using EDMA
 *
 *        The API takes a mutual exclusion lock to allow the same API on the
 *        same logical channel be called from multiple thread contexts
 *
 * \param pObj          [IN] Logical DMA channel handle
 * \param pInfo         [IN] Array of DMA transfer parameter's
 * \param numTransfers  [IN] Number of transfers to perform
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaCopy2D(Utils_DmaChObj *pObj, Utils_DmaCopyFill2D *pInfo,
                UInt32 numTransfers)
{
    return Utils_dmaCopyFill2D(pObj, pInfo, numTransfers, FALSE);
}

#include <src/utils_common/include/utils_mem.h>
#include <ti/sysbios/hal/Cache.h>
/**
 *******************************************************************************
 * \brief Max parameters to test DMA APIs for
 *******************************************************************************
 */
#define UTILS_DMA_TEST_MAX_PRM      (20)

/**
 *******************************************************************************
 *
 * \brief Sample test routine to test the DMA APIs
 *
 *        This tests the APIs only from completion interrupt point of view
 *        Data transfer validity is not checked.
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_dmaTestCopyFill(Int32 doInitDeInit)
{
    Int32 status;
    Utils_DmaChObj  chDmaObj;
    Utils_DmaChCreateParams chDmaCreatePrm;
    Utils_DmaCopyFill2D dmaCopyFill2D[UTILS_DMA_TEST_MAX_PRM];
    Int32 count, i, numTx;
    UInt32 *pCurAddr;
    Ptr srcAddr[2];
    Ptr dstAddr[2];
    UInt32 width, height, memSize, timeInMs;
    UInt32 mode;

    if(doInitDeInit)
        Utils_dmaInit();

    count  = 10;
    numTx  = UTILS_DMA_TEST_MAX_PRM;
    width  = 1920;
    height = 1080;

    memSize = width*height*6;

    srcAddr[0] = Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        memSize, 16);
    UTILS_assert(srcAddr[0]!=NULL);
    srcAddr[1] = (Ptr)((UInt32)srcAddr[0] + width*height);
    dstAddr[0] = (Ptr)((UInt32)srcAddr[0] + memSize/2   );
    dstAddr[1] = (Ptr)((UInt32)dstAddr[0] + width*height);

    memset(srcAddr[0], 0, memSize/2);

    pCurAddr = (Uint32*)srcAddr[0];
    for(i=0; i<(memSize/2)/4; i++) {
        *pCurAddr++ = i;
    }

    memset(dstAddr[0], 0, memSize/2);

    Cache_wbInv(srcAddr[0], memSize, Cache_Type_ALL, TRUE);

    Vps_printf(" UTILS: DMA: DMA Test (SRC = 0x%08x, DST = 0x%08x)!!!\n",
                    (Uint32)srcAddr[0], (Uint32)dstAddr[0]
                );

    for(i=0; i<UTILS_DMA_TEST_MAX_PRM; i++)
    {
        dmaCopyFill2D[i].dataFormat    = SYSTEM_DF_YUV420SP_UV;
        dmaCopyFill2D[i].destAddr[0]   = dstAddr[0];
        dmaCopyFill2D[i].destAddr[1]   = dstAddr[1];
        dmaCopyFill2D[i].destPitch[0]  = width;
        dmaCopyFill2D[i].destPitch[1]  = width;
        dmaCopyFill2D[i].destStartX    = 0;
        dmaCopyFill2D[i].destStartY    = 0;
        dmaCopyFill2D[i].width         = width;
        dmaCopyFill2D[i].height        = height;
        dmaCopyFill2D[i].srcAddr[0]    = srcAddr[0];
        dmaCopyFill2D[i].srcAddr[1]    = srcAddr[1];
        dmaCopyFill2D[i].srcPitch[0]   = width;
        dmaCopyFill2D[i].srcPitch[1]   = width;
        dmaCopyFill2D[i].srcStartX     = 0;
        dmaCopyFill2D[i].srcStartY     = 0;
    }

    Vps_printf(" UTILS: DMA: TEST: Started !!!"
               " (%d TXs per DMA API in a loop of %d)\r\n",
            numTx,
            count
        );

    for(mode=0; mode<3; mode++)
    {
        Utils_DmaChCreateParams_Init(&chDmaCreatePrm);

        chDmaCreatePrm.maxTransfers = numTx/3;

        if(mode==0)
        {
            Vps_printf(" UTILS: DMA: TEST: Test mode = Default EDMA, POLLED !!!");
            chDmaCreatePrm.enableIntCb = FALSE;
        }
        if(mode==1)
        {
            Vps_printf(" UTILS: DMA: TEST: Test mode = Default EDMA, INTERRUPT !!!");
            chDmaCreatePrm.enableIntCb = TRUE;
        }
        if(mode==2)
        {
            if(System_getSelfProcId()==SYSTEM_PROC_DSP1
                ||
                System_getSelfProcId()==SYSTEM_PROC_DSP2
            )
            {
                /* system and local EDMA only supported in DSP */
                Vps_printf(" UTILS: DMA: TEST: Test mode = System EDMA, POLLED mode !!!");
                chDmaCreatePrm.edmaInstId = UTILS_DMA_LOCAL_EDMA_INST_ID;
                chDmaCreatePrm.enableIntCb = FALSE;
            }
            else
            {
                Vps_printf(" UTILS: DMA: TEST: Test mode = Default EDMA, INTERRUPT !!!");
                chDmaCreatePrm.enableIntCb = TRUE;
            }
        }

        status = Utils_dmaCreateCh(&chDmaObj, &chDmaCreatePrm);
        UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

        timeInMs = Utils_getCurGlobalTimeInMsec();

        for(i=0; i<count; i++)
        {
            status = Utils_dmaCopy2D(&chDmaObj, dmaCopyFill2D, numTx);
            UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

            status = Utils_dmaFill2D(&chDmaObj, dmaCopyFill2D, numTx);
            UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
        }

        timeInMs = Utils_getCurGlobalTimeInMsec() - timeInMs;

        Vps_printf(" UTILS: DMA: TEST: Completed in %d msecs \r\n", timeInMs);

        Utils_dmaDeleteCh(&chDmaObj);
    }
    Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, srcAddr[0], memSize);

     if(doInitDeInit)
        Utils_dmaDeInit();

    return status;
}

