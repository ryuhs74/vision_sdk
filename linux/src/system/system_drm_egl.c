/*
 *******************************************************************************
 *
 * This is being reused from RobClark's eglimage work.
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "system_drm_egl.h"
#include <sys/ioctl.h>
#include <drm_fourcc.h>
#include <osa_mem.h>
#include <omap_drm.h>
#include <omap_drmif.h>

#ifndef EGL_TI_raw_video
#  define EGL_TI_raw_video             1
#  define EGL_RAW_VIDEO_TI             0x333A   /* eglCreateImageKHR target */
#  define EGL_GL_VIDEO_FOURCC_TI       0x3331   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_WIDTH_TI        0x3332   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_HEIGHT_TI       0x3333   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_BYTE_STRIDE_TI  0x3334   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_BYTE_SIZE_TI    0x3335   /* eglCreateImageKHR attribute */
#  define EGL_GL_VIDEO_YUV_FLAGS_TI    0x3336   /* eglCreateImageKHR attribute */
#endif

#ifndef EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE
#  define EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE (0 << 0)
#  define EGLIMAGE_FLAGS_YUV_FULL_RANGE       (1 << 0)
#  define EGLIMAGE_FLAGS_YUV_BT601            (0 << 1)
#  define EGLIMAGE_FLAGS_YUV_BT709            (1 << 1)
#endif

#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_STR(str)    FOURCC(str[0], str[1], str[2], str[3])

/*IDEALLY, these will come in the new libdrm_omap header files*/
struct drm_omap_gem_new_paddr {
   union omap_gem_size size;
   uint32_t flags;
   uint32_t handle;
   uint32_t paddr;
   uint32_t __pad;
};

#define DRM_IOCTL_OMAP_GEM_NEW_PADDR 0xC0146447

/*YES, IDEALLY*/

typedef struct
{
    struct gbm_bo *bo;
    uint32_t fb_id;
    System_DrmObj *pDrmObj;
} System_DrmFb ;

void System_drmFbDestroyCallback(struct gbm_bo *bo, void *data)
{
   System_DrmFb *fb = (System_DrmFb *)data;

   if (fb->fb_id)
       drmModeRmFB(fb->pDrmObj->drmFd, fb->fb_id);

   free(fb);
}

System_DrmFb * System_drmFbGetFromGbmBo(System_DrmObj *pObj, struct gbm_bo *bo)
{
   System_DrmFb *fb = (System_DrmFb *)gbm_bo_get_user_data(bo);
   uint32_t width, height, stride, handle;
   int ret;

   if (fb)
       return fb;

   fb = (System_DrmFb *)calloc(1, sizeof *fb);
   fb->bo = bo;
   fb->pDrmObj = pObj;

   width = gbm_bo_get_width(bo);
   height = gbm_bo_get_height(bo);
   stride = gbm_bo_get_stride(bo);
   handle = gbm_bo_get_handle(bo).u32;

   ret = drmModeAddFB(pObj->drmFd, width, height, 24, 32, stride, handle, &fb->fb_id);
   if (ret) {
       Vps_printf(" DRM: ERROR: drmModeAddFB() failed (%s) !!!\n", strerror(errno));
       free(fb);
       return NULL;
   }

   gbm_bo_set_user_data(bo, fb, System_drmFbDestroyCallback);

   return fb;
}

void System_drmPageFlipHandler(int fd, unsigned int frame,
       unsigned int sec, unsigned int usec, void *data)
{
   int *waiting_for_flip = (int *)data;

   *waiting_for_flip = 0;
}

int System_drmOpen(System_DrmObj *pObj, System_DrmOpenPrms *pPrm)
{
    drmModeRes *resources;
    drmModeConnector *connector = NULL;
    drmModeEncoder *encoder = NULL;
    int i, j;
    Bool display_found = FALSE;

    memset(pObj, 0, sizeof(*pObj));

    pObj->drmFd = drmOpen("omapdrm", NULL);
    if(pObj->drmFd < 0)
    {
        Vps_printf(" DRM: ERROR: drmOpen() failed !!!\n");
        return -1;
    }

    resources = drmModeGetResources(pObj->drmFd);
    if (resources==NULL) {
        Vps_printf(" DRM: ERROR: drmModeGetResources() failed (%s) !!!\n", strerror(errno));
        return -1;
    }
    pObj->resource_id = (uint32_t) resources;

    /* find a connected connector: */
    pObj->numDisplays = 0;
    pObj->curDisplay  = 0;
    for (i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(pObj->drmFd, resources->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED) {
            /* choose the first supported mode */
            pObj->mode[pObj->numDisplays] = &connector->modes[0];
            pObj->connector_id[pObj->numDisplays] = connector->connector_id;

            for (j=0; j<resources->count_encoders; j++) {
                encoder = drmModeGetEncoder(pObj->drmFd, resources->encoders[j]);
                if (encoder->encoder_id == connector->encoder_id)
                   break;

                drmModeFreeEncoder(encoder);
                encoder = NULL;
            }

            if (encoder==NULL) {
                Vps_printf(" DRM: ERROR: drmModeGetEncoder() did not find any "
                          "encoder connected to connector ID [%d] !!!\n",
                          connector->connector_id
                          );
                return -1;
            }

            pObj->encoder[pObj->numDisplays]    = (uint32_t) encoder;
            pObj->crtc_id[pObj->numDisplays]    = encoder->crtc_id;
            pObj->connectors[pObj->numDisplays] = connector;


            /*Lets see what folks tell about this. currently a security check*/
            OSA_assert(pPrm->connector_id == -1 && pPrm->displayWidth != 0 && pPrm->displayHeight != 0);

           /*No connector based selection supported. we dont use it anyway.*/
           /*if (display_found == FALSE && (pPrm->connector_id != -1) && (pPrm->connector_id == pObj->connector_id[pObj->numDisplays]))
           {
               pObj->curDisplay = pObj->numDisplays;
               display_found = TRUE;
           }*/
           if(display_found == FALSE && (pPrm->displayWidth != 0 || pPrm->displayHeight != 0))
           
               for (j = 0; j < connector->count_modes; j++) {
                   if(connector->modes[j].hdisplay == pPrm->displayWidth &&
                           connector->modes[j].vdisplay == pPrm->displayHeight)
                   {
                       pObj->curDisplay = pObj->numDisplays;
                       pObj->mode[pObj->curDisplay] = &connector->modes[j];
                       display_found = TRUE;
                       break;
                   }
               }

        }

        if(display_found == TRUE) {
            break;

        /*if we go on and choose the highest display, Who knows, it can be a 4K as well.*/
        /*We dont have that big a buffer incoming.                                      */

        /*
        if(pPrm->connector_id==-1 &&  pPrm->displayWidth == 0 && pPrm->displayHeight == 0)
        {
            maxRes = pObj->mode[pObj->curDisplay]->vdisplay * pObj->mode[pObj->curDisplay]->hdisplay;
            curRes = pObj->mode[pObj->numDisplays]->vdisplay * pObj->mode[pObj->numDisplays]->hdisplay;
            if (curRes > maxRes)
            {
                pObj->curDisplay = pObj->numDisplays;
            }
        }
        */


            pObj->numDisplays++;
        }
        else
        {
           drmModeFreeConnector(connector);
        }
    }


    if (display_found == FALSE) {
        Vps_printf(" DRM: ERROR: No connected connector found !!!\n");
        return -1;
    }
    else {
            Vps_printf("### Display [%d]: CRTC = %d, Connector = %d\n", pObj->curDisplay, pObj->crtc_id[pObj->curDisplay], pObj->connector_id[pObj->curDisplay]);
            Vps_printf("   Mode chosen [%s] : Clock => %d, Vertical refresh => %d, Type => %d\n", pObj->mode[pObj->curDisplay]->name, pObj->mode[pObj->curDisplay]->clock, pObj->mode[pObj->curDisplay]->vrefresh, pObj->mode[pObj->curDisplay]->type);
            Vps_printf("   Horizontal => %d, %d, %d, %d, %d\n", pObj->mode[pObj->curDisplay]->hdisplay, pObj->mode[pObj->curDisplay]->hsync_start, pObj->mode[pObj->curDisplay]->hsync_end, pObj->mode[pObj->curDisplay]->htotal, pObj->mode[pObj->curDisplay]->hskew);
            Vps_printf("   Vertical => %d, %d, %d, %d, %d\n", pObj->mode[pObj->curDisplay]->vdisplay, pObj->mode[pObj->curDisplay]->vsync_start, pObj->mode[pObj->curDisplay]->vsync_end, pObj->mode[pObj->curDisplay]->vtotal, pObj->mode[pObj->curDisplay]->vscan);
    }

    pObj->canUsePlanes = FALSE;
    drmModePlaneRes *plane_resource_id = drmModeGetPlaneResources(pObj->drmFd);
    if(plane_resource_id)
        pObj->canUsePlanes = TRUE;
    pObj->numPlanes = 0;
    OSA_assert(pObj->canUsePlanes);

    int pcount;
    unsigned int curr_crtc = 0;
    for (pcount = 0; pcount < plane_resource_id->count_planes; pcount++) {
        int ccount;
        drmModePlane *ovr = drmModeGetPlane(pObj->drmFd, plane_resource_id->planes[pcount]);
        for (ccount = 0; ccount < resources->count_crtcs; ccount++) {
            if (pObj->crtc_id[pObj->curDisplay] == resources->crtcs[ccount]) {
                curr_crtc = (1 << ccount);
                break;
            }
        }

        if (ovr->possible_crtcs & curr_crtc) {
            pObj->plane_id[pObj->numPlanes] = ovr->plane_id;
            pObj->numPlanes++;
        }

        drmModeFreePlane(ovr);
    }

    OSA_assert(pObj->numPlanes >= 2);
    pObj->grpx_plane_id    = pObj->plane_id[0];
    pObj->fc_plane_id      = pObj->plane_id[1];

    drmModeFreePlaneResources (plane_resource_id);
    /*this is also too much hardcoding*/
    drmModeObjectSetProperty(pObj->drmFd, pObj->grpx_plane_id, DRM_MODE_OBJECT_PLANE , 7, 3);
    drmModeObjectSetProperty(pObj->drmFd, pObj->fc_plane_id, DRM_MODE_OBJECT_PLANE , 7, 2);

    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 7, 0);

    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 12, 0x00000000);
    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 13, 1);
    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 14, 1);

    pObj->gbm_dev = gbm_create_device(pObj->drmFd);
    if (pObj->gbm_dev==NULL) {
        Vps_printf(" DRM: gbm_create_device() failed !!!\n");
        return -1;
    }

    pObj->gbm_surface = gbm_surface_create(
            pObj->gbm_dev,
            pObj->mode[pObj->curDisplay]->hdisplay,
            pObj->mode[pObj->curDisplay]->vdisplay,
            GBM_FORMAT_XRGB8888,
            GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
        );
    if (pObj->gbm_surface==NULL) {
        Vps_printf(" DRM: gbm_surface_create() failed !!!\n");
        return -1;
    }

    pObj->drmEvtCtx.version = DRM_EVENT_CONTEXT_VERSION;
    pObj->drmEvtCtx.page_flip_handler = System_drmPageFlipHandler;

    FD_ZERO(&pObj->drmFds); //real code
    FD_SET(pObj->drmFd, &pObj->drmFds); //real code



    pObj->omapDev = omap_device_new(pObj->drmFd);

    pObj->waitForFlip = 0;

    return 0;
}

int System_drmClose(System_DrmObj *pObj)
{
    int i;

    gbm_surface_destroy(pObj->gbm_surface);
    gbm_device_destroy(pObj->gbm_dev);

    for(i=0; i<pObj->numDisplays; i++)
    {
        drmModeFreeEncoder((drmModeEncoderPtr)pObj->encoder[i]);
        drmModeFreeConnector(pObj->connectors[i]);
    }

    drmModeFreeResources((drmModeResPtr)pObj->resource_id);

    drmClose(pObj->drmFd);

    return 0;
}

void System_eglPrintGLString(const char *name, GLenum s) {

   const char *v = (const char *) glGetString(s);

   Vps_printf(" EGL: GL %s = %s\n", name, v);
}

void System_eglCheckEglError(const char* op, EGLBoolean returnVal) {
   EGLint error;

   if (returnVal != EGL_TRUE) {
       fprintf(stderr, " EGL: %s() returned %d\n", op, returnVal);
   }

   for (error = eglGetError(); error != EGL_SUCCESS; error = eglGetError()) {
       fprintf(stderr, " EGL: after %s() eglError (0x%x)\n", op, error);
   }
}

void System_eglCheckGlError(const char* op) {
   GLint error;

   for (error = glGetError(); error; error = glGetError()) {
       fprintf(stderr, "GL: after %s() glError (0x%x)\n", op, error);
   }
}

int System_drmSetupFramebuffer(System_DrmObj *pObj, System_DrmFBProperty *pProp, void *bufAddr, int texIndex)
{
    struct drm_omap_gem_new_paddr new_handle;

    if(pProp->dataFormat!=SYSTEM_DF_BGR16_565 && pProp->dataFormat != SYSTEM_DF_YUV420SP_UV)
    {
        OSA_assert(0);
    }

    memset(&new_handle, 0, sizeof(new_handle));

    if(pProp->dataFormat == SYSTEM_DF_BGR16_565)
        new_handle.size.bytes = pProp->pitch[0] * pProp->height;
    else
        new_handle.size.bytes = pProp->pitch[0] * pProp->height + pProp->pitch[0] * pProp->height /2;

    new_handle.flags = OMAP_BO_SCANOUT | OMAP_BO_WC;
    new_handle.paddr = OSA_memVirt2Phys((unsigned int)bufAddr , OSA_MEM_REGION_TYPE_SR1);

    if(ioctl(pObj->drmFd, DRM_IOCTL_OMAP_GEM_NEW_PADDR, &new_handle))
            OSA_assert(0);
    pObj->gem_handle[texIndex] = new_handle.handle;

    unsigned int handles[4];
    handles[0] = new_handle.handle;
    if(pProp->dataFormat == SYSTEM_DF_YUV420SP_UV)
        handles[1] = new_handle.handle;

    unsigned int pitches[4];
    pitches[0] = pProp->pitch[0];
    if(pProp->dataFormat == SYSTEM_DF_YUV420SP_UV)
        pitches[1] = pProp->pitch[1];

    unsigned int offsets[4];
    offsets[0] = 0;
    if(pProp->dataFormat == SYSTEM_DF_YUV420SP_UV)
        offsets[1] = pProp->pitch[0] * pProp->height;

    if(pProp->dataFormat == SYSTEM_DF_BGR16_565) {
        if (drmModeAddFB2(pObj->drmFd, pProp->width,
                                        pProp->height,
                                        DRM_FORMAT_RGB565,
                                        handles, pitches, offsets, &pObj->fb_id[texIndex], 0))
        {
            OSA_assert(0);
        }
    } else {
        if (drmModeAddFB2(pObj->drmFd, pProp->width,
                                        pProp->height,
                                        DRM_FORMAT_NV12,
                                        handles, pitches, offsets, &pObj->fb_id[texIndex], 0))
        {
                        OSA_assert(0);
        }
    }

    pObj->bufAddr[texIndex] = bufAddr;
    return 0;
}

GLuint System_eglSetupYuvTexSurface(System_EglObj *pObj, System_EglTexProperty *pProp, void *bufAddr, int texIndex)
{
    EGLint attr[32];
    int attrIdx;
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

    attrIdx = 0;

    attr[attrIdx++] = EGL_GL_VIDEO_FOURCC_TI;
    attr[attrIdx++] = FOURCC_STR("NV12");

    attr[attrIdx++] = EGL_GL_VIDEO_WIDTH_TI;
    attr[attrIdx++] = pProp->width;

    attr[attrIdx++] = EGL_GL_VIDEO_HEIGHT_TI;
    attr[attrIdx++] = pProp->height;

    attr[attrIdx++] = EGL_GL_VIDEO_BYTE_STRIDE_TI;
    attr[attrIdx++] = pProp->pitch[0];

    attr[attrIdx++] = EGL_GL_VIDEO_BYTE_SIZE_TI;
    if(pProp->dataFormat==SYSTEM_DF_YUV420SP_UV)
    {
        attr[attrIdx++] = (pProp->pitch[0] * pProp->height * 3)/2;
    }
    else
    {
        Vps_printf(" EGL: ERROR: Unsupported data format (%d) !!!\n", pProp->dataFormat);
        OSA_assert(0);
    }

    attr[attrIdx++] = EGL_GL_VIDEO_YUV_FLAGS_TI;
    attr[attrIdx++] = EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE | EGLIMAGE_FLAGS_YUV_BT601;

    attr[attrIdx++] = EGL_NONE;


    eglCreateImageKHR =
        (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

    pObj->texImg[texIndex] = eglCreateImageKHR(
                                pObj->display,
                                EGL_NO_CONTEXT,
                                EGL_RAW_VIDEO_TI,
                                bufAddr,
                                attr
                              );

   System_eglCheckEglError("eglCreateImageKHR", EGL_TRUE);
   if (pObj->texImg[texIndex] == EGL_NO_IMAGE_KHR) {
       Vps_printf(" EGL: ERROR: eglCreateImageKHR failed !!!\n");
       return -1;
   }

   glGenTextures(1, &pObj->texYuv[texIndex]);
   System_eglCheckGlError("eglCreateImageKHR");

   glBindTexture(GL_TEXTURE_EXTERNAL_OES, pObj->texYuv[texIndex]);
   System_eglCheckGlError("glBindTexture");

   glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   System_eglCheckGlError("glTexParameteri");

   glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)pObj->texImg[texIndex]);
   System_eglCheckGlError("glEGLImageTargetTexture2DOES");

   pObj->bufAddr[texIndex] = bufAddr;

   return 0;
}

void System_eglPrintConfiguration(System_EglObj *pObj)
{

   int j;

   #define X(VAL) {VAL, #VAL}
   struct {
       EGLint attribute;
       const char* name;
       } names[] = {
           X(EGL_BUFFER_SIZE),
           X(EGL_ALPHA_SIZE),
           X(EGL_BLUE_SIZE),
           X(EGL_GREEN_SIZE),
           X(EGL_RED_SIZE),
           X(EGL_DEPTH_SIZE),
           X(EGL_STENCIL_SIZE),
           X(EGL_CONFIG_CAVEAT),
           X(EGL_CONFIG_ID),
           X(EGL_LEVEL),
           X(EGL_MAX_PBUFFER_HEIGHT),
           X(EGL_MAX_PBUFFER_PIXELS),
           X(EGL_MAX_PBUFFER_WIDTH),
           X(EGL_NATIVE_RENDERABLE),
           X(EGL_NATIVE_VISUAL_ID),
           X(EGL_NATIVE_VISUAL_TYPE),
           X(EGL_SAMPLES),
           X(EGL_SAMPLE_BUFFERS),
           X(EGL_SURFACE_TYPE),
           X(EGL_TRANSPARENT_TYPE),
           X(EGL_TRANSPARENT_RED_VALUE),
           X(EGL_TRANSPARENT_GREEN_VALUE),
           X(EGL_TRANSPARENT_BLUE_VALUE),
           X(EGL_BIND_TO_TEXTURE_RGB),
           X(EGL_BIND_TO_TEXTURE_RGBA),
           X(EGL_MIN_SWAP_INTERVAL),
           X(EGL_MAX_SWAP_INTERVAL),
           X(EGL_LUMINANCE_SIZE),
           X(EGL_ALPHA_MASK_SIZE),
           X(EGL_COLOR_BUFFER_TYPE),
           X(EGL_RENDERABLE_TYPE),
           X(EGL_CONFORMANT),
   };
   #undef X

   for (j = 0; j < sizeof(names) / sizeof(names[0]); j++) {
       EGLint value = -1;
       EGLint returnVal = eglGetConfigAttrib(pObj->display, pObj->config, names[j].attribute, &value);
       EGLint error = eglGetError();
       if (returnVal && error == EGL_SUCCESS) {
           Vps_printf(" EGL: %s: %d (0x%x)", names[j].name, value, value);
       }
   }
   Vps_printf(" EGL: \n");
}

int System_eglOpen(System_EglObj *pEglObj, System_DrmObj *pDrmObj)
{
    EGLint num_configs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint w, h;
    int ret;

    const EGLint attribs[] = {
       EGL_RED_SIZE, 1,
       EGL_GREEN_SIZE, 1,
       EGL_BLUE_SIZE, 1,
       EGL_ALPHA_SIZE, 0,
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
       EGL_DEPTH_SIZE, 8,
       EGL_NONE
    };
    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    memset(pEglObj, 0, sizeof(*pEglObj));

    pEglObj->display = eglGetDisplay((EGLNativeDisplayType)pDrmObj->gbm_dev);
    System_eglCheckEglError("eglGetDisplay", EGL_TRUE);
    if (pEglObj->display == EGL_NO_DISPLAY) {
       Vps_printf(" EGL: ERROR: eglGetDisplay() returned EGL_NO_DISPLAY !!!\n");
       return -1;
    }

    ret = eglInitialize(pEglObj->display, &majorVersion, &minorVersion);
    System_eglCheckEglError("eglInitialize", ret);
    Vps_printf(" EGL: version %d.%d\n", majorVersion, minorVersion);
    if (ret != EGL_TRUE) {
       Vps_printf(" EGL: eglInitialize() failed !!!\n");
       return -1;
    }

    if (!eglChooseConfig(pEglObj->display, attribs, &pEglObj->config, 1, &num_configs))
    {
       Vps_printf(" EGL: ERROR: eglChooseConfig() failed. Couldn't get an EGL visual config !!!\n");
       return -1;
    }

    pEglObj->surface = eglCreateWindowSurface(
                pEglObj->display,
                pEglObj->config,
                (EGLNativeWindowType)pDrmObj->gbm_surface,
                NULL);
    System_eglCheckEglError("eglCreateWindowSurface", EGL_TRUE);
    if (pEglObj->surface == EGL_NO_SURFACE)
    {
       Vps_printf(" EGL: eglCreateWindowSurface() failed!!!\n");
       return -1;
    }

    pEglObj->context = eglCreateContext(pEglObj->display, pEglObj->config, EGL_NO_CONTEXT, context_attribs);
    System_eglCheckEglError("eglCreateContext", EGL_TRUE);
    if (pEglObj->context == EGL_NO_CONTEXT) {
       Vps_printf(" EGL: eglCreateContext() failed !!!\n");
       return -1;
    }

    ret = eglMakeCurrent(pEglObj->display, pEglObj->surface, pEglObj->surface, pEglObj->context);
    System_eglCheckEglError("eglMakeCurrent", ret);
    if (ret != EGL_TRUE) {
       Vps_printf(" EGL: eglMakeCurrent() failed !!!\n");
       return -1;
    }

    eglQuerySurface(pEglObj->display, pEglObj->surface, EGL_WIDTH, &w);
    System_eglCheckEglError("eglQuerySurface", EGL_TRUE);
    eglQuerySurface(pEglObj->display, pEglObj->surface, EGL_HEIGHT, &h);
    System_eglCheckEglError("eglQuerySurface", EGL_TRUE);

    pEglObj->width = w;
    pEglObj->height = h;

    Vps_printf(" EGL: Window dimensions: %d x %d\n", w, h);

    System_eglPrintGLString("Version", GL_VERSION);
    System_eglPrintGLString("Vendor", GL_VENDOR);
    System_eglPrintGLString("Renderer", GL_RENDERER);
    System_eglPrintGLString("Extensions", GL_EXTENSIONS);

    glViewport(0, 0, w, h);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(pEglObj->display, pEglObj->surface);

    return 0;
}

int System_drmSetModeUserFB(System_DrmObj *pDrmObj, Int32 fb_id)
{
    int ret;

    ret = drmModeSetCrtc(
                pDrmObj->drmFd,
                pDrmObj->crtc_id[pDrmObj->curDisplay],
                fb_id,
                0,
                0,
                &pDrmObj->connector_id[pDrmObj->curDisplay],
                1,
                pDrmObj->mode[pDrmObj->curDisplay]
                );
    if (ret)
    {
        Vps_printf(" DRM: Display [%d] failed to set mode (%s) !!!\n", pDrmObj->curDisplay, strerror(errno));
        return ret;
    }
    return  0;
}

int System_drmSetMode(System_DrmObj *pDrmObj)
{
    System_DrmFb *fb;
    int ret;

    pDrmObj->curGbmBo = gbm_surface_lock_front_buffer(pDrmObj->gbm_surface);
    fb = System_drmFbGetFromGbmBo(pDrmObj, pDrmObj->curGbmBo);

    ret = drmModeSetCrtc(
                pDrmObj->drmFd,
                pDrmObj->crtc_id[pDrmObj->curDisplay],
                fb->fb_id,
                0,
                0,
                &pDrmObj->connector_id[pDrmObj->curDisplay],
                1,
                pDrmObj->mode[pDrmObj->curDisplay]
                );
    if (ret)
    {
        Vps_printf(" DRM: Display [%d] failed to set mode (%s) !!!\n", pDrmObj->curDisplay, strerror(errno));
        return ret;
    }

    return ret;
}

int System_drmEglSwapBuffersUserFB(System_DrmObj *pDrmObj, System_EglObj *pEglObj, Int32 fb_id)
{
    int ret;

    pDrmObj->waitForFlip = 1;

    ret = drmModePageFlip(pDrmObj->drmFd,
                         pDrmObj->crtc_id[pDrmObj->curDisplay],
                         fb_id,
                         DRM_MODE_PAGE_FLIP_EVENT,
                         &pDrmObj->waitForFlip);
    if (ret)
    {
       Vps_printf(" DRM: drmModePageFlip() failed (%s) !!!\n", strerror(errno));
       return -1;
    }

    while (pDrmObj->waitForFlip) {
       ret = select(pDrmObj->drmFd + 1, &pDrmObj->drmFds, NULL, NULL, NULL);
       if (ret < 0) {
           Vps_printf(" DRM: select() failed (%s)\n", strerror(errno));
           return ret;
       } else if (ret == 0) {
           Vps_printf(" DRM: select() timeout !!!\n");
           return -1;
       } else if (FD_ISSET(0, &pDrmObj->drmFds)) {
           continue;
       }
       drmHandleEvent(pDrmObj->drmFd, &pDrmObj->drmEvtCtx);
   }

   return 0;
}
int System_drmEglSwapBuffers(System_DrmObj *pDrmObj, System_EglObj *pEglObj)
{
    System_DrmFb *fb;
    int ret;

    eglSwapBuffers(pEglObj->display, pEglObj->surface);
    System_eglCheckEglError("eglSwapBuffers", EGL_TRUE);

    pDrmObj->nextGbmBo = gbm_surface_lock_front_buffer(pDrmObj->gbm_surface);
    fb = System_drmFbGetFromGbmBo(pDrmObj, pDrmObj->nextGbmBo);

    pDrmObj->waitForFlip = 1;

    /*
     * Here you could also update drm plane layers if you want
     * hw composition
     */
    ret = drmModePageFlip(pDrmObj->drmFd,
                         pDrmObj->crtc_id[pDrmObj->curDisplay],
                         fb->fb_id,
                         DRM_MODE_PAGE_FLIP_EVENT,
                         &pDrmObj->waitForFlip);
    if (ret)
    {
       Vps_printf(" DRM: drmModePageFlip() failed (%s) !!!\n", strerror(errno));
       return -1;
    }

    while (pDrmObj->waitForFlip) {
       ret = select(pDrmObj->drmFd + 1, &pDrmObj->drmFds, NULL, NULL, NULL);
       if (ret < 0) {
           Vps_printf(" DRM: select() failed (%s)\n", strerror(errno));
           return ret;
       } else if (ret == 0) {
           Vps_printf(" DRM: select() timeout !!!\n");
           return -1;
       } else if (FD_ISSET(0, &pDrmObj->drmFds)) {
           continue;
       }
       drmHandleEvent(pDrmObj->drmFd, &pDrmObj->drmEvtCtx);
   }

   /* release last buffer to render on again: */
   gbm_surface_release_buffer(pDrmObj->gbm_surface, pDrmObj->curGbmBo);
   pDrmObj->curGbmBo = pDrmObj->nextGbmBo;

   return 0;
}

int System_drmGetFB(System_DrmObj *pDrmObj, System_DrmFBProperty *pProp, void *bufAddr)
{
    uint32_t fb_id = 0;
    int fbFound = 0, i, status;

    for(i=0; i<pDrmObj->numBuf; i++)
    {
        if(pDrmObj->bufAddr[i]==bufAddr)
        {
            fb_id = pDrmObj->fb_id[i];
            fbFound = 1;
            break;
        }
    }
    if(fbFound==0)
    {
        OSA_assert(i<SYSTEM_DRM_MAX_FBS);

        status = System_drmSetupFramebuffer(
                        pDrmObj,
                        pProp,
                        bufAddr,
                        i
                        );
        if(status!=0)
        {
            Vps_printf(" DRM: ERROR: Unable to create framebuffer [%d] to address [0x%08x] !!!\n", i, (unsigned int)bufAddr);
        }
        OSA_assert(status==0);

        fb_id = pDrmObj->fb_id[i];



        pDrmObj->numBuf++;

    }

    return fb_id;
}
GLuint System_eglGetTexYuv(System_EglObj *pEglObj, System_EglTexProperty *pProp, void *bufAddr)
{
    GLuint texYuv = 0;
    int texFound = 0, i, status;

    for(i=0; i<pEglObj->numBuf; i++)
    {
        if(pEglObj->bufAddr[i]==bufAddr)
        {
            texYuv = pEglObj->texYuv[i];
            texFound = 1;
            break;
        }
    }
    if(texFound==0)
    {
        OSA_assert(i<SYSTEM_EGL_MAX_TEXTURES);

        status = System_eglSetupYuvTexSurface(
                        pEglObj,
                        pProp,
                        bufAddr,
                        i
                        );
        if(status!=0)
        {
            Vps_printf(" EGL: ERROR: Unable to bind texture[%d] to address [0x%08x] !!!\n", i, (unsigned int)bufAddr);
        }
        OSA_assert(status==0);

        texYuv = pEglObj->texYuv[i];

        pEglObj->numBuf++;
    }

    return texYuv;
}

int System_drmEglSetPlane(System_DrmObj *pDrmObj, System_EglObj *pEglObj, int plane_id, int fb_id, int x, int y, int width, int height)
{

    OSA_assert(plane_id < pDrmObj->numPlanes);

    if(drmModeSetPlane(pDrmObj->drmFd, pDrmObj->plane_id[plane_id],
                         pDrmObj->crtc_id[pDrmObj->curDisplay],
                         fb_id, 0,
                         x, y,
                         width, height,
                         0<<16, 0<<16,
                         width<<16, height<<16
                         ))
        OSA_assert(0);
    return 0;
}

void System_drmSetTransparencyKey(System_DrmObj *pObj, int transparencyKey)
{
    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 12, transparencyKey);
    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 13, 1);
    drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 14, 1);
}
void System_drmSetZOrder(System_DrmObj *pObj, unsigned int planeId, unsigned int zorder)
{
    if(planeId == 0)
        drmModeObjectSetProperty(pObj->drmFd, pObj->crtc_id[pObj->curDisplay], DRM_MODE_OBJECT_CRTC , 7, zorder);
    else
        drmModeObjectSetProperty(pObj->drmFd, pObj->plane_id[planeId-1], DRM_MODE_OBJECT_PLANE , 7, zorder);
}
