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
void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void generateKochSnowflake(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations);
//void generateKochSnowflakeRecurr(CPU_Geometry& cpuGeom, glm::vec3 leftVec3, glm::vec3 rightVec3, int currentIteration, const int numIterations, float angleOffSet);
//void generateKochSnowflakeRecurr(CPU_Geometry& cpuGeom, glm::vec3 startingVec3, int currentIteration, const int numIterations, float angleOffSet);

// Dragon Curve prototypes
void dragonCurveOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void genererateDragonCurve(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations);

void drawFractal(int option, const CPU_Geometry& cpuGeom);
//glm::vec3& rotateVec3(glm::vec3& vec3, float degree, int axisOption);
void rotateCCWAboutVec3(glm::vec3& vec3ToRotate, const glm::vec3 rotateAboutVec, const float angleOfRotation);

// Debugging prototypes
void printVectorLocation(glm::vec3 vec, int vecNum);
void printVectorLocation(glm::vec3 vec);


void clearCPUGeom(CPU_Geometry& cpuGeom);

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
			sceneData.fractalOption++;	// increment the fractal option by 1 to move right
			if (option > 4)
				sceneData.fractalOption = 1;

			std::cout << "\nswitching scene (Right) -> (Current Option = " << option << ")\n" << std::endl;

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
				generateKochSnowflake(cpuGeom, gpuGeom, subDiv);
				break;
			case 4:
				genererateDragonCurve(cpuGeom, gpuGeom, subDiv);
				break;
			default:
				std::cout << "\nInvalid option..." << std::endl;
			}
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			sceneData.fractalOption--;	// decrement the fractal option by 1 to move left
			if (option < 1)
				sceneData.fractalOption = 4;

			std::cout << "\nswitching scene (Left) <- (Current Option = " << option << ")\n" << std::endl;

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
				generateKochSnowflake(cpuGeom, gpuGeom, subDiv);
				break;
			case 4:
				genererateDragonCurve(cpuGeom, gpuGeom, subDiv);
				break;
			default:
				std::cout << "\nInvalid option..." << std::endl;
			}
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
					generateKochSnowflake(cpuGeom, gpuGeom, subDiv);
					break;
				case 4:
					genererateDragonCurve(cpuGeom, gpuGeom, subDiv);
					break;
				default:
					std::cout << "\nInvalid option..." << std::endl;
			}
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
					generateKochSnowflake(cpuGeom, gpuGeom, subDiv);
					break;
				case 4:
					genererateDragonCurve(cpuGeom, gpuGeom, subDiv);
					break;
				default:
					std::cout << "\nInvalid option..." << std::endl;
			}
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
				snowflakeOption(cpuGeom, gpuGeom, newData);
				break;
			case 4:
				dragonCurveOption(cpuGeom, gpuGeom, newData);
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
		while (!window.shouldClose())
		{
			option = newData.fractalOption;	// make sure to keep updating option
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

/// <summary>
/// Adds the contents of a std::vector(glm::vec3) to the cpuGeom.
/// </summary>
/// <param name="cpuGeom"> a CPU_Geometry.</param>
/// <param name="arr"> the std::vector(glm::vec3) contents to be added. </param>
void addToCpuGeomVerts(CPU_Geometry& cpuGeom, std::vector<glm::vec3>& arr)
{
	for (int i = 0; i < arr.size(); i++)
	{
		cpuGeom.verts.push_back(arr[i]);
		// debugging
		printVectorLocation(arr[i]);
		std::cout << "pushed to cpuGeom[" << cpuGeom.verts.size() - 1 << "]" << std::endl;
	}
}

/// <summary>
/// Draws the fractal the user chose.
/// </summary>
/// <param name="option"> an int, the fractal option generated. </param>
/// <param name="cpuGeom"> const CPU_Geometry, the CPU_Geometry reference that contains the 'vec3's of the fractal shape.</param>
void drawFractal(int option, const CPU_Geometry& cpuGeom)
{
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (option)
	{
		// Draw's Sierpinski's Triangle
		case 1:
			glDrawArrays(GL_TRIANGLES, 0, cpuGeom.verts.size());
			break;
		// Draw's Pythagoras Tree
		case 2:
			// "squareVec3Start" is the starting 'vec3' in 'cpuGeom' for this square.
			// 'j' is how many 'vec3's to draw (4) for a square.
			for (int squareVec3Start = 0, j = 4; squareVec3Start < cpuGeom.verts.size(); squareVec3Start += 4)
				glDrawArrays(GL_TRIANGLE_FAN, squareVec3Start, j);
			break;
		// Draw's Koch Snowflake
		case 3:
			//// use "GL_TRIANGLE_STRIP" ???
			//std::cout << "\nSnowflake drawing not implemented...\n" << std::endl;
			//glDrawArrays(GL_LINE_LOOP, 0, cpuGeom.verts.size());
			glDrawArrays(GL_LINE_STRIP, 0, cpuGeom.verts.size());
			//glDrawArrays(GL_TRIANGLES, 0, cpuGeom.verts.size());

			break;
		// Draw's Dragon curve
		case 4:
			glDrawArrays(GL_LINE_STRIP, 0, cpuGeom.verts.size());
			break;
		default:
			std::cout << "\nError drawing...\n" << std::endl;
			break;
	}
}

/// <summary>
/// Rotates a glm::vec3 around another glm::vec3 with a specified angle on the x-y plane (z coord. is not used or affected).
/// </summary>
/// <param name="vec3ToRotate"> the glm::vec3 to be rotated.</param>
/// <param name="rotateAboutVec"> a const glm::vec3 that will be rotated about.</param>
/// <param name="angleOfRotation"> the angle to rotate 'vec3ToRotate' about 'rotateAboutVec', in degree's. </param>
void rotateCCWAboutVec3(glm::vec3& vec3ToRotate, const glm::vec3 rotateAboutVec, const float angleOfRotation)
{
	// To rotate 'vec3ToRotate' around 'rotateAboutVec' I'll do these steps:
	// 1) Translate 'vec3ToRotate's x and y values by 'translateByX' and 'translateByY' respectively, getting x' and y'.
	// 2) Rotate the translated x' and y' by 'degree', getting x'' and y''.
	// 3) Translate x'' and y'' by the inverse of 'translateByX' and 'translateByY' (subtract). The result will be the rotated 'vec3ToRotate' about 'rotateAboutVec'.

	// Convert 'degree' into radians
	double pi = atan(1) * 4;	// pi approximation
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

/// <summary>
/// Asks the user how many subdivisions and then generates said number of subdivisions.
/// </summary>
/// <param name="cpuGeom"> reference to the CPU_Geometry to contain the 'vec3's generated.</param>
/// <param name="gpuGeom"> reference to the GPU_Geometry to set its 'vec3's and colours.</param>
/// <param name="sceneData"> a data.</param>
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

/// <summary>
/// 2.1 Part 1 : Sierpinski's Triangle.
/// Generates the Sierpinski's Triangle.
/// </summary>
/// <param name="cpuGeom"> a reference to a CPU_Geometry to store the fractal. </param>
/// <param name="gpuGeom"> a reference to a CPU_Geometry. </param>
/// <param name="numIterations"> an int, the desired number of subdivisions. </param>
void generateSierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	if (numIterations == -1)
	{
		std::cout << "\nCannot have negative subdivisions\n" << std::endl;
		return;
	}

	// Clear what is inside the cpuGeom
	clearCPUGeom(cpuGeom);

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
		// Generate the fractal
		int currentIteration = 1;
		generateSierpinskiRecurr(cpuGeom, baseTriangle, currentIteration, numIterations);
	}
	setRainbowCol(cpuGeom);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

/// <summary>
/// Calculates the halfway x-coord. of two glm::vec3's.
/// </summary>
/// <param name="v1"> a glm::vec3. </param>
/// <param name="v2"> a glm::vec3. </param>
/// <returns> the halfway x-coord., a float. </returns>
float calcHalfWayX(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.x + v2.x) / 2;
}

/// <summary>
/// Calculates the halfway y-coord. of two glm::vec3's.
/// </summary>
/// <param name="v1"> a glm::vec3.</param>
/// <param name="v2"> a glm::vec3.</param>
/// <returns> the halfway y-coord, a float.</returns>
float calcHalfWayY(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.y + v2.y) / 2;
}

/// <summary>
/// Pythagoras Tree Option, interacts with the user via the command prompt, for the number of desired subdivisions.
/// </summary>
/// <param name="window"> a Window, displaying the fractal. </param>
/// <param name="cpuGeom"> a CPU_Geometry that holds the fractal's glm::vec's, address.</param>
/// <param name="gpuGeom"> a GPU_Geometry, address.</param>
/// <param name="sceneData"> a data, address.</param>
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

/// <summary>
///  Generates the right-hand square for squares on the "left side" of the base square (root square).
/// Should only be called by itself and 'generatePythagorasRecurrLeft(...)' methods.
/// </summary>
/// <param name="cpuGeom"></param>
/// <param name="leftVec"></param>
/// <param name="rightVec"></param>
/// <param name="hypotenuse"></param>
/// <param name="currentIteration"></param>
/// <param name="numIterations"></param>
/// <param name="radianOffset"></param>
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

/// <summary>
/// Generates the left-hand squares for squares on the "left side" of the base square (root square).
/// Should only be called by itself and 'generatePythagorasRecurrLeftRight(...)' methods.
/// </summary>
/// <param name="cpuGeom"></param>
/// <param name="leftVec"></param>
/// <param name="rightVec"></param>
/// <param name="hypotenuse"></param>
/// <param name="currentIteration"></param>
/// <param name="numIterations"></param>
/// <param name="radianOffset"></param>
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

/// <summary>
/// Generates the left-hand square for squares on the "right side" of the base square (root square).
/// Should only be called by itself and 'generatePythagorasRecurrRight(...)' methods.
/// </summary>
/// <param name="cpuGeom"></param>
/// <param name="leftVec"></param>
/// <param name="rightVec"></param>
/// <param name="hypotenuse"></param>
/// <param name="currentIteration"></param>
/// <param name="numIterations"></param>
/// <param name="radianOffset"></param>
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

/// <summary>
/// Generates the right-hand squares for squares on the "right side" of the base square (root square).
/// Should only be called by itself and 'generatePythagorasRecurrRightLeft(...)' methods.
/// </summary>
/// <param name="cpuGeom"></param>
/// <param name="leftVec"></param>
/// <param name="rightVec"></param>
/// <param name="hypotenuse"></param>
/// <param name="currentIteration"></param>
/// <param name="numIterations"></param>
/// <param name="radianOffset"></param>
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

/// <summary>
/// 2.2 Part 2 : Pythagoras Tree
/// </summary>
/// <param name="window"></param>
/// <param name="cpuGeom"></param>
/// <param name="gpuGeom"></param>
/// <param name="numIterations"></param>
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

	//baseVec3[0] = glm::vec3(-0.25, -1.f, 0.f);	// bottom left
	//baseVec3[1] = glm::vec3(0.25, -1.f, 0.f);	// bottom right
	//baseVec3[2] = glm::vec3(0.25, -0.5, 0.f);	// top right
	//baseVec3[3] = glm::vec3(-0.25, -0.5, 0.f);	// top left

	baseVec3[0] = glm::vec3(-0.125, -0.5f, 0.f);	// bottom left
	baseVec3[1] = glm::vec3(0.125, -0.5f, 0.f);	// bottom right
	baseVec3[2] = glm::vec3(0.125, -0.25, 0.f);	// top right
	baseVec3[3] = glm::vec3(-0.125, -0.25, 0.f);	// top left

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

//glm::vec3 scaleVec3(glm::vec3 v, float scale)
//{
//	v.x = (v.x * scale);
//	v.y = (v.y * scale);
//	v.z = (v.z * scale);
//	return v;
//}

//float max(float num1, float num2)
//{
//	if (num1 > num2)
//		return num1;
//	else
//		return num2;
//}

//void generateKochSnowflakeRecurr(CPU_Geometry& cpuGeom, glm::vec3 leftVec3, glm::vec3 rightVec3, int currentIteration, const int numIterations, float angleOffSet)
//{
//	//if (currentIteration == numIterations)
//	//{
//	//	cpuGeom.verts.push_back(rightVec3);
//	//	cpuGeom.verts.push_back(leftVec3);
//	//	return;
//	//}
//	std::cout << "\nIteration: " << currentIteration;
//
//	// Calculate subtriangle points
//	
//	// Side length of subtriangle will be 1/3 of the length of it's parent triangle. (1/3 the length of the line that leftVec3 and rightVec3 draws)
//	float sideLength = 1 / 3.f;	// replace 1 / 3.f with an inital line lenght when i have time, since the triangle wont have a length of 1, this was hard coded
//	for (int i = 1; i < currentIteration; i++)
//		sideLength = sideLength / 3.f;
//
//	// left sub-point
//	glm::vec3 left = glm::vec3(leftVec3.x + sideLength, leftVec3.y, 0.f);
//
//	// right sub-point (translate by sideLength from 'left')
//	glm::vec3 right = glm::vec3(left.x + sideLength, left.y, 0.f);
//
//	// top point
//	float topX = (rightVec3.x + leftVec3.x) / 2.f;
//	//float topY = sqrt(pow(sideLength, 2) - pow(topX, 2));
//	// Convert 60-degrees into radians
//	double pi = atan(1) * 4;			// pi approx.
//	float rads = 60 * (pi / 180);
//	float topY = tan(rads) * (sideLength / 2.f);
//
//	glm::vec3 top = glm::vec3(topX, topY, 0.f);
//
//	std::cout << "\nLEFTVec3:";
//	printVectorLocation(leftVec3);
//	std::cout << "\nRIGHTVec3:";
//	printVectorLocation(rightVec3);
//
//	std::cout << "\nSideL: " << sideLength << std::endl;
//	std::cout << "\nLeft:";
//	printVectorLocation(left);
//	std::cout << "\nRight = ";
//	printVectorLocation(right);
//	std::cout << "\nTop:";
//	printVectorLocation(top);
//
//	// debugging
//	std::vector<glm::vec3> subTriangle(3);
//	subTriangle[0] = left;
//	subTriangle[1] = top;
//	subTriangle[2] = right;
//	for (int i = 0; i < subTriangle.size(); i++)
//		cpuGeom.verts.push_back(subTriangle[i]);
//
//	if (currentIteration != numIterations)
//	{
//		int iter = ++currentIteration;
//		float angleOff = angleOffSet + 60;
//		generateKochSnowflakeRecurr(cpuGeom, leftVec3, left, iter, numIterations, angleOff);
//		generateKochSnowflakeRecurr(cpuGeom, left, top, iter, numIterations, angleOff);
//		//generateKochSnowflakeRecurr(cpuGeom, leftVec3, left, iter, numIterations, angleOff); 
//
//	}
//	else
//	{
//		//cpuGeom.verts.push_back(rightVec3);
//		//cpuGeom.verts.push_back(leftVec3);
//	}
//
//}
//// rotate leftVec3 and rightVec3 by angleOffSet
//glm::vec3 origin = glm::vec3(0.f, 0.f, 0.f);
//rotateCCWAboutVec3(leftVec3, origin, angleOffSet);
//rotateCCWAboutVec3(rightVec3, origin, -angleOffSet);
// 
	//float translate = 2 * ((abs(leftVec3.x) + abs(rightVec3.x)) / 2.f) / 1/3.f;	// used to translate vec3's to create subtriangle bottom points
	////float translate = 2 / 3.f;
	//std::cout << "\n(ITERATION: " << currentIteration << ")\ntranslate = " << translate << std::endl;

	//// Calculate sub triangle points

	//// bottom right
	//std::cout << "\nbefore translate (right): ";
	//printVectorLocation(rightVec3);
	//glm::vec3 bottomRight = glm::vec3(rightVec3.x - translate, rightVec3.y, rightVec3.z);
	//std::cout << "\nafter translate: ";
	//printVectorLocation(bottomRight);

	//// bottom left
	//std::cout << "\nbefore translate (left): ";
	//printVectorLocation(leftVec3);
	//glm::vec3 bottomLeft = glm::vec3(leftVec3.x + translate, leftVec3.y, leftVec3.z);
	//std::cout << "\nafter translate: ";
	//printVectorLocation(bottomLeft);

	//// top middle
	//	 //calc. halfway x of 'leftVec3' and 'rightVec3'
	////float halfX = calcHalfWayX(leftVec3, rightVec3);
	//glm::vec3 top = glm::vec3(bottomLeft.x + translate, bottomLeft.y, bottomLeft.z);
	//std::cout << "\ntop before CCW rotate (60): ";
	//printVectorLocation(top);
	//rotateCCWAboutVec3(top, bottomLeft, 60);

	//std::cout << "\ntop after CCW rotate(60): ";
	//printVectorLocation(top);

	//std::vector<glm::vec3> subTriangle(3);
	//subTriangle[0] = bottomLeft;
	//subTriangle[1] = top;
	//subTriangle[2] = bottomRight;

	//if (currentIteration != numIterations)
	//{
	//	int iter = currentIteration + 1;
	//	float offset = angleOffSet + 60;
	//	generateKochSnowflakeRecurr(cpuGeom, bottomLeft, top, iter, numIterations, offset);	// left side
	//}
	//for (int i = 0; i < subTriangle.size(); i++)
	//	cpuGeom.verts.push_back(subTriangle[i]);

/// <summary>
/// Generates a sub triangle with 2 sides (and 3 glm::vec3) and adds them to the 'cpuGeom' while also incrementing the number of sides created by 2.
/// </summary>
/// <param name="cpuGeom"> a GPU_Geometry. </param>
/// <param name="prevVec"> an initial glm::vec3 to make this sub triangle, a glm::vec3. </param>
/// <param name="sideLength"> the side length of this sub triangle, a float.</param>
/// <param name="angleOffSetForTopVec"> an angle offset for the 'top' point of the triangle (in degrees), a float.</param>
/// <param name="angleOffSetForRightVec"> an angle offset for the 'right' point of the triangle (in degrees), a float.</param>
/// <param name="numSidesMade"> a address to a counter for how many sides are created, an int.</param> 
void genSingleTriangle(CPU_Geometry& cpuGeom, glm::vec3 prevVec, float sideLength, float angleOffSetForTopVec, float angleOffSetForRightVec, int& numSidesMade)
{
	std::vector<glm::vec3> smTriangle(3);
	smTriangle[0] = glm::vec3(prevVec.x, prevVec.y, prevVec.z);
	// top point
	glm::vec3 top = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	rotateCCWAboutVec3(top, smTriangle[0], angleOffSetForTopVec);
	smTriangle[1] = top;
	// right point
	glm::vec3 right = glm::vec3(top.x + sideLength, top.y, top.z);
	rotateCCWAboutVec3(right, top, angleOffSetForRightVec);
	smTriangle[2] = right;

	numSidesMade += 2;	// increment by 2 as this creates 2 lines

	// add the points to the cpuGeom
	addToCpuGeomVerts(cpuGeom, smTriangle);
}

void gen60degreeGroup(CPU_Geometry& cpuGeom, glm::vec3 prevVec, float sideLength, float angleOffset1, int& numSidesMade)
{
	// Generate 2 group 60-degree
	std::vector<glm::vec3> group(2);
	group[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	rotateCCWAboutVec3(group[0], prevVec, angleOffset1);
	group[1] = glm::vec3(group[0].x + sideLength, group[0].y, group[0].z);
	rotateCCWAboutVec3(group[1], group[0], angleOffset1 + 60);
	numSidesMade += 2;

	addToCpuGeomVerts(cpuGeom, group);
}

void genTriangleGroup(CPU_Geometry& cpuGeom, glm::vec3 prevVec, float sideLength, float angleOffset1, int& numSidesMade)
{
	// generate 3-sub-triangle group
	std::vector <glm::vec3> subTriangleGroup1(2);
	std::vector <glm::vec3> subTriangleGroup2(2);
	std::vector <glm::vec3> subTriangleGroup3(2);

	subTriangleGroup1[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	rotateCCWAboutVec3(subTriangleGroup1[0], prevVec, angleOffset1);
	subTriangleGroup1[1] = glm::vec3(subTriangleGroup1[0].x + sideLength, subTriangleGroup1[0].y, subTriangleGroup1[0].z);
	rotateCCWAboutVec3(subTriangleGroup1[1], subTriangleGroup1[0], angleOffset1 - 120);

	addToCpuGeomVerts(cpuGeom, subTriangleGroup1);
	prevVec = cpuGeom.verts.back();

	subTriangleGroup2[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	rotateCCWAboutVec3(subTriangleGroup2[0], prevVec, angleOffset1 - 60);
	subTriangleGroup2[1] = glm::vec3(subTriangleGroup2[0].x + sideLength, subTriangleGroup2[0].y, subTriangleGroup2[0].z);
	rotateCCWAboutVec3(subTriangleGroup2[1], subTriangleGroup2[0], angleOffset1 - 180);

	addToCpuGeomVerts(cpuGeom, subTriangleGroup2);
	prevVec = cpuGeom.verts.back();

	subTriangleGroup3[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	rotateCCWAboutVec3(subTriangleGroup3[0], prevVec, angleOffset1 - 120);
	subTriangleGroup3[1] = glm::vec3(subTriangleGroup3[0].x + sideLength, subTriangleGroup3[0].y, subTriangleGroup3[0].z);
	rotateCCWAboutVec3(subTriangleGroup3[1], subTriangleGroup3[0], angleOffset1 - 240);
	numSidesMade += 6;

	addToCpuGeomVerts(cpuGeom, subTriangleGroup3);
}

void genSideSet(CPU_Geometry& cpuGeom, glm::vec3 prevVec, float sideLength, float angleOffset1, int& numSidesMade)
{
	// Generate single sub-triangle
	genSingleTriangle(cpuGeom, prevVec, sideLength, 60, -60, numSidesMade);
	prevVec = cpuGeom.verts.back();

	// Generate 60-degree group
	//gen60degreeGroup(cpuGeom, prevVec, sideLength, angleOffset1, );

}

void genKochSnowflakeHelper(CPU_Geometry& cpuGeom, glm::vec3 startingVec3, float sideLength, float angleOffSet, int& numSidesMade, int maxNumSides)
{
	if (maxNumSides == 12)	// Case 1: 1 sub-division
	{
		float angleOffset1 = 60.f;
		float angleOffset2 = -60.f;
		glm::vec3 prevVec = startingVec3;

		// Generate all the subtriangles for 1 subdivision
		for (;numSidesMade < maxNumSides;)
		{
			//std::vector<glm::vec3> smTriangle(3);
			//smTriangle[0] = glm::vec3(prevVec.x, prevVec.y, prevVec.z);
			//// top point
			//glm::vec3 top = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(top, smTriangle[0], angleOffset1);
			//smTriangle[1] = top;
			//// right point
			//glm::vec3 right = glm::vec3(top.x + sideLength, top.y, top.z);
			//rotateCCWAboutVec3(right, top, angleOffset2);
			//smTriangle[2] = right;

			// add the points to the cpuGeom
			//addToVertsCpuGeom(cpuGeom, smTriangle);

			// Generate single sub-triangle
			genSingleTriangle(cpuGeom, prevVec, sideLength, angleOffset1, angleOffset2, numSidesMade);

			prevVec = cpuGeom.verts.back();

			angleOffset1 -= 60.f;
			angleOffset2 -= 60.f;
		}
		numSidesMade -= 1;	// counting starting side twice

		// debugging
		std::cout << "\n1 sub-division with " << numSidesMade << " sides made (expected to be: 12)" << std::endl;
	}
	else if (maxNumSides == 48) // for 2 sub divisions
	{
		float angleOffsetFor60degreeGroup = 0;
		float angleOffsetForTriangleGroup = 120;
		glm::vec3 prevVec = startingVec3;

		// Generate sub-triangle 1
		genSingleTriangle(cpuGeom, prevVec, sideLength, 60, -60, numSidesMade);
		prevVec = cpuGeom.verts.back();

		for (; numSidesMade < maxNumSides;)
		//for (int i = 0; i < 2; i++)
		{
			// Generate 2 group 60-degree
			gen60degreeGroup(cpuGeom, prevVec, sideLength, angleOffsetFor60degreeGroup, numSidesMade);
			prevVec = cpuGeom.verts.back();

			// generate 3-sub-triangle group
			genTriangleGroup(cpuGeom, prevVec, sideLength, angleOffsetForTriangleGroup, numSidesMade);
			prevVec = cpuGeom.verts.back();

			//// Generate 2 group 60-degree
			angleOffsetFor60degreeGroup -= 60.f;	// Update the angle offset
			//gen60degreeGroup(cpuGeom, prevVec, sideLength, angleOffsetFor60degreeGroup, angleOffsetFor60degreeGroup + 60.f, numSidesMade);
			//prevVec = cpuGeom.verts.back();

			angleOffsetForTriangleGroup -= 60;		// Update the angle offset
			//genTriangleGroup(cpuGeom, prevVec, sideLength, angleOffsetForTriangleGroup, NULL, numSidesMade);
			//prevVec = cpuGeom.verts.back();



			// Generate 2 group 60-degree
			//std::vector<glm::vec3> group = {2, glm::vec3()};
			//group[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(group[0], prevVec, 0);
			//group[1] = glm::vec3(group[0].x + sideLength, group[0].y, group[0].z);
			//rotateCCWAboutVec3(group[1], group[0], 60);
			//numSidesMade += 2;
			//addToVertsCpuGeom(cpuGeom, group);

			// generate 3-sub-triangle group
			//std::vector <glm::vec3> subTriangleGroup1(2);
			//std::vector <glm::vec3> subTriangleGroup2(2);
			//std::vector <glm::vec3> subTriangleGroup3(2);

			//subTriangleGroup1[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(subTriangleGroup1[0], prevVec, 120);
			//subTriangleGroup1[1] = glm::vec3(subTriangleGroup1[0].x + sideLength, subTriangleGroup1[0].y, subTriangleGroup1[0].z);
			//rotateCCWAboutVec3(subTriangleGroup1[1], subTriangleGroup1[0], 0);

			//addToVertsCpuGeom(cpuGeom, subTriangleGroup1);
			//prevVec = cpuGeom.verts.back();

			//subTriangleGroup2[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(subTriangleGroup2[0], prevVec, 60);
			//subTriangleGroup2[1] = glm::vec3(subTriangleGroup2[0].x + sideLength, subTriangleGroup2[0].y, subTriangleGroup2[0].z);
			//rotateCCWAboutVec3(subTriangleGroup2[1], subTriangleGroup2[0], -60);

			//addToVertsCpuGeom(cpuGeom, subTriangleGroup2);
			//prevVec = cpuGeom.verts.back();

			//subTriangleGroup3[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(subTriangleGroup2[0], prevVec, 0);
			//subTriangleGroup3[1] = glm::vec3(subTriangleGroup3[0].x + sideLength, subTriangleGroup3[0].y, subTriangleGroup3[0].z);
			//rotateCCWAboutVec3(subTriangleGroup3[1], subTriangleGroup3[0], -120);
			//numSidesMade += 6;

			//addToVertsCpuGeom(cpuGeom, subTriangleGroup3);

			// Generate 2 group 60-degree
			//group = {2, glm::vec3()};
			//group[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(group[0], prevVec, -60);
			//group[1] = glm::vec3(group[0].x + sideLength, group[0].y, group[0].z);
			//rotateCCWAboutVec3(group[1], group[0], 0);
			//numSidesMade += 2;
			//addToVertsCpuGeom(cpuGeom, group);

			// generate 3-sub-triangle group
			//subTriangleGroup1 = { 2, glm::vec3() };
			//subTriangleGroup2 = { 2, glm::vec3() };
			//subTriangleGroup3 = { 2, glm::vec3() };

			//subTriangleGroup1[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(subTriangleGroup1[0], prevVec, 60);
			//subTriangleGroup1[1] = glm::vec3(subTriangleGroup1[0].x + sideLength, subTriangleGroup1[0].y, subTriangleGroup1[0].z);
			//rotateCCWAboutVec3(subTriangleGroup1[1], subTriangleGroup1[0], -60);

			//addToVertsCpuGeom(cpuGeom, subTriangleGroup1);
			//prevVec = cpuGeom.verts.back();

			//subTriangleGroup2[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(subTriangleGroup2[0], prevVec, 0);
			//subTriangleGroup2[1] = glm::vec3(subTriangleGroup2[0].x + sideLength, subTriangleGroup2[0].y, subTriangleGroup2[0].z);
			//rotateCCWAboutVec3(subTriangleGroup2[1], subTriangleGroup2[0], -120);

			//addToVertsCpuGeom(cpuGeom, subTriangleGroup2);
			//prevVec = cpuGeom.verts.back();

			//subTriangleGroup3[0] = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
			//rotateCCWAboutVec3(subTriangleGroup3[0], prevVec, -60);
			//subTriangleGroup3[1] = glm::vec3(subTriangleGroup3[0].x + sideLength, subTriangleGroup3[0].y, subTriangleGroup3[0].z);
			//rotateCCWAboutVec3(subTriangleGroup3[1], subTriangleGroup3[0], -180);

			//addToVertsCpuGeom(cpuGeom, subTriangleGroup3);
			//numSidesMade += 6;
		}
		numSidesMade -= 1;	// counting starting side twice

		// debugging
		std::cout << "\n1 sub-division with " << numSidesMade << " sides made (expected to be: " << 3 * pow(4, 2) << ")" << std::endl;
	}
	else // for all other sub divisions 
	{
		
		float angleOffsetFor60degreeGroup = 0;
		float angleOffsetForTriangleGroup = 120;
		float angleOffsetForSingleSubTri = 60;

		glm::vec3 prevVec = startingVec3;

		// Generate single sub-triangle
		genSingleTriangle(cpuGeom, prevVec, sideLength, 60, -60, numSidesMade);
		prevVec = cpuGeom.verts.back();

		// Generate 2 group 60-degree
		gen60degreeGroup(cpuGeom, prevVec, sideLength, angleOffsetFor60degreeGroup, numSidesMade);
		prevVec = cpuGeom.verts.back();

		// generate 3-sub-triangle group
		genTriangleGroup(cpuGeom, prevVec, sideLength, angleOffsetForTriangleGroup, numSidesMade);
		prevVec = cpuGeom.verts.back();

		angleOffsetFor60degreeGroup -= 60.f;	// Update the angle offset
		//for (; numSidesMade < maxNumSides;)
		for (int counter = 0; counter < 3; counter++)
		{
			if (counter + 1 == 3)
			{
				std::cout << "counter = " << counter;
			}

			// Generate the sides with only 1 'single' triangle
			for (int i = 0; i < 2; i++)
			{
				// Generate 60-degree group
				gen60degreeGroup(cpuGeom, prevVec, sideLength, angleOffsetFor60degreeGroup, numSidesMade);
				prevVec = cpuGeom.verts.back();
				angleOffsetFor60degreeGroup = -60 * counter;

				// Generate single sub triangle
				genSingleTriangle(cpuGeom, prevVec, sideLength, angleOffsetForSingleSubTri, angleOffsetForSingleSubTri - 120, numSidesMade);
				prevVec = cpuGeom.verts.back();
				angleOffsetForSingleSubTri += 60;
			}

			// Generate alternating group (single triangle + 60 degree group)
			//angleOffsetFor60degreeGroup = 60 - (60 * counter);
			angleOffsetFor60degreeGroup += 60;
			angleOffsetForTriangleGroup += (60 * (counter + 1));
			for (int i = 0; i < 3; i++)
			{
				// Generate 60-degree group
				gen60degreeGroup(cpuGeom, prevVec, sideLength, angleOffsetFor60degreeGroup, numSidesMade);
				prevVec = cpuGeom.verts.back();
				angleOffsetFor60degreeGroup -= 60;

				// generate 3-sub-triangle group
				genTriangleGroup(cpuGeom, prevVec, sideLength, angleOffsetForTriangleGroup, numSidesMade);
				prevVec = cpuGeom.verts.back();
				angleOffsetForTriangleGroup -= 60;
			}

			std::cout << "\nangleOffsetForTriangleGroup = " << angleOffsetForTriangleGroup
				<< " angleOffsetFor60degreeGroup = " << angleOffsetFor60degreeGroup << std::endl;

			angleOffsetForSingleSubTri = -(60 * counter);

		}
	}

	//// Generate 1st 'smaller' triangle
	//std::vector<glm::vec3> smTriangle(3);
	//smTriangle[0] = glm::vec3(startingVec3.x, startingVec3.y, startingVec3.z);
	//// top point
	//glm::vec3 top = glm::vec3(startingVec3.x + sideLength, startingVec3.y, startingVec3.z);
	//rotateCCWAboutVec3(top, smTriangle[0], 60);
	//smTriangle[1] = top;
	//// right point
	//glm::vec3 right = glm::vec3(top.x + sideLength, top.y, top.z);
	//rotateCCWAboutVec3(right, top, -60);
	//smTriangle[2] = right;
	//// add the points to the cpuGeom
	//addToVertsCpuGeom(cpuGeom, smTriangle);
	//numSidesMade = numSidesMade + 2;

	//// Generate 2nd 'smaller' triangle
	//smTriangle = {3, glm::vec3()};
	//glm::vec3 prevVec = cpuGeom.verts.back();
	//smTriangle[0] = glm::vec3(prevVec.x, prevVec.y, prevVec.z);
	//// top point
	//top = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	//rotateCCWAboutVec3(top, smTriangle[0], 0);
	//smTriangle[1] = top;
	//// right point
	//right = glm::vec3(top.x + sideLength, top.y, top.z);
	//rotateCCWAboutVec3(right, top, -120);
	//smTriangle[2] = right;
	//// add the points to the cpuGeom
	//addToVertsCpuGeom(cpuGeom, smTriangle);
	//numSidesMade = numSidesMade + 2;

	//// Generate 3rd 'smaller' triangle
	//smTriangle = {3, glm::vec3()};
	//prevVec = cpuGeom.verts.back();
	//smTriangle[0] = glm::vec3(prevVec.x, prevVec.y, prevVec.z);
	//// top point
	//top = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	//rotateCCWAboutVec3(top, smTriangle[0], -60);
	//smTriangle[1] = top;
	//// right point
	//right = glm::vec3(top.x + sideLength, top.y, top.z);
	//rotateCCWAboutVec3(right, top, -180);
	//smTriangle[2] = right;
	//// add the points to the cpuGeom
	//addToVertsCpuGeom(cpuGeom, smTriangle);
	//numSidesMade = numSidesMade + 2;

	//// Generate 4th 'smaller' triangle
	//smTriangle = {3, glm::vec3() };
	//prevVec = cpuGeom.verts.back();
	//smTriangle[0] = glm::vec3(prevVec.x, prevVec.y, prevVec.z);
	//// top point
	//top = glm::vec3(prevVec.x + sideLength, prevVec.y, prevVec.z);
	//rotateCCWAboutVec3(top, smTriangle[0], -120);
	//smTriangle[1] = top;
	//// right point
	//right = glm::vec3(top.x + sideLength, top.y, top.z);
	//rotateCCWAboutVec3(right, top, -240);
	//smTriangle[2] = right;
	//// add the points to the cpuGeom
	//addToVertsCpuGeom(cpuGeom, smTriangle);
	//numSidesMade = numSidesMade + 2;

}

void generateKochSnowflakeRecurr(CPU_Geometry& cpuGeom, glm::vec3 startingVec3, float sideLength, int currentIteration, const int numIterations)
{
	glm::vec3 v = startingVec3;
	v.x += sideLength;

	int totalNumberOfSides = 3 * pow(4, numIterations);
	int numSidesMade = 1;

	genKochSnowflakeHelper(cpuGeom, v, sideLength, 60, numSidesMade, totalNumberOfSides);
}





void genKochSnowflake(CPU_Geometry& cpuGeom, glm::vec3 leftV, glm::vec3 rightV, float subTriangleSideLength, float angleOffset, int currentIter, const int& numIterations)
{
	// debug
	//std::cout << "\nIteration: " << currentIter << std::endl;

	// Calculate the side length that v1 and v2 makes
	//float sideLength = abs(leftV.x - rightV.x); // (make abs?)

	//float subTriangleSideLength = sideLength;

	//std::cout << "\nside length = " << sideLength << std::endl;
	//std::cout << "side length created from vec3's: " << std::endl;
	//printVectorLocation(leftV);
	//printVectorLocation(rightV);
	//std::cout << "\nSub triangle side length = " << subTriangleSideLength << std::endl;

	// Calculate sub triangle's vec3's
	glm::vec3 bottomL = glm::vec3(leftV.x + subTriangleSideLength, leftV.y, leftV.z);
	rotateCCWAboutVec3(bottomL, leftV, angleOffset);
	glm::vec3 bottomR = glm::vec3(rightV.x - subTriangleSideLength, rightV.y, rightV.z);
	rotateCCWAboutVec3(bottomR, rightV, angleOffset);
	glm::vec3 top = glm::vec3(bottomL.x + subTriangleSideLength, bottomL.y, bottomL.z);
	rotateCCWAboutVec3(top, bottomL, angleOffset + 60);
	std::vector < glm::vec3> subPoints = { bottomL, top, bottomR };

	// Keep Generating
	if (currentIter < numIterations)
	{
		++currentIter;
		genKochSnowflake(cpuGeom, leftV, bottomL, subTriangleSideLength / 3.f, angleOffset, currentIter, numIterations);		// generate sub triangle on the side made from vec3's leftV and bottomL
		genKochSnowflake(cpuGeom, bottomL, top, subTriangleSideLength / 3.f, angleOffset + 60, currentIter, numIterations);		// generate sub triangle on the side made from vec3's bottomL and top
		genKochSnowflake(cpuGeom, top, bottomR, subTriangleSideLength / 3.f, angleOffset - 60, currentIter, numIterations);		// generate sub triangle on the side made from vec3's top and bottomR
		genKochSnowflake(cpuGeom, bottomR, rightV, subTriangleSideLength / 3.f, angleOffset, currentIter, numIterations);		// generate sub triangle on the side made from vec3's bottomR rightV
	}
	else
	{
		// add the vec3's
		//printVectorLocation(leftV);
		cpuGeom.verts.push_back(leftV);
		//std::cout << "pushed to cpuGeom[" << (cpuGeom.verts.size() - 1) << "]" << std::endl;
		addToCpuGeomVerts(cpuGeom, subPoints);
	}

}

void generateKochSnowflake(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	if (numIterations < 0)
	{
		return;
	}

	// Clear cpuGeom
	clearCPUGeom(cpuGeom);

	std::cout << "NOT IMPLMENTED YET\n";
	// I think the best way to do this one is to generate one side of the snowflake
	// then copy and rotate the cpoies to 'generate' the other sides.
	// this could make computation not too difficult, but i should try doing it with all 3 sides.

	//std::vector<glm::vec3> baseSide	= // store the base side of the initial triangle
	//{
	//		glm::vec3(-0.75f, 0.f, 0.f),
	//		glm::vec3(0.75, 0.f, 0.f)
	//};
	//for (int i = 0; i < baseSide.size(); i++)
		//cpuGeom.verts.push_back(baseSide[i]);

	std::vector<glm::vec3> baseTriangleVec3 =
	{
		//glm::vec3(1.f, 0.f, 0.f),		// bottom right point
		//glm::vec3(0.f, 1.f, 0.f),		// top point
		//glm::vec3(-1.f, 0.f, 0.f)		// bottom left point
		glm::vec3(0.5f, 0.5f, 0.f),		// top right point
		glm::vec3(0.f, -0.5f, 0.f),		// bottom point
		glm::vec3(-0.5f, 0.5f, 0.f)		// top left point
	};

	//baseTriangle[0] = (glm::vec3(-1.f, -1.f, 0.f));
	//baseTriangle[1] = (glm::vec3(0.f, 1.f, 0.f));
	//baseTriangle[2] = (glm::vec3(1.f, -1.f, 0.f));


	//cpuGeom.verts.push_back(baseTriangleVec3[2]);

	if (numIterations > 0)
	{
		float sideLength = (abs(baseTriangleVec3[2].x) + abs(baseTriangleVec3[0].x));

		for (int i = 0; i < numIterations; i++)
			sideLength = sideLength / 3.f;

		int currentIter = 1;
		genKochSnowflake(cpuGeom, baseTriangleVec3[2], baseTriangleVec3[0], 1 / 3.f, 0, currentIter, numIterations);		// top side of the base triangle
		//genKochSnowflake(cpuGeom, baseTriangleVec3[0], baseTriangleVec3[1], 1 / 3.f, -120, currentIter, numIterations);	// right side of the base triangle
//genKochSnowflake(cpuGeom, baseTriangleVec3[1], baseTriangleVec3[2], 1 / 3.f, 120, currentIter, numIterations);	// left side of the base triangle

		std::vector<glm::vec3> right = { baseTriangleVec3[0] };
		addToCpuGeomVerts(cpuGeom, right);

		//// Finding the Centroid of the base triangle
		//float centroidX = (baseTriangleVec3[0].x + baseTriangleVec3[1].x + baseTriangleVec3[2].x) / 3.f;
		//float centroidY = (baseTriangleVec3[0].y + baseTriangleVec3[1].y + baseTriangleVec3[2].y) / 3.f;
		//float centroidZ = (baseTriangleVec3[0].z + baseTriangleVec3[1].z + baseTriangleVec3[2].z) / 3.f;
		//glm::vec3 triangleCentroid = glm::vec3(centroidX, centroidY, centroidZ);

		//// Generate the rest of the sides by cloning the top side and rotating said vec3's.

		//// Generate right side by cloning the vec3's from the top side
		//std::vector<glm::vec3> rightSide(cpuGeom.verts.size() - 1);

		//for (int i = 0; i < rightSide.size(); i++)
		//	rightSide[i] = cpuGeom.verts.at(i);

		//// Rotate all cloned vec3's about the middle of the triangle by 120 degrees
		//for (int i = 0; i < rightSide.size(); i++)
		//	rotateCCWAboutVec3(rightSide[i], triangleCentroid, -120);

		//// Add the right side
		//addToCpuGeomVerts(cpuGeom, rightSide);

		//// Generate left side by cloning the vec3's from the top side
		//std::vector<glm::vec3> leftSide(cpuGeom.verts.size() - 1);
		//for (int i = 0; i < leftSide.size(); i++)
		//	leftSide[i] = cpuGeom.verts.at(i);

		//// Rotate all cloned vec3's about the middle of the triangle by 120 degrees
		//for (int i = 0; i < leftSide.size(); i++)
		//	rotateCCWAboutVec3(leftSide[i], triangleCentroid, 120);

		//// Add the left side
		//addToCpuGeomVerts(cpuGeom, leftSide);
	}
	else
	{
		// Case where number of sub-divisions is 0
		for (int i = 0; i < baseTriangleVec3.size(); i++)
		{
			cpuGeom.verts.push_back(baseTriangleVec3[i]);
			//printVectorLocation(baseTriangleVec3[i]);
		}
	}

		
	setRainbowCol(cpuGeom);
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

//
//
//
//
void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData)
{
	//std::cout << "NOT IMPLMENTED YET";
	std::cout << "\n---IN PROGRESS---\n";
	int numIter = 2;
	generateKochSnowflake(cpuGeom, gpuGeom, numIter);
	std::cout << "\nCreated snowflake with " << numIter << " iterations." << std::endl;

}

/// <summary>
/// Reverses a std::vector(glm::vec3) 'a' elements: swap a[0] with a[size-1], swap a[1] with a[size-2] ... swap a[mid -1] with a[mid+1].
/// </summary>
/// <param name="arr"> the std::vector glm::vec3 to be reversed. </param>
void reverse(std::vector <glm::vec3>& arr)
{
	for (int i = 0, j = arr.size() - 1; i < arr.size() / 2; i++, j--)
		if (i == j)	// when the two counters are equal -> reached the middle
			break;
		else
		{
			// Swap the elements
			glm::vec3 tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
		}
}

/// <summary>
/// Shifts the Dragon Curve to the origin from its center.
/// </summary>
/// <param name="cpuGeom"> a CPU_Geometry that contains the Dragon Curve.</param>
void shiftDragonCurveCenterToOrigin(CPU_Geometry& cpuGeom)
{
	float yMax = cpuGeom.verts.at(0).y;
	float yMin = cpuGeom.verts.at(0).y;
	// Find the maximum and minimum y in the curve
	for (int i = 0; i < cpuGeom.verts.size(); i++)
	{
		if (cpuGeom.verts.at(i).y > yMax)
			yMax = cpuGeom.verts.at(i).y;
		else if (cpuGeom.verts.at(i).y <= yMin)
			yMin = cpuGeom.verts.at(i).y;
	}

	float xMax = cpuGeom.verts.at(0).x;
	float xMin = cpuGeom.verts.at(0).x;

	// Find the maximum and minimum x in the curve
	for (int i = 1; i < cpuGeom.verts.size(); i++)
	{
		if (cpuGeom.verts.at(i).x > xMax)
			xMax = cpuGeom.verts.at(i).x;
		else if (cpuGeom.verts.at(i).x <= xMin)
			xMin = cpuGeom.verts.at(i).x;
	}

	// Find the x and y center of the Curve
	float xMidDiff = (xMax - xMin) / 2.f;
	float yMidDiff = (yMax - yMin) / 2.f;
	float xMid = xMax - xMidDiff;
	float yMid = yMax - yMidDiff;

	// Shift all points in the curve by -xMid and -yMid (curve center to the origin)
	for (int i = 0; i < cpuGeom.verts.size(); i++)
	{
		cpuGeom.verts.at(i).x = cpuGeom.verts.at(i).x - xMid;
		cpuGeom.verts.at(i).y = cpuGeom.verts.at(i).y - yMid;
	}

	// debugging
	//std::cout << "\nCurve center: (" << xMid << ", " << yMid << ")\n"
	//	<< "Printing translated points:" << std::endl;
	//for (int i = 0; i < cpuGeom.verts.size(); i++)
	//	printVectorLocation(cpuGeom.verts.at(i));
}

/// <summary>
/// Generates the Dragon Curve recursively.
/// </summary>
/// <param name="cpuGeom"> a CPU_Geometry that contains the Dragon Curve glm::vec3's. </param>
/// <param name="currentIteration"> an int that counts the current subdivision. </param>
/// <param name="numIterations"> an int, the total number of subdivisions desired. </param>
void genDragonCurveRecur(CPU_Geometry& cpuGeom, int currentIteration, int numIterations)
{
	// debugging
	//std::cout << "\n(Before starting iter: " << currentIteration << ") cpuGeom size = " << cpuGeom.verts.size() << std::endl;

	glm::vec3 pointOfRotation = cpuGeom.verts.back();	// vec3 to rotate about

	// rotate all glm::vec3's inside cpuGeom by 45 degrees about 'rotationVec3' (excluding vec3 of rotation)
	for (int i = 0; i < cpuGeom.verts.size() - 1; i++)
		rotateCCWAboutVec3(cpuGeom.verts.at(i), pointOfRotation, -45.f); // rotate clockwise

	// Clone the current dragon curve
	std::vector<glm::vec3> clonedCurve(cpuGeom.verts.size() - 1);		// minus 1 to not include the pointOfRotation
	for (int i = 0; i < clonedCurve.size(); i++)
		clonedCurve[i] = cpuGeom.verts.at(i);

	// Rotate the cloned dragon curve by 90 degrees about the 'pointOfRotation'
	for (int i = 0; i < clonedCurve.size(); i++)
		rotateCCWAboutVec3(clonedCurve[i], pointOfRotation, -90.f);		// rotate clockwise

	// reverse the cloned curve so it is drawn correctly in order
	reverse(clonedCurve);

	// Add the rotated cloned dragon curve to the cpuGeom
	addToCpuGeomVerts(cpuGeom, clonedCurve);

	// debugging
	//std::cout << "\n(After finishing iter: " << currentIteration << ") cpuGeom size = " << cpuGeom.verts.size() << std::endl;
	//std::cout << "\nVectors inside: " << std::endl;
	//for (int i = 0; i < cpuGeom.verts.size(); i++)
	//	printVectorLocation(cpuGeom.verts.at(i));

	// Keep generating the Dragon Curve
	if (currentIteration < numIterations)
	{
		++currentIteration;	// increment by 1
		genDragonCurveRecur(cpuGeom, currentIteration, numIterations);
	}
}

/// <summary>
/// Part 2.4 Part IV : Dragon Curve
/// Does the set up before generating the Dragon curve by calling 'genDragonCurveRecur(...)'
/// </summary>
/// <param name="cpuGeom"> a CPU_Geometry that will hold the Dragon Curve glm::vec3's.</param>
/// <param name="gpuGeom"> a GPU_Geometry. </param>
/// <param name="numIterations"> an int, the number of subdivisions to create. </param>
void genererateDragonCurve(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations)
{
	if (numIterations == -1)
	{
		std::cout << "\nCannot have negative subdivisions\n" << std::endl;
		return;
	}

	// Clear what's inside the cpuGeom
	clearCPUGeom(cpuGeom);

	// Starting/Initial 'curve'
	std::vector<glm::vec3> startingCurve(2);

	// Different levels/sizes of the starting curve, used so it can fit on the screen better for larger subdivisions
	if (numIterations == 0)
	{
		startingCurve =
		{
			glm::vec3(-0.75f, 0.f, 0.f),
			glm::vec3(0.75f, 0.f, 0.f)
		};
	}
	else if (numIterations == 1)
	{
		startingCurve =
		{
			glm::vec3((-0.5f / numIterations), 0.f, 0.f),
			glm::vec3((0.5f / numIterations), 0.f, 0.f)
		};
	}
	else if (numIterations < 15)	// Offers a smooth transition from subdivisions 2-14
	{
		startingCurve =
		{
			glm::vec3((-1.f / pow(numIterations, 2)), 0.f, 0.f),
			glm::vec3((1.f / pow(numIterations, 2)), 0.f, 0.f)
		};
	}
	else
	{
		// For larger subdivisions > 14
		startingCurve =
		{
			glm::vec3((-0.5f / pow(numIterations, 2)), 0.f, 0.f),
			glm::vec3((0.5f / pow(numIterations, 2)), 0.f, 0.f)
		};
	}

	// Add the current curve to the cpuGeom
	addToCpuGeomVerts(cpuGeom, startingCurve);

	// Generate the rest of the curve
	if (numIterations > 0)
		genDragonCurveRecur(cpuGeom, 1, numIterations);

	// Shift the generated curve
	shiftDragonCurveCenterToOrigin(cpuGeom);

	// Set the colors and gpuGeom
	setRainbowCol(cpuGeom);
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

/// <summary
/// Dragon Curve Option, interacts with the user via the command prompt, for the number of desired subdivisions.
/// </summary>
/// <param name="cpuGeom"> a CPU_Geometry that holds the Dragon Curve points, address. </param>
/// <param name="gpuGeom"> a GPU_Geometry, address. </param>
/// <param name="sceneData"> a data, address. </param>
void dragonCurveOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData)
{
//	genererateDragonCurve(cpuGeom, gpuGeom, numIter);
//	std::cout << "\nCreated dragon curve with " << numIter << " iterations." << std::endl;

	int numSubDiv = -1;
	while (numSubDiv < 0)
	{
		std::cout << "\n--Dragon Curve--\n How many subdivisions would you like? ";
		std::cin >> numSubDiv;
		if (numSubDiv < 0)
		{
			std::cout << "\nInvalid input..." << std::endl;
			continue;
		}
		sceneData.numSubDiv = numSubDiv;
		genererateDragonCurve(cpuGeom, gpuGeom, numSubDiv);
		std::cout << "\nDragon Curve with " << numSubDiv << " subdivisions created." << std::endl;
	}
}

/// <summary>
/// Sets the colours for vertices inside the cpuGeom, rainbow.
/// </summary>
/// <param name="cpuGeom"> the address of cpuGeom that contains the vertices.</param>
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

/// <summary>
/// Prints the location of the 'vec', x, y, z coordinates.
/// </summary>
/// <param name="vec"> the glm::vec3's location to be printed. </param>
/// <param name="vecNum"> an int, a number identifier, default is -1. </param>
void printVectorLocation(glm::vec3 vec)
{
	printVectorLocation(vec, -1);
}

/// <summary>
/// Prints the location of the 'vec', x, y, z coordinates.
/// </summary>
/// <param name="vec"> the glm::vec3's location to be printed. </param>
/// <param name="vecNum"> an int, a number identifier, default is -1. </param>
void printVectorLocation(glm::vec3 vec, int vecNum)
{
	std::cout << "\nvec #" << vecNum << " @ (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
}

/// <summary>
/// Clears what is inside the cpuGeom.
/// Clears the vertices and colours.
/// </summary>
/// <param name="cpuGeom"> the CPU_Geometry to be cleared.</param>
void clearCPUGeom(CPU_Geometry& cpuGeom)
{
	// Clear what is inside the cpuGeom
	cpuGeom.verts.clear();
	cpuGeom.cols.clear();
}

// Debug, delete after.
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
