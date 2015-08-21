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
 * \file utils_stat_collector_cfg.c
 *
 * \brief  This file has the implementataion of statCollector task.
 *
 *
 * \version 0.0 (Jan 2014) : [FM] First version
 *
 *******************************************************************************
*/



static Utils_StatCollSdramUsecase sdram_cfg_array_0[] =
{
    {
        "SCI_EMIF1 RD+WR",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_EMIF1,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_RD_OR_WR_DONTCARE,
            SCI_ERR_DONTCARE,

        }
    },
    {
        "SCI_EMIF2 RD+WR",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_EMIF2,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_RD_OR_WR_DONTCARE,
            SCI_ERR_DONTCARE,
        }
    },
    {
        "SCI_EMIF1 RD ONLY",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_EMIF1,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_RD_ONLY,
            SCI_ERR_DONTCARE,

        }
    },
    {
        "SCI_EMIF1 WR ONLY",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_EMIF1,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_WR_ONLY,
            SCI_ERR_DONTCARE,

        }
    },
    {
        "SCI_EMIF2 RD ONLY",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_EMIF2,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_RD_ONLY,
            SCI_ERR_DONTCARE,
        }
    },
    {
        "SCI_EMIF2 WR ONLY",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_EMIF2,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_WR_ONLY,
            SCI_ERR_DONTCARE,
        }
    },
    {
        "SCI_MA_MPU_P1",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_MA_MPU_P1,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_RD_OR_WR_DONTCARE,
            SCI_ERR_DONTCARE,
        }
    },
    {
        "SCI_MA_MPU_P2",
        {
            SCI_SDRAM_THROUGHPUT,
            SCI_MA_MPU_P2,
            1,
            SCI_MASTID_ALL,
            0xff,
            SCI_RD_OR_WR_DONTCARE,
            SCI_ERR_DONTCARE,
        }
    },

};

static Utils_StatCollMstrUsecase mstr_cfg_array_0[] =
{
    /* set 0 */
    {
        "SCI_DSS",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_DSS,
            0
        }
    },

    {
        "SCI_IPU1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_IPU1,
            0
        }
    },

    {
        "SCI_VIP1_P1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VIP1_P1,
            0
        }
    },
    {
        "SCI_VIP1_P2",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VIP1_P2,
            0
        }
    },

    {
        "SCI_VPE_P1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VPE_P1,
            0
        }
    },
    {
        "SCI_VPE_P2",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VPE_P1,
            0
        }
    },

    {
        "SCI_DSP1_MDMA",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_DSP1_MDMA,
            0
        }
    },
    {
        "SCI_DSP1_EDMA",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_DSP1_EDMA,
            0
        }
    },

    {
        "SCI_DSP2_MDMA",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_DSP2_MDMA,
            0
        }
    },
    {
        "SCI_DSP2_EDMA",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_DSP2_EDMA,
            0
        }
    },

    {
        "SCI_EVE1_TC0",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE1_TC0,
            0
        }
    },

    {
        "SCI_EVE1_TC1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE1_TC1,
            0
        }
    },

    {
        "SCI_EVE2_TC0",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE2_TC0,
            0
        }
    },

    {
        "SCI_EVE2_TC1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE2_TC1,
            0
        }
    },

    {
        "SCI_EDMA_TC0_RD",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EDMA_TC0_RD,
            0
        }
    },
    {
        "SCI_EDMA_TC0_WR",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EDMA_TC0_WR,
            0
        }
    },

    {
        "SCI_EDMA_TC1_RD",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EDMA_TC1_RD,
            0
        }
    },
    {
        "SCI_EDMA_TC1_WR",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EDMA_TC1_WR,
            0
        }
    },
};

static Utils_StatCollMstrUsecase mstr_cfg_array_1[] =
{
    /* set 1 */
    {
        "SCI_VIP2_P1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VIP2_P1,
            0
        }
    },
    {
        "SCI_VIP2_P2",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VIP2_P2,
            0
        }
    },
    {
        "SCI_VIP3_P1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VIP3_P1,
            0
        }
    },
    {
        "SCI_VIP3_P2",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_VIP3_P2,
            0
        }
    },
    {
        "SCI_EVE3_TC0",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE3_TC0,
            0
        }
    },

    {
        "SCI_EVE3_TC1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE3_TC1,
            0
        }
    },

    {
        "SCI_EVE4_TC0",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE4_TC0,
            0
        }
    },

    {
        "SCI_EVE4_TC1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_EVE4_TC1,
            0
        }
    },

    {
        "SCI_IVA",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_IVA,
            0
        }
    },
    {
        "SCI_GPU_P1",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_GPU_P1,
            0
        }
    },
    {
        "SCI_GPU_P2",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_GPU_P2,
            0
        }
    },
    {
        "SCI_GMAC_SW",
        {
            SCI_MSTR_THROUGHPUT,
            SCI_GMAC_SW,
            0
        }
    },
};

