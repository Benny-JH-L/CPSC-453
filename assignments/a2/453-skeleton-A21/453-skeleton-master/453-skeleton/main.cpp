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

	glm::vec3 position;
	float theta; // Object's rotation
	// Alternatively, you could represent rotation via a normalized heading vec:
	// glm::vec3 heading;
	float scale; // Or, alternatively, a glm::vec2 scale;
	glm::mat4 transformationMatrix;
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
};

void drawGameObject(ShaderProgram& shader, GameObject& obj);
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

		glm::mat4 identity = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
		float angle = glm::radians(90.0f * counter);		// angle of rotation (converts degree to radians)
		glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
		glm::mat4 rotateMatrix = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix with the 12-degrees of freedom filled out.
		gameData.ship.transformationMatrix = rotateMatrix;
		drawGameObject(shader, gameData.ship);
		counter++;

		std::cout << "counter = " << counter << std::endl;

	}

	virtual void cursorPosCallback(double xpos, double ypos)
	{
		//std::cout << "Mouse pos: (" << xpos << ", " << ypos << ")" << std::endl;
	}

	//virtual void mouseButtonCallback(int button, int action, int mods)
	//{
	//	std::cout << "Hi there :)))" << std::endl;
	//}

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

// for now transformations will be done like this, until i set up the uniforms and what now.
void translate(GameObject& obj, float val1, float val2)
{
	for (int i = 0; i < obj.cgeom.verts.size(); i++)
	{
		glm::vec3& vecToTranslate = obj.cgeom.verts.at(i);
		vecToTranslate.x = vecToTranslate.x + val1;
		vecToTranslate.y = vecToTranslate.y + val2;
	}
}

void drawGameObject(ShaderProgram& shader, GameObject& obj)
{
	//glm::mat4 identity = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
	//float angle = glm::radians(90.0f);		// angle of rotation (converts degree to radians)
	//glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
	//glm::mat4 rotateMatrix = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix with the 12-degrees of freedom filled out.
	//obj.transformationMatrix = rotateMatrix;

	//GLuint shaderProgram = (GLuint)("shaders/test.frag"); // = 1;
	//GLint rotationLoc = glGetUniformLocation(shaderProgram, "transformationMatrix");
	//glUniformMatrix4fv(rotationLoc, shaderProgram, GL_FALSE, glm::value_ptr(obj.transformationMatrix));

	shader.use();
	obj.ggeom.bind();
	// Draw Game Object
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
	// Put the diamonds starting locations (using this method for now)
	translate(d0, -0.5f, -0.5f);
	translate(d1, 0.5f, 0.5f);
	translate(d2, -0.5f, 0.5f);
	translate(d3, 0.5f, -0.5f);

	// Set gpu geoms
	//ship.ggeom.setVerts(ship.cgeom.verts);
	//ship.ggeom.setTexCoords(ship.cgeom.texCoords);
	setGpuGeom(ship);

	//d0.ggeom.setVerts(d0.cgeom.verts);
	//d0.ggeom.setTexCoords(d0.cgeom.texCoords);
	setGpuGeom(d0);
	setGpuGeom(d1);
	setGpuGeom(d2);
	setGpuGeom(d3);

	// idk
	//glm::mat4 identity1 = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
	//float angle1 = glm::radians(90.f);		// angle of rotation (converts degree to radians)
	//glm::vec3 axisOfRotation1 = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
	//glm::mat4 noTransform = glm::rotate(identity1, angle1, axisOfRotation1);	// transformation matrix with the 12-degrees of freedom filled out.
	//ship.transformationMatrix = noTransform;
	//d0.transformationMatrix = noTransform;

	//GLuint shaderProgram = 1; //(GLuint)("shaders/test.frag");
	//GLint rotationLoc = glGetUniformLocation(shaderProgram, "transformationMatrix");
	//glUniformMatrix4fv(rotationLoc, shaderProgram, GL_FALSE, glm::value_ptr(noTransform));

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
		drawGameObject(shader, d1);
		drawGameObject(shader, d2);
		drawGameObject(shader, d3);

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
