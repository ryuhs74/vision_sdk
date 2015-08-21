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
#include "chains_networkTxMultiCamCapture_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

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

    chains_networkTxMultiCamCaptureObj ucObj;

    UInt32  netProcId;

    Chains_Ctrl *chainsCfg;

} Chains_NetworkTxCaptureAppObj;

/**
 *******************************************************************************
 * \brief   Set create parameters for NullSrc
 *******************************************************************************
*/
Void chains_networkTxMultiCamCapture_SetNullPrms(Chains_NetworkTxCaptureAppObj *pObj,
                        NullLink_CreateParams *pPrm)
{
    pPrm->dumpDataType = NULL_LINK_COPY_TYPE_NETWORK;
    pPrm->networkServerPort = NETWORK_TX_SERVER_PORT;
}

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
Void chains_networkTxMultiCamCapture_SetAppPrms(chains_networkTxMultiCamCaptureObj *pUcObj, Void *appObj)
{
    Chains_NetworkTxCaptureAppObj *pObj
        = (Chains_NetworkTxCaptureAppObj*)appObj;

    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];
    UInt32 i, numLvdsCh = 4;
    UInt32 frameSkipMask = 0x2AAAAAAA;

    pUcObj->IPCIn_A15_0_IPU1_0_0LinkID
        = SYSTEM_MAKE_LINK_ID(
            pObj->netProcId,
            pUcObj->IPCIn_A15_0_IPU1_0_0LinkID);

    pUcObj->NullLinkID
        = SYSTEM_MAKE_LINK_ID(
            pObj->netProcId,
            pUcObj->NullLinkID);

    ChainsCommon_MultiCam_StartCaptureDevice(
        CHAINS_CAPTURE_SRC_OV10635,
        portId,
        numLvdsCh
        );

    ChainsCommon_MultiCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            portId,
            numLvdsCh
            );

    for(i=0; i<numLvdsCh; i++)
    {
        pUcObj->CapturePrm.vipInst[i].outParams[0].frameSkipMask
            = frameSkipMask;
    }


    chains_networkTxMultiCamCapture_SetNullPrms(
        pObj,
        &pUcObj->NullPrm
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
 * \param   pObj  [IN] Chains_NetworkTxCaptureObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_networkTxMultiCamCapture_StartApp(Chains_NetworkTxCaptureAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    chains_networkTxMultiCamCapture_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_NetworkTxCaptureObj
 *
 *******************************************************************************
*/
Void chains_networkTxMultiCamCapture_StopAndDeleteApp(Chains_NetworkTxCaptureAppObj *pObj)
{
    chains_networkTxMultiCamCapture_Stop(&pObj->ucObj);
    chains_networkTxMultiCamCapture_Delete(&pObj->ucObj);

    ChainsCommon_StopCaptureDevice(CHAINS_CAPTURE_SRC_OV10635);

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
Void Chains_networkTxMultiCamCapture(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_NetworkTxCaptureAppObj chainsObj;

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

    chains_networkTxMultiCamCapture_Create(&chainsObj.ucObj, &chainsObj);

    chains_networkTxMultiCamCapture_StartApp(&chainsObj);

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
                chains_networkTxMultiCamCapture_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_networkTxMultiCamCapture_StopAndDeleteApp(&chainsObj);
}

