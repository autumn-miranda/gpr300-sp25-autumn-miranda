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

#include "assets1/AnimationClip.h"
#include "assets1/Animator.h"
#include "assets1/Vec3Key.h"
#include "assets1/Skeleton.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

void drawMonkeySkeleton(ew::Model* monkeyModel, ew::Transform* monkeyTransform, anm::Skeleton* monkeySkeleton);
void createJointGUI(int i);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//vars for shadows
float near_plane = 1.0f, far_plane = 10.0f;
float lightPosX = -2.0f;
float lightPosY = 4.0f;
float lightPosZ = -1.0f;
float minBias = 0.005f;
float maxBias = 0.015f;


ew::Transform monkeyTransform;
anm::Skeleton* monkeySkeleton;
ew::Camera camera;
ew::CameraController cameraController;


anm::AnimationClip animation;
anm::Animator animator;

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
	GLFWwindow* window = initWindow("Forward Kinematics", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//set global vars
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);//back face culling
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

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


	ew::Mesh sphereMesh(ew::createSphere(1.5f,20));

	ew::Transform sphereTransform;
	sphereTransform.position.x = 2.3f;
	sphereTransform.position.y = -2.0f;
	sphereTransform.position.z = 0.5f;

	//plane
	float planeWidth = 10.0f;
	float planeHeight = 10.0f;
	int planeSubdivisions = 4;

	//Create Shapes
	ew::Mesh planeMesh(ew::createPlane(planeWidth, planeHeight, planeSubdivisions));

	ew::Transform planeTransform;
	planeTransform.position.x = 1.5f;
	planeTransform.position.y = -2.0f;
	planeTransform.position.z = 0.0f;

	//Assignment 3 code
	animation.setModel(&monkeyTransform);

	monkeySkeleton = new anm::Skeleton(monkeyTransform);
	

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//draw as a wireframe
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		animator.playAnimation(animation, deltaTime);

		cameraController.move(window, &camera, deltaTime);

		//monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		glm::vec3 lightPos(lightPosX, lightPosY, lightPosZ);
		
		//create lightProjection variables
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos,
			glm::vec3(0.0f, 0.0f, 0.0f),//look at center of the scene
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		glDisable(GL_CULL_FACE);
		
		shadowShader.use();
		shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		//glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//shadowShader.setMat4("_Model", monkeyTransform.modelMatrix());
		//monkeyModel.draw();
		for (anm::Skeleton::Joint j : monkeySkeleton->modelSkeleton)
		{
			monkeyTransform = j.getGlobalTransform();
			shadowShader.setMat4("_Model", monkeyTransform.modelMatrix());
			monkeyModel.draw();
		}

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		//glCullFace(GL_BACK);
		shadowShader.setMat4("_Model", sphereTransform.modelMatrix());
		sphereMesh.draw();
		shadowShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);//back face culling

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

		shader.setInt("diffuseTexture", 0);
		shader.setInt("shadowMap", 1);
		shader.setFloat("minBias", minBias);
		shader.setFloat("maxBias", maxBias);
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		//shader.setMat4("_Model", monkeyTransform.modelMatrix());
		//monkeyModel.draw();
		for (anm::Skeleton::Joint j : monkeySkeleton->modelSkeleton)
		{
			monkeyTransform = j.getGlobalTransform();
			shader.setMat4("_Model", monkeyTransform.modelMatrix());
			monkeyModel.draw();
		}
		shader.setMat4("_Model", sphereTransform.modelMatrix());
		sphereMesh.draw();
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
	if (ImGui::CollapsingHeader("Shadows"))
	{
		if (ImGui::Button("Reset Light"))
		{
			lightPosX = -2.0f;
			lightPosY = 4.0f;
			lightPosZ = -1.0f;
		}
		ImGui::SliderFloat("Min Bias", &minBias, 0.0f, 0.05f);
		ImGui::SliderFloat("Max Bias", &maxBias, 0.0f, 0.05f);
		ImGui::SliderFloat("Light X", &lightPosX, 0.0f, 20.0f);
		ImGui::SliderFloat("Light Y", &lightPosY, 0.0f, 20.0f);
		ImGui::SliderFloat("Light Z", &lightPosZ, -10.0f, 10.0f);
		ImGui::SliderFloat("Near Plane", &near_plane, -10.0f, 5.0f);
		ImGui::SliderFloat("Far Plane", &far_plane, 5.0f, 50.0f);

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

	ImGui::Begin("Animation");


	if (ImGui::CollapsingHeader("Play Back"))
	{
		
		ImGui::Checkbox("Playing", &animator.isPlaying);
		ImGui::Checkbox("Loop", &animator.isLooping);

		//ImGui::DragFloat is unbounded
		ImGui::SliderFloat("Playback Speed", &animator.playbackSpeed, -5.0f, 5.0f);
		ImGui::SliderFloat("Playback Time", &animator.playbackTime, 0.0f, animation.maxDuration);
		ImGui::SliderFloat("Duration", &animation.maxDuration, 0.0f, 10.0f);
		
	}
	if (ImGui::CollapsingHeader("Position Keys"))
	{
		if (ImGui::Button("Reset##pos")){
			animation.clearPosArray();
			animation.setModelPos(animation.modelDefaults.position);
		}
		if (ImGui::Button("Add Frame##pos")) {
			animation.addKeyFrame(animation.getPosArray(), animator.playbackTime, monkeyTransform.position);
		}
		if (ImGui::Button("Remove Frame##pos")) {
			animation.removeKeyFrame(animation.getPosArray());
		}

		for (int i = 0; i < animation.getPosArray().size(); i++)
		{
			const char* itemNames[5] = {
			  "Default##pos",
			  "EaseInBack##pos",
			  "EaseInElastic##pos",
			  "EaseOutBack##pos",
			  "EaseOutCubic##pos"
			};

			ImGui::PushID(i+"##pos");
			ImGui::DragFloat("Time##pos", &animation.getPosArray()[i].time, 0.1f, 0.f, animation.maxDuration);
			ImGui::DragFloat3("Position##pos", &animation.getPosArray()[i].value.x, 0.1f);
			ImGui::Combo("Easing##pos", &animation.getPosArray()[i].easing, itemNames, 5);
			ImGui::Text("  ");
			ImGui::PopID();
		}

	}
	if (ImGui::CollapsingHeader("Rotation Keys"))
	{
		if (ImGui::Button("Reset##rot")) {
			animation.clearRotArray();
			animation.setModelRot(glm::degrees(glm::eulerAngles(animation.modelDefaults.rotation)));
		}
		if (ImGui::Button("Add Frame##rot")) {
			animation.addKeyFrame(animation.getRotArray(), animator.playbackTime, eulerAngles(monkeyTransform.rotation));
		}
		if (ImGui::Button("Remove Frame##rot")) {
			animation.removeKeyFrame(animation.getRotArray());
		}

		for (int i = 0; i < animation.getRotArray().size(); i++)
		{
			const char* itemNames[5] = {
			  "Default##rot",
			  "EaseInBack##rot",
			  "EaseInElastic##rot",
			  "EaseOutBack##rot",
			  "EaseOutCubic##rot"
			};


			ImGui::PushID(i+"##rot");
			ImGui::DragFloat("Time##rot", &animation.getRotArray()[i].time, 0.1f, 0.f, animation.maxDuration);
			ImGui::DragFloat3("Rotation##rot", &animation.getRotArray()[i].value.x, 0.1f);
			ImGui::Combo("Easing##rot", &animation.getRotArray()[i].easing, itemNames, 5);
			ImGui::Text("  ");
			ImGui::PopID();
		}
	}
	if (ImGui::CollapsingHeader("Scale Keys"))
	{
		if (ImGui::Button("Reset##scale")) {
			animation.clearScaleArray();
			animation.setModelScale(animation.modelDefaults.scale);

		}
		if (ImGui::Button("Add Frame##scale")) {
			animation.addKeyFrame(animation.getScaleArray(), animator.playbackTime, monkeyTransform.scale);
		}
		if (ImGui::Button("Remove Frame##scale")) {
			animation.removeKeyFrame(animation.getScaleArray());
		}

		for (int i = 0; i < animation.getScaleArray().size(); i++)
		{
			const char* itemNames[5] = {
			  "Default##scale",
			  "EaseInBack##scale",
			  "EaseInElastic##scale",
			  "EaseOutBack##scale",
			  "EaseOutCubic##scale"
			};


			ImGui::PushID(i + "##scale");
			ImGui::DragFloat("Time##scale", &animation.getScaleArray()[i].time, 0.1f, 0.f, animation.maxDuration);
			ImGui::DragFloat3("Scale##scale", &animation.getScaleArray()[i].value.x, 0.1f, 0.1f, 10.f);
			ImGui::Combo("Easing##scale", &animation.getScaleArray()[i].easing, itemNames, 5);
			ImGui::Text("  ");
			ImGui::PopID();
		}
	}
	ImGui::End();

	ImGui::Begin("Skeleton");
	createJointGUI(0);
	/*for (int i = 0; i < 8; i++)
	{
		const char* label = const_cast<char*>(monkeySkeleton->jointNames[i].c_str());
		if (ImGui::CollapsingHeader(label)) {
			float tempPos[3] = { monkeySkeleton->modelSkeleton[i].localTransform.position.x,
									monkeySkeleton->modelSkeleton[i].localTransform.position.y,
									monkeySkeleton->modelSkeleton[i].localTransform.position.z
			};

			float tempRot[3] = { monkeySkeleton->modelSkeleton[i].localTransform.rotation.x,
									monkeySkeleton->modelSkeleton[i].localTransform.rotation.y,
									monkeySkeleton->modelSkeleton[i].localTransform.rotation.z
			};

			float tempScale[3] = { monkeySkeleton->modelSkeleton[i].localTransform.scale.x,
									monkeySkeleton->modelSkeleton[i].localTransform.scale.y,
									monkeySkeleton->modelSkeleton[i].localTransform.scale.z
			};

			ImGui::PushID(i + "##Joint");

			if (ImGui::DragFloat3("Joint Position##i", tempPos, 0.1f)) 
			{
				monkeySkeleton->modelSkeleton[i].localTransform.position = glm::vec3(tempPos[0], tempPos[1], tempPos[2]);
				monkeySkeleton->calcGlobalTransforms(i);
			}
			if (ImGui::DragFloat3("Joint Rotation##i", tempRot, 0.1f)) 
			{
				monkeySkeleton->modelSkeleton[i].localTransform.rotation = glm::vec3(tempRot[0], tempRot[1], tempRot[2]);
				monkeySkeleton->calcGlobalTransforms(i);
			}
			if (ImGui::DragFloat3("Joint Scale##i", tempScale, 0.1f)) {
				monkeySkeleton->modelSkeleton[i].localTransform.scale = glm::vec3(tempScale[0], tempScale[1], tempScale[2]);
				monkeySkeleton->calcGlobalTransforms(i);
			}

			ImGui::Text("  ");
			ImGui::PopID();
		}
	}*/
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

void drawMonkeySkeleton(ew::Model* monkeyModel, ew::Transform* monkeyTransform, anm::Skeleton* monkeySkeleton)
{
	for (anm::Skeleton::Joint j : monkeySkeleton->modelSkeleton) 
	{
		monkeyTransform = &j.getGlobalTransform();
		monkeyModel->draw();
	}
}

void createJointGUI(int i) 
{
	const char* label = const_cast<char*>(monkeySkeleton->jointNames[i].c_str());
	if (ImGui::CollapsingHeader(label)) {
		float tempPos[3] = { monkeySkeleton->modelSkeleton[i].localTransform.position.x,
								monkeySkeleton->modelSkeleton[i].localTransform.position.y,
								monkeySkeleton->modelSkeleton[i].localTransform.position.z
		};

		float tempRot[3] = { monkeySkeleton->modelSkeleton[i].localTransform.rotation.x,
								monkeySkeleton->modelSkeleton[i].localTransform.rotation.y,
								monkeySkeleton->modelSkeleton[i].localTransform.rotation.z
		};

		float tempScale[3] = { monkeySkeleton->modelSkeleton[i].localTransform.scale.x,
								monkeySkeleton->modelSkeleton[i].localTransform.scale.y,
								monkeySkeleton->modelSkeleton[i].localTransform.scale.z
		};

		ImGui::PushID(i + "##Joint");

		if (ImGui::DragFloat3("Joint Position##i", tempPos, 0.1f))
		{
			monkeySkeleton->modelSkeleton[i].localTransform.position = glm::vec3(tempPos[0], tempPos[1], tempPos[2]);
			monkeySkeleton->calcGlobalTransforms(i);
		}
		if (ImGui::DragFloat3("Joint Rotation##i", tempRot, 0.1f))
		{
			monkeySkeleton->modelSkeleton[i].localTransform.rotation = glm::vec3(tempRot[0], tempRot[1], tempRot[2]);
			monkeySkeleton->calcGlobalTransforms(i);
		}
		if (ImGui::DragFloat3("Joint Scale##i", tempScale, 0.1f)) {
			monkeySkeleton->modelSkeleton[i].localTransform.scale = glm::vec3(tempScale[0], tempScale[1], tempScale[2]);
			monkeySkeleton->calcGlobalTransforms(i);
		}


		ImGui::Text("  ");
		ImGui::PopID();

		for (int child : monkeySkeleton->modelSkeleton[i].childIndex)
		{
			createJointGUI(child);
		}

	}
}



/*
for (int i = 0; i < numLights; i++)
{
	ImGui::PushID(i);
	if (ImGui::CollapsingHeader("Light")) {
		ImGui::DragFloat3("Position", &lights[i].position.x, 0.1f);
		ImGui::ColorEdit3("Color", &lights[i].color.x);
	}
	ImGui::PopID();
}


int currItem = 0;
const char* itemNames[3]={
  "Option A",
  "Option B",
  "Option C"
}
ImGui::Combo("My Enum",&currItem,itemNames,3);



*/