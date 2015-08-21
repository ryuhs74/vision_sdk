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
#include "chains_vipSingleCam_DualDisplayEdgeDetection_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH    (1280)
#define CAPTURE_SENSOR_HEIGHT   (720)
#define HDMI_DISPLAY_WIDTH      (1920)
#define HDMI_DISPLAY_HEIGHT     (1080)
#define PIP_DISPLAY_WIDTH       (256u)
#define PIP_DISPLAY_HEIGHT      (120u)
#define PIP_OFFSET              (20u)

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

    chains_vipSingleCam_DualDisplayEdgeDetectionObj ucObj;

    UInt32                                  displayCtrlLinkId;
    DisplayCtrlLink_ConfigParams            dctrlCfgPrms;
    DisplayCtrlLink_OvlyParams              dctrlOvlyParamsLCD;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsLCD;
    DisplayCtrlLink_OvlyParams              dctrlOvlyParamsHDMI;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsHDMI;
    DisplayCtrlLink_OvlyParams              dctrlOvlyParamsPIP;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsPIP;
    DisplayCtrlLink_OvlyPipeParams          dctrlPipeOvlyParamsGRPX;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidthLCD;
    UInt32  displayHeightLCD;
    UInt32  displayWidthHDMI;
    UInt32  displayHeightHDMI;
    UInt32  displayWidthPIP;
    UInt32  displayHeightPIP;
    UInt32  offsetPIP;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraDualDisplayEdgeDetectionAppObj;

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
 * \param   pPrm    [IN]    AlgorithmLink_EdgeDetectionCreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplayEdgeDetection_SetEdgeDetectionAlgPrms(
                                     AlgorithmLink_EdgeDetectionCreateParams *pPrm,
                                     UInt32 maxWidth,
                                     UInt32 maxHeight)
{
    pPrm->baseClassCreate.size  = sizeof(AlgorithmLink_EdgeDetectionCreateParams);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_EDGEDETECTION;
    pPrm->maxWidth    = maxWidth;
    pPrm->maxHeight   = maxHeight;
    pPrm->numOutputFrames = 3;
}

/**
 *******************************************************************************
 *
 * \brief   Set Display Create Parameters
 *
 *          This function is used to set the Display params.
 *          It is called in Create function. It is advisable to have
 *          Chains_VipSingleCameraDualDisplay_ResetLinkPrms prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [IN]    DisplayLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplayEdgeDetection_SetDisplayPrms(
                                    DisplayLink_CreateParams *pPrm_HDMI,
                                    DisplayLink_CreateParams *pPrm_LCD,
                                    DisplayLink_CreateParams *pPrm_PIP,
                                    DisplayLink_CreateParams *pPrm_Grpx,
                                    UInt32 displayWidthHDMI,
                                    UInt32 displayHeightHDMI,
                                    UInt32 displayWidthLCD,
                                    UInt32 displayHeightLCD,
                                    UInt32 displayWidthPIP,
                                    UInt32 displayHeightPIP,
                                    UInt32 offsetPIP
                                    )
{
    if(pPrm_HDMI)
    {
        pPrm_HDMI->rtParams.tarWidth         = displayWidthHDMI;
        pPrm_HDMI->rtParams.tarHeight        = displayHeightHDMI;
        pPrm_HDMI->displayId                 = DISPLAY_LINK_INST_DSS_VID1;
    }
    else
    {

    }

    if(pPrm_LCD)
    {
        pPrm_LCD->rtParams.tarWidth         = displayWidthLCD;
        pPrm_LCD->rtParams.tarHeight        = displayHeightLCD;
        pPrm_LCD->displayId                 = DISPLAY_LINK_INST_DSS_VID2;
    }
    else
    {

    }

    if(pPrm_PIP)
    {
        pPrm_PIP->rtParams.tarWidth         = displayWidthPIP;
        pPrm_PIP->rtParams.tarHeight        = displayHeightPIP;
        pPrm_PIP->rtParams.posX             = offsetPIP;
        pPrm_PIP->rtParams.posY             = offsetPIP;
        pPrm_PIP->displayId                 = DISPLAY_LINK_INST_DSS_VID3;
    }
    else
    {

    }

    if(pPrm_Grpx)
    {
        pPrm_Grpx->displayId                  = DISPLAY_LINK_INST_DSS_GFX1;
    }
    else
    {

    }
}

/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          Chains_lvdsMultiVipCaptureDisplay_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *          Scaling parameters are set .
 *
 *          Scale each CH to 1/2x size
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplayEdgeDetection_SetVpePrm(
                    VpeLink_CreateParams *pPrm,
                    UInt32 factor,
                    UInt32 displayWidth,
                    UInt32 displayHeight
                    )
{
    UInt16 chId;

    pPrm->enableOut[0] = TRUE;

    chId = 0;
    pPrm->chParams[chId].outParams[0].width = SystemUtils_floor(displayWidth/factor, 16);

    pPrm->chParams[chId].outParams[0].height = displayHeight/factor;

    pPrm->chParams[chId].outParams[0].dataFormat = SYSTEM_DF_YUV420SP_UV;
    pPrm->chParams[chId].outParams[0].numBufsPerCh = 4;
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
Void chains_vipSingleCam_DualDisplayEdgeDetection_SetAppPrms(chains_vipSingleCam_DualDisplayEdgeDetectionObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraDualDisplayEdgeDetectionAppObj *pObj
        = (Chains_VipSingleCameraDualDisplayEdgeDetectionAppObj*)appObj;

    pObj->captureOutWidth   = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight  = CAPTURE_SENSOR_HEIGHT;
    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &pObj->displayWidthLCD,
        &pObj->displayHeightLCD
        );
    pObj->displayWidthHDMI  = HDMI_DISPLAY_WIDTH;
    pObj->displayHeightHDMI = HDMI_DISPLAY_HEIGHT;
    pObj->displayHeightPIP  = PIP_DISPLAY_HEIGHT;
    pObj->displayWidthPIP   = PIP_DISPLAY_WIDTH;
    pObj->offsetPIP         = PIP_OFFSET;

    ChainsCommon_SingleCam_SetCapturePrms(&pUcObj->CapturePrm,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                               pObj->displayWidthHDMI,
                                               pObj->displayHeightHDMI
                                              );

    chains_vipSingleCam_DualDisplayEdgeDetection_SetVpePrm(&pUcObj->VPEPrm,
                                                2,
                                                pObj->captureOutWidth,
                                                pObj->captureOutHeight
                                                );

    chains_vipSingleCam_DualDisplayEdgeDetection_SetDisplayPrms(&pUcObj->Display_HDMIPrm,
                                                &pUcObj->Display_LCDPrm,
                                                &pUcObj->Display_PIPPrm,
                                                &pUcObj->Display_GrpxPrm,
                                                pObj->displayWidthHDMI,
                                                pObj->displayHeightHDMI,
                                                pObj->displayWidthLCD,
                                                pObj->displayHeightLCD,
                                                pObj->displayWidthPIP,
                                                pObj->displayHeightPIP,
                                                pObj->offsetPIP
                                                );

    chains_vipSingleCam_DualDisplayEdgeDetection_SetEdgeDetectionAlgPrms(&pUcObj->Alg_EdgeDetectPrm, pObj->captureOutWidth, pObj->captureOutHeight);

    //The displayType indicates which LCD model it is
    ChainsCommon_DualDisplay_StartDisplayCtrl(pObj->chainsCfg->displayType,
                                pObj->displayWidthLCD, pObj->displayHeightLCD);
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
 * \param   pObj  [IN] Chains_VipSingleCameraDualDisplayObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplayEdgeDetection_StartApp(Chains_VipSingleCameraDualDisplayEdgeDetectionAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    /* Explicitly start LCD device as it has to be powered up */
    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCam_DualDisplayEdgeDetection_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraDualDisplayObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplayEdgeDetection_StopAndDeleteApp(Chains_VipSingleCameraDualDisplayEdgeDetectionAppObj *pObj)
{
    chains_vipSingleCam_DualDisplayEdgeDetection_Stop(&pObj->ucObj);
    chains_vipSingleCam_DualDisplayEdgeDetection_Delete(&pObj->ucObj);

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
Void chains_vipSingleCam_DualDisplayEdgeDetection(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraDualDisplayEdgeDetectionAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCam_DualDisplayEdgeDetection_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCam_DualDisplayEdgeDetection_StartApp(&chainsObj);

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
                chains_vipSingleCam_DualDisplayEdgeDetection_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCam_DualDisplayEdgeDetection_StopAndDeleteApp(&chainsObj);
}

