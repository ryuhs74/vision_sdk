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
 * \file chains_main.c
 *
 * \brief  APIs for selecting the usecase chain.
 *
 *          APIs for selecting the required usecase chain and run
 *         time menu configurations .
 *         It also provide API's for instrumentation of load and heap usage
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 * \version 0.2 (Jun 2015) : [YM] Added fast boot usecase for tda3x
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <examples/tda2xx/include/chains.h>
#include <examples/tda2xx/include/chains_common.h>
#include <src/utils_common/include/utils_prcm_stats.h>
#include <examples/tda2xx/include/uartCmd.h>
#include <examples/tda2xx/include/error_monitor.h>
#include <src/utils_common/include/utils_lut.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Uncommented below line to disable user input
 *******************************************************************************
 */
//#define CHAINS_DISABLE_GET_CHAR


Chains_Ctrl gChains_usecaseCfg;

/**
 *******************************************************************************
 * \brief Max Size of the input string in characters.
 *******************************************************************************
 */
#define MAX_INPUT_STR_SIZE  (80)

/**
 *******************************************************************************
 * \brief Menu setting display string.
 *******************************************************************************
 */
char gChains_menuUseCases[] = {
    "\r\n "
    "\r\n Vision SDK Use-cases,"
    "\r\n -------------------- "
    "\r\n 1: Single Camera Use-cases"
    "\r\n 2: Multi-Camera LVDS Use-cases"
    "\r\n 3: AVB RX Use-cases, (TDA2x & TDA2Ex ONLY)"
    "\r\n 4: Dual Display Use-cases, (TDA2x EVM ONLY)"
    "\r\n 5: ISS Use-cases, (TDA3x ONLY)"
    "\r\n 6: Stereo Use-cases, (TDA2x MonsterCam ONLY)"
    "\r\n 7: Network RX/TX Use-cases"
	"\r\n 8: AVM-E500 Start"
	"\r\n "
    "\r\n s: System Settings "
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuSingleCameraUsecases[] = {
    "\r\n "
    "\r\n Single Camera Use-cases,"
    "\r\n ------------------------"
    "\r\n 1: 1CH VIP capture + Display "
    "\r\n 2: 1CH VIP capture + Alg Frame Copy (DSP1) + Display "
    "\r\n 3: 1CH VIP capture + Alg Frame Copy (EVE1) + Display "
    "\r\n 4: 1CH VIP capture + Alg Frame Copy (A15) + Display "
    "\r\n 5: 1CH VIP capture + Edge Detect (EVE1) + Display "
    "\r\n 6: 1CH VIP capture + Dense Optical Flow (EVEx) + Display (HDMI)"
    "\r\n 7: 1CH VIP capture (HDMI) + Pedestrain and Traffic Sign Detect (EVE1 + DSP1) + Display "
    "\r\n 8: 1Ch VIP capture + Sparse Optical Flow (EVE1) + Display"
    "\r\n 9: 1Ch VIP capture (HDMI) + Lane Detect (DSP1) + Display"
    "\r\n a: 1CH VIP capture + Alg Subframe Copy (EVE1) + Display "
    "\r\n b: 1Ch VIP capture (HDMI) + FrontCam Analytics (PD+TSR+LD+SOF) (DSPx, EVEx) + Display (HDMI)"
    "\r\n c: 1Ch VIP capture + DSSWB + CRC + Display (Supported only on TDA3x)"
    "\r\n d: 1Ch VIP capture + ENC + DEC + VPE + Display"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuMultiCameraLvdsUsecases[] = {
    "\r\n "
    "\r\n Multi-Camera LVDS Use-cases,"
    "\r\n ----------------------------"
    "\r\n 1: 4CH VIP Capture + Mosaic Display "
    "\r\n 2: 4CH VIP Capture + Surround View (DSP) + Display (HDMI) (TDA2x & TDA2Ex ONLY)"
    "\r\n 3: 5CH VIP Capture + Surround View (DSPx) + Analytics (DSP/EVE) + Ultrasound (DSPx) + HDMI Display (HDMI) (TDA2x ONLY)"
    "\r\n 4: 4CH VIP Capture + Surround View (DSPx) + Display (HDMI) (TDA3x ONLY)"
    "\r\n 5: 2CH VIP Capture (2560x720) + Surround View (DSPx) + Display (TDA2x + TIDA0455 only)"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuAvbRxUsecases[] = {
    "\r\n "
    "\r\n AVB RX Use-cases,"
    "\r\n -----------------"
    "\r\n 1: 4CH AVB Capture + Decode + VPE + Sync + Alg DMA SW Mosaic (IPU1-0) + Display (TDA2x & TDA2Ex ONLY)"
    "\r\n 2: 4CH AVB Capture + Surround View (DSPx) + Display (HDMI) (TDA2x & TDA2Ex ONLY)"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuDualDisplayUsecases[] = {
    "\r\n "
    "\r\n Dual Display Use-cases, (TDA2x EVM ONLY)"
    "\r\n ----------------------------------------"
    "\r\n 1: 1CH VIP capture + Dual Display"
    "\r\n 2: 1CH VIP capture + Edge Detect (EVE1) + Dual Display"
    "\r\n 3: 2CH LVDS VIP capture + Dual Display"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuIssUsecases[] = {
    "\r\n "
    "\r\n ISS Use-cases, (TDA3x ONLY)"
    "\r\n ---------------------------"
    "\r\n l: 1CH ISS capture + ISS ISP + ISS LDC+VTNF + Display"
    "\r\n 2: 4CH ISS capture + ISS ISP + Surround View (DSP1) +  Display"
    "\r\n 3: 1CH ISS capture (AR0132) + ISS ISP Monochrome +  Display"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuStereoUsecases[] = {
    "\r\n "
    "\r\n Stereo Use-cases, (TDA2x MonsterCam ONLY)"
    "\r\n -----------------------------------------"
    "\r\n 1: 2CH VIP capture + Stereo (DSPx, EVEx) + Display (HDMI)"
    "\r\n 2: Network capture + Stereo (DSPx, EVEx) + PD+TSR+LD+SOF (DSPx, EVEx) + Display (HDMI)"
    "\r\n 3: 2CH VIP  capture + SoftISP + Remap + Display - USED for Stereo Calibration "
    "\r\n 4: Network + Stereo + Display (HDMI)"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 * \brief Menu for system settings.
 *******************************************************************************
 */
 char gChains_menuSystemSettings[] = {
    "\r\n "
    "\r\n ==============="
    "\r\n System Settings"
    "\r\n ==============="
    "\r\n "
    "\r\n 1: Display Settings"
    "\r\n 2: Capture Settings"
    "\r\n 3: ISS Settings (TDA3x ONLY)"
    "\r\n 4: Enable Charging via USB2 Port (TDA2x EVM ONLY)"
    "\r\n 5: Print PRCM Statistics"
    "\r\n 6: Show Memory/CPU/DDR BW usage"
    "\r\n 7: Set EVE Power Settings"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 * \brief Menu for display settings.
 *******************************************************************************
 */
char gChains_menuDisplaySettings[] = {
    "\r\n "
    "\r\n ================"
    "\r\n Display Settings"
    "\r\n ================"
    "\r\n "
    "\r\n 1: Component 480P"
    "\r\n 2: LCD  10-inch 1280x720@60fps"
    "\r\n 3: HDMI 1080P60 "
    "\r\n 4: HDMI 720P60 "
    "\r\n 5: SDTV NTSC "
    "\r\n 6: SDTV PAL "
    "\r\n 7: HDMI XGA TDM mode (TDA3x ONLY) "
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};


char gChains_menuEVEPowerSetting[] = {
    "\r\n "
    "\r\n =================="
    "\r\n EVE Power Settings"
    "\r\n =================="
    "\r\n "
    "\r\n 1: EVE Auto Clock Gate (Cut the clocks to EVE)"
    "\r\n 2: EVE ARP32 Idle (ARP32 goes to Idle, clocks to EVE ON)"
    "\r\n "
    "\r\n Note: When Option 1 is chosen the CPU Load is"
    "\r\n       not given accurately."
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};


/**
 *******************************************************************************
 * \brief Menu for capture settings.
 *******************************************************************************
 */
char gChains_menuCaptureSrc[] = {
    "\r\n "
    "\r\n =============="
    "\r\n Capture Source"
    "\r\n =============="
    "\r\n "
    "\r\n 1: OV10635 Sensor 720P30"
    "\r\n 2: HDMI Capture 1080P60 "
    "\r\n 3: OV10640 Sensor 720P30 - CSI2 (TDA3x ONLY)"
    "\r\n 4: OV10640 Sensor 720P30 - Parallel (TDA3x ONLY)"
    "\r\n 5: AR0132  Sensor 720P60 - Parallel (TDA3x ONLY)"
    "\r\n 6: AR0140  Sensor 720P60 - Parallel (TDA3x ONLY)"
    "\r\n 7: IMX224  Sensor 1280x960 - CSI2 (TDA3x ONLY)"
    "\r\n 8: AR0132  Sensor 720P60 DM388 - Parallel (TDA2x MonsterCam ONLY)"
	"\r\n 9: ISX016  (AVM_C500)"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char *gChain_displayTypeName[] =
{
    "LCD   800x480  @ 60fps (7-inch)",
    "LCD  1280x720  @ 60fps (10-inch)",
    "HDMI 1280x720  @ 60fps",
    "HDMI 1920x1080 @ 60fps",
    "SDTV NTSC 720x480i @ 60fps",
    "SDTV PAL  720x576i @ 60fps",
    "XGA 1024x768 @ 60fps - (TDA3x ONLY - using TDM Mode)",
};

char *gChain_captureSrcName[] =
{
    "Sensor OV10635 1280x720  @ 30fps - VIP, YUV422",
    "HDMI   RX      1280x720  @ 60fps - VIP, YUV422",
    "HDMI   RX      1920x1080 @ 60fps - VIP, YUV422",
    "Sensor AR0132  1280x720  @ 30fps - VIP, RCCC (TDA2x MonsterCam ONLY)",
    "Sensor AR0132  1280x720  @ 60fps - VIP, Bayer, Ext. ISP (TDA2x MonsterCam ONLY)",
    "Sensor OV10640 1280x720  @ 30fps - ISS CSI2, Bayer (TDA3x EVM ONLY)",
    "Sensor OV10640 1280x720  @ 30fps - ISS CPI , Bayer (TDA3x EVM ONLY)",
    "Sensor AR0132  1280x720  @ 60fps - ISS CPI , Bayer (TDA3x EVM ONLY)",
    "Sensor AR0140  1280x800  @ 30fps - ISS CPI , Bayer (TDA3x EVM ONLY)",
    "Sensor IMX224  1280x960  @ 30fps - ISS CSI2, Bayer (TDA3x EVM ONLY)",
    "Sensor AR0132 DM388 1280x720 @ 60fps - VIP, YUV422 (TDA2x MonsterCam ONLY)",
};

/**
 *******************************************************************************
 * \brief Run Time Menu string.
 *******************************************************************************
 */
char gChains_runTimePMMenu[] = {
    "\r\n "
    "\r\n ===================="
    "\r\n Chains PRCM Stats Menu"
    "\r\n ===================="
    "\r\n "
    "\r\n 1: Show  Dpll Status "
    "\r\n 2: Show  Temperature "
#ifdef PMIC_I2C_ENABLE
    "\r\n 3: Show  Voltages "
#endif
    "\r\n 4: Show  Module Power State "
    "\r\n 5: Show  CPU Frequency "
    "\r\n 6: Show  Peripherals Frequency "
    "\r\n 7: Dump  Prcm Register Data "
    "\r\n 8: Print All PRCM Stats"
    "\r\n "
    "\r\n x: To Exit out of menu "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char gChains_menuIssSettings_0[] = {
    "\r\n "
    "\r\n ============"
    "\r\n ISS Settings"
    "\r\n ============"
    "\r\n "
};

char gChains_menuIssSettings_1[] = {
    "\r\n"
    "\r\n"
    "\r\n 1: Disable LDC and VTNF Mode"
    "\r\n 2: Enable LDC Only Mode"
    "\r\n 3: Enable VTNF Only Mode"
    "\r\n 4: Enable LDC and VTNF Mode"
    "\r\n 5: Enable Two Pass WDR"
    "\r\n 6: Disable WDR"
    "\r\n 7: Enable One Pass WDR"
    "\r\n "
    "\r\n x: Exit"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

char Chains_readChar2(); //ryuhs74@20151020 - HDMA On/Off

void Chains_showIssSettings()
{
    char *onOff[] = { "OFF", "ON" };
    char *wdr[] = {"OFF", "OnePass", "TwoPass"};

    Vps_printf(" ISS Settings   : LDC=[%s] VTNF=[%s] WDR=[%s] \n",
        onOff[gChains_usecaseCfg.issLdcEnable],
        onOff[gChains_usecaseCfg.issVtnfEnable],
        wdr[gChains_usecaseCfg.issWdrMode]
        );
}

void Chains_showIssSettingsMenu()
{
    char ch;
    int done = FALSE;

    do
    {
        Vps_printf(gChains_menuIssSettings_0);
        Chains_showIssSettings();
        Vps_printf(gChains_menuIssSettings_1);
        Vps_printf(" \r\n");
        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                gChains_usecaseCfg.issLdcEnable = 0;
                gChains_usecaseCfg.issVtnfEnable = 0;
                break;
            case '2':
                gChains_usecaseCfg.issLdcEnable = 1;
                gChains_usecaseCfg.issVtnfEnable = 0;
                break;
            case '3':
                gChains_usecaseCfg.issVtnfEnable = 1;
                gChains_usecaseCfg.issLdcEnable = 0;
                break;
            case '4':
                gChains_usecaseCfg.issVtnfEnable = 1;
                gChains_usecaseCfg.issLdcEnable = 1;
                break;
            case '5':
                gChains_usecaseCfg.issWdrMode = CHAINS_ISS_WDR_MODE_TWO_PASS;
                break;
            case '6':
                gChains_usecaseCfg.issWdrMode = CHAINS_ISS_WDR_MODE_DISABLED;
                break;
            case '7':
                gChains_usecaseCfg.issWdrMode = CHAINS_ISS_WDR_MODE_SINGLE_PASS;
                break;
            case 'x':
            case 'X':
                done = TRUE;
                break;
            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }

    }while(done == FALSE);

}

/**
 *******************************************************************************
 *
 * \brief   Print PM Statistics
 *
 *******************************************************************************
*/
Void ChainsCommon_PrintPMStatistics()
{
    char ch;
    UInt32 done = FALSE;

    while(!done)
    {
        Vps_printf(gChains_runTimePMMenu);
        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                Utils_prcmPrintAllDpllValues();
                break;
            case '2':
                Utils_prcmPrintAllVDTempValues();
                break;
            case '3':
#ifdef PMIC_I2C_ENABLE
                Utils_prcmPrintAllVoltageValues();
#endif
                break;
            case '4':
                Utils_prcmPrintAllModuleState();
                break;
            case '5':
                Utils_prcmPrintAllCPUFrequency();
                break;
            case '6':
                Utils_prcmPrintAllPeripheralsFrequency();
                break;
            case '7':
                Utils_prcmDumpRegisterData();
                break;
            case '8':
                Utils_prcmPrintAllDpllValues();
                Task_sleep(100);
                Utils_prcmPrintAllVDTempValues();
#ifdef PMIC_I2C_ENABLE
                Utils_prcmPrintAllVoltageValues();
#endif
                Task_sleep(100);
                Utils_prcmPrintAllModuleState();
                Task_sleep(100);
                Utils_prcmPrintAllCPUFrequency();
                Task_sleep(100);
                Utils_prcmPrintAllPeripheralsFrequency();
                Utils_prcmDumpRegisterData();
                Task_sleep(100);
                done = TRUE;
                break;
            case 'x':
                done = TRUE;
                break;
            default:
                    Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
            break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Function to display select options using uart print message
 *
 *******************************************************************************
*/
Void Chains_showMainMenu()
{
    char ipAddr[20];

    Utils_netGetIpAddrStr(ipAddr);

    Vps_printf(" \n");
    Vps_printf(" Current System Settings,\n");
    Vps_printf(" ========================\n");
    Vps_printf(" Display Type   : %s \n",
            gChain_displayTypeName[gChains_usecaseCfg.displayType]);
    Vps_printf(" Capture Source : %s \n",
            gChain_captureSrcName[gChains_usecaseCfg.captureSrc]);
    Vps_printf(" My IP address  : %s \n",
            ipAddr );
    Chains_showIssSettings();

    Vps_printf(" \n");
    Vps_printf(" ============\r\n");
    Vps_printf(" Usecase Menu\r\n");
    Vps_printf(" ============\r\n");

    Vps_printf(gChains_menuUseCases);
}

//ryuhs74@20150909 - For HW Test
Void Chains_showAVM_E500()
{
    char ipAddr[20];

    Utils_netGetIpAddrStr(ipAddr);

    Vps_printf(" \n");
    Vps_printf(" AVM-E500 System Settings,\n");
    Vps_printf(" ========================\n");
    Vps_printf(" Capture Source : ISX016  (AVM_C500) \n");
    Vps_printf(" My IP address  : %s \n", ipAddr );
    Chains_showIssSettings();

    Vps_printf(" \n");
    Vps_printf(" ============\r\n");
    Vps_printf(" Usecase Menu\r\n");
    Vps_printf(" ============\r\n");

    Vps_printf("Multi-Camera LVDS Use-cases\n4CH VIP Capture + Mosaic Display\n");
}

/**
 *******************************************************************************
 *
 * \brief   Function to set display settings.
 *
 *******************************************************************************
*/
Void Chains_showDisplaySettingsMenu()
{
    char ch;
    Bool displaySelectDone;
    displaySelectDone = FALSE;

    do
    {
        Vps_printf(gChains_menuDisplaySettings);
        Vps_printf(" \r\n");
        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_CH7026_480P;
                displaySelectDone = TRUE;
                break;
            case '2':
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_LCD_10_INCH;
                displaySelectDone = TRUE;
                break;
            case '3':
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_1080P;
                displaySelectDone = TRUE;
                break;
            case '4':
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_720P;
                displaySelectDone = TRUE;
                break;
            case '5':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_SDTV_NTSC;
                    displaySelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This display NOT supported on current platform\n");
                }
                break;
            case '6':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_SDTV_PAL;
                    displaySelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This display NOT supported on current platform\n");
                }
                break;
            case '7':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM;
                    displaySelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This display NOT supported on current platform\n");
                }
                break;
            case 'x':
            case 'X':
                displaySelectDone = TRUE;
                break;
            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }

    }while(displaySelectDone == FALSE);
}

/**
 *******************************************************************************
 *
 * \brief   Function to set capture settings.
 *
 *******************************************************************************
*/
Void Chains_showCaptureSettingsMenu()
{
    char ch;
    Bool captSrcSelectDone;
    captSrcSelectDone = FALSE;

    do
    {
        Vps_printf(gChains_menuCaptureSrc);
        Vps_printf(" \r\n");
        ch = Chains_readChar();
        switch(ch)
        {
            case '1':
                gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_OV10635;
                captSrcSelectDone = TRUE;
                break;
            case '2':
                gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_HDMI_1080P;
                captSrcSelectDone = TRUE;
                break;
            case '3':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_OV10640_CSI2;
                    captSrcSelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This sensor NOT supported on current platform\n");
                }
                break;
            case '4':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_OV10640_PARALLEL;
                    captSrcSelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This sensor NOT supported on current platform\n");
                }
                break;
            case '5':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL;
                    captSrcSelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This sensor NOT supported on current platform\n");
                }

                break;

            case '6':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.captureSrc =
                        CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL;
                    captSrcSelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This sensor NOT supported on current platform\n");
                }
                break;
            case '7':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.captureSrc =
                        CHAINS_CAPTURE_SRC_IMX224_CSI2;
                    captSrcSelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This sensor NOT supported on current platform\n");
                }
                break;
            case '8':
                if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                    captSrcSelectDone = TRUE;
                }
                else
                {
                    Vps_printf(" This sensor NOT supported on current platform\n");
                }
            break;
            case '9':
				gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_ISX016;
				captSrcSelectDone = TRUE;
            break;

            case 'x':
            case 'X':
                captSrcSelectDone = TRUE;
                break;
            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }

    }while(captSrcSelectDone == FALSE);
}

Void ChainsCommon_SendEVEPowerMsg()
{
    char ch;
    Bool evePowerSelectDone;
    UInt32 procId, linkId;

    evePowerSelectDone= FALSE;

    do
    {
        Vps_printf(gChains_menuEVEPowerSetting);
        Vps_printf(" \r\n");
        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                for (procId = SYSTEM_PROC_EVE1;
                        procId <= SYSTEM_PROC_EVE4;
                        procId++)
                {
                    if (System_isProcEnabled(procId)==FALSE)
                        continue;
                    linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);
                    System_linkControl(
                             linkId,
                             SYSTEM_COMMON_EVE_AUTOCG,
                             NULL,
                             0,
                             FALSE
                         );
                }
                evePowerSelectDone= TRUE;
                break;
            case '2':
                for (procId = SYSTEM_PROC_EVE1;
                        procId <= SYSTEM_PROC_EVE4;
                        procId++)
                {
                    if (System_isProcEnabled(procId)==FALSE)
                        continue;
                    linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);
                    System_linkControl(
                             linkId,
                             SYSTEM_COMMON_EVE_IDLE,
                             NULL,
                             0,
                             FALSE
                         );
                }
                evePowerSelectDone= TRUE;
                break;
            case 'x':
            case 'X':
                evePowerSelectDone= TRUE;
                break;
            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }

    }while(evePowerSelectDone == FALSE);

}

/**
 *******************************************************************************
 *
 * \brief   Function to select systems settings option.
 *
 *******************************************************************************
*/
Void Chains_showSystemSettingsMenu()
{
    char ch;
    Bool done;
    done = FALSE;

    do
    {
        Vps_printf(gChains_menuSystemSettings);
        Vps_printf(" \r\n");
        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                Chains_showDisplaySettingsMenu();
                done = TRUE;
                break;
            case '2':
                Chains_showCaptureSettingsMenu();
                done = TRUE;
                break;
            case '3':
                Chains_showIssSettingsMenu();
                done = TRUE;
            case '4':
                Board_enableUsbCharging();
                done = TRUE;
                break;
            case '5':
                ChainsCommon_PrintPMStatistics();
                done = TRUE;
                break;
            case '6':
                Utils_prcmPrintAllVDTempValues();
                Chains_memPrintHeapStatus();
                Chains_statCollectorPrint();
                Chains_prfLoadCalcEnable(TRUE, FALSE, FALSE);
                Vps_printf(" CHAINS: Waiting for CPU load calculation to Complete !!!!\n");
                Task_sleep(2000);
                Vps_printf(" CHAINS: CPU load calculation to Complete !!!!\n");
                Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
                done = TRUE;
                break;
            case '7':
                ChainsCommon_SendEVEPowerMsg();
                done = TRUE;
                break;
            case 'x':
            case 'X':
                done = TRUE;
                break;
            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }while(!done);
}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuSingleCameraRun()
{
    Chains_Ctrl usecaseCfg;
    char ch;
    Bool done = FALSE;

    while(!done)
    {
        Chains_statCollectorReset();

        Vps_printf(gChains_menuSingleCameraUsecases);

        ch = Chains_readChar();
        Vps_printf(" \r\n");

        switch(ch)
        {
            case '1':
                if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                }
                Chains_vipSingleCam_Display(&gChains_usecaseCfg);
                break;

            case '2':
                gChains_usecaseCfg.algProcId = SYSTEM_PROC_DSP1;
                if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                }
                Chains_vipSingleCameraFrameCopy(&gChains_usecaseCfg);
                break;

            case '3':
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                    gChains_usecaseCfg.algProcId = SYSTEM_PROC_EVE1;
                    if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                    {
                        gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                    }
                    Chains_vipSingleCameraFrameCopy(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### TDA2Ex do not have EVE core, usecase with EVE is not supported \n");
                }
                break;

            case '4':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    gChains_usecaseCfg.algProcId = SYSTEM_PROC_A15_0;
                    Chains_vipSingleCameraFrameCopy(&gChains_usecaseCfg);
                }
                break;

            case '5':
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                    gChains_usecaseCfg.algProcId = SYSTEM_PROC_EVE1;
                    if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                    {
                        gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                    }
                    Chains_vipSingleCameraEdgeDetection(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### TDA2Ex do not have EVE core, usecase with EVE is not supported \n");
                }
                break;

            case '6':
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                    gChains_usecaseCfg.numLvdsCh = 1;
                    usecaseCfg = gChains_usecaseCfg;
                    if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                    {
                        usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                    }
                    Chains_vipSingleCameraDenseOpticalFlow(&usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### TDA2Ex do not have EVE core, usecase with EVE is not supported \n");
                }
                break;

            case '7':
                usecaseCfg = gChains_usecaseCfg;
                if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_HDMI_1080P )
                {
                    Vps_printf(" ### ONLY HDMI 1080p60 input supported for this usecase ");
                    Vps_printf(" ### Please choose HDMI 1080p60 Capture Source using option 's'\n");
                    break;
                }
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                    Chains_vipSingleCameraObjectDetect(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### TDA2Ex do not have EVE core, usecase with EVE is not supported \n");
                }
                /* No HDMI input for monstercam */
                break;

            case '8':
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                    if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                    {
                        gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                    }
                    Chains_vipSingleCameraSparseOpticalFlow(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### TDA2Ex do not have EVE core, usecase with EVE is not supported \n");
                }
                break;

            case '9':
                usecaseCfg = gChains_usecaseCfg;
                if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_HDMI_1080P )
                {
                    Vps_printf(" ### ONLY HDMI 1080p60 input supported for this usecase ");
                    Vps_printf(" ### Please choose HDMI 1080p60 Capture Source using option 's'\n");
                    break;
                }
                Chains_vipSingleCameraLaneDetect(&gChains_usecaseCfg);
                /* No HDMI input for monstercam */
                break;

            case 'a':
            case 'A':
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                    if (BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                    {
                        gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_DM388;
                    }
                    Chains_vipSingleCameraSubFrameCopy(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### TDA2Ex do not have EVE core, usecase with EVE is not supported \n");
                }
                break;

            case 'b':
            case 'B':
                usecaseCfg = gChains_usecaseCfg;
                if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_HDMI_1080P )
                {
                    Vps_printf(" ### ONLY HDMI 1080p60 input supported for this usecase ");
                    Vps_printf(" ### Please choose HDMI 1080p60 Capture Source using option 's'\n");
                    break;
                }
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    Chains_vipSingleCameraAnalyticsTda3xx(&gChains_usecaseCfg);
                }
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    Chains_singleCameraAnalyticsTda2xx(&gChains_usecaseCfg);
                }
                /* No HDMI input for monstercam */
                break;

            case 'c':
            case 'C':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    Chains_vipSingleCam_DisplayWbCrc(&gChains_usecaseCfg);
                }
                break;

            case 'd':
            case 'D':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    Chains_vipSingleCam_EncDec_Display(&gChains_usecaseCfg);
                }
                break;

            case 'h':
            case 'H':
                /* Chains_runDmaTest(); */
                #ifdef HCF_INCLUDE
                {
                    Void Chains_hcfDemoMain(Chains_Ctrl *chainsCfg);

                    usecaseCfg = gChains_usecaseCfg;

                    Chains_hcfDemoMain(&usecaseCfg);
                }
                #endif
                break;

            case 'x':
            case 'X':
                done = TRUE;
                break;

            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuMultiCameraLvdsRun()
{
    Chains_Ctrl usecaseCfg;
    char ch;
    Bool done = FALSE;

    while(!done)
    {
        Chains_statCollectorReset();

        Vps_printf(gChains_menuMultiCameraLvdsUsecases);

        ch = Chains_readChar();
        Vps_printf(" \r\n");

        switch(ch)
        {
            case '1':
                if ((Bsp_platformIsTda2xxFamilyBuild()) ||
                    (Bsp_platformIsTda3xxFamilyBuild()))
                {
#if 0 ///craven@1509
                    if(Board_isMultiDesConnected())
                    {
                        gChains_usecaseCfg.algProcId = System_getSelfProcId();
                        gChains_usecaseCfg.numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;
                        usecaseCfg = gChains_usecaseCfg;
                        if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_OV10635 )
                        {
                            Vps_printf(" ### ONLY OV10635 Sensor supported for this usecase ");
                            Vps_printf(" ### Please choose OV10635 as Capture Source using option 's'\n");
                            break;
                        }
                        if (Bsp_platformIsTda2xxFamilyBuild() && BSP_BOARD_MULTIDES == Bsp_boardGetId())
                        {
                            Chains_lvdsVipMultiCam_Display_tda2xx(&gChains_usecaseCfg);
                        }
                        if (Bsp_platformIsTda2xxFamilyBuild() && BSP_BOARD_MONSTERCAM == Bsp_boardGetId())
                        {
                           gChains_usecaseCfg.numLvdsCh = 1;
                           Chains_lvdsVipMultiCam_Display_tda2xx_mc(&gChains_usecaseCfg);
                        }
                        if (Bsp_platformIsTda3xxFamilyBuild())
                        {
                            Chains_lvdsVipMultiCam_Display_tda3xx(&gChains_usecaseCfg);
                        }
                    }
                    else
                    {
                        Vps_printf(" ### Cannot run usecase. MulitDes Board Not Connected \n");
                    }
#endif
                    gChains_usecaseCfg.algProcId = System_getSelfProcId();
                    gChains_usecaseCfg.numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;
                    usecaseCfg = gChains_usecaseCfg;
                    Chains_lvdsVipMultiCam_Display_tda2xx(&gChains_usecaseCfg);
                }
                break;

            case '2':
#if 0
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    if(Board_isMultiDesConnected())
                    {
                        gChains_usecaseCfg.numLvdsCh = 4;
                        gChains_usecaseCfg.svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_2D;
                        gChains_usecaseCfg.enableCarOverlayInAlg = 0;
                        usecaseCfg = gChains_usecaseCfg;
                        if( usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_HDMI_1080P )
                        {
                            Vps_printf("  ### ONLY HDMI 1080p Display supported for this usecase ");
                            Vps_printf("Please choose HDMI Display Type using option 's'\n");
                            break;
                        }
                        if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_OV10635 )
                        {
                            Vps_printf(" ### ONLY OV10635 Sensor supported for this usecase ");
                            Vps_printf(" ### Please choose OV10635 as Capture Source using option 's'\n");
                            break;
                        }

                        Chains_lvdsVipSurroundViewStandalone(&gChains_usecaseCfg);
                    }
                    else
                    {
                        Vps_printf(" ### Cannot run usecase.  MulitDes Board Not Connected \n");
                    }
                }
#else
                gChains_usecaseCfg.numLvdsCh = 4;
				gChains_usecaseCfg.svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_2D;
				gChains_usecaseCfg.enableCarOverlayInAlg = 0;
				usecaseCfg = gChains_usecaseCfg;

				usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_1080P;

				if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_ISX016 )
				{
					Vps_printf(" ### ONLY OV10635 Sensor supported for this usecase ");
					Vps_printf(" ### Please choose OV10635 as Capture Source using option 's'\n");
					break;
				}

				Chains_lvdsVipSurroundViewStandalone(&gChains_usecaseCfg);
#endif
                break;

            case '3':
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    if(Board_isMultiDesConnected())
                    {
                        gChains_usecaseCfg.numLvdsCh = 5;
                        gChains_usecaseCfg.svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_2D;
                        gChains_usecaseCfg.enableCarOverlayInAlg = 0;
                        usecaseCfg = gChains_usecaseCfg;
                        if( usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_HDMI_1080P )
                        {
                            Vps_printf("  ### ONLY HDMI 1080p Display supported for this usecase ");
                            Vps_printf("Please choose HDMI Display Type using option 's'\n");
                            break;
                        }
                        if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_OV10635 )
                        {
                            Vps_printf(" ### ONLY OV10635 Sensor supported for this usecase ");
                            Vps_printf(" ### Please choose OV10635 as Capture Source using option 's'\n");
                            break;
                        }

                        Chains_lvdsVipSurroundViewAnalyticsUltrasound(&gChains_usecaseCfg);
                    }
                    else
                    {
                        Vps_printf(" ### Cannot run usecase. MulitDes Board Not Connected \n");
                    }
                }
                break;

            case '4':
                if (Bsp_platformIsTda3xxFamilyBuild())
                {
                    if(Board_isMultiDesConnected())
                    {
                        gChains_usecaseCfg.algProcId = System_getSelfProcId();
                        gChains_usecaseCfg.numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;
                        usecaseCfg = gChains_usecaseCfg;
                        Chains_lvdsVipSurroundView(&gChains_usecaseCfg);
                    }
                    else
                    {
                        Vps_printf(" ### Cannot run usecase. MulitDes Board Not Connected \n");
                    }
                }
                break;

            case '5':
                {
                    Chains_CaptureSrc srcBkp = gChains_usecaseCfg.captureSrc;
                    /* Set capture src to MAX to bypass sensor driver */
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_MAX;
                    Chains_ov490VipSurroundViewStandalone(&gChains_usecaseCfg);
                    /* Revert capture src to original value */
                    gChains_usecaseCfg.captureSrc = srcBkp;
                }
                break;

            case 'x':
            case 'X':
                done = TRUE;
                break;

            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuAvbRxRun()
{
    Chains_Ctrl usecaseCfg;
    char ch;
    Bool done = FALSE;

    while(!done)
    {
        Chains_statCollectorReset();

        Vps_printf(gChains_menuAvbRxUsecases);

        ch = Chains_readChar();
        Vps_printf(" \r\n");

        switch(ch)
        {
            case '1':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    if(Utils_netIsAvbEnabled())
                    {
                        gChains_usecaseCfg.numLvdsCh = 4;
                        Chains_avbRx_Dec_Display(&gChains_usecaseCfg);
                    }
                    else
                    {
                        Vps_printf(" ### Networking NOT enabled in build, Rebuild \
                               binaries  with NDK_PROC_TO_USE=<proc name> in Rules.make \n");
                    }
                }
                break;

            case '2':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    if(Utils_netIsAvbEnabled())
                    {
                        gChains_usecaseCfg.numLvdsCh = 4;
                        gChains_usecaseCfg.svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_2D;
                        gChains_usecaseCfg.enableCarOverlayInAlg = 0;
                        usecaseCfg = gChains_usecaseCfg;
                        if( usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_HDMI_1080P )
                        {
                            Vps_printf("  ### ONLY HDMI 1080p Display supported for this usecase ");
                            Vps_printf("Please choose HDMI Display Type using option 's'\n");
                            break;
                        }
                        Chains_avbRxSurroundView(&gChains_usecaseCfg);
                    }
                    else
                    {
                        Vps_printf(" ### Networking NOT enabled in build, Rebuild \
                               binaries  with NDK_PROC_TO_USE=<proc name> in Rules.make \n");
                    }
                }
                break;

            case 'x':
            case 'X':
                done = TRUE;
                break;

            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuDualDisplayRun()
{
    Chains_Ctrl usecaseCfg;
    char ch;
    Bool done = FALSE;

    while(!done)
    {
        Chains_statCollectorReset();

        Vps_printf(gChains_menuDualDisplayUsecases);

        ch = Chains_readChar();
        Vps_printf(" \r\n");

        switch(ch)
        {
            case '1':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    usecaseCfg = gChains_usecaseCfg;
                    if( usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_CH7026_480P &&
                        usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_LCD_10_INCH)
                    {
                        Vps_printf(" ### This usecase displays on HDMI and LCD");
                        Vps_printf(" ### Please choose connected LCD type using option 's'\n");
                        break;
                    }
                    chains_vipSingleCam_DualDisplay(&gChains_usecaseCfg);
                }
                break;

            case '2':
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    usecaseCfg = gChains_usecaseCfg;
                    if( usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_CH7026_480P &&
                        usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_LCD_10_INCH)
                    {
                        Vps_printf(" ### This usecase displays on HDMI and LCD");
                        Vps_printf(" ### Please choose connected LCD type using option 's'\n");
                        break;
                    }

                    chains_vipSingleCam_DualDisplayEdgeDetection(&gChains_usecaseCfg);
                }
                break;

            case '3':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    if(Board_isMultiDesConnected())
                    {
                        gChains_usecaseCfg.algProcId = System_getSelfProcId();
                        gChains_usecaseCfg.numLvdsCh = 2;
                        usecaseCfg = gChains_usecaseCfg;
                        if( usecaseCfg.captureSrc!= CHAINS_CAPTURE_SRC_OV10635 )
                        {
                            Vps_printf(" ### ONLY OV10635 Sensor supported for this usecase ");
                            Vps_printf(" ### Please choose OV10635 as Capture Source using option 's'\n");
                            break;
                        }
                        if( usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_CH7026_480P &&
                            usecaseCfg.displayType != CHAINS_DISPLAY_TYPE_LCD_10_INCH)
                        {
                            Vps_printf(" ### This usecase displays on HDMI and LCD");
                            Vps_printf(" ### Please choose connected LCD type using option 's'\n");
                            break;
                        }

                        chains_lvdsVipDualCam_DualDisplay(&gChains_usecaseCfg);
                    }
                    else
                    {
                        Vps_printf(" ### Cannot run usecase. MulitDes Board Not Connected \n");
                    }
                }
                break;
            case 'x':
            case 'X':
                done = TRUE;
                break;

            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }


}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuIssRun()
{
    char ch;
    Bool done = FALSE;

    while(!done)
    {
        Chains_statCollectorReset();

        Vps_printf(gChains_menuIssUsecases);

        ch = Chains_readChar();
        Vps_printf(" \r\n");

        switch(ch)
        {
            case '1':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    Chains_issIspSimcop_Display(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }
                break;
            case '2':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    Chains_issMultCaptIspSimcopSv_Display(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }
                break;
            case '3':
                if(Bsp_platformIsTda3xxFamilyBuild())
                {
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL;
                    Chains_monochrome_issIsp_Display(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }
                break;

            case 'x':
            case 'X':
                done = TRUE;
                break;

            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuStereoRun()
{
    char ch;
    Bool done = FALSE;

    while(!done)
    {
        Chains_statCollectorReset();

        Vps_printf(gChains_menuStereoUsecases);

        ch = Chains_readChar();
        Vps_printf(" \r\n");

        switch(ch)
        {
            case '1':
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    gChains_usecaseCfg.numLvdsCh = 2;
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_AR0132RCCC;
                    Chains_vipStereoOnlyDisplay(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }

                break;

            case '2':
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    gChains_usecaseCfg.numLvdsCh = 2;
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_AR0132RCCC;
                    Chains_vipStereoCameraAnalytics(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }
                break;

            case '3':
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    gChains_usecaseCfg.numLvdsCh = 2;
                    gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_AR0132RCCC;
                    chains_vipStereoCalibration(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }
                break;

            case '4':
                if(Bsp_platformIsTda2xxFamilyBuild() && (BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId()))
                {
                    chains_networkStereoDisplay(&gChains_usecaseCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform \n");
                }

            case 'x':
            case 'X':
                done = TRUE;
                break;

            default:
                Vps_printf(" Unsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Function to select demo depending on user input
 *          Accepts user input as ch and switches to corrosponding usecase
 *
 * \param   ch        [IN]  Input choise for user
 *
 *******************************************************************************
*/
Void Chains_menuNetworkRxTxRun()
{

    Chains_statCollectorReset();

    Chains_networkRxTx(&gChains_usecaseCfg);
}

Void PrintLut(void)
{
	LUT_INDEX i = Basic_frontView;
	for(i = Basic_frontView; i<MAX_LUT_INDEX; i++)
	{
		uint8_t* lut;
		lut = LUTAlloc(i);
		if(lut!=NULL)
		{
            Vps_printf("LUT[%d] %02X %02X %02X %02X %02X %02X %02X %02X\n",i, lut[0], lut[1], lut[2], lut[3], lut[4], lut[5], lut[6], lut[7]);
		}else
		{
            Vps_printf("LUT ALLOC ERROR[%d]\n",i);
		}
	}
}

/**
 *******************************************************************************
 *
 * \brief   Main call for usecase selection and configuration
 *
 *          Chains_main is called form the main of main_ipu1_0.c .
 *          This is the entry point for usecase selection.
 *          Board, UART LCD initializations and demo selections are performed.
 *          It waits in a while loop till the end of demo/usecase is triggred
 *
 * \param   arg0    [IN]  default args
 *
 * \param   arg1    [IN]  default args
 *
 *******************************************************************************
*/
void Start_AVM_E500(); //ryuhs74@20151020 - Add HDMI On/Off Test

Void Chains_main(UArg arg0, UArg arg1)
{
    ChainsCommon_Init();
    Chains_Ctrl_Init(&gChains_usecaseCfg);
    UartCmd_tsk_init();
    Error_Monitor_init();
    PrintLut();
    #ifdef TDA3XX_FAMILY_BUILD
    if(System_isFastBootEnabled())
    {
        gChains_usecaseCfg.displayType   = CHAINS_DISPLAY_TYPE_LCD_10_INCH;
        gChains_usecaseCfg.captureSrc    = CHAINS_CAPTURE_SRC_OV10640_PARALLEL;
        gChains_usecaseCfg.issWdrMode    = CHAINS_ISS_WDR_MODE_TWO_PASS;
        gChains_usecaseCfg.issLdcEnable  = 0;
        gChains_usecaseCfg.issVtnfEnable = 0;

        if(Bsp_platformIsTda3xxFamilyBuild())
        {
            Chains_fastBootIssIspSimcop_pd_Display(&gChains_usecaseCfg);
        }
        else
        {
            Vps_printf(" ### Cannot run usecase. Usecase not supported on this platform or "
                       "     Check FAST_BOOT_INCLUDE in Rules.make \n");
        }
    }
    else
    #endif
    {
        #ifdef CHAINS_DISABLE_GET_CHAR
        Chains_menuMainRun('n');
        #endif
        {
            char ch;
            Bool done;

            Start_AVM_E500();

            done = FALSE;

            while(!done)
            {
                Chains_showMainMenu();

                ch = Chains_readChar();

                Vps_printf(" \r\n");

                switch(ch)
                {
                    case '1':
                        Chains_menuSingleCameraRun();
                        break;

                    case '2':
                        Chains_menuMultiCameraLvdsRun();
                        break;

                    case '3':
                        Chains_menuAvbRxRun();
                        break;

                    case '4':
                        Chains_menuDualDisplayRun();
                        break;

                    case '5':
                        Chains_menuIssRun();
                        break;

                    case '6':
                        Chains_menuStereoRun();
                        break;

                    case '7':
                        Chains_menuNetworkRxTxRun();
                        break;
                    case '8':
                    	Start_AVM_E500();
						break;
                    case 's':
                    case 'S':
                        Chains_showSystemSettingsMenu();
                        break;

                    case 'x':
                    case 'X':
                        done = TRUE;
                        break;
                }
            }
        }
    }

    ChainsCommon_DeInit();
}


void Start_AVM_E500() //ryuhs74@20151020 - Add HDMI On/Off Test
{
	Chains_showAVM_E500();

	Chains_statCollectorReset();

	gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_ISX016;
	gChains_usecaseCfg.algProcId = System_getSelfProcId();
	gChains_usecaseCfg.numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;
	Chains_lvdsVipMultiCam_Display_tda2xx(&gChains_usecaseCfg);
}
/**
 *******************************************************************************
 * \brief Run Time Menu string.
 *******************************************************************************
 */
char gChains_runTimeMenu[] = {
    "\r\n "
    "\r\n ===================="
    "\r\n Chains Run-time Menu"
    "\r\n ===================="
    "\r\n "
    "\r\n 0: Stop Chain"
    "\r\n "
    "\r\n 1: Change Display Channel (Support 4CH LVDS + Mosaic use-case only)"
	"\r\n "
	"\r\n 2: Capture 4ch YUV"
    "\r\n "
    "\r\n p: Print Performance Statistics "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 * \brief   Read a charater from UART or CCS console
 *
 * \return character that is read
 *
 *******************************************************************************
*/

char Chains_readChar()
{
    Int8 ch[80];

    #ifdef ENABLE_UART
    uartRead(ch);
    #else
    fgets((char*)ch, MAX_INPUT_STR_SIZE, stdin);
    if(ch[1] != '\n' || ch[0] == '\n')
    ch[0] = '\n';
    #endif

    return ch[0];
}

/**
 *******************************************************************************
 *
 * \brief   Run time Menu selection
 *
 *          This functions displays the run time options available
 *          And receives user input and calls corrosponding functions run time
 *          Instrumentation logs are printing routine is called in same function
 *
 *******************************************************************************
*/
char Chains_menuRunTime()
{
    Vps_printf(gChains_runTimeMenu);

    return Chains_readChar();
}

Int32 Chains_runDmaTest()
{
    UInt32 procId, linkId;

    for(procId=0; procId<SYSTEM_PROC_MAX; procId++)
    {
        if(System_isProcEnabled(procId)==FALSE)
            continue;

        linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_PROCK_LINK_ID);

        System_linkControl(
            linkId,
            SYSTEM_COMMON_CMD_RUN_DMA_TEST,
            NULL,
            0,
            TRUE
        );
    }

    return SYSTEM_LINK_STATUS_SOK;
}

