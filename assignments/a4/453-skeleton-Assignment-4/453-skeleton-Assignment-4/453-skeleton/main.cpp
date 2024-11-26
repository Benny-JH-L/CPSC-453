//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitCube.h"

#include <tuple>

using namespace std;
using namespace glm;

const static int NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION = 20;
const static int NUMBER_OF_ITERATIONS_FOR_CHAIKIN = 4;

class Planet
{
public:
	Planet(float radius, string texturePath) :
		radius(radius),
		center(vec3(0.f, 0.f, 0.f)),
		texture(texturePath, GL_NEAREST)
	{
		generateGeometry();
	}

	float radius;
	vec3 center;
	Texture texture;

	CPU_Geometry cpu_geom;
	GPU_Geometry gpu_geom;

private:

	// Control points used to generate spherical geometry
	const vector<vec3> sphereControlPoints = 
	{
		{0.f, -.6f * radius, 0.f},
		{ .6f * radius, -.5f * radius, 0.f},
		{ .6f * radius,  .5f * radius, 0.f},
		{0.f, .6f * radius, 0.f}
	};

	void generateGeometry()
	{
		vector<vec3> chaikinCurvePts = sphereControlPoints;
		for (int i = 0; i < NUMBER_OF_ITERATIONS_FOR_CHAIKIN; i++)
			chaikinCurvePts = chaikinOpen(chaikinCurvePts);

		auto [meshPoints, meshTriangles] = surfaceOfRevolution(chaikinCurvePts, NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION);
		cpu_geom.verts = meshTriangles;
		cpu_geom.cols = vector<vec3>(meshTriangles.size(), vec3(vec2(0.f), 1.0));
		gpu_geom.setVerts(cpu_geom.verts);
		gpu_geom.setCols(cpu_geom.cols);
	}

	/// <summary>
	/// Generates an open polynomial B-Spline curve.
	/// </summary>
	/// <param name="coursePts"> set of control points, std::vector[glm::vec3].</param>
	/// <returns> a std::vector(glm::vec3) that contains the open polynomial B-spine curve.</returns>
	static vector<vec3> chaikinOpen(const vector<vec3> coursePts)
	{
		int numCoursePts = coursePts.size();
		vector<vec3> finePoints;

		// Special mask for the beginning
		finePoints.push_back(coursePts[0]);
		finePoints.push_back((0.5f * coursePts[0]) + (0.5f * coursePts[1]));

		// Interior points
		for (int i = 1; i < numCoursePts - 2; i++)
		{
			finePoints.push_back((0.75f * coursePts[i]) + (0.25f * coursePts[i + 1]));
			finePoints.push_back((0.25f * coursePts[i]) + (0.75f * coursePts[i + 1]));
		}

		// Special mask for the end
		finePoints.push_back((0.5f * coursePts[numCoursePts - 2]) + (0.5f * coursePts[numCoursePts - 1]));
		finePoints.push_back(coursePts[numCoursePts - 1]);

		return finePoints;
	}

	/// <summary>
	/// Takes in a B-Spline curve and the desired number of "slices" (larger slice value = smoother surface).
	/// Returns a tuple containing the mesh point positions, and triangles that make up the mesh.
	/// 1st element: mesh point positions, std::vector(vec3).
	/// 2nd element: triangle mesh, std::vector(vec3).
	/// </summary>
	/// <param name="bSplineCurve"> the b-spline curve to make the surface from.</param>
	/// <param name="numSlices"> how many 'slices' wanted when generating the surface.</param>
	/// <returns></returns>
	static tuple<vector<vec3>, vector<vec3>> surfaceOfRevolution(vector<vec3> bSplineCurve, int numSlices)
	{
		vector<vec3> meshPoints;
		vector<vec3> meshTriangles;

		float angleStep = 2.0f * glm::pi<float>() / numSlices;
		vector<vector<vec3>> temp2D;	// each nested vector contains a b-spline curve point rotated 'numSlices' times

		// Create mesh points
		for (int i = 0; i < bSplineCurve.size(); i++)
		{
			vector<vec3> temp;
			// Create mesh point by rotating the b-spline curve point
			for (int j = 0; j <= numSlices; j++)
			{
				float angle = j * angleStep;

				// rotate about x-axis
				//float x = bSplineCurve[i].x;
				//float y = bSplineCurve[i].y * cos(angle) - bSplineCurve[i].z * sin(angle);
				//float z = bSplineCurve[i].y * sin(angle) + bSplineCurve[i].z * cos(angle);

				// rotate about y-axis
				float x = bSplineCurve[i].x * cos(angle);
				float z = bSplineCurve[i].x * sin(angle);
				float y = bSplineCurve[i].y;

				// rotate about z-axis
				//float x = bSplineCurve[i].x * cos(angle) - bSplineCurve[i].y * sin(angle);
				//float y = bSplineCurve[i].x * sin(angle) + bSplineCurve[i].y * cos(angle);
				//float z = bSplineCurve[i].z;

				meshPoints.push_back(vec3(x, y, z));
				temp.push_back(vec3(x, y, z));
			}
			temp2D.push_back(temp);
		}

		// Create mesh triangles
		for (int i = 0; i < temp2D.size() - 1; i++)		// minus 1, as we can't draw a triangle with only the last set of points left
		{
			vector<vec3> currentSetOfPoints = temp2D[i];
			vector<vec3> nextSetOfPoints = temp2D[i + 1];

			for (int j = 0; j < currentSetOfPoints.size() - 1; j++)	// minus 1, as we can't draw a triangle with other points left
			{
				// First triangle
				meshTriangles.push_back(currentSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j + 1]);

				// Second triangle
				meshTriangles.push_back(currentSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j + 1]);
				meshTriangles.push_back(currentSetOfPoints[j + 1]);
			}
			// Second triangle
			//for (int j = 0; j < currentSetOfPoints.size() - 1; j++)
			//{
			//	meshTriangles.push_back(currentSetOfPoints[j]);
			//	meshTriangles.push_back(nextSetOfPoints[j + 1]);
			//	meshTriangles.push_back(currentSetOfPoints[j + 1]);

			//}
			// result is a square/rectangle defined by these two triangles (triangles are drawn counter clock wise)
		}

		return make_tuple(meshPoints, meshTriangles);
	};
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface
{

public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {}

	virtual void mouseButtonCallback(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos)
	{
		if (rightMouseDown)
		{
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}

	virtual void scrollCallback(double xoffset, double yoffset)
	{
		Log::info("yoffset = {}", yoffset);
		camera.incrementR(yoffset);
	}

	virtual void windowSizeCallback(int width, int height)
	{
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp)
	{
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		//GLint location = glGetUniformLocation(sp, "lightPosition");
		//glm::vec3 light = camera.getPos();
		//glUniform3fv(location, 1, glm::value_ptr(light));
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;
private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 3");

	GLDebug::enable();

	// CALLBACKS
	auto callBack = std::make_shared<Assignment4>();
	window.setCallbacks(callBack);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	UnitCube cube;
	cube.generateGeometry();

	Planet earth = Planet(2, "textures/ship.png");

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL /*GL_LINE*/);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE /*GL_LINE*/);


		shader.use();

		callBack->viewPipeline(shader);

		cube.m_gpu_geom.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(cube.m_size));

		earth.gpu_geom.bind();
		//earth.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(earth.cpu_geom.verts.size()));
		//earth.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
