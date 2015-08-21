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
 * \ingroup SAMPLE_LINUX_MODULE_API
 * \defgroup SGX3DSRV_LINK_API Sgx3Dsrv Link API
 *
 * \brief  This module has the interface for using Sgx3Dsrv Link
 *
 *         Sgx3Dsrv Link is used to feed video frames to SGX for
 *         creating the surround view (360 degree view) of the Car.
 *         The rendered output will be pushed to display via DRM.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file sgx3DsrvLink.h
 *
 * \brief Sgx3Dsrv Link API
 *
 * \version 0.0 (Sept 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SGX3DSRV_LINK_H_
#define _SGX3DSRV_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 * \brief Enum for the input Q IDs
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
*/
typedef enum
{
    SGX3DSRV_LINK_IPQID_MULTIVIEW = 0x0,
    /**< QueueId for multiview images */

    SGX3DSRV_LINK_IPQID_PALUT,
    /**< QueueId for PA statistics */

    SGX3DSRV_LINK_IPQID_GALUT,
    /**< QueueId for GA LUTs */

    SGX3DSRV_LINK_IPQID_BLENDLUT,
    /**< QueueId for SGX Blend LUT */

    SGX3DSRV_LINK_IPQID_GRPX,
    /**< QueueId for GRPX Plane buffer */

    SGX3DSRV_LINK_IPQID_VIDMOSAIC,
    /**< QueueId for Mosaic video buffer */

    SGX3DSRV_LINK_IPQID_MAXIPQ,
    /**< Maximum number of input queues */

    SGX3DSRV_LINK_IPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}Sgx3DsrvLink_InputQueId;

/* @} */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing the Sgx3Dsrv link create time parameters
 *
 *          This structure is used to create and configure a Sgx3Dsrv link
 *          instance.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32                   maxOutputHeight;
    /**< Max height of the output (stiched) frame */
    UInt32                   maxOutputWidth;
    /**< max width of the output (stiched) frame */
    UInt32                   maxInputHeight;
    /**< Max height of the input (captured) frame */
    UInt32                   maxInputWidth;
    /**< Max width of the input (captured) frame */
    UInt32                   numViews;
    /**< number of output views will be synthesized */
    UInt32                   numInQue;
    /**< Number of inputs queue's */
    System_BufferType        inBufType[SGX3DSRV_LINK_IPQID_MAXIPQ];
    /**< Input buffer type can be
         SYSTEM_BUFFER_TYPE_METADATA
         OR
         SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER
     */
    System_LinkInQueParams   inQueParams[SGX3DSRV_LINK_IPQID_MAXIPQ];
    /**< Sgx3Dsrv link input queue information */

} Sgx3DsrvLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Sgx3Dsrv link register and init function
 *
 *          For each sgx3Dsrv instance
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Sgx3DsrvLink_init();

/**
 *******************************************************************************
 *
 *   \brief Sgx3Dsrv link de-register and de-init function
 *
 *          For each sgx3Dsrv instance
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Sgx3DsrvLink_deInit();

/**
 *******************************************************************************
 *
 *   \brief Function to initialize the Sgx3Dsrv Link Create Params
 *
 *          Sets default values for Sgx3Dsrv link create time parameters
 *          User/App can override these default values later.
 *
 *   \param prm [IN] Sgx3Dsrv Link create parameters
 *
 *   \return void
 *
 *******************************************************************************
*/
static inline Void Sgx3DsrvLink_CreateParams_Init(
                                Sgx3DsrvLink_CreateParams *prm)
{
    memset(prm, 0, sizeof(*prm));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
