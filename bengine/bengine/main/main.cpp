#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include "stb_image.h"
#include "my_texture.h"

using std::cout, std::endl;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

}
//process input but in callback method
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) 
	{
		/*
		Shader* ourShader = (Shader*)glfwGetWindowUserPointer(window);

		if (ourShader) {
			ourShader->use();
			glUniform1f(glGetUniformLocation(ourShader->ID, "opacity"), 1.0f);
		}
		*/
		
		/*GLint activeVaoId = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &activeVaoId);
		if (activeVaoId == 1) 
		{
			glBindVertexArray(2);
		}
		else 
		{
			glBindVertexArray(1);
		}*/
	}
}


//vertex input bruh
float vertices[] = {
	//positions			//color			  //tex coords
	-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
	 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 
	 0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
	-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f 
};
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};

/*float vertices2[] = {
	// positions		//color
	-1.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
	-1.0f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	 1.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,// second 
	 1.0f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,

};*/



int main () {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "SCWEEN", NULL, NULL);
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

	//RECTANGLE VAO-VBO(initialize VAO FIRST)
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
	//color Attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	//shader with our new class
	Shader ourShader(R"(shaders\firstShader.vert)", R"(shaders\firstShader.frag)");
	ourShader.glfwSetPointer(window); //just because

	//lovely 2 textures yay
	Texture texture1(GL_TEXTURE_2D, "textures/container.jpg", false, 0);
	texture1.wrapMode(GL_REPEAT, GL_REPEAT, GL_REPEAT);
	texture1.filteringMode(GL_LINEAR, GL_LINEAR);

	Texture texture2(GL_TEXTURE_2D,"textures/awesomeface.png",true, 1);
	texture2.wrapMode(GL_REPEAT, GL_REPEAT, GL_REPEAT);
	texture2.filteringMode(GL_LINEAR,GL_LINEAR);


	//unbind all the stuff
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);


	//render loop
	glViewport(0, 0, 800, 600);
	while (!glfwWindowShouldClose(window)) {

		processInput(window);

		//clear color buffer
		glClearColor(0.2f, 0.3f, 0.3f, 0.4f);
		glClear(GL_COLOR_BUFFER_BIT);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
		glBindVertexArray(VAO);
		ourShader.use();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//swap
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(ourShader.ID);

	glfwTerminate();
	return 0;
}


