/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef STANDALONE
#include <linux/src/system/system_drm_egl.h>
#endif
#include "sgxRender3DSRV.h"
#include "jeep_object.txt"

extern float car_x;
extern float car_y;
extern float car_z;
extern float car_anglex;
extern float car_angley;
extern float car_anglez;
extern float car_scale;



int load_texture_from_raw_file(GLuint tex, int width, int height, int textureType, char* filename);

static const char* vshader = "\
    precision mediump float; \
    attribute vec4 aVertexPosition;\
    attribute vec2 aTextureCoord; \
    varying vec2 vTextureCoord; \
    uniform mat4 uMVMatrix;\
    void main(void) {\
        gl_Position = uMVMatrix*aVertexPosition;\
        vTextureCoord = aTextureCoord; \
    }";

static const char* fshader = "\
    precision mediump float; \
    varying vec2 vTextureCoord; \
    uniform sampler2D uSampler0; \
    vec4 tempcolor;\
    void main(void) {\
        tempcolor = texture2D(uSampler0, vTextureCoord);\
        gl_FragColor = tempcolor.bgra;\
    }";

static GLfloat mvMatrixCar[16] = {
    0.107114546,-0.318550438,-0.0354970358,-0.0353553370,
    -0.258597404, -0.131947935, -0.0147033501, -0.0146446554, 0.0, -0.214229092,
    0.139137357, 0.138581932, 0.000000000, 0.000000000, 1.80761504, 2.00000000
}; 

int car_shader_init(gl_state *car_gl)
{
    /* Create program and link */
    GLuint uiFragShader, uiVertShader;		// Used to hold the fragment and vertex shader handles

    // Create the fragment shader object
    uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load the source code into it
    glShaderSource(uiFragShader, 1, (const char**)&fshader, NULL);
    // Compile the source code
    glCompileShader(uiFragShader);

    // Check if compilation succeeded
    GLint bShaderCompiled;
    glGetShaderiv(uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);

    if (!bShaderCompiled)
    {
        D_PRINTF("Error in frag shader!\n");
    }
    // Loads the vertex shader in the same way
    uiVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(uiVertShader, 1, (const char**)&vshader, NULL);

    glCompileShader(uiVertShader);
    glGetShaderiv(uiVertShader, GL_COMPILE_STATUS, &bShaderCompiled);

    if (!bShaderCompiled)
    {
        D_PRINTF("Error: compiling vert shader\n");
    }
    // Create the shader program
    car_gl->program = glCreateProgram();

    // Attach the fragment and vertex shaders to it
    glAttachShader(car_gl->program, uiFragShader);
    glAttachShader(car_gl->program, uiVertShader);

    // Link the program
    glLinkProgram(car_gl->program);

    // Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(car_gl->program, GL_LINK_STATUS, &bLinked);

    //set the program
    glUseProgram(car_gl->program);

    if (!bLinked)
    {
        D_PRINTF("Error: linking prog\n");
    }


    //locate sampler uniforms
    car_gl->mvMatrixOffsetLoc = glGetUniformLocation(car_gl->program, "uMVMatrix");
    GL_CHECK(glGetUniformLocation);

    car_gl->samplerLoc = glGetUniformLocation(car_gl->program, "uSampler0");
    GL_CHECK(glGetUniformLocation);
    glUniform1i(car_gl->samplerLoc, 0);

    //locate attributes
    car_gl->attribIndices[0] = glGetAttribLocation(car_gl->program, "aVertexPosition");
    GL_CHECK(glGetAttribLocation);
    car_gl->attribIndices[1] = glGetAttribLocation(car_gl->program, "aTextureCoord");
    GL_CHECK(glGetAttribLocation);

    return 0;
}

#ifndef ENABLE_VBO_DRAW //NONVBO drawing for car
int car_init_vertices(gl_state *car_gl)
{
    //Shader
    car_shader_init(car_gl);

    //upload the vertex and texture and image index interleaved array
    glVertexAttribPointer(car_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), car_grid);
    GL_CHECK(glVertexAttribPointer);
    glVertexAttribPointer(car_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), car_texture);
    GL_CHECK(glVertexAttribPointer);

    glEnableVertexAttribArray(car_gl->attribIndices[0]);
    GL_CHECK(glEnableVertexAttribArray);
    glEnableVertexAttribArray(car_gl->attribIndices[1]);
    GL_CHECK(glEnableVertexAttribArray);

    //CAR texture
    //unsigned int carTextureRGBA;
    //carTextureRGBA = 0xFF0000FF;
    glGenTextures(1, &car_gl->textureID[0]);
    GL_CHECK(glGenTextures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, car_gl->textureID[0]);
    GL_CHECK(glBindTexture);
    load_texture_from_raw_file( car_gl->textureID[0], 512, 512, GL_RGB,  "./jeep_outside.bmp");
    GL_CHECK(load_texture_from_raw_file);

    return 0;
}


void onscreen_car_state_restore(gl_state *car_gl)
{
    //set the program we need
    glUseProgram(car_gl->program);
    GL_CHECK(glUseProgram);
    //font texture
    glUniform1i(car_gl->samplerLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, car_gl->textureID[0]);
    GL_CHECK(glBindTexture);

    //restore the vertices
    //upload the vertex and texture and image index interleaved array

    glVertexAttribPointer(car_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), car_grid);
    GL_CHECK(glVertexAttribPointer);
    glVertexAttribPointer(car_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), car_texture);
    GL_CHECK(glVertexAttribPointer);

    glEnableVertexAttribArray(car_gl->attribIndices[0]);
    GL_CHECK(glEnableVertexAttribArray);
    glEnableVertexAttribArray(car_gl->attribIndices[1]);
    GL_CHECK(glEnableVertexAttribArray);

#ifdef ENABLE_CPP
    car_gl->carMouse += 0.005;
    if (car_gl->carMouse > 0.7)
        car_gl->carMouse = 0.2;
    //get_matrix_output_car(2.f, (car_gl->carMouse-0.5)*135, (car_gl->carMouse-0.5)*135, 0.15f, 0.15f, 0.15f, mvMatrixCar);
	get_matrix_output_car(2.f, 0.0, (car_gl->carMouse-0.5)*135, 0.15f, 0.15f, 0.15f, mvMatrixCar);
    glUniformMatrix4fv(car_gl->mvMatrixOffsetLoc, 1, GL_FALSE, mvMatrixCar);
    GL_CHECK(glUniformMatrix4fv);
#else
    glUniformMatrix4fv(car_gl->mvMatrixOffsetLoc, 1, GL_FALSE, mvMatrixCar);
    GL_CHECK(glUniformMatrix4fv);
#endif
}

void car_draw(gl_state *car_gl )
{
    int numIndices = sizeof(car_indexgrid)/sizeof(short);

    onscreen_car_state_restore(car_gl);

    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT,  car_indexgrid);
    GL_CHECK(glDrawElements);
    glDisable(GL_DEPTH_TEST);
}

#else //ENABLE_VBO_DRAW

int car_init_vertices_vbo(gl_state *car_gl)
{
    //Shader
    car_shader_init(car_gl);

    //upload the vertex and texture and image index interleaved array
    glGenBuffers(1, &car_gl->vboID[0]);
    glBindBuffer(GL_ARRAY_BUFFER, car_gl->vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(car_grid), car_grid, GL_STATIC_DRAW);
    glVertexAttribPointer(car_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);

    glGenBuffers(1, &car_gl->vboID[1]);
    glBindBuffer(GL_ARRAY_BUFFER, car_gl->vboID[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(car_texture), car_texture, GL_STATIC_DRAW);
    glVertexAttribPointer(car_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

    glGenBuffers(1, &car_gl->vboID[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_gl->vboID[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(car_indexgrid), car_indexgrid, GL_STATIC_DRAW);

    glEnableVertexAttribArray(car_gl->attribIndices[0]);
    glEnableVertexAttribArray(car_gl->attribIndices[1]);

    //CAR texture
    //unsigned int carTextureRGBA;
    //carTextureRGBA = 0xFF0000FF;
    glGenTextures(1, &car_gl->textureID[0]);
    GL_CHECK(glGenTextures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, car_gl->textureID[0]);
    GL_CHECK(glBindTexture);
    load_texture_from_raw_file( car_gl->textureID[0], 512, 512, GL_RGB,  "./jeep_outside2_raw.bmp");
    GL_CHECK(load_texture_from_raw_file);

    car_gl->carMouse[0] = 0.5;
    car_gl->carMouse[1] = 0.5;
    car_gl->carMouse[2] = 0.5;
    car_gl->carMouse[3] = 0.5;
    car_gl->carMouse[4] = 0.5;
        
    car_gl->delta[0] = 0.00125;
    car_gl->delta[1] = 0.00125;
    car_gl->delta[2] = 0.00125;
    car_gl->delta[3] = 0.00125;
    car_gl->delta[4] = 0.0025; //0.00125;

    car_gl->carMouseMax[0] = 0.55;
    car_gl->carMouseMax[1] = 0.55;
    car_gl->carMouseMax[2] = 0.55;
    car_gl->carMouseMax[3] = 0.55;
    car_gl->carMouseMax[4] = 0.75; //0.55;

    car_gl->carMouseMin[0] = 0.45;
    car_gl->carMouseMin[1] = 0.45;
    car_gl->carMouseMin[2] = 0.45;
    car_gl->carMouseMin[3] = 0.45;
    car_gl->carMouseMin[4] = 0.25; //0.45;
    

    return 0;
}


void onscreen_car_state_restore_vbo(gl_state *car_gl, int viewid)
{
    //set the program we need
    glUseProgram(car_gl->program);
    GL_CHECK(glUseProgram);
    //font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, car_gl->textureID[0]);
    GL_CHECK(glBindTexture);

    //restore the vertices
    //upload the vertex and texture and image index interleaved array
    glBindBuffer(GL_ARRAY_BUFFER, car_gl->vboID[0]);
    glVertexAttribPointer(car_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, car_gl->vboID[1]);
    glVertexAttribPointer(car_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_gl->vboID[2]);

    glEnableVertexAttribArray(car_gl->attribIndices[0]);
    glEnableVertexAttribArray(car_gl->attribIndices[1]);
    GL_CHECK(glEnableVertexAttribArray);

    car_gl->carMouse[viewid] += car_gl->delta[viewid];
    if ((car_gl->carMouse[viewid] > car_gl->carMouseMax[viewid]) || (car_gl->carMouse[viewid] < car_gl->carMouseMin[viewid])) 
    { 
     car_gl->delta[viewid] *= -1; 
    }

#if 0
    get_matrix_output_car(2.f, (car_gl->carMouse[viewid]-0.5)*135, (car_gl->carMouse[viewid]-0.5)*135, 0.15f, 0.15f, 0.1f, mvMatrixCar, viewid);
    glUniformMatrix4fv(car_gl->mvMatrixOffsetLoc, 1, GL_FALSE, mvMatrixCar);
#else
// static int stage = 1;
 
#if 0
 car_anglez += 1.0;
 if(stage == 1)
 {
	  car_scale += 0.0005;
	  if(car_anglez > 90)
	  {
     car_anglez -= 0.5;
		  car_anglex += 0.5;
		  //car_angley += 0.5;
		  if(car_anglex > 70) stage = 2;
	  }
 }
 else
 { 
   if(car_anglez > 270){car_anglez = car_angley = car_anglex = 0; car_scale = 0; stage = 1;}
 }
#endif
	  get_matrix_output_car(car_x, car_y, car_z, -car_anglex, car_angley, car_anglez, car_scale*0.5+0.15f, 0.15f*0.6, (car_scale+0.1f)*0.8, mvMatrixCar, viewid);
   glUniformMatrix4fv(car_gl->mvMatrixOffsetLoc, 1, GL_FALSE, mvMatrixCar);  
#endif    
    GL_CHECK(glUniformMatrix4fv);
}

void car_draw_vbo(gl_state *car_gl, int viewid )
{
    int numIndices = sizeof(car_indexgrid)/sizeof(short);

    onscreen_car_state_restore_vbo(car_gl, viewid);

    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT,  0);
    GL_CHECK(glDrawElements);
    
}

#endif //ENABLE_VBO_DRAW

