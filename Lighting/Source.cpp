#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>
#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include "stb_image.h"
#include "Model.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int LoadTexture(const char* path, const string& directory);
void createFrameBuffer(unsigned int* fbo, unsigned int* texColorBuffer, unsigned int* rbo, GLint Format);
void createFrameBuffer(unsigned int* fbo, unsigned int* texColorBuffer, const char* format);
unsigned int loadCubeMap(vector<std::string> texture_faces);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
const unsigned int SHADOW_SIZE = 4096;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Light source Position
//glm::vec3 lightSourcePos = glm::vec3(0.5f, 0.0f, 2.0f);

float SpotLightInnerCutOff = 10.0f, SpotLightOuterCutOff = 12.5f;
int NR_POINT_LIGHTS = 1;

glm::vec3 lightPos = glm::vec3(-1.0f, 4.0f, -5.0f);

int main() {
	//INITIALIZING GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tutorial", NULL, NULL);
	if (window == NULL) {
		std::cout << "FAILED TO CREATE WINDOW\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//SHADER INITIALISATION
	//Shader LightingShader("objectShader.vs", "FinalObjectShader.fs", parameter.c_str());
	Shader myShader("objectShader.vs", "FinalObjectShader.fs");
	//Shader LightingShader("objectShader.vs", "objectShader.fs");
	//std::cout << "LightingShaderDone\n";
	Shader LightCubeShader("lightShader.vs", "lightShader.fs");
	//std::cout << "LightCubeShaderDone\n";
	Shader BackgroundShader("back.vs", "back.fs");
	Shader PPShader("PP.vs", "PP.fs");
	Shader SimpleDepthShader("SimpleDepthShader.vs", "SimpleDepthShader.fs");
	Shader SkyBoxShader("SkyBox.vs", "SkyBox.fs");
	Shader ReflectionShader("Reflection.vs", "Refraction.fs");
	Shader PointDepthShader("PointDepthShader.vs", "PointDepthShader.fs", "PointDepthShader.gs");

	unsigned int backTex = LoadTexture("get.png", "textures");

	//vertices for some objects like cubes, quads, point light positions and colors
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	float background[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  50.0f, 50.0f
	};

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	   // positions   // texCoords
	   -1.0f,  1.0f,  0.0f, 1.0f,
	   -1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

	   -1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  4.0f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glm::vec3 pointLightColors[] = {
	glm::vec3(0.8f, 0.8f, 0.6f),
	glm::vec3(0.3f, 0.3f, 0.7f),
	glm::vec3(0.0f, 0.0f, 0.3f),
	glm::vec3(0.4f, 0.4f, 0.4f)
	};

	
	//VAOs, VBOs for in scene objects 
	unsigned int VBO, backgroundVBO, lightVAO, backgroundVAO;
	glGenVertexArrays(1, &lightVAO);
	glGenVertexArrays(1, &backgroundVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &backgroundVBO);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(backgroundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(background), background, GL_STATIC_DRAW);
	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// screen quad VAO for PostProcessing
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	//CUBEMAP
	vector<std::string> faces = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubeMap(faces);

	//VAO, VBO FOR SKYBOX
	unsigned int SkyBoxVAO, SkyBoxVBO;
	glGenVertexArrays(1, &SkyBoxVAO);
	glGenBuffers(1, &SkyBoxVBO);
	glBindVertexArray(SkyBoxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, SkyBoxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//FrameBuffer for point light depth map
	unsigned int depthCubeFBO, depthCubeMap;
	glGenTextures(1, &depthCubeMap);
	const unsigned int POINT_SHADOW_SIZE = 1024;
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			POINT_SHADOW_SIZE, POINT_SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &depthCubeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//FRAMEBUFFERS FOR ANTI_ALIASING AND POST PROCESSING
	unsigned int MSAAFrameBuffer, MSAATexColorBuffer, MSAArbo; //MultiSample Anti-Aliasing
	createFrameBuffer(&MSAAFrameBuffer, &MSAATexColorBuffer, &MSAArbo, GL_RGB);
	unsigned int PPFrameBuffer, PPTexColorBuffer, PPrbo; //Post Processing
	createFrameBuffer(&PPFrameBuffer, &PPTexColorBuffer, &PPrbo, GL_RGB);


	//SETTING UP FRAMEBUFFER FOR DEPTH MAP GENERATION
	unsigned int DepthMapFrameBuffer, depthMap;
	createFrameBuffer(&DepthMapFrameBuffer, &depthMap, "DEPTH");

	PPShader.use();
	PPShader.setInt("screenTexture", 0);

	glEnable(GL_DEPTH_TEST);

	stbi_set_flip_vertically_on_load(true);
	//glEnable(GL_MULTISAMPLE);

	//INITIALIZING MODELS
	Model myModel("backpack/backpack.obj");
	//Model myModel("Sponza-Master/sponza.obj");
	glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);

	glDepthFunc(GL_LEQUAL);

	//RENDER LOOP
	while (!glfwWindowShouldClose(window)) {

	    // per-frame time logic
	   // --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
				
		processInput(window);

		pointLightPositions[0].x = 2.0 * sin(glfwGetTime());
		pointLightPositions[0].z = 2.0 * cos(glfwGetTime());

		//lightSourcePos = glm::vec3(1.414*(float)sin(glfwGetTime()), 0.0f, 1.414*(float)cos(glfwGetTime()));

		// render
       // ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//FIRST LIGHTING PASS 
		//GENERATE DIRECTIONAL DEPTH MAP
		
		float near_plane = 0.1f, far_plane = 20.0f;
		glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos,
										  glm::vec3(0.0f, 0.0f, 0.0f),
										  glm::vec3(0.0f, 1.0f, 0.0f));
		//lightProjection = glm::perspective((float)glm::radians(45.0), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		SimpleDepthShader.use();
		SimpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFrameBuffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		glm::mat4 lightmodel = glm::mat4(1.0f);
		lightmodel = glm::translate(lightmodel, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		lightmodel = glm::scale(lightmodel, size);
		SimpleDepthShader.setMat4("model", lightmodel);
		myModel.Draw(SimpleDepthShader);
		lightmodel = glm::mat4(1.0f);
		lightmodel = glm::translate(lightmodel, glm::vec3(0.0f, -2.5f, 0.0f));
		SimpleDepthShader.setMat4("model", lightmodel);
		glBindVertexArray(backgroundVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// SECOND LIGHTING PASS
		// GENERATE OMNIDIRCETIONAL DEPTH MAP
		glViewport(0, 0, POINT_SHADOW_SIZE, POINT_SHADOW_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		float aspect = (float)POINT_SHADOW_SIZE / (float)POINT_SHADOW_SIZE;
		float near = 1.0f;
		float far = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		PointDepthShader.use();
		lightmodel = glm::mat4(1.0f);
		for (unsigned int i = 0; i < 6; ++i) {
			PointDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		PointDepthShader.setVec3("lightPos", pointLightPositions[0]);
		PointDepthShader.setFloat("far_plane", far);
		lightmodel = glm::translate(lightmodel, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		lightmodel = glm::scale(lightmodel, size);
		PointDepthShader.setMat4("model", lightmodel);
		myModel.Draw(PointDepthShader);
		lightmodel = glm::mat4(1.0f);
		lightmodel = glm::translate(lightmodel, glm::vec3(0.0f, -2.5f, 0.0f));
		PointDepthShader.setMat4("model", lightmodel);
		glBindVertexArray(backgroundVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		//COLOR RENDER PASS 
		//RENDER SCENE WITH MULTISAMPLING FRAMEBUFFER
		glDisable(GL_CULL_FACE);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, MSAAFrameBuffer); //Using Anti-Aliasing FrameBuffer to create scene
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//VIEW AND PROJECTION MATRICES
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//SETTING UP SHADER VALUES FOR MAIN MODEL
		myShader.use();

		//myShader.setFloat("material.shininess", 31.0f);

		//Setting up Directional Light
		myShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
		myShader.setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
		myShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
		myShader.setVec3("dirLight.direction", lightPos);

		//Setting up Point Lights
		for (unsigned int i = 0; i < NR_POINT_LIGHTS; i++) {
			std::string name = "pointLight[" + std::to_string(i) + "].";
			myShader.setVec3(name + "ambient", pointLightColors[i] * 0.1f);
			myShader.setVec3(name + "diffuse", pointLightColors[i]); // darken diffuse light a bit
			myShader.setVec3(name + "specular", 1.0f, 1.0f, 1.0f);
			myShader.setVec3(name + "position", pointLightPositions[i]);
			myShader.setFloat(name + "AttConstant", 1.0f);
			myShader.setFloat(name + "AttLinear", 0.09f);
			myShader.setFloat(name + "AttQuadratic", 0.032f);
		}

		myShader.setMat4("projection", projection);
		myShader.setMat4("view", view);
		myShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		myShader.setFloat("far_plane", far);
		myShader.setInt("shadowMap", 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		myShader.setInt("shadowCube", 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f)); 

		myShader.setInt("material.texture_diffuse1", 0);
		myShader.setInt("material.texture_specular1", 0);
		myShader.setMat4("model", model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, backTex);
		glBindVertexArray(backgroundVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, size);
		myShader.setMat4("model", model);
		myShader.setInt("shadowMap", 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		myModel.Draw(myShader); 

		//SETTING UP SHADER VALUES FOR LIGHT CUBES
		LightCubeShader.use();
		LightCubeShader.setMat4("view", view);
		LightCubeShader.setMat4("projection", projection);

		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(1.0f));
		LightCubeShader.setMat4("model", model);
		LightCubeShader.setVec3("Color", pointLightColors[0]);

		//render the cube
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		for (int i = 0; i < NR_POINT_LIGHTS; i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			LightCubeShader.setMat4("model", model);
			LightCubeShader.setVec3("Color", pointLightColors[i]);

			//render the cube
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//SKYBOX
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

		glDepthMask(GL_FALSE);
		SkyBoxShader.use();
		SkyBoxShader.setMat4("projection", projection);
		SkyBoxShader.setMat4("view", view);
		glBindVertexArray(SkyBoxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);


		//DOWNSCALING MULTISAMPLED FRAMEBUFFER
		glBindFramebuffer(GL_READ_FRAMEBUFFER, MSAAFrameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, PPFrameBuffer);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//POST PROCESSING PASS

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PPShader.use();
		PPShader.setInt("screenTexture", 0);
		glBindVertexArray(quadVAO);
		//glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PPTexColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

//PROCESSING INPUT
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		if (SpotLightInnerCutOff == 0) {
			SpotLightInnerCutOff = 10.0f;
			SpotLightOuterCutOff = 12.5f;
		}
		else {
			SpotLightInnerCutOff = 0.0f;
			SpotLightOuterCutOff = 0.0f;
		}
}

//CALLBACK FOR ADJUSTING SIZE OF WINDOW
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double Xpos, double Ypos) {
	if (firstMouse)
	{
		lastX = Xpos;
		lastY = Ypos;
		firstMouse = false;
	}

	float xoffset = Xpos - lastX;
	float yoffset = lastY - Ypos; // reversed since y-coordinates go from bottom to top

	lastX = Xpos;
	lastY = Ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double Xoffset, double Yoffset) {
	camera.ProcessMouseScroll(Yoffset);
}

unsigned int LoadTexture(const char* path, const string& directory)
{
	string filename = string(path);
	bool isDiffuse = (filename == "diffuse.jpg") ? true : false;
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format, outformat;
		if (nrComponents == 1) {
			format = GL_RED;
			outformat = GL_RED;
		}
		else if (nrComponents == 3) {
			format = GL_RGB;
			outformat = (isDiffuse) ? GL_SRGB : GL_RGB;
		}
		else if (nrComponents == 4) {
			format = GL_RGBA;
			outformat = (isDiffuse) ? GL_SRGB_ALPHA : GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, outformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void createFrameBuffer(unsigned int* fbo, unsigned int* texColorBuffer, unsigned int* rbo, GLint Format) {
	glGenFramebuffers(1, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

	glGenTextures(1, texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, *texColorBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (Format == GL_RGB) {
		glTexImage2D(GL_TEXTURE_2D, 0, Format, SCR_WIDTH, SCR_HEIGHT, 0, Format, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texColorBuffer, 0);
	}
	else if (Format == GL_DEPTH_COMPONENT) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *texColorBuffer, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	glGenRenderbuffers(1, rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, *rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::Framebuffer is not complete" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

}

void createFrameBuffer(unsigned int* fbo, unsigned int* texColorBuffer, const char* format) {
	glGenFramebuffers(1, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

	glGenTextures(1, texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, *texColorBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (format == "RGB") {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texColorBuffer, 0);
	}
	else if (format == "DEPTH") {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *texColorBuffer, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::Framebuffer is not complete" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int loadCubeMap(vector<std::string> texture_faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < texture_faces.size(); i++) {
		
		unsigned char* data = stbi_load(texture_faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap tex failed to load at path: " << texture_faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
