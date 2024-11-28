//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitCube.h"

#include <tuple>
#include "Panel.h"

using namespace std;
using namespace glm;

const static int NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION = 20;
const static int NUMBER_OF_ITERATIONS_FOR_CHAIKIN = 4;
const static float MIN_FRAME_RATE = 10.f;	// 10 frames per second
const static float MAX_FRAME_RATE = 200.f;	// 200 frams per second

const static vector<string> typesOfCelestialBodies = { "Planet", "Moon", "Star" };
const static vector<string> PLANET_NAMES =
{"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
const static vector<string> PLANET_TEXUTRE_PATHS =
{"textures/mercury.jpg", "textures/venus_atmosphere.jpg", "textures/earth_day.jpg", "textures/mars.jpg",
"textures/jupiter.jpg", "textures/saturn.jpg", "textures/uranus.jpg", "textures/neptune.jpg" };

//struct allMoons
//{
//	const vector<string> MARS_MOON_NAMES = { "Phobos", "Deimos" };
//	const vector<string> JUPITER_MOON_NAMES = { "Io", "Europa", "Ganymede" };	// 3 largest
//	const vector<string> SATURN_MOON_NAMES = { "Titan", "Rhea", "Iapetus" };		// 3 largest
//	const vector<string> URANUS_MOON_NAMES = { "Titania", "Oberon", "Ariel" };	// 3 largest
//	const vector<string> NEPTUNE_MOON_NAMES = { "Triton", "Nereid", "Proteus" };	// 3 largest
//};

const vector<string> MARS_MOON_NAMES = { "Phobos", "Deimos" };
const vector<string> JUPITER_MOON_NAMES = { "Io", "Europa", "Ganymede" };	// 3 largest
const vector<string> SATURN_MOON_NAMES = { "Titan", "Rhea", "Iapetus" };		// 3 largest
const vector<string> URANUS_MOON_NAMES = { "Titania", "Oberon", "Ariel" };	// 3 largest
const vector<string> NEPTUNE_MOON_NAMES = { "Triton", "Nereid", "Proteus" };	// 3 largest

struct windowData;

class CelestialBody
{
public:
	static windowData windowData;

	// type is the type of celestial body, moon, planet, star etc.
	// 0: planet
	// 1: moon
	// 2: star
	CelestialBody(string name, float radius, float axisTilt, float axialRotationRate, string texturePath) :
		name(name),
		radius(radius),
		position(vec3(0.f, 0.f, 0.f)),
		axisTilt(axisTilt),
		axialRotationRate(axialRotationRate),
		texture(texturePath, GL_NEAREST)
	{
		generateGeometry();

		mat4 tiltAxis = glm::rotate(mat4(1.0f), glm::radians(axisTilt), vec3(0.f, 0.f, 1.f));
		model = tiltAxis * model;
	}

	string name;
	float radius;
	vec3 position;
	float axisTilt;							// in degrees
	float axialRotationRate;				// How much to rotate about it's axis, in degrees
	vec3 axis = vec3(0.f, 1.f, 0.f);		// y-axis
	string type;
	Texture texture;

	CPU_Geometry cpu_geom;
	GPU_Geometry gpu_geom;

	void translateBody(vec3 val)
	{
		translation = glm::translate(translation, val);

		// same thing
		//mat4 t = glm::translate(mat4(1.0f), val);
		//model = t * model;

		//position += val;
	}

	void setAxisRotationRate(float degree)
	{
		axialRotationRate = degree;
	}

	void rotateViaAxis()
	{
		axialRotation = glm::rotate(axialRotation, glm::radians(axialRotationRate), axis);
		//axialRotation = glm::rotate(axialRotation, glm::radians(axialRotationRate), vec3(0.f, 0.f, 1.f));
	}

	virtual void orbitCelestialBody()
	{}


	virtual mat4 getModelMat4()
	{
		return translation * axialRotation * model;
	}

	mat4 getOrbitalInclination()
	{
		return orbitalInclination;
	}

	mat4 getTranslation()
	{
		return translation;
	}

protected:
	mat4 model = mat4(1.0f);
	mat4 translation = mat4(1.0f);
	mat4 axialRotation = mat4(1.0f);
	mat4 orbitalInclination = mat4(1.0f);

	/// <summary>
	/// Rotates a vec3 by 'degree's, (Use negative degree for clockwise).
	/// (Rotates with respect to the origin)
	/// </summary>
	/// <param name="vecToRotate"></param>
	/// <param name="degree"></param>
	//void rotateVec3(vec3& vecToRotate, float degree)
	//{
	//	float x = vecToRotate.x;
	//	float y = vecToRotate.y;
	//	float rad = glm::radians(degree);

	//	float xfinal = x * cos(rad) - y * sin(rad);
	//	if (abs(xfinal) < 1.0e-6)	// if the x-value is a very small number,
	//		xfinal = 0.f;			// make xfinal 0
	//	float yfinal = x * sin(rad) + y * cos(rad);
	//	if (abs(yfinal) < 1.0e-6)	// if the y-value is a very small number,
	//		yfinal = 0.f;			// make yfinal 0

	//	// Set the rotated values
	//	vecToRotate.x = xfinal;
	//	vecToRotate.y = yfinal;
	//}

	// Control points used to generate spherical geometry
	const vector<vec3> sphereControlPoints =
	{
		{0.f, -.6f * radius, 0.f},
		{ .6f * radius, -.5f * radius, 0.f},
		{ .6f * radius,  .5f * radius, 0.f},
		{0.f, .6f * radius, 0.f}
	};

	void generateGeometry()
	{
		vector<vec3> chaikinCurvePts = sphereControlPoints;
		for (int i = 0; i < NUMBER_OF_ITERATIONS_FOR_CHAIKIN; i++)
			chaikinCurvePts = chaikinOpen(chaikinCurvePts);

		auto [meshPoints, meshTriangles] = surfaceOfRevolution(chaikinCurvePts, NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION);
		cpu_geom.verts = meshTriangles;
		//cpu_geom.cols = vector<vec3>(meshTriangles.size(), vec3(0.f, 0.f, 1.0));

		vector<vec3> cols;
		for (int i = 0; i < meshTriangles.size(); i++)
		{
			if (i <= meshTriangles.size() / 3.f)
				cols.push_back(vec3(1.0f, 0.f, 0.f));
			else if (i <= 2 * (meshTriangles.size() / 3.f))
				cols.push_back(vec3(0.0f, 1.f, 0.f));
			else
				cols.push_back(vec3(0.0f, 0.f, 1.f));
		}
		cpu_geom.cols = cols;
		gpu_geom.setVerts(cpu_geom.verts);
		gpu_geom.setCols(cpu_geom.cols);
	}

	/// <summary>
	/// Generates an open polynomial B-Spline curve.
	/// </summary>
	/// <param name="coursePts"> set of control points, std::vector[glm::vec3].</param>
	/// <returns> a std::vector(glm::vec3) that contains the open polynomial B-spine curve.</returns>
	static vector<vec3> chaikinOpen(const vector<vec3> coursePts)
	{
		int numCoursePts = coursePts.size();
		vector<vec3> finePoints;

		// Special mask for the beginning
		finePoints.push_back(coursePts[0]);
		finePoints.push_back((0.5f * coursePts[0]) + (0.5f * coursePts[1]));

		// Interior points
		for (int i = 1; i < numCoursePts - 2; i++)
		{
			finePoints.push_back((0.75f * coursePts[i]) + (0.25f * coursePts[i + 1]));
			finePoints.push_back((0.25f * coursePts[i]) + (0.75f * coursePts[i + 1]));
		}

		// Special mask for the end
		finePoints.push_back((0.5f * coursePts[numCoursePts - 2]) + (0.5f * coursePts[numCoursePts - 1]));
		finePoints.push_back(coursePts[numCoursePts - 1]);

		return finePoints;
	}

	/// <summary>
	/// Takes in a B-Spline curve and the desired number of "slices" (larger slice value = smoother surface).
	/// Returns a tuple containing the mesh point positions, and triangles that make up the mesh.
	/// 1st element: mesh point positions, std::vector(vec3).
	/// 2nd element: triangle mesh, std::vector(vec3).
	/// </summary>
	/// <param name="bSplineCurve"> the b-spline curve to make the surface from.</param>
	/// <param name="numSlices"> how many 'slices' wanted when generating the surface.</param>
	/// <returns></returns>
	static tuple<vector<vec3>, vector<vec3>> surfaceOfRevolution(vector<vec3> bSplineCurve, int numSlices)
	{
		vector<vec3> meshPoints;
		vector<vec3> meshTriangles;

		float angleStep = 2.0f * glm::pi<float>() / numSlices;
		vector<vector<vec3>> temp2D;	// each nested vector contains a b-spline curve point rotated 'numSlices' times

		// Create mesh points
		for (int i = 0; i < bSplineCurve.size(); i++)
		{
			vector<vec3> temp;
			// Create mesh point by rotating the b-spline curve point
			for (int j = 0; j <= numSlices; j++)
			{
				float angle = j * angleStep;

				// rotate about x-axis
				//float x = bSplineCurve[i].x;
				//float y = bSplineCurve[i].y * cos(angle) - bSplineCurve[i].z * sin(angle);
				//float z = bSplineCurve[i].y * sin(angle) + bSplineCurve[i].z * cos(angle);

				// rotate about y-axis
				float x = bSplineCurve[i].x * cos(angle);
				float z = bSplineCurve[i].x * sin(angle);
				float y = bSplineCurve[i].y;

				// rotate about z-axis
				//float x = bSplineCurve[i].x * cos(angle) - bSplineCurve[i].y * sin(angle);
				//float y = bSplineCurve[i].x * sin(angle) + bSplineCurve[i].y * cos(angle);
				//float z = bSplineCurve[i].z;

				meshPoints.push_back(vec3(x, y, z));
				temp.push_back(vec3(x, y, z));
			}
			temp2D.push_back(temp);
		}

		// Create mesh triangles
		for (int i = 0; i < temp2D.size() - 1; i++)		// minus 1, as we can't draw a triangle with only the last set of points left
		{
			vector<vec3> currentSetOfPoints = temp2D[i];
			vector<vec3> nextSetOfPoints = temp2D[i + 1];

			for (int j = 0; j < currentSetOfPoints.size() - 1; j++)	// minus 1, as we can't draw a triangle with other points left
			{
				// First triangle
				meshTriangles.push_back(currentSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j + 1]);

				// Second triangle
				meshTriangles.push_back(currentSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j + 1]);
				meshTriangles.push_back(currentSetOfPoints[j + 1]);
			}
			// Second triangle
			//for (int j = 0; j < currentSetOfPoints.size() - 1; j++)
			//{
			//	meshTriangles.push_back(currentSetOfPoints[j]);
			//	meshTriangles.push_back(nextSetOfPoints[j + 1]);
			//	meshTriangles.push_back(currentSetOfPoints[j + 1]);

			//}
			// result is a square/rectangle defined by these two triangles (triangles are drawn counter clock wise)
		}

		return make_tuple(meshPoints, meshTriangles);
	};
};

class Star : public CelestialBody
{
public:
	Star(string name, float radius, float axisTilt, float axisRotation, string texturePath) :
		CelestialBody(name, radius, axisTilt, axisRotation, texturePath)
	{
		type = typesOfCelestialBodies[2];
	}
};

class Moon;	// prototype

class Planet : public CelestialBody
{
public:
	Planet(string name, float radius, float axisTilt, float axisRotation, float orbitRate, Star& sun, string texturePath) :
		CelestialBody(name, radius, axisTilt, axisRotation, texturePath),
		orbitRate(orbitRate),
		orbitingStar(sun)
	{
		type = typesOfCelestialBodies[0];
	}

	void addMoon(Moon* m)
	{
		moons.push_back(m);
	}

	void orbitCelestialBody() override
	{
		numOrbitCelestialBodyCalls++;
		updatePosition();
		orbitalInclination = glm::rotate(orbitalInclination, glm::radians(orbitRate), vec3(0.f, 1.f, 0.f));
	}

	virtual mat4 getModelMat4() override
	{
		//return axialRotation * translation * orbitRotation * model;
		return orbitalInclination * translation * axialRotation * model;	// I think this is the correcet way?, test axialRotation and orbitRotation seperatley and you'll understamd
		//return orbitalInclination * axialRotation * model;	// I think this is the correcet way?, test axialRotation and orbitRotation seperatley and you'll understamd

	}

	vector<Moon*> moons;
	Star& orbitingStar;
	float orbitRate;

private:
	int numOrbitCelestialBodyCalls = 0;

	void updatePosition()
	{
		float x = radius * cos(glm::radians(orbitRate) * numOrbitCelestialBodyCalls);
		float z = radius * sin(glm::radians(orbitRate) * numOrbitCelestialBodyCalls);
		position = vec3(x, 0.f, z);
	}
};

class Moon : public CelestialBody
{
public:
	Moon(string name, float radius, float axisTilt, float axisRotation, float orbitRate, Planet& orbitingPlanet, string texturePath) :
		CelestialBody(name, radius, axisTilt, axisRotation, texturePath),
		orbitingPlanet(orbitingPlanet),
		orbitRate(orbitRate)
	{
		type = typesOfCelestialBodies[1];
	}

	virtual mat4 getModelMat4() override
	{
		//return axialRotation * translation * orbitRotation * model;
		mat4 starToPlanetOrbitInclination = orbitingPlanet.getOrbitalInclination();
		mat4 planetTranslation = orbitingPlanet.getTranslation();

		return orbitalInclination * planetTranslation * starToPlanetOrbitInclination * translation * axialRotation * model;

		//return orbitRotation * translation * axialRotation * model;	// I think this is the correcet way?, test axialRotation and orbitRotation seperatley and you'll understamd
	}

	void orbitCelestialBody() override
	{
		orbitalInclination = glm::rotate(orbitalInclination, glm::radians(orbitRate), vec3(0.f, 1.f, 0.f));
	}

	Planet& orbitingPlanet;
	float orbitRate;
};

struct windowData
{
	bool& pause;
	float& frameRate;
	vector<Planet>& planets;
	vector<Moon>& moons;
	Star sun;
	vec3& lookAt;	// for camera
};


class GuiPanel : public PanelRendererInterface
{
public:

	GuiPanel(windowData& d, Camera& camera) :
		windowData(d),
		camera(camera)
	{
		//int optionsSize = PLANET_NAMES.size();
		options[0] = "Planets";
		options[1] = "Moons";
		options[2] = "Sun";

		// Set planet options
		for (int i = 0; i < PLANET_NAMES.size(); i++)
			planetOptions[i] = PLANET_NAMES[i].c_str();

		//allMoons allMoons;

		// Set moon options
		moonOptions[0] = windowData.moons[0].name.c_str();

		int indexCounter = 1;
		//for (int i = 0; i < allMoons.MARS_MOON_NAMES.size(); i++, indexCounter++)
		//	moonOptions[indexCounter] = allMoons.MARS_MOON_NAMES[i].c_str();

		//for (int i = 0; i < allMoons.JUPITER_MOON_NAMES.size(); i++, indexCounter++)
		//	moonOptions[indexCounter] = allMoons.JUPITER_MOON_NAMES[i].c_str();

		//for (int i = 0; i < allMoons.SATURN_MOON_NAMES.size(); i++, indexCounter++)
		//	moonOptions[indexCounter] = allMoons.SATURN_MOON_NAMES[i].c_str();

		//for (int i = 0; i < allMoons.URANUS_MOON_NAMES.size(); i++, indexCounter++)
		//	moonOptions[indexCounter] = allMoons.URANUS_MOON_NAMES[i].c_str();

		//for (int i = 0; i < allMoons.NEPTUNE_MOON_NAMES.size(); i++, indexCounter++)
		//	moonOptions[indexCounter] = allMoons.NEPTUNE_MOON_NAMES[i].c_str();

		for (int i = 0; i < MARS_MOON_NAMES.size(); i++, indexCounter++)
			moonOptions[indexCounter] = MARS_MOON_NAMES[i].c_str();

		for (int i = 0; i < JUPITER_MOON_NAMES.size(); i++, indexCounter++)
			moonOptions[indexCounter] = JUPITER_MOON_NAMES[i].c_str();

		for (int i = 0; i < SATURN_MOON_NAMES.size(); i++, indexCounter++)
			moonOptions[indexCounter] = SATURN_MOON_NAMES[i].c_str();

		for (int i = 0; i < URANUS_MOON_NAMES.size(); i++, indexCounter++)
			moonOptions[indexCounter] = URANUS_MOON_NAMES[i].c_str();

		for (int i = 0; i < NEPTUNE_MOON_NAMES.size(); i++, indexCounter++)
			moonOptions[indexCounter] = NEPTUNE_MOON_NAMES[i].c_str();

		cout << "hi";
	}

	virtual void render() override
	{
		ImGui::SliderInt("Frame Rate", &frames, MIN_FRAME_RATE, MAX_FRAME_RATE, "%d Frames/sec");
		frameRate = 1.f / frames;

		ImGui::Combo("Center Camera", &comboSelection, options, IM_ARRAYSIZE(options));

		// Show planets
		if (comboSelection == 0)
		{
			ImGui::Combo("Select a Planet", &planetSelection, planetOptions, IM_ARRAYSIZE(planetOptions));
			//cameraFocusedOnBodyStr = planetOptions[planetSelection];
			celestialBodyFocusedOn = &planetObjects[planetSelection];
			//camera.lookAt = planetObjects[planetSelection].position;
			camera.lookAt = celestialBodyFocusedOn->position;
		}
		// Show Moons
		else if (comboSelection == 1)
		{
			ImGui::Combo("Select a Moon", &moonSelection, moonOptions, IM_ARRAYSIZE(moonOptions));
			celestialBodyFocusedOn = &moonObjects[moonSelection];
			//cameraFocusedOnBodyStr = moonOptions[moonSelection];
			//camera.lookAt = moonObjects[moonSelection].position;
			camera.lookAt = celestialBodyFocusedOn->position;
		}
		else
		{
			//cameraFocusedOnBodyStr = windowData.sun.name;
			celestialBodyFocusedOn = &windowData.sun;
			//camera.lookAt = windowData.sun.position;
			camera.lookAt = celestialBodyFocusedOn->position;
		}

		if (previousComboSelection != comboSelection)
		{
			cout << "reset camera values" << endl;
			previousComboSelection = comboSelection;
			resetCameraValues();
			float r = calcCameraRadiusFromCelestialBodyPosition(celestialBodyFocusedOn->position);
			camera.setRadius(r);
		}

		string s = "Camera focused on: ";
		s.append(celestialBodyFocusedOn->name);
		ImGui::Text(s.c_str());

		vec3 celestialBodyPos = celestialBodyFocusedOn->position;
		ImGui::Text("Position: (%.3f, %.3f, %.3f)", celestialBodyPos.x, celestialBodyPos.y, celestialBodyPos.z);
		ImGui::Text("Radius: %.3f", celestialBodyFocusedOn->radius);

		// Camera Values
		ImGui::Text("---Camera Values---");
		vec3 cameraPos = camera.getPos();
		ImGui::Text("Position: (%.3f, %.3f, %.3f)", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("Radius: %.3f", camera.getRadius());
		ImGui::Text("Theta: %.3f", camera.getTheta() * 180.f);
		ImGui::Text("Phi: %.3f", camera.getPhi());

		ImGui::Checkbox("Pause", &windowData.pause);


		ImGui::Text("---DEBUG---");
		Planet& earth = windowData.planets[2];
		vec3 earthPos = earth.position;
		ImGui::Text("Earth pos: (%.3f, %.3f, %.3f)", earthPos.x, earthPos.y, earthPos.z);

	}
private:
	windowData& windowData;
	Camera& camera;
	const char* options[3];			// Options for the combo box
	int comboSelection = 2;
	int previousComboSelection = 2;
	const char* planetOptions[8];			// 8 = size of PLANET_NAMES
	int planetSelection = 2;				// Earth set as default
	const char* moonOptions[15];		// 15 = all the moons of the planets
	int moonSelection = 0;				// Earth's moon set as default

	string cameraFocusedOnBodyStr = windowData.sun.name;	// initialy set it focused on the sun
	int frames = 10;
	float& frameRate = windowData.frameRate;
	CelestialBody* celestialBodyFocusedOn;
	vector<Planet>& planetObjects = windowData.planets;
	vector<Moon>& moonObjects = windowData.moons;

	void resetCameraValues()
	{
		camera.resetPhi();
		//camera.resetRadius();
		camera.resetTheta();
	}

	float calcCameraRadiusFromCelestialBodyPosition(vec3 cbPosition)
	{
		vec3 cameraPos = camera.getPos();
		return glm::length(cameraPos - cbPosition);
	}
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface
{

public:
	Assignment4(windowData& windowData) :
		camera(glm::radians(45.f), glm::radians(45.f), 3.0, windowData.lookAt)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
		, windowData(windowData)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {}

	virtual void mouseButtonCallback(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos)
	{
		if (rightMouseDown)
		{
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}

	virtual void scrollCallback(double xoffset, double yoffset)
	{
		Log::info("yoffset = {}", yoffset);
		camera.incrementR(yoffset);
	}

	virtual void windowSizeCallback(int width, int height)
	{
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp)
	{
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		//GLint location = glGetUniformLocation(sp, "lightPosition");
		//glm::vec3 light = camera.getPos();
		//glUniform3fv(location, 1, glm::value_ptr(light));
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void setPlanetModelMat(ShaderProgram& sp, CelestialBody& p)
	{
		//mat4 M = p.model;
		mat4 M = p.getModelMat4();
		//glm::mat4 M = glm::mat4(1.0);

		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;
private:
	windowData& windowData;
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};

void renderCelestialBody(shared_ptr<Assignment4>& callBack, ShaderProgram& sp, CelestialBody& cb)
{
	callBack->setPlanetModelMat(sp, cb);
	cb.gpu_geom.bind();
	//p.texture.bind();
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(cb.cpu_geom.verts.size()));
	//p.texture.unbind();
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 3");
	Panel panel(window.getGLFWwindow());
	
	float frameRate = 1.0f / 10.f;
	vector<Planet> planets;
	vector<Moon> moons;
	vec3 lookAt = vec3(0.f);
	bool pause = false;
	//Star sun = Star("sun", 1, 0.f, "textures/ship.png");
	windowData windowData = {pause, frameRate, planets, moons, Star("Sun", 1, 0.f, 0.f, "textures/sun.jpg"), lookAt };
	
	GLDebug::enable();

	UnitCube cube;
	cube.generateGeometry();

	// Create Planets and Moons

	float radius = 1;
	float axisRotation = 10.f; // degrees
	float orbitRate = 5.0f;	// degrees
	for (int i = 0; i < PLANET_NAMES.size(); i++, radius++)
	{
		planets.push_back(Planet(PLANET_NAMES[i], radius, 45.f, axisRotation, orbitRate, windowData.sun, PLANET_TEXUTRE_PATHS[i]));
		planets[i].translateBody(vec3(vec2(0.f), i + 1));
	}

	/*
	planets.push_back(Planet("earth", 2, -45.f, 0.f, windowData.sun, "textures/earth_day.jpg"));
	Planet& earth = planets[0];
	//earth.translateBody(vec3(3.0f, vec2(0.f)));
	earth.translateBody(vec3(vec2(0.f), 3.f));
	*/
	Planet& earth = planets[2];
	float axisRotationMoon = 10.f; // degrees
	float orbitRateMoon = 5.0f;	// degrees

	moons.push_back(Moon("The Moon", 1, 45.f, axisRotationMoon, orbitRateMoon, earth, "textures/the_moon.jpg"));
	Moon* moon = &moons[0];
	earth.addMoon(moon);
	//moon->translateBody(earth.radius + vec3(1.0f, vec2(0.f)));
	moon->translateBody(vec3(vec2(0.f), 1.0f + earth.position.z + earth.radius));
	
	//Star& sun = windowData.sun;

	// CALLBACKS
	shared_ptr<Assignment4> callBack = std::make_shared<Assignment4>(windowData);
	callBack->camera.lookAt = windowData.lookAt;
	window.setCallbacks(callBack);

	auto gui_panel_renderer = std::make_shared<GuiPanel>(windowData, callBack->camera);
	panel.setPanelRenderer(gui_panel_renderer);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// RENDER LOOP
	while (!window.shouldClose())
	{
		double startTime = glfwGetTime();

		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL /*GL_LINE*/);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE /*GL_LINE*/);


		shader.use();
		callBack->viewPipeline(shader);

		// cube
		cube.m_gpu_geom.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(cube.m_size));

		// Render celestial bodies
		renderCelestialBody(callBack, shader, windowData.sun);

		//for (Planet& p : windowData.planets)
		//{
			//renderCelestialBody(callBack, shader, p);
			/*
			callBack->setPlanetModelMat(shader, p);
			p.gpu_geom.bind();
			//p.texture.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(p.cpu_geom.verts.size()));
			//p.texture.unbind();
			*/
		//}
		renderCelestialBody(callBack, shader, earth);


		for (Moon& m : windowData.moons)
			renderCelestialBody(callBack, shader, m);

		if (!pause)
		{
			earth.rotateViaAxis();
			earth.orbitCelestialBody();
			moon->rotateViaAxis();
			moon->orbitCelestialBody();
		}


		//callBack->setPlanetModelMat(shader, earth);
		//earth.gpu_geom.bind();
		//earth.texture.bind();
		//glDrawArrays(GL_TRIANGLES, 0, GLsizei(earth.cpu_geom.verts.size()));
		//earth.texture.unbind();

		//moon.rotatePlanetAxis(45.f);
		//callBack->setPlanetModelMat(shader, moon);
		//moon.gpu_geom.bind();
		//glDrawArrays(GL_TRIANGLES, 0, GLsizei(moon.cpu_geom.verts.size()));

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		panel.render();
		window.swapBuffers();

		double endTime = glfwGetTime();

		if (endTime - startTime < windowData.frameRate)
		{
			double sleepTime = windowData.frameRate - (endTime - startTime);
			//glfwWaitEventsTimeout(sleepTime);

			double sleepStart = glfwGetTime();
			double sleepEnd = -1;

			while (sleepEnd - sleepStart < sleepTime)
			{
				sleepEnd = glfwGetTime();
			}
		}

	}
	glfwTerminate();
	return 0;
}
