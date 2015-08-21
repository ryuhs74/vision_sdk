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
 * \ingroup SRV2D_LINK_API
 * \defgroup SRV2D_LINK_IMPL srv2dLink Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file srv2dLink_priv.h srv2dLink Link private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - srv2dLink link instance/handle object
 *         - All the local data structures
 *         - srv2dLink driver interfaces
 *
 * \version 0.0 (Sept 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SRV2DINFOADAS_LINK_PRIV_H_
#define _SRV2DINFOADAS_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <linux/src/system/system_priv_common.h>
#include <osa.h>
#include <osa_mutex.h>
#include <osa_que.h>
#include <osa_prf.h>
#include <include/link_api/srv2dInfoadasLink.h>
#include <scene_mgr.h>
#include <link_proxy.h>

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
#define SRV2DINFOADAS_LINK_OBJ_MAX                     (1)
/**
 *******************************************************************************
 *
 *   \brief Max Number of surfaces supported
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SRV2DINFOADAS_LINK_MAX_SURFACES                     (5)
/**
 *******************************************************************************
 *
 * \brief Task size for SRV2D link task
 *
 *******************************************************************************
 */
#define SRV2DINFOADAS_LINK_TSK_STACK_SIZE              (OSA_TSK_STACK_SIZE_DEFAULT)

/**
 *******************************************************************************
 *
 *   \brief Max number of elements for local queues
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SRV2DINFOADAS_LINK_MAX_LOCALQUEUELENGTH        (16)

#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_STR(str)    FOURCC(str[0], str[1], str[2], str[3])


/**
 *******************************************************************************
 *
 *   \brief srv2dLink link instance object
 *
 *          This structure contains
 *          - All the local data structures
 *          - VPS Data structures required for srv2dLink driver interfaces
 *          - All fields to support the Link stats and status information
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32  linkId;
    /**< placeholder to store the srv2dLink link Id */
    OSA_TskHndl  tsk;

    OSA_MutexHndl lock;
    
    /**< Number of input channels on input Q (Prev link output Q) */
    srv2dInfoAdasLink_CreateParams createArgs;
    /**< placeholder to store the srv2dLink link create parameters */
    System_LinkInfo inTskInfo[SRV2DINFOADAS_LINK_IPQID_MAXIPQ];
    /**< Specifies a place holder that describe the LINK information */
    System_LinkQueInfo inQueInfo[SRV2DINFOADAS_LINK_IPQID_MAXIPQ];
    /**< place holder that describe the output information of the LINK */
    
    struct scene_buffer_object multiView;
	
    struct scene_buffer_object stitched;


    UInt32 scene_id;

    UInt32 num_surfaces;

    UInt32 surface_Id[SRV2DINFOADAS_LINK_MAX_SURFACES];

} srv2dInfoAdasLink_Obj;

/*******************************************************************************
 *  srv2dLink Link Private Functions
 *******************************************************************************
 */
Int32 srv2dInfoAdasLink_drvCreate(srv2dInfoAdasLink_Obj *pObj,
                               srv2dInfoAdasLink_CreateParams *pPrm);
Int32 srv2dInfoAdasLink_drvStart(srv2dInfoAdasLink_Obj *pObj);
Int32 srv2dInfoAdasLink_drvDoProcessFrames(srv2dInfoAdasLink_Obj *pObj);
Int32 srv2dInfoAdasLink_drvStop(srv2dInfoAdasLink_Obj *pObj);
Int32 srv2dInfoAdasLink_drvDelete(srv2dInfoAdasLink_Obj *pObj);
Void srv2dInfoAdasLink_getSurfaceIds(UInt32 num_surfaces, UInt32* surfaceIds);
Void srv2dInfoAdasLink_putEmptyBuffers(Void* buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
