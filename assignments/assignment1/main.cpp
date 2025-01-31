#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include "ew/shader.h"
#include "ew/model.h"
#include "ew/transform.h"
#include "ew/camera.h"
#include "ew/cameraController.h"
#include "ew/texture.h"

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
unsigned int fbo;

ew::Transform monkeyTransform;
ew::Camera camera;
ew::CameraController cameraController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//set global vars
	glEnable(GL_CULL_FACE);
	glEnable(GL_BACK);//back face culling
	glEnable(GL_DEPTH_TEST);//depth testing

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//create a shader from fragment and vertex shaders
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	//load model ^

	//create a camera
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target - glm::vec3(0.0f, 0.0f, 0.0f);//look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;//Vertical fov in degrees

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//target, attachment, textarget, texture, mipmap level
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); 
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, texture, 0);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);



		cameraController.move(window, &camera, deltaTime);

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear backbuffer values

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		/*glActiveTexture(GL_TEXTURE);//old style
		glBindTexture(GL_TEXTURE_2D, brickTexture);*/

		glBindTextureUnit(0, brickTexture);

		shader.use();
		shader.setInt("_MainTex",0);
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);
		monkeyModel.draw();


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
