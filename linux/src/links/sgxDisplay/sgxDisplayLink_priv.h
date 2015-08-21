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
 * \ingroup SGXDISPLAY_LINK_API
 * \defgroup SGXDISPLAY_LINK_IMPL SgxDisplay Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file sgxDisplayLink_priv.h SgxDisplay Link private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - SgxDisplay link instance/handle object
 *         - All the local data structures
 *         - SgxDisplay driver interfaces
 *
 * \version 0.0 (Jun 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SGXDISPLAY_LINK_PRIV_H_
#define _SGXDISPLAY_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <linux/src/system/system_priv_common.h>
#include <linux/src/system/system_drm_egl.h>
#include <osa.h>
#include <osa_mutex.h>
#include <osa_que.h>
#include <osa_prf.h>
#include <include/link_api/sgxDisplayLink.h>
#include "sgxRender1x1.h"
#include "sgxRender2x2.h"
#include "sgxRenderKmsCube.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Max Number of sgxDisplay link instances supported
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGXDISPLAY_LINK_OBJ_MAX                     (1)

/**
 *******************************************************************************
 *
 * \brief Task size for SGXDISPLAY link task
 *
 *******************************************************************************
 */
#define SGXDISPLAY_LINK_TSK_STACK_SIZE              (OSA_TSK_STACK_SIZE_DEFAULT)





/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */



/**
 *******************************************************************************
 *
 *   \brief SgxDisplay link instance object
 *
 *          This structure contains
 *          - All the local data structures
 *          - VPS Data structures required for SgxDisplay driver interfaces
 *          - All fields to support the Link stats and status information
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 linkId;
    /**< placeholder to store the SgxDisplay link Id */

    OSA_TskHndl tsk;
    /**< placeholder to store the SgxDisplay link task handler */

    SgxDisplayLink_CreateParams createArgs;
    /**< placeholder to store the SgxDisplay link create parameters */

    System_LinkInfo inTskInfo;
    /**< Specifies a place holder that describe the LINK information */

    System_LinkQueInfo inQueInfo;
    /**< place holder that describe the output information of the LINK */

    OSA_LatencyStats linkLatency;
    /**< Structure to find out min, max and average latency of the link */

    OSA_LatencyStats srcToLinkLatency;
    /**< Structure to find out min, max and average latency from
     *   source to this link
     */

    OSA_LinkStatistics linkStats;
    /* link specific statistics */

    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */

    System_EglObj eglObj;
    /**< EGL object information */

    System_DrmObj drmObj;
    /**< DRM Display object information */

    SgxRender1x1_Obj render1x1Obj;
    /**< 1x1 rendering prgram obj */

    SgxRender2x2_Obj render2x2Obj;
    /**< 2x2 rendering prgram obj */

    SgxRenderKmsCube_Obj renderKmsCubeObj;
    /**< KMS Cube rendering prgram obj */

} SgxDisplayLink_Obj;

/*******************************************************************************
 *  SgxDisplay Link Private Functions
 *******************************************************************************
 */
Int32 SgxDisplayLink_drvCreate(SgxDisplayLink_Obj *pObj,
                               SgxDisplayLink_CreateParams *pPrm);
Int32 SgxDisplayLink_drvStart(SgxDisplayLink_Obj *pObj);
Int32 SgxDisplayLink_drvDoProcessFrames(SgxDisplayLink_Obj *pObj);
Int32 SgxDisplayLink_drvStop(SgxDisplayLink_Obj *pObj);
Int32 SgxDisplayLink_drvDelete(SgxDisplayLink_Obj *pObj);
Int32 SgxDisplayLink_drvPrintStatistics(SgxDisplayLink_Obj *pObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
