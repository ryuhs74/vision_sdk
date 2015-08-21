/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _SGXRENDER1x1_H_
#define _SGXRENDER1x1_H_

#include <linux/src/system/system_drm_egl.h>

typedef struct
{
   GLuint program;
   GLint  vPositionHandle;
   GLint  yuvTexSamplerHandle;
   GLint  vTexHandle;
} SgxRender1x1_Obj;

int SgxRender1x1_setup(SgxRender1x1_Obj *pObj);
void SgxRender1x1_renderFrame(SgxRender1x1_Obj *pObj, System_EglObj *pEglObj, GLuint texYuv);

/* used by 2x2 rendering and 1x1 rendering intnerally.
 * NOT to be used by sgxDisplayLink directly
 */
void SgxRender1x1_renderFrame1x1(SgxRender1x1_Obj *pObj, System_EglObj *pEglObj, const GLfloat *vertices, GLuint texYuv);

#endif /*   _SGXRENDER1x1_H_    */