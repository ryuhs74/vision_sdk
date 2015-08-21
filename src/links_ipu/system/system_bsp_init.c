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
 * \file system_bsp_init.c
 *
 * \brief  APIs for initializing BIOS Drivers.
 *
 *         BIOS Support Package inits, which includes BIOS video drivers inits
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *                           Include Files                                     *
 ******************************************************************************/

#include "system_bsp_init.h"

/**
 *******************************************************************************
 * \brief Align BSS memory with that of BIOS heap library requirement
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Link Stack
 *
 *        Align descriptor memory with that of VPDMA requirement.
 *        Place the descriptor in non-cached heap section.
 *
 *******************************************************************************
 */

 /*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Initialize the required modules of BIOS video drivers
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 System_bspInit()
{
    Int32 nStatus = SYSTEM_LINK_STATUS_SOK;
    Bsp_CommonInitParams commonInitPrms; /* Initialized using BSP APIs */
    Bsp_PlatformInitParams platInitPrms; /* Initialized using BSP APIs */
    Vps_InitParams vpsInitPrms; /* Initialized using BSP APIs */

    do
    {
        BspCommonInitParams_init(&commonInitPrms);

        Vps_printf(" SYSTEM: BSP Common Init in progress !!!\n");

        nStatus = Bsp_commonInit(&commonInitPrms);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: BSP Common Init Failed !!!\n");
            break;
        }
        Vps_printf(" SYSTEM: BSP Common Init Done !!!\n");

        BspPlatformInitParams_init(&platInitPrms);

        Vps_printf(" SYSTEM: BSP Platform Init in progress !!!\n");

        platInitPrms.isPinMuxSettingReq = TRUE;
        nStatus = Bsp_platformInit(&platInitPrms);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: BSP Platform Init Failed !!!\n");
            break;
        }

        Vps_printf(" SYSTEM: BSP Platform Init Done !!!\n");

        Vps_printf(" SYSTEM: FVID2 Init in progress !!!\n");

        nStatus = Fvid2_init(NULL);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: FVID2 Init Failed !!!\n");
            break;
        }

        Vps_printf(" SYSTEM: FVID2 Init Done !!!\n");

        Vps_printf(" SYSTEM: VPS Init in progress !!!\n");

        VpsInitParams_init(&vpsInitPrms);
#ifdef TDA2XX_FAMILY_BUILD
        vpsInitPrms.virtBaseAddr = 0x80000000;
        vpsInitPrms.physBaseAddr = 0x80000000;
#else
        vpsInitPrms.virtBaseAddr = 0xA0000000;
        vpsInitPrms.physBaseAddr = 0x80000000;
#endif
        if(vpsInitPrms.virtBaseAddr!=vpsInitPrms.physBaseAddr)
        {
            Vps_printf(" SYSTEM: VPDMA Descriptor Memory Address translation"
                       " ENABLED [0x%08x -> 0x%08x]\n",
                    vpsInitPrms.virtBaseAddr,
                    vpsInitPrms.physBaseAddr
                );
            /* if Virtual address != Physical address then enable translation
             * In TDA3xx this is required
             */
            vpsInitPrms.isAddrTransReq = TRUE;
        }
        nStatus = Vps_init(&vpsInitPrms);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: VPS Init Failed !!!\n");
            break;
        }

        Vps_printf(" SYSTEM: VPS Init Done !!!\n");
    } while (0);

    return nStatus;
}

/**
 *******************************************************************************
 *
 * \brief De-initialize the previously initialized modules
 *  of BIOS video drivers
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 System_bspDeInit()
{
    Int32 nStatus = SYSTEM_LINK_STATUS_SOK;

    do
    {
        nStatus = Vps_deInit(NULL);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: VPS De-Init Failed !!!\n");
            break;
        }

        nStatus = Fvid2_deInit(NULL);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: FVID2 De-Init Failed !!!\n");
            break;
        }

        nStatus = Bsp_platformDeInit(NULL);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: BSP Platform De-Init Failed !!!\n");
            break;
        }

        nStatus = Bsp_commonDeInit(NULL);
        if (SYSTEM_LINK_STATUS_SOK != nStatus)
        {
            Vps_printf(" SYSTEM: BSP Common De-Init Failed !!!\n");
            break;
        }
    } while (0);

    return nStatus;
}
