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

void printVec4Pos(glm::vec4 vec, int vecNum);
void printVec4Pos(glm::vec4 vec);

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

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface
{

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		cout << "Current cursor location: " << endl;
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			shader.recompile();
		}


	}

private:
	ShaderProgram& shader;
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
	// vertices for the diamond quad
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

// END EXAMPLES

int main()
{
	Log::debug("Starting main");

	// My tests
	glm::vec3 coord = glm::vec3(1.f, 1.f, 0.f);
	glm::vec Position = glm::vec4(coord, 1.0f);		// Homogeneous coord sys. this is a point at (0, 0, 0)
	cout << "position = " ;
	printVec4Pos(Position);

	glm::mat4 identity = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
	float angle = glm::radians(90.0f);		// angle of rotation (converts degree to radians)
	glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
	glm::mat4 Model = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix with the 12-degrees of freedom filled out.
	glm::vec4 Transformed = Model * Position;

	cout << "\nTransformed = ";
	printVec4Pos(Transformed);

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones
	

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	GameObject ship("textures/ship.png", GL_NEAREST);
	GameObject diamond("textures/diamond.png", GL_NEAREST);

	ship.cgeom = shipGeom(0.18f, 0.12f);
	diamond.cgeom = diamondGeom(0.14f, 0.14f);

	ship.ggeom.setVerts(ship.cgeom.verts);
	ship.ggeom.setTexCoords(ship.cgeom.texCoords);

	diamond.ggeom.setVerts(diamond.cgeom.verts);
	diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);

	// RENDER LOOP
	while (!window.shouldClose()) {
		int score;
		glfwPollEvents();

		shader.use();

		ship.ggeom.bind();
		diamond.ggeom.bind();

		GLuint shaderProgram = 1;	// idk what this should be, but its working xd
		GLint rotationLoc = glGetUniformLocation(shaderProgram, "rotationMatrix");
		glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, glm::value_ptr(Model));

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ship.texture.bind();
		diamond.texture.bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);
		ship.texture.unbind();
		diamond.texture.unbind();

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
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

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
