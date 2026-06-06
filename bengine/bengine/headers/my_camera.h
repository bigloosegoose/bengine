#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

float pitch = 0.0f, yaw = -90.0f;
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;

void cursor_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}


	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; //reversed, y goes bottom to top
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity; //step 2
	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) { //step 3
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	glm::vec3 direction; //step4
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	cameraFront = glm::normalize(direction);


}


class Camera {
public:
	//as always, the camera ID
	unsigned int ID;


	Camera(GLFWwindow* window) {
		glfwSetCursorPosCallback(window, cursor_callback);
	}

};