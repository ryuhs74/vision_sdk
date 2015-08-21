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
 *   \file  ndk_nsp_hooks.c
 *
 *   \brief Do all necessary board level initialization for NDK.
 *
 *******************************************************************************
 */


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

/* Standard language headers */
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

/* OS/Posix headers */

/* NDK Dependencies */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/tools/servers.h>
#include <ti/ndk/inc/tools/console.h>

/* NSP Dependencies */
#include <ti/nsp/drv/inc/gmacsw_config.h>

/* Project dependency headers */
#include <src/utils_common/include/utils.h>
#include <include/link_api/systemLink_common.h>
#include <include/link_api/networkCtrl_api.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#define NET_IF_IDX  (1)

#define PAB_MII (0)
#define PAB_RMII (0)


/* Ethernet MAC ID registers(Devcice configuration) from EFuse */
#define MAC_ID0_LO              (*(volatile uint32_t*)0x4A002514)
#define MAC_ID0_HI              (*(volatile uint32_t*)0x4A002518)
#define MAC_ID1_LO              (*(volatile uint32_t*)0x4A00251C)
#define MAC_ID1_HI              (*(volatile uint32_t*)0x4A002520)

/* I/O Delay related registers */
#define CFG_IO_DELAY_UNLOCK_KEY     (0x0000AAAA)
#define CFG_IO_DELAY_LOCK_KEY       (0x0000AAAB)

#define CFG_IO_DELAY_ACCESS_PATTERN (0x00029000)
#define CFG_IO_DELAY_LOCK_MASK      (0x400)

#define CFG_IO_DELAY_BASE           (0x4844A000)
#define CFG_IO_DELAY_LOCK           (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0x02C))
#define CFG_RGMII0_TXCTL_OUT        (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0x74C))
#define CFG_RGMII0_TXD0_OUT         (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0x758))
#define CFG_RGMII0_TXD1_OUT         (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0x764))
#define CFG_RGMII0_TXD2_OUT         (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0x770))
#define CFG_RGMII0_TXD3_OUT         (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0x77C))
#define CFG_VIN2A_D13_OUT           (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0xA7C))
#define CFG_VIN2A_D17_OUT           (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0xAAC))
#define CFG_VIN2A_D16_OUT           (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0xAA0))
#define CFG_VIN2A_D15_OUT           (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0xA94))
#define CFG_VIN2A_D14_OUT           (*(volatile uint32_t*)(CFG_IO_DELAY_BASE + 0xA88))

/* PAD Configuration Registers */
#define SYSCFG_PAD_RGMII0_TXCTL     (*(volatile uint32_t*)(0x4A003654))
#define SYSCFG_PAD_RGMII0_TXD3      (*(volatile uint32_t*)(0x4A003658))
#define SYSCFG_PAD_RGMII0_TXD2      (*(volatile uint32_t*)(0x4A00365C))
#define SYSCFG_PAD_RGMII0_TXD1      (*(volatile uint32_t*)(0x4A003660))
#define SYSCFG_PAD_RGMII0_TXD0      (*(volatile uint32_t*)(0x4A003664))
#define SYSCFG_PAD_VIN2A_D13        (*(volatile uint32_t*)(0x4A00359C))
#define SYSCFG_PAD_VIN2A_D14        (*(volatile uint32_t*)(0x4A0035A0))
#define SYSCFG_PAD_VIN2A_D15        (*(volatile uint32_t*)(0x4A0035A4))
#define SYSCFG_PAD_VIN2A_D16        (*(volatile uint32_t*)(0x4A0035A8))
#define SYSCFG_PAD_VIN2A_D17        (*(volatile uint32_t*)(0x4A0035AC))


/*******************************************************************************
 *  Function's
 *******************************************************************************
 */
static void LOCAL_linkStatus( uint32_t phy, uint32_t linkStatus );


/*******************************************************************************
 *  Global's
 *******************************************************************************
 */

/* This string array corresponds to link state */
static char *LinkStr[] = { "No Link",
                           "None",
                           "10Mb/s Half Duplex",
                           "10Mb/s Full Duplex",
                           "100Mb/s Half Duplex",
                           "100Mb/s Full Duplex",
                           "1000Mb/s Half Duplex", /*not suported*/
                           "1000Mb/s Full Duplex"};


/**
 *******************************************************************************
 *
 * \brief HW specific initialization
 *
 *        We changed our CFG file to point call this private init
 *        function. Here we initialize our board and read in our
 *        MAC address.
 *
 *******************************************************************************
 */
void NDK_NSP_Init( void )
{
#if (defined(TDA2XX_FAMILY_BUILD) && (defined(BOARD_TYPE_TDA2XX_EVM) || defined(BOARD_TYPE_TDA2EX_EVM)))
    #if ((PAB_MII != 1) && (PAB_RMII != 1))
    uint32_t regValue, delta, coarse, fine;

    /*
     * Adjust I/O delays on the Tx control and data lines of each MAC port. This is
     * a workaround in order to work properly with the DP83865 PHYs on the EVM. In 3COM
     * RGMII mode this PHY applies it's own internal clock delay, so we essentially need to
     * counteract the DRA7xx internal delay, and we do this by delaying the control and
     * data lines. If not using this PHY, you probably don't need to do this stuff!
     */

    /* Global unlock for I/O Delay registers */
    CFG_IO_DELAY_LOCK = CFG_IO_DELAY_UNLOCK_KEY;

    /* Tweaks to RGMII0 Tx Control and Data */
    CFG_RGMII0_TXCTL_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_RGMII0_TXCTL = (SYSCFG_PAD_RGMII0_TXCTL & ~0xF) | 0x0;
    delta       = (0x3 << 5) + 0x8;     /* Delay value to add to calibrated value */
    regValue    = CFG_RGMII0_TXCTL_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_RGMII0_TXCTL_OUT = regValue;

    CFG_RGMII0_TXD0_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_RGMII0_TXD0 = (SYSCFG_PAD_RGMII0_TXD0 & ~0xF) | 0x0;
    delta       = (0x3 << 5) + 0x8;     /* Delay value to add to calibrated value */
    regValue    = CFG_RGMII0_TXD0_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_RGMII0_TXD0_OUT = regValue;

    CFG_RGMII0_TXD1_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_RGMII0_TXD1 = (SYSCFG_PAD_RGMII0_TXD1 & ~0xF) | 0x0;
    delta       = (0x3 << 5) + 0x2;     /* Delay value to add to calibrated value */
    regValue    = CFG_RGMII0_TXD1_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_RGMII0_TXD1_OUT = regValue;

    CFG_RGMII0_TXD2_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_RGMII0_TXD2 = (SYSCFG_PAD_RGMII0_TXD2 & ~0xF) | 0x0;
    delta       = (0x4 << 5) + 0x0;     /* Delay value to add to calibrated value */
    regValue    = CFG_RGMII0_TXD2_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_RGMII0_TXD2_OUT = regValue;

    CFG_RGMII0_TXD3_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_RGMII0_TXD3 = (SYSCFG_PAD_RGMII0_TXD3 & ~0xF) | 0x0;
    delta       = (0x4 << 5) + 0x0;     /* Delay value to add to calibrated value */
    regValue    = CFG_RGMII0_TXD3_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_RGMII0_TXD3_OUT = regValue;

    /* Tweaks to RGMII1 Tx Control and Data */
    CFG_VIN2A_D13_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_VIN2A_D13 = (SYSCFG_PAD_VIN2A_D13 & ~0xF) | 0x3;
    delta       = (0x3 << 5) + 0x8;     /* Delay value to add to calibrated value */
    regValue    = CFG_VIN2A_D13_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_VIN2A_D13_OUT = regValue;

    CFG_VIN2A_D17_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_VIN2A_D17 = (SYSCFG_PAD_VIN2A_D17 & ~0xF) | 0x3;
    delta       = (0x3 << 5) + 0x8;
    regValue    = CFG_VIN2A_D17_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_VIN2A_D17_OUT = regValue;

    CFG_VIN2A_D16_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_VIN2A_D16 = (SYSCFG_PAD_VIN2A_D16 & ~0xF) | 0x3;
    delta       = (0x3 << 5) + 0x2;
    regValue    = CFG_VIN2A_D16_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_VIN2A_D16_OUT = regValue;

    CFG_VIN2A_D15_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_VIN2A_D15 = (SYSCFG_PAD_VIN2A_D15 & ~0xF) | 0x3;
    delta       = (0x4 << 5) + 0x0;
    regValue    = CFG_VIN2A_D15_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_VIN2A_D15_OUT = regValue;

    CFG_VIN2A_D14_OUT = (CFG_IO_DELAY_ACCESS_PATTERN & ~CFG_IO_DELAY_LOCK_MASK);
    SYSCFG_PAD_VIN2A_D14 = (SYSCFG_PAD_VIN2A_D14 & ~0xF) | 0x3;
    delta       = (0x4 << 5) + 0x0;
    regValue    = CFG_VIN2A_D14_OUT & ~0xFFFFFC00;
    coarse      = ((regValue >> 5) & 0x1F) + ((delta >> 5) & 0x1F);
    coarse      = (coarse > 0x1F) ? (0x1F) : (coarse);
    fine        = (regValue & 0x1F) + (delta & 0x1F);
    fine        = (fine > 0x1F) ? (0x1F) : (fine);
    regValue    = CFG_IO_DELAY_ACCESS_PATTERN | CFG_IO_DELAY_LOCK_MASK | ((coarse << 5) | (fine));
    CFG_VIN2A_D14_OUT = regValue;

    /* Global lock */
    CFG_IO_DELAY_LOCK = CFG_IO_DELAY_LOCK_KEY;
    #endif
#endif
}

/**
 *******************************************************************************
 *
 * \brief Callback to get GMAC HW config
 *
 *        This is a callback from the Ethernet driver. This function
 *        is used by the driver to an application-specific config structure
 *        for the GMACSW driver. Typically it will be used to provide the
 *        MAC address(es) and the link status update callback function.
 *
 *******************************************************************************
 */
GMACSW_Config *GMACSW_getConfig(void)
{
    int i = 0;
    uint8_t macAddr[6];

    /* Get digital loopback starting config */
    GMACSW_Config *pGMACSWConfig = NULL;

    #ifdef BUILD_M4_0
        #ifdef NDK_PROC_TO_USE_IPU1_0
        pGMACSWConfig = GMACSW_CONFIG_getDefaultConfig();
        #endif
    #endif
    #ifdef BUILD_M4_1
        #ifdef NDK_PROC_TO_USE_IPU1_1
        pGMACSWConfig = GMACSW_CONFIG_getDefaultConfig();
        #endif
    #endif
    #ifdef BUILD_A15
        #ifdef NDK_PROC_TO_USE_A15_0
        pGMACSWConfig = GMACSW_CONFIG_getDefaultConfig();
        #endif
    #endif

    if(pGMACSWConfig == NULL)
        return pGMACSWConfig;

    /* Update default config with the correct MAC addresses */
    for(i=0; i<(pGMACSWConfig->activeMACPortCount); i++)
    {
        if (0==i)
        {
            /* Get the MAC Address from control module register space */
            macAddr[5] = (uint8_t)((MAC_ID0_LO & 0x000000FFu) >> 0u );
            macAddr[4] = (uint8_t)((MAC_ID0_LO & 0x0000FF00u) >> 8u );
            macAddr[3] = (uint8_t)((MAC_ID0_LO & 0x00FF0000u) >> 16u);

            macAddr[2] = (uint8_t)((MAC_ID0_HI & 0x000000FFu) >> 0u );
            macAddr[1] = (uint8_t)((MAC_ID0_HI & 0x0000FF00u) >> 8u );
            macAddr[0] = (uint8_t)((MAC_ID0_HI & 0x00FF0000u) >> 16u);
        }
        else
        {
            /* Get the MAC Address from control module register space */
            macAddr[5] = (uint8_t)((MAC_ID1_LO & 0x000000FFu) >> 0u );
            macAddr[4] = (uint8_t)((MAC_ID1_LO & 0x0000FF00u) >> 8u );
            macAddr[3] = (uint8_t)((MAC_ID1_LO & 0x00FF0000u) >> 16u);

            macAddr[2] = (uint8_t)((MAC_ID1_HI & 0x000000FFu) >> 0u );
            macAddr[1] = (uint8_t)((MAC_ID1_HI & 0x0000FF00u) >> 8u );
            macAddr[0] = (uint8_t)((MAC_ID1_HI & 0x00FF0000u) >> 16u);
        }

        printf("\nMAC Port %d Address:\n\t%02x-%02x-%02x-%02x-%02x-%02x\n", i,
                macAddr[0], macAddr[1], macAddr[2],
                macAddr[3], macAddr[4], macAddr[5]);

        /* Copy the correct MAC address into the driver config */
        memcpy( (void *)&(pGMACSWConfig->macInitCfg[i].macAddr[0]), (void *)&macAddr[0], 6 );

#if (defined(TDA2XX_FAMILY_BUILD) && (defined(BOARD_TYPE_TDA2XX_EVM) || defined(BOARD_TYPE_TDA2EX_EVM)))
    #if ((PAB_MII == 1) || (PAB_RMII == 1))
        /*
         * Adjust the PHY mask numbers for the Vayu PAB. The first MAC
         * port is connected to a PHY with address = 3, the second MAC
         * port is connected to a PHY with address = 2.
         */
        pGMACSWConfig->macInitCfg[i].phyMask = 0x1 << (3 - i);
    #else
        /*
         * Adjust the PHY mask numbers for the Vayu EVM. The first MAC
         * port is connected to a PHY with address = 2, the second MAC
         * port is connected to a PHY with address = 3.
         */
#ifndef TDA2EX_BUILD
        pGMACSWConfig->macInitCfg[i].phyMask = 0x1 << (2 + i);
#else
	pGMACSWConfig->macInitCfg[i].phyMask = 0x8 >> (i*2);
#endif
		// For older combination where port2 is connected t0 phy 8 
    #endif
#endif

#if defined(TDA3XX_FAMILY_BUILD) || defined(BOARD_TYPE_TDA2XX_MC)
    /*
     * Adjust the PHY mask numbers for the Vayu EVM. The first MAC
     * port is connected to a PHY with address = 2, the second MAC
     * port is connected to a PHY with address = 3.
     */
    pGMACSWConfig->macInitCfg[i].phyMask = 0x1 << i;

    pGMACSWConfig->macInitCfg[i].macConnectionType =
            MAC_CONNECTION_TYPE_RGMII_DETECT_INBAND;
#endif

#if (defined(TDA2XX_FAMILY_BUILD) && (defined(BOARD_TYPE_TDA2XX_EVM) || defined(BOARD_TYPE_TDA2EX_EVM)))
    #if (PAB_MII == 1)
        pGMACSWConfig->macInitCfg[i].macConnectionType =
            MAC_CONNECTION_TYPE_MII_100;
    #elif (PAB_RMII == 1)
        pGMACSWConfig->macInitCfg[i].macConnectionType =
            MAC_CONNECTION_TYPE_RMII_100;
    #else
        /*
         * National PHY on Vayu EVM does not work with the default INBAND detection mode.
         * It would seem the Rx clock from the PHY is not generated unless the Tx clock
         * from the Vayu device is present. So set the mode to force 1Gbps to start.
         */
        pGMACSWConfig->macInitCfg[i].macConnectionType =
            MAC_CONNECTION_TYPE_RGMII_FORCE_1000_FULL;
    #endif
#endif
    }

    pGMACSWConfig->linkStatusCallback = &LOCAL_linkStatus;

    /* Return the config */
    return pGMACSWConfig;
}




/**
 *******************************************************************************
 * \brief String to displayed on telnet terminal
 *******************************************************************************
 */
char *VerStr = "\n\n **** Vision SDK **** \n\n";

#ifdef BUILD_M4_0
    #ifdef NDK_PROC_TO_USE_IPU1_0
    static HANDLE hEcho = 0;
    static HANDLE hEchoUdp = 0;
    static HANDLE hData = 0;
    static HANDLE hNull = 0;
    static HANDLE hOob = 0;
    #endif
#endif
#ifdef BUILD_M4_1
    #ifdef NDK_PROC_TO_USE_IPU1_1
    static HANDLE hEcho = 0;
    static HANDLE hEchoUdp = 0;
    static HANDLE hData = 0;
    static HANDLE hNull = 0;
    static HANDLE hOob = 0;
    #endif
#endif
#ifdef BUILD_A15
    #ifdef NDK_PROC_TO_USE_A15_0
    static HANDLE hEcho = 0;
    static HANDLE hEchoUdp = 0;
    static HANDLE hData = 0;
    static HANDLE hNull = 0;
    static HANDLE hOob = 0;
    #endif
#endif


/**
 *******************************************************************************
 *
 * \brief NDK callback to start DEAMON services
 *
 *******************************************************************************
 */
void netOpenHook()
{
#ifdef BUILD_M4_0
    #ifdef NDK_PROC_TO_USE_IPU1_0
    // Create our local servers
    hEcho = DaemonNew( SOCK_STREAMNC, 0, 7, dtask_tcp_echo,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hEchoUdp = DaemonNew( SOCK_DGRAM, 0, 7, dtask_udp_echo,
                          OS_TASKPRINORM, OS_TASKSTKNORM, 0, 1 );
    hData = DaemonNew( SOCK_STREAM, 0, 1000, dtask_tcp_datasrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hNull = DaemonNew( SOCK_STREAMNC, 0, 1001, dtask_tcp_nullsrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hOob  = DaemonNew( SOCK_STREAMNC, 0, 999, dtask_tcp_oobsrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );

    NetworkCtrl_init();
    #endif
#endif
#ifdef BUILD_M4_1
    #ifdef NDK_PROC_TO_USE_IPU1_1
    // Create our local servers
    hEcho = DaemonNew( SOCK_STREAMNC, 0, 7, dtask_tcp_echo,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hEchoUdp = DaemonNew( SOCK_DGRAM, 0, 7, dtask_udp_echo,
                          OS_TASKPRINORM, OS_TASKSTKNORM, 0, 1 );
    hData = DaemonNew( SOCK_STREAM, 0, 1000, dtask_tcp_datasrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hNull = DaemonNew( SOCK_STREAMNC, 0, 1001, dtask_tcp_nullsrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hOob  = DaemonNew( SOCK_STREAMNC, 0, 999, dtask_tcp_oobsrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );

    NetworkCtrl_init();
    #endif
#endif
#ifdef BUILD_A15
    #ifdef NDK_PROC_TO_USE_A15_0
    // Create our local servers
    hEcho = DaemonNew( SOCK_STREAMNC, 0, 7, dtask_tcp_echo,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hEchoUdp = DaemonNew( SOCK_DGRAM, 0, 7, dtask_udp_echo,
                          OS_TASKPRINORM, OS_TASKSTKNORM, 0, 1 );
    hData = DaemonNew( SOCK_STREAM, 0, 1000, dtask_tcp_datasrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hNull = DaemonNew( SOCK_STREAMNC, 0, 1001, dtask_tcp_nullsrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );
    hOob  = DaemonNew( SOCK_STREAMNC, 0, 999, dtask_tcp_oobsrv,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3 );

    NetworkCtrl_init();
    #endif
#endif

}

/**
 *******************************************************************************
 *
 * \brief NDK callback to stop DEAMON services
 *
 *******************************************************************************
 */
void netCloseHook()
{
#ifdef BUILD_M4_0
    #ifdef NDK_PROC_TO_USE_IPU1_0
    DaemonFree(hOob);
    DaemonFree(hNull);
    DaemonFree(hData);
    DaemonFree(hEchoUdp);
    DaemonFree(hEcho);

    // Kill any active console
    ConsoleClose();
    NetworkCtrl_deInit();
    #endif
#endif
#ifdef BUILD_M4_1
    #ifdef NDK_PROC_TO_USE_IPU1_1
    DaemonFree(hOob);
    DaemonFree(hNull);
    DaemonFree(hData);
    DaemonFree(hEchoUdp);
    DaemonFree(hEcho);

    // Kill any active console
    ConsoleClose();
    NetworkCtrl_deInit();
    #endif
#endif
#ifdef BUILD_A15
    #ifdef NDK_PROC_TO_USE_A15_0
    DaemonFree(hOob);
    DaemonFree(hNull);
    DaemonFree(hData);
    DaemonFree(hEchoUdp);
    DaemonFree(hEcho);

    // Kill any active console
    ConsoleClose();
    NetworkCtrl_deInit();
    #endif
#endif

}


/**
 *******************************************************************************
 *
 * \brief Print link status
 *
 *        This is a callback from the Ethernet driver. This function
 *        is called whenever there is a change in link state. The
 *        current PHY and current link state are passed as parameters.
 *
 *******************************************************************************
 */
static void LOCAL_linkStatus( uint32_t phy, uint32_t linkStatus )
{
    Vps_printf(" NDK: Link Status: %s on PHY %" PRIu32 "\n",LinkStr[linkStatus],phy);
}

/**
 * \brief Return ID of processor on which networking runs
 */
UInt32 Utils_netGetProcId()
{
    UInt32 procId = SYSTEM_PROC_INVALID;

    #ifdef NDK_PROC_TO_USE_IPU1_0
    procId = SYSTEM_PROC_IPU1_0;
    #endif

    #ifdef NDK_PROC_TO_USE_IPU1_1
    procId = SYSTEM_PROC_IPU1_1;
    #endif

    #ifdef NDK_PROC_TO_USE_A15_0
    procId = SYSTEM_PROC_A15_0;
    #endif

    return procId;
}

/**
 *******************************************************************************
 * \brief Retrun IP address as a string
 *
 *        If network stack is not initialized correctly 0.0.0.0 IP address
 *        is returned
 *
 * \param ipAddrStr [OUT] Assigned IP address as a string
 *
 *******************************************************************************
 */
void Utils_ndkGetIpAddrStr(char *ipAddrStr)
{
    IPN ipAddr;

    memset(&ipAddr, 0, sizeof(ipAddr));

    strcpy(ipAddrStr,"none");

#ifdef BUILD_M4_0
    #ifdef NDK_PROC_TO_USE_IPU1_0
    NtIfIdx2Ip(NET_IF_IDX, &ipAddr);
    NtIPN2Str(ipAddr, ipAddrStr);
    #endif
#endif
#ifdef BUILD_M4_1
    #ifdef NDK_PROC_TO_USE_IPU1_1
    NtIfIdx2Ip(NET_IF_IDX, &ipAddr);
    NtIPN2Str(ipAddr, ipAddrStr);
    #endif
#endif
#ifdef BUILD_A15
    #ifdef NDK_PROC_TO_USE_A15_0
    NtIfIdx2Ip(NET_IF_IDX, &ipAddr);
    NtIPN2Str(ipAddr, ipAddrStr);
    #endif
#endif
}

Int32 Utils_netGetIpAddrStr(char *ipAddr)
{
    UInt32 linkId, procId;
    Int32 status;
    SystemCommon_IpAddr prm;

    strcpy(ipAddr, "none" );

    procId = Utils_netGetProcId();

    if(procId==SYSTEM_PROC_INVALID)
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }
    else
    {
        linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);

        status = System_linkControl(
            linkId,
            SYSTEM_COMMON_CMD_GET_IP_ADDR,
            &prm,
            sizeof(prm),
            TRUE
        );

        if(status==SYSTEM_LINK_STATUS_SOK)
        {
            strcpy(ipAddr, prm.ipAddr);
        }
    }

    return status;
}

Bool Utils_netIsAvbEnabled()
{
    Bool status = FALSE;

#ifdef NDK_PROC_TO_USE_IPU1_0
    status = TRUE;
#endif

#ifdef NDK_PROC_TO_USE_IPU1_1
    status = TRUE;
#endif

#ifdef NDK_PROC_TO_USE_A15_0
    status = TRUE;
#endif

    return status;
}

