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
 * \ingroup ISSM2MSIMCOP_LINK_API
 * \defgroup ISSM2MSIMCOP_LINK_IMPL Iss M2m Simcop Link Implementation
 *
 * @{
 */


#ifndef _ISSM2MSIMCOP_LINK_PRIV_H_
#define _ISSM2MSIMCOP_LINK_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/issM2mSimcopLink.h>
#include <vps/iss/vps_m2mIss.h>
#include <vps/vps_m2mIntf.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Maximum number of frames getting allocated for iss M2m Simcop link.
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH     (SYSTEM_LINK_MAX_FRAMES_PER_CH)

/**
 *******************************************************************************
 * \brief Maximum number of link objects
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_OBJ_MAX (1)

/**
 *******************************************************************************
 *
 * \brief Maximum number of frames getting allocated for entire Link.
 *
 *******************************************************************************
 */
#define ISSM2MSIMCOP_LINK_MAX_FRAMES \
    ( ISSM2MSIMCOP_LINK_MAX_CH* \
      ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH \
    )


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure containing attributes for each channel operation
 *
 *******************************************************************************
*/
typedef struct {

    vpsissSimcopCfgPrms_t drvSimcopCfg;
    /**< SIMCOP configuration parameters */

    Utils_QueHandle emptyBufQue;
    /**< Link output side empty buffer queue */

    System_Buffer  *emptyBufsMem[ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH];
   /** Holds individual channel empty buffers */

    System_Buffer *pPrevOutBuffer;
    /**< Previous frame's output to be given as input to next frame */

    System_Buffer buffers[ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH];
    /**< System buffers for image output */

    System_VideoFrameBuffer videoFrames[ISSM2MSIMCOP_LINK_MAX_FRAMES_PER_CH];
    /**< Payload for System buffers for image output */

    Fvid2_Handle                drvHandle;
    /**< FVID2 driver handle. */

    UInt32 outBufSize;
    /**< Size of output buffer */

    UInt8           *saveFrameBufAddr;
    /**< Frame buffer used for saving captured frame */
    volatile UInt32  saveFrame;
    /**< Flag to indicate saving of the frame from process callback */
    Utils_DmaChObj   dumpFramesDmaObj;
    /**< DMA object to use when dumping frames to memory */
} IssM2mSimcopLink_ChObj;

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
    /**< Link ID */

    Utils_TskHndl tsk;
    /**< Handle to link task */

    IssM2mSimcopLink_CreateParams createArgs;
    /**< Create params link */

    System_LinkInfo prevLinkInfo;
    /**< Information of previous link */

    System_LinkInfo linkInfo;
    /**< Link information, which will be given to next link */

    BspOsal_SemHandle            drvSemProcessComplete;
    /**< Semaphore for tracking process call of the driver */

    IssM2mSimcopLink_ChObj  chObj[ISSM2MSIMCOP_LINK_MAX_CH];
    /**< Attributes for operating each channel */

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

    Utils_QueHandle fullBufQue;
    /**< Link output side full buffer queue */

    System_Buffer   *fullBufsMem[ISSM2MSIMCOP_LINK_MAX_FRAMES];
    /**< Memory for full buff queue */

} IssM2mSimcopLink_Obj;

extern IssM2mSimcopLink_Obj gIssM2mSimcopLink_obj[];

Int32 IssM2mSimcopLink_getInfo(Void * pTsk, System_LinkInfo * info);
Int32 IssM2mSimcopLink_getFullBuffers(Void * pTsk, UInt16 queId,
                                 System_BufferList * pBufList);
Int32 IssM2mSimcopLink_putEmptyBuffers(Void * pTsk, UInt16 queId,
                                  System_BufferList * pBufList);

Int32 IssM2mSimcopLink_drvCreate(IssM2mSimcopLink_Obj * pObj,
                            IssM2mSimcopLink_CreateParams * pPrm);

Int32 IssM2mSimcopLink_drvStart(IssM2mSimcopLink_Obj * pObj);
Int32 IssM2mSimcopLink_drvProcessData(IssM2mSimcopLink_Obj * pObj);
Int32 IssM2mSimcopLink_drvStop(IssM2mSimcopLink_Obj * pObj);
Int32 IssM2mSimcopLink_drvDelete(IssM2mSimcopLink_Obj * pObj);

Int32 IssM2mSimcopLink_drvPutEmptyBuffers(IssM2mSimcopLink_Obj * pObj,
                                     System_BufferList * pBufList);

Int32 IssM2mSimcopLink_drvPrintStatus(IssM2mSimcopLink_Obj * pObj);

Int32 IssM2mSimcopLink_drvSetSimcopConfig(
        IssM2mSimcopLink_Obj             *pObj,
        IssM2mSimcopLink_ConfigParams    *pCfgPrm);

Int32 IssM2mSimcopLink_drvSaveFrame(IssM2mSimcopLink_Obj *pObj);

Int32 IssM2mSimcopLink_drvGetSaveFrameStatus(IssM2mSimcopLink_Obj *pObj,
                    IssM2mSimcopLink_GetSaveFrameStatus *pPrm);

#endif

/* @} */
