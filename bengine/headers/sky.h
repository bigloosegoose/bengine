#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

// Renders an analytic Preetham sky dome and holds the shared sun/atmosphere
// parameters used both here and by the ocean surface for reflections.
class Sky {
public:
	Shader     shader;
	glm::vec3  sunDir;
	float      turbidity;
	float      exposure;

	Sky()
		: shader(R"(shaders/sky/sky.vert)", R"(shaders/sky/sky.frag)"),
		  sunDir(glm::normalize(glm::vec3(0.4f, 0.35f, 0.3f))),
		  turbidity(2.6f),
		  exposure(1.0f)
	{
		float cube[] = {
			-1, -1, -1,  1, -1, -1,  1,  1, -1,  1,  1, -1, -1,  1, -1, -1, -1, -1,
			-1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1, -1,  1,
			-1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1,  1,
			 1,  1,  1,  1,  1, -1,  1, -1, -1,  1, -1, -1,  1, -1,  1,  1,  1,  1,
			-1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1,  1, -1, -1, -1,
			-1,  1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1,  1, -1
		};
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	void draw(const glm::mat4& view, const glm::mat4& projection) {
		glDepthFunc(GL_LEQUAL);
		shader.use();
		glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view)); // strip translation
		shader.setMat4("view", viewNoTrans);
		shader.setMat4("projection", projection);
		shader.setVec3("sunDir", sunDir);
		shader.setFloat("turbidity", turbidity);
		shader.setFloat("exposure", exposure);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
	}

private:
	unsigned int VAO, VBO;
};
