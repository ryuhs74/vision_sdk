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
 * \ingroup GRPX_SRC_LINK_API
 * \defgroup GRPX_SRC_LINK_IMPL Grpx Src Link Implementation
 *
 * @{
 *
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \file GrpxSrcLink_priv.h Grpx Src Link private API/Data structures
 *
 * \brief  This file is a private header file for grpx src link implementation
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of grpx src link.
 *
 * \version 0.0 (Oct 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _GRPX_SRC_LINK_PRIV_H_
#define _GRPX_SRC_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/grpxSrcLink.h>
#include <include/link_api/algorithmLink_ultrasonicFusion.h>

#include "examples/tda2xx/include/draw2d.h"
#include <src/utils_common/include/utils_prf.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Maximum number of grpx src link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GRPX_SRC_LINK_OBJ_MAX    (2)

/**
 *******************************************************************************
 *
 * \brief Max Number of buffers this link uses
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GRPX_SRC_LINK_MAX_OUT_FRAMES   (SYSTEM_LINK_FRAMES_PER_CH)

/**
 *******************************************************************************
 *
 * \brief Number of buffers this link uses
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GRPX_SRC_LINK_OUT_FRAMES       (1)

/**
 *******************************************************************************
 *
 * \brief Interval at which stats have to be calculated in Msec
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GRPX_SRC_LINK_LOAD_REFRESH_INTERVAL_MSEC (5000)

/**
 *******************************************************************************
 *
 * \brief Interval at which GrpxSrcLink processing happens
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define GRPX_SRC_LINK_PROCESS_INTERVAL_MSEC (10)

/* String size for prints */
#define GRPX_SRC_LINK_STR_SZ (128)


typedef enum {

    GRAPHICS_SRC_LINK_PROFILER_NOT_RUNNING,
    /**< Profiler not Running */

    GRAPHICS_SRC_LINK_PROFILER_RUNNING
    /**< Profiler Running */

}GraphicsLink_ProfilerState;

typedef enum {

    GRAPHICS_SRC_LINK_START_PROFILER,
    /**< Command to start profiler */

    GRAPHICS_SRC_LINK_STOP_PROFILER,
    /**< Command to stop profiler */

    GRAPHICS_SRC_LINK_RESET_PROFILER
    /**< Command to reset profiler */

}GraphicsLink_ProfilerCommands;

typedef enum {

    GRAPHICS_SRC_LINK_STRPRINT_INACTIVE,
    GRAPHICS_SRC_LINK_STRPRINT_PRINTSTR,
    GRAPHICS_SRC_LINK_STRPRINT_ACTIVE,
    GRAPHICS_SRC_LINK_STRPRINT_CLEARSTR
} GraphicsLink_StringPrintState;
/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */
typedef struct {

    Bool isFirstTime; /* Are we invoking the Ultrasonic draw API for this first time */
    UInt32 startTime;
    UInt32 refreshInterval;
    Utils_DmaChObj dmaObj;
} GrpxSrcLink_UltrasonicDrawObj;

typedef struct {
    Utils_BufHndl bufOutQue;
    /**< Output buffer queue */

    System_Buffer buffers[GRPX_SRC_LINK_MAX_OUT_FRAMES];
    /**< System buffer data structure to exchange buffers between links */

    System_VideoFrameBuffer videoFrames[GRPX_SRC_LINK_MAX_OUT_FRAMES];
    /**< Payload for System buffers */

    Fvid2_Frame frames[GRPX_SRC_LINK_MAX_OUT_FRAMES];
    /**< Fvid2_frames to hold buffers */

    UInt32 numFrames;
    /**< Number of output buffers, must be <= GRPX_SRC_LINK_MAX_OUT_FRAMES */

    Fvid2_Format format;
    /**< Format object used for buffer creation */

}GrpxSrcLink_OutObj;

typedef struct {

    Utils_SystemLoadStats systemLoadStats[SYSTEM_PROC_MAX];
    /**< Load object for each processor in the system */

    UInt32                profilerState;
    /**< Variable to keep track of profiler state */

    UInt32                refreshInterval;
    /**< Variable to control load calculation period in Msec */

    UInt32                startTime;
    /**< Varaile to hold stats start time */


}GrpxSrc_StatsDisplayObj;


typedef struct
{
    GraphicsLink_StringPrintState    stringPrintState;
    /**< State of stirng print  */
    Int32  remainingDuration;
    /**< Time in ms that specified string needs to be dispalyed */
    GrpxSrcLink_StringInfo stringInfo;
    /**< String to be displayed */
} GrpxSrcLink_StringRunTimePrintObj;
/**
 *******************************************************************************
 *
 * \brief Structure to hold all grpx Src link related information
 *
 *******************************************************************************
 */

typedef struct {
    UInt32 tskId;
    /**< Placeholder to store grpx src link task id */

    Utils_TskHndl tsk;
    /**< Handle to grpx src link task */

    GrpxSrcLink_CreateParams createArgs;
    /**< Create params for grpx src link */

    System_LinkInfo prevLinkInfo;
    /**< previous link information */
    System_LinkInfo info;
    /**< Output queue information of this link */

    GrpxSrcLink_OutObj outObj;
    /**< Data structure to hold output information */

    Bool isFirstProcessCall;

    GrpxSrc_StatsDisplayObj statsDisplayObj;
    GrpxSrcLink_UltrasonicDrawObj ultrasonicDrawObj;
    Draw2D_Handle draw2DHndl;

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by capture link */

    Bool isLinkStarted;

    Bool isNewDataCmdSendOut;

    GrpxSrcLink_StringRunTimePrintObj stringPrintInfo;
    /**< String info to be printed */


}GrpxSrcLink_Obj;

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */

//Int32 GrpxSrcLink_drawSurroundViewEdgeDetectLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displaySurroundViewEdgeDetectStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawOpticalFlowLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displayOpticalFlowDetectStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawStereoDisparityLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displayStereoDisparityStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawStereoDisparityMultiFCAlgLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displayStereoDisparityMultiFCAlgStats(GrpxSrcLink_Obj *pObj);

Int32 GrpxSrcLink_drawCpuLoad(GrpxSrcLink_Obj *pObj,
                    UInt32 x, UInt32 y,
                    UInt32 barWidth,
                    UInt32 barHeight,
                    UInt32 padX,
                    UInt32 padY,
                    UInt32 fontIdx
                    );

//Int32 GrpxSrcLink_drawPdTsrLdLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displayPdTsrLdStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawPdTsrLdSofLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displayPdTsrLdSofStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawPdTsrLdSofStereoLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displayPdTsrLdSofStereoStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawSurroundViewSOFLayout(GrpxSrcLink_Obj *pObj, System_LinkChInfo *pChInfo);

//Int32 GrpxSrcLink_display3DSurroundViewStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displaySurroundViewSOFStats(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_drawSurroundViewStandaloneLayout(GrpxSrcLink_Obj *pObj);

//Int32 GrpxSrcLink_displaySurroundViewStandaloneStats(GrpxSrcLink_Obj *pObj);


//Int32 GrpxSrcLink_drawUltrasonicResultsCreate(GrpxSrcLink_Obj *pObj);
//Int32 GrpxSrcLink_drawUltrasonicResultsRun(GrpxSrcLink_Obj *pObj);
//Int32 GrpxSrcLink_drawUltrasonicResultsDelete(GrpxSrcLink_Obj *pObj);

Int32 GrpxSrcLink_drawAVM_E500Layout(GrpxSrcLink_Obj *pObj); 	//ryuhs74@20151103 - Add AVM-E500 Layout
Int32 GrpxSrcLink_drawAVM_E500Button(GrpxSrcLink_Obj *pObj); 	//ryuhs74@20151103 - Add AVM-E500 Layout
Int32 Draw2D_FillBacgroundColor( GrpxSrcLink_Obj *pObj );	 	//ryuhs74@20151103 - Add AVM-E500 Layout
Int32 Draw2D_AVME500_TopView( GrpxSrcLink_Obj *pObj );			//ryuhs74@20151103 - Add AVM-E500 Layout
Int32 Draw2D_AVME500_FullView( GrpxSrcLink_Obj *pObj );			//ryuhs74@20151103 - Add AVM-E500 Layout

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

