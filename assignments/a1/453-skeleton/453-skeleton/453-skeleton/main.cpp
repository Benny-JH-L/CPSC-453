
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
void sierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations);
void setRainbowCol(CPU_Geometry& cpuGeom);
void printVectorLocation(glm::vec3 vec, int vecNum);
void printVectorLocation(glm::vec3 vec);

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

	// DEMO (from og file)
	// vertices
	//cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	//cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	//cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));

	//// colours (these should be in linear space)
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));

	//gpuGeom.setVerts(cpuGeom.verts);
	//gpuGeom.setCols(cpuGeom.cols);

	/* my tests
	// vertices
	//cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));	// (1.f, 1.f, 1.f) will be the top right corner of the window
	//cpuGeom.verts.push_back(glm::vec3(-0.5f, 0.5f, 0.f));
	//cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	//cpuGeom.verts.push_back(glm::vec3(0.5f, 0.5f, 0.f));

	//// colours (these should be in linear space)
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));	// if i used all 1's the triangle will be white,
	//cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));	// all 0's for black
	//cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
	//cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	*/


	//squarePatternTest(cpuGeom, gpuGeom);
	sierpinskiTriangle(cpuGeom, gpuGeom, 2);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, cpuGeom.verts.size());

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}

/**
* Calculates the halfway x-coord. of two vectors
* @return float, the halfway x-coord. of two vectors.
*/
float calcHalfWayX(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.x + v2.x) / 2;
}

/**
* Calculates the halfway y-coord. of two vectors
* @return float, the halfway y-coord. of two vectors.
*/
float calcHalfWayY(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.y + v2.y) / 2;
}

void sierpinskiRecurr(CPU_Geometry& cpuGeom, int numIterations)
{
	glm::vec3 bottomLeftVec = cpuGeom.verts.at(0);
	glm::vec3 topMidVec = cpuGeom.verts.at(1);
	glm::vec3 bottomRightVec = cpuGeom.verts.at(2);

	printVectorLocation(bottomLeftVec, 0);
	printVectorLocation(topMidVec, 1);
	printVectorLocation(bottomRightVec, 2);

	glm::vec3 vects[3] = {bottomLeftVec, topMidVec, bottomRightVec};

	// calculating x and y halfway points of vectors
	float halfwayX[3] = {};	// stores all the halfway x-coordinates
	float halfwayY[3] = {};	// stores all the halfway y-coordinates

	// half way points for vectors 0 and 1
	halfwayX[0] = calcHalfWayX(vects[0], vects[1]);
	halfwayY[0] = calcHalfWayY(vects[0], vects[1]);
	// half way points for vectors 1 and 2
	halfwayX[1] = calcHalfWayX(vects[1], vects[2]);
	halfwayY[1] = calcHalfWayY(vects[1], vects[2]);
	// half way points for vectors 0 and 2
	halfwayX[2] = calcHalfWayX(vects[0], vects[2]);
	halfwayY[2] = calcHalfWayY(vects[0], vects[2]);

	// debug
	for (int i = 0; i < 3; i++)
	{
		std::cout	<< "(" << halfwayX[i] << ", "
					<< halfwayY[i] << ")" << std::endl;
	}

	for (int i = 0; i < 3; i++)
		cpuGeom.verts.push_back(glm::vec3(halfwayX[i], halfwayY[i], 0.f));

}

void sierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations)
{
	if (numIterations < 0)
		return;
	// Create base triangle
	cpuGeom.verts.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f, -0.5f, 0.f));
	setRainbowCol(cpuGeom);

	 //Test decolor (works)
	//cpuGeom.verts.push_back(glm::vec3(-0.25f, -0.25f, 0.f));
	//cpuGeom.verts.push_back(glm::vec3(0.f, 0.25f, 0.f));
	//cpuGeom.verts.push_back(glm::vec3(0.25f, -0.25f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 0.f));
	//cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 0.f));

	sierpinskiRecurr(cpuGeom, numIterations);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

/**
* Sets the colours for vertices inside the cpuGeom, rainbow.
* @param CPU_Geometry& cpuGeom, the address of cpuGeom that contains the vertices.
*/
void setRainbowCol(CPU_Geometry& cpuGeom)
{
	for (int i = 0, j = 0; i < cpuGeom.verts.size(); i++, j++)
	{
		if (j == 3)
			j = 0;

		switch (j)
		{
		case 0:
			cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
			break;
		case 1:
			cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
			break;
		case 2:
			cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
			break;
		}
	}
}


void printVectorLocation(glm::vec3 vec)
{
	printVectorLocation(vec, -1);
}

void printVectorLocation(glm::vec3 vec, int vecNum)
{
	std::cout << "vec #" << vecNum << " @ (" << vec.x << ", " << vec.y << ")" << std::endl;
}

//void squarePatternTest(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
//{
//	//int numVerts = 10;
//	float xOffset = 0;
//	float yOffset = 0;
//	bool posDirection = true;
//	bool xAxis = true;	// true = x-axis, false = y-axis
//
//	for (int i = 1; i <= numVerts; i++)
//	{
//		if (xAxis)
//		{
//			if (xOffset >= 0 && i != 1)
//				xOffset += 0.05 * i;
//			else if (xOffset < 0)
//				xOffset -= 0.05 * i;
//
//			if (posDirection)
//			{
//				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
//				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
//			}
//			else
//			{
//				xOffset = -xOffset;
//				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
//				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
//				yOffset = -yOffset;
//			}
//		}
//		else
//		{
//			if (xOffset == 0)
//				yOffset = 0.05;
//
//			yOffset += xOffset;
//
//			if (posDirection)
//			{
//				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
//				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
//			}
//			else
//			{
//				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
//				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
//				xOffset = -xOffset;
//			}
//			posDirection = !posDirection;
//		}
//
//		xAxis = !xAxis;
//	}
//
//	std::cout << "Number of vertices inside cpuGeom = " << cpuGeom.verts.size() << std::endl;
//
//	for (int i = 0, j = 0; i < numVerts; i++, j++)
//	{
//		switch (j)
//		{
//		case 0:
//			cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
//			break;
//		case 1:
//			cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
//			break;
//		case 2:
//			cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
//			break;
//		}
//
//		if (j == 2)
//			j = 0;
//	}
//
//	// equivalent code:
//	//int count = 0, count2 = 0;;
//	//while (count2 < numVerts)
//	//{
//	//	if (count == 0)
//	//		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
//	//	else if (count == 1)
//	//		cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
//	//	else
//	//		cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
//
//	//	if (count == 2)
//	//		count = 0;
//	//	count++;
//	//	count2++;
//	//}
//
//	gpuGeom.setVerts(cpuGeom.verts);
//	gpuGeom.setCols(cpuGeom.cols);
//}



