
/* CPSC 453 A4 Virtual Orrery and Real Time Rendering | Benny Liang 30192142 */

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

const static double PI_APPROX = atan(1) * 4;	// pi approximation

const static float MIN_SIMULATION_RATE = 0.25f;
const static float MAX_SIMULATION_RATE = 1000.f;
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
const static string SATURN_RING_TEXTURE_PATH = "textures/saturn_ring.png";
//const static string EARTH_CLOUD_TEXTURE_PATH = "textures/8k_earth_clouds.jpg";
//const static string EARTH_CLOUD_TEXTURE_PATH = "textures/earth_clouds.png";	// noo like
//const static string EARTH_CLOUD_TEXTURE_PATH = "textures/ecl.png";	// no like
const static string EARTH_CLOUD_TEXTURE_PATH = "textures/idk.png";	// no like


const static int segmentsForRing = 60;

const vector<string> MARS_MOON_NAMES = { "Phobos", "Deimos" };
const vector<string> JUPITER_MOON_NAMES = { "Io", "Europa", "Ganymede" };	// 3 largest
const vector<string> SATURN_MOON_NAMES = { "Titan", "Rhea", "Iapetus" };		// 3 largest
const vector<string> URANUS_MOON_NAMES = { "Titania", "Oberon", "Ariel" };	// 3 largest
const vector<string> NEPTUNE_MOON_NAMES = { "Triton", "Nereid", "Proteus" };	// 3 largest

struct windowData;

/// <summary>
/// Used for Transformation Hierarchy.
/// </summary>
class Transformations
{
public:
	Transformations(float axisTilt, float axialRotationRate, float orbitRate) :
		axisTilt(axisTilt),
		axialRotationRate(axialRotationRate),
		orbitRate(orbitRate)
	{
		//axialTiltMat4 = glm::rotate(mat4(1.f), glm::radians(axisTilt), vec3(0.f, 0.f, 1.f));		// tilts in the z-axis. (Use x-axis tilt, for some reason z-axis tilt doesn't give the desired result).
		axialTiltMat4 = glm::rotate(mat4(1.f), glm::radians(axisTilt), vec3(1.f, 0.f, 0.f));		// tilts in the x-axis (need to use this or else it wont look quite right)
	}

	/// <summary>
	/// Rotates the CelestialBody by 'axialRotationRate' * 'deltaTime' about it's planetary axis (y-axis).
	/// </summary>
	void rotateViaCelestialBodyAxis(float deltaTime)
	{
		axialRotationMat4 = glm::rotate(axialRotationMat4, glm::radians(axialRotationRate * deltaTime), vec3(0.f, 1.f, 0.f));	// time dependent
	}

	// to be overridden in subclasses
	virtual void orbitCelestialBody(float deltaTime) {}

	// to be overridden in subclasses
	virtual mat4 getModelMat4()
	{
		return axialRotationMat4 * getCelestialBodyAxisTilt() * model;
	}

	mat4 getOrbitalInclination()
	{
		return orbitalInclination;
	}

	mat4 getCelestialBodyAxisTilt()
	{
		return axialTiltMat4;
	}

	float getAxisTilt()
	{
		return axisTilt;
	}

	float getAxialRotationRate()
	{
		return axialRotationRate;
	}

	float getOrbitRate()
	{
		return orbitRate;
	}

	/// <summary>
	/// Resets all (except the 'axialTiltMat4') mat4's.
	/// </summary>
	void resetTransformations()
	{
		model = mat4(1.0f);
		axialRotationMat4 = mat4(1.0f);
		orbitalInclination = mat4(1.0f);
		orbitAboutOrbitingCelestialBody = mat4(1.f);
	}

protected:
	mat4 model = mat4(1.0f);
	mat4 axialTiltMat4 = mat4(1.f);
	mat4 axialRotationMat4 = mat4(1.0f);	// stores the celestial body's axial rotation (about y-axis)
	mat4 orbitalInclination = mat4(1.0f);
	mat4 orbitAboutOrbitingCelestialBody = mat4(1.f);	// Stores the total y-axis rotation of a CelestialBody 'A' about the CelestialBody 'B' that 'A' is orbiting. For example, a Planet orbiting (y-axis rotation) about its Star
	float axisTilt;							// In degrees (About the x-axis, instead of z-axis)
	float axialRotationRate;				// How much to rotate about it's axis, in degrees.
	float orbitRate;						// How much to orbit about its orbiting CelestialBody
};

class CelestialBody : public Transformations
{
public:
	// type is the type of celestial body, moon, planet, star etc.
	// 0: planet
	// 1: moon
	// 2: star
	CelestialBody(string name, float radius, float distanceFromOrbitingCelestialBody, float axisTilt, float axialRotationRate, float orbitRate, string texturePath) :
		Transformations(axisTilt, axialRotationRate, orbitRate),
		name(name),
		radius(radius),
		distanceFromOrbitingCelestialBody(distanceFromOrbitingCelestialBody),
		texturePath(texturePath),
		texture(texturePath, GL_NEAREST)
	{
		// Generate sphere
		generateGeometryUsingSphericalCoords();
	}

	string name;	// Name of the Celestial body
	float radius;
	float distanceFromOrbitingCelestialBody;
	vec3 position = vec3(0.f);
	string type;
	string texturePath;

	Texture texture;
	CPU_Geometry cpu_geom;
	GPU_Geometry gpu_geom;

protected:

	/// <summary>
	/// Generates a sphere using spherical coordinate (vec3's), texture coordinates (vec2's) and normals (vec3's).
	/// </summary>
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

				tempTextCoords.push_back(vec2(1.f - u, v));	// 1.f - u to map texture correctly (or else it will look 'fliped')
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
		cpu_geom.normals = normals;
		cpu_geom.textCoords = sphereTextCoordMesh;

		gpu_geom.setVerts(cpu_geom.verts);
		gpu_geom.setNormals(cpu_geom.normals);
		gpu_geom.setTexCoords(cpu_geom.textCoords);

		// debug
		std::cout << "\n" << name << ":" << std::endl;
		std::cout << "Verts size: " << cpu_geom.verts.size() << std::endl;
		std::cout << "TexCoords size: " << cpu_geom.textCoords.size() << std::endl;
		std::cout << "normals size: " << cpu_geom.normals.size() << std::endl;
	}
};

class Star : public CelestialBody
{
public:
	Star(string name, float radius, float axisTilt, float axisRotation, string texturePath) :
		CelestialBody(name, radius, 0.f, axisTilt, axisRotation, 0.f, texturePath)
	{
		type = TYPES_OF_CELESTIAL_BODIES[2];
	}

	virtual mat4 getModelMat4() override
	{
		return axialRotationMat4 * getCelestialBodyAxisTilt() * model;
	}
};

class Moon;	// prototype

class Planet : public CelestialBody
{
private:
	bool haveRings = false;
	bool haveClouds = false;

	class PlanetaryRing : public Transformations
	{
	public:
		PlanetaryRing(Planet* orbitingPlanet, string texturePath) :
			Transformations(0.f, 0.f, 0.f),
			orbitingPlanet(orbitingPlanet),
			distanceFromPlanet(-1.f),
			thicknessOfRing(-1.f),
			ringTexture(texturePath, GL_NEAREST)
		{}
		PlanetaryRing(Planet* orbitingPlanet, float orbitRate, float axialTilt, float axialRotationRate, float distanceFromPlanet, float thicknessOfRing, string texturePath) :
			Transformations(axialTilt, axialRotationRate, orbitRate),
			orbitingPlanet(orbitingPlanet),
			distanceFromPlanet(distanceFromPlanet),
			thicknessOfRing(thicknessOfRing),
			ringTexture(texturePath, GL_NEAREST)
		{
			// Generate the ring
			float planetRadius = orbitingPlanet->radius;
			float radiusOfInnerCircle = distanceFromPlanet + planetRadius;
			float radiusOfOutterCircle = distanceFromPlanet + planetRadius + thicknessOfRing;
			vector<vec3> innerRing;
			vector<vec3> outerRing;
			vector<vec2> texCoordInner;
			vector<vec2> texCoordOuter;

			// Generate points on the circle
			for (int i = 0; i < segmentsForRing; ++i)
			{
				// Angle for the current segment
				float angle = (2.0f * PI_APPROX * i) / segmentsForRing;

				// Calculate the position of ring points
				float xInner = radiusOfInnerCircle * std::cos(angle);
				float yInner = radiusOfInnerCircle * std::sin(angle);

				float xOuter = radiusOfOutterCircle * std::cos(angle);
				float yOuter = radiusOfOutterCircle * std::sin(angle);

				float z = 0.0f; // Z is zero for flat ring

				innerRing.push_back(vec3(xInner, yInner, z));
				outerRing.push_back(vec3(xOuter, yOuter, z));

				// texture coords
				float u = (float)i / segmentsForRing;	// (horizontal axis)
				float v = 0.f;					// bottom of texture (vertical axis)

				texCoordInner.push_back(vec2(u, v));
				v = 1.f;						// top of texture (vertical axis)
				texCoordOuter.push_back(vec2(u, v));
			}

			vector<vec3> triangleMesh;
			// Connect the ring's vec3's to generate mesh
			for (int i = 0; i < innerRing.size() - 1; i++)
			{
				// First triangle
				triangleMesh.push_back(outerRing[i]);
				triangleMesh.push_back(innerRing[i]);
				triangleMesh.push_back(outerRing[i + 1]);
				// second triangle
				triangleMesh.push_back(innerRing[i]);
				triangleMesh.push_back(outerRing[i + 1]);
				triangleMesh.push_back(innerRing[i + 1]);
			}

			// Add the last triangle connecting back to the first triangle
			triangleMesh.push_back(outerRing[innerRing.size() - 1]);
			triangleMesh.push_back(innerRing[innerRing.size() - 1]);
			triangleMesh.push_back(outerRing[0]);

			triangleMesh.push_back(innerRing[innerRing.size() - 1]);
			triangleMesh.push_back(outerRing[0]);
			triangleMesh.push_back(innerRing[0]);

			vector<vec2> textureMesh;
			// Connect the texture vec2's to generate mesh
			for (int i = 0; i < innerRing.size() - 1; i++)
			{
				// First triangle
				textureMesh.push_back(texCoordOuter[i]);
				textureMesh.push_back(texCoordInner[i]);
				textureMesh.push_back(texCoordOuter[i + 1]);
				// second triangle
				textureMesh.push_back(texCoordInner[i]);
				textureMesh.push_back(texCoordOuter[i + 1]);
				textureMesh.push_back(texCoordInner[i + 1]);
			}

			// Add the last triangle connecting back to the first triangle
			textureMesh.push_back(texCoordOuter[innerRing.size() - 1]);
			textureMesh.push_back(texCoordInner[innerRing.size() - 1]);
			textureMesh.push_back(texCoordOuter[0]);

			textureMesh.push_back(texCoordInner[innerRing.size() - 1]);
			textureMesh.push_back(texCoordOuter[0]);
			textureMesh.push_back(texCoordInner[0]);

			// generate normals
			vector<vec3> normals;
			for (vec3 v : triangleMesh)
				normals.push_back(normalize(v));

			// Set geom values
			cpu_geom_ring.verts = triangleMesh;
			cpu_geom_ring.textCoords = textureMesh;
			cpu_geom_ring.normals = normals;

			gpu_geom_ring.setVerts(cpu_geom_ring.verts);
			gpu_geom_ring.setTexCoords(cpu_geom_ring.textCoords);
			gpu_geom_ring.setNormals(cpu_geom_ring.normals);
		};

		virtual void orbitCelestialBody(float deltaTime) override
		{
			// Create orbital inclination
			mat4 rotateAboutZWithAlpha = rotate(mat4(1.f), glm::radians(0.f), vec3(0.f, 0.f, 1.f));	// alpha is a constant: 0.f

			mat4 p0 = mat4(1.f);
			orbitAboutOrbitingCelestialBody = rotate(orbitAboutOrbitingCelestialBody, glm::radians(orbitRate * deltaTime), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its planet. (Time dependent)
			mat4 translateDistanceFromOrbitingCelestialBody = translate(mat4(1.f), vec3(0.f, 0.f, 0.f));	// distance from ring to planet is 0 (due to how i generated the rings)
			p0 = orbitAboutOrbitingCelestialBody * translateDistanceFromOrbitingCelestialBody;

			orbitalInclination = rotateAboutZWithAlpha * translate(p0, vec3(0.f));
		}

		/// <summary>
		/// Returns a mat4 containing the result of, its axial tilt, axial rotation, orbitalInclination about the planet, and the planet's orbitalInclination
		/// </summary>
		/// <returns></returns>
		virtual mat4 getModelMat4() override
		{
			return orbitingPlanet->getOrbitalInclination() * orbitalInclination * axialRotationMat4 * axialTiltMat4 * model;
		}

		Planet* orbitingPlanet;

		CPU_Geometry cpu_geom_ring;
		GPU_Geometry gpu_geom_ring;
		Texture ringTexture;

	private:
		float distanceFromPlanet;
		float thicknessOfRing;
	};

	class Clouds : public CelestialBody
	{
	public:
		Clouds(Planet* orbitingPlanet, string texturePath) :
			CelestialBody("NULL_CLOUD", 0.f, 0.f, 0.f, 0.f, 0.f, texturePath),
			orbitingPlanet(orbitingPlanet),
			cloudTexture(texturePath, GL_NEAREST)
		{
		}
		Clouds(Planet* orbitingPlanet, string name, float radius, float distanceFromOrbitingCelestialBody, float axisRotation, string texturePath) :
			CelestialBody(name, radius, distanceFromOrbitingCelestialBody, 0.f, axisRotation, 0.f, texturePath),
			orbitingPlanet(orbitingPlanet),
			cloudTexture(texturePath, GL_NEAREST)
		{
		}

		virtual void orbitCelestialBody(float deltaTime) override
		{
			// Create orbital inclination
			mat4 rotateAboutZWithAlpha = rotate(mat4(1.f), glm::radians(0.f), vec3(0.f, 0.f, 1.f));	// alpha is a constant: 0.f

			mat4 p0 = mat4(1.f);
			orbitAboutOrbitingCelestialBody = rotate(orbitAboutOrbitingCelestialBody, glm::radians(orbitRate * deltaTime), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its planet. (Time dependent)
			mat4 translateDistanceFromOrbitingCelestialBody = translate(mat4(1.f), vec3(0.f, 0.f, 0.f));	// distance from ring to planet is 0 (due to how i generated the rings)
			p0 = orbitAboutOrbitingCelestialBody * translateDistanceFromOrbitingCelestialBody;

			orbitalInclination = rotateAboutZWithAlpha * translate(p0, vec3(0.f));
		}

		/// <summary>
		/// Returns a mat4 containing the result of, its axial tilt, axial rotation, orbitalInclination about the planet, and the planet's orbitalInclination
		/// </summary>
		/// <returns></returns>
		virtual mat4 getModelMat4() override
		{
			return orbitingPlanet->getOrbitalInclination() * orbitalInclination * axialRotationMat4 * axialTiltMat4 * model;
		}

		Texture cloudTexture;
	private:
		Planet* orbitingPlanet;
	};

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
		CelestialBody(name, radius, distanceFromOrbitingCelestialBody + radius + sun.radius, axisTilt, axisRotation, orbitRate, texturePath),
		orbitingStar(sun)
	{
		type = TYPES_OF_CELESTIAL_BODIES[0];
	}

	void addMoon(Moon* m)
	{
		moons.push_back(m);
	}

	/// <summary>
	/// The Planet orbits around its Star by 'orbitRate' * 'deltaTime'.
	/// Updates the 'orbitalInclination' glm::mat4.
	/// </summary>
	virtual void orbitCelestialBody(float deltaTime) override
	{
		// Do orbital inclination
		mat4 rotateAboutZWithAlpha = rotate(mat4(1.f), glm::radians(0.f), vec3(0.f, 0.f, 1.f));	// alpha is a constant: 0.f

		mat4 p0 = mat4(1.f);
		orbitAboutOrbitingCelestialBody = rotate(orbitAboutOrbitingCelestialBody, glm::radians(orbitRate * deltaTime), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its star. (Time dependent)
		mat4 translateDistanceFromOrbitingCelestialBody = translate(mat4(1.f), vec3(vec2(0.f), distanceFromOrbitingCelestialBody));
		p0 = orbitAboutOrbitingCelestialBody * translateDistanceFromOrbitingCelestialBody;

		orbitalInclination = rotateAboutZWithAlpha * translate(p0, vec3(0.f));
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

	/// <summary>
	/// Generates a ring about this planet.
	/// </summary>
	/// <param name="orbitRate"></param>
	/// <param name="ringTilt"></param>
	/// <param name="axialRotationRate"></param>
	/// <param name="distanceFromPlanet"></param>
	/// <param name="thicknessOfRing"></param>
	/// <param name="texturePath"></param>
	void generateRing(float orbitRate, float ringTilt, float axialRotationRate, float distanceFromPlanet, float thicknessOfRing, string texturePath)
	{
		haveRings = true;
		ring = PlanetaryRing(this, orbitRate, ringTilt, axialRotationRate, distanceFromPlanet, thicknessOfRing, texturePath);
	}

	void generateClouds(float distanceFromPlanet, float axisRotation, string texturePath)
	{
		haveClouds = true;
		string cloudName = name + " clouds";
		clouds = Clouds(this, cloudName, radius + distanceFromPlanet, 0.f, axisRotation, texturePath);
	}

	bool hasRings()
	{
		return haveRings;
	}

	bool hasClouds()
	{
		return haveClouds;
	}

	vector<Moon*> moons;
	Star& orbitingStar;
	PlanetaryRing ring = PlanetaryRing(this, texturePath);
	Clouds clouds = Clouds(this, texturePath);
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
		CelestialBody(name, radius, distanceFromOrbitingCelestialBody + orbitingPlanet.radius, axisTilt, axisRotation, orbitRate, texturePath),
		orbitingPlanet(orbitingPlanet)
	{
		type = TYPES_OF_CELESTIAL_BODIES[1];
	}

	/// <summary>
	/// The Moon orbits around its Planet by 'orbitRate' * 'deltaTime'.
	/// Updates the 'orbitalInclination' glm::mat4.
	/// </summary>
	virtual void orbitCelestialBody(float deltaTime) override
	{
		// Create orbital inclination
		mat4 rotateAboutZWithAlpha = rotate(mat4(1.f), glm::radians(0.f), vec3(0.f, 0.f, 1.f));	// alpha is a constant: 0.f

		mat4 p0 = mat4(1.f);
		orbitAboutOrbitingCelestialBody = rotate(orbitAboutOrbitingCelestialBody, glm::radians(orbitRate * deltaTime), vec3(0.f, 1.f, 0.f));	// rotate about y-axis, the orbit about its planet. (Time dependent)
		mat4 translateDistanceFromOrbitingCelestialBody = translate(mat4(1.f), vec3(vec2(0.f), distanceFromOrbitingCelestialBody));	// distance from Moon to planet
		p0 = orbitAboutOrbitingCelestialBody * translateDistanceFromOrbitingCelestialBody;

		orbitalInclination = rotateAboutZWithAlpha * translate(p0, vec3(0.f));
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
};

/// <summary>
/// Stores relevant data for the GUI and window.
/// </summary>
struct windowData
{
	bool& pause;
	float simulationSpeedMultiplier;
	float scrollSensitivity;
	vector<Planet>& planets;	// Stores all the Planet instances.
	vector<Moon>& moons;		// Stores all the Moon instances.
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
		restartSim = false;
	}

	virtual void render() override
	{
		ImGui::SliderFloat("Simulation Speed Multiplier", &simulationSpeedMultiplier, MIN_SIMULATION_RATE, MAX_SIMULATION_RATE, "Current multiplier: %.3f");
		ImGui::SliderFloat("Scroll Sensitivity", &windowData.scrollSensitivity, MIN_SCROLL_SENSITIVITY, MAX_SCROLL_SENSITIVITY, "Current sensitivity: %.3f");

		ImGui::Checkbox("Pause", &windowData.pause);
		ImGui::Checkbox("Restart", &restartSim);

		if (restartSim)
			restartSimulation();

		// Camera Values
		ImGui::Text("Camera focused on: The Sun");
		ImGui::Text("The Sun's radius: %.3f", windowData.sun.radius);
		ImGui::Text("---Camera Values---");
		vec3 cameraPos = camera.getPos();
		ImGui::Text("Position: (%.3f, %.3f, %.3f)", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("Radius: %.3f", camera.getRadius());
		ImGui::Text("Theta: %.3f", camera.getTheta() * 180.f);
		ImGui::Text("Phi: %.3f", camera.getPhi());
	}

private:
	windowData& windowData;
	Camera& camera;
	float& simulationSpeedMultiplier = windowData.simulationSpeedMultiplier;
	bool restartSim;

	void restartSimulation()
	{
		restartSim = false;
		// reset transformations of the Planets (and rings if they have any)
		for (Planet& p : windowData.planets)
		{
			p.resetTransformations();

			// reset ring transformations
			if (p.hasRings())
				p.ring.resetTransformations();
		}

		//  reset transformations of the moons
		for (Moon& m : windowData.moons)
			m.resetTransformations();

		// reset transformation for the sun
		windowData.sun.resetTransformations();
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

	void setCelestialBodyMat(ShaderProgram& sp, CelestialBody& p)
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

	void setSunLightingAndModelForShader(ShaderProgram& sp, CelestialBody& sun)
	{
		if (typeid(sun) != typeid(Star))	// Error check if i'm lighting the sun and not something else.
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

	void setRingMat4ForShader(ShaderProgram& sp, Planet* p)
	{
		mat4 M = p->ring.getModelMat4();
		mat4 V = camera.getView();
		mat4 P = glm::perspective(glm::radians(45.0f), aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

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
	sp.use();

	if (typeid(cb) == typeid(Star))
		callBack->setSunLightingAndModelForShader(sp, cb);
	else
		callBack->setCelestialBodyMat(sp, cb);

	cb.gpu_geom.bind();
	cb.texture.bind();
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(cb.cpu_geom.verts.size()));
	cb.texture.unbind();

	// render ring if the planet has them
	Planet* planet = dynamic_cast<Planet*>(&cb);
	if (planet != nullptr)
	{
		if (planet->hasRings())
		{
			callBack->setRingMat4ForShader(sp, planet);

			planet->ring.gpu_geom_ring.bind();
			planet->ring.ringTexture.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(planet->ring.cpu_geom_ring.verts.size()));
			planet->ring.ringTexture.unbind();
		}

		if (planet->hasClouds())
		{
			callBack->setCelestialBodyMat(sp, planet->clouds);

			planet->clouds.gpu_geom.bind();
			planet->clouds.cloudTexture.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(planet->clouds.cpu_geom.verts.size()));
			planet->clouds.cloudTexture.unbind();
		}
	}
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
	windowData windowData = {pause, simulationSpeedMultiplier, sensitivity, planets, moons, Star("The Sun", sunRadius, sunAxialTilt, sunAxialRotation, "textures/sun.jpg"), lookAt };

	GLDebug::enable();

	// Create skybox
	float skyBoxRadius = MAX_CAMERA_RADIUS + 100.f;			// Make sky box larger than the max radius of camera so the camera does not go out of the sky box.
	float distanceFromOrbitingCelestialBodySkyBox = 0.f;
	float skyBoxAxialTilt = 0.f;
	float skyBoxAxialRotationRate = 0.f;
	float skyBoxOrbitRate = 0.f;
	CelestialBody skyBox = CelestialBody("Sky Box", skyBoxRadius, distanceFromOrbitingCelestialBodySkyBox, skyBoxAxialTilt, skyBoxAxialRotationRate, skyBoxOrbitRate, SKYBOX_TEXTURE_PATH);

	// Create Planets and Moons
	float axisTilt = 45.f;
	float axisRotationRate = 24.f; // degrees
	vector<float> axisRotationRates =
	{
		6.14f,   // Mercury: rotates ~6.14 degrees per Earth day
		-1.48f,  // Venus: rotates ~-1.48 degrees per Earth day (Venus rotates backwards)
		360.0f,  // Earth (reference for axial rotation rates)
		350.9f,  // Mars: rotates slightly slower than Earth
		870.5f,  // Jupiter
		810.0f,  // Saturn
		525.6f,  // Uranus
		536.3f,  // Neptune
	};
	float orbitRate = 1.f;	// degrees

	// orbit rates about the sun in degrees
	vector<float> orbitRatesAboutSun =
	{	4.15f,  // Mercury
		1.63f,  // Venus
		1.0f,   // Earth (reference for other orbit rates)
		0.53f,  // Mars
		0.084f, // Jupiter
		0.034f, // Saturn
		0.012f, // Uranus
		0.006f, // Neptune
	};

	const vector<float> radiusOfPlanets = { 0.75, 2.f, 2.25, 1.5, 5.f, 4.f, 3.f, 3.f };	// left to right: mercury, venus, earth, ..., neptune radius's
	const vector<float> distanceFromSun = { 3.f, 4.f, 10.f, 17.f, 25.f, 35.f, 55.f, 80.f };	// 1st: sun-mercury, 2nd: sun-venus, ..., 8th: sun-neptune
	const float distanceConst = 5.5f;	// distance from sun center to its surface.
	
	// Create Planets
	planets.push_back(Planet(PLANET_NAMES[0], radiusOfPlanets[0], distanceFromSun[0] + distanceConst, axisTilt, axisRotationRates[0], orbitRatesAboutSun[0], windowData.sun, PLANET_TEXUTRE_PATHS[0]));
	for (int i = 1; i < PLANET_NAMES.size(); i++)
	{
		planets.push_back(Planet(PLANET_NAMES[i], radiusOfPlanets[i], distanceFromSun[i] + planets[i-1].distanceFromOrbitingCelestialBody, axisTilt, axisRotationRates[i], orbitRatesAboutSun[i], windowData.sun, PLANET_TEXUTRE_PATHS[i]));
	}
	// Generate Earth clouds
	planets[2].generateClouds(2.f, 10.f, EARTH_CLOUD_TEXTURE_PATH);

	// Generate Saturn's ring
	float ringOrbitRate = planets[5].getOrbitRate();
	float ringTilt = 70.f;
	float ringAxialRotationRate = planets[5].getOrbitRate(); //planets[5].axialRotationRate;
	float distanceFromPlanet = 5.f;
	float ringThickness = 7.f;
	planets[5].generateRing(ringOrbitRate, ringTilt, ringAxialRotationRate, distanceFromPlanet, ringThickness, SATURN_RING_TEXTURE_PATH);

	float moonRadius = 0.5;
	float axisTiltMoon = 40.f;
	float axisRotationMoon = 10.f;						// degrees
	vector<float> moonOrbitRates = { 2.0f, 5.f, 8.f};	// degrees; 1st element is for the futhest moon, and 3rd element is for the closest moon
	const float moonDistanceFromOrbitingPlanet = 6.f;
	vector<float> difference = { 1.5f, 3.f, 4.5f };

	moons.push_back(Moon("The Moon", moonRadius, moonDistanceFromOrbitingPlanet, axisTiltMoon, axisRotationMoon, moonOrbitRates[0], planets[2], MOON_TEXTURE_PATH));
	Moon* moon = &moons[0];
	planets[2].addMoon(&moons[0]);
	
	// Mars moons
	int indexCounter = 1;
	for (int j = 0; j < MARS_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(MARS_MOON_NAMES[j], moonRadius - (0.1 * j), moonDistanceFromOrbitingPlanet - difference[j], axisTiltMoon, axisRotationMoon, moonOrbitRates[j], planets[3], MOON_TEXTURE_PATH));
	}
	// Set the Moons for mars
	planets[3].addMoon(&moons[moons.size() - 2]);
	planets[3].addMoon(&moons[moons.size() - 1]);

	// Jupiter moons
	for (int j = 0; j < JUPITER_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(JUPITER_MOON_NAMES[j], moonRadius - (0.1 * j), moonDistanceFromOrbitingPlanet - difference[j], axisTiltMoon, axisRotationMoon, moonOrbitRates[j], planets[4], MOON_TEXTURE_PATH));
	}
	// Set the Moons for Jupiter
	planets[4].addMoon(&moons[moons.size() - 3]);
	planets[4].addMoon(&moons[moons.size() - 2]);
	planets[4].addMoon(&moons[moons.size() - 1]);

	// Saturn moons
	for (int j = 0; j < SATURN_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(SATURN_MOON_NAMES[j], moonRadius - (0.1 * j), moonDistanceFromOrbitingPlanet - difference[j], axisTiltMoon, axisRotationMoon, moonOrbitRates[j], planets[5], MOON_TEXTURE_PATH));
	}
	// Set the Moons for Saturn
	planets[5].addMoon(&moons[moons.size() - 3]);
	planets[5].addMoon(&moons[moons.size() - 2]);
	planets[5].addMoon(&moons[moons.size() - 1]);

	// Uranus moons
	for (int j = 0; j < URANUS_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(URANUS_MOON_NAMES[j], moonRadius - (0.1 * j), moonDistanceFromOrbitingPlanet - difference[j], axisTiltMoon, axisRotationMoon, moonOrbitRates[j], planets[6], MOON_TEXTURE_PATH));
	}
	// Set the Moons for Uranus
	planets[6].addMoon(&moons[moons.size() - 3]);
	planets[6].addMoon(&moons[moons.size() - 2]);
	planets[6].addMoon(&moons[moons.size() - 1]);

	// Neptune moons
	for (int j = 0; j < NEPTUNE_MOON_NAMES.size(); j++, indexCounter++)
	{
		moons.push_back(Moon(NEPTUNE_MOON_NAMES[j], moonRadius - (0.1 * j), moonDistanceFromOrbitingPlanet - difference[j], axisTiltMoon, axisRotationMoon, moonOrbitRates[j], planets[7], MOON_TEXTURE_PATH));
	}
	// Set the Moons for Neptune
	planets[7].addMoon(&moons[moons.size() - 3]);
	planets[7].addMoon(&moons[moons.size() - 2]);
	planets[7].addMoon(&moons[moons.size() - 1]);

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

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

		// Render the Planets and update axial and orbital rotations
		for (Planet& p : windowData.planets)
		{
			renderCelestialBody(callBack, shaderForPlanetsMoonsSkyBox, p);

			if (p.hasRings())
			{
				p.ring.rotateViaCelestialBodyAxis(deltaTime);
				p.ring.orbitCelestialBody(deltaTime);
			}

			if (p.hasClouds())
			{
				p.clouds.rotateViaCelestialBodyAxis(deltaTime);
				p.clouds.orbitCelestialBody(deltaTime);
			}

			if (!pause)
			{
				p.rotateViaCelestialBodyAxis(deltaTime);
				p.orbitCelestialBody(deltaTime);
			}
		}

		// Render the Moons and update axial and orbital rotations
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
