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
 * \file utils_dma_cfg_sys_edma.c
 *
 * \brief This file has the common configuration for System EDMA controller
 *
 * \version 0.0 (Aug 2013) : [KC] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/src/dma_cfg/utils_dma_edma3cc.h>

/*
 *******************************************************************************
 *
 *   This file in included in
 *   - utils_dma_cfg_a15_0.c
 *   - utils_dma_cfg_ipu1_0.c
 *   - utils_dma_cfg_ipu1_1.c
 *   - utils_dma_cfg_dsp1.c
 *   - utils_dma_cfg_dsp2.c
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  EDMA controller region allocation based on CPU ID
 *
 *  Region ID's MUST be unique across different CPUs
 *
 *  Max number of regions can be 8
 *******************************************************************************
 */
#define UTILS_DMA_SYSTEM_DMA_A15_0_REGION       (0)
#define UTILS_DMA_SYSTEM_DMA_IPU1_0_REGION      (1)
#define UTILS_DMA_SYSTEM_DMA_IPU1_1_REGION      (2)
#define UTILS_DMA_SYSTEM_DMA_DSP1_REGION        (3)
#define UTILS_DMA_SYSTEM_DMA_DSP2_REGION        (4)

/*******************************************************************************
 *  IPU1_0 EDMA allocation
 *******************************************************************************
 */
#ifdef NDK_PROC_TO_USE_IPU1_0
#define UTILS_IPU1_0_QDMACH_ALLOC_0     (0x0000000F) /* required for AVB */
#else
#define UTILS_IPU1_0_QDMACH_ALLOC_0     (0x00000000)
#endif

#define UTILS_IPU1_0_EDMACH_ALLOC_0     (0x0000FFFF)
#define UTILS_IPU1_0_EDMACH_ALLOC_1     (0x00000000)

#define UTILS_IPU1_0_PARAM_ALLOC_0      (0xFFFFFFFF)
#define UTILS_IPU1_0_PARAM_ALLOC_1      (0xFFFFFFFF)
#define UTILS_IPU1_0_PARAM_ALLOC_2      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_3      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_4      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_5      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_6      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_7      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_8      (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_9      (0x00000000)
#ifdef NDK_PROC_TO_USE_IPU1_0
#define UTILS_IPU1_0_PARAM_ALLOC_10     (0xFFFFFFFF) /* required for AVB */
#define UTILS_IPU1_0_PARAM_ALLOC_11     (0xFFFFFFFF) /* required for AVB */
#define UTILS_IPU1_0_PARAM_ALLOC_12     (0xFFFFFFFF) /* required for AVB */
#define UTILS_IPU1_0_PARAM_ALLOC_13     (0xFFFFFFFF) /* required for AVB */
#else
#define UTILS_IPU1_0_PARAM_ALLOC_10     (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_11     (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_12     (0x00000000)
#define UTILS_IPU1_0_PARAM_ALLOC_13     (0x00000000)
#endif

/*******************************************************************************
 *  IPU1_1 EDMA allocation
 *******************************************************************************
 */
#ifdef NDK_PROC_TO_USE_IPU1_1
#define UTILS_IPU1_1_QDMACH_ALLOC_0     (0x0000000F) /* required for AVB */
#else
#define UTILS_IPU1_1_QDMACH_ALLOC_0     (0x00000000)
#endif

#define UTILS_IPU1_1_EDMACH_ALLOC_0     (0x00FF0000)
#define UTILS_IPU1_1_EDMACH_ALLOC_1     (0x00000000)

#define UTILS_IPU1_1_PARAM_ALLOC_0      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_1      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_2      (0x0000FFFF)
#define UTILS_IPU1_1_PARAM_ALLOC_3      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_4      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_5      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_6      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_7      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_8      (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_9      (0x00000000)
#ifdef NDK_PROC_TO_USE_IPU1_1
#define UTILS_IPU1_1_PARAM_ALLOC_10     (0xFFFFFFFF) /* required for AVB */
#define UTILS_IPU1_1_PARAM_ALLOC_11     (0xFFFFFFFF) /* required for AVB */
#define UTILS_IPU1_1_PARAM_ALLOC_12     (0xFFFFFFFF) /* required for AVB */
#define UTILS_IPU1_1_PARAM_ALLOC_13     (0xFFFFFFFF) /* required for AVB */
#else
#define UTILS_IPU1_1_PARAM_ALLOC_10     (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_11     (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_12     (0x00000000)
#define UTILS_IPU1_1_PARAM_ALLOC_13     (0x00000000)
#endif

/*******************************************************************************
 *  DSP1 EDMA allocation
 *******************************************************************************
 */
#define UTILS_DSP1_QDMACH_ALLOC_0     (0x00000000)

#define UTILS_DSP1_EDMACH_ALLOC_0     (0x0F000000)
#define UTILS_DSP1_EDMACH_ALLOC_1     (0x00000000)

#define UTILS_DSP1_PARAM_ALLOC_0      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_1      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_2      (0xFFFF0000)
#define UTILS_DSP1_PARAM_ALLOC_3      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_4      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_5      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_6      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_7      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_8      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_9      (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_10     (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_11     (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_12     (0x00000000)
#define UTILS_DSP1_PARAM_ALLOC_13     (0x00000000)

/*******************************************************************************
 *  DSP2 EDMA allocation
 *******************************************************************************
 */
#define UTILS_DSP2_QDMACH_ALLOC_0     (0x00000000)

#define UTILS_DSP2_EDMACH_ALLOC_0     (0xF0000000)
#define UTILS_DSP2_EDMACH_ALLOC_1     (0x00000000)

#define UTILS_DSP2_PARAM_ALLOC_0      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_1      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_2      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_3      (0x0000FFFF)
#define UTILS_DSP2_PARAM_ALLOC_4      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_5      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_6      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_7      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_8      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_9      (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_10     (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_11     (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_12     (0x00000000)
#define UTILS_DSP2_PARAM_ALLOC_13     (0x00000000)


/*******************************************************************************
 *  A15_0 EDMA allocation
 *******************************************************************************
 */
#define UTILS_A15_0_QDMACH_ALLOC_0     (0x000000F0)

#define UTILS_A15_0_EDMACH_ALLOC_0     (0x00000000)
#define UTILS_A15_0_EDMACH_ALLOC_1     (0xFFFFFFFF)

#define UTILS_A15_0_PARAM_ALLOC_0      (0x00000000)
#define UTILS_A15_0_PARAM_ALLOC_1      (0x00000000)
#define UTILS_A15_0_PARAM_ALLOC_2      (0x00000000)
#define UTILS_A15_0_PARAM_ALLOC_3      (0xFFFF0000)
#define UTILS_A15_0_PARAM_ALLOC_4      (0xFFFFFFFF)
#define UTILS_A15_0_PARAM_ALLOC_5      (0xFFFFFFFF)
#define UTILS_A15_0_PARAM_ALLOC_6      (0xFFFFFFFF)
#define UTILS_A15_0_PARAM_ALLOC_7      (0xFFFFFFFF)
#define UTILS_A15_0_PARAM_ALLOC_8      (0xFFFFFFFF)
#define UTILS_A15_0_PARAM_ALLOC_9      (0xFFFFFFFF)
#define UTILS_A15_0_PARAM_ALLOC_10     (0x00000000)
#define UTILS_A15_0_PARAM_ALLOC_11     (0x00000000)
#define UTILS_A15_0_PARAM_ALLOC_12     (0x00000000)
#define UTILS_A15_0_PARAM_ALLOC_13     (0x00000000)


/**
 *******************************************************************************
 * XBAR interrupt source index numbers for EDMA interrupts
 *******************************************************************************
 */
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION0    (361u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION1    (362u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION2    (363u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION3    (364u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION4    (365u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION5    (366u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION6    (367u)
#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_REGION7    (368u)

#define UTILS_DMA_XBAR_SYS_EDMA_TPCC_IRQ_ERR        (359u)
#define UTILS_DMA_XBAR_SYS_EDMA_TC0_IRQ_ERR         (370u)
#define UTILS_DMA_XBAR_SYS_EDMA_TC1_IRQ_ERR         (371u)


/**
 *******************************************************************************
 * \brief System EDMA global information
 *******************************************************************************
 */
EDMA3_DRV_GblConfigParams gUtils_dmaSysEdmaGblCfgParams =
{
    /** Total number of DMA Channels supported by the EDMA3 Controller */
    64u,

    /** Total number of QDMA Channels supported by the EDMA3 Controller */
    8u,

    /** Total number of TCCs supported by the EDMA3 Controller */
    64u,

    /** Total number of PaRAM Sets supported by the EDMA3 Controller */
    512u,

    /** Total number of Event Queues in the EDMA3 Controller */
    4u,

    /** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
    2u,

    /** Number of Regions on this EDMA3 controller */
    8u,

    /**
     * Channel mapping existence.
     * A value of 0 (No channel mapping) implies that there is fixed association
     * for a channel number to a parameter entry number or, in other words,
     * PaRAM entry n corresponds to channel n.
     */
    1u,

    /** Existence of memory protection feature */
    1u,

    /** Global Register Region of CC Registers */
    (void *)UTILS_DMA_SYSTEM_DMA_BASE_ADDR,

    /** Transfer Controller (TC) Registers */
    {
        (void *)(UTILS_DMA_SYSTEM_DMA_BASE_ADDR+0x100000),
        (void *)(UTILS_DMA_SYSTEM_DMA_BASE_ADDR+0x200000),
        (void *)NULL,
        (void *)NULL,
        (void *)NULL,
        (void *)NULL,
        (void *)NULL,
        (void *)NULL
    },

    /** Interrupt no. for Transfer Completion */
    UTILS_DMA_SYS_EDMA_CCXFER_COMPLETION_INT,

    /** Interrupt no. for CC Error */
    UTILS_DMA_SYS_EDMA_CCERROR_INT,

    /** Interrupt no. for TCs Error */
    {
        UTILS_DMA_SYS_EDMA_TC0_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC1_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC2_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC3_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC4_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC5_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC6_ERROR_INT,
        UTILS_DMA_SYS_EDMA_TC7_ERROR_INT
    },

    /**
     * EDMA3 TC priority setting
     *
     * User can program the priority of the Event Queues
     * at a system-wide level.  This means that the user can set the
     * priority of an IO initiated by either of the TCs (Transfer Controllers)
     * relative to IO initiated by the other bus masters on the
     * device (ARM, DSP, USB, etc)
     */
    {
        4u,
        4u,
        0u,
        0u,
        0u,
        0u,
        0u,
        0u
    },

    /**
     * To Configure the Threshold level of number of events
     * that can be queued up in the Event queues. EDMA3CC error register
     * (CCERR) will indicate whether or not at any instant of time the
     * number of events queued up in any of the event queues exceeds
     * or equals the threshold/watermark value that is set
     * in the queue watermark threshold register (QWMTHRA).
     */
    {
        16u,
        16u,
        16u,
        16u,
        0u,
        0u,
        0u,
        0u
    },

    /**
     * To Configure the Default Burst Size (DBS) of TCs.
     * An optimally-sized command is defined by the transfer controller
     * default burst size (DBS). Different TCs can have different
     * DBS values. It is defined in Bytes.
     */
    {
        16u,
        16u,
        0u,
        0u,
        0u,
        0u,
        0u,
        0u
    },

    /**
     * Mapping from each DMA channel to a Parameter RAM set,
     * if it exists, otherwise of no use.
     */
    {
        0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
        8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
        16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
        24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
        32u, 33u, 34u, 35u, 36u, 37u, 38u, 39u,
        40u, 41u, 42u, 43u, 44u, 45u, 46u, 47u,
        48u, 49u, 50u, 51u, 52u, 53u, 54u, 55u,
        56u, 57u, 58u, 59u, 60u, 61u, 62u, 63u
    },

     /**
      * Mapping from each DMA channel to a TCC. This specific
      * TCC code will be returned when the transfer is completed
      * on the mapped channel.
      */
    {
        0u, 1u, 2u, 3u,
        4u, 5u, 6u, 7u,
        8u, 9u, 10u, 11u,
        12u, 13u, 14u, 15u,
        16u, 17u, 18u, 19u,
        20u, 21u, 22u, 23u,
        24u, 25u, 26u, 27u,
        28u, 29u, 30u, 31u,
        32u, 33u, 34u, 35u,
        36u, 37u, 38u, 39u,
        40u, 41u, 42u, 43u,
        44u, 45u, 46u, 47u,
        48u, 49u, 50u, 51u,
        52u, 53u, 54u, 55u,
        56u, 57u, 58u, 59u,
        60u, 61u, 62u, 63u
    },


    /**
     * Mapping of DMA channels to Hardware Events from
     * various peripherals, which use EDMA for data transfer.
     * All channels need not be mapped, some can be free also.
     */
    {
        0x00000000u,
        0x00000000u
    },
};

/**
 *******************************************************************************
 *
 * \brief Utility APIs to detect allocation conflicts given a channel allocation
 *        mask
 *
 * \param maskIpu1_0    [IN] Allocation mask
 * \param maskIpu1_1    [IN] Allocation mask
 * \param maskDsp1      [IN] Allocation mask
 * \param maskDsp2      [IN] Allocation mask
 * \param maskA15_0     [IN] Allocation mask
 *
 *******************************************************************************
 */
static inline void Utils_edma3CheckAllocationConlficts(
                        UInt32 maskIpu1_0,
                        UInt32 maskIpu1_1,
                        UInt32 maskDsp1,
                        UInt32 maskDsp2,
                        UInt32 maskA15_0
                        )
{
    UInt32 mask[16];
    UInt32 numMask = 0;

    mask[numMask++] = maskIpu1_0 & maskIpu1_1;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskIpu1_0 & maskDsp1;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskIpu1_0 & maskDsp2;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskIpu1_0 & maskA15_0;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskIpu1_1 & maskDsp1;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskIpu1_1 & maskDsp2;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskIpu1_1 & maskA15_0;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskDsp1   & maskDsp2;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskDsp1   & maskA15_0;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    mask[numMask++] = maskDsp2   & maskA15_0;
    UTILS_assert(numMask <= UTILS_ARRAYSIZE(mask));

    while(numMask--)
    {
        if(mask[numMask])
        {
            Vps_printf(
             " #### ERROR: UTILS: DMA: Conflict in EDMA static allocation !!!\n"
             " ####        Check static EDMA allocation and try again.\n"
             " ####        See file %s\n", __FILE__
            );
            UTILS_assert(0);
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief Utility APIs to detect EDMA CH/PaRAM allocation conflicts across
 *        processor's
 *
 *******************************************************************************
 */
void Utils_edma3CheckStaticAllocationConlficts()
{
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_QDMACH_ALLOC_0,
        UTILS_IPU1_1_QDMACH_ALLOC_0,
        UTILS_DSP1_QDMACH_ALLOC_0,
        UTILS_DSP2_QDMACH_ALLOC_0,
        UTILS_A15_0_QDMACH_ALLOC_0
        );

    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_QDMACH_ALLOC_0,
        UTILS_IPU1_1_QDMACH_ALLOC_0,
        UTILS_DSP1_QDMACH_ALLOC_0,
        UTILS_DSP2_QDMACH_ALLOC_0,
        UTILS_A15_0_QDMACH_ALLOC_0
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_EDMACH_ALLOC_1,
        UTILS_IPU1_1_EDMACH_ALLOC_1,
        UTILS_DSP1_EDMACH_ALLOC_1,
        UTILS_DSP2_EDMACH_ALLOC_1,
        UTILS_A15_0_EDMACH_ALLOC_1
        );

    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_0,
        UTILS_IPU1_1_PARAM_ALLOC_0,
        UTILS_DSP1_PARAM_ALLOC_0,
        UTILS_DSP2_PARAM_ALLOC_0,
        UTILS_A15_0_PARAM_ALLOC_0
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_1,
        UTILS_IPU1_1_PARAM_ALLOC_1,
        UTILS_DSP1_PARAM_ALLOC_1,
        UTILS_DSP2_PARAM_ALLOC_1,
        UTILS_A15_0_PARAM_ALLOC_1
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_2,
        UTILS_IPU1_1_PARAM_ALLOC_2,
        UTILS_DSP1_PARAM_ALLOC_2,
        UTILS_DSP2_PARAM_ALLOC_2,
        UTILS_A15_0_PARAM_ALLOC_2
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_3,
        UTILS_IPU1_1_PARAM_ALLOC_3,
        UTILS_DSP1_PARAM_ALLOC_3,
        UTILS_DSP2_PARAM_ALLOC_3,
        UTILS_A15_0_PARAM_ALLOC_3
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_4,
        UTILS_IPU1_1_PARAM_ALLOC_4,
        UTILS_DSP1_PARAM_ALLOC_4,
        UTILS_DSP2_PARAM_ALLOC_4,
        UTILS_A15_0_PARAM_ALLOC_4
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_5,
        UTILS_IPU1_1_PARAM_ALLOC_5,
        UTILS_DSP1_PARAM_ALLOC_5,
        UTILS_DSP2_PARAM_ALLOC_5,
        UTILS_A15_0_PARAM_ALLOC_5
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_6,
        UTILS_IPU1_1_PARAM_ALLOC_6,
        UTILS_DSP1_PARAM_ALLOC_6,
        UTILS_DSP2_PARAM_ALLOC_6,
        UTILS_A15_0_PARAM_ALLOC_6
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_7,
        UTILS_IPU1_1_PARAM_ALLOC_7,
        UTILS_DSP1_PARAM_ALLOC_7,
        UTILS_DSP2_PARAM_ALLOC_7,
        UTILS_A15_0_PARAM_ALLOC_7
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_8,
        UTILS_IPU1_1_PARAM_ALLOC_8,
        UTILS_DSP1_PARAM_ALLOC_8,
        UTILS_DSP2_PARAM_ALLOC_8,
        UTILS_A15_0_PARAM_ALLOC_8
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_9,
        UTILS_IPU1_1_PARAM_ALLOC_9,
        UTILS_DSP1_PARAM_ALLOC_9,
        UTILS_DSP2_PARAM_ALLOC_9,
        UTILS_A15_0_PARAM_ALLOC_9
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_10,
        UTILS_IPU1_1_PARAM_ALLOC_10,
        UTILS_DSP1_PARAM_ALLOC_10,
        UTILS_DSP2_PARAM_ALLOC_10,
        UTILS_A15_0_PARAM_ALLOC_10
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_11,
        UTILS_IPU1_1_PARAM_ALLOC_11,
        UTILS_DSP1_PARAM_ALLOC_11,
        UTILS_DSP2_PARAM_ALLOC_11,
        UTILS_A15_0_PARAM_ALLOC_11
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_12,
        UTILS_IPU1_1_PARAM_ALLOC_12,
        UTILS_DSP1_PARAM_ALLOC_12,
        UTILS_DSP2_PARAM_ALLOC_12,
        UTILS_A15_0_PARAM_ALLOC_12
        );
    Utils_edma3CheckAllocationConlficts(
        UTILS_IPU1_0_PARAM_ALLOC_13,
        UTILS_IPU1_1_PARAM_ALLOC_13,
        UTILS_DSP1_PARAM_ALLOC_13,
        UTILS_DSP2_PARAM_ALLOC_13,
        UTILS_A15_0_PARAM_ALLOC_13
        );
}



