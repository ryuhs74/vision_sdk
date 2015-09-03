/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \ingroup EXAMPLES_API
 * \defgroup EXAMPLES_CHAIN APIs for selecting the usecase chain.
 *
 * \brief  APIs for selecting the required usecase chain and run
 *         time menu configurations .
 *         It also provide API's for instrumentation of load and heap usage
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file chains.h
 *
 * \brief APIs for selecting the required usecase chain.
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _CHAINS_H_
#define _CHAINS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/captureLink.h>
#include <include/link_api/vpeLink.h>
#include <include/link_api/syncLink.h>
#include <include/link_api/selectLink.h>
#include <include/link_api/nullLink.h>
#include <include/link_api/nullSrcLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/algorithmLink_edgeDetection.h>
#include <include/link_api/algorithmLink_frameCopy.h>
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
#include <include/link_api/algorithmLink_ultrasonicFusion.h>
#include <include/link_api/algorithmLink_issAewb.h>
#include <include/link_api/displayLink.h>
#include <include/link_api/dupLink.h>
#include <include/link_api/splitLink.h>
#include <include/link_api/gateLink.h>
#include <include/link_api/mergeLink.h>
#include <include/link_api/avbRxLink.h>
#include <include/link_api/decLink.h>
#include <include/link_api/encLink.h>
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <include/link_api/displayCtrlLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/issCaptureLink.h>
#include <include/link_api/issM2mIspLink.h>
#include <include/link_api/issM2mSimcopLink.h>
#include <include/link_api/algorithmLink_objectDraw.h>
#include <include/link_api/grpxSrcLink.h>
#include <include/link_api/ultrasonicCaptureLink.h>
#include <include/link_api/algorithmLink_subframeCopy.h>
#include <include/link_api/algorithmLink_softIsp.h>
#include <include/link_api/algorithmLink_census.h>
#include <include/link_api/algorithmLink_stereoPostProcess.h>
#include <include/link_api/algorithmLink_disparityHamDist.h>
#include <include/link_api/algorithmLink_remapMerge.h>
#include <include/link_api/algorithmLink_laneDetect.h>
#include <include/link_api/algorithmLink_laneDetectDraw.h>

#include <examples/tda2xx/include/lcd.h>
#include <examples/tda2xx/include/board.h>
#include <examples/tda2xx/include/video_sensor.h>
#include <examples/tda2xx/include/hdmi_recvr.h>
#include <examples/tda2xx/include/hdmi_tx.h>
#include <examples/tda2xx/include/component.h>
#include <examples/tda2xx/include/draw2d.h>

#include <src/utils_common/include/utils_uart.h>
#include <src/utils_common/include/utils_qspi.h>
#include <src/utils_common/include/utils_dma.h>
#include <src/utils_common/include/utils_mem.h>
#include <src/utils_common/include/utils_l3_emif_bw.h>
#include <src/utils_common/include/utils_boot_slaves.h>

#include <ti/sysbios/knl/Task.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

 typedef enum {
	 CHAINS_DISPLAY_TYPE_CH7026_480P = 0,
     /**< Output displayed on 480P Component */

     CHAINS_DISPLAY_TYPE_LCD_10_INCH,
    /**< Output displayed on 10-inch LCD */

     CHAINS_DISPLAY_TYPE_HDMI_720P,
     /**< Output displayed on HDMI in 720P resolution */

     CHAINS_DISPLAY_TYPE_HDMI_1080P,
     /**< Output displayed on HDMI in 1080P resolution */

     CHAINS_DISPLAY_TYPE_SDTV_NTSC,
     /**< Output displayed on SD TV in NTSC format */

     CHAINS_DISPLAY_TYPE_SDTV_PAL,
     /**< Output displayed on SD TV in PAL format */

     CHAINS_DISPLAY_TYPE_HDMI_XGA_TDM
     /**< Output displayed on HDMI in XGA resolution using 8bit TDM mode */


 } Chains_DisplayType;

 typedef enum {
     CHAINS_CAPTURE_SRC_OV10635 = 0,
     /**< Capture source is OV 10635
      *   SUPPORTED on TDA2x EVM, TDA3x EVM
      */
     CHAINS_CAPTURE_SRC_HDMI_720P,
     /**< Capture source is HDMI in 720P resolution
      *   SUPPORTED on TDA2x EVM
      */
     CHAINS_CAPTURE_SRC_HDMI_1080P,
     /**< Capture source is HDMI in 1080P resolution
      *   SUPPORTED on TDA2x EVM
      */
     CHAINS_CAPTURE_SRC_AR0132RCCC,
     /**< Capture source is Aptina AR0132 RCCC sensor
      *   SUPPORTED on TDA2x MonsterCam Board
      */
     CHAINS_CAPTURE_SRC_AR0132ISP,
     /**< Capture source is Aptina AR0132 BAYER + External ISP sensor
      *   SUPPORTED on TDA2x MonsterCam Board
      */
     CHAINS_CAPTURE_SRC_OV10640_CSI2,
     /**< OV10640 sensor with CSI2 interface
      *   SUPPORTED on TDA3x EVM
      */
     CHAINS_CAPTURE_SRC_OV10640_PARALLEL,
     /**< OV10640 sensor with parallel interface
      *   SUPPORTED on TDA3x EVM
      */
     CHAINS_CAPTURE_SRC_AR0132BAYER_PARALLEL,
     /**< Capture source is Aptina AR0132 BAYER Parallel interface
      *   SUPPORTED on TDA3x EVM ONLY via Aptina Connector
      *   Only works with ISS related use-cases
      */

     CHAINS_CAPTURE_SRC_AR0132MONOCHROME_PARALLEL,
     /**< Capture source is Aptina AR0132 Monochrome Parallel interface
      *   SUPPORTED on TDA3x EVM ONLY via Aptina Connector
      *   Only works with ISS related use-cases
      */

     CHAINS_CAPTURE_SRC_AR0140BAYER_PARALLEL,
     /**< Capture source is Aptina AR0140 BAYER Parallel interface
      *   SUPPORTED on TDA3x EVM ONLY via Aptina Connector
      *   Only works with ISS related use-cases
      */
     CHAINS_CAPTURE_SRC_IMX224_CSI2,
     /**< Capture source is Sony IMX224 CSI interface
      *   SUPPORTED on TDA3x EVM ONLY LI module
      *   Only works with ISS related use-cases
      */

     CHAINS_CAPTURE_SRC_DM388,
     /**< Capture source is external DM388 chip giving out put on vout
      *   This is for TDA2x MonsterCam Board
      */

     CHAINS_CAPTURE_SRC_UB960_TIDA00262,
     /* Capture source is UB960 aggregator, to which 1 or more (4) TIDA00262
      *     could be connected.
      *     Each TIDA00262 module, has AR0140AT video sensor
      */
     CHAINS_CAPTURE_SRC_MAX
     /**< Max number of capture source's */

 } Chains_CaptureSrc;

 typedef enum
 {
     CHAINS_ISS_WDR_MODE_DISABLED = 0,
     /**< No WDR mode */
     CHAINS_ISS_WDR_MODE_SINGLE_PASS,
     /**< Single PASS WDR mode */
     CHAINS_ISS_WDR_MODE_TWO_PASS,
     /**< Two PASS WDR mode */
     CHAINS_ISS_WDR_MODE_MAX
     /**< Max number of capture source's */
 } Chains_IssWdrMode;



/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Chain Parameters.
 *
 *******************************************************************************
*/
typedef struct {

    UInt32 algProcId;
    /**<  Processor ID on which algorithm runs for
     *    - Frame copy algorithm use-case
     *    - DMA SW MS algorithm use-case
     */

    UInt32 numLvdsCh;
    /**< Number of channels of LVDS to enable */

    Chains_DisplayType displayType;
    /**< LCD/HDM display */

    Chains_CaptureSrc captureSrc;
    /**< OV/HDMI-720p/HDMI-1080p capture */

    UInt32 numPyramids;
    /**< numPyramids - used to select Alg with One/Two Pyramid Mode in  Dense Optical Flow*/

    Bool   issLdcEnable;
    /**< TRUE: LDC is enabled in ISS use-cases
     *       ONLY applicable for TDA3x platform
     */

    Bool   issVtnfEnable;
    /**< TRUE: VTNF is enabled in ISS use-cases
     *       ONLY applicable for TDA3x platform
     */

    Chains_IssWdrMode issWdrMode;
    /**< WDR mode, disabled, 2 pass or 1 pass
     *       ONLY applicable for TDA3x platform
     */

    AlgorithmLink_SrvOutputModes svOutputMode;
    /**< To slect 2D vs 3D Surround View (SRV) Alg */

    Bool enableCarOverlayInAlg;
    /**< Set to 1, if DSP need to create the car image, apply only for 2D SRV */

} Chains_Ctrl;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief   Main call for usecase selection and configuration
 *
 *          Chains_main is called form the main of main_ipu1_0.c .
 *          This is the entry point for usecase selection.
 *          Board, LCD initializations and demo selections are performed.
 *          It waits in a while loop till the end of demo/usecase is triggred
 *
 *******************************************************************************
*/
Void Chains_main();

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCam_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Dual Display usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplay(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Dual Display + ED usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DualDisplayEdgeDetection(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase with frame copy algorithm
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
 */
Void Chains_vipSingleCameraFrameCopy(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Edge Detection usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraEdgeDetection(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase with Analytics algorithm
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
 */
Void Chains_vipSingleCameraAnalyticsTda3xx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase with Analytics algorithm
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
 */
Void Chains_vipSingleCameraAnalyticsTda2xx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Optical Flow usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraDenseOpticalFlow(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief Single camera Pedestrain + Traffic sign Detection
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraObjectDetect(Chains_Ctrl * chainsCfg);

/**
 *******************************************************************************
 *
 * \brief Single camera Lane Detection
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraLaneDetect(Chains_Ctrl * chainsCfg);

/**
 *******************************************************************************
 *
 * \brief Single camera Sparse Optical Flow
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraSparseOpticalFlow(Chains_Ctrl * chainsCfg);


/**
 *******************************************************************************
 *
 * \brief   Multi Channel Capture Display usecase function for TDA2xx
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_lvdsVipMultiCam_Display_tda2xx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Multi Channel Capture Display usecase function for TDA2xx_MC
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_lvdsVipMultiCam_Display_tda2xx_mc(Chains_Ctrl *chainsCfg);


/**
 *******************************************************************************
 *
 * \brief   Multi Channel Capture Display usecase function for TDA3xx
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_lvdsVipMultiCam_Display_tda3xx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Dual Channel Capture Dual Display usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_lvdsVipDualCam_DualDisplay(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Surround View of 4Ch LVDS capture on TDA2xx and TDA2Ex
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_lvdsVipSurroundViewStandalone(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Surround View of 4Ch LVDS capture on TDA3x
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_lvdsVipSurroundView(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Surround View of 4Ch LVDS capture + FrontCam Analytics + Ultrasound
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_lvdsVipSurroundViewAnalyticsUltrasound(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief Avb Mcv Capture + Decode + Diaplay
 *
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_avbRx_Dec_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   AVB ED usecase function
 *
 * \param   chainsCfg         [IN] Chains_Ctrl
 *
 *******************************************************************************
 */
Void Chains_avbRxSurroundView(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel capture DssWB CRC Display usecase function
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCam_DisplayWbCrc(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel capture Enc Dec VPE Display usecase function
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCam_EncDec_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Single Channel Subframe Capture Display usecase function
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipSingleCameraSubFrameCopy(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   ISS Based capture, m2m isp processing, m2m simcop processing
 *          and display
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_issIspSimcop_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   ISS Based multi channel capture, m2m isp processing,
 *              m2m simcop processing & display
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_issMultCaptIspSimcopSv_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   ISS Based capture, m2m isp processing and display usecase
 *          It captures monochroma data from the sensor, processes it in
 *          ISP and displays the output of the ISP.
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_monochrome_issIsp_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Fast boot usecase showing ISS Based capture, m2m isp processing,
 *          Pedestrian Detect algorithm integration and display
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_fastBootIssIspSimcop_pd_Display(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Stereo Camera Soft ISP and Display
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipStereoOnlyDisplay(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Stereo Calibration
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_vipStereoCalibration(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Pre-recorded data input to Stereo
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_networkStereoDisplay(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data over network and display
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkRxDisplay(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data over network, decode and display
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkRxDecDisplay(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data from camera and stream over network
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkTxCapture(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data from camera, encode and stream over network
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkTxEncCapture(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Networking use-cases
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkRxTx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   TIDA0455 + OV490 based capture + SV
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_ov490VipSurroundViewStandalone(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data over network, PD/TSR/LD/SOF with Stereo
 *          from live camera
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_vipStereoCameraAnalytics(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data over network and do front cam analytics
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_networkRxCameraAnalyticsTda2xx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Capture Data over network or VIP and do front cam analytics
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void Chains_singleCameraAnalyticsTda2xx(Chains_Ctrl *chainsCfg);

/**
 *******************************************************************************
 *
 * \brief   Run time Menu selection
 *
 *          This functions displays the run time options available
 *          And receives user input and calls corrosponding functions run time
 *          Instrumentation logs are printing routine is called in same function
 *
 *
 * \return  char input by uart on success
 *******************************************************************************
*/
char Chains_menuRunTime();

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

/**
 *******************************************************************************
 *
 * \brief   Reset Memory DDR Statistics
 *
 *          This function send a system control message
 *          SYSTEM_COMMON_CMD_RESET_STAT_COLL to all cores.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_statCollectorReset();


/**
 *******************************************************************************
 *
 * \brief   Print Memory DDR Statistics
 *
 *          This function send a system control message
 *          SYSTEM_COMMON_CMD_PRINT_STAT_COLL to all cores.
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_statCollectorPrint();

/**
 *******************************************************************************
 *
 * \brief   Print Load Calculation.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_prfCpuLoadPrint();

/**
 *******************************************************************************
 *
 * \brief   Get IP Addr for the system
 *
 * \param   ipAddr  [OUT] IP address as a string
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
 */
Int32 Chains_getIpAddr(char *ipAddr);

/**
 *******************************************************************************
 *
 * \brief   Read a charater from UART or CCS console
 *
 * \return character that is read
 *
 *******************************************************************************
*/
char Chains_readChar();


/**
 *******************************************************************************
 *
 * \brief   Run DMA standalone test
 *
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 Chains_runDmaTest();

/**
 *******************************************************************************
 *
 * \brief   Set default values to chains control parameters
 *
 *******************************************************************************
*/
static inline Void Chains_Ctrl_Init(Chains_Ctrl *pPrm)
{
    memset(pPrm, 0, sizeof(pPrm));

    pPrm->algProcId = SYSTEM_PROC_DSP1;
    pPrm->numLvdsCh = VIDEO_SENSOR_NUM_LVDS_CAMERAS;
    pPrm->displayType = CHAINS_DISPLAY_TYPE_HDMI_1080P;
    pPrm->captureSrc = CHAINS_CAPTURE_SRC_OV10635;
    pPrm->numPyramids = 0;
    pPrm->issLdcEnable = FALSE;
    pPrm->issVtnfEnable = FALSE;
    pPrm->issWdrMode = CHAINS_ISS_WDR_MODE_DISABLED;
    pPrm->svOutputMode = ALGORITHM_LINK_SRV_OUTPUT_2D;
    pPrm->enableCarOverlayInAlg = 0;
}


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

