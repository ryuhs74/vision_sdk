/*
*******************************************************************************
*
* Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*
*******************************************************************************
*/

/*
*******************************************************************************
*
* IMPORTANT NOTE:
*  This file is AUTO-GENERATED by Vision SDK use case generation tool
*
*******************************************************************************
*/
#include "chains_vipSingleCameraEdgeDetection_priv.h"
Void chains_vipSingleCameraEdgeDetection_SetLinkId(chains_vipSingleCameraEdgeDetectionObj *pObj){
       pObj->CaptureLinkID                  = SYSTEM_LINK_ID_CAPTURE;
       pObj->IPCOut_IPU1_0_EVE1_0LinkID     = IPU1_0_LINK (SYSTEM_LINK_ID_IPC_OUT_0);
       pObj->IPCIn_EVE1_IPU1_0_0LinkID      = EVE1_LINK (SYSTEM_LINK_ID_IPC_IN_0);
       pObj->Alg_EdgeDetectLinkID           = EVE1_LINK (SYSTEM_LINK_ID_ALG_0);
       pObj->IPCOut_EVE1_IPU1_0_0LinkID     = EVE1_LINK (SYSTEM_LINK_ID_IPC_OUT_0);
       pObj->IPCIn_IPU1_0_EVE1_0LinkID      = IPU1_0_LINK (SYSTEM_LINK_ID_IPC_IN_0);
       pObj->Display_VideoLinkID            = SYSTEM_LINK_ID_DISPLAY_0;
       pObj->GrpxSrcLinkID                  = IPU1_0_LINK (SYSTEM_LINK_ID_GRPX_SRC_0);
       pObj->Display_GrpxLinkID             = SYSTEM_LINK_ID_DISPLAY_1;
}

Void chains_vipSingleCameraEdgeDetection_ResetLinkPrms(chains_vipSingleCameraEdgeDetectionObj *pObj){
       CaptureLink_CreateParams_Init(&pObj->CapturePrm);
       IpcLink_CreateParams_Init(&pObj->IPCOut_IPU1_0_EVE1_0Prm);
       IpcLink_CreateParams_Init(&pObj->IPCIn_EVE1_IPU1_0_0Prm);
       IpcLink_CreateParams_Init(&pObj->IPCOut_EVE1_IPU1_0_0Prm);
       IpcLink_CreateParams_Init(&pObj->IPCIn_IPU1_0_EVE1_0Prm);
       DisplayLink_CreateParams_Init(&pObj->Display_VideoPrm);
       GrpxSrcLink_CreateParams_Init(&pObj->GrpxSrcPrm);
       DisplayLink_CreateParams_Init(&pObj->Display_GrpxPrm);
}

Void chains_vipSingleCameraEdgeDetection_SetPrms(chains_vipSingleCameraEdgeDetectionObj *pObj){
       (pObj->Alg_EdgeDetectPrm).baseClassCreate.size  = sizeof(AlgorithmLink_EdgeDetectionCreateParams);
       (pObj->Alg_EdgeDetectPrm).baseClassCreate.algId  = ALGORITHM_LINK_EVE_ALG_EDGEDETECTION;
}

Void chains_vipSingleCameraEdgeDetection_ConnectLinks(chains_vipSingleCameraEdgeDetectionObj *pObj){

       //Capture -> IPCOut_IPU1_0_EVE1_0
       pObj->CapturePrm.outQueParams.nextLink = pObj->IPCOut_IPU1_0_EVE1_0LinkID;
       pObj->IPCOut_IPU1_0_EVE1_0Prm.inQueParams.prevLinkId = pObj->CaptureLinkID;
       pObj->IPCOut_IPU1_0_EVE1_0Prm.inQueParams.prevLinkQueId = 0;

       //IPCOut_IPU1_0_EVE1_0 -> IPCIn_EVE1_IPU1_0_0
       pObj->IPCOut_IPU1_0_EVE1_0Prm.outQueParams.nextLink = pObj->IPCIn_EVE1_IPU1_0_0LinkID;
       pObj->IPCIn_EVE1_IPU1_0_0Prm.inQueParams.prevLinkId = pObj->IPCOut_IPU1_0_EVE1_0LinkID;
       pObj->IPCIn_EVE1_IPU1_0_0Prm.inQueParams.prevLinkQueId = 0;

       //IPCIn_EVE1_IPU1_0_0 -> Alg_EdgeDetect
       pObj->IPCIn_EVE1_IPU1_0_0Prm.outQueParams.nextLink = pObj->Alg_EdgeDetectLinkID;
       pObj->Alg_EdgeDetectPrm.inQueParams.prevLinkId = pObj->IPCIn_EVE1_IPU1_0_0LinkID;
       pObj->Alg_EdgeDetectPrm.inQueParams.prevLinkQueId = 0;

       //Alg_EdgeDetect -> IPCOut_EVE1_IPU1_0_0
       pObj->Alg_EdgeDetectPrm.outQueParams.nextLink = pObj->IPCOut_EVE1_IPU1_0_0LinkID;
       pObj->IPCOut_EVE1_IPU1_0_0Prm.inQueParams.prevLinkId = pObj->Alg_EdgeDetectLinkID;
       pObj->IPCOut_EVE1_IPU1_0_0Prm.inQueParams.prevLinkQueId = 0;

       //IPCOut_EVE1_IPU1_0_0 -> IPCIn_IPU1_0_EVE1_0
       pObj->IPCOut_EVE1_IPU1_0_0Prm.outQueParams.nextLink = pObj->IPCIn_IPU1_0_EVE1_0LinkID;
       pObj->IPCIn_IPU1_0_EVE1_0Prm.inQueParams.prevLinkId = pObj->IPCOut_EVE1_IPU1_0_0LinkID;
       pObj->IPCIn_IPU1_0_EVE1_0Prm.inQueParams.prevLinkQueId = 0;

       //IPCIn_IPU1_0_EVE1_0 -> Display_Video
       pObj->IPCIn_IPU1_0_EVE1_0Prm.outQueParams.nextLink = pObj->Display_VideoLinkID;
       pObj->Display_VideoPrm.inQueParams.prevLinkId = pObj->IPCIn_IPU1_0_EVE1_0LinkID;
       pObj->Display_VideoPrm.inQueParams.prevLinkQueId = 0;

       //GrpxSrc -> Display_Grpx
       pObj->GrpxSrcPrm.outQueParams.nextLink = pObj->Display_GrpxLinkID;
       pObj->Display_GrpxPrm.inQueParams.prevLinkId = pObj->GrpxSrcLinkID;
       pObj->Display_GrpxPrm.inQueParams.prevLinkQueId = 0;

}

Int32 chains_vipSingleCameraEdgeDetection_Create(chains_vipSingleCameraEdgeDetectionObj *pObj, Void *appObj){

       Int32 status;

       chains_vipSingleCameraEdgeDetection_SetLinkId(pObj);
       chains_vipSingleCameraEdgeDetection_ResetLinkPrms(pObj);

       chains_vipSingleCameraEdgeDetection_SetPrms(pObj);
       chains_vipSingleCameraEdgeDetection_SetAppPrms(pObj, appObj);

       chains_vipSingleCameraEdgeDetection_ConnectLinks(pObj);
       status = System_linkCreate(pObj->CaptureLinkID, &pObj->CapturePrm, sizeof(pObj->CapturePrm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->IPCOut_IPU1_0_EVE1_0LinkID, &pObj->IPCOut_IPU1_0_EVE1_0Prm, sizeof(pObj->IPCOut_IPU1_0_EVE1_0Prm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->IPCIn_EVE1_IPU1_0_0LinkID, &pObj->IPCIn_EVE1_IPU1_0_0Prm, sizeof(pObj->IPCIn_EVE1_IPU1_0_0Prm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->Alg_EdgeDetectLinkID, &pObj->Alg_EdgeDetectPrm, sizeof(pObj->Alg_EdgeDetectPrm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->IPCOut_EVE1_IPU1_0_0LinkID, &pObj->IPCOut_EVE1_IPU1_0_0Prm, sizeof(pObj->IPCOut_EVE1_IPU1_0_0Prm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->IPCIn_IPU1_0_EVE1_0LinkID, &pObj->IPCIn_IPU1_0_EVE1_0Prm, sizeof(pObj->IPCIn_IPU1_0_EVE1_0Prm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->Display_VideoLinkID, &pObj->Display_VideoPrm, sizeof(pObj->Display_VideoPrm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->GrpxSrcLinkID, &pObj->GrpxSrcPrm, sizeof(pObj->GrpxSrcPrm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkCreate(pObj->Display_GrpxLinkID, &pObj->Display_GrpxPrm, sizeof(pObj->Display_GrpxPrm));
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       return status;
}

Int32 chains_vipSingleCameraEdgeDetection_Start(chains_vipSingleCameraEdgeDetectionObj *pObj){

       Int32 status;

       status = System_linkStart(pObj->Display_GrpxLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->GrpxSrcLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->Display_VideoLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->IPCIn_IPU1_0_EVE1_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->IPCOut_EVE1_IPU1_0_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->Alg_EdgeDetectLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->IPCIn_EVE1_IPU1_0_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->IPCOut_IPU1_0_EVE1_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStart(pObj->CaptureLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       return status;
}

Int32 chains_vipSingleCameraEdgeDetection_Stop(chains_vipSingleCameraEdgeDetectionObj *pObj){

       Int32 status;

       status = System_linkStop(pObj->Display_GrpxLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->GrpxSrcLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->Display_VideoLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->IPCIn_IPU1_0_EVE1_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->IPCOut_EVE1_IPU1_0_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->Alg_EdgeDetectLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->IPCIn_EVE1_IPU1_0_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->IPCOut_IPU1_0_EVE1_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkStop(pObj->CaptureLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       return status;
}

Int32 chains_vipSingleCameraEdgeDetection_Delete(chains_vipSingleCameraEdgeDetectionObj *pObj){

       Int32 status;

       status = System_linkDelete(pObj->Display_GrpxLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->GrpxSrcLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->Display_VideoLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->IPCIn_IPU1_0_EVE1_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->IPCOut_EVE1_IPU1_0_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->Alg_EdgeDetectLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->IPCIn_EVE1_IPU1_0_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->IPCOut_IPU1_0_EVE1_0LinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       status = System_linkDelete(pObj->CaptureLinkID);
       UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

       return status;
}

Void chains_vipSingleCameraEdgeDetection_printBufferStatistics(chains_vipSingleCameraEdgeDetectionObj *pObj){
       System_linkPrintBufferStatistics(pObj->CaptureLinkID);
       System_linkPrintBufferStatistics(pObj->IPCOut_IPU1_0_EVE1_0LinkID);
       Task_sleep(500);
       System_linkPrintBufferStatistics(pObj->IPCIn_EVE1_IPU1_0_0LinkID);
       System_linkPrintBufferStatistics(pObj->Alg_EdgeDetectLinkID);
       System_linkPrintBufferStatistics(pObj->IPCOut_EVE1_IPU1_0_0LinkID);
       Task_sleep(500);
       System_linkPrintBufferStatistics(pObj->IPCIn_IPU1_0_EVE1_0LinkID);
       System_linkPrintBufferStatistics(pObj->Display_VideoLinkID);
       System_linkPrintBufferStatistics(pObj->GrpxSrcLinkID);
       System_linkPrintBufferStatistics(pObj->Display_GrpxLinkID);
       Task_sleep(500);
}

Void chains_vipSingleCameraEdgeDetection_printStatistics(chains_vipSingleCameraEdgeDetectionObj *pObj){
       System_linkPrintStatistics(pObj->CaptureLinkID);
       System_linkPrintStatistics(pObj->IPCOut_IPU1_0_EVE1_0LinkID);
       Task_sleep(500);
       System_linkPrintStatistics(pObj->IPCIn_EVE1_IPU1_0_0LinkID);
       System_linkPrintStatistics(pObj->Alg_EdgeDetectLinkID);
       System_linkPrintStatistics(pObj->IPCOut_EVE1_IPU1_0_0LinkID);
       Task_sleep(500);
       System_linkPrintStatistics(pObj->IPCIn_IPU1_0_EVE1_0LinkID);
       System_linkPrintStatistics(pObj->Display_VideoLinkID);
       System_linkPrintStatistics(pObj->GrpxSrcLinkID);
       System_linkPrintStatistics(pObj->Display_GrpxLinkID);
       Task_sleep(500);
}

