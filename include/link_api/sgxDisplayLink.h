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
 * \defgroup SGXDISPLAY_LINK_API SgxDisplay Link API
 *
 * \brief  This module has the interface for using SgxDisplay Link
 *
 *         SgxDisplay Link is used to feed video frames to SGX for
 *         rendering.
 *         The rendered output will be pushed to display via DRM.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file sgxDisplayLink.h
 *
 * \brief SgxDisplay Link API
 *
 * \version 0.0 (Jun 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _SGXDISPLAY_LINK_H_
#define _SGXDISPLAY_LINK_H_

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

/* @} */


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

typedef enum {

    SGXDISPLAY_RENDER_TYPE_1x1,
    /**< Display video rendered full-screen on the display */

    SGXDISPLAY_RENDER_TYPE_2x2,
    /**< Display video rendered full-screen as 2x2 mosiac on the display */

    SGXDISPLAY_RENDER_TYPE_3D_CUBE,
    /**< Display video rendered as a rotating 3D cube */

    SGXDISPLAY_RENDER_TYPE_MAX,
    /**< Max value for this enum */

    SGXDISPLAY_RENDER_TYPE_FORCE_32BITS = 0x7FFFFFFF
    /**< value to force enum to be 32-bit */

} SgxDisplay_RenderType;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing the SgxDisplay link create time parameters
 *
 *          This structure is used to create and configure a SgxDisplay link
 *          instance.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 displayWidth;
    /**< Display width */

    UInt32 displayHeight;
    /**< Display height */

    SgxDisplay_RenderType    renderType;
    /**< type of rendering to do using OpenGL */

    System_BufferType inBufType;
    /**< Input buffer type can be
     *   SYSTEM_BUFFER_TYPE_VIDEO_FRAME
     *   or
     *   SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER
     */

    System_LinkInQueParams   inQueParams;
    /**< SgxDisplay link input queue information */

} SgxDisplayLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief SgxDisplay link register and init function
 *
 *          For each sgxDisplay instance (VID1, VID2, VID3 or GRPX1)
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SgxDisplayLink_init();

/**
 *******************************************************************************
 *
 *   \brief SgxDisplay link de-register and de-init function
 *
 *          For each sgxDisplay instance (VID1, VID2, VID3 or GRPX1)
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SgxDisplayLink_deInit();

/**
 *******************************************************************************
 *
 *   \brief Function to initialize the SgxDisplay Link Create Params
 *
 *          Sets default values for SgxDisplay link create time parameters
 *          User/App can override these default values later.
 *
 *   \param prm [IN] SgxDisplay Link create parameters
 *
 *   \return void
 *
 *******************************************************************************
*/
static inline Void SgxDisplayLink_CreateParams_Init(
                                  SgxDisplayLink_CreateParams *prm)
{
    memset(prm, 0, sizeof(*prm));

    prm->renderType = SGXDISPLAY_RENDER_TYPE_1x1;
    prm->inBufType = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
