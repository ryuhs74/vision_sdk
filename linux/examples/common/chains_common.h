/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *
 * \file chains_common.h
 *
 * \brief This file contains common utility functions used by all use-cases
 *
 *******************************************************************************
 */

#ifndef _CHAINS_COMMON_H_
#define _CHAINS_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <linux/src/osa/include/osa.h>

#include <examples/tda2xx/include/draw2d.h> // TBD : Remove dependency on bios examples
#include <examples/tda2xx/include/chains_main_srv_calibration.h>

#include <include/link_api/system.h>
#include <include/link_api/captureLink.h>
#include <include/link_api/vpeLink.h>
#include <include/link_api/syncLink.h>
#include <include/link_api/selectLink.h>
#include <include/link_api/nullLink.h>
#include <include/link_api/sgxDisplayLink.h>
#include <include/link_api/sgx3DsrvLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/algorithmLink_frameCopy.h>
#include <include/link_api/algorithmLink_edgeDetection.h>
#include <include/link_api/algorithmLink_dmaSwMs.h>
#include <include/link_api/algorithmLink_geometricAlignment.h>
#include <include/link_api/algorithmLink_photoAlignment.h>
#include <include/link_api/algorithmLink_synthesis.h>
#include <include/link_api/algorithmLink_denseOpticalFlow.h>
#include <include/link_api/algorithmLink_vectorToImage.h>
#include <include/link_api/algorithmLink_featurePlaneComputation.h>
#include <include/link_api/algorithmLink_objectDetection.h>
#include <include/link_api/algorithmLink_sparseOpticalFlow.h>
#include <include/link_api/algorithmLink_sparseOpticalFlowDraw.h>
#include <include/link_api/algorithmLink_objectDraw.h>
#include <include/link_api/algorithmLink_laneDetect.h>
#include <include/link_api/algorithmLink_laneDetectDraw.h>
#include <include/link_api/ultrasonicCaptureLink.h>
#include <include/link_api/grpxSrcLink.h>
#include <include/link_api/displayLink.h>
#include <include/link_api/dupLink.h>
#include <include/link_api/mergeLink.h>
#include <include/link_api/avbRxLink.h>
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <include/link_api/displayCtrlLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/decLink.h>
#include <include/link_api/encLink.h>
#include <linux/examples/common/appCtrlLink.h>
/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
/**
 *******************************************************************************
 *
 * \brief Maximum Number of LVDS cameras supported by the board
 *
 *******************************************************************************
 */
#define VIDEO_SENSOR_MAX_LVDS_CAMERAS   (6)
#define LVDS_CAPTURE_WIDTH              (1280)
#define LVDS_CAPTURE_HEIGHT             (720)

#define SV_OUTPUT_WIDTH                 (880) //(720)
#define SV_OUTPUT_HEIGHT                (1080)
#define SV_OUTPUT_POSX                  (25+320+10)
#define SV_OUTPUT_POSY                  (0)
#define SV_INPUT_WIDTH                  (LVDS_CAPTURE_WIDTH)
#define SV_INPUT_HEIGHT                 (LVDS_CAPTURE_HEIGHT)
#define SV_NUM_VIEWS                    (4)

#define SV_CARBOX_WIDTH                 (190)
#define SV_CARBOX_HEIGHT                (360)

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define TIGHT_SYNC_DELTA_IN_MSEC              (16)
#define LOOSE_SYNC_DELTA_IN_MSEC              (0x7FFFFFFF)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define TIGHT_SYNC_DROP_THRESHOLD_IN_MSEC     (33)
#define LOOSE_SYNC_DROP_THRESHOLD_IN_MSEC     (0x7FFFFFFF)


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

 typedef enum {
     CHAINS_DISPLAY_TYPE_LCD_7_INCH = 0,
     /**< Output displayed on 7 inch LCD */
     CHAINS_DISPLAY_TYPE_HDMI_1080P
     /**< Output displayed on HDMI in 1080P resolution */
 } Chains_DisplayType;

 typedef enum {
     CHAINS_CAPTURE_SRC_OV10635 = 0,
     /**< Capture source is OV 10635 */
     CHAINS_CAPTURE_SRC_HDMI_720P,
     /**< Capture source is HDMI in 720P resolution */
     CHAINS_CAPTURE_SRC_HDMI_1080P,
     /**< Capture source is HDMI in 1080P resolution */
     CHAINS_CAPTURE_SRC_AR0132RCCC,
     /**< Capture source is Aptina AR0132 RCCC sensor */
     CHAINS_CAPTURE_SRC_AR0132ISP
     /**< Capture source is Aptina AR0132 BAYER + External ISP sensor */
 } Chains_CaptureSrc;


/*******************************************************************************
 *  Function's
 *******************************************************************************
 */
Void ChainsCommon_SingleCam_SetCapturePrms(
                        CaptureLink_CreateParams *pPrm,
                        UInt32 captureInWidth,
                        UInt32 captureInHeight,
                        UInt32 captureOutWidth,
                        UInt32 captureOutHeight,
                        Chains_CaptureSrc captureSrc
                        );

Void ChainsCommon_MultiCam_SetCapturePrms(
                        CaptureLink_CreateParams *pPrm,
                        UInt32 numLvdsCh);

char ChainsCommon_SurroundView_MenuCalibration(
                  Chain_Common_SRV_CalibParams * gaCalibPrm);
Void ChainsCommon_SurroundView_InitCalibration(
                  Chain_Common_SRV_CalibParams * gaCalibPrm,
                  Bool startWithCalibration);

Void ChainsCommon_SurroundView_StopCalibration(
                  Chain_Common_SRV_CalibParams * gaCalibPrm);


Void ChainsCommon_3DSurroundView_SetParams(
                            CaptureLink_CreateParams *pVipCapture,
                            AvbRxLink_CreateParams *pAvbRxPrm,
                            DecLink_CreateParams *pDecPrm,
                            SelectLink_CreateParams *pCaptureSelect,
                            VpeLink_CreateParams *pSvOrgVpe,
                            VpeLink_CreateParams *pFrontCamVpe,
                            SyncLink_CreateParams *pSvSync,
                            SyncLink_CreateParams *pSvOrgSync,
                            SyncLink_CreateParams *pFrontCamSync,
                            AlgorithmLink_SynthesisCreateParams *pSynthPrm,
                            AlgorithmLink_GAlignCreateParams *pGAlignPrm,
                            AlgorithmLink_PAlignCreateParams *pPAlignPrm,
                            AlgorithmLink_EdgeDetectionCreateParams *pEdgeDetect,
                            AlgorithmLink_DenseOptFlowCreateParams *pDof,
                            AlgorithmLink_VectorToImageCreateParams *pDofVectorToImage,
                            AlgorithmLink_DmaSwMsCreateParams *pSvOrgDmaSwMs,
                            AlgorithmLink_DmaSwMsCreateParams *pFrontCamDmaSwMs,
                            GrpxSrcLink_CreateParams *pGrpxSrc,
                            DisplayLink_CreateParams *pSvDisplay,
                            DisplayLink_CreateParams *pSvOrgDisplay,
                            DisplayLink_CreateParams *pFrontCamDisplay,
                            DisplayLink_CreateParams *pGrpxDisplay,
                            Chains_DisplayType displayType,
                            UInt32 numLvdsCh,
                            AlgorithmLink_SrvOutputModes svOutputMode,
                            VpeLink_CreateParams *VPE_algPdPrm,
                            AlgorithmLink_FeaturePlaneComputationCreateParams *Alg_FeaturePlaneComputationPrm,
                            AlgorithmLink_ObjectDetectionCreateParams *Alg_ObjectDetectionPrm,
                            SyncLink_CreateParams *Sync_algPdPrm,
                            AlgorithmLink_ObjectDrawCreateParams *Alg_ObjectDrawPrm,
                            SyncLink_CreateParams *Sync_algLdPrm,
                            AlgorithmLink_LaneDetectCreateParams *Alg_LaneDetectPrm,
                            AlgorithmLink_LaneDetectDrawCreateParams *Alg_LaneDetectDrawPrm,
                            Chain_Common_SRV_CalibParams *gaCalibPrm,
                            Bool enableCarOverlayInAlg
                            );

Void ChainsCommon_GetDisplayWidthHeight(
                            Chains_DisplayType displayType,
                            UInt32 *displayWidth,
                            UInt32 *displayHeight
                            );

/**
 *******************************************************************************
 *
 * \brief   Read a charater from UART or CCS console
 *
 * \return character that is read
 *
 *******************************************************************************
*/
Char Chains_readChar();
/**
 *******************************************************************************
 *
 * \brief   Load Calculation enable/disable
 *
 *          This functions enables load profiling. A control command
 *          SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START is passed to chianed links.
 *          If parameter Enable is set true Load profiling is enabled.
 *          If printStatus is set true a System CMD to Print CPU load,
 *          Task Laod and Heap status information is sent
 *          While creating enable = TRUE , printStatus & printTskLoad = FALSE
 *          While deleting enable = FALSE , printStatus & printTskLoad = TRUE
 *
 * \param   enable               [IN]   is set true Load profiling
 *
 * \param   printStatus          [IN] true a System CMD
 *
 * \param   printTskLoad         [IN]  true a Print CPU load
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Chains_prfLoadCalcEnable(Bool enable,
                               Bool printStatus,
                               Bool printTskLoad);

/**
 *******************************************************************************
 *
 * \brief   Print Memory Heap Statistics
 *
 *          This function send a system control message
 *           to all cores.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_memPrintHeapStatus();
Void ChainsCommon_PrintStatistics();
/**
 *******************************************************************************
 *
 * \brief   common APP CTRL Link commands for initialization
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 ChainsCommon_appCtrlCommonInit();

/**
 *******************************************************************************
 *
 * \brief   common APP CTRL Link commands for de intialization
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 ChainsCommon_appCtrlCommonDeInit();

Int32 ChainsCommon_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad);
Int32 ChainsCommon_prfCpuLoadPrint();
Int32 ChainsCommon_memPrintHeapStatus();
Int32 ChainsCommon_statCollectorPrint();
Int32 ChainsCommon_statCollectorReset();
void  ChainsCommon_printCpuStatus();
Void ChainsCommon_SurroundView_SetSyncPrm(
                    SyncLink_CreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 syncMode
                    );


Void ChainsCommon_SurroundView_SetSynthParams(
                                    AlgorithmLink_SynthesisCreateParams *pPrm,
                                    UInt16 svInWidth,
                                    UInt16 svInHeight,
                                    UInt16 svOutWidth,
                                    UInt16 svOutHeight,
                                    UInt16 svNumViews,
                                    Int16  svCarBoxWidth,
                                    Int16  svCarBoxHeight,
                                    AlgorithmLink_SrvOutputModes svOutputMode,
                                    Bool enableCarOverlayInAlg);

Void ChainsCommon_SurroundView_SetPAlignParams(
                                    AlgorithmLink_PAlignCreateParams *pPrm,
                                    UInt16 svInWidth,
                                    UInt16 svInHeight,
                                    UInt16 svOutWidth,
                                    UInt16 svOutHeight,
                                    UInt16 svNumViews,
                                    Int16  svCarBoxWidth,
                                    Int16  svCarBoxHeight,
                                    AlgorithmLink_SrvOutputModes svOutputMode
                                    );
Void ChainsCommon_SurroundView_SetGAlignParams(
                            AlgorithmLink_GAlignCreateParams *pPrm,
                            UInt16 svInWidth,
                            UInt16 svInHeight,
                            UInt16 svOutWidth,
                            UInt16 svOutHeight,
                            UInt16 svNumViews,
                            Int16  svCarBoxWidth,
                            Int16  svCarBoxHeight,
                            AlgorithmLink_SrvOutputModes svOutputMode,
                            Chain_Common_SRV_CalibParams *gaCalibPrm);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

/* @} */

/**
 *******************************************************************************
 *
 *   \defgroup EXAMPLES_API Example code implementation
 *
 *******************************************************************************
 */

