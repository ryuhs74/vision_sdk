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
 * \defgroup SRV2D_INFOADAS_LINK_API Srv2DLink Link API for InfoADAS
 *
 * \brief  This module has the interface for using srv2DLink Link
 *
 *         srv2DLink Link is used to feed video frames to SGX for
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
 * \brief srv2DLink Link API
 *
 * \version 0.0 (Sept 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SRV2DINFOADAS_LINK_H_
#define _SRV2DINFOADAS_LINK_H_
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
    SRV2DINFOADAS_LINK_IPQID_STITCHED_DSP = 0x0,
    /**< QueueId for stitched image */

    SRV2DINFOADAS_LINK_IPQID_MULTIVIEW,
    /**< QueueId for multiview images */

    SRV2DINFOADAS_LINK_IPQID_MAXIPQ,
    /**< Maximum number of input queues */

    SRV2DINFOADAS_LINK_IPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}srv2dInfoAdasLink_InputQueId;
/* @} */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing the srv2DLink link create time parameters
 *
 *          This structure is used to create and configure a srv2DLink link
 *          instance.
 *
 *******************************************************************************
*/
typedef struct
{

    UInt32                   numInQue;
    /**< Number of inputs queue's */
    System_BufferType        inBufType[SRV2DINFOADAS_LINK_IPQID_MAXIPQ];
    System_LinkInQueParams   inQueParams[SRV2DINFOADAS_LINK_IPQID_MAXIPQ];
    /**< srv2dLink link input queue information */
} srv2dInfoAdasLink_CreateParams;
/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief srv2dLink link register and init function
 *
 *          For each sgx3Dsrv instance
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 srv2dInfoAdasLink_init();
/**
 *******************************************************************************
 *
 *   \brief srv2dLink link de-register and de-init function
 *
 *          For each sgx3Dsrv instance
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 srv2dInfoAdasLink_deInit();
/**
 *******************************************************************************
 *
 *   \brief Function to initialize the srv2dLink Link Create Params
 *
 *          Sets default values for srv2dLink link create time parameters
 *          User/App can override these default values later.
 *
 *   \param prm [IN] srv2dLink Link create parameters
 *
 *   \return void
 *
 *******************************************************************************
*/
static inline Void srv2dInfoAdasLink_CreateParams_Init(
                                srv2dInfoAdasLink_CreateParams *prm)
{
    memset(prm, 0, sizeof(*prm));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
