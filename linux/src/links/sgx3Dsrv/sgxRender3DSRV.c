/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "sgxRender3DSRV.h"
static int stride = 3+2+2+1+1;
static void * prevLUT=(void *)0xdead;
static void * prevblendLUT=(void *)0xbeef;

#define QUADRANTS 4
#define QUADRANT_WIDTH ((POINTS_WIDTH/2)+1)
#define QUADRANT_HEIGHT ((POINTS_HEIGHT/2)+1)

srv_coords_t srv_coords[] = {
{0.000000, 0.100000, -3.300000, 0.00000, 0.00000, 0.00000, 0.10000, 0.000000, 0.000000, -4.500000, 0.000000, 0.000000, 0.000000, 1.799999},
{0.000000, 0.100000, -3.300000, 0.00000, 0.00000, 0.00000, 0.10000, 0.000000, 0.000000, -4.500000, 0.000000, 0.000000, 0.000000, 1.799999},
{0.000000, -0.500000, -4.500000, 84.20000, -0.50000, 0.00000, 0.50000, 0.10000, -0.300000, -1.900000, 80.10000, 0.000000, 0.000000, 1.0000}
};

//		{0.000000, 0.100000, -2.500000, 0.000000, 0.000000, 0.000000, -0.000000, 0.000000, 0.000000, -4.500000, 0.000000, 0.000000, 0.000000, 0.999999},
//		{0.000000, 0.100000, -2.500000, 0.000000, 0.000000, 0.000000, 0.060000, 0.000000, 0.000000, -4.500000, 0.000000, 0.000000, 0.000000, 1.799999},
//		{0.000000, 0.100000, -2.500000, 70.800026, 0.000001, 0.000000, 0.100000, 0.000000, 0.000000, -4.500000, 64.199982, 0.000001, -0.000001, 2.299998}
//};

float car_x = 0;
float car_y = 0;
float car_z = -2.5;
float bowl_x = 0;
float bowl_y = 0;
float bowl_z = -4.5;
float car_anglex = 0;
float car_angley = 0;
float car_anglez = 0;
float bowl_anglex = 0;
float bowl_angley = 0;
float bowl_anglez = 0;
float car_scale = 0;
float bowl_scale = 0;

GLenum render_mode = GL_TRIANGLE_STRIP;

//#define ENABLE_CAR_SIDE_PANES 1
//#define ENABLE_GLOBAL_BLENDING 1
#define ENABLE_BOWL_ROTATION 1
#define ENABLE_SGX_RENDERED_PREVIEW 1

//Layout
#define LAYOUT_NUM_SINGLEVIEWS_HORIZ 2
#define LAYOUT_NUM_SINGLEVIEWS_VERT 2
//Index generation logic
#define MAX_POINTS_WIDTH 440
#define MAX_POINTS_HEIGHT 540

//Mesh splitting logic
#define MAX_VBO_MESH_SPLIT 8
static GLuint vboId[MAX_VBO_MESH_SPLIT*3];

#ifdef ENABLE_BOWL_ROTATION
static GLfloat bowl_matrix[16];
#endif

static const char gSgxRender3DSRV_vertexShader_1VBO[] =
#ifdef TEXTURE_IN_VERTEX_SHADER
#ifndef STANDALONE
        " #extension GL_OES_EGL_image_external : require \n"
#endif
#endif
        "  attribute vec3 aVertexPosition;\n "
        "  attribute vec2 aTextureCoord1;\n "
        "  attribute vec2 aTextureCoord2;\n "
        "  attribute vec2 blendVals;\n "
        "  uniform mat4 uMVMatrix;\n "
#ifdef TEXTURE_IN_VERTEX_SHADER
#ifndef STANDALONE
        "  uniform samplerExternalOES uSampler[2];\n "
#else
        "  uniform sampler2D uSampler[2];\n "
#endif
        "  mediump float normTextureX;\n"
        "  mediump float normTextureY;\n"
        "  mediump float normTexture1X;\n"
        "  mediump float normTexture1Y;\n"
        " vec4 iFragColor1; \n "
        " vec4 iFragColor2; \n "
        " varying vec4 outColor; \n "
#else
        "  varying vec2 outNormTexture;\n "
        "  varying vec2 outNormTexture1;\n "
        "  varying vec2 outBlendVals;\n "
#endif
        "  mediump float RANGE_X = 220.0; \n "
        "  mediump float RANGE_Y = 270.0; \n "
        "  mediump float TEXTURE_X = 1280.0*16.0; \n "
        "  mediump float TEXTURE_Y = 720.0*16.0;\n "

 " void main(void) {\n "
 "     gl_Position = uMVMatrix * vec4((-RANGE_X+aVertexPosition.x), (RANGE_Y-aVertexPosition.y), aVertexPosition.z, 1.0);\n "
#ifdef TEXTURE_IN_VERTEX_SHADER
 "     normTextureX = aTextureCoord1.t/TEXTURE_X;\n"
"     normTextureY = aTextureCoord1.s/TEXTURE_Y;\n"
  "   iFragColor1 = texture2D(uSampler[0],vec2(normTextureX, normTextureY));                         \n  "
 "     normTexture1X = aTextureCoord2.t/TEXTURE_X;\n"
"     normTexture1Y = aTextureCoord2.s/TEXTURE_Y;\n"
		"iFragColor2 = texture2D(uSampler[1],vec2(normTexture1X, normTexture1Y));                         \n  "
 "      outColor = (blendVals.x)*iFragColor1 + (blendVals.y)*iFragColor2;\n "
#else
		"     outNormTexture.x = aTextureCoord1.t/TEXTURE_X;\n"
		"     outNormTexture.y = aTextureCoord1.s/TEXTURE_Y;\n"
		"     outNormTexture1.x = aTextureCoord2.t/TEXTURE_X;\n"
		"     outNormTexture1.y = aTextureCoord2.s/TEXTURE_Y;\n"
		"     outBlendVals = blendVals;\n"
#endif
 "}\n"
;

static const char gSgxRender3DSRV_fragmentShader_1VBO[] =
#ifndef TEXTURE_IN_VERTEX_SHADER
#ifndef STANDALONE
        " #extension GL_OES_EGL_image_external : require \n"
#endif
#endif
        " precision mediump float;\n "
#ifndef TEXTURE_IN_VERTEX_SHADER
#ifndef STANDALONE
        " uniform samplerExternalOES uSampler[2];\n "
#else
        " uniform sampler2D uSampler[2];\n "
#endif
        " varying vec2 outNormTexture;\n "
        " varying vec2 outNormTexture1;\n "
        " varying vec2 outBlendVals;\n "
		" vec4 iFragColor1; \n "
        " vec4 iFragColor2; \n "
#else
        " varying vec4 outColor;\n"
#endif
        " void main(){\n"
#ifndef TEXTURE_IN_VERTEX_SHADER
		"     iFragColor1 = texture2D(uSampler[0], outNormTexture);\n "
		"     iFragColor2 = texture2D(uSampler[1], outNormTexture1);\n "
        "     gl_FragColor = (outBlendVals.x)*iFragColor1 + (outBlendVals.y)*iFragColor2;\n "
#else
        "     gl_FragColor = outColor;\n"
#endif
        " }\n"
   ;

#ifdef STANDALONE
void System_eglCheckEglError(const char* op, EGLBoolean returnVal) {
   EGLint error;

   if (returnVal != EGL_TRUE) {
       printf(" EGL: %s() returned %d\n", op, returnVal);
   }

   for (error = eglGetError(); error != EGL_SUCCESS; error = eglGetError()) {
       printf(" EGL: after %s() eglError (0x%x)\n", op, error);
   }
}

void System_eglCheckGlError(const char* op) {
   GLint error;

   for (error = glGetError(); error; error = glGetError()) {
       printf("GL: after %s() glError (0x%x)\n", op, error);
   }
}
#endif

unsigned short index_buffer[542*442*3];
unsigned int index_buf_length;
void generate_indices(int xlength, int ylength)
{
       unsigned int x, y, k=0;
       for (y=0; y<ylength-1; y++)
       {
               if(y>0)
                       index_buffer[k++]=(unsigned short) (y*xlength);
               for (x=0; x<xlength; x++)
               {
                       index_buffer[k++]=(unsigned short) (y*xlength + x);
                       index_buffer[k++]=(unsigned short) ((y+1)*xlength +x);
               }
               if(y < ylength - 2)
                       index_buffer[k++]=(unsigned short) ((y+1)*xlength + (xlength -1));
       }
       index_buf_length = k;
}



int generate_short_indices(int width, int height, unsigned short* out)
{
    int i, j;
    unsigned int indexlen = 0;
    if(width > MAX_POINTS_WIDTH) return -1;
    if(height > MAX_POINTS_HEIGHT) return -1;
    for(i = 0;i < height-1;i ++)
    {
        int inc = i*width;
        for(j = 0;j < width-1;j ++)
        {
            out[indexlen + 0] = 0+inc;
            out[indexlen + 1] = width+inc;
            out[indexlen + 2] = width+1+inc;
            out[indexlen + 3] = 0+inc;
            out[indexlen + 4] = width+1+inc;
            out[indexlen + 5] = 1+inc;
            inc ++;
            indexlen += 6;
        }
    }
    return 0;
}   
   
GLuint SgxRender3DSRV_loadShader(GLenum shaderType, const char* pSource) {
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
                   D_PRINTF(" GL: Could not compile shader %d:\n%s\n",
                       shaderType, buf);
                   free(buf);
               }
           } else {
               D_PRINTF(" GL: Guessing at GL_INFO_LOG_LENGTH size\n");
               char* buf = (char*) malloc(0x1000);
               if (buf) {
                   glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                   D_PRINTF(" GL: Could not compile shader %d:\n%s\n",
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

GLuint SgxRender3DSRV_createProgram(const char* pVertexSource, const char* pFragmentSource) {
   GLuint vertexShader = SgxRender3DSRV_loadShader(GL_VERTEX_SHADER, pVertexSource);
   if (!vertexShader) {
       return 0;
   }

   GLuint pixelShader = SgxRender3DSRV_loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
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
                   D_PRINTF(" GL: Could not link program:\n%s\n", buf);
                   free(buf);
               }
           }
           glDeleteProgram(program);
           program = 0;
       }
   }
   if(vertexShader && pixelShader && program)
   {
     glDeleteShader(vertexShader);
     glDeleteShader(pixelShader);
    }
   return program;
}

//Vertices init for surround view (VBO approach)
static int surroundview_init_vertices_vbo(SgxRender3DSRV_Obj *pObj, GLuint vertexId, GLuint blendId, GLuint indexId, 
                                          void* vertexBuff, void* blendBuff, void* indexBuff,
                                          int vertexBuffSize, int blendBuffSize, int indexBuffSize
                                          )
{  
     //upload the vertex and texture and image index interleaved array
     //Bowl LUT - Interleaved data (5 data)
     glBindBuffer(GL_ARRAY_BUFFER, vertexId);

     glBufferData(GL_ARRAY_BUFFER, vertexBuffSize, vertexBuff, GL_STATIC_DRAW);
     glVertexAttribPointer(pObj->vertexPositionAttribLoc, 3, GL_LUT_DATATYPE, GL_FALSE, stride*sizeof(LUT_DATATYPE), 0);     
     
     glVertexAttribPointer(pObj->vertexTexCoord1AttribLoc, 2, GL_LUT_DATATYPE, GL_FALSE, (stride)*sizeof(LUT_DATATYPE), (GLvoid*)(3*sizeof(LUT_DATATYPE)));
     glVertexAttribPointer(pObj->vertexTexCoord2AttribLoc, 2, GL_LUT_DATATYPE, GL_FALSE, (stride)*sizeof(LUT_DATATYPE), (GLvoid*)(5*sizeof(LUT_DATATYPE)));
     GL_CHECK(glVertexAttribPointer);

     //blend LUT
     glBindBuffer(GL_ARRAY_BUFFER, blendId);
     glBufferData(GL_ARRAY_BUFFER, blendBuffSize, blendBuff, GL_STATIC_DRAW);
     glVertexAttribPointer(pObj->blendAttribLoc, 2, GL_BLENDLUT_DATATYPE, GL_TRUE, 2*sizeof(char), 0);
     GL_CHECK(glVertexAttribPointer);
                                                 
     //Index buffer
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffSize, indexBuff, GL_STATIC_DRAW);
     GL_CHECK(glBufferData);

     //Enable for the rendering
     glEnableVertexAttribArray(pObj->vertexPositionAttribLoc);
     glEnableVertexAttribArray(pObj->vertexTexCoord1AttribLoc);
     glEnableVertexAttribArray(pObj->vertexTexCoord2AttribLoc);

     glEnableVertexAttribArray(pObj->blendAttribLoc);     

     return 0;
}

void surroundview_init_vertices_vbo_wrap(SgxRender3DSRV_Obj *pObj)
{
 int i;
 int vertexoffset = 0, blendoffset = 0;
 
    generate_indices(QUADRANT_WIDTH, QUADRANT_HEIGHT);
	glGenBuffers(QUADRANTS*3, vboId);
	for(i = 0;i < QUADRANTS;i ++)
	{
		vertexoffset = i * (sizeof(LUT_DATATYPE)*stride*QUADRANT_WIDTH*QUADRANT_HEIGHT);
		blendoffset = i * (sizeof(BLENDLUT_DATATYPE)*2*QUADRANT_WIDTH*QUADRANT_HEIGHT);

		surroundview_init_vertices_vbo(
     pObj,
			vboId[i*3], vboId[i*3+1], vboId[i*3+2],
			(char*)pObj->LUT3D + vertexoffset,
			(char*)pObj->blendLUT3D + blendoffset, 
			(char*)index_buffer,
			sizeof(LUT_DATATYPE)*stride*QUADRANT_WIDTH*QUADRANT_HEIGHT,
			sizeof(BLENDLUT_DATATYPE)*2*QUADRANT_WIDTH*QUADRANT_HEIGHT,
			sizeof(short)*index_buf_length
			);
	}
}

#define RANGE_X_MESH 220.0
#define RANGE_Y_MESH 270.0
#define RANGE_Z_MESH 430.0
void onscreen_mesh_state_restore_program_textures_attribs(SgxRender3DSRV_Obj *pObj, GLuint *texYuv, int tex1, int tex2)
{
     //set the program we need
     glUseProgram(pObj->uiProgramObject);

     glUniform1i(pObj->samplerLocation0, 0);
     glActiveTexture(GL_TEXTURE0);

#ifndef STANDALONE
     glBindTexture(GL_TEXTURE_EXTERNAL_OES, texYuv[tex1]);
     glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
     glBindTexture(GL_TEXTURE_2D, texYuv[tex1]);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
     GL_CHECK(glBindTexture);

     glUniform1i(pObj->samplerLocation1, 1);  
     glActiveTexture(GL_TEXTURE1);

#ifndef STANDALONE
     glBindTexture(GL_TEXTURE_EXTERNAL_OES, texYuv[tex2]);
     glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
     glBindTexture(GL_TEXTURE_2D, texYuv[tex2]);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
     GL_CHECK(glBindTexture);

     //Enable the attributes
     glEnableVertexAttribArray(pObj->vertexPositionAttribLoc);
     glEnableVertexAttribArray(pObj->vertexTexCoord1AttribLoc);
     glEnableVertexAttribArray(pObj->vertexTexCoord2AttribLoc);

     glEnableVertexAttribArray(pObj->blendAttribLoc);

    //Update the transformation
#if 0 //OLD METHOD
    pObj->mouse += pObj->delta;
    //if ((pObj->mouse > 0.55) || (pObj->mouse < 0.45)) { pObj->delta *= -1; };
    if ((pObj->mouse > 0.75) || (pObj->mouse < 0.25)) { pObj->delta *= -1; };
#ifdef ENABLE_BOWL_ROTATION    
    //get_matrix_output_mesh(3.5f, (pObj->mouse-0.5)*135, (pObj->mouse-0.5)*135, 1.0/RANGE_X_MESH, 1.0/RANGE_Y_MESH, 1.0/RANGE_Z_MESH, bowl_matrix);
    get_matrix_output_mesh(4.5f, 0.0, (pObj->mouse-0.5)*135, 1.0/RANGE_X_MESH, 1.0/RANGE_Y_MESH, 1.0/RANGE_Z_MESH, bowl_matrix);
    glUniformMatrix4fv(pObj->mvMatrixLocation, 1, GL_FALSE, bowl_matrix);
    GL_CHECK(glUniformMatrix4fv);          
#endif    
#else //NEW METHOD
 //static int stage = 1;

#if 0
	bowl_anglez += 1.0;
  if(stage == 1)
 {
	  bowl_scale += 0.007;
	  if(bowl_anglez > 90)
	  {
     bowl_anglez -= 0.5;
		  bowl_anglex += 0.5;
		  //bowl_angley += 0.5;
		  if(bowl_anglex > 70) stage = 2;
	  }
 }
 else
 { 
   if(bowl_anglez > 270){bowl_anglez = bowl_angley = bowl_anglex = 0; bowl_scale = 0; stage = 1;}
 }
#endif

#ifdef ENABLE_BOWL_ROTATION
	get_matrix_output_mesh(bowl_x, bowl_y, bowl_z, bowl_anglex, bowl_angley, bowl_anglez, (1.0+bowl_scale)/RANGE_X_MESH, (1.0+bowl_scale)/RANGE_Y_MESH, (1.0)/RANGE_Z_MESH, bowl_matrix);
    glUniformMatrix4fv(pObj->mvMatrixLocation, 1, GL_FALSE, bowl_matrix);
    GL_CHECK(glUniformMatrix4fv);
#endif	

#endif
}

void onscreen_mesh_state_restore_vbo(SgxRender3DSRV_Obj *pObj,
                                        GLuint vertexId, GLuint blendId, GLuint indexId)
{
           
     //restore the vertices and indices
     glBindBuffer(GL_ARRAY_BUFFER, vertexId);
     glVertexAttribPointer(pObj->vertexPositionAttribLoc, 3, GL_LUT_DATATYPE, GL_FALSE, stride*sizeof(LUT_DATATYPE), 0);
     glVertexAttribPointer(pObj->vertexTexCoord1AttribLoc, 2, GL_LUT_DATATYPE, GL_FALSE, (stride)*sizeof(LUT_DATATYPE), (GLvoid*)(3*sizeof(LUT_DATATYPE)));
     glVertexAttribPointer(pObj->vertexTexCoord2AttribLoc, 2, GL_LUT_DATATYPE, GL_FALSE, (stride)*sizeof(LUT_DATATYPE), (GLvoid*)(5*sizeof(LUT_DATATYPE)));

     glBindBuffer(GL_ARRAY_BUFFER, blendId);
     glVertexAttribPointer(pObj->blendAttribLoc, 2, GL_BLENDLUT_DATATYPE, GL_TRUE, 2*sizeof(char), 0);

     //Index buffer
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
}


int SgxRender3DSRV_setup(SgxRender3DSRV_Obj *pObj)
{
    //STEP1 - shader setup
    pObj->uiProgramObject = SgxRender3DSRV_createProgram(
                        gSgxRender3DSRV_vertexShader_1VBO,
                        gSgxRender3DSRV_fragmentShader_1VBO
                     );
    if (pObj->uiProgramObject==0)
    {
       return -1;
    }

    glUseProgram(pObj->uiProgramObject);
    System_eglCheckGlError("glUseProgram");

    //locate sampler uniforms
    pObj->samplerLocation0 = glGetUniformLocation(pObj->uiProgramObject, "uSampler[0]");
    glUniform1i(pObj->samplerLocation0, 0);
    GL_CHECK(glUniform1i);
    pObj->samplerLocation1 = glGetUniformLocation(pObj->uiProgramObject, "uSampler[1]");
    glUniform1i(pObj->samplerLocation1, 1);
    GL_CHECK(glUniform1i);
    pObj->mvMatrixLocation = glGetUniformLocation(pObj->uiProgramObject, "uMVMatrix");
    GL_CHECK(glGetAttribLocation);
    pObj->vertexPositionAttribLoc = glGetAttribLocation(pObj->uiProgramObject, "aVertexPosition");
    GL_CHECK(glGetAttribLocation);
    pObj->blendAttribLoc = glGetAttribLocation(pObj->uiProgramObject, "blendVals");
    GL_CHECK(glGetAttribLocation);
    pObj->vertexTexCoord1AttribLoc = glGetAttribLocation(pObj->uiProgramObject, "aTextureCoord1");
    GL_CHECK(glGetAttribLocation);
    pObj->vertexTexCoord2AttribLoc = glGetAttribLocation(pObj->uiProgramObject, "aTextureCoord2");
    GL_CHECK(glGetAttribLocation);
#if 0
    pObj->vertexIndexImage1AttribLoc = glGetAttribLocation(pObj->uiProgramObject, "dspImageIndex1");
    GL_CHECK(glGetAttribLocation);
    pObj->vertexIndexImage2AttribLoc = glGetAttribLocation(pObj->uiProgramObject, "dspImageIndex2");
    GL_CHECK(glGetAttribLocation);
#endif

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    System_eglCheckGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    System_eglCheckGlError("glClear");

    glDisable(GL_DEPTH_TEST);
#ifdef ENABLE_GLOBAL_BLENDING    
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif    

    pObj->mouse = 0.5; 
    pObj->delta = 0.0025; //0.00125;

    //STEP2 - initialise the vertices
    car_init_vertices_vbo(&pObj->car_gl1);
    GL_CHECK(car_init_vertices_vbo);
    
    //STEP3 - initialise the individual views
    screen1_init_vbo();
    GL_CHECK(screen1_init_vbo);
 
    //cull
    //glEnable(GL_CULL_FACE);
 
    return 0;
}

void SgxRender3DSRV_drawWrapper(SgxRender3DSRV_Obj *pObj, GLuint *texYuv)
{
	int i;
	if(prevLUT != pObj->LUT3D || prevblendLUT != pObj->blendLUT3D) 
	{
		prevLUT = pObj->LUT3D;
		prevblendLUT = pObj->blendLUT3D;
		surroundview_init_vertices_vbo_wrap(pObj);
	}

 //First setup the program once
 glUseProgram(pObj->uiProgramObject);
    //then change the meshes and draw
    for(i = 0;i < QUADRANTS;i ++)
    {
		onscreen_mesh_state_restore_program_textures_attribs(
				pObj, texYuv, (0+i)%4, (3+i)%4);
        onscreen_mesh_state_restore_vbo(
            pObj, vboId[i*3], vboId[i*3+1], vboId[i*3+2]);
        GL_CHECK(onscreen_mesh_state_restore_vbo);
		glDrawElements(render_mode, index_buf_length, GL_UNSIGNED_SHORT,  0);
        GL_CHECK(glDrawElements);
    }
}

void SgxRender3DSRV_renderFrame3DSRV(SgxRender3DSRV_Obj *pObj, System_EglObj *pEglObj, GLuint *texYuv)
{
    glClear(GL_COLOR_BUFFER_BIT);

     {
        glViewport(520, 0, 880, 1080);
        SgxRender3DSRV_drawWrapper(pObj, texYuv);
        car_draw_vbo(&pObj->car_gl1, 4);
     }

#if ENABLE_SGX_RENDERED_PREVIEW
     // Draw the other panes
     {
        glViewport(0, 1080-(200+440*1),520,440);
        screen1_draw_vbo(texYuv[0]);
#ifdef ENABLE_CAR_SIDE_PANES
        car_draw_vbo(&pObj->car_gl1, 0);
#endif
     }
     {
        glViewport(0, 1080-(200+440*2),520,440);
        screen1_draw_vbo(texYuv[1]);
#ifdef ENABLE_CAR_SIDE_PANES
        car_draw_vbo(&pObj->car_gl1, 1);
#endif
     }
     {
        glViewport(520+880, 1080-(200+440*1),520,440);
        screen1_draw_vbo(texYuv[2]);
#ifdef ENABLE_CAR_SIDE_PANES
        car_draw_vbo(&pObj->car_gl1, 2);
#endif
     }
     {
        glViewport(520+880, 1080-(200+440*2),520,440);
        screen1_draw_vbo(texYuv[3]);
#ifdef ENABLE_CAR_SIDE_PANES
        car_draw_vbo(&pObj->car_gl1, 3);
#endif
     }
#endif
}

void SgxRender3DSRV_renderFrame(SgxRender3DSRV_Obj *pObj, System_EglObj *pEglObj, GLuint *texYuv)
{
    SgxRender3DSRV_renderFrame3DSRV(pObj, pEglObj, texYuv);
}


void render3dFrame(SgxRender3DSRV_Obj *pObj, System_EglObj *pEglObj, GLuint *texYuv)
{
    glClear(GL_COLOR_BUFFER_BIT);

     {
        glViewport(0, 0, 880, 1080);
        SgxRender3DSRV_drawWrapper(pObj, texYuv);
        car_draw_vbo(&pObj->car_gl1, 4);
    eglSwapBuffers(pEglObj->display, pEglObj->surface);
     }
}


