/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <examples/tda2xx/include/chains.h>
#include <examples/tda2xx/include/chains_common.h>


#define CHAINS_COMMON_ISS_MESH_TABLE_00_HEIGHT         (1080)
#define CHAINS_COMMON_ISS_MESH_TABLE_00_WIDTH          (1920)
#define CHAINS_COMMON_ISS_MESH_TABLE_PITCH(w, r)       ((((((w)/(r))+1) + 15U) & ~15U) * (4U))

#define CHAINS_COMMON_ISS_MESH_TABLE_ELEMENTSIZE       (4)

/* Table start address must be atleast 16 byte aligned, keeping it 32 here
 */
#pragma DATA_ALIGN(gChainsCommonIss_ldcTable00, 32)

UInt8 gChainsCommonIss_ldcTable00[] =
{
    #include "chains_common_iss_ldc_table_00.h"
};

#pragma DATA_ALIGN(gChainsCommonIss_ldcTable01, 32)
UInt8 gChainsCommonIss_ldcTable01[] =
{
    #include "chains_common_iss_ldc_table_02.h"
};
Void ChainsCommon_SetIssLdcLutConfig(vpsissldcLutCfg_t *pLdcLutCfg, UInt32 tableIdx)
{
    if(tableIdx==0)
    {
        /*
         * This is NOT a tuned table, its a sample table for reference only
         *
         * this table is for 1920x1080 frame.
         * Point start address of table to center cropped 1280x720 section
         *
         */
        pLdcLutCfg->address          = (UInt32)&gChainsCommonIss_ldcTable00
            [
                    CHAINS_COMMON_ISS_MESH_TABLE_PITCH(CHAINS_COMMON_ISS_MESH_TABLE_00_WIDTH, (1 <<VPS_ISS_LDC_LUT_DOWN_SCALE_FACTOR_4))
                    *
                    (
                        (CHAINS_COMMON_ISS_MESH_TABLE_00_HEIGHT-720)/2/4
                    )
                    +
                    (
                        ((CHAINS_COMMON_ISS_MESH_TABLE_00_WIDTH-1280)*CHAINS_COMMON_ISS_MESH_TABLE_ELEMENTSIZE)/2/4
                    )
            ];
        pLdcLutCfg->lineOffset       = CHAINS_COMMON_ISS_MESH_TABLE_PITCH(CHAINS_COMMON_ISS_MESH_TABLE_00_WIDTH, (1U << VPS_ISS_LDC_LUT_DOWN_SCALE_FACTOR_4));
        pLdcLutCfg->downScaleFactor  = VPS_ISS_LDC_LUT_DOWN_SCALE_FACTOR_4;
    }
    else if (tableIdx==1)
    {
        pLdcLutCfg->address          = (UInt32)&gChainsCommonIss_ldcTable01;
        pLdcLutCfg->lineOffset       = CHAINS_COMMON_ISS_MESH_TABLE_PITCH(1280, (1U << VPS_ISS_LDC_LUT_DOWN_SCALE_FACTOR_16));
        pLdcLutCfg->downScaleFactor  = VPS_ISS_LDC_LUT_DOWN_SCALE_FACTOR_16;
    }
    else
    {
        UTILS_assert(0);
    }
}
