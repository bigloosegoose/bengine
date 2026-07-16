#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <random>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "stb_image.h"
#include "my_texture.h"
#include "my_UI.h"
#include "model.h"
#include "camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

SoLoud::Soloud soloud; //audio engine
SoLoud::Wav sample;     //one wave file

constexpr float PI = 3.1415926535f;
using std::cout, std::endl, std::string, std::vector;

//settings
unsigned int width = 800;
unsigned int height = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

bool cursorEnabled = false;

//functions
void processInput(GLFWwindow* window);

void framebuffer_size_callback(GLFWwindow* window, int fwidth, int fheight);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void toggleMouse(GLFWwindow* window);

void uiDefine(ImGuiIO io);
void uiDraw();
void uiRefresh();
void uiClickCheck();

unsigned int loadCubemap(vector<string> faces);

float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

float quadVertices[] = {
	// positions	// texCoords
	-1.0f,  1.0f,	0.0f, 1.0f,	
	-1.0f, -1.0f,	0.0f, 0.0f,
	 1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f,  1.0f,	0.0f, 1.0f,
	 1.0f, -1.0f,	1.0f, 0.0f,
	 1.0f,  1.0f,	1.0f, 1.0f
};


float points[] = {
	-0.5f,
0.5f, 1.0f, 0.0f, 0.0f, // top-left
0.5f,
0.5f, 0.0f, 1.0f, 0.0f, // top-right
0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
-0.5f, -0.5f, 1.0f, 1.0f, 0.0f
// bottom-left
};

//UI ELEMENTS
std::unique_ptr<UI> cross_button;

//****************************************************************************************************************************************
int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(width, height, "SCWEEN", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//important -> load all the function pointers for openGL before calling any of its functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	//setting up imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	//Setup platform for imgui
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//setting up soloud(audio)
	soloud.init();
	sample.load("audio\\LOOP1.wav");
	sample.setLooping(true);
	sample.setVolume(0.25f);

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	camera.SetCameraMode(FLY);




	//***********************************************************************************************
	//quad VAO, VBO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	//VertexPos Attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//texture attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//***********************************************************************************************
	//frame buffers
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	unsigned int texColorBuffer; 
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0); // unbinding so its not interfered with / interfering
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);


	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	Shader screenShader(R"(shaders/lab.vert)", R"(shaders/lab.frag)");
	Shader buttonShader(R"(shaders/lab.vert)", R"(shaders/lab.frag)");

	//to be executed last
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//***********************************************************************************************

	//skybox
	vector<string> textures_faces;
	textures_faces.push_back(R"(textures/skybox/right.jpg)");
	textures_faces.push_back(R"(textures/skybox/left.jpg)");
	textures_faces.push_back(R"(textures/skybox/top.jpg)");
	textures_faces.push_back(R"(textures/skybox/bottom.jpg)");
	textures_faces.push_back(R"(textures/skybox/front.jpg)");
	textures_faces.push_back(R"(textures/skybox/back.jpg)");
	unsigned int skyboxTexture = loadCubemap(textures_faces);


	Shader skyboxShader(R"(shaders\skybox.vert)", R"(shaders\skybox.frag)");

	unsigned int skyboxVAO, skyboxVBO;
	glGenBuffers(1, &skyboxVBO);
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	//VertexPos Attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);



	//***********************************************************************************************
	Shader lightingShader(R"(shaders\lightingShader.vert)", R"(shaders\lightingShader.frag)");
	lightingShader.use();
	lightingShader.setFloat("material.shininess", 32.0f);

	//temp box shader(cleaner yippeee)
	Shader boxShader(R"(shaders\temp.vert)", R"(shaders\temp.frag)", R"(shaders\basicGeometryShader.geo)");
	Shader lightSourceShader(R"(shaders\lightSourceShader.vert)", R"(shaders\lightSourceShader.frag)");

	Shader basicTextureShader(R"(shaders\basicTexture.vert)", R"(shaders\basicTexture.frag)");
	Shader instanceShader(R"(shaders\instanceShader.vert)", R"(shaders\instanceShader.frag)");
	Shader starInstanceShader(R"(shaders\starInstanceShader.vert)", R"(shaders\starInstanceShader.frag)");

	Model backpack("models/backpack/backpack.obj");
	Model planet("models/planet/planet.obj");
	Model rock("models/rock/rock.obj");
	Model star("models/star/octahedron.obj");
	//***********************************************************************************************
	//Aesteorids and stars!

	//setting up new random style
	std::random_device rdSeed; //random device, seed gen
	std::mt19937 gen(rdSeed()); // random engine, mersenne twister

	unsigned int amount = 1000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime()); //make a random seed
	float radius = 50.0;
	float offset = 2.5f;

	for (unsigned int i = 0; i < amount; i++) 
	{
		glm::mat4 model = glm::mat4(1.0);

		//putting aesteroids in the ring
		float angle = ((float)i / (float)amount) * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;


		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;

		model = glm::translate(model, glm::vec3(x,y,z));

		//give a random scaling(0.05 - 0.25)
		float scale = ((rand() % 20) / 100.0f) + 0.05f;
		model = glm::scale(model, glm::vec3(scale));

		//give a random rotation
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));

		//populating the array
		modelMatrices[i] = model;

	}


	std::uniform_real_distribution<float> disTheta(0.0f, 2.0f * PI);
	std::uniform_real_distribution<float> disZ(-1.0f, 1.0f);
	std::uniform_real_distribution<float> disScale(0.1f, 0.4f);

	unsigned int starAmount = 3000;
	float starRadius = 150.0f;
	glm::mat4* starModelMatrices;
	starModelMatrices = new glm::mat4[starAmount];

	//to generated random sphere coordinates
	//surface area of sphere = LSA of cylinder, so we just do that(2piRh)

	for (unsigned int i = 0; i < starAmount; i++)
	{
		float theta = disTheta(gen);
		float z = disZ(gen);
		float scale = disScale(gen);

		//getting a uniform direction on a unit sphere

		float r_xy = std::sqrt(1.0f - z * z);
		float x = (r_xy * cos(theta)); 
		float y = (r_xy * sin(theta));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(x * starRadius, y * starRadius, z * starRadius));
		model = glm::scale(model, glm::vec3(scale));
		model = glm::rotate(model, glm::radians(theta), glm::vec3(0.4f, 0.6f, 0.8f));

		starModelMatrices[i] = model;
	}

	glm::vec3 starColor = (glm::vec3(206.0f, 224.0f, 244.0f) / 255.0f);

	starInstanceShader.use();
	starInstanceShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	starInstanceShader.setVec3("dirLight.ambient", starColor * 0.7f);
	starInstanceShader.setVec3("dirLight.diffuse", starColor);
	starInstanceShader.setVec3("dirLight.specular", starColor);
	starInstanceShader.setVec3("viewPos", camera.Position);
	
	//***********************************************************************************************

	//temp
	MTexture grassTex(GL_TEXTURE_2D, "textures/metal.png", true, 0);
	MTexture playerTex(GL_TEXTURE_2D, "textures/LR.png", true, 0);
	MTexture asciiTex(GL_TEXTURE_2D, "textures/8x8.png", true, 0);
	mtWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	//making the UI
	cross_button =  std::make_unique<UI>(quadVAO, buttonShader, [window]() {glfwSetWindowShouldClose(window, true); });
	
	cross_button->convertPosition(false);
	cout << cross_button->width << "," << cross_button->height;
	cross_button->SetPosition(width - cross_button->width/ 2, height - cross_button->height / 2, false);
	cross_button->SetDimensions(0.1f, 0.1f);
	cross_button->SetTexture("textures/cross.png");
	//cout << width - cross_button->width / 2 << "," << height - cross_button->height / 2 << endl;

	//***********************************************************************************************
	//bloom implementation(for later)

	/*struct BloomMip {
		glm::ivec2 size;
		unsigned int texture;
	};

	vector<BloomMip> bloomMips;
	glm::ivec2 mipSize(width, height);

	unsigned int bloomFBO;
	glGenBuffers(1, &bloomFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);

	for (unsigned int i = 0; i < 3; i++) {


	}*/

	//***********************************************************************************************

	rock.setupInstanceBuffer(amount, modelMatrices);
	star.setupInstanceBuffer(starAmount, starModelMatrices);
	glm::mat4 rotation = glm::mat4(1.0f);
	glm::mat4 rotationX = glm::mat4(1.0f);
	rotationX = glm::rotate(rotationX, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//***********************************************************************************************

	//unbind all the stuff
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glLinkProgram(0);

	//start stuff
	soloud.play(sample);
	camera.Position = glm::vec3(0.0f, -20.0f, 100.0f);

	//######################################################################################################
	//render loop
	glViewport(0, 0, width, height);
	while (!glfwWindowShouldClose(window)) {

		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		uiClickCheck();
		processInput(window);

		//matrices
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width/(float)height, 0.1f, 150.0f);
		glm::mat4 view = camera.GetViewMatrix();
		

		//first pass bruh
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f));
		basicTextureShader.use();
		basicTextureShader.setMat4("model",model);
		basicTextureShader.setMat4("rotation",rotation);
		basicTextureShader.setMat4("view",view);
		basicTextureShader.setMat4("projection", projection);
		planet.Draw(basicTextureShader);

		instanceShader.use();
		rotation = glm::rotate(rotation, glm::radians(3.0f * deltaTime), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
		instanceShader.setMat4("view", view);
		instanceShader.setMat4("projection", projection);
		instanceShader.setMat4("rotation", rotation);
		instanceShader.setMat4("rotationX", rotationX);
		rock.DrawInstanced(instanceShader);

		glDepthFunc(GL_LEQUAL);
		starInstanceShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		starInstanceShader.setMat4("view", view);
		starInstanceShader.setMat4("projection", projection);
		star.DrawInstanced(starInstanceShader);
		glDepthFunc(GL_LESS);

		//second pass
		/*glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		screenShader.use();
		screenShader.setInt("screenTexture", 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);*/


		//unbind stuff 
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		//UI
		uiDefine(io);	//Design
		uiDraw();		//Render

		//swap
		glfwSwapBuffers(window);
		glfwPollEvents();


	}
//######################################################################################################

	soloud.deinit();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
		toggleMouse(window);
	}



}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; //reversed, y goes bottom to top
		lastX = xpos;
		lastY = ypos;

		clickPosX = xpos;
		clickPosY = ypos;

		if (cursorEnabled == false)
		{
		camera.ProcessMouseMovement(xoffset, yoffset, deltaTime);
		}
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera.ProcessKeyboard(DOWN, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.MovementSpeed = 15.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
		camera.MovementSpeed = 5.0f;
	}
	
	
}

void framebuffer_size_callback(GLFWwindow* window, int fwidth, int fheight) {
	glViewport(0, 0, fwidth, fheight);
	if (fwidth == 0 || fheight == 0) {
		return;
	}

	width = fwidth;
	height = fheight;

	screenWidth = fwidth;
	screenHeight = fheight;

	uiRefresh();

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS && cursorEnabled == true) 
	{
		mouseClicked = true;
		//cout << lastX - width/2 << "," <<height/2 - lastY<< endl;


	}
	else {
		mouseClicked = false;
	}
}

void toggleMouse(GLFWwindow* window) {
	if (cursorEnabled == true) {
		cursorEnabled = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else {
		cursorEnabled = true;
		firstMouse = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void uiDefine(ImGuiIO io) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Pending Panel");
		ImGui::Text("Application Average: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}
}

void uiDraw() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	for (unsigned int i = 0; i < ui_elements.size(); i++) {
		ui_elements[i]->DrawButton();
	}
}

void uiRefresh() {

	for (unsigned int i = 0; i < ui_elements.size(); i++) {
		ui_elements[i]->RefreshVars();
	}

}

void uiClickCheck() {
	for (unsigned int i = 0; i < ui_elements.size(); i++) {
		ui_elements[i]->CheckForCLick();
	}
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	stbi_set_flip_vertically_on_load(false);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	stbi_set_flip_vertically_on_load(true);

	return textureID;
}

//EWWW function
/*void outlineModel(Model model, Shader defShader, glm::vec3 position) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	defShader.use();

	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, position);
	defShader.setMat4("model", modelMat);
	model.Draw(defShader);



	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glEnable(GL_DEPTH_TEST);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);

	Shader outlineShader(R"(shaders/lightingShader.vert)", R"(shaders/temp.frag)");

	outlineShader.use();
	modelMat = glm::scale(modelMat, glm::vec3(1.01f));
	outlineShader.setVec3("viewPos", camera.Position);
	outlineShader.setMat4("view", view);
	outlineShader.setMat4("projection", projection);
	outlineShader.setMat4("model", modelMat);

	model.Draw(outlineShader);
	glDisable(GL_DEPTH_TEST);
	glStencilMask(0xFF);

}*/