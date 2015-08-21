/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4, glm::ivec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

void apply_math_car(glm::vec3 Translate, glm::vec2 const & Rotate, glm::vec3 inScale, float* out)
{
   glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
   glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
   glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
   glm::mat4 View = glm::rotate(ViewRotateX, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
   View = glm::rotate(View, Rotate.x*0+180, glm::vec3(0.0f, 1.0f, 0.0f));
   View = glm::rotate(View, Rotate.x*0-90, glm::vec3(1.0f, 0.0f, 0.0f));
   glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
   glm::mat4 MVP = Projection * View * Model;
   memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 
//rear view
void apply_math_car_rear(glm::vec3 Translate, glm::vec2 const & Rotate, glm::vec3 inScale, float* out)
{
	glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::mat4 View = glm::rotate(ViewRotateX, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
	View = glm::rotate(View,  Rotate.x*0+180, glm::vec3(0.0f, 1.0f, 0.0f));
	View = glm::rotate(View,  Rotate.x*0-45, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
	glm::mat4 MVP = Projection * View * Model;
	memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 

//left side view
void apply_math_car_leftside(glm::vec3 Translate, glm::vec2 const & Rotate, glm::vec3 inScale, float* out)
{
	glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::mat4 View = glm::rotate(ViewRotateX, Rotate.x+20, glm::vec3(0.0f, 1.0f, 0.0f));
	View = glm::rotate(View,  Rotate.x*0+90, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
	glm::mat4 MVP = Projection * View * Model;
	memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 
//right side view
void apply_math_car_rightside(glm::vec3 Translate, glm::vec2 const & Rotate, glm::vec3 inScale, float* out)
{
	glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::mat4 View = glm::rotate(ViewRotateX, Rotate.x+20, glm::vec3(0.0f, 1.0f, 0.0f));
	View = glm::rotate(View,  Rotate.x*0-90, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
	glm::mat4 MVP = Projection * View * Model;
	memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 
//front view
void apply_math_car_front(glm::vec3 Translate, glm::vec2 const & Rotate, glm::vec3 inScale, float* out)
{
	glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::mat4 View = glm::rotate(ViewRotateX, Rotate.x+20, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
	glm::mat4 MVP = Projection * View * Model;
	memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 
//top view
void apply_math_car_top(glm::vec3 Translate, glm::vec3 const & Rotate, glm::vec3 inScale, float* out)
{
	glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 ViewRotateY = glm::rotate(ViewRotateX, Rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 View = glm::rotate(ViewRotateY, Rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));
	View = glm::rotate(View,  Rotate.x*0+180, glm::vec3(0.0f, 1.0f, 0.0f));
	View = glm::rotate(View,  Rotate.x*0-90, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
	glm::mat4 MVP = Projection * View * Model;
	memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 


static void apply_math_mesh(glm::vec3 Translate, glm::vec3 const & Rotate, glm::vec3 inScale, float* out)
{
   glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
   glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
   glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
   glm::mat4 View = glm::rotate(ViewRotateX, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
   //View = glm::rotate(View, Rotate.x*0+180, glm::vec3(0.0f, 0.0f, 1.0f));
   glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
   glm::mat4 MVP = Projection * View * Model;
   memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 

static void apply_math_mesh1(glm::vec3 Translate, glm::vec3 const & Rotate, glm::vec3 inScale, float* out)
{
	glm::mat4 Projection = glm::perspective(30.0f, 4.0f/3.0f,  0.1f, 50.f);  //glm::mat4(1.0f); //
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), Translate);
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Rotate.x, glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::mat4 ViewRotateY = glm::rotate(ViewRotateX, Rotate.y, glm::vec3(0.0f, 1.0f, 0.0f)); //rotate about y
	glm::mat4 View = glm::rotate(ViewRotateY, Rotate.z, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate about z
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), inScale);
	glm::mat4 MVP = Projection * View * Model;
    memcpy(out, glm::value_ptr(MVP), 4*4*sizeof(float));
} 


extern "C" void get_matrix_output_car(float x, float y, float z, float angleX, float angleY, float angleZ, float scaleX, float scaleY, float scaleZ, float* matrix, int id)
{
	if(id == 0)
		apply_math_car_rear(glm::vec3(x, y, z), glm::vec2(angleX, angleY), glm::vec3(scaleX, scaleY, scaleZ), matrix);
	else 	if(id == 1)
		apply_math_car_leftside(glm::vec3(x, y, z), glm::vec2(angleX, angleY), glm::vec3(scaleX, scaleY, scaleZ), matrix);
	else 	if(id == 2)
		apply_math_car_front(glm::vec3(x, y, z), glm::vec2(angleX, angleY), glm::vec3(scaleX, scaleY, scaleZ), matrix);
	else 	if(id == 3)
		apply_math_car_rightside(glm::vec3(x, y, z), glm::vec2(angleX, angleY), glm::vec3(scaleX, scaleY, scaleZ), matrix);
	else if(id == 4) //surroundview so position top
		apply_math_car_top(glm::vec3(x, y, z), glm::vec3(angleX, angleY, angleZ), glm::vec3(scaleX, scaleY, scaleZ), matrix);
	else
	{
		//printf("FATAL ERROR: get_matrix_output\n");
		//while(1);
	}
}


extern "C" void get_matrix_output_mesh(float x, float y, float z, float angleX, float angleY, float angleZ, float scaleX, float scaleY, float scaleZ, float* matrix)
{
   apply_math_mesh1(glm::vec3(x, y, z), glm::vec3(angleX, angleY, angleZ), glm::vec3(scaleX, scaleY, scaleZ), matrix);
}