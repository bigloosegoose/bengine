#pragma once

#include <glad/glad.h> //GET THOSE OPENGL HEADERS (4.3+ for compute)

#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


// a minimal compute shader wrapper following the same style as Shader.
// loads a single .comp file, compiles GL_COMPUTE_SHADER, links a program.
class ComputeShader
{
public:
	unsigned int ID;

	ComputeShader(const char* computePath) {
		std::string computeCode;
		std::ifstream cShaderFile;
		cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			cShaderFile.open(computePath);
			std::stringstream cShaderStream;
			cShaderStream << cShaderFile.rdbuf();
			cShaderFile.close();
			computeCode = cShaderStream.str();
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::COMPUTE_SHADER::FILE_NOT_SUCCESSFULLY_READ (" << computePath << ")" << std::endl;
		}
		const char* cShaderCode = computeCode.c_str();

		int success;
		char infoLog[1024];

		unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &cShaderCode, NULL);
		glCompileShader(compute);
		glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(compute, 1024, NULL, infoLog);
			std::cout << "ERROR::COMPUTE_SHADER::COMPILATION_FAILED (" << computePath << ")\n" << infoLog << std::endl;
		}

		ID = glCreateProgram();
		glAttachShader(ID, compute);
		glLinkProgram(ID);
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 1024, NULL, infoLog);
			std::cout << "ERROR::COMPUTE_SHADER::PROGRAM::LINKING_FAILED (" << computePath << ")\n" << infoLog << std::endl;
		}

		glDeleteShader(compute);
	}

	void use() const {
		glUseProgram(ID);
	}

	// dispatch and wait for image/buffer writes to be visible to subsequent passes
	void dispatch(unsigned int x, unsigned int y, unsigned int z) const {
		glDispatchCompute(x, y, z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setUint(const std::string& name, unsigned int value) const {
		glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setVec2(const std::string& name, float x, float y) const {
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void setVec2(const std::string& name, glm::vec2 v) const {
		glUniform2f(glGetUniformLocation(ID, name.c_str()), v.x, v.y);
	}
	void setVec3(const std::string& name, glm::vec3 v) const {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
	}
};
