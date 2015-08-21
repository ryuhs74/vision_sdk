/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _SGXRENDER_KMS_CUBE_H_
#define _SGXRENDER_KMS_CUBE_H_

#include <linux/src/system/system_drm_egl.h>

typedef struct
{
   GLuint program;
   GLint modelviewmatrix, modelviewprojectionmatrix, normalmatrix, uniform_texture;
   GLuint texture_name;
   float distance, fov;
   uint32_t i;

} SgxRenderKmsCube_Obj;

int  SgxRenderKmsCube_setup(SgxRenderKmsCube_Obj *pObj);
void SgxRenderKmsCube_renderFrame(SgxRenderKmsCube_Obj *pObj, System_EglObj *pEglObj, GLuint texYuv);


#endif /*   _SGXRENDER_KMS_CUBE_H_    */