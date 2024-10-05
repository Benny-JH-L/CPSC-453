#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/gtc/type_ptr.hpp>	// so i can use the "glm::value_ptr(...)"

using namespace std;
using namespace glm;
const static double piApprox = atan(1) * 4;	// pi approximation

// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject
{
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		theta(0),
		scale(1),
		transformationMatrix(1.0f) // This constructor sets it as the identity matrix
	{}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position = glm::vec3(0.f, 0.f, 1.f);	// initial position
	float theta; // Object's rotation
	// Alternatively, you could represent rotation via a normalized heading vec:
	// glm::vec3 heading;
	float scale; // Or, alternatively, a glm::vec2 scale;
	glm::mat4 transformationMatrix = mat4(1.0f);
	glm::mat4 transformationTexMatrix;	// not needed (DELETE)
};

struct GameData
{
	GameData(GameObject& s, GameObject& d0, GameObject& d1, GameObject& d2, GameObject& d3) :
		ship(s),
		d0(d0),
		d1(d1),
		d2(d2),
		d3(d3)
	{}

	GameObject& ship;	// Ship object
	// Diamond objects
	GameObject& d0;
	GameObject& d1;
	GameObject& d2;
	GameObject& d3;
	glm::vec3 previousMouseLoc = glm::vec3(0.f, 0.f, -1.f);	// initially previous is not existant
	glm::vec3 currMouseLoc;
};

void rotateAboutObjCenter(GameObject& obj, float degreeOfRotation);
double calcAngle(glm::vec3 initialV3, glm::vec3 finalV3);
void drawGameObject(ShaderProgram& shader, GameObject& obj);
void setGpuGeom(GameObject& obj);
void translateObj(GameObject& obj, double deltaX, double deltaY);
void uniformScaleObj(GameObject& obj, float scaleVal);
void scaleObj(GameObject& obj, float scaleX, float scaleY);

// Debug
void printVec4Pos(glm::vec4 vec, int vecNum);
void printVec4Pos(glm::vec4 vec);

class MyCallbacks : public CallbackInterface
{

public:
	MyCallbacks(ShaderProgram& shader, GameData& data) :
		shader(shader),
		gameData(data)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		std::cout << "key pressed " << std::endl;
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			shader.recompile();
		}
		// TEST
		else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			glm::mat4 identity = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
			float angle = glm::radians(22.5f);		// angle of rotation (converts degree to radians)
			glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
			glm::mat4 rotateMatrix = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix with the 12-degrees of freedom filled out.
			gameData.ship.transformationMatrix = rotateMatrix * gameData.ship.transformationMatrix;
			drawGameObject(shader, gameData.ship);
		}
		else if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			// for now rotate SHIP as a test. ( this is the opposite code of the above test of rotating 22.5 CCW), this rotates Clock wise 22.5 degrees
			rotateAboutObjCenter(gameData.ship, -22.5f);
			drawGameObject(shader, gameData.ship);
		}

		if (action == GLFW_PRESS)
		{
			counter++;
			std::cout << "counter = " << counter << std::endl;
		}
		 
	}

	// THERE IS A PROBLEM WHERE THE SHIP DISAPEARS, -> current theory is: is due to -nand angle (refer to photos)'
	// and i need to figure a way to go negative rotations, calculated with the mouse input (look at the y values and see if one is bigger than the other... def with y values prolly.

	// need to figure out how to point the ships nose to the mouse pointer (prolly need another variable for it that points somwhere initialy and moves to mouse location)
	// ^ MAY fix the issues descrbed above...
	virtual void cursorPosCallback(double xpos, double ypos)
	{
		std::cout << "\nMouse pos: (" << xpos << ", " << ypos << ")" << std::endl;
		gameData.currMouseLoc = glm::vec3(xpos, ypos, 0.f);

		if (gameData.previousMouseLoc != glm::vec3(0.f, 0.f, -1.f))
		{
			double angle = calcAngle(gameData.previousMouseLoc, gameData.currMouseLoc);
			cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;
			// for now rotate as a test.
			angle = angle * (180.f / piApprox);
			rotateAboutObjCenter(gameData.ship, angle);
			drawGameObject(shader, gameData.ship);
		}

		gameData.previousMouseLoc = gameData.currMouseLoc;	// set the previous mouse location
	}

private:
	ShaderProgram& shader;
	GameData& gameData;
	int counter = 1;
};


CPU_Geometry shipGeom(float width, float height)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	CPU_Geometry retGeom;
	// vertices for the spaceship quad
	//retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(-halfWidth, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth, halfHeight, 0.f));

	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));
	

	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

CPU_Geometry diamondGeom(float width, float height)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	CPU_Geometry retGeom;
	// vertices for the diamond quad (test)
	//float a = -0.25;
	//retGeom.verts.push_back(glm::vec3(-halfWidth + a, halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(-halfWidth + a, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth + a, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(-halfWidth + a, halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth + a, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth + a, halfHeight, 0.f));

	retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth, -halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth, halfHeight, 0.f));

	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.
	/*
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));
	*/

	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));

	return retGeom;
}

// for now the start transformation will be done like this, until i set up the uniforms and what now.
//void translate(GameObject& obj, float val1, float val2)
//{
//	for (int i = 0; i < obj.cgeom.verts.size(); i++)
//	{
//		glm::vec3& vecToTranslate = obj.cgeom.verts.at(i);
//		vecToTranslate.x = vecToTranslate.x + val1;
//		vecToTranslate.y = vecToTranslate.y + val2;
//	}
//}

void drawGameObject(ShaderProgram& shader, GameObject& obj)
{
	//setGpuGeom(obj);
	shader.use();

	shader.setMat4Transform("transformationMatrix", obj.transformationMatrix);
	shader.setMat4TextureTransform("transformationMatrixTexture", obj.transformationTexMatrix);
	
	obj.ggeom.bind();
	obj.texture.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	obj.texture.unbind();
}

void setGpuGeom(GameObject& obj)
{
	obj.ggeom.setVerts(obj.cgeom.verts);
	obj.ggeom.setTexCoords(obj.cgeom.texCoords);
}

int main() {
	Log::debug("Starting main");

	// debug
	double angle = calcAngle(glm::vec3(9.f, 2.f, 1.f), glm::vec3(2.f, 6.f, 1.f)); // expected value is 59.0362 degrees (returns in rads)
	cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f/piApprox)  << endl;

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	
	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	GameObject ship("textures/ship.png", GL_NEAREST);
	GameObject d0("textures/diamond.png", GL_NEAREST);
	GameObject d1("textures/diamond.png", GL_NEAREST);
	GameObject d2("textures/diamond.png", GL_NEAREST);
	GameObject d3("textures/diamond.png", GL_NEAREST);

	// CALLBACKS
	GameData newData = { ship, d0, d1, d2, d3 };
	window.setCallbacks(std::make_shared<MyCallbacks>(shader, newData)); // can also update callbacks to new ones

	// Create ship cpuGeom
	ship.cgeom = shipGeom(0.18f, 0.12f);
	// Create Diamonds cpuGeom
	d0.cgeom = diamondGeom(0.14f, 0.14f);
	d1.cgeom = diamondGeom(0.14f, 0.14f);
	d2.cgeom = diamondGeom(0.14f, 0.14f);
	d3.cgeom = diamondGeom(0.14f, 0.14f);
	// Put the diamonds to their starting locations
	scaleObj(ship, 0.09f, 0.06f);
	//translateObj(ship, 0.f, 0.f);	// Center of the screen
	translateObj(d0, -1.f, -1.f);	// bottom left
	translateObj(d1, 1.f, 1.f);	// top right
	translateObj(d2, -1.f, 1.f);	// top left
	translateObj(d3, 1.f, -1.f);	// bottom right
	// Set positions (already set in the struct
	//ship.position = glm::vec3(0.f, 0.f, 0.f);
	//d0.position = glm::vec3(-0.5f, -0.5f, 0.f);
	//d1.position = glm::vec3(0.5f, 0.5f, 0.f);
	//d2.position = glm::vec3(-0.5f, 0.5f, 0.f);
	//d3.position = glm::vec3(0.5f, -0.5f, 0.f);

	// Set gpu geoms
	//ship.ggeom.setVerts(ship.cgeom.verts);
	//ship.ggeom.setTexCoords(ship.cgeom.texCoords);
	setGpuGeom(ship);

	//d0.ggeom.setVerts(d0.cgeom.verts);
	//d0.ggeom.setTexCoords(d0.cgeom.texCoords);
	setGpuGeom(d0);
	//setGpuGeom(d1);
	//setGpuGeom(d2);
	//setGpuGeom(d3);

	// Set initial matrices (DO NOT DO THIS - DELETE)
	//glm::mat4 identity1 = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
	//float angle1 = glm::radians(90.f);		// angle of rotation (converts degree to radians)
	//glm::vec3 axisOfRotation1 = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
	//glm::mat4 noTransform = glm::rotate(identity1, angle1, axisOfRotation1);	// transformation matrix with the 12-degrees of freedom filled out.
	//ship.transformationMatrix = noTransform;
	//ship.transformationTexMatrix = noTransform;
	//d0.transformationMatrix = noTransform;
	//d0.transformationTexMatrix = noTransform;
	//d1.transformationMatrix = noTransform;
	//d1.transformationTexMatrix = noTransform;
	//d2.transformationMatrix = noTransform;
	//d2.transformationTexMatrix = noTransform;
	//d3.transformationMatrix = noTransform;
	//d3.transformationTexMatrix = noTransform;

	// RENDER LOOP
	while (!window.shouldClose()) {
		int score;
		glfwPollEvents();

		shader.use();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// note to self: if i have this in between drawing objects, it will remove all previous ones.

		// Draw ship
		//ship.ggeom.bind();
		//ship.texture.bind();
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//ship.texture.unbind();
		drawGameObject(shader, ship);

		// Draw Diamonds
		//shader.use();
		//d0.ggeom.bind();
		//d0.texture.bind();
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//d0.texture.unbind();
		drawGameObject(shader, d0);
		//drawGameObject(shader, d1);
		//drawGameObject(shader, d2);
		//drawGameObject(shader, d3);

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool*)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("Score: %d", 0); // Second parameter gets passed into "%d"

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

/// <summary>
/// Returns the angle in radians
/// </summary>
/// <param name="initialV3"></param>
/// <param name="finalV3"></param>
/// <returns></returns>
double calcAngle(glm::vec3 initialV3, glm::vec3 finalV3)
{
	//float dotProuct = (initialV3.x * finalV3.x) + (initialV3.y * finalV3.y) + (initialV3.z * finalV3.z);
	double dotProuct = (initialV3.x * finalV3.x) + (initialV3.y * finalV3.y);

	float squareInitialX = initialV3.x * initialV3.x;
	float squareInitialY = initialV3.y * initialV3.y;
	//float squareInitialZ = initialV3.z * initialV3.z;

	float squareFinalX = finalV3.x * finalV3.x;
	float squareFinalY = finalV3.y * finalV3.y;
	//float squareFinalZ = finalV3.z * finalV3.z;

	//float initialV3Length = sqrt(squareInitialX + squareInitialY + squareInitialZ);
	float initialV3Length = sqrt(squareInitialX + squareInitialY);
	//float finalV3Length = sqrt(squareFinalX + squareFinalY + squareFinalZ);
	float finalV3Length = sqrt(squareFinalX + squareFinalY);

	double angle = acos(dotProuct / (initialV3Length * finalV3Length));
	
	// debug
	//cout << "\nAngle calculted (radians) = " << angle << " (degree) = " << angle * (180.f/piApprox)  << endl;

	return angle;
}

/// <summary>
/// 
/// </summary>
/// <param name="obj"></param>
/// <param name="degreeOfRotation"> in degrees, a float.</param>
void rotateAboutObjCenter(GameObject& obj, float degreeOfRotation)
{
	mat4 translateToOrigin = glm::translate(mat4(1.0f), -obj.position);	// Used to translate all the object's vec3 verts by -Position (Position should be at origin)
	mat4 translateBack = glm::translate(glm::mat4(1.0f), obj.position);

	glm::mat4 identity = glm::mat4(1.0f);					// identity matrix for transformations (4x4)
	float angle = glm::radians(degreeOfRotation);			// angle of rotation (converts degree to radians)
	glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos

	glm::mat4 rotateMatrix = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix with the 12-degrees of freedom filled out.

	// 1. translate by -Position (Position should be (0,0,0) after)
	// 2. do rotation
	// 3. translate by Position
	mat4 model = translateBack * rotateMatrix * translateToOrigin;	// resulting matrix of all those transformations

	// Update transformation matrix
	obj.transformationMatrix = model * obj.transformationMatrix;	// multiply the newly calculated model with the old model to get the actual model (want to retain the previous transformations)
	//obj.transformationTexMatrix = model; // (DELETE)

	//// do the transformations on the obj's vec3's (DELETE)
	//for (int i = 0; i < obj.cgeom.verts.size(); i++)
	//{
	//	//obj.cgeom.verts.at(i) = model * glm::vec4(obj.cgeom.verts.at(i), 0.f);	// idk if i want to have '0' or '1' for 4-th cord
	//	//obj.cgeom.texCoords.at(i) = model * glm::vec4(obj.cgeom.texCoords.at(i), 0.f, 0.f);
	//}
}

void translateObj(GameObject& obj, double deltaX, double deltaY)
{
	// Create translation matrix
	mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(deltaX, deltaY, 0.f));

	// Update the object's position
	obj.position += vec3(deltaX, deltaY, 0.0f);

	// Update the transformation matrix
	obj.transformationMatrix = translationMatrix + obj.transformationMatrix; // update the transformation matrix

	// DELETE
	//obj.transformationTexMatrix = translationMatrix * obj.transformationTexMatrix; // Apply translation to texture transformation
}

void uniformScaleObj(GameObject& obj, float scaleVal)
{
	// Create scale matrix
	mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleVal, scaleVal, 0.f));

	// Update the transformation matrix
	obj.transformationMatrix = scaleMatrix * obj.transformationMatrix; // update the transformation matrix
}

void scaleObj(GameObject& obj, float scaleX, float scaleY)
{
	// Create scale matrix
	mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 0.f));

	// Update the transformation matrix
	obj.transformationMatrix *= scaleMatrix; // update the transformation matrix
}

/// <summary>
/// Prints the position of the 'vec4', x, y, z, w coordinates.
/// </summary>
/// <param name="vec"> the glm::vec4's position to be printed. </param>
/// <param name="vecNum"> an int, a number identifier, default is -1. </param>
void printVec4Pos(glm::vec4 vec, int vecNum)
{
	std::cout << "vec4 #" << vecNum << " @ (" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")" << std::endl;
}

/// <summary>
/// Prints the position of the 'vec4', x, y, z, w coordinates.
/// </summary>
/// <param name="vec"> the glm::vec4's position to be printed. </param>
/// <param name="vecNum"> an int, a number identifier, default is -1. </param>
void printVec4Pos(glm::vec4 vec)
{
	printVec4Pos(vec, -1);
}
