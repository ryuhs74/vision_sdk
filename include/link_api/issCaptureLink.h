/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup ISSCAPTURE_LINK_API ISS Capture Link API
 *
 *
 *   ISS Capture Link is used to capture video frames from ISS ports
 *   Camera Rx Phy (CSI2 / LVDS) OR Parallel Port and store in DDR.
 *   Data path which gets serviced by this link is as follows
 *   CSI2 / LVDS / Parallel port -> CAL -> DDR
 *
 *   Each single capture interface needs one instantiation
 *   of the ISS capture link.
 *
 *   The captured frames will be output on a single output queue. In case of
 *      CSI2 interface mutiple virtual channels can be captured. For every
 *      virtual channel, the captured frame will sent on a seperate channel.
 *   The output queue can in turn to be connected to another link like M2M ISP
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file issCaptureLink.h
 *
 * \brief ISS Capture link API public header file.
 *
 * \version 0.0 (Apr 2014) : [PS] First version
 * \version 0.1 (Apr 2015) : [Suj] Updated to support multiple channel
 *                                  reception on CSI2 interface.
 *
 *******************************************************************************
 */

#ifndef _ISSCAPTURE_LINK_H_
#define _ISSCAPTURE_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <include/link_api/system_inter_link_api.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 * \brief Max Channels number of channel that could be received.
 *      Note only CSI2 interface supports capture of mutiple channels. For other
 *      interfaces, we can capture only 1 channel.
 *******************************************************************************
*/
#define ISSCAPT_LINK_MAX_CH          (4U)


/**
 *******************************************************************************
 *
 * \brief Controls the default number of output buffer allocated for each
 *          channel.
 *
 *******************************************************************************
*/
#define ISSCAPTURE_LINK_NUM_BUFS_DEFAULT    (3)


/* @} */

/* Control Command's    */

/**
    \ingroup LINK_API_CMD
    \addtogroup ISSCAPTURE_LINK_API_CMD  ISS Capture Link Control Commands

    @{
*/

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To save/dump captured frame into extra frame
 *                    Dumps the frame allocated at CreateTime
 *                    If extra frame buffer is not allocated at create time
 *                      returns Error
 *
 *      Can be used to save frame only when capture is running
 *
 *   \param None
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSCAPTURE_LINK_CMD_SAVE_FRAME             (0x5000)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Return's pointer to saved frame
 *
 *   \param IssCaptureLink_GetSaveFrameStatus   [OUT]
 *
 *   \return SYSTEM_STATUS_SOK on success
 *
 *******************************************************************************
 */
#define ISSCAPTURE_LINK_CMD_GET_SAVE_FRAME_STATUS      (0x5001)

/* @} */


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

    /* None */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
*/

/**
 *******************************************************************************
 *  \brief Callback that is called when a frame is received at this link
 *
 *  \param appObj [IN] User pointer specified during create
 *  \param pFrame [IN] Pointer to the received frame
 *
 *******************************************************************************
 */
typedef Void (*IssCaptureLink_Callback)(Void *appObj,
                            System_Buffer *pBuffer);


/**
 *******************************************************************************
 *  \brief Information of saved RAW data frame
 *******************************************************************************
 */
typedef struct
{
    UInt32 isSaveFrameComplete;
    /**< TRUE: Frame is saved at address mentioned in 'bufAddr'
     *   FALSE: Frame is not yet saved, try after some time
     */

    UInt32 bufAddr;
    /**< Address where frame is saved */

    UInt32 bufSize;
    /**< Size of buffer where frame is saved */

} IssCaptureLink_GetSaveFrameStatus;

/**
 *******************************************************************************
 *  \brief Configure the position and order of lane for the complex IO.
 *******************************************************************************
 */
typedef struct
{
    UInt32 pol;
    /**< TRUE configures for - / + order of differential signal.
        + / - order otherwise */
    UInt32 position;
    /**< Specify if this lane is to be used, if so, on which position.
        0x0 - Not used / disabled lane
        0x1 - Position 1
        0x2 - Position 2
        0x3 - Position 3
        0x4 - Position 4
        0x5 - Position 5 */
} IssCaptureLink_CalCmplxIoLaneCfg;

/**
 *******************************************************************************
 *  \brief Lane configuration complex IO.
 *******************************************************************************
 */
typedef struct
{
    IssCaptureLink_CalCmplxIoLaneCfg clockLane;
    /**< Configure position and order of differential signal */
    IssCaptureLink_CalCmplxIoLaneCfg data1Lane;
    /**< Configure position and order of differential signal */
    IssCaptureLink_CalCmplxIoLaneCfg data2Lane;
    /**< Configure position and order of differential signal */
    IssCaptureLink_CalCmplxIoLaneCfg data3Lane;
    /**< Configure position and order of differential signal */
    IssCaptureLink_CalCmplxIoLaneCfg data4Lane;
    /**< Configure position and order of differential signal */
} IssCaptureLink_CalCmplxIoCfg;

/**
 *******************************************************************************
 * \brief ISS capture link CSI2 related parameters
 *******************************************************************************
*/
typedef struct
{
    IssCaptureLink_CalCmplxIoCfg       cmplxIoCfg;
    /**< Number of data lanes that will be used - Value between 1 - 4 */

    UInt32                  csi2PhyClk;
    /**< Clock provided to CSI2 Phy by the SoC */

} IssCaptureLink_Csi2Params;

/**
 *******************************************************************************
 * \brief ISS capture link LVDS related parameters.
 *          LVDS interface is not yet supported.
 *******************************************************************************
*/
typedef struct
{
    UInt32 reserved;
    /* Reserved */
} IssCaptureLink_LvdsParams;

/**
 *******************************************************************************
 * \brief ISS capture link Parallel port related parameters.
 *
 * Reserved for future.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 reserved;
    /* Reserved */
} IssCaptureLink_ParallelPortParams;


/**
 *******************************************************************************
 * \brief Structure for setting the output buffer and stream
 *        parameters for capture.
 *******************************************************************************
*/
typedef struct
{
    UInt32                          width;
    /**<  Output width, in pixels.
     *
     *   This represents the output width of the data that is captured
     */

    UInt32                          height;
    /**< Output height, in lines.
     *
     *   This represents the output height of the data that is captured
     */

    System_VideoDataFormat          dataFormat;
    /**< Output Data format, valid options are
     *
     *   SYSTEM_DF_BAYER_RAW
     */

    System_Csi2DataFormat           inCsi2DataFormat;
    /**< Specify CSI2 input data format.
     *
     *   Valid only when videoIfMode == SYSTEM_VIFM_SCH_CSI2.
     *   Ignored for others
     */

    UInt32                          inCsi2VirtualChanNum;
    /**< Specify the CSI2 virtual channel number
     *
     *   Valid only when videoIfMode == SYSTEM_VIFM_SCH_CSI2.
     *   0 <= value < 4.
     *   Ignored for others
     */

    UInt32                          maxWidth;
    /**< Max width in pixels
     *
     *   This should be >= width
     *   This field decides the width of buffer that is allocated.
     */

    UInt32                          maxHeight;
    /**< Max height in pixels
     *
     *   This should be >= height
     *   This field decides the height of buffer that is allocated.
     */

     UInt32                         numOutBuf;
     /**< Number of output buffer to allocate */

} IssCaptureLink_OutParams;


/**
 *******************************************************************************
 * \brief ISS capture link creation parameters.
 *******************************************************************************
*/
typedef struct
{
    System_VideoIfMode videoIfMode;
    /**< [IN] Video capture mode.
     *
     *    Valid values are,
     *    SYSTEM_VIFM_SCH_CSI2  - CSI2 - csi2Params are used
     *    SYSTEM_VIFM_SCH_LVDS  - LVDS - lvdsParams are used
     *    SYSTEM_VIFM_SCH_CPI   - parallel port - parallelPortParams are used
     */

    System_VideoIfWidth videoIfWidth;
    /**< [IN] Video interface mode. */

    System_CaptBufferCaptMode bufCaptMode;
    /**< [IN] Buffer capture mode.
     *   For valid values see #System_CaptBufferCaptMode. */

    IssCaptureLink_Csi2Params csi2Params;
    /**< Only valid when videoIfMode == SYSTEM_VIFM_SCH_CSI2
     */

    UInt32                    numCh;
    /**< Number of channels to be received.
     *  0 <= value < 4 (ISSCAPT_LINK_MAX_CH)
     *  Valid only when videoIfMode == SYSTEM_VIFM_SCH_CSI2.
     *  For other it HAS to be 1.
     */

    IssCaptureLink_LvdsParams lvdsParams;
    /**< Only valid when videoIfMode == SYSTEM_VIFM_SCH_LVDS
     */

    IssCaptureLink_ParallelPortParams parallelPortParams;
    /**< Only valid when videoIfMode == SYSTEM_VIFM_SCH_CPI
     */

    IssCaptureLink_OutParams outParams[ISSCAPT_LINK_MAX_CH];
    /**< Data Output related parameters. */

    System_LinkOutQueParams outQueParams;
    /**< Output queue information */

    IssCaptureLink_Callback callback;
    /**< User defined callback that is called when a frame is received at
     *   this link
     */

    Void                    *appObj;
    /**< User specified pointer that is returned to user via the callback */

    UInt32                  allocBufferForRawDump;
    /**< [IN] Flag to allocate extra frame buffer for RAW dump
              1, extra frame buffer is allocated
              0, extra frame buffer is not allocated, so RAW frames
                 cannot be dumped */

    System_LinkMemAllocInfo memAllocInfo;
    /**< Memory alloc region info, used to pass user alloc memory address */

} IssCaptureLink_CreateParams;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Iss Capture link register and init
 *
 * Creates the tasks for the link. Registers Link within System with
 * unique link ID and callback functions.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_init(void);


/**
 *******************************************************************************
 *
 * \brief Iss Capture link de-register and de-init
 *
 * Delete the tasks and de-registers itself from the system.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssCaptureLink_deInit(void);

/**
 *******************************************************************************
 *
 * \brief Set defaults for creation time parameters
 *
 * \param  pPrm [OUT] Create parameters for capture link.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static inline void IssCaptureLink_CreateParams_Init (
                                            IssCaptureLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->videoIfMode = SYSTEM_VIFM_SCH_CSI2;
    pPrm->videoIfWidth = SYSTEM_VIFW_4LANES;
    pPrm->bufCaptMode = SYSTEM_CAPT_BCM_LAST_FRM_REPEAT;
    pPrm->allocBufferForRawDump = 0;
    pPrm->numCh = 1U;

    pPrm->outParams[0U].width = 1280U;
    pPrm->outParams[0U].height = 720U;
    pPrm->outParams[0U].dataFormat = SYSTEM_DF_BAYER_BGGR;
    pPrm->outParams[0U].inCsi2DataFormat = SYSTEM_CSI2_RAW12;
    pPrm->outParams[0U].inCsi2VirtualChanNum = 0U;
    pPrm->outParams[0U].maxWidth = pPrm->outParams[0U].width;
    pPrm->outParams[0U].maxHeight = pPrm->outParams[0U].height;
    pPrm->outParams[0U].numOutBuf = ISSCAPTURE_LINK_NUM_BUFS_DEFAULT;

    pPrm->csi2Params.cmplxIoCfg.clockLane.pol       = FALSE;
    pPrm->csi2Params.cmplxIoCfg.clockLane.position  = 2U;
    pPrm->csi2Params.cmplxIoCfg.data1Lane.pol       = FALSE;
    pPrm->csi2Params.cmplxIoCfg.data1Lane.position  = 1U;
    pPrm->csi2Params.cmplxIoCfg.data2Lane.pol       = FALSE;
    pPrm->csi2Params.cmplxIoCfg.data2Lane.position  = 3U;
    pPrm->csi2Params.cmplxIoCfg.data3Lane.pol       = FALSE;
    pPrm->csi2Params.cmplxIoCfg.data3Lane.position  = 4U;
    pPrm->csi2Params.cmplxIoCfg.data4Lane.pol       = FALSE;
    pPrm->csi2Params.cmplxIoCfg.data4Lane.position  = 5U;
    pPrm->csi2Params.csi2PhyClk                     = 400U;

}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/
