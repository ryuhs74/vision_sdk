/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "sgxRender1x1.h"

static const char gSgxRender1x1_vertexShader[] =
    "attribute vec4 vPosition;\n"
    "attribute vec2 texCoords;\n"
    "varying vec2 yuvTexCoords;\n"
    "void main() {\n"
    "  yuvTexCoords = texCoords;\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char gSgxRender1x1_fragmentShader[] =
   "#extension GL_OES_EGL_image_external : require\n"
   "precision mediump float;\n"
   "uniform samplerExternalOES yuvTexSampler;\n"
   "varying vec2 yuvTexCoords;\n"
   "void main() {\n"
   "  gl_FragColor = texture2D(yuvTexSampler, yuvTexCoords);\n"
   "}\n"
   ;

static const GLfloat gSgxRender1x1_triangleVertices_fullscreen[] = {
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f
 };

static const GLfloat gSgxRender1x1_texCoords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
};

GLuint SgxRender1x1_loadShader(GLenum shaderType, const char* pSource) {
   GLuint shader = glCreateShader(shaderType);
   if (shader) {
       glShaderSource(shader, 1, &pSource, NULL);
       glCompileShader(shader);
       GLint compiled = 0;
       glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
       if (!compiled) {
           GLint infoLen = 0;
           glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
           if (infoLen) {
               char* buf = (char*) malloc(infoLen);
               if (buf) {
                   glGetShaderInfoLog(shader, infoLen, NULL, buf);
                   fprintf(stderr, " GL: Could not compile shader %d:\n%s\n",
                       shaderType, buf);
                   free(buf);
               }
           } else {
               fprintf(stderr, " GL: Guessing at GL_INFO_LOG_LENGTH size\n");
               char* buf = (char*) malloc(0x1000);
               if (buf) {
                   glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                   fprintf(stderr, " GL: Could not compile shader %d:\n%s\n",
                   shaderType, buf);
                   free(buf);
               }
           }
           glDeleteShader(shader);
           shader = 0;
       }
   }
   return shader;
}


GLuint SgxRender1x1_createProgram(const char* pVertexSource, const char* pFragmentSource) {
   GLuint vertexShader = SgxRender1x1_loadShader(GL_VERTEX_SHADER, pVertexSource);
   if (!vertexShader) {
       return 0;
   }

   GLuint pixelShader = SgxRender1x1_loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
   if (!pixelShader) {
       return 0;
   }

   GLuint program = glCreateProgram();
   if (program) {
       glAttachShader(program, vertexShader);
       System_eglCheckGlError("glAttachShader");
       glAttachShader(program, pixelShader);
       System_eglCheckGlError("glAttachShader");
       glLinkProgram(program);
       GLint linkStatus = GL_FALSE;
       glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
       if (linkStatus != GL_TRUE) {
           GLint bufLength = 0;
           glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
           if (bufLength) {
               char* buf = (char*) malloc(bufLength);
               if (buf) {
                   glGetProgramInfoLog(program, bufLength, NULL, buf);
                   fprintf(stderr, " GL: Could not link program:\n%s\n", buf);
                   free(buf);
               }
           }
           glDeleteProgram(program);
           program = 0;
       }
   }
   return program;
}

int SgxRender1x1_setup(SgxRender1x1_Obj *pObj)
{
    pObj->program = SgxRender1x1_createProgram(
                        gSgxRender1x1_vertexShader,
                        gSgxRender1x1_fragmentShader
                     );
    if (pObj->program==0)
    {
       return -1;
    }

    pObj->vPositionHandle = glGetAttribLocation(pObj->program, "vPosition");
    System_eglCheckGlError("glGetAttribLocation");

    pObj->yuvTexSamplerHandle = glGetUniformLocation(pObj->program, "yuvTexSampler");
    System_eglCheckGlError("glGetUniformLocation");

    pObj->vTexHandle = glGetAttribLocation(pObj->program, "texCoords");
    System_eglCheckGlError("glGetAttribLocation");

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    System_eglCheckGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    System_eglCheckGlError("glClear");

    glUseProgram(pObj->program);
    System_eglCheckGlError("glUseProgram");

    return 0;
}

void SgxRender1x1_renderFrame1x1(SgxRender1x1_Obj *pObj, System_EglObj *pEglObj, const GLfloat *vertices, GLuint texYuv)
{
    glVertexAttribPointer(pObj->vPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    System_eglCheckGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(pObj->vPositionHandle);
    System_eglCheckGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(pObj->vTexHandle, 2, GL_FLOAT, GL_FALSE, 0, gSgxRender1x1_texCoords);
    System_eglCheckGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(pObj->vTexHandle);
    System_eglCheckGlError("glEnableVertexAttribArray");

    glUniform1i(pObj->yuvTexSamplerHandle, 0);
    System_eglCheckGlError("glUniform1i");
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texYuv);
    System_eglCheckGlError("glBindTexture");

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    System_eglCheckGlError("glDrawArrays");
}

void SgxRender1x1_renderFrame(SgxRender1x1_Obj *pObj, System_EglObj *pEglObj, GLuint texYuv)
{
    SgxRender1x1_renderFrame1x1(pObj, pEglObj, gSgxRender1x1_triangleVertices_fullscreen, texYuv);
}
