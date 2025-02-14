#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include "ew/shader.h"
#include "ew/model.h"
#include "ew/transform.h"
#include "ew/camera.h"
#include "ew/cameraController.h"
#include "ew/texture.h"
#include "ew/procGen.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Transform monkeyTransform;
ew::Camera camera;
ew::CameraController cameraController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

//framebuffer variables
unsigned int fbo;
unsigned int dummyVAO;
unsigned int textureColorbuffer;
unsigned int depthBuffer;
unsigned int depthMap;

//tutorial effects
bool boxBlur;
bool invert;

//my effects
bool sharpen;
bool custom;
bool edge;

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//set global vars
	glEnable(GL_CULL_FACE);
	glEnable(GL_BACK);//back face culling
	glEnable(GL_DEPTH_TEST);//depth testing
	glDepthMask(GL_TRUE);

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//create a shader from fragment and vertex shaders
	ew::Shader shadowShader = ew::Shader("assets1/depthMap.vert", "assets1/depthMap.frag");
	ew::Shader shader = ew::Shader("assets1/lighting.vert", "assets1/lighting.frag");
	ew::Shader screenShader = ew::Shader("assets1/post.vert", "assets1/post.frag");
	
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	//load model ^

	//create a camera
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target - glm::vec3(0.0f, 0.0f, 0.0f);//look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;//Vertical fov in degrees

	/*Assignment 2 code*/
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//dummy vao
	glCreateVertexArrays(1, &dummyVAO);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//plane
	float planeWidth = 100.0f;
	float planeHeight = 100.0f;
	int planeSubdivisions = 5;

	//Create Shapes
	ew::Mesh planeMesh(ew::createPlane(planeWidth, planeHeight, planeSubdivisions));

	ew::Transform planeTransform;
	planeTransform.position.x = 1.5f;
	planeTransform.position.y = -10.0f;
	planeTransform.position.z = 0.0f;


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//draw as a wireframe
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		cameraController.move(window, &camera, deltaTime);

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
		
		//create lightProjection variables
		float near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0, 4.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;


		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		shadowShader.use();
		shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		//glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		shadowShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();
		shadowShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//RENDER
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glNamedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);//draw to custom frame buffer 
		glEnable(GL_DEPTH_TEST);//depth testing
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear backbuffer values

		

		glBindTextureUnit(0, brickTexture);
		glBindTextureUnit(1, depthMap);

		shader.use();
		shader.setMat4("projection", camera.projectionMatrix());
		shader.setMat4("view", camera.viewMatrix());
		// set light uniforms
		shader.setVec3("viewPos", camera.position);
		shader.setVec3("lightPos", lightPos);
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		shader.setInt("_MainTex", 0);
		shader.setInt("shadowMap", 1);
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);
		monkeyModel.draw();
		shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();


		screenShader.use();
		screenShader.setInt("shadeBlur", boxBlur);
		screenShader.setInt("sharpen", sharpen);
		screenShader.setInt("invert", invert);
		screenShader.setInt("outline", edge);
		screenShader.setInt("custom", custom);
		screenShader.setFloat("_Time", time);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.6f, 0.09f, 0.92f, 1.0f);
		// Bind the texture we just rendered to for reading
		glBindTextureUnit(0, textureColorbuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(dummyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	//ImGui::Text("Add Controls Here!");
	if (ImGui::Button("Reset Camera")) 
	{
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) 
	{
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	if (ImGui::CollapsingHeader("Post-Processing"))
	{
		ImGui::Checkbox("Box Blur", &boxBlur);
		ImGui::Checkbox("Sharpen", &sharpen);
		ImGui::Checkbox("Outline", &edge);
		ImGui::Checkbox("Custom", &custom);

		ImGui::Checkbox("Invert", &invert);
	}
	ImGui::End();

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)depthMap, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target - glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}
