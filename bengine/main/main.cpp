#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "stb_image.h"
#include "my_texture.h"
#include "camera.h"

using std::cout, std::endl; 

//settings
const unsigned int width = 800;
const unsigned int height = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD,deltaTime);
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}



//vertex input bruh
float vertices2[] = {
	//positions          //tex coords
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 
	 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 
	 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 
	-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f, 0.0f, 1.0f
};
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0,
	0, 1, 5,
	5, 4, 0,
	0, 3, 7,
	7, 4, 0,
	6, 5, 1,
	1, 2, 6,
	6, 7, 3,
	3, 2, 6,
	6, 7, 4,
	4, 5, 6
};

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

glm::vec3 cubePositions[] = {
	glm::vec3(0.0f, 0.0f,   0.0f),
	glm::vec3(2.0f, 5.0f, -15.0f),
	glm::vec3(1.0f, 4.0f,  -2.0f),
	glm::vec3(8.0f, 3.0f, -12.0f),
	glm::vec3(2.0f, 9.0f,  -3.5f)
};

//****************************************************************************************************************************************
int main () {

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

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glEnable(GL_DEPTH_TEST);


	camera.SetCameraMode(FLY);

	//cube VAO, VBO, EBO?
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
	//The light source ;-; (clean the code further)

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO); //we not making lightVBO cuz we using the one from the container
	
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);

	//VertexPos Attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//***********************************************************************************************
	Shader lightingShader(R"(shaders\lightingShader.vert)", R"(shaders\lightingShader.frag)");

	lightingShader.use();
	
	//setting the material and light (temp?)
	//lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);
	lightingShader.setFloat("material.shininess", 32.0f);

	lightingShader.setVec3("light.ambient", 0.2f);
	lightingShader.setVec3("light.diffuse", 0.5f);
	lightingShader.setVec3("light.specular", 1.0f);
	//***********************************************************************************************
	Texture diffuseMap(GL_TEXTURE_2D, "textures/container2.png", true, 0);
	diffuseMap.wrapMode(GL_REPEAT, GL_REPEAT, GL_REPEAT);
	diffuseMap.filteringMode(GL_LINEAR, GL_LINEAR);

	Texture specularMap(GL_TEXTURE_2D, "textures/container2_specular.png", true, 1);
	specularMap.wrapMode(GL_REPEAT, GL_REPEAT, GL_REPEAT);
	specularMap.filteringMode(GL_LINEAR, GL_LINEAR);

	Shader lightSourceShader(R"(shaders\lightSourceShader.vert)", R"(shaders\lightSourceShader.frag)");
	//***********************************************************************************************




	//unbind all the stuff
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//render loop
	glViewport(0, 0, 800, 600);
	while (!glfwWindowShouldClose(window)) {

		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;



		processInput(window);

		//clear color buffer and depth buffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

		//the container
		//view matrix(calculate from camPos,camTarget,worldUp)
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(1.0f, 0.3f, 0.5f));

		lightPos = glm::vec3(sin((float) glfwGetTime()), cos((float) glfwGetTime()), 0.0f);

		lightingShader.use();
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setMat4("view", view);
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		//the light
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));

		lightSourceShader.use();
		lightSourceShader.setMat4("view", view);
		lightSourceShader.setMat4("projection", projection);
		lightSourceShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


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


