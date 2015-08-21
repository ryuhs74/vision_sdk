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
 * \ingroup SGX3DSRV_LINK_API
 * \defgroup SGX3DSRV_LINK_IMPL Sgx3Dsrv Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file sgx3DsrvLink_priv.h Sgx3Dsrv Link private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Sgx3Dsrv link instance/handle object
 *         - All the local data structures
 *         - Sgx3Dsrv driver interfaces
 *
 * \version 0.0 (Sept 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SGX3DSRV_LINK_PRIV_H_
#define _SGX3DSRV_LINK_PRIV_H_

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
#include <include/link_api/sgx3DsrvLink.h>
#include "sgxRender3DSRV.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 *   \brief Max Number of sgx3Dsrv link instances supported
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_OBJ_MAX                     (1)

/**
 *******************************************************************************
 *
 * \brief Task size for SGX3DSRV link task
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_TSK_STACK_SIZE              (OSA_TSK_STACK_SIZE_DEFAULT)

/**
 *******************************************************************************
 *
 *   \brief Max number of elements for local queues
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_MAX_LOCALQUEUELENGTH        (16)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Input Width
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_INPUT_FRAME_WIDTH           (1280)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Input Height
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_INPUT_FRAME_HEIGHT          (720)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Output Height
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_OUTPUT_FRAME_WIDTH          (880)

/**
 *******************************************************************************
 *
 *   \brief SV Algorithm Output Height
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SGX3DSRV_LINK_OUTPUT_FRAME_HEIGHT         (1080)


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Data Structure for the Que of system buffers.
 *
 *   Que handle and the associated memory for queue elements are grouped.
 *
 *******************************************************************************
*/
typedef struct {
    OSA_QueHndl     queHandle;
    /**< Handle to the queue for this channel */
} Sgx3DsrvLink_SysBufferQue;


/**
 *******************************************************************************
 *
 *   \brief Sgx3Dsrv link instance object
 *
 *          This structure contains
 *          - All the local data structures
 *          - VPS Data structures required for Sgx3Dsrv driver interfaces
 *          - All fields to support the Link stats and status information
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32  linkId;
    /**< placeholder to store the Sgx3Dsrv link Id */
    OSA_TskHndl  tsk;
    /**< placeholder to store the Sgx3Dsrv link task handler */
    UInt32  inPitch[SYSTEM_MAX_PLANES];
    /**< Pitch of the input video buffer, This is kept same for all channels */
    UInt32  inDataFormat;
    /**< Data format of the video to operate on */
    UInt32  numInputChannels;
    /**< Number of input channels on input Q (Prev link output Q) */
    Sgx3DsrvLink_CreateParams createArgs;
    /**< placeholder to store the Sgx3Dsrv link create parameters */
    System_LinkInfo inTskInfo[SGX3DSRV_LINK_IPQID_MAXIPQ];
    /**< Specifies a place holder that describe the LINK information */
    System_LinkQueInfo inQueInfo[SGX3DSRV_LINK_IPQID_MAXIPQ];
    /**< place holder that describe the output information of the LINK */
    /**< Payload for System buffers */
    Sgx3DsrvLink_SysBufferQue localInputQ[SGX3DSRV_LINK_IPQID_MAXIPQ];
    /**< Local Qs to hold input */
    /**< Counter to keep track of number of frame drops */
    System_Buffer  *sysBufferGALUT;
    /**< Place holder for the GA LUT sysBuffer. Only one will be held
     * inside Synthesis link at any point in time.
     */
    System_Buffer  *sysBufferBlendLUT;
    /**< Place holder for the Blend LUT sysBuffer. Only one will be held
     * inside Synthesis link at any point in time.
     */
    System_Buffer  *sysBufferPALUT;
    /**< Place holder for the last PA LUT sysBuffer. Only one will be held
     * inside Synthesis link at any point in time.
     */
    Bool  receivedGALUTFlag;
    /**< Flag to check the availabilty of the GA LUT*/
    Bool  receivedBlendLUTFlag;
    /**< Flag to check the availabilty of the Blending LUT*/
    Bool  receivedFirstPALUTFlag;
    /**< Flag to check the availabilty of the fisrt PA LUT*/
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
    SgxRender3DSRV_Obj render3DSRVObj;
    /**< 3D SRV rendering prgram obj */

} Sgx3DsrvLink_Obj;

/*******************************************************************************
 *  Sgx3Dsrv Link Private Functions
 *******************************************************************************
 */
Int32 Sgx3DsrvLink_drvCreate(Sgx3DsrvLink_Obj *pObj,
                               Sgx3DsrvLink_CreateParams *pPrm);
Int32 Sgx3DsrvLink_drvStart(Sgx3DsrvLink_Obj *pObj);
Int32 Sgx3DsrvLink_drvDoProcessFrames(Sgx3DsrvLink_Obj *pObj);
Int32 Sgx3DsrvLink_drvStop(Sgx3DsrvLink_Obj *pObj);
Int32 Sgx3DsrvLink_drvDelete(Sgx3DsrvLink_Obj *pObj);
Int32 Sgx3DsrvLink_drvPrintStatistics(Sgx3DsrvLink_Obj *pObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
