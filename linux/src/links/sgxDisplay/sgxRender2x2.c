/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "sgxRender2x2.h"

static  const GLfloat gTriangleVertices_topleft[] = {
 		-1.0f, 1.0f, 0.0f,
 		-1.0f, 0.0f, 0.0f,
 		0.0f, 0.0f, 0.0f,
 		0.0f, 1.0f, 0.0f
};

static const GLfloat gTriangleVertices_bottomleft[] = {
 		-1.0f, 0.0f, 0.0f,
 		-1.0f, -1.0f, 0.0f,
 		0.0f, -1.0f, 0.0f,
 		0.0f, 0.0f, 0.0f
};

static const GLfloat gTriangleVertices_bottomright[] = {
 		0.0f, 0.0f, 0.0f,
 		0.0f, -1.0f, 0.0f,
 		1.0f, -1.0f, 0.0f,
 		1.0f, 0.0f, 0.0f
};

static const GLfloat gTriangleVertices_topright[] = {
 		0.0f, 1.0f, 0.0f,
 		0.0f, 0.0f, 0.0f,
 		1.0f, 0.0f, 0.0f,
 		1.0f, 1.0f, 0.0f
};

int SgxRender2x2_setup(SgxRender2x2_Obj *pObj)
{
    SgxRender1x1_setup(&pObj->render1x1Obj);

    return 0;
}

void SgxRender2x2_renderFrame(SgxRender2x2_Obj *pObj, System_EglObj *pEglObj, GLuint texYuv[], UInt16 numTex )
{
    UTILS_assert(numTex==4);

    SgxRender1x1_renderFrame1x1(&pObj->render1x1Obj, pEglObj, gTriangleVertices_topleft, texYuv[0]);
    SgxRender1x1_renderFrame1x1(&pObj->render1x1Obj, pEglObj, gTriangleVertices_topright, texYuv[1]);
    SgxRender1x1_renderFrame1x1(&pObj->render1x1Obj, pEglObj, gTriangleVertices_bottomleft, texYuv[2]);
    SgxRender1x1_renderFrame1x1(&pObj->render1x1Obj, pEglObj, gTriangleVertices_bottomright, texYuv[3]);
}
