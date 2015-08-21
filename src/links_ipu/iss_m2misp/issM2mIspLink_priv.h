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
 * \ingroup ISSM2MISP_LINK_API
 * \defgroup ISSM2MISP_LINK_IMPL Iss M2m Isp Link Implementation
 *
 * @{
 */

 /**
 *******************************************************************************
 *
 * \file issM2mIspLink_priv.h Iss M2m Isp link private header file.
 *
 * \brief  This file is a private header file for iss M2misp link implementation
 *
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of iss M2misp link.
 *
 *         Links and chains operate on channel number to identify the buffers
 *         from different sources and streams.
 *
 *         Output of Iss M2m Isp link consists of two queues
 *          - Output Image
 *          - H3A statistics
 *
 * \version 0.0 (Jul 2014) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ISSM2MISP_LINK_PRIV_H_
#define _ISSM2MISP_LINK_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/issM2mIspLink.h>

#include <vps/iss/vps_m2mIss.h>
#include <vps/vps_m2mIntf.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* Sensor Exposure Ratio,
   TODO: Read it from the sensor register and calculate WDR Parameters */
#define SENSOR_EXPOSURE_RATIO   (128)
/* Typically Set to log2(SENSOR_EXPOSURE_RATIO) */
#define SENSOR_EV_RATIO         (7)

/**
 *******************************************************************************
 * \brief Number of frames (buffers) allocated per channel of ISP processing
 *******************************************************************************
 */
#define ISSM2MISP_LINK_MAX_FRAMES_PER_CH     (SYSTEM_LINK_FRAMES_PER_CH)


/**
 *******************************************************************************
 * \brief Maximum number of Iss M2m isp link objects
 *******************************************************************************
 */
#define ISSM2MISP_LINK_OBJ_MAX            (1)

/**
 *******************************************************************************
 *
 * \brief Maximum number of frames getting allocated for entire capture Link.
 *
 *        This is to allocate frame container statically, which will point to
 *        actual frames. Frames will be allocated based on application requests
 *        but frame containers are always allocated at init time that is max
 *        of frames possible.
 *
 *******************************************************************************
 */
#define ISSM2MISP_LINK_MAX_FRAMES (ISSM2MISP_LINK_MAX_CH*ISSM2MISP_LINK_MAX_FRAMES_PER_CH)

/* \brief WDR Merge threshold percentage,
    the range of THRESHOLD_PERCENTAGE is 0-100% specified Q16 format
   65535 Maps to 100% */
#define ISSM2MISP_LINK_THRESHOLD_PERCENTAGE    (65535)


//#define ISSM2MISP_LINK_ENABLE_IPIPEIF_OUTPUT
//#define ISSM2MISP_LINK_ENABLE_GLBCE_OUTPUT
/* #define ISSM2MISP_LINK_ENABLE_IPIPE_OUTPUT */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *  \brief  Enumerations for 1st pass or second pass
 *******************************************************************************
*/
typedef enum
{
    ISSM2MISP_LINK_FIRST_PASS = 0,
    /**< First pass */

    ISSM2MISP_LINK_SECOND_PASS,
    /**< Second pass */

    ISSM2MISP_LINK_MAXNUM_PASS,
    /**< Maximum number of passes for this link */

    ISSM2MISP_LINK_INPUT_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */

} IssM2mIspLink_PassId;

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Data Structure for the ISS M2M ISP link Output object
 *
 *          This includes elements which are common for every output queue
 *
 *******************************************************************************
*/
typedef struct {
    Utils_QueHandle fullBufQue;
    /**< Link output side full buffer queue */
    System_Buffer   *fullBufsMem[ISSM2MISP_LINK_MAX_FRAMES];
    /**< Memory for full buff queue */
    System_Buffer   buffers[ISSM2MISP_LINK_MAX_CH]
                           [ISSM2MISP_LINK_MAX_FRAMES_PER_CH];
    /**< System buffer data structure to exchange buffers between links */
    Utils_QueHandle emptyBufQue[ISSM2MISP_LINK_MAX_CH];
    /**< Link output side empty buffer queue */
    System_Buffer  *emptyBufsMem[ISSM2MISP_LINK_MAX_CH]
                           [ISSM2MISP_LINK_MAX_FRAMES_PER_CH];

    UInt32  bufSize[ISSM2MISP_LINK_MAX_CH];

   /** Holds individual channel empty buffers */
} IssM2mIspLink_OutObj;


typedef struct
{
    Fvid2_Handle               drvHandle;
    /**< FVID2 display driver handle. */

    vpsissIspParams_t          ispPrms;
    /**< isp parameters */

    vpsissIspOpenParams_t      openPrms;
    /**< Core Open Parameters */

    vpsissIpipeWbConfig_t      ipipeWbCfg;
    /**< White balance config at ISIF */

    vpsissIsifGainOfstConfig_t  isifWbCfg;
    /**< ISIF WB config */

    vpsissIsifBlackClampConfig_t isifBlkClampCfg;
    /**< ISIF Black Clamping Configuration */

    vpsissIpipeRgb2RgbConfig_t rgb2rgb1;
    /**< RGB to RGB Matrix 1 */
    vpsissIpipeRgb2RgbConfig_t rgb2rgb2;
    /**< RGB to RGB Matrix 2 */

    vpsissIpipeifWdrCfg_t       wdrCfg;
    /**< WDR Configuration, used for updating White balance gains
         and also merge parameters based on exposure ratio */
    vpsissIpipeifDeCompandInsts_t compDecompCfg;
    /**< WDR Companding configuration,
         Companding can be enabled/disabled based on exposure ratio */

    vpsissRszCfg_t                rszCfg;
    vpsissIpipeifSaturaInsts_t    satCfg;

} IssM2mIspLink_PassObj;

/**
 *******************************************************************************
 *
 *  \brief  Structure containing attributes for each channel operation
 *
 *******************************************************************************
*/
typedef struct {

    IssIspConfigurationParameters ispCfgParams;

    IssM2mIspLink_PassObj         passCfg[ISSM2MISP_LINK_MAXNUM_PASS];

    vpsissIpipeLutFmtCnvt_t       lutFmtCnvt;
    vpsissRszCtrl_t               rszCtrl;
    vpsissIpipeifCtrl_t           ipipeifCtrl;
    vpsissIsifCtrl_t              isifCtrl;
    vpsissIpipeCtrl_t             ipipeCtrl;
    vpsissGlbceCtrl_t             glbceCtrl;
    vpsissIpipeInConfig_t         inCfg;
    vpsissH3aCtrl_t               h3aCtrl;

    UInt32                        expRatio;
    UInt32                        evRatio;
    /* Sensor Exposure Ratio */

} IssM2mIspLink_ChannelObject;

/**
 *******************************************************************************
 *
 *  \brief  Structure containing information for each instance of
 *          Iss capture link.
 *
 *******************************************************************************
*/
typedef struct {

    UInt32 linkId;
    /**< Iss M2m Isp Link ID */

    Utils_TskHndl tsk;
    /**< Handle to iss M2m isp link task */

    System_VideoFrameBuffer videoFramesRszA[ISSM2MISP_LINK_MAX_CH]
                                       [ISSM2MISP_LINK_MAX_FRAMES_PER_CH];
    /**< Payload for System buffers for image output */

    System_VideoFrameBuffer videoFramesRszB[ISSM2MISP_LINK_MAX_CH]
                                       [ISSM2MISP_LINK_MAX_FRAMES_PER_CH];
    /**< Payload for System buffers for image output */

    System_MetaDataBuffer h3aBuffer[ISSM2MISP_LINK_MAX_CH]
                                   [ISSM2MISP_LINK_MAX_FRAMES_PER_CH];
    /**< Payload for System buffers for h3a statistics output */

    IssM2mIspLink_OutObj linkOutObj[ISSM2MISP_LINK_OUTPUTQUE_MAXNUM];
    /**< Output queue properties */

    IssM2mIspLink_CreateParams createArgs;
    /**< Create params for iss M2m isp link */

    System_LinkInfo prevLinkInfo;
    /**< Information of previous link */

    System_LinkQueInfo inQueInfo;
    /**< Input Q channel specific info, read from the outQ of previous LINK */

    System_LinkInfo linkInfo;
    /**< Link information, which will be given to next link */

    IssM2mIspLink_ChannelObject chObj[ISSM2MISP_LINK_MAX_CH];
    /**< Attributes for operating each channel */

    Void    *pIntermediateBufAddr;
    /**< Pointer to data buffer used to hold first pass output
     *   in 20-bit WDR mode
     */
    UInt32  intermediateBufSize;
    /**< Size of intermediate buffer */

    BspOsal_SemHandle semProcessCall;
    /**< Semaphore for tracking process call of the driver */

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Used to log the memory usage of the VPE link */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */

    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */

    UInt32  statsStartTime;
    /**< Time at which capture is started */

} IssM2mIspLink_Obj;



extern IssM2mIspLink_Obj gIssM2mIspLink_obj[];

Int32 IssM2mIspLink_getInfo(Void * pTsk, System_LinkInfo * info);
Int32 IssM2mIspLink_getFullBuffers(Void * pTsk, UInt16 queId,
                                 System_BufferList * pBufList);
Int32 IssM2mIspLink_putEmptyBuffers(Void * pTsk, UInt16 queId,
                                  System_BufferList * pBufList);

Int32 IssM2mIspLink_drvCreate(IssM2mIspLink_Obj * pObj,
                            IssM2mIspLink_CreateParams * pPrm);

Int32 IssM2mIspLink_drvProcessData(IssM2mIspLink_Obj * pObj);

Int32 IssM2mIspLink_drvDelete(IssM2mIspLink_Obj * pObj);

Int32 IssM2mIspLink_drvPutEmptyBuffers(IssM2mIspLink_Obj * pObj,
                                     System_BufferList * pBufList);

Int32 IssM2mIspLink_drvPrintStatus(IssM2mIspLink_Obj * pObj);

Int32 IssM2mIspLink_drvSetIspConfig(IssM2mIspLink_Obj                 * pObj,
                                    IssIspConfigurationParameters     * pCfgPrm,
                                    UInt32                              isDccSetCfg);

Int32 IssM2mIspLink_drvCallBack(Fvid2_Handle handle,
                                       Ptr appData,
                                       Ptr reserved);

Int32 IssM2mIspLink_drvApplyConfig(IssM2mIspLink_Obj * pObj,
                                   UInt32 chId,
                                   UInt32 passId,
                                   Bool enableOut[ISSM2MISP_LINK_OUTPUTQUE_MAXNUM]
                                   );

Int32 IssM2mIspLink_drvUpdateAwbResults(
        IssM2mIspLink_Obj *pObj,
        Void *pParams);
Int32 IssM2mIspLink_drvUpdateWdrMergeParams(
        IssM2mIspLink_Obj *pObj,
        Void *pParams);

static inline Bool IssM2mIspLink_isWdrMode(IssM2mIspLink_OperatingMode opMode)
{
    Bool isWdrMode = FALSE;

    if ((ISSM2MISP_LINK_OPMODE_2PASS_WDR == opMode) ||
        (ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED == opMode))
    {
        isWdrMode = TRUE;
    }

    return (isWdrMode);
}

#endif

/* @} */
