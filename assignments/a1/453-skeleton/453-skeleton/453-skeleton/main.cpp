/*
* CPSC 453 | Assignment 1 Points, Lines and Triangles
* UCID: 30192142
* NAME: BENNY LIANG
*/

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


void squarePatternTest(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numVerts);	// test, delete after.

void sierpinskiOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);
void generateSierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations);
void setRainbowCol(CPU_Geometry& cpuGeom);
float calcHalfWayX(const glm::vec3& v1, const glm::vec3& v2);
float calcHalfWayY(const glm::vec3& v1, const glm::vec3& v2);

void pythagorasOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);
void generatePythagorasTree(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);

void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);

void dragonCurveOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);

void drawFractal(int option, const CPU_Geometry& cpuGeom);

// Debugging prototypes
void printVectorLocation(glm::vec3 vec, int vecNum);
void printVectorLocation(glm::vec3 vec);


int numSubDiv = -1;

int main()
{
	Log::debug("Starting main");

	bool exit = false;
	int option = -1; 	// used to determine what fractal the user wants to generate (1: triangle, 2: pythagoras, 3: snowflake, 4: dragon curve, 0: exit) 

	// Keep looping until the user wants to exit
	while (!exit)
	{
		// Option menu
		std::cout
			<< "\nWhat would you like to generate?\n"
			<< "1: Sierpinski's Triangle\n"
			<< "2: Pythagoras Tree\n"
			<< "3: Koch Snowflake\n"
			<< "4: Dragon Curve\n"
			<< "0: EXIT\n"
			<< std::endl;

		std::cin >> option;

		bool invalidMenuInput = false;
		// Checking if the user entered a valid menu input
		if (std::cin.fail())				// non-int entered
		{
			std::cin.clear();				// clear fail flag
			invalidMenuInput = true;
		}
		else if (option < 0 || option > 4)	// out of menu range
			invalidMenuInput = true;

		if (invalidMenuInput)
		{
			// If the user entered a non-int or
			// the user typed an int but then a non-int after -> invalid input.
			std::cout << "\Invalid option..." << std::endl;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		
			option = -1;
			continue;			// iterate loop from start
		}

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

		numSubDiv = -1;

		// Checking the option chosen
		switch(option)
		{
			case 0:
				exit = !exit;
				std::cout << "\nExiting..." << std::endl;
				continue;			// iterate loop from start
			case 1:
				sierpinskiOption(cpuGeom, gpuGeom);
				break;
			case 2:
				pythagorasOption(cpuGeom, gpuGeom);
				break;
			case 3:
				snowflakeOption(cpuGeom, gpuGeom);
				break;
			case 4:
				dragonCurveOption(cpuGeom, gpuGeom);
				break;
			default:
				std::cout << "\Invalid option..." << std::endl;
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// ignore the rest of the user's input
				option = -1;
				continue;			// iterate loop from start
		}

		std::cout << "\nPlease close the shape/fractal generation window to go back to selection menu.\n" << std::endl;

		// RENDER LOOP
		while (!window.shouldClose()) {
			glfwPollEvents();

			shader.use();
			gpuGeom.bind();
			
			glEnable(GL_FRAMEBUFFER_SRGB);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			drawFractal(option, cpuGeom);	// Drawing
			window.swapBuffers();
		}

		glfwTerminate();	// close the shape/generated fractal window
	}

	return 0;

}

/**
* Draws the fractal the user chose.
* @param int option, the fractal option generated.
* @param const CPU_Geometry& cpuGeom, the CPU_Geometry reference that contains the 'vec3's of the fractal shape.
*/
void drawFractal(int option, const CPU_Geometry& cpuGeom)
{
	switch (option)
	{
		case 1:
			glDrawArrays(GL_TRIANGLES, 0, cpuGeom.verts.size());
			break;
		case 2:
			// "squareVec3Start" is the starting 'vec3' in 'cpuGeom' for this square.
			// 'j' is how many 'vec3's to draw (4) for a square.
			for (int squareVec3Start = 0, j = 4; squareVec3Start < cpuGeom.verts.size(); squareVec3Start += 4)
				glDrawArrays(GL_TRIANGLE_FAN, squareVec3Start, j);
			break;
		case 3:
			// use "GL_TRIANGLE_STRIP" ???
			std::cout << "\nSnowflake drawing not implemented...\n" << std::endl;
			break;
		case 4:
			std::cout << "\nDragon curve drawing not implemented...\n" << std::endl;
			break;
		default:
			std::cout << "\nError drawing...\n" << std::endl;
			break;
	}
}

/**
* Asks the user how many subdivisions and then generates said number of subdivisions.
* @param CPU_Geometry& cpuGeom, reference to the CPU_Geometry to contain the 'vec3's generated.
* @param GPU_Geometry& gpuGeom, reference to the GPU_Geometry to set its 'vec3's and colours.
*/
void sierpinskiOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	while (numSubDiv < 0)
	{
		std::cout << "\n--Sierpinski Triangle--\n How many subdivisions would you like? ";
		std::cin >> numSubDiv;
		if (numSubDiv < 0)
		{
			std::cout << "\nInvalid input..." << std::endl;
			continue;
		}
		generateSierpinskiTriangle(cpuGeom, gpuGeom, numSubDiv);
		std::cout << "\nSierpinski Triangle with " << numSubDiv << " subdivisions created." << std::endl;
	}
}

/**
* Recursively calls itself to find Sierpinski Triangle's at sub-divisions/iterations.
* @param CPU_Geometry& cpuGeom, a reference to a CPU_Geometry.
* @param std::vector<glm::vec3>& triangleVec3, a std::vector of type <glm::vec3> that contains 3 vec3 'vectors' that make up a triangle.
* @param int currentIteration, the current iteration, or sub-division, to be calculated.
* @param const int numIterations, the total number of iterations, or sub-divisions, to be calculate.
*/
void generateSierpinskiRecurr(CPU_Geometry& cpuGeom, std::vector<glm::vec3>& triangleVec3, int currentIteration, const int numIterations)
{
	// calculating x and y halfway points of vectors
	float halfwayX[3] = {};	// stores all the halfway x-coordinates
	float halfwayY[3] = {};	// stores all the halfway y-coordinates

	// half way points for vectors 0 and 1
	halfwayX[0] = calcHalfWayX(triangleVec3[0], triangleVec3[1]);
	halfwayY[0] = calcHalfWayY(triangleVec3[0], triangleVec3[1]);
	// half way points for vectors 1 and 2
	halfwayX[1] = calcHalfWayX(triangleVec3[1], triangleVec3[2]);
	halfwayY[1] = calcHalfWayY(triangleVec3[1], triangleVec3[2]);
	// half way points for vectors 0 and 2
	halfwayX[2] = calcHalfWayX(triangleVec3[0], triangleVec3[2]);
	halfwayY[2] = calcHalfWayY(triangleVec3[0], triangleVec3[2]);

	// Create 'vec3' vectors using the halfway points calculated.
	std::vector<glm::vec3> halfWayVec3(3);
	for (int i = 0; i < halfWayVec3.size(); i++)
	{
		halfWayVec3[i] = glm::vec3(halfwayX[i], halfwayY[i], 0.f);
		// std::cout << "halfway vec3s: " << halfWayVec3[i] << std::endl; // debug
	}

	// debug
	//std::cout << "Halfway points " << currentIteration << " iteration: \n";
	//for (int i = 0; i < 3; i++)
	//{
	//	std::cout << "(" << halfwayX[i] << ", "
	//		<< halfwayY[i] << ")" << std::endl;
	//}

	// debug
	//std::cout << "Printing everything in 'triangleVec3': \n";
	//for (std::vector<glm::vec3>::iterator it = triangleVec3.begin(); it != triangleVec3.end(); it++)
	//{
	//	std::cout << (*it) << std::endl;

	//	//if ((*it) == glm::vec3())
	//	//	std::cout << "HI :) \n";
	//}
	// NOTE: Both do the same
	// std::cout << "Printing everything in 'allVec3': \n";
	//for (int i = 0; i < allVec3.size(); i++)
	//{
	//	std::cout << allVec3[i] << std::endl;
	//	if (allVec3[i] == glm::vec3())
	//		std::cout << "HI :) \n";
	//}

	// Creating sub-triangle groups
	std::vector<glm::vec3> subTriangle1(3);
	std::vector<glm::vec3> subTriangle2(3);
	std::vector<glm::vec3> subTriangle3(3);

	// sub-triangle 1
	subTriangle1[0] = triangleVec3[0];
	subTriangle1[1] = halfWayVec3[2];
	subTriangle1[2] = halfWayVec3[0];
	// sub-triangle 2
	subTriangle2[0] = triangleVec3[1];
	subTriangle2[1] = halfWayVec3[0];
	subTriangle2[2] = halfWayVec3[1];
	// sub-triangle 3
	subTriangle3[0] = triangleVec3[2];
	subTriangle3[1] = halfWayVec3[1];
	subTriangle3[2] = halfWayVec3[2];

	// Base case
	if (currentIteration == numIterations)
	{
		// Add the sub-triangles to the cpuGeom
		for (int i = 0; i < subTriangle1.size(); i++)
			cpuGeom.verts.push_back(subTriangle1[i]);
		for (int i = 0; i < subTriangle2.size(); i++)
			cpuGeom.verts.push_back(subTriangle2[i]);
		for (int i = 0; i < subTriangle3.size(); i++)
			cpuGeom.verts.push_back(subTriangle3[i]);

		return;
	}
	// Recursive case
	else
	{
		// Continue with recursion
		currentIteration++;
		generateSierpinskiRecurr(cpuGeom, subTriangle1, currentIteration, numIterations);
		generateSierpinskiRecurr(cpuGeom, subTriangle2, currentIteration, numIterations);
		generateSierpinskiRecurr(cpuGeom, subTriangle3, currentIteration, numIterations);
	}
}

/**
* 2.1 Part 1 : Sierpinski Triangle
*/
void generateSierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	std::vector<glm::vec3> baseTriangle(3, glm::vec3());

	// create base triangle
	baseTriangle[0] = (glm::vec3(-0.5f, -0.5f, 0.f));
	baseTriangle[1] = (glm::vec3(0.f, 0.5f, 0.f));
	baseTriangle[2] = (glm::vec3(0.5f, -0.5f, 0.f));

	// Debug
	//std::cout << "Printing base triangle: \n"
	//	<< baseTriangle[0] << "\n"
	//	<< baseTriangle[1] << "\n"
	//	<< baseTriangle[2] << "\n" << std::endl;

	if (numIterations == 0)
	{
		cpuGeom.verts.push_back(baseTriangle[0]);
		cpuGeom.verts.push_back(baseTriangle[1]);
		cpuGeom.verts.push_back(baseTriangle[2]);
	}
	else
	{
		int currentIteration = 1;
		generateSierpinskiRecurr(cpuGeom, baseTriangle, currentIteration, numIterations);
	}
	setRainbowCol(cpuGeom);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

/**
* Calculates the halfway x-coord. of two vectors.
* @return float, the halfway x-coord. of two vectors.
*/
float calcHalfWayX(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.x + v2.x) / 2;
}

/**
* Calculates the halfway y-coord. of two vectors.
* @return float, the halfway y-coord. of two vectors.
*/
float calcHalfWayY(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.y + v2.y) / 2;
}


void pythagorasOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	std::cout << "NOT IMPLMENTED YET";
	generatePythagorasTree(cpuGeom, gpuGeom);
}

void generatePythagorasTree(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	// Create base square
	std::vector<glm::vec3> baseVec3(4);
	baseVec3[0] = glm::vec3(-0.25, -0.5, 0.f);	// bottom left
	baseVec3[1] = glm::vec3(0.25, -0.5, 0.f);	// bottom right
	baseVec3[2] = glm::vec3(0.25, 0.f,0.f);		// top right
	baseVec3[3] = glm::vec3(-0.25, 0.f, 0.f);	// top left

	for (int i = 0; i < baseVec3.size(); i++)
	{
		cpuGeom.verts.push_back(baseVec3[i]);
		printVectorLocation(baseVec3[i], i);
	}

	std::cout << "cpu geom size = " << cpuGeom.verts.size() << std::endl;

	// first iteration test, left square
	std::vector<glm::vec3> sq1(4);
	sq1[0] = glm::vec3(-0.25, 0.f, 0.f);	// bottom
	sq1[1] = glm::vec3(0.f, 1 / (2 * sqrt(2)), 0.f);	// right
	sq1[2] = glm::vec3(-0.25, 2 / (2 * sqrt(2)), 0.f);	// top


	setRainbowCol(cpuGeom);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	std::cout << "NOT IMPLMENTED YET";
}

void generateKochSnowflake(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	std::cout << "NOT IMPLMENTED YET";
}

void dragonCurveOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	std::cout << "NOT IMPLMENTED YET";

}

/**
* Sets the colours for vertices inside the cpuGeom, rainbow.
* @param CPU_Geometry& cpuGeom, the address of cpuGeom that contains the vertices.
*/
void setRainbowCol(CPU_Geometry& cpuGeom)
{
	for (int i = 0, j = 0; i < cpuGeom.verts.size(); i++, j++)
	{
		if (j == 3)	// reset the count for 'j'
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

/**
* Prints the x and y locations of a "vec3"
* with a designated number, default is -1.
*/
void printVectorLocation(glm::vec3 vec)
{
	printVectorLocation(vec, -1);
}

/**
* Prints the x and y locations of a "vec3"
* with a designated number, default is -1.
*/
void printVectorLocation(glm::vec3 vec, int vecNum)
{
	std::cout << "vec #" << vecNum << " @ (" << vec.x << ", " << vec.y << ")" << std::endl;
}

void squarePatternTest(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numVerts)
{
	//int numVerts = 10;
	float xOffset = 0;
	float yOffset = 0;
	bool posDirection = true;
	bool xAxis = true;	// true = x-axis, false = y-axis

	for (int i = 1; i <= numVerts; i++)
	{
		if (xAxis)
		{
			if (xOffset >= 0 && i != 1)
				xOffset += 0.05 * i;
			else if (xOffset < 0)
				xOffset -= 0.05 * i;

			if (posDirection)
			{
				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
			}
			else
			{
				xOffset = -xOffset;
				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
				yOffset = -yOffset;
			}
		}
		else
		{
			if (xOffset == 0)
				yOffset = 0.05;

			yOffset += xOffset;

			if (posDirection)
			{
				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
			}
			else
			{
				cpuGeom.verts.push_back(glm::vec3(xOffset, yOffset, 0.f));
				std::cout << "added @ " << xOffset << ", " << yOffset << std::endl;
				xOffset = -xOffset;
			}
			posDirection = !posDirection;
		}

		xAxis = !xAxis;
	}

	std::cout << "Number of vertices inside cpuGeom = " << cpuGeom.verts.size() << std::endl;

	for (int i = 0, j = 0; i < numVerts; i++, j++)
	{
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

		if (j == 2)
			j = 0;
	}

	// equivalent code:
	//int count = 0, count2 = 0;;
	//while (count2 < numVerts)
	//{
	//	if (count == 0)
	//		cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	//	else if (count == 1)
	//		cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
	//	else
	//		cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));

	//	if (count == 2)
	//		count = 0;
	//	count++;
	//	count2++;
	//}

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}



