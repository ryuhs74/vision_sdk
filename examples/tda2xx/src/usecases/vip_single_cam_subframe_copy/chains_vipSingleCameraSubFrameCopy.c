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
#include "chains_vipSingleCameraSubFrameCopy_priv.h"
#include <examples/tda2xx/include/chains_common.h>


#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)

/**
 *******************************************************************************
 *
 *  \brief  SingleCameraSubFrameCopyObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipSingleCameraSubFrameCopyObj ucObj;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;
    UInt32  displayWidth;
    UInt32  displayHeight;

    Chains_Ctrl *chainsCfg;

} Chains_VipSingleCameraSubFrameCopyAppObj;

/**
 *******************************************************************************
 *
 * \brief   Set Capture Create Parameters
 *
 *          This function is used to set the Capture params.specific to subframe
 *
 * \param   pPrm         [IN]    CaptureLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraSubFrameCopy_SetCapturePrms(
                            CaptureLink_CreateParams *pPrm)
{
    UInt32 i, streamId;

    CaptureLink_VipInstParams *pInstPrm;
    CaptureLink_OutParams *pOutprms;

    for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
    {
        pInstPrm = &pPrm->vipInst[i];
        pInstPrm->bufCaptMode   =   SYSTEM_CAPT_BCM_CIRCULAR_FRM_REPEAT;
        for (streamId = 0; streamId < CAPTURE_LINK_MAX_OUTPUT_PER_INST;
                streamId++)
        {
            pOutprms = &pInstPrm->outParams[streamId];
            pOutprms->scEnable = TRUE;
            pOutprms->subFrmPrms.subFrameEnable = TRUE;
            pOutprms->subFrmPrms.numLinesPerSubFrame = 60;
        }
        pInstPrm->numBufs = 4;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId.
 *          Number of input buffers required by alg are also set here.
 *
 *
 * \param   pPrm    [IN]    AlgorithmLink_SubframeCopyCreateParams
 * \param   chainsCfg    [IN]    Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraSubFrameCopy_SetSubFrameCopyAlgPrms
        ( AlgorithmLink_SubframeCopyCreateParams *pPrm,Chains_Ctrl *chainsCfg)
{
    pPrm->numBufs = 4;
    pPrm->inChannelId= 0;
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
Void chains_vipSingleCameraSubFrameCopy_SetAppPrms
                (chains_vipSingleCameraSubFrameCopyObj *pUcObj, Void *appObj)
{
    Chains_VipSingleCameraSubFrameCopyAppObj *pObj
        = (Chains_VipSingleCameraSubFrameCopyAppObj*)appObj;

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
    chains_vipSingleCameraSubFrameCopy_SetCapturePrms(&pUcObj->CapturePrm);

    if(pObj->chainsCfg->captureSrc==CHAINS_CAPTURE_SRC_HDMI_720P
        ||
       pObj->chainsCfg->captureSrc==CHAINS_CAPTURE_SRC_HDMI_1080P
        ||
       pObj->chainsCfg->captureSrc==CHAINS_CAPTURE_SRC_DM388

    )
    {
        /* dont skip alternate frames, keep it 60fps  */
        pUcObj->CapturePrm.vipInst[0].outParams[0].frameSkipMask
            = 0;
    }

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

    chains_vipSingleCameraSubFrameCopy_SetSubFrameCopyAlgPrms
                            (&pUcObj->Alg_SubframeCopyPrm, pObj->chainsCfg);
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
 * \param   pObj  [IN] Chains_VipSingleCameraSubFrameCopyAppObj
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraSubFrameCopy_StartApp(Chains_VipSingleCameraSubFrameCopyAppObj *pObj)
{
    Chains_memPrintHeapStatus();

    ChainsCommon_StartDisplayDevice(pObj->chainsCfg->displayType);

    ChainsCommon_StartCaptureDevice(
        pObj->chainsCfg->captureSrc,
        pObj->captureOutWidth,
        pObj->captureOutHeight
        );

    chains_vipSingleCameraSubFrameCopy_Start(&pObj->ucObj);

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
 * \param   pObj   [IN]   Chains_VipSingleCameraSubFrameCopyAppObj
 *
 *******************************************************************************
*/
Void chains_vipSingleCameraSubFrameCopy_StopAndDeleteApp(Chains_VipSingleCameraSubFrameCopyAppObj *pObj)
{
    chains_vipSingleCameraSubFrameCopy_Stop(&pObj->ucObj);
    chains_vipSingleCameraSubFrameCopy_Delete(&pObj->ucObj);

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
Void Chains_vipSingleCameraSubFrameCopy(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    Chains_VipSingleCameraSubFrameCopyAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCameraSubFrameCopy_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCameraSubFrameCopy_StartApp(&chainsObj);

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
                chains_vipSingleCameraSubFrameCopy_printStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCameraSubFrameCopy_StopAndDeleteApp(&chainsObj);

}

