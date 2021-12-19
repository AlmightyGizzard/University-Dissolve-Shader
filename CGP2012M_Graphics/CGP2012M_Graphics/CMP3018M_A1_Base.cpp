//#include <cstdio>
//#include <cstdlib>
#include <iostream>
#include <vector>

#include "GLerror.h"
#include "SDL_Start.h"
#include "Camera_LOGL.h"
#include "Model.h"
#include "ModelLoaderClass.h"
#include "SkyBox.h"
#ifndef TEXTURECLASS_H
#define TEXTURECLASS_H
#ifndef SHADERCLASS_H
#define SHADERCLASS_H

// // GLEW - OpenGL Extension Wrangler - http://glew.sourceforge.net/
// // NOTE: include before SDL.h
#ifndef GLEW_H
#define GLEW_H
//#include <GL/glew.h>
#include "windows.h"

// SDL - Simple DirectMedia Layer - https://www.libsdl.org/
#ifndef SDL_H
#define SDL_H
#include "SDL.h"
#include "SDL_image.h"

// // - OpenGL Mathematics - https://glm.g-truc.net/
#define GLM_FORCE_RADIANS // force glm to use radians
// // NOTE: must do before including GLM headers
// // NOTE: GLSL uses radians, so will do the same, for consistency
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//***************
//variables
SDL_Event event;
SDL_Window *win;
bool windowOpen = true;
bool isFullScreen = false;
//screen centre
float centreX = 0.0f;
float centreY = 0.0f;
//window aspect/width/height
int w;
int h;
float aspect;
int left;
int newwidth;
int newheight;

//transform matrices
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

glm::mat4 normalMatrix;

glm::mat4 translate;
glm::mat4 rotate;
glm::mat4 scale;
glm::mat4 modelRotate;
glm::mat4 modelScale;
glm::mat4 modelTranslate;
glm::mat4 planeTranslate;
glm::mat4 planeScale;
glm::vec2 screenSize;
float angle = 0;


//create camera
Camera_LOGL camera(glm::vec3(0.0f, 0.0f, 3.0f));
//load camera variables
glm::vec3 camPos;
glm::vec3 camTarget;
//mouse variables
int lastX;
int lastY;
int xpos;
int ypos;
bool firstMouse = true;
//time variables
GLuint currentTime = 0;
GLuint lastTime = 0;
GLuint elapsedTime = 0;

bool flag = true;

glm::vec3 lightCol;
glm::vec3 lightPos;
float ambientIntensity;

//**************
//function prototypes
void handleWindowInput();

int main(int argc, char *argv[]) {
	//start and initialise SDL
	SDL_Start sdl;
	SDL_GLContext context = sdl.Init();
	win = sdl.win;

	SDL_GetWindowSize(win, &w, &h);
	glViewport(0, 0, w, h);
	aspect = (float)w / (float)h;

	lastX = w/2.0f;
	lastY = h/2.0f;

	//error class
	GLerror glerr;
	int errorLabel;

	//GLEW initialise
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//register debug callback
	if (glDebugMessageCallback)
	{
		std::cout << "Registering OpenGL Debug callback function" << std::endl;
		glDebugMessageCallback(glerr.openglCallbackFunction, &errorLabel);
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			NULL,
			true);
	}

	//*****************************************************
	//OpenGL specific data

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	//objects
	//create SkyBox
	SkyBox sb;
	//create models
	//could make this better by specifying the texture in this model header
	Model models[4];
	//create model loader
	ModelImport modelLoader; 
	//load sphere
	modelLoader.LoadOBJ2("..//..//Assets//Models//blenderSphere.obj", models[0].vertices, models[0].texCoords, models[0].normals, models[0].indices);
	//load plane
	modelLoader.LoadOBJ2("..//..//Assets//Models//blenderPlane16x16.obj", models[1].vertices, models[1].texCoords, models[1].normals, models[1].indices);
	errorLabel = 0;

	//*********************
	//create texture collection
	//create textures - space for 4, but only using 2
	Texture texArray[4];
	//background texture
	texArray[0].load("..//..//Assets//Textures//terrain.png ");
	texArray[0].setBuffers();
	texArray[1].load("..//..//Assets//Textures//deathstar.png");
	texArray[1].setBuffers();
	texArray[2].load("..//..//Assets//Textures//earthmap1k.png");
	texArray[2].setBuffers();


	errorLabel = 2;

	//load skybox textures
	sb.loadSkyBox();

	//set shaders
	//1: plane, 0:sphere
	models[1].setShader("..//..//Assets//Shaders//shader_Projection_lighting_ADS_PLANE.vert", "..//..//Assets//Shaders//shader_Projection_lighting_ADS_PLANE.frag");
	models[0].setShader("..//..//Assets//Shaders//shader_Projection_lighting_ADS_SPHERE.vert", "..//..//Assets//Shaders//shader_Projection_lighting_ADS_SPHERE.frag");

	//OpenGL buffers
	sb.setBuffers();
	models[0].setBuffers();
	models[1].setBuffers();

	errorLabel = 3;
	//*****************************************
	//set uniform variables so that we can communicate with the shaders
	int transformLocation;
	int modelLocation;
	int viewLocation;
	int projectionLocation;
	int importModelLocation;
	int importViewLocation;
	int importProjectionLocation;
	int backgroundColourLocation;
	int ambientIntensityLocation;
	int modelColourLocation;
	int modelAmbientLocation;
	int modelLightPosLocation;
	int normalMatrixLocation;
	int viewPositionLocation;
	int timeLocation;
	int srLocation;
	int dsLocation;
	int eLocation;


	float screen[2] = { w, h }; 
	std::cout << w << " " << h << std::endl;

	//light colour initial setting
	lightCol = glm::vec3(1.0f, 0.95f, 0.95f);
	//light position
	lightPos = glm::vec3(2.0f, 0.0, 0.0f);
	//light distance setting
	ambientIntensity = 1.0f;


	//initialise transform matrices 
	//orthographic (2D) projection
	//projectionMatrix = glm::ortho(0.0f, 4.0f, 0.0f, 3.0f, -1.0f, 100.0f);
	//perspective (3D) projection
	projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);
	//initialise view matrix to '1'
	viewMatrix = camera.GetViewMatrix();
	modelScale = glm::mat4(1.0f);
	modelRotate = glm::mat4(1.0f);
	modelTranslate = glm::mat4(1.0f);
	planeTranslate = glm::mat4(1.0f);
	planeScale = glm::mat4(1.0f);
	

	//translate imported model away from the camera
	modelTranslate = glm::translate(modelTranslate, glm::vec3(0.0f, 0.0f, -1.0f));
	//translate imported plane down the Y axis
	planeTranslate = glm::translate(planeTranslate, glm::vec3(0.0f, -0.5f, 0.0f));

	//once only scale to model = make sphere smaller
	modelScale = glm::scale(modelScale, glm::vec3(0.3f, 0.3f, 0.3f));
	//planeScale to make plane larger
	planeScale = glm::scale(modelScale, glm::vec3(8.0f, 1.0f, 8.0f));
	errorLabel = 4;

	//*****************************
	//'game' loop
	while (windowOpen)
	{
		//*************************
		//****************************
		// OpenGL calls.

		glClearColor(1.0f, 1.0f, 1.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//process inputs
		handleWindowInput();

		//time
		currentTime = SDL_GetTicks();
		elapsedTime = currentTime - lastTime;
		lastTime = currentTime;

		//update camera matrix
		//camera only moves side to side, formards and backwards (no rotation)
		// set position, target, up direction
		//viewMatrix = glm::lookAt(glm::vec3(cam.camXPos, cam.camYPos, cam.camZPos), cam.cameraTarget, cam.cameraUp);
		projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);

		errorLabel = 5;
		
		//plane model
		//set .obj model
		glUseProgram(models[1].shaderProgram);
		//set sphere lighting
		modelColourLocation = glGetUniformLocation(models[1].shaderProgram, "uLightColour");
		glProgramUniform3fv(models[1].shaderProgram, modelColourLocation, 1, glm::value_ptr(lightCol));
		//light position
		modelLightPosLocation = glGetUniformLocation(models[1].shaderProgram, "uLightPosition");
		glProgramUniform3fv(models[1].shaderProgram, modelLightPosLocation, 1, glm::value_ptr(lightPos));
		//light distance
		modelAmbientLocation = glGetUniformLocation(models[1].shaderProgram, "uAmbientIntensity");
		glProgramUniform1f(models[1].shaderProgram, modelAmbientLocation, ambientIntensity);
		
		importModelLocation = glGetUniformLocation(models[1].shaderProgram, "uModel");
		glUniformMatrix4fv(importModelLocation, 1, GL_FALSE, glm::value_ptr(planeTranslate*planeScale));
		importViewLocation = glGetUniformLocation(models[1].shaderProgram, "uView");
		glUniformMatrix4fv(importViewLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		importProjectionLocation = glGetUniformLocation(models[1].shaderProgram, "uProjection");
		glUniformMatrix4fv(importProjectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		//time
		timeLocation = glGetUniformLocation(models[1].shaderProgram, "uTime");
		glProgramUniform1f(models[1].shaderProgram, timeLocation, currentTime);
		
		//set normal matrix for the plane
		normalMatrix = (glm::mat3)glm::transpose(glm::inverse(glm::mat4(1.0f)));
		//set the normalMatrix in the shaders
		normalMatrixLocation = glGetUniformLocation(models[1].shaderProgram, "uNormalMatrix");
		glUniformMatrix4fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		
		//set view position for specular component - use the camera position
		viewPositionLocation = glGetUniformLocation(models[1].shaderProgram, "uViewPosition");
		glProgramUniform3fv(models[1].shaderProgram, viewPositionLocation, 1, glm::value_ptr(camera.GetViewMatrix()));
		errorLabel = 6;
		//render
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texArray[0].texture);
		models[1].render();
		
		//skybox
		//no lighting on skybox, so we don't have to sort normal matrix etc
		viewMatrix = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glUseProgram(sb.sbShaderProgram);
		modelLocation = glGetUniformLocation(sb.sbShaderProgram, "uModel");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		viewLocation = glGetUniformLocation(sb.sbShaderProgram, "uView");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		projectionLocation = glGetUniformLocation(sb.sbShaderProgram, "uProjection");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		sb.renderSkyBox();

		//sphere model
		//set .obj model
		glUseProgram(models[0].shaderProgram);
		//set sphere lighting
		modelColourLocation = glGetUniformLocation(models[0].shaderProgram, "uLightColour");
		glProgramUniform3fv(models[0].shaderProgram, modelColourLocation, 1, glm::value_ptr(lightCol));
		//light position
		modelLightPosLocation = glGetUniformLocation(models[0].shaderProgram, "uLightPosition");
		glProgramUniform3fv(models[0].shaderProgram, modelLightPosLocation, 1, glm::value_ptr(lightPos));
		//light distance
		modelAmbientLocation = glGetUniformLocation(models[0].shaderProgram, "uAmbientIntensity");
		glProgramUniform1f(models[0].shaderProgram, modelAmbientLocation, ambientIntensity);
		//rotation
		modelRotate = glm::rotate(modelRotate, (float)elapsedTime / 6000, glm::vec3(0.0f, 1.0f, 0.0f));
		importModelLocation = glGetUniformLocation(models[0].shaderProgram, "uModel");
		glUniformMatrix4fv(importModelLocation, 1, GL_FALSE, glm::value_ptr(modelTranslate*modelRotate*modelScale));
		importViewLocation = glGetUniformLocation(models[0].shaderProgram, "uView");
		glUniformMatrix4fv(importViewLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		importProjectionLocation = glGetUniformLocation(models[0].shaderProgram, "uProjection");
		glUniformMatrix4fv(importProjectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		//time
		timeLocation = glGetUniformLocation(models[0].shaderProgram, "uTime");
		glProgramUniform1f(models[0].shaderProgram, timeLocation, currentTime);

		//set norml matrix for the sphere
		normalMatrix = (glm::mat3)glm::transpose(glm::inverse(modelTranslate * modelRotate * modelScale));
		//set the normalMatrix in the shaders
		normalMatrixLocation = glGetUniformLocation(models[0].shaderProgram, "uNormalMatrix");
		glUniformMatrix4fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		//set view position for specular component - use the camera position
		viewPositionLocation = glGetUniformLocation(models[0].shaderProgram, "uViewPosition");
		glProgramUniform3fv(models[0].shaderProgram, viewPositionLocation, 1, glm::value_ptr(camera.GetViewMatrix()));

		dsLocation = glGetUniformLocation(models[0].shaderProgram, "aTex");
		glUniform1i(dsLocation, 1);
		eLocation = glGetUniformLocation(models[0].shaderProgram, "aTex2");
		glUniform1i(eLocation, 2);

		//render
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texArray[1].texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texArray[2].texture);

		models[0].render();
		errorLabel = 7;

		

		//set to wireframe so we can see the circles
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		SDL_GL_SwapWindow(sdl.win);

	}//end loop

	//****************************
	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(context);

	SDL_Quit();
	return 0;
}

void handleWindowInput()
{
	//*****************************
		//SDL handled input
		//Any input to the program is done here

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			windowOpen = false;
		}
		if (event.type == SDL_WINDOWEVENT)
		{
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				std::cout << "Window resized w:" << w << " h:" << h << std::endl;
				SDL_GetWindowSize(win, &w, &h);
				newwidth = h * aspect;
				left = (w - newwidth) / 2;
				glViewport(left, 0, newwidth, h);
				break;
			default:
				break;
			}
		}
		if (event.type == SDL_MOUSEMOTION)
		{
			SDL_GetMouseState(&xpos, &ypos);

			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}
			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			lastX = xpos;
			lastY = ypos;

			std::cout << xpos << " " << ypos << std::endl;
			camera.ProcessMouseMovement(xoffset, yoffset);

		}
		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_w)
			{
				std::cout << "w" << std::endl;
				camera.ProcessKeyboard(FORWARD, elapsedTime);
			}
			if (event.key.keysym.sym == SDLK_s)
			{
				camera.ProcessKeyboard(BACKWARD, elapsedTime);
			}
			if (event.key.keysym.sym == SDLK_a)
			{
				camera.ProcessKeyboard(LEFT, elapsedTime);
			}
			if (event.key.keysym.sym == SDLK_d)
			{
				camera.ProcessKeyboard(RIGHT, elapsedTime);
			}
		}
		
		
	}
}
#endif
#endif
#endif
#endif