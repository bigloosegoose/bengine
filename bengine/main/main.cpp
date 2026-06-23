#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "stb_image.h"
#include "my_texture.h"
#include "model.h"
#include "camera.h"
#include "sky.h"
#include "ocean.h"

using std::cout, std::endl, std::string, std::vector;

// render the FFT ocean (default) or the old cube/grass scene; toggle with O
bool oceanMode = true;

//settings
const unsigned int width = 800;
const unsigned int height = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;


//functions
void processInput(GLFWwindow* window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void outlineModel(Model model, Shader defShader, glm::vec3 position);


float vertices[] = {
	// positions		  //normals          // texture coords
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f, 0.0f,  0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f, 0.0f,  0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f, 0.0f,   1.0f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.0f,   1.0f,  0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 0.0f,   1.0f,  0.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.0f,   1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, 0.0f,   1.0f,  0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f, 0.0f,   1.0f,  0.0f, 0.0f, 0.0f
};

float vertices2[] = {
	-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
};

glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f, 0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f, 2.0f, -12.0f),
	glm::vec3(0.0f, 0.0f, -3.0f)
};


//****************************************************************************************************************************************
int main () {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);



	camera.SetCameraMode(FLY);

	//cube VAO, VBO
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//VertexPos Attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	//***********************************************************************************************
	Shader lightingShader(R"(shaders\lightingShader.vert)", R"(shaders\lightingShader.frag)");

	lightingShader.use();
	
	//setting the material and light (temp?)
	lightingShader.setFloat("material.shininess", 32.0f);

	
	//***********************************************************************************************
	//MTexture diffuseMap(GL_TEXTURE_2D, "textures/container2.png", true, 0);

	//MTexture specularMap(GL_TEXTURE_2D, "textures/container2_specular.png", true, 1);

	//GRASS
	unsigned int grassVAO, grassVBO;
	glGenVertexArrays(1, &grassVAO);
	glGenBuffers(1, &grassVBO);
	glBindVertexArray(grassVAO);

	

	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	//VertexPos Attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	vector<glm::vec3> grassPositions;
	grassPositions.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	grassPositions.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	grassPositions.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	grassPositions.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	grassPositions.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	Shader tempGrassShader(R"(shaders/lightingShader.vert)", R"(shaders/temp.frag)");
	

	MTexture grassTex(GL_TEXTURE_2D, "textures/grass.png", true, 0);

	tempGrassShader.use();
	tempGrassShader.setInt("material.texture_diffuse1", 0);
	//***********************************************************************************************

	Shader lightSourceShader(R"(shaders\lightSourceShader.vert)", R"(shaders\lightSourceShader.frag)");

	//HELL
	//***********************************************************************************************
	lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[0].constant", 1.0f);
	lightingShader.setFloat("pointLights[0].linear", 0.09f);
	lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
	// point light 2
	lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[1].constant", 1.0f);
	lightingShader.setFloat("pointLights[1].linear", 0.09f);
	lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
	// point light 3
	lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[2].constant", 1.0f);
	lightingShader.setFloat("pointLights[2].linear", 0.09f);
	lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
	// point light 4
	lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[3].constant", 1.0f);
	lightingShader.setFloat("pointLights[3].linear", 0.09f);
	lightingShader.setFloat("pointLights[3].quadratic", 0.032f);

	//***********************************************************************************************

	//temporary shader for this model
	Shader outlineShader(R"(shaders/lightingShader.vert)", R"(shaders/temp.frag)");
	//loading models
	Model backpack("models/backpack/backpack.obj");

	//***********************************************************************************************
	//FFT ocean + analytic sky
	Sky sky;
	Ocean ocean;
	if (oceanMode) camera.Position = glm::vec3(0.0f, 18.0f, 40.0f);

	//***********************************************************************************************

	//unbind all the stuff
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glLinkProgram(0);

	//render loop
	glViewport(0, 0, 800, 600);
	while (!glfwWindowShouldClose(window)) {

		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		processInput(window);

		//clear color buffer and depth buffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		//model
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 1500.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

		if (oceanMode) {
			ocean.update(currentTime);
			sky.draw(view, projection);
			ocean.draw(view, projection, camera.Position, sky.sunDir, sky.turbidity, sky.exposure);
		}
		else {

		lightingShader.use();
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setMat4("view", view);
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("model", model);
		
		// spotLight
		lightingShader.setVec3("spotLight.position", camera.Position);
		lightingShader.setVec3("spotLight.direction", camera.Front);
		lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("spotLight.constant", 1.0f);
		lightingShader.setFloat("spotLight.linear", 0.09f);
		lightingShader.setFloat("spotLight.quadratic", 0.032f);
		lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

		//backpack.Draw(lightingShader);

		//bad outline function testing
		//outlineModel(backpack, lightingShader, glm::vec3(5.0f));

		//drawing random
		tempGrassShader.use();
		glBindVertexArray(grassVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTex.ID);
		for (unsigned int i = 0; i < grassPositions.size(); i++) {
		
			model = glm::mat4(1.0f);
			model = glm::translate(model, grassPositions[i]);
			tempGrassShader.setMat4("model", model);
			tempGrassShader.setVec3("viewPos", camera.Position);
			tempGrassShader.setMat4("view", view);
			tempGrassShader.setMat4("projection", projection);
			glDrawArrays(GL_TRIANGLES, 0, 6);

		}

		} // end !oceanMode

		//unbind stuff (keep code below this out of any loops)
		glBindVertexArray(0);
		//swap
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(lightingShader.ID);

	glfwTerminate();
	return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		oceanMode = !oceanMode;
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

	camera.ProcessMouseMovement(xoffset, yoffset, deltaTime);
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

//EWWW function
void outlineModel(Model model, Shader defShader, glm::vec3 position) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS,1,0xFF);
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

}