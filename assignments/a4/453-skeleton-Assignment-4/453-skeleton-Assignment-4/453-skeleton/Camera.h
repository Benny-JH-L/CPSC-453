#pragma once

//------------------------------------------------------------------------------
// This file contains an implementation of a spherical camera
//------------------------------------------------------------------------------

//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:

	Camera(float t, float p, float r, glm::vec3& lookAt);

	glm::mat4 getView();
	glm::vec3 getPos();
	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);

	void resetTheta();
	void resetPhi();
	void resetRadius();
	void setRadius(float r);
	glm::vec3& lookAt;

	float getTheta();
	float getPhi();
	float getRadius();

private:

	float theta;
	float phi;
	float radius;
};
