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
 * \ingroup DISPLAY_LINK_API
 * \defgroup DISPLAY_LINK_IMPL Display Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file displayLink_priv.h Display Link private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Display link instance/handle object
 *         - All the local data structures
 *         - Display driver interfaces
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _DISPLAY_LINK_PRIV_H_
#define _DISPLAY_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/displayLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Max Number of display link instances supported
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DISPLAY_LINK_OBJ_MAX                     (4)

/**
 *******************************************************************************
 *
 *   \brief Link local CMD: Releases displayed frames to previous link
 *
 *******************************************************************************
 */
#define DISPLAY_LINK_CMD_RELEASE_FRAMES                (0x0500)



/**
 *******************************************************************************
 *
 *   \brief The MAX number of FVID2 frames to be allocated in the display link
 *
 *          This is set to max number of frames queued. Though display link
 *          can support multiple channels, only one channel is selected
 *          and active.
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define DISPLAY_LINK_MAX_FRAMES_PER_HANDLE      (SYSTEM_LINK_MAX_FRAMES_PER_CH)

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Display link instance object
 *
 *          This structure contains
 *          - All the local data structures
 *          - VPS Data structures required for Display driver interfaces
 *          - All fields to support the Link stats and status information
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 tskId;
    /**< placeholder to store the Display link task Id */
    Utils_TskHndl tsk;
    /**< placeholder to store the Display link task handler */
    DisplayLink_CreateParams createArgs;
    /**< placeholder to store the Display link create parameters */
    System_LinkInfo inTskInfo;
    /**< Specifies a place holder that describe the LINK information */
    System_LinkQueInfo inQueInfo;
    /**< place holder that describe the output information of the LINK */
    UInt32 curDisplayChannelNum;
    /**< The active input channel selected for display */
    FVID2_Handle displayHndl;
    /**< FVID2 display driver handle */
    Vps_DispCreateParams displayCreateArgs;
    /**< VPS driver Create time parameters */
    Vps_DispDssParams dssPrms;
    /**< DSS driver parameters */
    Vps_DssDispcVidConfig vidCfg;
    /**< DSS driver video pipe configuration structure */
    Vps_DssDispcGfxConfig gfxCfg;
    /**< DSS driver graphics pipe configuration structure */
    UInt32 displayInstId;
    /**< Display driver ID used in this link instance */
    FVID2_Frame frames[DISPLAY_LINK_MAX_FRAMES_PER_HANDLE];
    /**< FVID2 Frames to interface with the display driver */
    Utils_QueHandle fvidFrameQueue;
    /**< Free FVID2 Frames queue for mapping system buffers from input queue */
    FVID2_Frame *fvidFrameQueueMem[DISPLAY_LINK_MAX_FRAMES_PER_HANDLE];
    /**< Free FVID2 Frames Queue Mem */
    Utils_QueHandle systemBufferQueue;
    /**< Free FVID2 Frames queue for mapping system buffers from input queue */
    System_Buffer *systemBufferQueueMem[DISPLAY_LINK_MAX_FRAMES_PER_HANDLE];
    /**< Free FVID2 Frames Queue Mem */
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

    UInt32 isDisplayRunning;
    /**< Flag to indicate if display is running or not */

    UInt32 queueCount;
    /**< Queue count */

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by this link */

} DisplayLink_Obj;


/*******************************************************************************
 *  Display Link Private Functions
 *******************************************************************************
 */
Int32 DisplayLink_drvCreate(DisplayLink_Obj *pObj,
                            DisplayLink_CreateParams *pPrm);
Int32 DisplayLink_drvStart(DisplayLink_Obj *pObj);
Int32 DisplayLink_drvProcessData(DisplayLink_Obj *pObj);
Int32 DisplayLink_drvReleaseData(DisplayLink_Obj *pObj);
Int32 DisplayLink_drvStop(DisplayLink_Obj *pObj);
Int32 DisplayLink_drvDelete(DisplayLink_Obj *pObj);
Int32 DisplayLink_drvSwitchCh(DisplayLink_Obj *pObj,
                              DisplayLink_SwitchChannelParams *prm);
Int32 DisplayLink_drvPrintStatistics(DisplayLink_Obj *pObj);
Int32 DisplayLink_drvGetStatistics(DisplayLink_Obj *pObj,
            DisplayLink_Statistics *pPrm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
