#pragma once

#include "Shader.h"

#include "GLHandles.h"

//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr

//using namespace std;
//using namespace glm;

class ShaderProgram {

public:
	ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

	// Because we're using the ShaderProgramHandle to do RAII for the shader for us
	// and our other types are trivial or provide their own RAII
	// we don't have to provide any specialized functions here. Rule of zero
	//
	// https://en.cppreference.com/w/cpp/language/rule_of_three
	// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rc-zero

	// Public interface
	bool recompile();
	void use() const { glUseProgram(programID); }

	void friend attach(ShaderProgram& sp, Shader& s);

	//void setModelMat4(const string& name, const mat4& matrix) const;
	//void setViewMat4(const string& name, const mat4& matrix) const;
	//void setProjectionMat4(const string& name, const mat4& matrix) const;

	operator GLuint() const {
		return programID;
	}

private:
	ShaderProgramHandle programID;

	Shader vertex;
	Shader fragment;

	bool checkAndLogLinkSuccess() const;
};
