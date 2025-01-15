//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2042;		// 4096 for better shadow quality, but slower rendering
const unsigned int SHADOW_HEIGHT = 2042;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;



gps::Camera myCamera(
				glm::vec3(15.2f, 13.5f, 67.6f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.025f;

bool pressedKeys[1024];
GLfloat lightAngle;

gps::Model3D canon;
gps::Model3D scene;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;
float canonAngle = 0.0f;
float canonDirecion = 1.0f;

// point light
glm::vec3 pointLightPosition(17.27f, 1.94f, -7.0f); 
glm::vec3 pointLightColor(1.0f, 0.5f, 0.5f);    // Reddish light
float pointLightConstant = 1.0f;
float pointLightLinear = 0.22f;
float pointLightQuadratic = 0.20f;

int shotTime = 0;

// Automated tour waypoints
std::vector<glm::vec3> tourPositions = {
	glm::vec3(23.4f, 13.5f, 72.0f),
	glm::vec3(18.0f, 14.0f, 55.0f),
	glm::vec3(-2.4f, 14.0f, -24.8f),
	glm::vec3(0.0f, 1.5f, -6.7f),
	glm::vec3(-5.1f, 1.5f, -26.4f)
};

std::vector<glm::vec3> tourTargets = {
	glm::vec3(18.0f, 14.0f, 55.0f),
	glm::vec3(-2.4f, 14.0f, -24.8f),
	glm::vec3(0.0f, 1.5f, -6.7f),
	glm::vec3(-5.1f, 1.5f, -26.4f),
	glm::vec3(15.9f, 2.2f, -19.0f)
};

int currentSegment = 0;
float t = 0.0f; // Interpolation parameter
bool autoTour = false;

// Fog
float isFog = 0.0f;

// Skybox
gps::SkyBox mySkyBox;
gps::SkyBox mySkyBoxNight;
gps::Shader skyboxShader;
bool night = 0;


GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	glWindowWidth = width;
	glWindowHeight = height;

	glfwGetFramebufferSize(window, &retina_width, &retina_height);
	glViewport(0, 0, retina_width, retina_height);

	float aspectRatio = (float)retina_width / (float)retina_height;
	projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		night = !night;

	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		isFog = 1.0f - isFog;
		std::cout << "Fog state changed";
		myCustomShader.useShaderProgram();
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "isFog"), isFog);
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default rendering mode

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Point mode

	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		autoTour = !autoTour;
		if (autoTour) {
			currentSegment = 0;
			t = 0.0f;
			std::cout << "Starting Automated Tour...\n";
		}
		else {
			std::cout << "Stopping Automated Tour...\n";
		}
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
	myCamera.mouseCallback(window, xPos * 2.3, yPos * 2.3);
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	static float fov = 45.0f; // Default FOV

	// Adjust FOV based on scroll input
	fov -= static_cast<float>(yOffset);
	if (fov < 1.0f) fov = 1.0f;     // Clamp to avoid extreme zoom-in
	if (fov > 90.0f) fov = 90.0f;   // Clamp to avoid extreme zoom-out

	// Update projection matrix
	float aspectRatio = static_cast<float>(glWindowWidth) / static_cast<float>(glWindowHeight);
	projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);

	// Use your shader program and send the updated projection matrix to it
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void processMovement()
{
	// check other inputs
	if (pressedKeys[GLFW_KEY_X]) {
		shotTime = 600;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (autoTour) {
		if (currentSegment < tourPositions.size() - 1) {
			// Interpolate between current and next waypoint
			glm::vec3 startPos = tourPositions[currentSegment];
			glm::vec3 endPos = tourPositions[currentSegment + 1];
			glm::vec3 startTarget = tourTargets[currentSegment];
			glm::vec3 endTarget = tourTargets[currentSegment + 1];

			// Compute interpolated position and target
			glm::vec3 newPosition = glm::mix(startPos, endPos, t);
			glm::vec3 newTarget = glm::mix(startTarget, endTarget, t);

			myCamera.setPosition(newPosition);
			myCamera.setTarget(newTarget);

			// Increment t for interpolation speed
			t += 0.01f;
			if (t >= 1.0f) {
				t = 0.0f;
				currentSegment++;
			}
		}
		else {
			autoTour = false; // End tour after the last segment
			std::cout << "Automated Tour Completed." << std::endl;
		}
	}
	else {
		// 2. Only process WASD movement if NOT in autoTour
		if (pressedKeys[GLFW_KEY_W]) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_S]) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_A]) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_D]) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		}
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scrollCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);

	glPointSize(5.0f); // Adjust point size 
}

void initObjects() {
	canon.LoadModel("objects/canon/canon.obj");
	scene.LoadModel("objects/scene/scene.obj");
	ground.LoadModel("objects/ground/ground.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 25.0f, 60.0f );		
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set point light position, color and attenuation
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPosition"), 1, glm::value_ptr(pointLightPosition));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor"), 1, glm::value_ptr(pointLightColor));
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightConstant"), pointLightConstant);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLinear"), pointLightLinear);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightQuadratic"), pointLightQuadratic);

	//set the fog uniform
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "isFog"), isFog);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkybox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");

	mySkyBox.Load(faces);
}

void initSkyboxNight()
{
	std::vector<const GLchar*> faces;
	faces.push_back("nightsky/nightsky_rt.tga");
	faces.push_back("nightsky/nightsky_lf.tga");
	faces.push_back("nightsky/nightsky_up.tga");
	faces.push_back("nightsky/nightsky_dn.tga");
	faces.push_back("nightsky/nightsky_bk.tga");
	faces.push_back("nightsky/nightsky_ft.tga");

	mySkyBoxNight.Load(faces);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 160.0f;
	glm::mat4 lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();

	canonAngle += (0.2f * canonDirecion);
	if (abs(canonAngle) > 37) {
		canonDirecion *= -1;
	}
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -5.0f));
	model = glm::rotate(model, glm::radians(180.0f + canonAngle), glm::vec3(0.0f, 1.0f, 0.0f));	// 180 first to position it in the right direction
	model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.75f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	canon.Draw(shader);

	
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scene.Draw(shader);

}

void canonAnimation() {
	if (shotTime == 0) {
		pointLightLinear = 0.22f;
		pointLightQuadratic = 0.20f;
		return;
	}

	shotTime-=10;

	if (shotTime > 500) {
		pointLightLinear = 0.027f;
		pointLightQuadratic = 0.0028f;
	}
	else if (shotTime > 450) {
		pointLightLinear = 0.035f;
		pointLightQuadratic = 0.005f;
	}
	else if (shotTime > 400) {
		pointLightLinear = 0.045f;
		pointLightQuadratic = 0.0075f;
	}
	else if (shotTime > 350) {
		pointLightLinear = 0.055f;
		pointLightQuadratic = 0.012f;
	}
	else if (shotTime > 300) {
		pointLightLinear = 0.07f;
		pointLightQuadratic = 0.017f;
	}
	else if (shotTime > 250) {
		pointLightLinear = 0.08f;
		pointLightQuadratic = 0.025f;
	}
	else if (shotTime > 200) {
		pointLightLinear = 0.09f;
		pointLightQuadratic = 0.032f;
	}
	else if (shotTime > 150) {
		pointLightLinear = 0.11f;
		pointLightQuadratic = 0.05f;
	}
	else if (shotTime > 100) {
		pointLightLinear = 0.14f;
		pointLightQuadratic = 0.07f;
	}
	else if (shotTime > 50) {
		pointLightLinear = 0.18f;
		pointLightQuadratic = 0.12f;
	}
	else {
		pointLightLinear = 0.22f;
		pointLightQuadratic = 0.20f;
	}

	myCustomShader.useShaderProgram();
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLinear"), pointLightLinear);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightQuadratic"), pointLightQuadratic);
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	canonAnimation();

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		if (night && isFog == 0.0f) {
			mySkyBoxNight.Draw(skyboxShader, view, projection);
		}
		else if (isFog == 0.0f) {
			mySkyBox.Draw(skyboxShader, view, projection);
		}

	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();
	initSkyboxNight();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
