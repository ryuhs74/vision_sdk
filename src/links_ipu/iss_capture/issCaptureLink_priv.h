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
 * \ingroup ISSCAPTURE_LINK_API
 * \defgroup ISSCAPTURE_LINK_IMPL Iss Capture Link Implementation
 *
 * @{
 */

 /**
 *******************************************************************************
 *
 * \file issCaptureLink_priv.h Iss Capture link private header file.
 *
 * \brief  This file is a private header file for isscapture link implementation
 *
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of iss capture link.
 *
 *         Links and chains operate on channel number to identify the buffers
 *         from different sources and streams.
 *
 *         Output of Iss Capture link is single queue and one or more channels.
 *         If multiple input sources (CSI2 / LVDS / parallel port) need to be
 *         used in the system simultaneously, then multiple instances of
 *         Iss iss capture link needs to be used. This is not yet supported.
 *
 * \version 0.0 (Apr 2014) : [PS] First version
 * \version 0.1 (Apr 2015) : [Suj] Updated to support multiple channel
 *                                  reception on CSI2 interface.
 *
 *******************************************************************************
 */

#ifndef _ISSCAPTURE_LINK_PRIV_H_
#define _ISSCAPTURE_LINK_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/system_linkId.h>
#include <include/link_api/issCaptureLink.h>

#include <vps/iss/vps_isscommon.h>
#include <vps/iss/vps_cfgcal.h>
#include <vps/vps_captureIss.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Maximum number of output queues that iss capture link supports.
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define ISSCAPTURE_LINK_MAX_OUT_QUE         (1)


/**
 *******************************************************************************
 *
 * \brief Maximum number of Iss capture link objects
 *
 *******************************************************************************
 */
#define ISSCAPTURE_LINK_OBJ_MAX             (1)

/**
 *******************************************************************************
 *
 * \brief Specifies the alignment of allocated buffer boundary
 *
 *******************************************************************************
 */
#define ISSCAPTURE_LINK_BUF_ALIGNMENT       (32)

/**
 *******************************************************************************
 *
 * \brief Maximum number of frames getting allocated for entire capture Link.
 *
 *        This is to allocate frame container statically, which will point to
 *        actual frames. Frames will be allocated based on application requests
 *        but frame containers are always allocated at init time that is max
 *        of frames possible for all the channels.
 *
 *******************************************************************************
 */
#define ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE (SYSTEM_LINK_MAX_FRAMES_PER_CH *\
                                                ISSCAPT_LINK_MAX_CH)


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *  \brief  Structure containing information for each instance of
 *          Iss capture link.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 linkId;
    /**< Iss Capture Link ID */

    Utils_TskHndl tsk;
    /**< Handle to iss capture link task */

    UInt32                  drvInstId;
    /**< As defined in <bsp>\include\vps\vps_capture.h*/
    Vps_CaptCreateParams    drvCreatePrms;
    /**< FVID2 Create time parameters. */
    Vps_CaptIssOpenParams_t drvIssCaptureCreatePrms;
    /**< Additional args passed to driver during FVID2 create */
    Vps_CaptCreateStatus    drvCreateStatus;
    /**< Create status returned by driver during Fvid2_create(). */
    Fvid2_Handle            drvHandle;
    /**< FVID2 capture driver handle. */
    Fvid2_CbParams          drvCbPrms;
    /**< Callback params. */
    vpsissCalCfg_t          drvCalCfg;
    /**< CAL config */

    Fvid2_Frame fvid2Frames[ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE];
    /**< FVID2 Frames that will be used for capture. */

    System_Buffer buffers[ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE];
    /**< System buffer data structure to exchange buffers between links */

    System_VideoFrameBuffer videoFrames[ISSCAPTURE_LINK_MAX_FRAMES_PER_HANDLE];
    /**< Payload for System buffers */

    UInt32 outBufSize;
    /**< Size of output buffer */

    IssCaptureLink_CreateParams createArgs;
    /**< Create params for iss capture link */

    Utils_BufHndl bufQue;
    /**< Output buffer queue */

    System_LinkInfo info;
    /**< Capture link information */

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by iss capture link */

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

    UInt8           *saveFrameBufAddr;
    /**< Frame buffer used for saving captured frame */
    volatile UInt32  saveFrame;
    /**< Flag to indicate saving of the frame from process callback */
    Utils_DmaChObj   dumpFramesDmaObj;
    /**< DMA object to use when dumping frames to memory */
} IssCaptureLink_Obj;

extern IssCaptureLink_Obj gIssCaptureLink_obj[];

Int32 IssCaptureLink_getInfo(Void * pTsk, System_LinkInfo * info);
Int32 IssCaptureLink_getFullBuffers(Void * pTsk, UInt16 queId,
                                 System_BufferList * pBufList);
Int32 IssCaptureLink_putEmptyBuffers(Void * pTsk, UInt16 queId,
                                  System_BufferList * pBufList);

Int32 IssCaptureLink_drvCreate(IssCaptureLink_Obj * pObj,
                               IssCaptureLink_CreateParams * pPrm);

Int32 IssCaptureLink_drvStart(IssCaptureLink_Obj * pObj);
Int32 IssCaptureLink_drvProcessData(IssCaptureLink_Obj * pObj);
Int32 IssCaptureLink_drvStop(IssCaptureLink_Obj * pObj);
Int32 IssCaptureLink_drvDelete(IssCaptureLink_Obj * pObj);

Int32 IssCaptureLink_drvPutEmptyBuffers(IssCaptureLink_Obj * pObj,
                                        System_BufferList * pBufList);

Int32 IssCaptureLink_drvPrintStatus(IssCaptureLink_Obj * pObj);

Int32 IssCaptureLink_drvPrintBufferStatus(IssCaptureLink_Obj * pObj);

Void IssCaptureLink_enumAssertCheck();

Int32 IssCaptureLink_drvSaveFrame(IssCaptureLink_Obj *pObj);

Int32 IssCaptureLink_drvGetSaveFrameStatus(IssCaptureLink_Obj *pObj,
                    IssCaptureLink_GetSaveFrameStatus *pPrm);

#endif

/* @} */
