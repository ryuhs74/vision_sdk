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
 * \file utils_dma_cfg_dsp_local_edma.c
 *
 * \brief This file has the common configuration for Local DSP EDMA controller
 *
 * \version 0.0 (Sept 2013) : [KC] First version
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
 *   - utils_dma_cfg_dsp1.c
 *   - utils_dma_cfg_dsp2.c
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  EDMA controller region allocation based on CPU ID
 *
 *  Since only a given DSP will use the EDMA controller we assign region 0
 *  to it
 *
 *******************************************************************************
 */
#define UTILS_DMA_DSP_LOCAL_DMA_REGION       (0)


/*******************************************************************************
 *  \brief base address of EDMA controller
 *******************************************************************************
 */
#define UTILS_DMA_DSP_LOCAL_DMA_BASE_ADDR   (0x01D10000)

/**
 *******************************************************************************
 * \brief System EDMA global information
 *******************************************************************************
 */
EDMA3_DRV_GblConfigParams gUtils_dmaDspLocalEdmaGblCfgParams =
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
    (void *)UTILS_DMA_DSP_LOCAL_DMA_BASE_ADDR,

    /** Transfer Controller (TC) Registers */
    {
        (void *)(UTILS_DMA_DSP_LOCAL_DMA_BASE_ADDR+0x100000),
        (void *)(UTILS_DMA_DSP_LOCAL_DMA_BASE_ADDR+0x200000),
        (void *)NULL,
        (void *)NULL,
        (void *)NULL,
        (void *)NULL,
        (void *)NULL,
        (void *)NULL
    },

    /** Interrupt no. for Transfer Completion */
    0,  /* NOT used */

    /** Interrupt no. for CC Error */
    0,  /* NOT used */

    /** Interrupt no. for TCs Error */
    {
        0,  /* NOT used */
        0,  /* NOT used */
        0,  /* NOT used */
        0,  /* NOT used */
        0,  /* NOT used */
        0,  /* NOT used */
        0,  /* NOT used */
        0,  /* NOT used */
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
 * \brief Local EDMA resource allocation for this DSP
 *
 *        All resources allocated to this DSP
 *
 *******************************************************************************
 */
EDMA3_DRV_InstanceInitConfig gUtils_dmaDspLocalEdmaInstInitConfig =
{
    /* 31     0                     63    32                    95    64                    127   96 */
    {0xFFFFFFFF,                    0xFFFFFFFF,                 0xFFFFFFFF,                 0xFFFFFFFF,
    /* 159  128                     191  160                    223  192                    255  224 */
     0xFFFFFFFF,                    0xFFFFFFFF,                 0xFFFFFFFF,                 0xFFFFFFFF,
    /* 287  256                     319  288                    351  320                    383  352 */
     0xFFFFFFFF,                    0xFFFFFFFF,                 0xFFFFFFFF,                 0xFFFFFFFF,
    /* 415  384                     447  416                    479  448                    511  480 */
     0xFFFFFFFF,                    0xFFFFFFFF,                 0xFFFFFFFF,                 0xFFFFFFFF,
    },

    /* ownDmaChannels */
    /* 31     0                     63    32 */
    {0xFFFFFFFF,                    0xFFFFFFFF,                 },

    /* ownQdmaChannels */
    /* 31     0 */
    {0x000000FF         },

    /* ownTccs */
    /* 31     0                     63    32 */
    {0xFFFFFFFF,                    0xFFFFFFFF,                 },


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
