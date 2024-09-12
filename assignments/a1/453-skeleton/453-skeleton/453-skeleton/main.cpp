
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"


// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

private:
	ShaderProgram& shader;
};

class MyCallbacks2 : public CallbackInterface {

public:
	MyCallbacks2() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			std::cout << "called back" << std::endl;
		}
	}
};
// END EXAMPLES

void squarePatternTest(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);

int main()
{
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	/* DEMO (from og file)
	// vertices
	cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	// colours (these should be in linear space)
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
	*/

	squarePatternTest(cpuGeom, gpuGeom);
	std::cout << "Number of verticies inside cpuGeom = " << cpuGeom.verts.size() << std::endl;

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_LINE_LOOP, 0, 3);	

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}

void squarePatternTest(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	// vertices
	cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));	// (1.f, 1.f, 1.f) will be the top right corner of the window
	cpuGeom.verts.push_back(glm::vec3(-0.5f, 0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, 0.5f, 0.f));

	// colours (these should be in linear space)
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));	// if i used all 1's the triangle will be white,
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));	// all 0's for black
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}
