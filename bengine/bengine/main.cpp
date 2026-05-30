#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"

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

		GLint activeVaoId = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &activeVaoId);
		if (activeVaoId == 1) 
		{
			glBindVertexArray(2);
		}
		else 
		{
			glBindVertexArray(1);
		}
	}
}



 // temp c string for our shaders ewww
const char* vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"ourColor = aColor;\n"
"}\n"; 

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(ourColor.x, ourColor.y, ourColor.z , 1.0);\n"
"}\n";

const char* tempFragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

//vertex input bruh
float vertices[] = {

	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f
};
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};

float vertices2[] = {
	// positions		//color
	-1.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
	-1.0f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	 1.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,// second 
	 1.0f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,

};



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

	//how to interpret vertex buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); //attribs are disabled by default

	//TRIANGLE
	unsigned int VAO2, VBO2;
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	//position Attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//color Attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//temp fragment shader
	unsigned int secondaryFragmentShader;
	secondaryFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(secondaryFragmentShader, 1, &tempFragmentShaderSource, NULL);
	glCompileShader(secondaryFragmentShader);

	//shader program object
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//temporary shader program
	unsigned int tempShaderProgram;
	tempShaderProgram = glCreateProgram();
	glAttachShader(tempShaderProgram, vertexShader);
	glAttachShader(tempShaderProgram, secondaryFragmentShader);
	glLinkProgram(tempShaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//shader and linking compilation check
	int success;
	char infolog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
		std::cout << "ERROR::PROGRAM::SHADERPROGRAM::FAILED\n" << infolog << std::endl;
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0); //registered alr by attrib pointer
	glBindVertexArray(0);

	//tryna create shader with our new class
	Shader ourShader(R"(C:\Users\thebi\source\repos\bengine\bengine\firstShader.vert)", R"(C:\Users\thebi\source\repos\bengine\bengine\firstShader.frag)");

	//render loop
	glViewport(0, 0, 800, 600);
	glBindVertexArray(2);
	while (!glfwWindowShouldClose(window)) {

		processInput(window);

		//clear color buffer
		glClearColor(0.2f, 0.3f, 0.3f, 0.4f);
		glClear(GL_COLOR_BUFFER_BIT);

		GLint activeVaoId = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &activeVaoId);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
		
		if (activeVaoId == 2) {
			//the fancy chameleon stuff
			float timeValue = glfwGetTime();
			float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
			int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
			//glUseProgram(shaderProgram);
			ourShader.use();
			glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);\

			//glUseProgram(shaderProgram);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		if (activeVaoId == 1) {
			glUseProgram(tempShaderProgram);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0);



		//swap
		glfwSwapBuffers(window);
		glfwPollEvents();
		

	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}


