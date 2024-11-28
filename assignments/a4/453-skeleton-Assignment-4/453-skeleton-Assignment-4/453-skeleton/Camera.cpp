#include "Camera.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(float t, float p, float r, glm::vec3& lookAt) :
	theta(t),
	phi(p),
	radius(r),
	lookAt(lookAt)
{
}

glm::mat4 Camera::getView()
{
	glm::vec3 eye = radius * glm::vec3(std::cos(theta) * std::sin(phi), std::sin(theta), std::cos(theta) * std::cos(phi));
	//glm::vec3 at = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	return glm::lookAt(eye, lookAt, up);
}

glm::vec3 Camera::getPos() {
	return radius * glm::vec3(std::cos(theta) * std::sin(phi), std::sin(theta), std::cos(theta) * std::cos(phi));
}

void Camera::incrementTheta(float dt) {
	if (theta + (dt / 100.0f) < M_PI_2 && theta + (dt / 100.0f) > -M_PI_2) {
		theta += dt / 100.0f;
	}
}

void Camera::incrementPhi(float dp) {
	phi -= dp / 100.0f;
	if (phi > 2.0 * M_PI) {
		phi -= 2.0 * M_PI;
	} else if (phi < 0.0f) {
		phi += 2.0 * M_PI;
	}
}

void Camera::incrementR(float dr)
{
	radius -= dr;
	if (radius <= 0.1)	// Don't allow the camera to invert
		radius = 0.1;
}

void Camera::resetPhi()
{
	phi = glm::radians(45.f);
}

void Camera::resetTheta()
{
	theta = glm::radians(45.f);
}

void Camera::resetRadius()
{
	radius = 3.f;
}

void Camera::setRadius(float r)
{
	radius = r;
}

float Camera::getPhi()
{
	return phi;
}

float Camera::getTheta()
{
	return theta;
}

float Camera::getRadius()
{
	return radius;
}
