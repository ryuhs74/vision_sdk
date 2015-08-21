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
#include "chains_networkRxDecDisplay_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define NETWORK_RX_FRAME_WIDTH      (1920)
#define NETWORK_RX_FRAME_HEIGHT     (1080)

/**
 *******************************************************************************
 *
 *  \brief  Use-case object
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_networkRxDecDisplayObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    UInt32  netProcId;

    Chains_Ctrl *chainsCfg;

} Chains_NetworkRxDecDisplayAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set link Parameters
 *
 *          It is called in Create function of the auto generated use-case file.
 *
 * \param pUcObj    [IN] Auto-generated usecase object
 * \param appObj    [IN] Application specific object
 *
 *******************************************************************************
*/
Void chains_networkRxDecDisplay_SetAppPrms(chains_networkRxDecDisplayObj *pUcObj, Void *appObj)
{
    Chains_NetworkRxDecDisplayAppObj *pObj
        = (Chains_NetworkRxDecDisplayAppObj*)appObj;

    pUcObj->IPCOut_A15_0_IPU1_0_0LinkID
        = SYSTEM_MAKE_LINK_ID(
            pObj->netProcId,
            pUcObj->IPCOut_A15_0_IPU1_0_0LinkID);

    pUcObj->NullSourceLinkID
        = SYSTEM_MAKE_LINK_ID(
            pObj->netProcId,
            pUcObj->NullSourceLinkID);

    pObj->captureOutWidth  = NETWORK_RX_FRAME_WIDTH;
    pObj->captureOutHeight = NETWORK_RX_FRAME_HEIGHT;

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SetNetworkRxPrms(
            &pUcObj->NullSourcePrm,
            &pUcObj->DecodePrm,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            1,
            30
        );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );

    ChainsCommon_SetDisplayPrms(&pUcObj->Display_VideoPrm,
                                &pUcObj->Display_GrpxPrm,
                                pObj->chainsCfg->displayType,
                                pObj->displayWidth,
                                pObj->displayHeight
                               );

    ChainsCommon_StartDisplayCtrl(
        pObj->chainsCfg->displayType,
        pObj->displayWidth,
        pObj->displayHeight
        );
}

/**
 *******************************************************************************
 *
 * \brief   Start the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] Chains_NetworkRxDecDisplayObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_networkRxDecDisplay_StartApp(Chains_NetworkRxDecDisplayAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    /*This is required to select Ethernet port0 device on J6Eco*/
#ifdef TDA2EX_BUILD
    Bsp_boardSelectDevice(BSP_DRV_ID_ENET_PHY_DP83865,BSP_DEVICE_ENET_PHY_DP83865_INST_ID_0);
#endif

    chains_networkRxDecDisplay_Start(&pObj->ucObj);

    Chains_prfLoadCalcEnable(TRUE, FALSE, FALSE);
}

/**
 *******************************************************************************
 *
 * \brief   Delete the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to delete all the prior created links
 *          System_linkDelete is called with LinkId to delete the links.
 *
 * \param   pObj   [IN]   Chains_NetworkRxDecDisplayObj
 *
 *******************************************************************************
*/
Void chains_networkRxDecDisplay_StopAndDeleteApp(Chains_NetworkRxDecDisplayAppObj *pObj)
{
    chains_networkRxDecDisplay_Stop(&pObj->ucObj);
    chains_networkRxDecDisplay_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();
    ChainsCommon_StopDisplayDevice(pObj->chainsCfg->displayType);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase function
 *
 *          This functions executes the create, start functions
 *
 *          Further in a while loop displays run time menu and waits
 *          for user inputs to print the statistics or to end the demo.
 *
 *          Once the user inputs end of demo stop and delete
 *          functions are executed.
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkRxDecDisplay(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_NetworkRxDecDisplayAppObj chainsObj;

    chainsObj.netProcId = Utils_netGetProcId();
    if(chainsObj.netProcId==System_getSelfProcId())
    {
        Vps_printf(" \n");
        Vps_printf(" CHAINS: ERROR: Networking/NDK MUST be run on different CPU"
                   " than IPU1-0.\n");
        Vps_printf(" CHAINS: ERROR: If you need to run this use-case with NDK"
                   " on IPU1-0 then regenerate the use-case with NullSrc/Null"
                   " links on IPU1-0.\n"
                   );
        Vps_printf(" \n");
        return;
    }

    chainsObj.chainsCfg = chainsCfg;

    chains_networkRxDecDisplay_Create(&chainsObj.ucObj, &chainsObj);

    chains_networkRxDecDisplay_StartApp(&chainsObj);

    while(!done)
    {
        ch = Chains_menuRunTime();

        switch(ch)
        {
            case '0':
                done = TRUE;
                break;
            case 'p':
            case 'P':
                ChainsCommon_PrintStatistics();
                chains_networkRxDecDisplay_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_networkRxDecDisplay_StopAndDeleteApp(&chainsObj);
}

