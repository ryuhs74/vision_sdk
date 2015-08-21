/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _SGXRENDER2x2_H_
#define _SGXRENDER2x2_H_

/*
 * 2x2 rendering is same as 1x1 except that
 * the rendering triangle co-ordinates are different
 * and we invoke the triangle rendering 4 times, once for each texture
 */

#include <linux/src/system/system_drm_egl.h>
#include "sgxRender1x1.h"

typedef struct
{
    SgxRender1x1_Obj render1x1Obj;

} SgxRender2x2_Obj;

int SgxRender2x2_setup(SgxRender2x2_Obj *pObj);
void SgxRender2x2_renderFrame(SgxRender2x2_Obj *pObj, System_EglObj *pEglObj, GLuint texYuv[], UInt16 numTex );


#endif /*   _SGXRENDER2x2_H_    */