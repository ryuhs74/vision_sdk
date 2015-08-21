/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "sgxRenderKmsCube.h"
#include "esUtil.h"

static const char *gSgxRenderKmsCube_vertexShader =
        "uniform mat4 modelviewMatrix;      \n"
        "uniform mat4 modelviewprojectionMatrix;\n"
        "uniform mat3 normalMatrix;         \n"
        "                                   \n"
        "attribute vec4 in_position;        \n"
        "attribute vec3 in_normal;          \n"
        "attribute vec4 in_color;           \n"
        "attribute vec2 in_texuv;           \n"
        "\n"
        "vec4 lightSource = vec4(2.0, 2.0, 20.0, 0.0);\n"
        "                                   \n"
        "varying float VaryingLight;        \n"
        "varying vec2 vVaryingTexUV;        \n"
        "                                   \n"
        "void main()                        \n"
        "{                                  \n"
        "    gl_Position = modelviewprojectionMatrix * in_position;\n"
        "    vec3 vEyeNormal = normalMatrix * in_normal;\n"
        "    vec4 vPosition4 = modelviewMatrix * in_position;\n"
        "    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;\n"
        "    vec3 vLightDir = normalize(lightSource.xyz - vPosition3);\n"
        "    VaryingLight = max(0.0, dot(vEyeNormal, vLightDir));\n"
        "    vVaryingTexUV = in_texuv;      \n"
        "}                                  \n";

static const char *gSgxRenderKmsCube_fragmentShader =
        "#extension GL_OES_EGL_image_external : require\n"
        "                                   \n"
        "precision mediump float;           \n"
        "                                   \n"
        "uniform samplerExternalOES texture;\n"
        "                                   \n"
        "varying float VaryingLight;        \n"
        "varying vec2 vVaryingTexUV;        \n"
        "                                   \n"
        "void main()                        \n"
        "{                                  \n"
        "    vec4 t = texture2D(texture, vVaryingTexUV);\n"
        "    gl_FragColor = vec4(VaryingLight * t.rgb, 1.0);\n"
        "}                                  \n";

GLuint SgxRenderKmsCube_loadShader(GLenum shaderType, const char* pSource) {
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

GLuint SgxRenderKmsCube_createProgram(const char* pVertexSource, const char* pFragmentSource) {
   GLuint vertexShader = SgxRenderKmsCube_loadShader(GL_VERTEX_SHADER, pVertexSource);
   if (!vertexShader) {
       return 0;
   }

   GLuint pixelShader = SgxRenderKmsCube_loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
   if (!pixelShader) {
       return 0;
   }

   GLuint program = glCreateProgram();
   if (program) {
       glAttachShader(program, vertexShader);
       System_eglCheckGlError("glAttachShader");
       glAttachShader(program, pixelShader);
       System_eglCheckGlError("glAttachShader");

       glBindAttribLocation(program, 0, "in_position");
       glBindAttribLocation(program, 1, "in_normal");
       glBindAttribLocation(program, 2, "in_color");
       glBindAttribLocation(program, 3, "in_texuv");

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

int SgxRenderKmsCube_setup(SgxRenderKmsCube_Obj *pObj)
{
    memset(pObj, 0, sizeof(*pObj));

    pObj->program = SgxRenderKmsCube_createProgram(
                        gSgxRenderKmsCube_vertexShader,
                        gSgxRenderKmsCube_fragmentShader
                     );
    if (pObj->program==0)
    {
       return -1;
    }

    pObj->modelviewmatrix = glGetUniformLocation(pObj->program, "modelviewMatrix");
    pObj->modelviewprojectionmatrix =
        glGetUniformLocation(pObj->program, "modelviewprojectionMatrix");
    pObj->normalmatrix = glGetUniformLocation(pObj->program, "normalMatrix");

    pObj->uniform_texture = glGetUniformLocation(pObj->program, "uniform_texture");

    pObj->distance = 8;
    pObj->fov = 45;

    return 0;
}

void SgxRenderKmsCube_renderFrame(SgxRenderKmsCube_Obj *pObj, System_EglObj *pEglObj, GLuint texYuv)
{
    ESMatrix modelview;
    static const GLfloat vVertices[] = {
            // front
            -1.0f, -1.0f, +1.0f, // point blue
            +1.0f, -1.0f, +1.0f, // point magenta
            -1.0f, +1.0f, +1.0f, // point cyan
            +1.0f, +1.0f, +1.0f, // point white
            // back
            +1.0f, -1.0f, -1.0f, // point red
            -1.0f, -1.0f, -1.0f, // point black
            +1.0f, +1.0f, -1.0f, // point yellow
            -1.0f, +1.0f, -1.0f, // point green
            // right
            +1.0f, -1.0f, +1.0f, // point magenta
            +1.0f, -1.0f, -1.0f, // point red
            +1.0f, +1.0f, +1.0f, // point white
            +1.0f, +1.0f, -1.0f, // point yellow
            // left
            -1.0f, -1.0f, -1.0f, // point black
            -1.0f, -1.0f, +1.0f, // point blue
            -1.0f, +1.0f, -1.0f, // point green
            -1.0f, +1.0f, +1.0f, // point cyan
            // top
            -1.0f, +1.0f, +1.0f, // point cyan
            +1.0f, +1.0f, +1.0f, // point white
            -1.0f, +1.0f, -1.0f, // point green
            +1.0f, +1.0f, -1.0f, // point yellow
            // bottom
            -1.0f, -1.0f, -1.0f, // point black
            +1.0f, -1.0f, -1.0f, // point red
            -1.0f, -1.0f, +1.0f, // point blue
            +1.0f, -1.0f, +1.0f  // point magenta
    };

    static const GLfloat vColors[] = {
            // front
            0.0f,  0.0f,  1.0f, // blue
            1.0f,  0.0f,  1.0f, // magenta
            0.0f,  1.0f,  1.0f, // cyan
            1.0f,  1.0f,  1.0f, // white
            // back
            1.0f,  0.0f,  0.0f, // red
            0.0f,  0.0f,  0.0f, // black
            1.0f,  1.0f,  0.0f, // yellow
            0.0f,  1.0f,  0.0f, // green
            // right
            1.0f,  0.0f,  1.0f, // magenta
            1.0f,  0.0f,  0.0f, // red
            1.0f,  1.0f,  1.0f, // white
            1.0f,  1.0f,  0.0f, // yellow
            // left
            0.0f,  0.0f,  0.0f, // black
            0.0f,  0.0f,  1.0f, // blue
            0.0f,  1.0f,  0.0f, // green
            0.0f,  1.0f,  1.0f, // cyan
            // top
            0.0f,  1.0f,  1.0f, // cyan
            1.0f,  1.0f,  1.0f, // white
            0.0f,  1.0f,  0.0f, // green
            1.0f,  1.0f,  0.0f, // yellow
            // bottom
            0.0f,  0.0f,  0.0f, // black
            1.0f,  0.0f,  0.0f, // red
            0.0f,  0.0f,  1.0f, // blue
            1.0f,  0.0f,  1.0f  // magenta
    };

    static const GLfloat vNormals[] = {
            // front
            +0.0f, +0.0f, +1.0f, // forward
            +0.0f, +0.0f, +1.0f, // forward
            +0.0f, +0.0f, +1.0f, // forward
            +0.0f, +0.0f, +1.0f, // forward
            // back
            +0.0f, +0.0f, -1.0f, // backbard
            +0.0f, +0.0f, -1.0f, // backbard
            +0.0f, +0.0f, -1.0f, // backbard
            +0.0f, +0.0f, -1.0f, // backbard
            // right
            +1.0f, +0.0f, +0.0f, // right
            +1.0f, +0.0f, +0.0f, // right
            +1.0f, +0.0f, +0.0f, // right
            +1.0f, +0.0f, +0.0f, // right
            // left
            -1.0f, +0.0f, +0.0f, // left
            -1.0f, +0.0f, +0.0f, // left
            -1.0f, +0.0f, +0.0f, // left
            -1.0f, +0.0f, +0.0f, // left
            // top
            +0.0f, +1.0f, +0.0f, // up
            +0.0f, +1.0f, +0.0f, // up
            +0.0f, +1.0f, +0.0f, // up
            +0.0f, +1.0f, +0.0f, // up
            // bottom
            +0.0f, -1.0f, +0.0f, // down
            +0.0f, -1.0f, +0.0f, // down
            +0.0f, -1.0f, +0.0f, // down
            +0.0f, -1.0f, +0.0f  // down
    };

    static const GLfloat vTexUVs[] = {
            // front
            0.1f,  0.9f,
            0.9f,  0.9f,
            0.1f,  0.1f,
            0.9f,  0.1f,
            // back
            0.1f,  0.9f,
            0.9f,  0.9f,
            0.1f,  0.1f,
            0.9f,  0.1f,
            // right
            0.1f,  0.9f,
            0.9f,  0.9f,
            0.1f,  0.1f,
            0.9f,  0.1f,
            // left
            0.1f,  0.9f,
            0.9f,  0.9f,
            0.1f,  0.1f,
            0.9f,  0.1f,
            // top
            0.1f,  0.9f,
            0.9f,  0.9f,
            0.1f,  0.1f,
            0.9f,  0.1f,
            // bottom
            0.1f,  0.9f,
            0.9f,  0.9f,
            0.1f,  0.1f,
            0.9f,  0.1f,
    };

    /* clear the color buffer */
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(pObj->program);
    System_eglCheckGlError("glUseProgram");

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, vNormals);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, vColors);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, vTexUVs);
    glEnableVertexAttribArray(3);

    esMatrixLoadIdentity(&modelview);
    esTranslate(&modelview, 0.0f, 0.0f, -pObj->distance);
    esRotate(&modelview, 45.0f + (0.25f * pObj->i), 1.0f, 0.0f, 0.0f);
    esRotate(&modelview, 45.0f - (0.5f * pObj->i), 0.0f, 1.0f, 0.0f);
    esRotate(&modelview, 10.0f + (0.15f * pObj->i), 0.0f, 0.0f, 1.0f);

    pObj->i++;

    GLfloat aspect = (GLfloat)(pEglObj->width) / (GLfloat)(pEglObj->height);

    ESMatrix projection;
    esMatrixLoadIdentity(&projection);
    esPerspective(&projection, pObj->fov, aspect, 1.0f, 10.0f);

    ESMatrix modelviewprojection;
    esMatrixLoadIdentity(&modelviewprojection);
    esMatrixMultiply(&modelviewprojection, &modelview, &projection);

    float normal[9];
    normal[0] = modelview.m[0][0];
    normal[1] = modelview.m[0][1];
    normal[2] = modelview.m[0][2];
    normal[3] = modelview.m[1][0];
    normal[4] = modelview.m[1][1];
    normal[5] = modelview.m[1][2];
    normal[6] = modelview.m[2][0];
    normal[7] = modelview.m[2][1];
    normal[8] = modelview.m[2][2];

    glUniformMatrix4fv(pObj->modelviewmatrix, 1, GL_FALSE, &modelview.m[0][0]);
    glUniformMatrix4fv(pObj->modelviewprojectionmatrix, 1, GL_FALSE, &modelviewprojection.m[0][0]);
    glUniformMatrix3fv(pObj->normalmatrix, 1, GL_FALSE, normal);

    glEnable(GL_CULL_FACE);

    glUniform1i(pObj->uniform_texture, 0);
    System_eglCheckGlError("glUniform1i");
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texYuv);
    System_eglCheckGlError("glBindTexture");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
}
