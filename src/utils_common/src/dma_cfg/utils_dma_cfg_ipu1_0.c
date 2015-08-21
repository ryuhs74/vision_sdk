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
 * \file utils_dma_cfg_ipu1_0.c
 *
 * \brief This file has the configuration for IPU1_0 CPU
 *
 * \version 0.0 (Aug 2013) : [KC] First version
 *
 *******************************************************************************
 */


/** Offset from which CPU interrupts start in control module XBAR config */
#define UTILS_IPU1_0_XBAR_OFFSET                  (23-1)

/** Interrupt no. for Transfer Completion */
#define UTILS_DMA_SYS_EDMA_CCXFER_COMPLETION_INT  (34)

/** Interrupt no. for CC Error */
#define UTILS_DMA_SYS_EDMA_CCERROR_INT            (35u)

/** Interrupt no. for TCs Error */
#define UTILS_DMA_SYS_EDMA_TC0_ERROR_INT          (36u)
#define UTILS_DMA_SYS_EDMA_TC1_ERROR_INT          (37u)
#define UTILS_DMA_SYS_EDMA_TC2_ERROR_INT          (0u)
#define UTILS_DMA_SYS_EDMA_TC3_ERROR_INT          (0u)
#define UTILS_DMA_SYS_EDMA_TC4_ERROR_INT          (0u)
#define UTILS_DMA_SYS_EDMA_TC5_ERROR_INT          (0u)
#define UTILS_DMA_SYS_EDMA_TC6_ERROR_INT          (0u)
#define UTILS_DMA_SYS_EDMA_TC7_ERROR_INT          (0u)

#define UTILS_DMA_SYSTEM_DMA_BASE_ADDR            (0x63300000)

#include "utils_dma_cfg_sys_edma.c"

/**
 *******************************************************************************
 * \brief System EDMA resource allocation for this CPU
 *******************************************************************************
 */
EDMA3_DRV_InstanceInitConfig gUtils_dmaSysEdmaInstInitConfig =
{
    /* 31     0                     63    32                    95    64                    127   96 */
    {UTILS_IPU1_0_EDMACH_ALLOC_0, UTILS_IPU1_0_EDMACH_ALLOC_1, UTILS_IPU1_0_PARAM_ALLOC_0, UTILS_IPU1_0_PARAM_ALLOC_1,
    /* 159  128                     191  160                    223  192                    255  224 */
     UTILS_IPU1_0_PARAM_ALLOC_2 , UTILS_IPU1_0_PARAM_ALLOC_3 , UTILS_IPU1_0_PARAM_ALLOC_4, UTILS_IPU1_0_PARAM_ALLOC_5,
    /* 287  256                     319  288                    351  320                    383  352 */
     UTILS_IPU1_0_PARAM_ALLOC_6 , UTILS_IPU1_0_PARAM_ALLOC_7 , UTILS_IPU1_0_PARAM_ALLOC_8, UTILS_IPU1_0_PARAM_ALLOC_9,
    /* 415  384                     447  416                    479  448                    511  480 */
     UTILS_IPU1_0_PARAM_ALLOC_10, UTILS_IPU1_0_PARAM_ALLOC_11, UTILS_IPU1_0_PARAM_ALLOC_12, UTILS_IPU1_0_PARAM_ALLOC_13},

    /* ownDmaChannels */
    /* 31     0                     63    32 */
    {UTILS_IPU1_0_EDMACH_ALLOC_0, UTILS_IPU1_0_EDMACH_ALLOC_1},

    /* ownQdmaChannels */
    /* 31     0 */
    {UTILS_IPU1_0_QDMACH_ALLOC_0},

    /* ownTccs */
    /* 31     0                     63    32 */
    {UTILS_IPU1_0_EDMACH_ALLOC_0, UTILS_IPU1_0_EDMACH_ALLOC_1},


    /* resvdPaRAMSets */
    /* 31     0     63    32     95    64     127   96 */
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
    /* 159  128     191  160     223  192     255  224 */
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
    /* 287  256     319  288     351  320     383  352 */
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
    /* 415  384     447  416     479  448     511  480 */
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,},

    /* resvdDmaChannels */
    /* 31     0    63     32 */
    {0x00000000u, 0x00000000u},

    /* resvdQdmaChannels */
    /* 31     0 */
    {0x00000000u},

    /* resvdTccs */
    /* 31     0    63     32 */
    {0x00000000u, 0x00000000u},
};

/**
 *******************************************************************************
 * \brief Interrupt related config for this CPU
 *******************************************************************************
 */
Utils_DmaIntrCfg gUtils_dmaSysEdmaIntrCfg;

/**
 *******************************************************************************
 * \brief See utils_dma_edma3cc.h for function description
 *******************************************************************************
 */
UInt32 Utils_edma3GetRegionId(unsigned int edmaInstId)
{
    (void)edmaInstId;

    return UTILS_DMA_SYSTEM_DMA_IPU1_0_REGION;
}

/**
 *******************************************************************************
 * \brief See utils_dma_edma3cc.h for function description
 *******************************************************************************
 */
Bool Utils_edma3IsGblConfigRequired(unsigned int edmaInstId)
{
    (void)edmaInstId;

    return TRUE;
}

/**
 *******************************************************************************
 * \brief See utils_dma_edma3cc.h for function description
 *******************************************************************************
 */
EDMA3_DRV_GblConfigParams *Utils_edma3GetGblCfg  (unsigned int edmaInstId)
{
    (void)edmaInstId;

    /* check allocation config statically */
    Utils_edma3CheckStaticAllocationConlficts();

    return &gUtils_dmaSysEdmaGblCfgParams;
}

/**
 *******************************************************************************
 * \brief See utils_dma_edma3cc.h for function description
 *******************************************************************************
 */
EDMA3_DRV_InstanceInitConfig *Utils_edma3GetInstCfg (unsigned int edmaInstId)
{
    (void)edmaInstId;
    return &gUtils_dmaSysEdmaInstInitConfig;
}

/**
 *******************************************************************************
 * \brief See utils_dma_edma3cc.h for function description
 *******************************************************************************
 */
Utils_DmaIntrCfg *Utils_edma3GetIntrCfg (unsigned int edmaInstId)
{
    gUtils_dmaSysEdmaIntrCfg.ccXferCompCpuInt
        = UTILS_DMA_SYS_EDMA_CCXFER_COMPLETION_INT;

    /* region interrupt used MUST match region returned via
     * Utils_edma3GetRegionId() in this file
     */
    gUtils_dmaSysEdmaIntrCfg.ccXferCompXbarInt
        = UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION0 +
            Utils_edma3GetRegionId(edmaInstId);
        ;

    gUtils_dmaSysEdmaIntrCfg.ccXferCompCtrlModXbarIndex
        = UTILS_DMA_SYS_EDMA_CCXFER_COMPLETION_INT - UTILS_IPU1_0_XBAR_OFFSET;

    gUtils_dmaSysEdmaIntrCfg.ccErrorCpuInt
        = UTILS_DMA_SYS_EDMA_CCERROR_INT;

    gUtils_dmaSysEdmaIntrCfg.ccErrorXbarInt
        = UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_ERR;

    gUtils_dmaSysEdmaIntrCfg.ccErrorCtrlModXbarIndex
        = UTILS_DMA_SYS_EDMA_CCERROR_INT - UTILS_IPU1_0_XBAR_OFFSET;

    gUtils_dmaSysEdmaIntrCfg.tc0ErrorCpuInt
        = UTILS_DMA_SYS_EDMA_TC0_ERROR_INT;

    gUtils_dmaSysEdmaIntrCfg.tc0ErrorXbarInt
        = UTILS_DMA_XBAR_SYS_EDMA_TC0_IRQ_ERR;

    gUtils_dmaSysEdmaIntrCfg.tc0ErrorCtrlModXbarIndex
        = UTILS_DMA_SYS_EDMA_TC0_ERROR_INT - UTILS_IPU1_0_XBAR_OFFSET;

    gUtils_dmaSysEdmaIntrCfg.tc1ErrorCpuInt
        = UTILS_DMA_SYS_EDMA_TC0_ERROR_INT;

    gUtils_dmaSysEdmaIntrCfg.tc1ErrorXbarInt
        = UTILS_DMA_XBAR_SYS_EDMA_TC0_IRQ_ERR;

    gUtils_dmaSysEdmaIntrCfg.tc1ErrorCtrlModXbarIndex
        = UTILS_DMA_SYS_EDMA_TC0_ERROR_INT - UTILS_IPU1_0_XBAR_OFFSET;

    return &gUtils_dmaSysEdmaIntrCfg;
}
