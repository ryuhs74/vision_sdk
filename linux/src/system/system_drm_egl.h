/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_DRM_EGL_H_
#define _SYSTEM_DRM_EGL_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <osa.h>
#include <include/link_api/system_const.h>

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sched.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <linux/src/system/gbm/gbm.h>



/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#define SYSTEM_DRM_MAX_DISPLAYS     (4)
#define SYSTEM_DRM_MAX_PLANES       (4)

#define SYSTEM_EGL_MAX_TEXTURES    (100)
#define SYSTEM_DRM_MAX_FBS         (100)





/*******************************************************************************
 *  Enums
 *******************************************************************************
 */




/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

typedef struct
{
    int drmFd;
    uint32_t numDisplays;
    uint32_t curDisplay;
    uint32_t crtc_id[SYSTEM_DRM_MAX_DISPLAYS];
    uint32_t connector_id[SYSTEM_DRM_MAX_DISPLAYS];
    uint32_t resource_id;
    uint32_t encoder[SYSTEM_DRM_MAX_DISPLAYS];
    drmModeModeInfo *mode[SYSTEM_DRM_MAX_DISPLAYS];
    drmModeConnector *connectors[SYSTEM_DRM_MAX_DISPLAYS];
    uint32_t plane_id[SYSTEM_DRM_MAX_PLANES];
    int numPlanes;
    Bool canUsePlanes;


    uint32_t fb_id[SYSTEM_DRM_MAX_FBS];
    uint32_t gem_handle[SYSTEM_DRM_MAX_FBS];
    Void *bufAddr[SYSTEM_DRM_MAX_FBS];
    int numBuf;

    struct gbm_device *gbm_dev;
    struct gbm_surface *gbm_surface;

    drmEventContext drmEvtCtx;

    fd_set drmFds;

    struct gbm_bo *curGbmBo;
    struct gbm_bo *nextGbmBo;

    Int32 current_fb;

    int waitForFlip;
 
    struct omap_device *omapDev;

    int grpx_plane_id;
    int fc_plane_id;

} System_DrmObj;

typedef struct
{
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;

    GLuint      texYuv[SYSTEM_EGL_MAX_TEXTURES];
    EGLImageKHR texImg[SYSTEM_EGL_MAX_TEXTURES];
    Void        *bufAddr[SYSTEM_EGL_MAX_TEXTURES];
    int numBuf;

    int width;
    int height;

} System_EglObj;

typedef struct {

    uint32_t connector_id;
    uint32_t displayWidth;
    uint32_t displayHeight;

} System_DrmOpenPrms;

typedef struct {

    System_VideoDataFormat dataFormat;
    /**< SUPPORTED Formats,
     *    SYSTEM_DF_YUV420SP_UV
     */

    UInt32 width;
    /**< in pixels */

    UInt32 height;
    /**< in lines */

    UInt32 pitch[SYSTEM_MAX_PLANES];
    /**< in bytes, only pitch[0] used right now */

} System_EglTexProperty;

 typedef struct {

    System_VideoDataFormat dataFormat;
    /**< SUPPORTED Formats,
     *    SYSTEM_DF_BGR565
     *    SYSTEM_DF_YUV420SP_UV
     */

    UInt32 width;
    /**< in pixels */

    UInt32 height;
    /**< in lines */

    UInt32 pitch[SYSTEM_MAX_PLANES];
    /**< in bytes, only pitch[0] used right now */

} System_DrmFBProperty;

/**
 *******************************************************************************
 * \brief Function's
 *******************************************************************************
 */

int System_drmOpen(System_DrmObj *pObj, System_DrmOpenPrms *pPrm);
int System_drmSetMode(System_DrmObj *pDrmObj);
int System_drmSetModeUserFB(System_DrmObj *pDrmObj, Int32 fb_id);
int System_drmClose(System_DrmObj *pObj);

int System_eglOpen(System_EglObj *pEglObj, System_DrmObj *pDrmObj);

GLuint System_eglGetTexYuv(System_EglObj *pEglObj, System_EglTexProperty *pProp, void *bufAddr);
int System_drmGetFB(System_DrmObj *pDrmObj, System_DrmFBProperty *pProp, void *bufAddr);

void System_eglPrintGLString(const char *name, GLenum s);
void System_eglCheckEglError(const char* op, EGLBoolean returnVal);
void System_eglCheckGlError(const char* op);
void System_eglPrintConfiguration(System_EglObj *pObj);

int System_drmEglSwapBuffers(System_DrmObj *pDrmObj, System_EglObj *pEglObj);
int System_drmEglSwapBuffersUserFB(System_DrmObj *pDrmObj, System_EglObj *pEglObj, Int32 fb_id);
int System_drmEglSetPlane(System_DrmObj *pDrmObj, System_EglObj *pEglObj, int plane_id, int fb_id, int x, int y, int width, int height);

void System_drmSetTransparencyKey(System_DrmObj *pDrmObj, int transparencyKey);
void System_drmSetZOrder(System_DrmObj *pDrmObj, unsigned int planeId, unsigned int zorder);

#endif

/* @} */
