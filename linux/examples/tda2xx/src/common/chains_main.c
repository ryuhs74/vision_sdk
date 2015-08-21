/*
 * Copyright (c) 2012-2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <linux/examples/tda2xx/include/chains.h>


Chains_Ctrl gChains_usecaseCfg;

/**
 *******************************************************************************
 * \brief Run Time Test Menu string.
 *******************************************************************************
 */
char gChains_testMenu[] = {
    "\r\n "
    "\r\n ========="
    "\r\n Test Menu"
    "\r\n ========="
    "\r\n "
    "\r\n Single Camera Use-cases,"
    "\r\n ------------------------"
    "\r\n 1: 1CH VIP capture + SGX DISPLAY(A15)"
    "\r\n 2: 1CH VIP capture + Encode + Decode + SGX DISPLAY(A15)"
    "\r\n 3: 1CH VIP capture + PD + TSR (EVE1 + DSPx) + SGX DISPLAY(A15)"
    "\r\n "
    "\r\n Multi-Camera LVDS Use-cases,"
    "\r\n ----------------------------"
    "\r\n 4: 4CH VIP LVDS capture + SGX DISPLAY (Mosaic, A15)"
    "\r\n 5: 4CH VIP LVDS capture + 3D SRV (SGX/A15) + SGX/DRM DISPLAY(A15) - Only HDMI 1080p display supported"
    "\r\n 6: 2CH OV490 2560x720 capture + Split + 3D SRV (SGX/A15) + SGX/DRM DISPLAY(A15) - Only HDMI 1080p display supported"
    "\r\n "
    "\r\n AVB RX Use-cases, (TDA2x ONLY)"
    "\r\n ------------------------------"
    "\r\n 7: 4CH AVB Capture + Decode + SGX DISPLAY (Mosaic, A15)"
    "\r\n "
    "\r\n p: CPU Status"
    "\r\n i: Show IP Addr (IPU + NDK + AVB) "
    "\r\n "
    "\r\n d: Display Settings "
    "\r\n "
    "\r\n x: Exit "
    "\r\n z: Exit - AND SHUTDOWN Remote CPUs"
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
    "\r\n 1: LCD  7-inch 800x480@60fps"
    "\r\n 2: HDMI 1080P60 "
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

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
    "\r\n p: Print Performance Statistics "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

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
        ch = getchar();

        switch(ch)
        {
            case '1':
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_LCD_7_INCH;
                displaySelectDone = TRUE;
                break;
            case '2':
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_1080P;
                displaySelectDone = TRUE;
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


/**
 *******************************************************************************
 *
 * \brief   Application's main - entry function
 *
 *******************************************************************************
*/
Int32 main (Int32 argc, Char ** argv)
{
    Char ch;
    Bool shutdownRemoteCores = FALSE;

    System_init();
    ChainsCommon_appCtrlCommonInit();
    Chains_Ctrl_Init(&gChains_usecaseCfg);

    /* allow remote prints to complete, before showing main menu */
    OSA_waitMsecs(500);

    while(1)
    {
        Vps_printf(gChains_testMenu);
        ch = getchar();

        if((ch=='x') || (ch=='X'))
            break;

        if((ch=='z') || (ch=='Z'))
        {
            shutdownRemoteCores = TRUE;
            break;
        }

        switch(ch)
        {
            case '1':
                chains_vipSingleCam_SgxDisplay(&gChains_usecaseCfg);
                break;
            case '2':
                chains_vipSingleCam_Enc_Dec_SgxDisplay(&gChains_usecaseCfg);
                break;
            case '3':
                chains_vipSingleCamAnalytics_SgxDisplay(&gChains_usecaseCfg);
                break;
            case '4':
                gChains_usecaseCfg.numLvdsCh = 4;
                gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_OV10635;
                chains_lvdsVipMultiCam_SgxDisplay(&gChains_usecaseCfg);
                break;
            case '5':
                gChains_usecaseCfg.numLvdsCh = 4;
                gChains_usecaseCfg.svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_3D;
                gChains_usecaseCfg.enableCarOverlayInAlg = 0;
                gChains_usecaseCfg.captureSrc = CHAINS_CAPTURE_SRC_OV10635;
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_1080P;
                chains_lvdsVipMultiCam_Sgx3Dsrv(&gChains_usecaseCfg);
                break;
            case '6':
                gChains_usecaseCfg.numLvdsCh = 4;
                gChains_usecaseCfg.svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_3D;
                gChains_usecaseCfg.enableCarOverlayInAlg = 0;
                gChains_usecaseCfg.displayType = CHAINS_DISPLAY_TYPE_HDMI_1080P;
                chains_ov490VipMultiCam_Sgx3Dsrv(&gChains_usecaseCfg);
                break;
            case '7':
                gChains_usecaseCfg.numLvdsCh = 4;
                Chains_AvbRxDecodeSgxDisplay(&gChains_usecaseCfg);
                break;
            case 'P':
            case 'p':
                ChainsCommon_printCpuStatus();
                break;
            case 'I':
            case 'i':
                if(System_isProcEnabled(SYSTEM_PROC_IPU1_0))
                {
                    SystemCommon_IpAddr ipuIpAddr;
                    System_linkControl(
                        SYSTEM_LINK_ID_IPU1_0,
                        SYSTEM_COMMON_CMD_GET_IP_ADDR,
                        &ipuIpAddr,
                        sizeof(SystemCommon_IpAddr),
                        TRUE
                    );
                    Vps_printf(" Network IP address : %s\n", ipuIpAddr.ipAddr);
                }
                break;
            case 'D':
            case 'd':
                Chains_showDisplaySettingsMenu();
                break;
        }
    }
    ChainsCommon_appCtrlCommonDeInit();
    System_deInit(shutdownRemoteCores);

    return 0;
}

