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
#include "chains_vipSingleCam_Display_priv.h"
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

    chains_vipSingleCam_DisplayObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraViewAppObj;

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
Void chains_vipSingleCam_Display_SetAppPrms(chains_vipSingleCam_DisplayObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraViewAppObj *pObj
        = (Chains_VipSingleCameraViewAppObj*)appObj;

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SingleCam_SetCapturePrms(&pUcObj->CapturePrm,
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    pUcObj->CapturePrm.vipInst[0].numBufs = 5;

    /*
     * This code snippet shows an example of allocating memory for
     * a link from within the use-case instead of from within the link
     *
     * This allows user's to potentially allocate memory statically outside of
     * link implementation and then pass the memory to the link during use-case
     * create.
     *
     * If user wants the link to allocate memory then dont set below parameters
     * <link create params>.memAllocInfo.memSize,
     * <link create params>.memAllocInfo.memAddr
     */
    /* memory for YUV420SP buffer's */
    pUcObj->CapturePrm.memAllocInfo.memSize =
       (   CAPTURE_SENSOR_WIDTH
        * CAPTURE_SENSOR_HEIGHT
        * pUcObj->CapturePrm.vipInst[0].numBufs
        * 3 ) / 2 ;

    pUcObj->CapturePrm.memAllocInfo.memAddr =
        (UInt32)Utils_memAlloc(
                UTILS_HEAPID_DDR_CACHED_SR,
                pUcObj->CapturePrm.memAllocInfo.memSize,
                128
            );
    UTILS_assert(pUcObj->CapturePrm.memAllocInfo.memAddr!=NULL);

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidth,
                                               pObj->displayHeight
                                              );

    pUcObj->GrpxSrcPrm.grpxBufInfo.dataFormat = SYSTEM_DF_BGRA16_4444;

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
 * \param   pObj  [IN] Chains_VipSingleCameraViewObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_Display_StartApp(Chains_VipSingleCameraViewAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCam_Display_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraViewObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_Display_StopAndDeleteApp(Chains_VipSingleCameraViewAppObj *pObj)
{
    chains_vipSingleCam_Display_Stop(&pObj->ucObj);
    chains_vipSingleCam_Display_Delete(&pObj->ucObj);

    if(pObj->ucObj.CapturePrm.memAllocInfo.memAddr)
    {
        Int32 status;

        status = Utils_memFree(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    (Ptr)pObj->ucObj.CapturePrm.memAllocInfo.memAddr,
                    pObj->ucObj.CapturePrm.memAllocInfo.memSize);
        UTILS_assert(status==0);
    }

    ChainsCommon_StopDisplayCtrl();
    ChainsCommon_StopCaptureDevice(pObj->chainsCfg->captureSrc);
    ChainsCommon_StopDisplayDevice(pObj->chainsCfg->displayType);

    /* Print the HWI, SWI and all tasks load */
    /* Reset the accumulated timer ticks */
    Chains_prfLoadCalcEnable(FALSE, TRUE, TRUE);
}

/**
 *******************************************************************************
 * \brief Run Time Menu string.
 *******************************************************************************
 */
char gChains_vipSingleCam_runTimeMenu[] = {
    "\r\n "
    "\r\n ===================="
    "\r\n Chains Run-time Menu"
    "\r\n ===================="
    "\r\n "
    "\r\n 0: Stop Chain"
    "\r\n "
    "\r\n 2: Pause Capture"
    "\r\n 3: Resume Capture"
    "\r\n "
    "\r\n p: Print Performance Statistics "
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

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
Void Chains_vipSingleCam_Display(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraViewAppObj chainsObj;
    Int32 status;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCam_Display_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCam_Display_StartApp(&chainsObj);

    while(!done)
    {
        Vps_printf(gChains_vipSingleCam_runTimeMenu);

        ch = Chains_readChar();

        switch(ch)
        {
            case '0':
                done = TRUE;
                break;

            case '2':
                status = System_linkStop(chainsObj.ucObj.CaptureLinkID);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                break;

            case '3':
                status = System_linkStart(chainsObj.ucObj.CaptureLinkID);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
                break;

            case 'p':
            case 'P':
                ChainsCommon_PrintStatistics();
                chains_vipSingleCam_Display_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCam_Display_StopAndDeleteApp(&chainsObj);
}

