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

//#include <glm/gtc/quaternion.hpp>
#include <cmath>;

struct data
{
	int fractalOption;
	int numSubDiv;
	CPU_Geometry& cpuGeom;
	GPU_Geometry& gpuGeom;
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
float calcHypotenuse(float squareSide);

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
		switchSceneCallBack(ShaderProgram& shader, data& d) : shader(shader), sceneData(d) {}

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
					//pythagorasOption(cpuGeom, gpuGeom);
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
					//pythagorasOption(cpuGeom, gpuGeom);
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
};

//int numSubDiv = -1;

int main()
{
	Log::debug("Starting main");

	// test/debug
	//glm::vec3 v1(2.f, 3.f, 0.f);
	//glm::vec3 v2(5.f, 1.f, 0.f);
	//glm::vec3 aboutV(-5.f, 4.f, 0.f);

	//std::cout << "\ncos(100)= " << cos(100) << std::endl;	// 100 is rads
	//std::cout << "\nsin(100)= " << sin(100) << std::endl;

	//rotateCCWAboutVec3(v1, aboutV, 100.f);	// expected = (-5.2, 11.1, 0)
	//rotateCCWAboutVec3(v2, aboutV, 100.f);	// expected = (-3.8, 14.4, 0)

	//std::cout << "\nrotated v1: " << std::endl;
	//printVectorLocation(v1, 1);

	//std::cout << "\nrotated v2: " << std::endl;
	//printVectorLocation(v2, 2);

	//std::cout << "\nAbout rotate vec: " << std::endl;
	//printVectorLocation(aboutV, 0);

	//glm::vec3 originalVec(1.0f, 0.0f, 0.0f); // Original vector (1, 0, 0)

	//float angle = glm::radians(90.0f); // Rotate by 90 degrees
	//glm::vec3 axis(0.0f, 0.0f, 1.0f); // Axis of rotation (z-axis)

	//// Create a quaternion representing the rotation
	//glm::quat quaternion = glm::angleAxis(angle, axis);

	//// Apply the quaternion rotation to the vector
	//glm::vec3 rotatedVec = quaternion * originalVec;
	//printVectorLocation(rotatedVec);

	// test/debug
	//glm::vec3 v1 = glm::vec3(1.0f, 0.f, 0.f);
	//printVectorLocation(v1);
	//std::cout << "Rotating around y-axis 45-degrees" << std::endl;
	//glm::vec3 rotatedV1 = rotateVec3(v1, 45.f, 2);
	//printVectorLocation(rotatedV1);
	//std::cout << "Rotating around x-axis 180-degrees" << std::endl;
	//rotatedV1 = rotateVec3(v1, 45.f, 0);
	//printVectorLocation(rotatedV1);
	//std::cout << "Rotating around z-axis 180-degrees" << std::endl;
	//rotatedV1 = rotateVec3(v1, 45.f, 2);
	//printVectorLocation(rotatedV1);

	//glm::vec3 point(1.0f, 0.0f, 0.0f);  // Example point (1, 0, 0)

	//// Define the rotation angle in radians (45 degrees counterclockwise)
	//float angle = glm::radians(45.0f);  // Convert degrees to radians

	//// Define the rotation matrix for rotation along the z-axis
	//glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));

	//// Apply the rotation to the point (convert to vec4 for matrix multiplication)
	//glm::vec4 rotatedPoint = rotationMatrix * glm::vec4(point, 1.0f);

	//// Output the rotated point (we cast vec4 back to vec3 to ignore the w component)
	//std::cout << "Rotated point: ("
	//	<< rotatedPoint.x << ", "
	//	<< rotatedPoint.y << ", "
	//	<< rotatedPoint.z << ")" << std::endl;


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

		data newData = {option, -1, cpuGeom, gpuGeom};

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
* @param glm::vec3& vec3ToRotate, the glm::vec3 to be rotated.
* @param glm::vec3 rotateAboutVec, the glm::vec3 to rotate about.
* @param float degree, the number of radians the glm::vec3 needs to be rotated.
* @param int axisOption, 0: rotate about x-axis, 1: rotate about y-axis, 2: rotate about z-axis
*/
//glm::vec3& rotateVec3(glm::vec3& vec3, float degree, int axisOption)
//{

	// first attempt....
	//float angle = glm::radians(degree);
	//glm::vec3 axis;
	//glm::quat quaternion;

	//switch (axisOption)
	//{
	//	case 0:	// rotate about x-axis
	//		axis = glm::vec3(1.0f, 0.f, 0.f);
	//		quaternion = glm::angleAxis(angle, axis);
	//		break;
	//	case 1:	// rotate about y-axis
	//		//glm::vec3 axis(0.f, 1.0f, 0.f);
	//		axis = glm::vec3(0.f, 1.0f, 0.f);
	//		quaternion = glm::angleAxis(angle, axis);
	//		break;
	//	case 2:	// rotate about z-axis
	//		axis = glm::vec3(0.f, 0.f, 1.f);
	//		quaternion = glm::angleAxis(angle, axis);
	//		break;
	//	default:
	//		std::cout << "\nInvalid axis of rotation\n" << std::endl;
	//		break;
	//}

	//vec3 = quaternion * vec3;

	//// If values are very small, round to zero
	//if (glm::abs(vec3.x) < 1e-7)
	//	vec3.x = 0.0f;
	//if (glm::abs(vec3.y) < 1e-7)
	//	vec3.y = 0.0f;
	//if (glm::abs(vec3.z) < 1e-7)
	//	vec3.z = 0.0f;

	//return vec3;
//}

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
	baseTriangle[0] = (glm::vec3(-0.5f, -0.5f, 0.f));
	baseTriangle[1] = (glm::vec3(0.f, 0.5f, 0.f));
	baseTriangle[2] = (glm::vec3(0.5f, -0.5f, 0.f));

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
	//while (numSubDiv < 0)
	//{
	//	std::cout << "\n--Pythagoras Tree--\n How many subdivisions would you like? ";
	//	std::cin >> numSubDiv;
	//	if (numSubDiv < 0)
	//	{
	//		std::cout << "\nInvalid input..." << std::endl;
	//		continue;
	//	}
	//	generatePythagorasTree(cpuGeom, gpuGeom, numSubDiv);
	//	std::cout << "\nPythagoras Tree with " << numSubDiv << " subdivisions created." << std::endl;
	//}
	generatePythagorasTree(window, cpuGeom, gpuGeom, 7);
}

/**
* Generates the right-hand square for squares on the "left side" of the base square (root square).
* Should only be called by <name> method.
*/
void generatePythagorasRecurrLeftRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side length at the same subdiv.

	// Calc. right square vec3's
	std::vector<glm::vec3> rightSq(4);

	// bottom left point
	rightSq[0] = rightVec;
	// bottom right point
	rightSq[1] = glm::vec3(rightVec.x + side, rightVec.y, 0.f);
	// top right point
	glm::vec3 bottomRight = rightSq[1];
	rightSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	// top left point
	glm::vec3 bottomLeft = rightSq[0];
	glm::vec3 topRight = rightSq[2];
	rightSq[3] = glm::vec3(bottomLeft.x, topRight.y, 0.f);

	// Rotate the right square's vec3s
	for (int i = 0; i < rightSq.size(); i++)
	{
		//std::cout << "\nBefore rotation vec:" << std::endl;
		rotateCCWAboutVec3(rightSq[i], rightVec, radianOffset);
		//std::cout << "After rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);
	}

	//std::cout << "\nprinting (right sq) vec3 inside cpuGeom..." << std::endl; 	 // debug
	for (int i = 0; i < rightSq.size(); i++)
	{
		cpuGeom.verts.push_back(rightSq[i]);
		//printVectorLocation(rightSq[i], i); 	 // debug
	}

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;
		generatePythagorasRecurrLeft(cpuGeom, rightSq[2], rightSq[1], side, currentIteration, numIterations, radianOffset - 90);
		generatePythagorasRecurrLeftRight(cpuGeom, rightSq[2], rightSq[1], side, currentIteration, numIterations, radianOffset - 90);
	}
}

void generatePythagorasRecurrLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	// debug
	//std::cout << "\n(LEFT) radian offset: " << radianOffset << std::endl;;
	//std::cout << "SUBDIVISION: " << currentIteration << " using these vec3's to calculate left/right squares: " << std::endl;
	//printVectorLocation(leftVec);
	//printVectorLocation(rightVec);
	//std::cout << std::endl;

	//float hypotenuse = abs(rightVec.x) + abs(leftVec.x);

	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side legnth at the same subdiv.

	// Calc. left Square vec3's
	std::vector< glm::vec3> leftSq(4);

	// bottom left point
	leftSq[0] = leftVec;
	// bottom right point
	leftSq[1] = glm::vec3(leftVec.x + side, leftVec.y, 0.f);
	// top right point
	glm::vec3 bottomRight = leftSq[1];
	leftSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	// top left point
	glm::vec3 topRight = leftSq[2];
	leftSq[3] = glm::vec3(leftVec.x, topRight.y, 0.f);


	// MOVED
	//// Calc. right square vec3's
	//std::vector<glm::vec3> rightSq(4);

	//// bottom left point
	//rightSq[0] = rightVec;
	//// bottom right point
	//rightSq[1] = glm::vec3(rightVec.x + side, rightVec.y, 0.f);
	//// top right point
	//bottomRight = rightSq[1];
	//rightSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	//// top left point
	//glm::vec3 bottomLeft = rightSq[0];
	//topRight = rightSq[2];
	//rightSq[3] = glm::vec3(bottomLeft.x, topRight.y, 0.f);

	// debug
	//std::cout << "\nvec3's before rotation: " << std::endl;
	//for (int i = 0; i < leftSq.size(); i++)
	//{
	//	printVectorLocation(leftSq[i], i);
	//}

	// Rotate the left and right square's vec3s
	for (int i = 0; i < leftSq.size(); i++)
	{
		//std::cout << "\nBefore rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);

		//if (leftSq[i] != leftVec)
		rotateCCWAboutVec3(leftSq[i], leftVec, radianOffset);
		//else if (rightSq[i] != rightVec)
		//rotateCCWAboutVec3(rightSq[i], rightVec, radianOffset);
		//std::cout << "After rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);
	}

	//std::cout << "\nprinting vec3 inside cpuGeom..." << std::endl; 	 // debug

	// Add the left and right square vec3's to the cpuGeom
	for (int i = 0; i < leftSq.size(); i++)
	{
		cpuGeom.verts.push_back(leftSq[i]);
		//printVectorLocation(leftSq[i], i); 	 // debug
	}

	//std::cout << "\nprinting (right sq) vec3 inside cpuGeom..." << std::endl; 	 // debug
	//for (int i = 0; i < rightSq.size(); i++)
	//{
	//	cpuGeom.verts.push_back(rightSq[i]);
	//	//printVectorLocation(rightSq[i], i); 	 // debug
	//}

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;
		generatePythagorasRecurrLeft(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);
		generatePythagorasRecurrLeftRight(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);
		//generatePythagorasRecurrRight(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);
		//generatePythagorasRecurr(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);
		//generatePythagorasRecurr(cpuGeom, rightSq[3], rightSq[2], side, currentIteration, numIterations, radianOffset);
	}
}

void generatePythagorasRecurrRightLeft(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side length at the same subdiv.

	// Calc. left Square vec3's
	std::vector< glm::vec3> leftSq(4);

	// bottom left point
	leftSq[0] = glm::vec3(leftVec.x - side, leftVec.y, 0.f);
	// bottom right point
	leftSq[1] = leftVec;
	// top right point
	glm::vec3 bottomRight = leftSq[1];
	leftSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	// top left point
	glm::vec3 topRight = leftSq[2];
	glm::vec3 bottomLeft = leftSq[0];
	leftSq[3] = glm::vec3(bottomLeft.x, topRight.y, 0.f);


	// Rotate the left and right square's vec3s
	for (int i = 0; i < leftSq.size(); i++)
	{
		rotateCCWAboutVec3(leftSq[i], leftVec, -radianOffset);
	}


	// Add the left and right square vec3's to the cpuGeom
	for (int i = 0; i < leftSq.size(); i++)
	{
		cpuGeom.verts.push_back(leftSq[i]);
	}

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;
		generatePythagorasRecurrRight(cpuGeom, leftSq[0], leftSq[3], side, currentIteration, numIterations, radianOffset - 90);
		generatePythagorasRecurrRightLeft(cpuGeom, leftSq[0], leftSq[3], side, currentIteration, numIterations, radianOffset - 90);
	}
}

void generatePythagorasRecurrRight(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side length at the same subdiv.

	//// Calc. left Square vec3's
	//std::vector< glm::vec3> leftSq(4);

	//// bottom left point
	//leftSq[0] = glm::vec3(leftVec.x - side, leftVec.y, 0.f);
	//// bottom right point
	//leftSq[1] = leftVec;
	//// top right point
	//glm::vec3 bottomRight = leftSq[1];
	//leftSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	//// top left point
	//glm::vec3 topRight = leftSq[2];
	//glm::vec3 bottomLeft = leftSq[0];
	//leftSq[3] = glm::vec3(bottomLeft.x, topRight.y, 0.f);


	// Calc. right square vec3's
	std::vector<glm::vec3> rightSq(4);

	// bottom left point
	rightSq[0] = glm::vec3(rightVec.x - side, rightVec.y, 0.f);
	// bottom right point
	rightSq[1] = rightVec;
	// top right point
	glm::vec3 bottomRight = rightSq[1];
	rightSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	// top left point
	glm::vec3 bottomLeft = rightSq[0];
	glm::vec3 topRight = rightSq[2];
	rightSq[3] = glm::vec3(bottomLeft.x, topRight.y, 0.f);

	// debug
	//std::cout << "\nvec3's before rotation: " << std::endl;
	//for (int i = 0; i < leftSq.size(); i++)
	//{
	//	printVectorLocation(leftSq[i], i);
	//}

	// Rotate the left and right square's vec3s
	for (int i = 0; i < rightSq.size(); i++)
	{
		//std::cout << "\nBefore rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);

		//if (leftSq[i] != leftVec)
		//rotateCCWAboutVec3(leftSq[i], leftVec, -radianOffset);
		//else if (rightSq[i] != rightVec)
		rotateCCWAboutVec3(rightSq[i], rightVec, -radianOffset);
		//std::cout << "After rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);
	}

	//std::cout << "\nprinting vec3 inside cpuGeom..." << std::endl; 	 // debug

	// Add the left and right square vec3's to the cpuGeom
	//for (int i = 0; i < leftSq.size(); i++)
	//{
	//	cpuGeom.verts.push_back(leftSq[i]);
	//	//printVectorLocation(leftSq[i], i); 	 // debug
	//}

	std::cout << "\nprinting (right sq) vec3 inside cpuGeom..." << std::endl; 	 // debug
	for (int i = 0; i < rightSq.size(); i++)
	{
		cpuGeom.verts.push_back(rightSq[i]);
		printVectorLocation(rightSq[i], i); 	 // debug
	}

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset = radianOffset + 45;	// increase the rotation offset by 45-degrees
		generatePythagorasRecurrRight(cpuGeom, rightSq[3], rightSq[2], side, currentIteration, numIterations, radianOffset);
		generatePythagorasRecurrRightLeft(cpuGeom, rightSq[3], rightSq[2], side, currentIteration, numIterations, radianOffset);

	}
}

void generatePythagorasRecurr(CPU_Geometry& cpuGeom, const glm::vec3& leftVec, const glm::vec3& rightVec, float hypotenuse, int currentIteration, const int numIterations, float radianOffset)
{
	// debug
	std::cout << "\nSUBDIVISION: " << currentIteration << " using these vec3's to calculate left/right squares: " << std::endl;
	printVectorLocation(leftVec);
	printVectorLocation(rightVec);
	std::cout << std::endl;

	//float hypotenuse = abs(rightVec.x) + abs(leftVec.x);

	float side = calcSideS(hypotenuse);	// both the left and right squares will have the same side legnth at the same subdiv.

	// Calc. left Square vec3's
	std::vector< glm::vec3> leftSq(4);

	// bottom left point
	leftSq[0] = leftVec;
	// bottom right point
	leftSq[1] = glm::vec3(leftVec.x + side, leftVec.y, 0.f);
	// top right point
	glm::vec3 bottomRight = leftSq[1];
	leftSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	// top left point
	glm::vec3 topRight = leftSq[2];
	leftSq[3] = glm::vec3(leftVec.x, topRight.y, 0.f);


	// Calc. right square vec3's
	std::vector<glm::vec3> rightSq(4);

	// bottom left point
	rightSq[0] = glm::vec3(rightVec.x - side, rightVec.y, 0.f);
	// bottom right point
	rightSq[1] = rightVec;
	// top right point
	bottomRight = rightSq[1];
	rightSq[2] = glm::vec3(bottomRight.x, bottomRight.y + side, 0.f);
	// top left point
	glm::vec3 bottomLeft = rightSq[0];
	topRight = rightSq[2];
	rightSq[3] = glm::vec3(bottomLeft.x, topRight.y, 0.f);

	// debug
	//std::cout << "\nvec3's before rotation: " << std::endl;
	//for (int i = 0; i < leftSq.size(); i++)
	//{
	//	printVectorLocation(leftSq[i], i);
	//}

	// Rotate the left and right square's vec3s
	for (int i = 0; i < leftSq.size(); i++)
	{
		//std::cout << "\nBefore rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);

		//if (leftSq[i] != leftVec)
			rotateCCWAboutVec3(leftSq[i], leftVec, radianOffset);
		//else if (rightSq[i] != rightVec)
			rotateCCWAboutVec3(rightSq[i], rightVec, -radianOffset);
		//std::cout << "After rotation vec:" << std::endl;
		//printVectorLocation(leftSq[i], i);
	}

	//std::cout << "\nprinting vec3 inside cpuGeom..." << std::endl; 	 // debug

	// Add the left and right square vec3's to the cpuGeom
	for (int i = 0; i < leftSq.size(); i++)
	{
		cpuGeom.verts.push_back(leftSq[i]);
		//printVectorLocation(leftSq[i], i); 	 // debug
	}

	std::cout << "\nprinting (right sq) vec3 inside cpuGeom..." << std::endl; 	 // debug
	for (int i = 0; i < rightSq.size(); i++)
	{
		cpuGeom.verts.push_back(rightSq[i]);
		printVectorLocation(rightSq[i], i); 	 // debug
	}

	if (currentIteration != numIterations)
	{
		currentIteration++;
		radianOffset += radianOffset;
		generatePythagorasRecurr(cpuGeom, leftSq[3], leftSq[2], side, currentIteration, numIterations, radianOffset);
		generatePythagorasRecurr(cpuGeom, rightSq[3], rightSq[2], side, currentIteration, numIterations, radianOffset);
	}

	// -------------- OLD
	//// leftVec is used to calculate the 'left side' square, rightVec is used to calculate the 'right side' square.

	//// Initialize the two squares, and set what we know about their points
	//std::vector<glm::vec3> leftSq(4);
	//leftSq[0] = leftVec;				// we know the left square will have this as a point
	//std::vector<glm::vec3> rightSq(4);
	//rightSq[0] = rightVec;				// we know the right square will have this as a point
	///* Note:
	//* [0] is reserved for initial point from parent square
	//* [1] is reserved for 'right' point
	//* [2] is reserved for 'top' point
	//* [3] is reserved for 'left' point
	//*/

	//// Find side 's' (side of square) of the 2 new squares to be created
	//float hypotenuse = abs(rightVec.x) + abs(leftVec.x);	// length of the hypotenuse made by 'leftVec' and 'rightVec'
	//float s = calcSideS(hypotenuse);

	//// Find left square points
	//float topPtY = calcHypotenuse(s) + leftVec.y;	// 'top' point y-coord will be 's' + 'leftVec.y'
	//float topPtX = leftVec.x;				// 'leftVec' and the left square's 'top' point share the same x coord.
	//leftSq[2] = glm::vec3(topPtX, topPtY, 0.f);

	//float sidePtY = topPtY / 2;				// the 'side' y-coord for the square will be half of the 'top' points y coord.
	//float sidePtXDiff = calcSideS(s);			// the 'side' x-ccord for the square will be the difference of length (s) of a smaller square with side lengths of 'sidePtY' val with 'leftVec.x'
	//float sideXRight = leftVec.x + sidePtXDiff;
	//float sideXLeft = leftVec.x - sidePtXDiff;
	//glm::vec3 rightSide = glm::vec3(sideXRight, sidePtY, 0.f);
	//glm::vec3 leftSide = glm::vec3(sideXLeft, sidePtY, 0.f);
	//leftSq[1] = rightSide;
	//leftSq[3] = leftSide;

	//for (int i = 0; i < leftSq.size(); i++)
	//{
	//	cpuGeom.verts.push_back(leftSq[i]);
	//	printVectorLocation(leftSq[i], i);
	//}

	//if (currentIteration == numIterations)
	//{
	//	std::cout << "done making left" << std::endl;
	//	return;
	//}
	//else
	//{
	//	++currentIteration;
	//	generatePythagorasRecurr(cpuGeom, leftSq[3], leftSq[2], currentIteration, numIterations);
	//}
	// Find right square points
	
}

// FIRST TRY - DOES NOT WORK SINCE THE SQUARES ROTATE AFTER EACH ITERATION...
//// debug
//std::cout << "using vec3s: " << std::endl;
//printVectorLocation(leftVec);
//printVectorLocation(rightVec);
//
//// leftVec is used to calculate the 'left side' square, rightVec is used to calculate the 'right side' square.
//
//// Initialize the two squares, and set what we know about their points
//std::vector<glm::vec3> leftSq(4);
//leftSq[0] = leftVec;				// we know the left square will have this as a point
//std::vector<glm::vec3> rightSq(4);
//rightSq[0] = rightVec;				// we know the right square will have this as a point
///* Note:
//* [0] is reserved for initial point from parent square
//* [1] is reserved for 'right' point
//* [2] is reserved for 'top' point
//* [3] is reserved for 'left' point
//*/
//
//// Find side 's' (side of square) of the 2 new squares to be created
//float hypotenuse = abs(rightVec.x) + abs(leftVec.x);	// length of the hypotenuse made by 'leftVec' and 'rightVec'
//float s = calcSideS(hypotenuse);
//
//// Find left square points
//float topPtY = calcHypotenuse(s) + leftVec.y;	// 'top' point y-coord will be 's' + 'leftVec.y'
//float topPtX = leftVec.x;				// 'leftVec' and the left square's 'top' point share the same x coord.
//leftSq[2] = glm::vec3(topPtX, topPtY, 0.f);
//
//float sidePtY = topPtY / 2;				// the 'side' y-coord for the square will be half of the 'top' points y coord.
//float sidePtXDiff = calcSideS(s);			// the 'side' x-ccord for the square will be the difference of length (s) of a smaller square with side lengths of 'sidePtY' val with 'leftVec.x'
//float sideXRight = leftVec.x + sidePtXDiff;
//float sideXLeft = leftVec.x - sidePtXDiff;
//glm::vec3 rightSide = glm::vec3(sideXRight, sidePtY, 0.f);
//glm::vec3 leftSide = glm::vec3(sideXLeft, sidePtY, 0.f);
//leftSq[1] = rightSide;
//leftSq[3] = leftSide;
//
//for (int i = 0; i < leftSq.size(); i++)
//{
//	cpuGeom.verts.push_back(leftSq[i]);
//	printVectorLocation(leftSq[i], i);
//}
//
//if (currentIteration == numIterations)
//{
//	std::cout << "done making left" << std::endl;
//	return;
//}
//else
//{
//	++currentIteration;
//	generatePythagorasRecurr(cpuGeom, leftSq[3], leftSq[2], currentIteration, numIterations);
//}
//// Find right square points

void generatePythagorasTree(Window& window, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom, const int numIterations)
{
	int height = window.getHeight();
	int width = window.getWidth();

	std::cout << "\nwindow height = " << height
		<< "\nwindow width = " << width << std::endl;

	// Create base square
	std::vector<glm::vec3> baseVec3(4);
	// OLD BASE
	//baseVec3[0] = glm::vec3(-0.25, -0.5, 0.f);	// bottom left
	//baseVec3[1] = glm::vec3(0.25, -0.5, 0.f);	// bottom right
	//baseVec3[2] = glm::vec3(0.25, 0.f,0.f);		// top right
	//baseVec3[3] = glm::vec3(-0.25, 0.f, 0.f);	// top left

	baseVec3[0] = glm::vec3(-0.25, -1.f, 0.f);	// bottom left
	baseVec3[1] = glm::vec3(0.25, -1.f, 0.f);	// bottom right
	baseVec3[2] = glm::vec3(0.25, -0.5, 0.f);		// top right
	baseVec3[3] = glm::vec3(-0.25, -0.5, 0.f);	// top left

	for (int i = 0; i < baseVec3.size(); i++)
	{
		cpuGeom.verts.push_back(baseVec3[i]);
		//printVectorLocation(baseVec3[i], i);
	}

	std::cout << "cpu geom size = " << cpuGeom.verts.size() << std::endl;

	// (debug)
	// first iteration test, left square 
	//std::vector<glm::vec3> sq1(4);

	//sq1[0] = glm::vec3(-0.25, 0.f, 0.f);	// bottom
	//sq1[1] = glm::vec3(0.f, 0.25, 0.f);		// right
	//sq1[2] = glm::vec3(-0.25, 0.5, 0.f);	// top
	//sq1[3] = glm::vec3(-0.5, 0.25, 0.f);	// left

	// right square
	//std::vector<glm::vec3> sq2(4);
	//sq2[0] = glm::vec3(0.25, 0.f, 0.f);	// bottom
	//sq2[1] = glm::vec3(0.5, 0.25, 0.f);		// right
	//sq2[2] = glm::vec3(0.25, 0.5, 0.f);	// top
	//sq2[3] = glm::vec3(0.f, 0.25, 0.f);	// left

	//for (int i = 0; i < sq1.size(); i++)
	//{
	//	cpuGeom.verts.push_back(sq1[i]);
	//	printVectorLocation(sq1[i], i);
	//}

	//std::cout << "iter1 sq right vec3:" << std::endl;
	//for (int i = 0; i < sq2.size(); i++)
	//{
	//	cpuGeom.verts.push_back(sq2[i]);
	//	printVectorLocation(sq2[i], i);
	//}
	//std::cout << "\n" << std::endl;

	float hypotenuse = abs(baseVec3[3].x) + abs(baseVec3[2].x);	// the hyptoenuse will the the length of the parent square

	//generatePythagorasRecurr(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);
	generatePythagorasRecurrLeft(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);
	generatePythagorasRecurrRight(cpuGeom, baseVec3[3], baseVec3[2], hypotenuse, 1, numIterations, 45.0f);


	setRainbowCol(cpuGeom);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

float calcSideS(float hypotenuse)
{
	return hypotenuse / (sqrt(2));
}

float calcHypotenuse(float squareSide)
{
	return sqrt((2 * pow(squareSide, 2)));
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



