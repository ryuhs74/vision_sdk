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


static char usecase_menu[] = {
    "\r\n "
    "\r\n Select input for use-case,"
    "\r\n --------------------------"
    "\r\n 1: Capture via VIP Port (HDMI)"
    "\r\n 2: Capture via Network"
    "\r\n "
    "\r\n x: Exit"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

Void Chains_singleCameraAnalyticsTda2xx(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;

    chainsCfg->captureSrc = CHAINS_CAPTURE_SRC_HDMI_1080P;

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
                Chains_vipSingleCameraAnalyticsTda2xx(chainsCfg);
                break;
            case '2':
                Chains_networkRxCameraAnalyticsTda2xx(chainsCfg);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }
}

