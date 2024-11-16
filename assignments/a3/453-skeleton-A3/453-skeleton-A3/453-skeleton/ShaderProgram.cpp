#include "ShaderProgram.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include "Log.h"

ShaderProgram::ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
	: programID()
	, vertex(vertexPath, GL_VERTEX_SHADER)
	, fragment(fragmentPath, GL_FRAGMENT_SHADER)
{
	attach(*this, vertex);
	attach(*this, fragment);
	glLinkProgram(programID);

	if (!checkAndLogLinkSuccess()) {
		glDeleteProgram(programID);
		throw std::runtime_error("Shaders did not link.");
	}
}

bool ShaderProgram::recompile() {

	try {
		// Try to create a new program
		ShaderProgram newProgram(vertex.getPath(), fragment.getPath());
		*this = std::move(newProgram);
		return true;
	}
	catch (std::runtime_error &e) {
		Log::warn("SHADER_PROGRAM falling back to previous version of shaders");
		return false;
	}
}

// Function to set a mat4 uniform
void ShaderProgram::setMat4Transform(const std::string& name, const glm::mat4& matrix) const
{
	// Get the uniform location
	GLint location = glGetUniformLocation(programID, name.c_str());
	if (location == -1) {
		std::cerr << "Warning: uniform '" << name << "' doesn't exist or isn't used in the shader program." << std::endl;
	}
	// Send the matrix to the GPU
	glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
	// GL_FALSE means the matrix is not transposed
}

void attach(ShaderProgram& sp, Shader& s) {
	glAttachShader(sp.programID, s.shaderID);
}


bool ShaderProgram::checkAndLogLinkSuccess() const {

	GLint success;

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		GLint logLength;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength);
		glGetProgramInfoLog(programID, logLength, NULL, log.data());

		Log::error("SHADER_PROGRAM linking {} + {}:\n{}",
			  vertex.getPath()
			, fragment.getPath()
			, log.data()
		);
		return false;
	}
	else {
		Log::info("SHADER_PROGRAM successfully compiled and linked {} + {}",
			  vertex.getPath()
			, fragment.getPath()
		);
		return true;
	}
}
