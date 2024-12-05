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
#include <random>

using namespace std;
using namespace glm;

const static double PI_APPROX = atan(1) * 4;	// pi approximation

const static int NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION = 20;
const static int NUMBER_OF_ITERATIONS_FOR_CHAIKIN = 4;
const static float MIN_FRAME_RATE = 10.f;	// 10 frames per second
const static float MAX_FRAME_RATE = 200.f;	// 200 frams per second
const static float MIN_SIMULATION_RATE = 2.f;
const static float MAX_SIMULATION_RATE = 100.f;
const static float MIN_SCROLL_SENSITIVITY = 2.f;
const static float MAX_SCROLL_SENSITIVITY = 20.f;

// For sphere generation (higher = smoother)
const static int sectors = 20;	// num slices of the sphere
const static int stacks = 20;	// num levels of the sphere

// Camera
const static float MAX_CAMERA_RADIUS = 1000.f;
const static float CAMERA_FAR_PLANE = 10000.f;
const static float CAMERA_NEAR_PLANE = 0.01;
const static float INITIAL_CAMERA_RADIUS = 100.f;

// Shading
const static vec3 LIGHT_COLOUR = vec3(1.f);		// White colour
// For planets & Moons
const static GLfloat diffuseCoeff = 1.f;
const static GLfloat specularCoeff = 0.5f;
const static GLfloat ambientCoeff = 0.1f;
const static GLfloat shininessCoeff = 5.f;
// For the Sun
const static GLfloat SUN_AMBIENT_COEFF = 1.2f;

// Planet and Moon details for generation
const static vector<string> TYPES_OF_CELESTIAL_BODIES = { "Planet", "Moon", "Star" };
const static vector<string> PLANET_NAMES =
{"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
const static vector<string> PLANET_TEXUTRE_PATHS =
{"textures/mercury.jpg", "textures/venus_atmosphere.jpg", "textures/earth_day.jpg", "textures/mars.jpg",
"textures/jupiter.jpg", "textures/saturn.jpg", "textures/uranus.jpg", "textures/neptune.jpg" };
//const static string SKYBOX_TEXTURE_PATH = "textures/4k_stars_milky_way.jpg";	// Lower resolution of the sky box texture
const static string SKYBOX_TEXTURE_PATH = "textures/8k_stars_milky_way.jpg";
const static string MOON_TEXTURE_PATH = "textures/the_moon.jpg";	// all the moons will be using this texture.

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
	CelestialBody(string name, float radius, float distanceFromOrbitingCelestialBody, float axisTilt, float axialRotationRate, string texturePath) :
		name(name),
		radius(radius),
		distanceFromOrbitingCelestialBody(distanceFromOrbitingCelestialBody),
		axisTilt(axisTilt),
		axialRotationRate(axialRotationRate),
		texture(texturePath, GL_NEAREST)
	{
		//generateGeometryUsingSurfaceOfRevolution();
		generateGeometryUsingSphericalCoords();

		//mat4 tiltAxis = glm::rotate(mat4(1.0f), glm::radians(axisTilt), vec3(0.f, 0.f, 1.f));
		//model = tiltAxis * model;
	}

	string name;
	float radius;
	float distanceFromOrbitingCelestialBody;
	vec3 position = vec3(0.f);
	float axisTilt;							// in degrees (About the z-axis)
	float axialRotationRate;				// How much to rotate about it's axis, in degrees.
	string type;
	Texture texture;

	CPU_Geometry cpu_geom;
	GPU_Geometry gpu_geom;

	/// <summary>
	/// Sets the rate at which the CelestialBody rotates about its axis (y-axis), in degrees.
	/// </summary>
	/// <param name="degree"></param>
	void setAxisRotationRate(float degree)
	{
		axialRotationRate = degree;
	}

	/// <summary>
	/// Rotates the CelestialBody by 'axialRotationRate' about it's planetary axis (y-axis).
	/// </summary>
	void rotateViaCelestialBodyAxis(float time)
	{
		axialRotationMat4 = glm::rotate(axialRotationMat4, glm::radians(axialRotationRate * time), vec3(0.f, 1.f, 0.f));	// time dependent
		//axialRotation = glm::rotate(axialRotation, glm::radians(axialRotationRate), vec3(0.f, 0.f, 1.f));
	}

	// to be overridden in subclasses
	virtual void orbitCelestialBody(float time) {}

	// to be overridden in subclasses
	virtual mat4 getModelMat4()
	{
		//return axialRotationMat4 * model;
		return model;
	}

	mat4 getOrbitalInclination()
	{
		//return mat4(1.f);
		return orbitalInclination;
	}

	mat4 getCelestialBodyAxisTilt()
	{
		//return glm::rotate(mat4(1.f), glm::radians(axisTilt), vec3(0.f, 0.f, 1.f));		// tilts in the z-axis
		return glm::rotate(mat4(1.f), glm::radians(axisTilt), vec3(1.f, 0.f, 0.f));		// (WORKS) tilts in the x-axis (need to use this since it wont look quite right -> just test earth and you'll see)

		// test (just do the x-axis rotate...)
		//mat4 xAxisTilt =  glm::rotate(mat4(1.f), glm::radians(axisTilt), vec3(1.f, 0.f, 0.f));		// tilts in the x-axis (need to use this since it wont look quite right -> just test earth and you'll see)
		//mat4 zAxisTilt =  glm::rotate(mat4(1.f), glm::radians(axisTilt), vec3(0.f, 0.f, 1.f));		// tilts in the z-axis

		//return zAxisTilt * xAxisTilt;
		//return xAxisTilt * zAxisTilt;

	}
	//mat4 getTranslation()
	//{
	//	return translation;
	//}



protected:
	mat4 orbitalInclination = mat4(1.0f);
	mat4 model = mat4(1.0f);
	mat4 axialRotationMat4 = mat4(1.0f);		// stores the celestial body's axial rotation (about y-axis)

	// Control points used to generate spherical geometry
	const vector<vec3> sphereControlPoints =
	{
		{0.f, -.6f * radius, 0.f},
		{ .6f * radius, -.5f * radius, 0.f},
		{ .6f * radius,  .5f * radius, 0.f},
		{0.f, .6f * radius, 0.f}
	};

	void generateGeometryUsingSurfaceOfRevolution()
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

	void generateGeometryUsingSphericalCoords()
	{
		vector<vector<vec3>> sphereStacks;	// each <vector<vec3> contains a stack of the sphere.
		vector<vec3> sphereVec3s;
		vector<vec3> sphereMesh;	// a triangle is a group of 3 vec3's.

		vector<vector<vec2>> textureCoordsStack;
		vector<vec2> textureCoords;
		vector<vec2> sphereTextCoordMesh;	// a triangle is a group of 3 vec3's.

		for (int i = 0; i <= stacks; i++)
		{
			float stackAngle = PI_APPROX / 2 - i * (PI_APPROX / stacks); // from pi/2 to -pi/2
			float xy = radius * cosf(stackAngle);
			float z = radius * sinf(stackAngle);

			vector<vec3> tempStack;		// stores the sphere's vectors on a stack
			vector<vec2> tempTextCoords;
			for (int j = 0; j <= sectors; j++)
			{
				float sectorAngle = j * (2 * PI_APPROX / sectors); // from 0 to 2pi
				float x = xy * cosf(sectorAngle);
				float y = xy * sinf(sectorAngle);
				vec3 vertex = vec3(x, y, z);

				sphereVec3s.push_back(vertex);
				tempStack.push_back(vertex);

				float u = (float)j / sectors;
				float v = (float)i / stacks;

				if (j == sectors)	// Ensure the last sector (slice) aligns with the first sector (slice)
					u = 1.f;

				//textureCoords.push_back(vec2(u, v));
				tempTextCoords.push_back(vec2(u, v));
			}
			sphereStacks.push_back(tempStack);
			textureCoordsStack.push_back(tempTextCoords);
		}

		// Create triangle mesh
		for (int i = 0; i < sphereStacks.size() - 1; i++)			// minus 1, as we can't draw a triangle with only the last set of points left
		{
			vector<vec3> currentSetOfPoints = sphereStacks[i];
			vector<vec3> nextSetOfPoints = sphereStacks[i + 1];

			for (int j = 0; j < currentSetOfPoints.size() - 1; j++)	// minus 1, as we can't draw a triangle with other points left
			{
				// First triangle
				sphereMesh.push_back(currentSetOfPoints[j]);
				sphereMesh.push_back(nextSetOfPoints[j]);
				sphereMesh.push_back(nextSetOfPoints[j + 1]);

				// Second triangle
				sphereMesh.push_back(currentSetOfPoints[j]);
				sphereMesh.push_back(nextSetOfPoints[j + 1]);
				sphereMesh.push_back(currentSetOfPoints[j + 1]);
			}
		}

		// Create texture coord mesh
		for (int i = 0; i < textureCoordsStack.size() - 1; i++)			// minus 1, as we can't draw a triangle with only the last set of points left
		{
			vector<vec2> currentSetOfPoints = textureCoordsStack[i];
			vector<vec2> nextSetOfPoints = textureCoordsStack[i + 1];

			for (int j = 0; j < currentSetOfPoints.size() - 1; j++)	// minus 1, as we can't draw a triangle with other points left
			{
				// First triangle
				sphereTextCoordMesh.push_back(currentSetOfPoints[j]);
				sphereTextCoordMesh.push_back(nextSetOfPoints[j]);
				sphereTextCoordMesh.push_back(nextSetOfPoints[j + 1]);

				// Second triangle
				sphereTextCoordMesh.push_back(currentSetOfPoints[j]);
				sphereTextCoordMesh.push_back(nextSetOfPoints[j + 1]);
				sphereTextCoordMesh.push_back(currentSetOfPoints[j + 1]);
			}
		}

		// generate normals
		vector<vec3> normals = sphereMesh;
		for (vec3& n : normals)
			normalize(n);

		cpu_geom.verts = sphereMesh;
		//cpu_geom.verts = sphereVec3s;	// test (works with textureCoords) -> as in: number of textCoord verties == number of cpu_geom.verts

		//cpu_geom.cols = textureCoords;
		cpu_geom.normals = normals;

		//cpu_geom.textCoords = textureCoords;
		cpu_geom.textCoords = sphereTextCoordMesh;

		gpu_geom.setVerts(cpu_geom.verts);
		//gpu_geom.setCols(cpu_geom.cols);
		gpu_geom.setNormals(cpu_geom.normals);
		gpu_geom.setTexCoords(cpu_geom.textCoords);

		// debug
		std::cout << "Verts size: " << cpu_geom.verts.size() << std::endl;
		std::cout << "TexCoords size: " << cpu_geom.textCoords.size() << std::endl;
		std::cout << "normals size: " << cpu_geom.normals.size() << std::endl;

	}
};

class Star : public CelestialBody
{
public:
	Star(string name, float radius, float axisTilt, float axisRotation, string texturePath) :
		CelestialBody(name, radius, 0.f, axisTilt, axisRotation, texturePath)
	{
		type = TYPES_OF_CELESTIAL_BODIES[2];
	}

	virtual mat4 getModelMat4() override
	{
		return axialRotationMat4 * model;
	}
};

class Moon;	// prototype

class Planet : public CelestialBody
{
public:

	/// <summary>
	/// Note: The distance of the planet from the star will be the: 'distanceFromOrbitingCelestialBody' + its own radius + sun's radius.
	/// </summary>
	/// <param name="name"></param>
	/// <param name="radius"></param>
	/// <param name="distanceFromOrbitingCelestialBody"></param>
	/// <param name="axisTilt"></param>
	/// <param name="axisRotation"></param>
	/// <param name="orbitRate"></param>
	/// <param name="sun"></param>
	/// <param name="texturePath"></param>
	Planet(string name, float radius, float distanceFromOrbitingCelestialBody, float axisTilt, float axisRotation, float orbitRate, Star& sun, string texturePath) :
		//CelestialBody(name, radius, distanceFromOrbitingCelestialBody + sun.radius + radius, axisTilt, axisRotation, texturePath),
		CelestialBody(name, radius, distanceFromOrbitingCelestialBody + radius + sun.radius, axisTilt, axisRotation, texturePath),
		orbitRate(orbitRate),
		orbitingStar(sun)
	{
		type = TYPES_OF_CELESTIAL_BODIES[0];
	}

	void addMoon(Moon* m)
	{
		moons.push_back(m);
	}

	mat4 rotateAboutStar = mat4(1.f);
	/// <summary>
	/// The Planet orbits around its Star by 'orbitRate'.
	/// Updates the 'orbitalInclination' glm::mat4.
	/// </summary>
	void orbitCelestialBody(float time) override
	{
		// Do orbital inclination
		mat4 rotateAboutZWithAlpha = rotate(mat4(1.f), glm::radians(0.f), vec3(0.f, 0.f, 1.f));	// alpha is a constant: 0.f

		mat4 p0 = mat4(1.f);
		//mat4 rotateAboutStar = rotate(mat4(1.f), glm::radians(orbitRate * time), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its star. (Time dependent)
		rotateAboutStar = rotate(rotateAboutStar, glm::radians(orbitRate * time), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its star. (Time dependent)
		mat4 translateDistanceFromOrbitingCelestialBody = translate(mat4(1.f), vec3(vec2(0.f), distanceFromOrbitingCelestialBody));
		p0 = rotateAboutStar * translateDistanceFromOrbitingCelestialBody;
		//p0 =  translateDistanceFromOrbitingCelestialBody * rotateAboutStar;	// this does not give us the orbit correctly.


		//orbitalInclination = translate(p0, vec3(0.f)) * rotateAboutZWithAlpha;//vec3(vec2(0.f), distanceFromOrbitingCelestialBody));
		orbitalInclination = rotateAboutZWithAlpha * translate(p0, vec3(0.f));//vec3(vec2(0.f), distanceFromOrbitingCelestialBody));
	}

	/// <summary>
	/// Returns the model-matrix for this Planet. After it does its axial rotation and orbit about its star.
	/// </summary>
	/// <returns></returns>
	virtual mat4 getModelMat4() override
	{
		/*
		First rotate celestial body about the z - axis to get the 'axial tilt',
		then do the axial rotation (about y-axis). Finally do the orbital inclination about the celestial body its orbiting (Star).
		*/
		return orbitalInclination * axialRotationMat4 * getCelestialBodyAxisTilt() * model;
	}

	void generateRing(float distanceFromPlanet, float thicknessOfRing)
	{
		haveRings = true;

		vector<vec3> chaikinCurvePts = { vec3(distanceFromPlanet + radius, vec2(0.f)), vec3(distanceFromPlanet + radius + thicknessOfRing, vec2(0.f))};
		for (int i = 0; i < NUMBER_OF_ITERATIONS_FOR_CHAIKIN; i++)
			chaikinCurvePts = chaikinOpen(chaikinCurvePts);

		auto [meshPoints, meshTriangles] = surfaceOfRevolution(chaikinCurvePts, NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION);
		cpu_geom.verts = meshTriangles;
		
	}

	bool hasRings()
	{
		return haveRings;
	}

	vector<Moon*> moons;
	Star& orbitingStar;
	float orbitRate;
	CPU_Geometry cpu_geom_ring;
	GPU_Geometry gpu_geom_ring;

private:
	int numOrbitCelestialBodyCalls = 0;
	bool haveRings = false;
};

class Moon : public CelestialBody
{
public:

	/// <summary>
	/// Note: The distance of the Moon from its orbiting Planet will be the: planet's radius + 'distanceFromOrbitingCelestialBody'.
	/// </summary>
	/// <param name="name"></param>
	/// <param name="radius"></param>
	/// <param name="distanceFromOrbitingCelestialBody"></param>
	/// <param name="axisTilt"></param>
	/// <param name="axisRotation"></param>
	/// <param name="orbitRate"></param>
	/// <param name="orbitingPlanet"></param>
	/// <param name="texturePath"></param>
	Moon(string name, float radius, float distanceFromOrbitingCelestialBody, float axisTilt, float axisRotation, float orbitRate, Planet& orbitingPlanet, string texturePath) :
		CelestialBody(name, radius, distanceFromOrbitingCelestialBody + orbitingPlanet.radius, axisTilt, axisRotation, texturePath),
		orbitingPlanet(orbitingPlanet),
		orbitRate(orbitRate)
	{
		type = TYPES_OF_CELESTIAL_BODIES[1];
	}

	mat4 rotateAboutPlanet = mat4(1.f);
	/// <summary>
	/// The Moon orbits around its Planet by 'orbitRate'.
	/// Updates the 'orbitalInclination' glm::mat4.
	/// </summary>
	void orbitCelestialBody(float time) override
	{
		// Create orbital inclination
		mat4 rotateAboutZWithAlpha = rotate(mat4(1.f), glm::radians(0.f), vec3(0.f, 0.f, 1.f));	// alpha is a constant: 0.f

		mat4 p0 = mat4(1.f);
		//mat4 rotateAboutPlanet = rotate(mat4(1.f), glm::radians(orbitRate * time), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its planet. (Time dependent)
		rotateAboutPlanet = rotate(rotateAboutPlanet, glm::radians(orbitRate * time), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its planet. (Time dependent)
		mat4 translateDistanceFromOrbitingCelestialBody = translate(mat4(1.f), vec3(vec2(0.f), distanceFromOrbitingCelestialBody));	// distance from Moon to planet
		p0 = rotateAboutPlanet * translateDistanceFromOrbitingCelestialBody;
		//p0 =  translateDistanceFromOrbitingCelestialBody * rotateAboutStar;	// this does not give us the orbit correctly.


		//orbitalInclination = translate(p0, vec3(0.f)) * rotateAboutZWithAlpha;//vec3(vec2(0.f), distanceFromOrbitingCelestialBody));
		orbitalInclination = rotateAboutZWithAlpha * translate(p0, vec3(0.f));//vec3(vec2(0.f), distanceFromOrbitingCelestialBody));
	}

	/// <summary>
	/// Returns the model-matrix for this Moon.
	/// </summary>
	/// <returns></returns>
	virtual mat4 getModelMat4() override
	{
		/*
		First rotate celestial body about the z - axis to get the 'axial tilt',
		then do the axial rotation (about y-axis). Next do the orbital inclination about the celestial body its orbiting (Planet).
		Finally do the orbital inclination of the orbiting celestial body (Planet's orbital inclination).
		*/
		return orbitingPlanet.getOrbitalInclination() * orbitalInclination * axialRotationMat4 * getCelestialBodyAxisTilt() * model; // should be correct.
	}

	Planet& orbitingPlanet;
	float orbitRate;
};

/// <summary>
/// Stores relevant data for the GUI and window.
/// </summary>
struct windowData
{
	bool& pause;
	float simulationSpeedMultiplier;
	float scrollSensitivity;
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
		ImGui::SliderFloat("Simulation Speed Multiplier", &simulationSpeedMultiplier, MIN_SIMULATION_RATE, MAX_SIMULATION_RATE, "Current multiplier: %.3f");

		ImGui::Combo("Center Camera", &comboSelection, options, IM_ARRAYSIZE(options));

		// Show planets
		if (comboSelection == 0)
		{
			ImGui::Combo("Select a Planet", &planetSelection, planetOptions, IM_ARRAYSIZE(planetOptions));
			//cameraFocusedOnBodyStr = planetOptions[planetSelection];
			celestialBodyFocusedOn = &planetObjects[planetSelection];
			//camera.lookAt = planetObjects[planetSelection].position;
			//camera.lookAt = celestialBodyFocusedOn->position;
		}
		// Show Moons
		else if (comboSelection == 1)
		{
			ImGui::Combo("Select a Moon", &moonSelection, moonOptions, IM_ARRAYSIZE(moonOptions));
			celestialBodyFocusedOn = &moonObjects[moonSelection];
			//cameraFocusedOnBodyStr = moonOptions[moonSelection];
			//camera.lookAt = moonObjects[moonSelection].position;
			//camera.lookAt = celestialBodyFocusedOn->position;
		}
		else
		{
			//cameraFocusedOnBodyStr = windowData.sun.name;
			celestialBodyFocusedOn = &windowData.sun;
			//camera.lookAt = windowData.sun.position;
			//camera.lookAt = celestialBodyFocusedOn->position;
		}

		if (previousComboSelection != comboSelection)
		{
			cout << "reset camera values" << endl;
			previousComboSelection = comboSelection;
			resetCameraValues();
			//float r = calcCameraRadiusFromCelestialBodyPosition(celestialBodyFocusedOn->position);
			//camera.setRadius(r);
		}

		string s = "Camera focused on: ";
		s.append(celestialBodyFocusedOn->name);
		ImGui::Text(s.c_str());

		//vec3 celestialBodyPos = celestialBodyFocusedOn->position;
		//ImGui::Text("Position: (%.3f, %.3f, %.3f)", celestialBodyPos.x, celestialBodyPos.y, celestialBodyPos.z);
		ImGui::Text("Radius: %.3f", celestialBodyFocusedOn->radius);

		// Camera Values
		ImGui::Text("---Camera Values---");
		vec3 cameraPos = camera.getPos();
		ImGui::Text("Position: (%.3f, %.3f, %.3f)", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("Radius: %.3f", camera.getRadius());
		ImGui::Text("Theta: %.3f", camera.getTheta() * 180.f);
		ImGui::Text("Phi: %.3f", camera.getPhi());

		ImGui::Checkbox("Pause", &windowData.pause);
		ImGui::SliderFloat("Scroll Sensitivity", &windowData.scrollSensitivity, MIN_SCROLL_SENSITIVITY, MAX_SCROLL_SENSITIVITY, "Current sensitivity: %.3f");


		ImGui::Text("---DEBUG---");
		Planet& earth = windowData.planets[2];
		//vec3 earthPos = earth.position;
		//ImGui::Text("Earth pos: (%.3f, %.3f, %.3f)", earthPos.x, earthPos.y, earthPos.z);

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

	float& simulationSpeedMultiplier = windowData.simulationSpeedMultiplier;
	string cameraFocusedOnBodyStr = windowData.sun.name;	// initialy set it focused on the sun
	int frames = 10;
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
		camera(glm::radians(45.f), glm::radians(45.f), INITIAL_CAMERA_RADIUS, windowData.lookAt, windowData.scrollSensitivity, MAX_CAMERA_RADIUS)
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

	/// <summary>
	/// Sets the Model, View, and Perspective uniform matrices,
	/// and also other necessary uniforms for the planet and moon shader.
	/// </summary>
	/// <param name="sp"></param>
	void viewPipeline(ShaderProgram &sp)
	{
		sp.use();
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));

		// Set shading uniforms for this shader 
		vec3 lightPos = vec3(0.f);
		GLint uniVec3 = glGetUniformLocation(sp, "lightPosition");
		glUniform3fv(uniVec3, 1, glm::value_ptr(vec3(0.f)));
		uniVec3 = glGetUniformLocation(sp, "viewPos");
		glUniform3fv(uniVec3, 1, glm::value_ptr(camera.getPos()));
		uniVec3 = glGetUniformLocation(sp, "lightColour");
		glUniform3fv(uniVec3, 1, glm::value_ptr(LIGHT_COLOUR));	

		GLint uniFloat = glGetUniformLocation(sp, "diffuseCoefficient");
		glUniform1fv(uniFloat, 1, &diffuseCoeff);
		uniFloat = glGetUniformLocation(sp, "specularCoefficient");
		glUniform1fv(uniFloat, 1, &specularCoeff);
		uniFloat = glGetUniformLocation(sp, "ambientCoefficient");
		glUniform1fv(uniFloat, 1, &ambientCoeff);
		uniFloat = glGetUniformLocation(sp, "shininessCoefficient");
		glUniform1fv(uniFloat, 1, &shininessCoeff);
	}

	void setPlanetModelMat(ShaderProgram& sp, CelestialBody& p)
	{
		mat4 M = p.getModelMat4();
		mat4 V = camera.getView();
		mat4 P = glm::perspective(glm::radians(45.0f), aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void setSunLighting(ShaderProgram& sp, CelestialBody& sun)
	{
		if (typeid(sun) != typeid(Star))	// Error check if i'm lighting the sun and not someting else.
		{
			cerr << "\nERROR: Can not 'setSunLighting()' on non Star instance." << endl;
			abort();
		}

		mat4 M = sun.getModelMat4();
		mat4 V = camera.getView();
		mat4 P = glm::perspective(glm::radians(45.0f), aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);
		
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));

		// Set shading uniforms for this shader 
		GLint uniVec3 = glGetUniformLocation(sp, "lightColour");
		glUniform3fv(uniVec3, 1, glm::value_ptr(LIGHT_COLOUR));

		GLfloat uniFloat = glGetUniformLocation(sp, "ambientCoefficient");
		glUniform1fv(uniFloat, 1, &SUN_AMBIENT_COEFF);
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
	sp.use();

	if (typeid(cb) == typeid(Star))
		callBack->setSunLighting(sp, cb);
	else
		callBack->setPlanetModelMat(sp, cb);

	cb.gpu_geom.bind();
	cb.texture.bind();
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(cb.cpu_geom.verts.size()));
	cb.texture.unbind();
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 3");
	Panel panel(window.getGLFWwindow());
	
	float simulationSpeedMultiplier = 2.f;
	float sensitivity = 10.f;
	vector<Planet> planets;
	vector<Moon> moons;
	vec3 lookAt = vec3(0.f);
	bool pause = false;

	float sunRadius = 10.f;
	float sunAxialTilt = 0.f;
	float sunAxialRotation = 2.f;
	windowData windowData = {pause, simulationSpeedMultiplier, sensitivity, planets, moons, Star("Sun", sunRadius, sunAxialTilt, sunAxialRotation, "textures/sun.jpg"), lookAt };
	
	GLDebug::enable();

	// Create skybox
	float skyBoxRadius = MAX_CAMERA_RADIUS + 100.f;			// Make sky box larger than the max radius of camera so it does not go out of the sky box.
	float distanceFromOrbitingCelestialBodySkyBox = 0.f;
	float axialTilt = 0.f;
	float axialRotationRate = 0.f;
	CelestialBody skyBox = CelestialBody("Sky Box", skyBoxRadius, distanceFromOrbitingCelestialBodySkyBox, axialTilt, axialRotationRate, SKYBOX_TEXTURE_PATH);

	// Create Planets and Moons
	float axisTilt = 45.f;
	//float axisTilt = 90.f;
	float axisRotation = 10.f; // degrees
	float orbitRate = 5.0f;	// degrees
	const vector<float> radiusOfPlanets = { 0.75, 2.f, 2.25, 1.5, 5.f, 4.f, 3.f, 3.f };	// left to right: mercury, venus, earth, ..., neptune radius's
	const vector<float> distanceFromSun = { 3.f, 4.f, 10.f, 17.f, 25.f, 35.f, 55.f, 80.f };	// 1st: sun-mercury, 2nd: sun-venus, ..., 8th: sun-neptune
	const float distanceConst = 5.5f;	// distance from sun center to its surface.
	
	// Create Planets
	planets.push_back(Planet(PLANET_NAMES[0], radiusOfPlanets[0], distanceFromSun[0] + distanceConst, axisTilt, axisRotation, orbitRate, windowData.sun, PLANET_TEXUTRE_PATHS[0]));
	for (int i = 1; i < PLANET_NAMES.size(); i++)
	{
		planets.push_back(Planet(PLANET_NAMES[i], radiusOfPlanets[i], distanceFromSun[i] + planets[i-1].distanceFromOrbitingCelestialBody, axisTilt, axisRotation, orbitRate, windowData.sun, PLANET_TEXUTRE_PATHS[i]));
	}

	float moonRadius = 0.5;
	float axisTiltMoon = 40.f;
	float axisRotationMoon = 10.f; // degrees
	float orbitRateMoon = 5.0f;	// degrees
	const float distanceFromOrbitingPlanet = 2.f;

	moons.push_back(Moon("The Moon", moonRadius, distanceFromOrbitingPlanet, axisTiltMoon, axisRotationMoon, orbitRateMoon, planets[2], MOON_TEXTURE_PATH));
	Moon* moon = &moons[0];
	planets[2].addMoon(&moons[0]);
	
	// Mars moons
	int indexCounter = 1;
	for (int j = 0; j < MARS_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(MARS_MOON_NAMES[j], moonRadius / (j + 1), distanceFromOrbitingPlanet / (j + 1), axisTiltMoon, axisRotationMoon, orbitRateMoon + (2 * (j + 1)), planets[3], MOON_TEXTURE_PATH));
		//moons[indexCounter].translateBody(vec3(vec2(0.f), planets[3].radius + (2.f / (j+1))));
	}
	// Set the Moons for mars
	planets[3].addMoon(&moons[moons.size() - 2]);
	planets[3].addMoon(&moons[moons.size() - 1]);

	// Jupiter moons
	for (int j = 0; j < JUPITER_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(JUPITER_MOON_NAMES[j], moonRadius / (j + 1), distanceFromOrbitingPlanet / (j + 1), axisTiltMoon, axisRotationMoon, orbitRateMoon + (2 * (j + 1)), planets[4], MOON_TEXTURE_PATH));
		//moons[indexCounter].translateBody(vec3(vec2(0.f), planets[4].radius + (2.f / (j + 1))));
	}
	// Set the Moons for Jupiter
	planets[4].addMoon(&moons[moons.size() - 3]);
	planets[4].addMoon(&moons[moons.size() - 2]);
	planets[4].addMoon(&moons[moons.size() - 1]);

	// Saturn moons
	for (int j = 0; j < SATURN_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(SATURN_MOON_NAMES[j], moonRadius / (j + 1), distanceFromOrbitingPlanet / (j + 1), axisTiltMoon, axisRotationMoon, orbitRateMoon + (2 * (j + 1)), planets[5], MOON_TEXTURE_PATH));
		//moons[indexCounter].translateBody(vec3(vec2(0.f), planets[5].radius + (2.f / (j + 1))));
	}
	// Set the Moons for Saturn
	planets[5].addMoon(&moons[moons.size() - 3]);
	planets[5].addMoon(&moons[moons.size() - 2]);
	planets[5].addMoon(&moons[moons.size() - 1]);

	// Uranus moons
	for (int j = 0; j < URANUS_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(URANUS_MOON_NAMES[j], moonRadius / (j + 1), distanceFromOrbitingPlanet / (j + 1), axisTiltMoon, axisRotationMoon, orbitRateMoon + (2 * (j + 1)), planets[6], MOON_TEXTURE_PATH));
		//moons[indexCounter].translateBody(vec3(vec2(0.f), planets[6].radius + (2.f / (j + 1))));
	}
	// Set the Moons for Uranus
	planets[6].addMoon(&moons[moons.size() - 3]);
	planets[6].addMoon(&moons[moons.size() - 2]);
	planets[6].addMoon(&moons[moons.size() - 1]);

	// Neptune moons
	for (int j = 0; j < NEPTUNE_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(NEPTUNE_MOON_NAMES[j], moonRadius / (j + 1), distanceFromOrbitingPlanet / (j + 1), axisTiltMoon, axisRotationMoon, orbitRateMoon + (2 * (j + 1)), planets[7], MOON_TEXTURE_PATH));
		//moons[indexCounter].translateBody(vec3(vec2(0.f), planets[7].radius + (2.f / (j + 1))));
	}
	// Set the Moons for Neptune
	planets[7].addMoon(&moons[moons.size() - 3]);
	planets[7].addMoon(&moons[moons.size() - 2]);
	planets[7].addMoon(&moons[moons.size() - 1]);

	// Add more orbital variation for the moons
	float variance = 0.2;
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(0.1, 1.0);
	for (int j = 0; j < moons.size(); j++)
	{
		float varianceMultiplier = dis(gen);
		moons[j].orbitRate += (variance * varianceMultiplier);
	}

	// CALLBACKS
	shared_ptr<Assignment4> callBack = std::make_shared<Assignment4>(windowData);
	callBack->camera.lookAt = windowData.lookAt;
	window.setCallbacks(callBack);

	auto gui_panel_renderer = std::make_shared<GuiPanel>(windowData, callBack->camera);
	panel.setPanelRenderer(gui_panel_renderer);

	ShaderProgram shaderForPlanetsMoonsSkyBox("shaders/test.vert", "shaders/test.frag");
	ShaderProgram shaderForSun("shaders/sun.vert", "shaders/sun.frag");

	float currentTime;
	float lastTime = glfwGetTime();

	//UnitCube cube;
	//cube.generateGeometry();

	// RENDER LOOP
	while (!window.shouldClose())
	{
		currentTime = glfwGetTime();
		float deltaTime = abs(currentTime - lastTime) * windowData.simulationSpeedMultiplier;
		lastTime = glfwGetTime();

		//cout << "Delta time: " << deltaTime << endl;	// debug

		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL /*GL_LINE*/);		// texture
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE /*GL_LINE*/);	// wireframe

		callBack->viewPipeline(shaderForPlanetsMoonsSkyBox);

		// cube
		// (WILL CAUSE ISSUES IF GL_FILL (TEXTURES) IS USED)
		//cube.m_gpu_geom.bind();
		//glDrawArrays(GL_TRIANGLES, 0, GLsizei(cube.m_size));


		// Render celestial bodies
		renderCelestialBody(callBack, shaderForPlanetsMoonsSkyBox, skyBox);

		renderCelestialBody(callBack, shaderForSun, windowData.sun);
		if (!pause)
			windowData.sun.rotateViaCelestialBodyAxis(deltaTime);

		// Testing with only Earth and Moon
		//Planet& earth = planets[2];
		//earth.rotateViaCelestialBodyAxis(deltaTime);
		//earth.orbitCelestialBody(deltaTime);
		//renderCelestialBody(callBack, shader, earth);

		//Moon& moon = moons[0];
		//moon.rotateViaCelestialBodyAxis(deltaTime);
		//moon.orbitCelestialBody(deltaTime);
		//renderCelestialBody(callBack, shader, moon);

		// for bonus
		for (Planet& p : windowData.planets)
		{
			renderCelestialBody(callBack, shaderForPlanetsMoonsSkyBox, p);

			if (!pause)
			{
				p.rotateViaCelestialBodyAxis(deltaTime);
				p.orbitCelestialBody(deltaTime);
			}
		}

		for (Moon& m : windowData.moons)
		{
			renderCelestialBody(callBack, shaderForPlanetsMoonsSkyBox, m);

			if (!pause)
			{
				m.rotateViaCelestialBodyAxis(deltaTime);
				m.orbitCelestialBody(deltaTime);
			}
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		panel.render();
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
