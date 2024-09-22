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

//#include <cmath>;

struct data
{
	int fractalOption;	// 1: Sierpinski Triangle | 2: Pythagoras Tree | 3: Koch Snowflake | 4: Dragon Curve
	int numSubDiv;
	CPU_Geometry& cpuGeom;
	GPU_Geometry& gpuGeom;
	Window& window;
};

// EXAMPLE CALLBACKS
//class MyCallbacks : public CallbackInterface {
//
//public:
//	MyCallbacks(ShaderProgram& shader) : shader(shader) {}
//
//	virtual void keyCallback(int key, int scancode, int action, int mods) {
//		if (key == GLFW_KEY_R && action == GLFW_PRESS)
//		{
//			shader.recompile();
//		}
//	}
//
//private:
//	ShaderProgram& shader;
//};
//
//class MyCallbacks2 : public CallbackInterface {
//
//public:
//	MyCallbacks2() {}
//
//	virtual void keyCallback(int key, int scancode, int action, int mods) {
//		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
//			std::cout << "called back" << std::endl;
//		}
//	}
//};
// END EXAMPLES

//void squarePatternTest(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numVerts);	// test, delete after.

// Sierpinski Triangle prototypes
void sierpinskiOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void generateSierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations);
void setRainbowCol(CPU_Geometry& cpuGeom);
float calcHalfWayX(const glm::vec3& v1, const glm::vec3& v2);
float calcHalfWayY(const glm::vec3& v1, const glm::vec3& v2);

// Pythagoras Tree prototypes
void pythagorasOption(Window& window, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void generatePythagorasTree(Window& window, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations);
void generatePythagorasRecurr(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);
float calcSideS(float hypotenuse);

void generatePythagorasRecurrLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);
void generatePythagorasRecurrLeftRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);

void generatePythagorasRecurrRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);
void generatePythagorasRecurrRightLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);

// Koch Snowflake prototypes
void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);

// Dragon Curve prototypes
void dragonCurveOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom);

void drawFractal(int option, const CPU_Geometry& cpuGeom);
//glm::vec3& rotateVec3(glm::vec3& vec3, float degree, int axisOption);
void rotateCCWAboutVec3(glm::vec3& vec3ToRotate, const glm::vec3 rotateAboutVec, const float angleOfRotation);

// Debugging prototypes
void printVectorLocation(glm::vec3 vec, int vecNum);
void printVectorLocation(glm::vec3 vec);

class switchSceneCallBack : public CallbackInterface
{
	public:
		switchSceneCallBack(ShaderProgram& shader, data& d) : shader(shader), sceneData(d), win(d.window) {}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		int& option = sceneData.fractalOption;
		int& subDiv = sceneData.numSubDiv;
		CPU_Geometry& cpuGeom = sceneData.cpuGeom;
		GPU_Geometry& gpuGeom = sceneData.gpuGeom;

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			shader.recompile();
		}
		else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			std::cout << "\nswitching scene ->... (not implmented)\n" << std::endl;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			std::cout << "\nswitching scene <-... (not implmented)\n" << std::endl;
		}
		else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			sceneData.numSubDiv++;	// increment the current subdivisions by 1
			std::cout << "\nIncrementing number of subdivisions by 1 (Current = " << subDiv << ")...\n" << std::endl;

			// Checking the option chosen
			switch (option)
			{
				case 1:
					generateSierpinskiTriangle(cpuGeom, gpuGeom, subDiv);
					break;
				case 2:
					generatePythagorasTree(win, cpuGeom, gpuGeom, subDiv);
					break;
				case 3:
					
					break;
				case 4:
					
					break;
				default:
					std::cout << "\nInvalid option..." << std::endl;
			}
			drawFractal(option, cpuGeom);

		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			sceneData.numSubDiv--;	// decrement the current subdivisions by 1

			if (sceneData.numSubDiv < 0)
			{
				sceneData.numSubDiv = 0;
				std::cout << "\nCannot have negative subdivisions...\n" << std::endl;
				return;
			}

			std::cout << "\nDecrementing number of subdivisions by 1 (Current = " << subDiv << ")...\n" << std::endl;

			// Checking the option chosen
			switch (option)
			{
				case 1:
					generateSierpinskiTriangle(cpuGeom, gpuGeom, subDiv);
					break;
				case 2:
					generatePythagorasTree(win, cpuGeom, gpuGeom, subDiv);
					break;
				case 3:
					//snowflakeOption(cpuGeom, gpuGeom);
					break;
				case 4:
					//dragonCurveOption(cpuGeom, gpuGeom);
					break;
				default:
					std::cout << "\nInvalid option..." << std::endl;
			}
			drawFractal(option, cpuGeom);

		}
	}

	private:
		ShaderProgram& shader;
		data& sceneData;
		Window& win;
};

int main()
{
	Log::debug("Starting main");

	bool exit = false;
	int option = -1; 	// used to determine what fractal the user wants to generate (1: triangle, 2: Pythagoras, 3: snowflake, 4: dragon curve, 0: exit) 

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
			std::cout << "\nInvalid option..." << std::endl;
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

		// GEOMETRY
		CPU_Geometry cpuGeom;
		GPU_Geometry gpuGeom;

		data newData = {option, -1, cpuGeom, gpuGeom, window};

		// CALLBACKS
		window.setCallbacks(std::make_shared<switchSceneCallBack>(shader, newData));

		//numSubDiv = -1;

		// Checking the option chosen
		switch(option)
		{
			case 0:
				exit = !exit;
				std::cout << "\nExiting..." << std::endl;
				continue;			// iterate loop from start
			case 1:
				sierpinskiOption(cpuGeom, gpuGeom, newData);
				break;
			case 2:
				pythagorasOption(window, cpuGeom, gpuGeom, newData);
				break;
			case 3:
				snowflakeOption(cpuGeom, gpuGeom);
				break;
			case 4:
				dragonCurveOption(cpuGeom, gpuGeom);
				break;
			default:
				std::cout << "\nInvalid option..." << std::endl;
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// ignore the rest of the user's input
				option = -1;
				continue;			// iterate loop from start
		}

		std::cout	<< "\nPlease close the shape/fractal generation window to go back to selection menu/console.\n"
					<< "---Additionally you use the following keys to:\n"
					<< "UP_ARROW:			Increment the number of current subdivisions by 1.\n"
					<< "DOWN_ARROW:			Decrement the number of current subdivisions by 1.\n"
					<< "LEFT_SIDE_ARROW:	Move to another scene <-.\n"
					<< "RIGHT_UP_ARROW:		Move to another scene ->.\n"
					<< "\n" << std::endl;

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
		case 1:		// Draw's Sierpinski Triangle
			glDrawArrays(GL_TRIANGLES, 0, cpuGeom.verts.size());
			break;
		case 2:		// Draw's Pythagoras Tree
			// "squareVec3Start" is the starting 'vec3' in 'cpuGeom' for this square.
			// 'j' is how many 'vec3's to draw (4) for a square.
			for (int squareVec3Start = 0, j = 4; squareVec3Start < cpuGeom.verts.size(); squareVec3Start += 4)
				glDrawArrays(GL_TRIANGLE_FAN, squareVec3Start, j);
			break;
		case 3:		// Draw's Koch Snowflake
			// use "GL_TRIANGLE_STRIP" ???
			std::cout << "\nSnowflake drawing not implemented...\n" << std::endl;
			break;
		case 4:		// Draw's Dragon curve
			std::cout << "\nDragon curve drawing not implemented...\n" << std::endl;
			break;
		default:
			std::cout << "\nError drawing...\n" << std::endl;
			break;
	}
}

/**
* Rotates a glm::vec3 around another glm::vec3 with a specified angle on the x-y plane (z coord. is not used).
* @param glm::vec3& vec3ToRotate, the glm::vec3 to be rotated.
* @param const glm::vec3 rotateAboutVec, the glm::vec3 that will be rotated about.
* @param const float angleOfRotation, the angle to rotate 'vec3ToRotate' about 'rotateAboutVec', in degree's.
*/
void rotateCCWAboutVec3(glm::vec3& vec3ToRotate, const glm::vec3 rotateAboutVec, const float angleOfRotation)
{
	// To rotate 'vec3ToRotate' around 'rotateAboutVec' I'll do these steps:
	// 1) Translate 'vec3ToRotate's x and y values by 'translateByX' and 'translateByY' respectively, getting x' and y'.
	// 2) Rotate the translated x' and y' by 'degree', getting x'' and y''.
	// 3) Translate x'' and y'' by the inverse of 'translateByX' and 'translateByY' (subtract). The result will be the rotated 'vec3ToRotate' about 'rotateAboutVec'.

	// Convert 'degree' into radians
	double pi = atan(1) * 4;			// pi approx.
	float rads = angleOfRotation * (pi/ 180);

	// get x and y values that would translate 'rotateAboutVec' to origin
	float translateByX = -rotateAboutVec.x;
	float translateByY = -rotateAboutVec.y;

	// Calculate the final x and y coordinates using the steps described above. (Note: The steps above have been merged into one 'step')
	float originalX = vec3ToRotate.x;
	float originalY = vec3ToRotate.y;
	float finalX = (originalX * cos(rads)) - (originalY * sin(rads)) + (translateByX * cos(rads) - translateByY * sin(rads) - translateByX);
	float finalY = (originalX * sin(rads)) + (originalY * cos(rads)) + (translateByX * sin(rads) + translateByY * cos(rads) - translateByY);

	// Set the x and y values.
	vec3ToRotate.x = finalX;
	vec3ToRotate.y = finalY;
}

/**
* Asks the user how many subdivisions and then generates said number of subdivisions.
* @param CPU_Geometry& cpuGeom, reference to the CPU_Geometry to contain the 'vec3's generated.
* @param GPU_Geometry& gpuGeom, reference to the GPU_Geometry to set its 'vec3's and colours.
*/
void sierpinskiOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData)
{
	int numSubDiv = -1;
	while (numSubDiv < 0)
	{
		std::cout << "\n--Sierpinski Triangle--\n How many subdivisions would you like? ";
		std::cin >> numSubDiv;
		if (numSubDiv < 0)
		{
			std::cout << "\nInvalid input..." << std::endl;
			continue;
		}
		sceneData.numSubDiv = numSubDiv;
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
	if (numIterations == -1)
	{
		std::cout << "\nCannot have negative subdivisions\n" << std::endl;
		return;
	}

	// Clear what is inside the cpuGeom
	cpuGeom.verts.clear();
	cpuGeom.cols.clear();

	std::vector<glm::vec3> baseTriangle(3, glm::vec3());

	// create base triangle
	baseTriangle[0] = (glm::vec3(-1.f, -1.f, 0.f));
	baseTriangle[1] = (glm::vec3(0.f, 1.f, 0.f));
	baseTriangle[2] = (glm::vec3(1.f, -1.f, 0.f));

	// OLD
	//baseTriangle[0] = (glm::vec3(-0.5f, -0.5f, 0.f));
	//baseTriangle[1] = (glm::vec3(0.f, 0.5f, 0.f));
	//baseTriangle[2] = (glm::vec3(0.5f, -0.5f, 0.f));

	// this way works too
	//baseTriangle[2] = (glm::vec3(-0.5f, -0.5f, 0.f));
	//baseTriangle[1] = (glm::vec3(0.f, 0.5f, 0.f));
	//baseTriangle[0] = (glm::vec3(0.5f, -0.5f, 0.f));

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


void pythagorasOption(Window& window, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData)
{
	int numSubDiv = -1;
	while (numSubDiv < 0)
	{
		std::cout << "\n--Pythagoras Tree--\n How many subdivisions would you like? ";
		std::cin >> numSubDiv;
		if (numSubDiv < 0)
		{
			std::cout << "\nInvalid input..." << std::endl;
			continue;
		}
		sceneData.numSubDiv = numSubDiv;
		generatePythagorasTree(window, cpuGeom, gpuGeom, numSubDiv);
		std::cout << "\nPythagoras Tree with " << numSubDiv << " subdivisions created." << std::endl;
	}
}

/**
* Generates the right-hand square for squares on the "left side" of the base square (root square).
* Should only be called by itself and 'generatePythagorasRecurrLeft(...)' methods.
*/
void generatePythagorasRecurrLeftRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side length at the same subdiv.

	// Calc. right square vec3's
	std::vector<glm::vec3> rightSq(4);
	float z = currentIteration / numIterations;
	// bottom left point
	rightSq[0] = glm::vec3(rightVec.x, rightVec.y, z);  // rightVec;
	// bottom right point
	rightSq[1] = glm::vec3(rightVec.x + side, rightVec.y, z);
	// top right point
	glm::vec3 bottomRight = rightSq[1];
	rightSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, z);
	// top left point
	glm::vec3 bottomLeft = rightSq[0];
	glm::vec3 topRight = rightSq[2];
	rightSq[3] = glm::vec3(bottomLeft.x, topRight.y, z);

	// Rotate the right square's vec3s about 'rightVec'
	for (int i = 0; i < rightSq.size(); i++)
		rotateCCWAboutVec3(rightSq[i], rightVec, radianOffset);

	// Add the vec3's to the cpuGeom
	for (int i = 0; i < rightSq.size(); i++)
		cpuGeom.verts.push_back(rightSq[i]);

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;		// Increase the offset by 45 degrees for the next subdivision level
		// Generate the next subdivision
		generatePythagorasRecurrLeft(cpuGeom, rightSq[2], rightSq[1], side, currentIteration, numIterations, radianOffset - 90);		// generate this square's left square (reset radianOffset)
		generatePythagorasRecurrLeftRight(cpuGeom, rightSq[2], rightSq[1], side, currentIteration, numIterations, radianOffset - 90);	// generate this square's right square (reset radianOffset)
	}
}

/**
* Generates the left-hand squares for squares on the "left side" of the base square (root square).
* Should only be called by itself and 'generatePythagorasRecurrLeftRight(...)' methods.
*/
void generatePythagorasRecurrLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side legnth at the same subdiv.

	// Calc. left Square vec3's
	std::vector< glm::vec3> leftSq(4);
	float z = currentIteration / numIterations;
	// bottom left point
	leftSq[0] = glm::vec3(leftVec.x, leftVec.y, z); // leftVec;
	// bottom right point
	leftSq[1] = glm::vec3(leftVec.x + side, leftVec.y, z);
	// top right point
	glm::vec3 bottomRight = leftSq[1];
	leftSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, z);
	// top left point
	glm::vec3 topRight = leftSq[2];
	leftSq[3] = glm::vec3(leftVec.x, topRight.y, z);

	// Rotate the left square's vec3s about 'leftVec'
	for (int i = 0; i < leftSq.size(); i++)
		rotateCCWAboutVec3(leftSq[i], leftVec, radianOffset);

	// Add the left square vec3's to the cpuGeom
	for (int i = 0; i < leftSq.size(); i++)
		cpuGeom.verts.push_back(leftSq[i]);

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;	// Increase the offset by 45 degrees for the next subdivision level
		// Generate the next subdivision
		generatePythagorasRecurrLeft(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);		// generate this square's left square
		generatePythagorasRecurrLeftRight(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);	// generate this square's right square
	}
}

/**
* Generates the left-hand square for squares on the "right side" of the base square (root square).
* Should only be called by itself and 'generatePythagorasRecurrRight(...)' methods.
*/
void generatePythagorasRecurrRightLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side length at the same subdiv.

	// Calc. left Square vec3's
	std::vector< glm::vec3> leftSq(4);
	float z = currentIteration / numIterations;
	// bottom left point
	leftSq[0] = glm::vec3(leftVec.x - side, leftVec.y, z);
	// bottom right point
	leftSq[1] = glm::vec3(leftVec.x, leftVec.y, z); // leftVec;
	// top right point
	glm::vec3 bottomRight = leftSq[1];
	leftSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, z);
	// top left point
	glm::vec3 topRight = leftSq[2];
	glm::vec3 bottomLeft = leftSq[0];
	leftSq[3] = glm::vec3(bottomLeft.x, topRight.y, z);

	// Rotate the left square's vec3s about 'leftVec'
	for (int i = 0; i < leftSq.size(); i++)
		rotateCCWAboutVec3(leftSq[i], leftVec, -radianOffset);
		// I made radianOffset negative so this rotates in the clockwise direction due to how I defined the initial squares glm::vec3 locations

	// Add the left square vec3's to the cpuGeom
	for (int i = 0; i < leftSq.size(); i++)
		cpuGeom.verts.push_back(leftSq[i]);
	
	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;	// Increase the offset by 45 degrees for the next subdivision level
		// Generate the next subdivision
		generatePythagorasRecurrRight(cpuGeom, leftSq[0], leftSq[3], side, currentIteration, numIterations, radianOffset - 90);		// generate this square's right square (reset radianOffset)
		generatePythagorasRecurrRightLeft(cpuGeom, leftSq[0], leftSq[3], side, currentIteration, numIterations, radianOffset - 90); // generate this square's left square (reset radianOffset)
	}
}

/**
* Generates the right-hand squares for squares on the "right side" of the base square (root square).
* Should only be called by itself and 'generatePythagorasRecurrRightLeft(...)' methods.
*/
void generatePythagorasRecurrRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side length at the same subdiv.

	// Calc. right square vec3's
	std::vector<glm::vec3> rightSq(4);
	float z = currentIteration / numIterations;
	// bottom left point
	rightSq[0] = glm::vec3(rightVec.x - side, rightVec.y, z);
	// bottom right point
	rightSq[1] = glm::vec3(rightVec.x, rightVec.y, z); // rightVec;
	// top right point
	glm::vec3 bottomRight = rightSq[1];
	rightSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, z);
	// top left point
	glm::vec3 bottomLeft = rightSq[0];
	glm::vec3 topRight = rightSq[2];
	rightSq[3] = glm::vec3(bottomLeft.x, topRight.y, z);

	// Rotate the right square's vec3s about 'rightVec'
	for (int i = 0; i < rightSq.size(); i++)
		rotateCCWAboutVec3(rightSq[i], rightVec, -radianOffset);
		// I made radianOffset negative so this rotates in the clockwise direction due to how I defined the initial squares glm::vec3 locations

	for (int i = 0; i < rightSq.size(); i++)
		cpuGeom.verts.push_back(rightSq[i]);

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;	// increase the rotation offset by 45-degrees
		// Generate the next subdivision
		generatePythagorasRecurrRight(cpuGeom, rightSq[3], rightSq[2], side, currentIteration, numIterations, radianOffset);		// generate this square's right square
		generatePythagorasRecurrRightLeft(cpuGeom, rightSq[3], rightSq[2], side, currentIteration, numIterations, radianOffset);	// generate this square's left square

	}
}

/**
* 2.2 Part 2 : Pythagoras Tree
*/
void generatePythagorasTree(Window& window, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	if (numIterations == -1)
	{
		std::cout << "\nCannot have negative subdivisions\n" << std::endl;
		return;
	}

	// Clear what is inside the cpuGeom
	cpuGeom.verts.clear();
	cpuGeom.cols.clear();

	// debug/test
	int height = window.getHeight();
	int width = window.getWidth();
	//std::cout	<< "\nwindow height = " << height
	//			<< "\nwindow width = " << width << std::endl;

	// Create base square
	std::vector<glm::vec3> baseVec3(4);

	// OLD BASE (ROOT) SQUARE
	//baseVec3[0] = glm::vec3(-0.25, -0.5, 0.f);	// bottom left
	//baseVec3[1] = glm::vec3(0.25, -0.5, 0.f);	// bottom right
	//baseVec3[2] = glm::vec3(0.25, 0.f,0.f);		// top right
	//baseVec3[3] = glm::vec3(-0.25, 0.f, 0.f);	// top left

	baseVec3[0] = glm::vec3(-0.25, -1.f, 0.f);	// bottom left
	baseVec3[1] = glm::vec3(0.25, -1.f, 0.f);	// bottom right
	baseVec3[2] = glm::vec3(0.25, -0.5, 0.f);	// top right
	baseVec3[3] = glm::vec3(-0.25, -0.5, 0.f);	// top left

	// Add the base (root) square to the cpuGeom
	for (int i = 0; i < baseVec3.size(); i++)
		cpuGeom.verts.push_back(baseVec3[i]);
	
	float hypotenuse = abs(baseVec3[3].x) + abs(baseVec3[2].x);	// the hypotenuse will the the length of the parent square

	if (numIterations > 0)
	{
		// Generate sub divisions if the user input is > 0
		generatePythagorasRecurrLeft(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);
		generatePythagorasRecurrRight(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);
	}

	// for now, set colors to rainbow
	setRainbowCol(cpuGeom);

	// Add the colours and vec3's to the gpuGeom
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	// debug/test
	//std::cout << "\nNumber of squares to draw = " << cpuGeom.verts.size() / 4 << std::endl;
}

/**
* Calculates the side 's' of a square using the hypotenuse (side length of its 'parent' square).
* @param float hypotenuse, the side length of its parent square.
*/
float calcSideS(float hypotenuse)
{
	return hypotenuse / (sqrt(2));
}

void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom)
{
	std::cout << "NOT IMPLMENTED YET";
}

void generateKochSnowflakeRecurr(CPU_Geometry& cpuGeom, int currentIteration, const int numIterations)
{

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
	std::cout << "vec #" << vecNum << " @ (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
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



