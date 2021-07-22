
#include <stdio.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "shaderprog.h"


GLFWwindow *window;


mat4 V,P;


static void get_point_arcball(double,double,vec3);
void rotate_callback(GLFWwindow*,double,double);
void action_callback(GLFWwindow*,int,int,int);


enum { NONE,ROTATE };
int cube_action = NONE;
int new_cube_action = 1;

float last_xpos;
float last_ypos;
float start_xpos;
float start_ypos;


GLuint cubeVAO;
GLuint cubeVBO;
GLuint cubeShader;
GLuint cubeNumVertices;
mat4 cubeM;

GLuint axesVAO;
GLuint axesVBO;
GLuint axesShader;
GLuint axesNumVertices;
mat4 axesM;


int context_init();
int variable_load();
void gl_settings();
void printGLErrors();
void variable_free();




int main()
{
	if (!context_init())
		return -1;
	if (!variable_load())
		return -1;
	gl_settings();


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glfwSetMouseButtonCallback(window,action_callback);
		switch (cube_action)
		{
			case ROTATE:
					glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
					glfwSetCursorPosCallback(window,rotate_callback);
					break;
			default:
					glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
					glfwSetCursorPosCallback(window,NULL);
					new_cube_action = 1;
		}
		

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		/* cube */
		{
			glUseProgram(cubeShader);
			glUniformMatrix4fv(glGetUniformLocation(cubeShader,"M"),1,GL_FALSE,(GLfloat*)cubeM);
			glUniformMatrix4fv(glGetUniformLocation(cubeShader,"V"),1,GL_FALSE,(GLfloat*)V);
			glUniformMatrix4fv(glGetUniformLocation(cubeShader,"P"),1,GL_FALSE,(GLfloat*)P);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES,0,cubeNumVertices);
		}
		/* axes */
		{
			glUseProgram(axesShader);
			glUniformMatrix4fv(glGetUniformLocation(axesShader,"M"),1,GL_FALSE,(GLfloat*)cubeM);
			glUniformMatrix4fv(glGetUniformLocation(axesShader,"V"),1,GL_FALSE,(GLfloat*)V);
			glUniformMatrix4fv(glGetUniformLocation(axesShader,"P"),1,GL_FALSE,(GLfloat*)P);
			glBindVertexArray(axesVAO);
			glDrawArrays(GL_LINES,0,axesNumVertices);
		}


		glfwSwapBuffers(window);
		printGLErrors();
	}


	variable_free();
	glfwTerminate();
	return 0;
}


void action_callback(GLFWwindow *window,int button,int action,int mods)
{
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		cube_action = ROTATE;
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
		cube_action = NONE;
}


static void get_point_arcball(double xpos,double ypos,vec3 p)
{
	const float radius = 300.f;


	p[0] = (xpos - 400.f) / radius;
	p[1] = -(ypos - 300.f) / radius;
	p[2] = 0.f;

	if (p[0] > 1.f)
		p[0] = 1.f;
	if (p[0] < -1.f)
		p[0] = -1.f;
	if (p[1] > 1.f)
		p[1] = 1.f;
	if (p[1] < -1.f)
		p[1] = -1.f;

	float r = p[0]*p[0] + p[1]*p[1];
	if (r > 1.f)
		glm_vec3_norm(p);
	else
		p[2] = sqrt(1.f - r);
}

void rotate_callback(GLFWwindow *window,double xpos,double ypos)
{
	static float yaw = -90.f;
	static float pitch = 0.f;

	if (new_cube_action)
	{
		start_xpos = xpos;
		start_ypos = ypos;
		new_cube_action = 0;
	}


	yaw += (xpos - start_xpos) * 0.045f;
	pitch += (start_ypos - ypos) * 0.03f;

	vec4 yaw_quat = { 0.f };
	yaw_quat[1] = sin(glm_rad(yaw) / 2.f) * 1.f;
	yaw_quat[3] = cos(glm_rad(yaw) / 2.f);
	glm_quat_normalize(yaw_quat);

	vec4 pitch_quat = { 0.f };
	pitch_quat[0] = sin(glm_rad(pitch) / 2.f) * 1.f;
	pitch_quat[3] = cos(glm_rad(pitch) / 2.f);
	glm_quat_normalize(pitch_quat);


	mat4 lookat; glm_lookat((vec3){0.f,-5.f,-5.f},(vec3){0.f,0.f,0.f},GLM_YUP,lookat);

	glm_mat4_identity(V);
	glm_quat_rotate(V,pitch_quat,V);
	glm_mat4_mul(lookat,V,V);
	glm_quat_rotate(V,yaw_quat,V);

}
void old_rotate_callback(GLFWwindow *window,double xpos,double ypos)
{

	if (new_cube_action)
	{
		start_xpos = xpos;
		start_ypos = ypos;
		new_cube_action = 0;
	}

	
	vec3 ptA,ptB;
	get_point_arcball(start_xpos,start_ypos,ptA);
	get_point_arcball(xpos,ypos,ptB);

	vec3 ptA_cross_ptB;
	glm_vec3_crossn(ptA,ptB,ptA_cross_ptB);
	float ptA_angle_ptB = acos(fmin(1.f,glm_vec3_dot(ptA,ptB)));


	vec3 axis;
	mat3 invRotV;
	glm_mat4_pick3(V,invRotV);
	glm_mat3_inv(invRotV,invRotV);
	glm_mat3_mulv(invRotV,ptA_cross_ptB,axis);
	glm_vec3_norm(axis);

	vec4 rot_quat;
	rot_quat[0] = sin(ptA_angle_ptB / 2.f) * axis[0];
	rot_quat[1] = sin(ptA_angle_ptB / 2.f) * axis[1];
	rot_quat[2] = sin(ptA_angle_ptB / 2.f) * axis[2];
	rot_quat[3] = cos(ptA_angle_ptB / 2.f);
	glm_quat_normalize(rot_quat);
	glm_quat_slerp(GLM_QUAT_IDENTITY,rot_quat,0.1f,rot_quat);


	glm_quat_rotate(V,rot_quat,V);
}


int context_init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	if ((window = glfwCreateWindow(800,600,"gl",NULL,NULL)) == NULL)
		return 0;
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return 0;
	return 1;
}

void printGLErrors()
{
	int i;
	while ((i = glGetError()) != GL_NO_ERROR)
		fprintf(stderr,"GL: %#08x\n",i);
}

void gl_settings()
{
	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.22f,0.22f,0.22f,1.f);
	glViewport(0,0,800,600);
	printGLErrors();
}

void variable_free()
{
	/* shaders */
	{
		glDeleteProgram(cubeShader);
		glDeleteProgram(axesShader);
	}

	/* vertices */
	{
		glDeleteBuffers(1,&cubeVBO);
		glDeleteVertexArrays(1,&cubeVAO);
		glDeleteBuffers(1,&axesVBO);
		glDeleteVertexArrays(1,&axesVAO);
	}
}

int variable_load()
{
	/* shaders */
	{
		cubeShader = shaderprog_load("asset/shd/cube.vert",NULL,"asset/shd/cube.frag");
		if (!cubeShader)
			return 0;

		axesShader = shaderprog_load("asset/shd/axes.vert",NULL,"asset/shd/axes.frag");
		if (!axesShader)
			return 0;
	}


	/* vertices */
	{
		/* WOUND WRONG ORDER */
		float cubeVertices[] = {
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
		};
		cubeNumVertices = 36;
		glGenVertexArrays(1,&cubeVAO);
		glBindVertexArray(cubeVAO);
		glGenBuffers(1,&cubeVBO);
		glBindBuffer(GL_ARRAY_BUFFER,cubeVBO);
		glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
		glEnableVertexAttribArray(0);


		GLfloat axesVertices[] = {
			1.f,0.f,0.f,1.f,0.f,0.f,
			-1.f,0.f,0.f,0.2f,0.f,0.f,
			0.f,1.f,0.f,0.f,1.f,0.f,
			0.f,-1.f,0.f,0.f,0.2f,0.f,
			0.f,0.f,1.f,0.f,0.f,1.f,
			0.f,0.f,-1.f,0.f,0.f,0.2f
		};
		axesNumVertices = 6;
		glGenVertexArrays(1,&axesVAO);
		glBindVertexArray(axesVAO);
		glGenBuffers(1,&axesVBO);
		glBindBuffer(GL_ARRAY_BUFFER,axesVBO);
		glBufferData(GL_ARRAY_BUFFER,sizeof(axesVertices),axesVertices,GL_STATIC_DRAW);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
		glEnableVertexAttribArray(1);

	}


	/* matrices */
	{
		glm_mat4_identity(cubeM);

		glm_mat4_identity(V);
		glm_lookat((vec3){0.f,-5.f,-5.f},(vec3){0.f,0.f,0.f},GLM_YUP,V);

		glm_mat4_identity(P);
		glm_perspective(glm_rad(45.f),800.f/600.f,0.1f,1000.f,P);
	}


	printGLErrors();
	return 1;
}
