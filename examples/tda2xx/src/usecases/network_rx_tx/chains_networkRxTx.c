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

Void Chains_networkTxMultiCamCapture(Chains_Ctrl *chainsCfg);

static char usecase_menu[] = {
    "\r\n "
    "\r\n Select use-case,"
    "\r\n ----------------"
    "\r\n 1: Network RX + Display"
    "\r\n 2: Network RX + Decode + Display (TDA2x ONLY)"
    "\r\n 3: 1CH VIP Capture + Network TX"
    "\r\n 4: 1CH VIP Capture + Encode + Network TX (TDA2x ONLY)"
    "\r\n 5: 4CH VIP Capture + Network TX"
    "\r\n "
    "\r\n x: Exit"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

Void Chains_networkRxTx(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;

    if(!Bsp_platformIsTda2xxFamilyBuild())
    {
        Vps_printf(" ### Cannot run usecase on this platform\n");
    }

    while(!done)
    {
        Vps_printf(usecase_menu);

        ch = Chains_readChar();

        switch(ch)
        {
            case 'x':
                done = TRUE;
                break;
            case '1':
                Chains_networkRxDisplay(chainsCfg);
                break;
            case '2':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    Chains_networkRxDecDisplay(chainsCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase on this platform\n");
                }
                break;
            case '3':
                Chains_networkTxCapture(chainsCfg);
                break;
            case '4':
                if(Bsp_platformIsTda2xxFamilyBuild())
                {
                    Chains_networkTxEncCapture(chainsCfg);
                }
                else
                {
                    Vps_printf(" ### Cannot run usecase on this platform\n");
                }
                break;
            case '5':
                if(BSP_PLATFORM_SOC_ID_TDA2EX != Bsp_platformGetSocId())
                {
                 if(Board_isMultiDesConnected())
                 {
                     Chains_networkTxMultiCamCapture(chainsCfg);
                 }
                 else
                 {
                     Vps_printf(" ### Cannot run usecase on this platform\n");
                 }
                }
                else
                {
                     Vps_printf("Invalid option on TDA2EX, Network does not work on LVDS board\n");
                }
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
}

