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
#include <examples/tda2xx/include/chains_common.h>
#include "chains_vipSingleCameraFullView_priv.h"
#include <src/utils_common/include/utils_lut.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

/**
 *******************************************************************************
 *
 *  \brief  SingleCameraFullViewObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipSingleCameraFullViewObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraFullViewAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_FullViewCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraFullView_SetFullViewAlgPrms(
                                    AlgorithmLink_FullViewCreateParams *pPrm,
                                      Chains_Ctrl *chainsCfg)
{
    if( chainsCfg->algProcId == SYSTEM_PROC_DSP1
             ||
             chainsCfg->algProcId == SYSTEM_PROC_DSP2)
             {
                 pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_FULLVIEW;
             }
    else if( chainsCfg->algProcId == SYSTEM_PROC_EVE1
             ||
             chainsCfg->algProcId == SYSTEM_PROC_EVE2
             ||
             chainsCfg->algProcId == SYSTEM_PROC_EVE3
             ||
             chainsCfg->algProcId == SYSTEM_PROC_EVE4)
            {
                pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_FULLVIEW;
            }
    else if(chainsCfg->algProcId == SYSTEM_PROC_A15_0)
            {
                pPrm->baseClassCreate.algId = ALGORITHM_LINK_A15_ALG_FULLVIEW;
            }

    pPrm->maxWidth    = CAPTURE_SENSOR_WIDTH;
    pPrm->maxHeight   = CAPTURE_SENSOR_HEIGHT;

    pPrm->numOutputFrames = 3;
    pPrm->pLut = LUTAlloc(Basic_frontFullView);
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
Void chains_vipSingleCameraFullView_SetAppPrms(chains_vipSingleCameraFullViewObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraFullViewAppObj *pObj
        = (Chains_VipSingleCameraFullViewAppObj*)appObj;

    /*
     * Use case is auto-generated assuming A15 as the frame copy alg link
     * But override the auto-generation to parameterize the CPU on which the
     * alg link runs
     *
     * Below code replaces A15 proc ID with 'algLinkProcId' which is set from
     * top level "chainsCfg"
     */
    pUcObj->IPCIn_A15_0_IPU1_0_0LinkID     =
        SYSTEM_MAKE_LINK_ID(pObj->chainsCfg->algProcId, pUcObj->IPCIn_A15_0_IPU1_0_0LinkID);

    pUcObj->Alg_FullViewLinkID            =
        SYSTEM_MAKE_LINK_ID(pObj->chainsCfg->algProcId, pUcObj->Alg_FullViewLinkID);

    pUcObj->IPCOut_A15_0_IPU1_0_0LinkID    =
        SYSTEM_MAKE_LINK_ID(pObj->chainsCfg->algProcId, pUcObj->IPCOut_A15_0_IPU1_0_0LinkID);

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;
    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidth,
        &pObj->displayHeight
        );

    ChainsCommon_SingleCam_SetCapturePrms(&(pUcObj->CapturePrm),
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
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

    chains_vipSingleCameraFullView_SetFullViewAlgPrms
                                                    (&pUcObj->Alg_FullViewPrm,
                                                            pObj->chainsCfg);
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
 * \param   pObj  [IN] Chains_VipSingleCameraFullViewAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraFullView_StartApp(Chains_VipSingleCameraFullViewAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCameraFullView_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraFullViewAppObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraFullView_StopAndDeleteApp(Chains_VipSingleCameraFullViewAppObj *pObj)
{
	LUTFree(Basic_frontFullView);
    chains_vipSingleCameraFullView_Stop(&pObj->ucObj);
    chains_vipSingleCameraFullView_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();
    ChainsCommon_StopCaptureDevice(pObj->chainsCfg->captureSrc);
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
Void Chains_vipSingleCameraFullView(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraFullViewAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCameraFullView_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCameraFullView_StartApp(&chainsObj);

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
                chains_vipSingleCameraFullView_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCameraFullView_StopAndDeleteApp(&chainsObj);

}

