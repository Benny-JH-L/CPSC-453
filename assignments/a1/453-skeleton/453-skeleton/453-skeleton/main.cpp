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

/// <summary>
/// contains:
/// int fractalOption >> 1: Sierpinski's Triangle | 2: Pythagoras Tree | 3: Koch Snowflake | 4: Dragon Curve
/// int numSubDiv
/// CPU_Geometry& cpuGeom
/// GPU_Geometry& gpuGeom
/// Window& window
/// </summary>
struct data
{
	int fractalOption;	// 1: Sierpinski's Triangle | 2: Pythagoras Tree | 3: Koch Snowflake | 4: Dragon Curve
	int numSubDiv;
	CPU_Geometry& cpuGeom;
	GPU_Geometry& gpuGeom;
	Window& window;
};


// Sierpinski's Triangle prototypes
void sierpinskiOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void generateSierpinskiTriangle(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations);
float calcHalfWayX(const glm::vec3& v1, const glm::vec3& v2);
float calcHalfWayY(const glm::vec3& v1, const glm::vec3& v2);

// Pythagoras Tree prototypes
void pythagorasOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void generatePythagorasTree(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations);
void generatePythagorasRecurr(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);
float calcSideS(float hypotenuse);

void generatePythagorasRecurrLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);
void generatePythagorasRecurrLeftRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);

void generatePythagorasRecurrRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);
void generatePythagorasRecurrRightLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset);

// Koch Snowflake prototypes
void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void generateKochSnowflake(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations);

// Dragon Curve prototypes
void dragonCurveOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData);
void genererateDragonCurve(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, int numIterations);

// Other prototypes
void drawFractal(int option, const CPU_Geometry& cpuGeom);
void rotateCCWAboutVec3(glm::vec3& vec3ToRotate, const glm::vec3 rotateAboutVec, const float angleOfRotation);
void clearCPUGeom(CPU_Geometry& cpuGeom);
void setRainbowCol(CPU_Geometry& cpuGeom);

// Debugging prototypes
void printVectorLocation(glm::vec3 vec, int vecNum);
void printVectorLocation(glm::vec3 vec);

/// <summary>
/// My call back class used to increase/decrease subdivisions and swap (left or right) between fractal's while interacting with the window.
/// </summary>
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
				generatePythagorasTree(cpuGeom, gpuGeom, subDiv);
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
				generatePythagorasTree(cpuGeom, gpuGeom, subDiv);
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
					generatePythagorasTree(cpuGeom, gpuGeom, subDiv);
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
					generatePythagorasTree(cpuGeom, gpuGeom, subDiv);
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
				pythagorasOption(cpuGeom, gpuGeom, newData);
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
					<< "---Additionally at the Window you use the following keys to:\n"
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
		//printVectorLocation(arr[i]);
		//std::cout << "pushed to cpuGeom[" << cpuGeom.verts.size() - 1 << "]" << std::endl;
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
			glDrawArrays(GL_LINE_LOOP, 0, cpuGeom.verts.size());
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

/// <summary>
/// Recursively calls itself to find Sierpinski's Triangle's at sub-divisions/iterations.
/// </summary>
/// <param name="cpuGeom"> a reference to a CPU_Geometry.</param>
/// <param name="triangleVec3"> a std::vector of type glm::vec3 that contains 3 vec3 'vectors' that make up a triangle.</param>
/// <param name="currentIteration"> the current iteration, or sub-division, to be calculated.</param>
/// <param name="numIterations"> the total number of iterations, or sub-divisions, to be calculate.</param>
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
		{
			cpuGeom.verts.push_back(subTriangle1[i]);
			//cpuGeom.cols.push_back(glm::vec3(1.f / currentIteration, 1.f / i, 1.f));
		}
		for (int i = 0; i < subTriangle2.size(); i++)
		{
			cpuGeom.verts.push_back(subTriangle2[i]);
			//cpuGeom.cols.push_back(glm::vec3(1.f, 1.f / currentIteration, 1.f / i));
		}
		for (int i = 0; i < subTriangle3.size(); i++)
		{
			cpuGeom.verts.push_back(subTriangle3[i]);
			//cpuGeom.cols.push_back(glm::vec3(1.f / i, 1.f, 1.f / currentIteration));
		}
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
	if (numIterations < 0)
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

	if (numIterations == 0)
	{
		cpuGeom.verts.push_back(baseTriangle[0]);
		cpuGeom.verts.push_back(baseTriangle[1]);
		cpuGeom.verts.push_back(baseTriangle[2]);
		setRainbowCol(cpuGeom);		// set to rainbow colour
	}
	else
	{
		// Generate the fractal
		int currentIteration = 1;
		generateSierpinskiRecurr(cpuGeom, baseTriangle, currentIteration, numIterations);

		// Set the colours of the triangles
		int start = 0, cpuSize = cpuGeom.verts.size();
		for (int i = 0, j = 1, k = 1; start < cpuSize * (1/3.f); i++, start++, j++)
		{
			cpuGeom.cols.push_back(glm::vec3(1.f / i, 1.f / k, 1.f / j));
		}
		for (int i = 0, j = 2, k = 2; start < cpuSize * (2 / 3.f); i++, start++, j++)
		{
			cpuGeom.cols.push_back(glm::vec3(1.f / j, 1.f / i, 1.f / k));
		}
		for (int i = 0, j = 3, k = 3; start < cpuSize; i++, start++, j++)
		{
			cpuGeom.cols.push_back(glm::vec3(1.f / k, 1.f / j, 1.f / i));
		}
	}

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
/// <param name="cpuGeom"> a CPU_Geometry that holds the fractal's glm::vec's, address.</param>
/// <param name="gpuGeom"> a GPU_Geometry, address.</param>
/// <param name="sceneData"> a data, address.</param>
void pythagorasOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData)
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
		generatePythagorasTree(cpuGeom, gpuGeom, numSubDiv);
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

	// Add colours
	for (int i = 0; i < rightSq.size(); i++)
		cpuGeom.cols.push_back(glm::vec3(1.f / (i + currentIteration), 1.f, 1.f));

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

	// Add colours
	for (int i = 0; i < leftSq.size(); i++)
		cpuGeom.cols.push_back(glm::vec3(1.f / (i + currentIteration), 1.f, 1.f));

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

	// Add colours
	for (int i = 0; i < leftSq.size(); i++)
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f / (i + currentIteration)));
	
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

	// Add colours
	for (int i = 0; i < rightSq.size(); i++)
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f / (i + currentIteration)));

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
/// <param name="cpuGeom"></param>
/// <param name="gpuGeom"></param>
/// <param name="numIterations"></param>
void generatePythagorasTree(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	if (numIterations < 0)
	{
		std::cout << "\nCannot have negative subdivisions\n" << std::endl;
		return;
	}

	// Clear what is inside the cpuGeom
	clearCPUGeom(cpuGeom);

	// Create base square
	std::vector<glm::vec3> baseVec3(4);

	baseVec3[0] = glm::vec3(-0.125, -0.5f, 0.f);	// bottom left
	baseVec3[1] = glm::vec3(0.125, -0.5f, 0.f);		// bottom right
	baseVec3[2] = glm::vec3(0.125, -0.25, 0.f);		// top right
	baseVec3[3] = glm::vec3(-0.125, -0.25, 0.f);	// top left

	// Add the base (root) square to the cpuGeom
	for (int i = 0; i < baseVec3.size(); i++)
		cpuGeom.verts.push_back(baseVec3[i]);

	// Colour for the root square
	cpuGeom.cols.push_back(glm::vec3(0.5f, 0.5f, 1.f));
	cpuGeom.cols.push_back(glm::vec3(0.5f, 0.5f, 1.f));
	cpuGeom.cols.push_back(glm::vec3(0.5f, 0.5f, 1.f));
	cpuGeom.cols.push_back(glm::vec3(0.5f, 0.5f, 1.f));

	float hypotenuse = abs(baseVec3[3].x) + abs(baseVec3[2].x);	// the hypotenuse will the the length of the parent square

	if (numIterations > 0)
	{
		// Generate sub divisions if the user input is > 0
		generatePythagorasRecurrLeft(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);
		generatePythagorasRecurrRight(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);
	}

	// Add the colours and vec3's to the gpuGeom
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

}

/// <summary>
/// Calculates the side 's' of a square using the hypotenuse (side length of its 'parent' square).
/// </summary>
/// <param name="hypotenuse"> the side length of its parent square.</param>
/// <returns>the calculated side, a float.</returns>
float calcSideS(float hypotenuse)
{
	return hypotenuse / (sqrt(2));
}

void genKochSnowflakeTop(CPU_Geometry& cpuGeom, glm::vec3 leftV, glm::vec3 rightV, float subTriangleSideLength, float angleOffset, int currentIter, const int& numIterations)
{
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
		genKochSnowflakeTop(cpuGeom, leftV, bottomL, subTriangleSideLength / 3.f, angleOffset, currentIter, numIterations);			// generate sub triangle on the side made from vec3's leftV and bottomL
		genKochSnowflakeTop(cpuGeom, bottomL, top, subTriangleSideLength / 3.f, angleOffset + 60, currentIter, numIterations);		// generate sub triangle on the side made from vec3's bottomL and top
		genKochSnowflakeTop(cpuGeom, top, bottomR, subTriangleSideLength / 3.f, angleOffset - 60, currentIter, numIterations);		// generate sub triangle on the side made from vec3's top and bottomR
		genKochSnowflakeTop(cpuGeom, bottomR, rightV, subTriangleSideLength / 3.f, angleOffset, currentIter, numIterations);		// generate sub triangle on the side made from vec3's bottomR rightV
	}
	else
	{
		// Add the 'vec3's to the cpuGeom
		cpuGeom.verts.push_back(leftV);
		addToCpuGeomVerts(cpuGeom, subPoints);
	}
}

/// <summary>
/// 2.3 Part III : Koch Snowflake.
/// Generates the Koch Snowflake.
/// </summary>
/// <param name="cpuGeom"> a reference to a CPU_Geometry to store the fractal.</param>
/// <param name="gpuGeom"> a reference to a CPU_Geometry.</param>
/// <param name="numIterations"> an int, the desired number of subdivisions.</param>
void generateKochSnowflake(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	if (numIterations < 0)
	{
		std::cout << "\nCannot have negative subdivisions\n" << std::endl;
		return;
	}

	// Clear cpuGeom
	clearCPUGeom(cpuGeom);

	std::vector<glm::vec3> baseTriangleVec3 =
	{
		glm::vec3(0.5f, 0.289f, 0.f),		// top right point
		glm::vec3(0.f, -0.577f, 0.f),		// bottom point
		glm::vec3(-0.5f, 0.289f, 0.f)		// top left point
	};

	if (numIterations > 0)
	{
		float sideLength = (abs(baseTriangleVec3[2].x) + abs(baseTriangleVec3[0].x));
		int currentIter = 1;

		// Generate top side
		genKochSnowflakeTop(cpuGeom, baseTriangleVec3[2], baseTriangleVec3[0], 1 / 3.f, 0, currentIter, numIterations);		// top side of the base triangle
		cpuGeom.verts.push_back(baseTriangleVec3[0]);	// Add the top right point of the base triangle

		// Generate right side
		int sizeBeforeRightSideCalc = cpuGeom.verts.size();
		glm::vec3 ghostVec = glm::vec3(baseTriangleVec3[0].x + sideLength, baseTriangleVec3[0].y, baseTriangleVec3[0].z);
		// Calculate another top side of the snowflake but from the top right point and ghostVec
		genKochSnowflakeTop(cpuGeom, baseTriangleVec3[0], ghostVec, 1 / 3.f, 0, currentIter, numIterations);			

		// rotate vec3's from sizeBeforeRightSideCalc to cpuGeom.size() by 120 degrees
		for (int i = sizeBeforeRightSideCalc; i < cpuGeom.verts.size(); i++)
			rotateCCWAboutVec3(cpuGeom.verts.at(i), baseTriangleVec3[0], -120);	// rotate clock wise about the top right point of the base triangle
		cpuGeom.verts.push_back(baseTriangleVec3[1]);	// Add the bottom point of the base triangle


		// Generate left side
		int sizeBeforeLeftSideCalc = cpuGeom.verts.size();
		ghostVec = glm::vec3(baseTriangleVec3[2].x - sideLength, baseTriangleVec3[2].y, baseTriangleVec3[2].z);
		genKochSnowflakeTop(cpuGeom, ghostVec, baseTriangleVec3[2], 1 / 3.f, 0, currentIter, numIterations);	// top side of the base triangle

		// rotate vec3's from sizeBeforeRightSideCalc to cpuGeom.size() by 120 degrees
		for (int i = sizeBeforeLeftSideCalc; i < cpuGeom.verts.size(); i++)
			rotateCCWAboutVec3(cpuGeom.verts.at(i), baseTriangleVec3[2], 120);	// rotate counter clock wise about the top left point of the base triangle
		cpuGeom.verts.push_back(baseTriangleVec3[2]);	// Add the top left point of the base triangle

	}
	else
	{
		// Case where number of sub-divisions is 0
		for (int i = 0; i < baseTriangleVec3.size(); i++)
		{
			cpuGeom.verts.push_back(baseTriangleVec3[i]);
			//printVectorLocation(baseTriangleVec3[i]);	// debugging
		}
	}

	// Set colours and gpuGeom stuff
	setRainbowCol(cpuGeom);
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

/// <summary>
/// Koch Snowflake Option, interacts with the user via the command prompt, for the number of desired subdivisions.
/// </summary>
/// <param name="cpuGeom"> a CPU_Geometry that holds the fractal's glm::vec's, reference.</param>
/// <param name="gpuGeom"> a GPU_Geometry, reference.</param>
/// <param name="sceneData"> a data, reference.</param>
void snowflakeOption(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, data& sceneData)
{
	int numSubDiv = -1;
	while (numSubDiv < 0)
	{
		std::cout << "\n--Koch Snowflake--\n How many subdivisions would you like? ";
		std::cin >> numSubDiv;
		if (numSubDiv < 0)
		{
			std::cout << "\nInvalid input..." << std::endl;
			continue;
		}
		sceneData.numSubDiv = numSubDiv;
		generateKochSnowflake(cpuGeom, gpuGeom, numSubDiv);
		std::cout << "\nKoch Snowflake " << numSubDiv << " subdivisions created." << std::endl;
	}
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

	// Set the colors and gpuGeom stuff
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
