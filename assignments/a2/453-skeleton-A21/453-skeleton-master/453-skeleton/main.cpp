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

using namespace std;
using namespace glm;

const static double piApprox = atan(1) * 4;	// pi approximation
const static float MOVEMENT_VALUE = 0.03f;
const static float DIAMOND_LENGTH_SCALE = 0.18f;
const static float DIAMOND_WIDTH_SCALE = 0.18f;

// Used to scale the ship
static float SHIP_WIDTH_SCALE = 0.18f;
static float SHIP_LENGTH_SCALE = 0.12f;

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

	glm::vec3 position = glm::vec3(0.f, 0.f, 1.f);		// initial position
	glm::vec3 facing = glm::vec3(0.f, 1.f, 0.f);		// initial position object is facing (used by ship atm)
	float theta;										// Object's total rotation (in degrees)
	// Alternatively, you could represent rotation via a normalized heading vec:
	// glm::vec3 heading;
	float scale;										// Or, alternatively, a glm::vec2 scale;
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
	glm::vec3 previousMouseLoc = glm::vec3(0.f, 0.f, -1.f);	// initially previous mouse location is not existant (debug)
	glm::vec3 currMouseLoc = vec3();						// current mouse location
	float score = 0;
};

// Utility function prototypes
void drawGameObject(ShaderProgram& shader, GameObject& obj);
float convertFromPixelSpace(float pos);
void setGpuGeom(GameObject& obj);
double calcAngle(vec3 shipPos, glm::vec3 initialV3, glm::vec3 finalV3);
float convertToDegree(float radians);
float convertToRad(float degree);
float calcVec3Length(vec3 vec);

// Transformation function prototypes
void rotateAboutObjCenter(GameObject& obj, float degreeOfRotation);
void scaleObj(GameObject& obj, float scale);
void scaleObj(GameObject& obj, float scaleX, float scaleY);
void translateObj(GameObject& obj, double deltaX, double deltaY);

//void moveForward(Window& win, GameObject& obj, float moveBy, vec3& mouseLoc);
//void moveBackward(Window& win, GameObject& obj, float moveBy);

void rotateCCWAboutVec3(glm::vec3& vec3ToRotate, const glm::vec3 rotateAboutVec, const float angleOfRotation); // dunno if i need yet


// Debug function prototypes
void printVec4Pos(glm::vec4 vec, int vecNum);
void printVec4Pos(glm::vec4 vec);


// Example of transformations from tuts/used in tut #10
vec3 mousePos = vec3(0.f, 0.f, 0.f);
mat4 translate0 = translate(mat4(1.0), vec3(0.5f, 0.f, 0.f));	// move in x
mat4 rotation0 = rotate(mat4(1.f), radians(90.f), vec3(0.f, 0.f, 1.f));	// rotate 90-degrees in z axis
mat4 scale0 = scale(mat4(1.f), vec3(0.5f, 0.5f, 1.f));
mat4 combined0 = translate0 * rotation0;	// rotate first then translate

class MyCallbacks : public CallbackInterface
{

public:
	MyCallbacks(ShaderProgram& shader, GameData& data, Window& window) :
		shader(shader),
		gameData(data),
		window(window)
	{
	}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		std::cout << "\n---key pressed---" << std::endl;

		GameObject& ship = gameData.ship;

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			shader.recompile();
		}
		//// TEST : only used to see if rotationAboutObjCenter() works, otherwise do not use.
		//else if (key == GLFW_KEY_Q )// && action == GLFW_PRESS)
		//{
		//	glm::mat4 identity = glm::mat4(1.0f);	// identity matrix for transformations (4x4)
		//	float angle = glm::radians(22.5f);		// angle of rotation (converts degree to radians)
		//	glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis, notice how there is a 1.0f at the 'z' pos
		//	glm::mat4 rotateMatrix = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix with the 12-degrees of freedom filled out.
		//	gameData.ship.transformationMatrix = rotateMatrix * gameData.ship.transformationMatrix;
		//	//drawGameObject(shader, gameData.ship);
		//}
		//else if (key == GLFW_KEY_E ) //&& action == GLFW_PRESS)
		//{
		//	// for now rotate SHIP as a test. ( this is the opposite code of the above test of rotating 22.5 CCW), this rotates Clock wise 22.5 degrees
		//	rotateAboutObjCenter(gameData.ship, -2*ship.theta);
		//	//drawGameObject(shader, gameData.ship);
		//}
		//-- end of test
		else if (key == GLFW_KEY_W)// && action == GLFW_PRESS)
		{
			//translateObj(ship, 0.f, 0.1f);
			//drawGameObject(shader, gameData.ship);
			rotateShipToCursor(); //?
			moveForward(ship, MOVEMENT_VALUE, gameData.currMouseLoc);
			rotateShipToCursor(); //?
		}
		else if (key == GLFW_KEY_S)// && action == GLFW_PRESS)
		{
			//translateObj(ship, 0.f, -0.1f);
			//drawGameObject(shader, gameData.ship);
			rotateShipToCursor(); //?
			moveBackward(ship, MOVEMENT_VALUE);
			rotateShipToCursor(); //?
		}

		if (action == GLFW_PRESS)
		{
			counter++;
			std::cout << "counter = " << counter << std::endl;
		}

		std::cout << "\n(clip space)\nShip facing: (" << ship.facing.x << ", " << ship.facing.y << ")";
		std::cout << "\nMouse pos curr: (" << gameData.currMouseLoc.x << ", " << gameData.currMouseLoc.y << ")" << endl;

	}

	virtual void cursorPosCallback(double xpos, double ypos)
	{
		// debug
		//std::cout << "\n(Pixel space)\nMouse pos curr: (" << xpos << ", " << ypos << ")" << std::endl;
		//std::cout << "\n(clip space)\nMouse pos prev: (" << gameData.previousMouseLoc.x << ", " << gameData.previousMouseLoc.y << ")";
		std::cout << "\n(clip space)\nShip facing: (" << ship.facing.x << ", " << ship.facing.y << ")";
		std::cout << "\nMouse pos curr: (" << convertFromPixelSpace(xpos) << ", " << -convertFromPixelSpace(ypos) << ")" << endl;

		// convert pixel space values to clip space values.
		gameData.currMouseLoc = vec3(convertFromPixelSpace(xpos), -convertFromPixelSpace(ypos), 0.f);

		GameObject& ship = gameData.ship;
		rotateShipToCursor();

		//// Find the angle from where the ship is facing and current mouse location (radians)
		//double angle = calcAngle(ship.position, ship.facing, gameData.currMouseLoc);

		//// debug
		//cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;

		//angle = convertToDegree(angle);

		//ship.theta += angle;								// update the ship's total rotation (about its center)
		//ship.facing = gameData.currMouseLoc;				// update where the ship is facing
		//rotateAboutObjCenter(gameData.ship, angle);	// do rotation of the ship

		gameData.previousMouseLoc = gameData.currMouseLoc;	// set the previous mouse location (debug)

		// debug
		cout << "\nTotal ship angle (radians) = " << convertToRad(ship.theta) << " (degree) = " << ship.theta << endl;


		// NEW-- this tut example puts the ship to the cursor pos.
		// convert to clip space
		//std::cout << "\n(clip space)\nMouse pos curr: (" << convertFromPixelSpace(xpos) << ", " << -convertFromPixelSpace(ypos) << ")" << endl;

		//mousePos.x = convertFromPixelSpace(xpos);
		//mousePos.y = -convertFromPixelSpace(ypos);

		//GameObject& ship = gameData.ship;
		//ship.transformationMatrix = translate(mat4(1.f), mousePos - ship.position) * ship.transformationMatrix;
		//ship.position = mousePos;
		checkCollectDiamond(); // here for testing only
	}

private:
	ShaderProgram& shader;
	GameData& gameData;
	Window& window;
	GameObject& ship = gameData.ship;
	GameObject& d0 = gameData.d0;
	GameObject& d1 = gameData.d1;
	GameObject& d2 = gameData.d2;
	GameObject& d3 = gameData.d3;
	int counter = 1;

	/// <summary>
	/// Rotates the ship by THETA (The angle between the facing vector and the cursor's current location vector).
	/// Ship's facing vector is set to the cursor's current location vector, and the ship's total angle is updated.
	/// </summary>
	void rotateShipToCursor()
	{
		// Find the angle from where the ship is facing and current mouse location (radians)
		double angle = calcAngle(ship.position, ship.facing, gameData.currMouseLoc);

		// debug
		cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;

		angle = convertToDegree(angle);

		ship.theta += angle;								// update the ship's total rotation (about its center)
		ship.facing = gameData.currMouseLoc;				// update where the ship is facing
		rotateAboutObjCenter(gameData.ship, angle);	// do rotation of the ship

		// debug
		cout << "\nship facing (updated): (" << ship.facing.x << ", " << ship.facing.y << ")" << endl;
	}

	void checkCollectDiamond()
	{
		// Check first diamond.
		checkCollectDiamondHelper(d0);
	}

	void checkCollectDiamondHelper(GameObject& diamond)
	{
		vec3 shipPos = ship.position;
		float halfWidth = SHIP_WIDTH_SCALE / 2.f;
		float halfLength = SHIP_LENGTH_SCALE / 2.f;

		// The Box that 'surrounds' the ship, if a diamond is within this box, it is collected.
		std::vector<vec3> shipCollectionBox =
		{
			shipPos + vec3(-halfWidth, -halfLength, 0.f),	// bottom left
			shipPos + vec3(halfWidth, -halfLength, 0.f),	// bottom right
			shipPos + vec3(halfWidth, halfLength, 0.f),		// top right
			shipPos + vec3(-halfWidth, halfLength, 0.f)		// top left
		};

		// rotate the Box such that it 'surrounds' the ship
		for (int i = 0; i < shipCollectionBox.size(); i++)
			rotateCCWAboutVec3(shipCollectionBox[i], shipPos, ship.theta); // idk what i should rotate by... abs(ship.theta)?

		vec3 diamondPos = diamond.position;
		float dHalfWidth = DIAMOND_WIDTH_SCALE / 2.f;
		float dHalfLen = DIAMOND_LENGTH_SCALE / 2.f;

		// Box that 'surrounds' the Diamond // note: the Box that 'surrounds' the diamond does not need to be rotated like the ship's Box
		std::vector<vec3> diamondBox =
		{
			diamondPos + vec3(-dHalfWidth, -dHalfLen, 0.f),		// bottom left
			diamondPos + vec3(dHalfWidth, -dHalfLen, 0.f),		// bottom right
			diamondPos + vec3(dHalfWidth, dHalfLen, 0.f),		// top right
			diamondPos + vec3(-dHalfWidth, dHalfLen, 0.f)		// top left
		};

		// Checking if the Diamond's Box points are inside the Ship's Collection Box
		// if there is such a point inside the Ship's Collection Box, the diamond is collected.





		// Draw the box DEBUG -- prolly need to bring the old CPU, GPU, and shader classes from assignment 1 to do this
		//CPU_Geometry testCPU;
		//GPU_Geometry testGPU;

		//for (int i = 0; i < collectionBox.size(); i++)
		//{
		//	testCPU.verts.push_back(collectionBox[i]);
		//	testCPU.texCoords.push_back(collectionBox[i]);
		//}

		//testGPU.setVerts(testCPU.verts);
		//testGPU.setTexCoords(testCPU.texCoords);

		//shader.use();
		//testGPU.bind();
		////glEnable(GL_FRAMEBUFFER_SRGB);
		////glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDrawArrays(GL_TRIANGLE_FAN, 0, testCPU.verts.size());
	}

	//void checkWithinShipCollectionBox(GameObject& diamond)
	//{

	//}

	/// <summary>
	/// Rotates a vec3 CCW by 'degree's, (Use negative degree for clockwise).
	/// </summary>
	/// <param name="vecToRotate"></param>
	/// <param name="degree"></param>
	void rotateVec3(vec3& vecToRotate, float degree)
	{
		float x = vecToRotate.x;
		float y = vecToRotate.y;
		float rad = convertToRad(degree);

		float xfinal = x * cos(rad) - y * sin(rad);
		if (abs(xfinal) < 1.0e-6)	// if the x-value is a very small number,
			xfinal = 0.f;			// make xfinal 0
		float yfinal = x * sin(rad) + y * cos(rad);
		if (abs(yfinal) < 1.0e-6)	// if the y-value is a very small number,
			yfinal = 0.f;			// make yfinal 0

		// Set the rotated values
		vecToRotate.x = xfinal;
		vecToRotate.y = yfinal;
	}

	/// <summary>
	/// Checks if the GameObject is within the game window after moving.
	/// Return True if the GameOvject is within the game window after move, false otherwise.
	/// </summary>
	/// <param name="win"></param>
	/// <param name="obj"></param>
	/// <param name="move"></param>
	/// <returns></returns>
	bool isObjectWithinWindowAfterMove(GameObject& obj, vec3 move)
	{
		float windowX = convertFromPixelSpace(window.getWidth());
		float windowY = convertFromPixelSpace(window.getHeight());

		float objFuturePosX = obj.position.x + move.x;
		float objFuturePosY = obj.position.y + move.y;

		// Object will go out the screen if moved by 'move' -> return false
		if (objFuturePosY > windowY || objFuturePosY < -windowY)
			return false;
		else if (objFuturePosX > windowX || objFuturePosX < -windowX)
			return false;

		return true;	// otherwise return true
	}

	void moveForward(GameObject& obj, float moveBy, vec3& mouseLoc)
	{
		vec3 moveByVec = vec3(moveBy, 0.f, 0.f);	// set the moveBy value along the x-axis
		rotateVec3(moveByVec, obj.theta);			// rotate moveByVec by the objects 'theta'

		// Check if the ship moving forward will cross the mouse location
		vec3 futurePos = obj.position + moveByVec;
		float vecLen = calcVec3Length(futurePos - mouseLoc);
		bool withinEpsilon = vecLen <= MOVEMENT_VALUE;

		if (withinEpsilon)	// If within epsilon, don't move the ship
		{
			cout << "\nwithin epsilon, did not move up" << endl;	// debug
			return;
		}

		bool withinWindow = isObjectWithinWindowAfterMove(obj, moveByVec);

		// If the object moves by 'moveByVec' and it goes outside the window, don't move
		if (!withinWindow)
			return;

		obj.position += moveByVec;					// update position
		//obj.facing += moveByVec;	// ?

		//obj.transformationMatrix[3] += vec4(moveByVec, 0.f);	// update transformation
		obj.transformationMatrix = translate(mat4(1.f), moveByVec) * obj.transformationMatrix;
	}

	void moveBackward(GameObject& obj, float moveBy)
	{
		vec3 moveByVec = vec3(-moveBy, 0.f, 0.f);	// set the moveBy value along the x-axis
		rotateVec3(moveByVec, obj.theta);			// rotate moveByVec by the objects 'theta'

		bool withinWindow = isObjectWithinWindowAfterMove(obj, moveByVec);

		// If the object moves by 'moveByVec' and it goes outside the window, don't move
		if (!withinWindow)
			return;

		obj.position += moveByVec;					// update position
		//obj.facing -= moveByVec;	// ?
		//obj.transformationMatrix[3] -= vec4(moveByVec, 0.f);	// update transformation
		obj.transformationMatrix = translate(mat4(1.f), moveByVec) * obj.transformationMatrix;
	}
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

int main() {
	Log::debug("Starting main");

	// debug
	double angle = calcAngle(vec3(0.f), glm::vec3(9.f, 2.f, 1.f), glm::vec3(2.f, 6.f, 1.f)); // expected value is 59.0362 degrees (returns in rads)
	cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f/piApprox)  << endl;
	angle = calcAngle(vec3(0.f), glm::vec3(convertFromPixelSpace(800.f), 0.f, 1.f), glm::vec3(0.f, convertFromPixelSpace(800.f), 1.f));	// 90 degrees
	cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;
	angle = calcAngle(vec3(0.f), glm::vec3(0.f, convertFromPixelSpace(800.f), 1.f), glm::vec3(convertFromPixelSpace(800.f), 0.f, 1.f));	// -90 degrees
	cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;
	angle = calcAngle(vec3(0.f), vec3(0.7775f, -0.04f, 0.f), vec3(0.78, -0.04, 0.f));								// 0 degrees
	cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;
	angle = calcAngle(vec3(0.f), vec3(-0.0025f, -0.f, 0.f), vec3(0.f, -0.f, 0.f));									// 0 degrees
	cout << "\nAngle calculated (radians) = " << angle << " (degree) = " << angle * (180.f / piApprox) << endl;

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
	window.setCallbacks(std::make_shared<MyCallbacks>(shader, newData, window)); // can also update callbacks to new ones

	// Create ship cpuGeom
	//ship.cgeom = shipGeom(0.18f, 0.12f);
	ship.cgeom = shipGeom(SHIP_WIDTH_SCALE, SHIP_LENGTH_SCALE);
	ship.theta = 90.f;	// facing 'up', top of the screen
	// Create Diamonds cpuGeom
	d0.cgeom = diamondGeom(0.14f, 0.14f);
	d1.cgeom = diamondGeom(0.14f, 0.14f);
	d2.cgeom = diamondGeom(0.14f, 0.14f);
	d3.cgeom = diamondGeom(0.14f, 0.14f);
	// Put the GameObjects in their starting locations
	// if i have translate above the scale, the scale will be applied to the transformation values
	scaleObj(ship, 0.09f, 0.06f);
	//translateObj(ship, 0.5f, 0.5f);	// Center of the screen
	translateObj(ship, 0.f, 0.f);	// Center of the screen
	// debugging rotatation
	//rotateAboutObjCenter(ship, 90.f);
	scaleObj(d0, 0.07f);
	translateObj(d0, -0.5f, -0.5f);	// bottom left
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

		// from  tutorial
		//GLint shaderLoc = glGetUniformLocation(shader.getID(), "transformationMatrix"); // need to define 'getID()' in shaderprogram file
		//glUniformMatrix4fv(shaderLoc, 1, GL_FALSE, &rotation0[0][0]);
		//--

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
		ImGui::Text("Score: %d", newData.score); // Second parameter gets passed into "%d"

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

void drawGameObject(ShaderProgram& shader, GameObject& obj)
{
	//setGpuGeom(obj); // not needed, prolly
	shader.use();

	shader.setMat4Transform("transformationMatrix", obj.transformationMatrix);
	shader.setMat4TextureTransform("transformationMatrixTexture", obj.transformationTexMatrix);

	obj.ggeom.bind();
	obj.texture.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	obj.texture.unbind();
}

/// <summary>
/// Converts values from pixel space to clip space
/// </summary>
/// <param name="pos"></param>
/// <returns></returns>
float convertFromPixelSpace(float pos)
{
	float clipPos = pos / (800 / 2.f);
	clipPos = clipPos - 1;
	return clipPos;
}

void setGpuGeom(GameObject& obj)
{
	obj.ggeom.setVerts(obj.cgeom.verts);
	obj.ggeom.setTexCoords(obj.cgeom.texCoords);
}

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
	float rads = angleOfRotation * (piApprox / 180.f);

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
/// Returns the angle in radians. Ensure initalV3 and finalV3's values are in clip space.
/// </summary>
/// <param name="shipPos"></param>
/// <param name="initialV3"></param>
/// <param name="finalV3"></param>
/// <returns></returns>
double calcAngle(vec3 shipPos, glm::vec3 initialV3, glm::vec3 finalV3)
{
	// translate the ship pos to origin and translate initial and final vec3 by -ship pos.
	vec3 translateBack = shipPos;
	vec3 translateToOrigin = -shipPos;
	initialV3 = initialV3 + translateToOrigin;
	finalV3 = finalV3 + translateToOrigin;

	//float dotProuct = (initialV3.x * finalV3.x) + (initialV3.y * finalV3.y) + (initialV3.z * finalV3.z);
	double dotProuct = (initialV3.x * finalV3.x) + (initialV3.y * finalV3.y);

	float initialV3Length = calcVec3Length(initialV3);
	float finalV3Length = calcVec3Length(finalV3);
	//float squareInitialX = initialV3.x * initialV3.x;
	//float squareInitialY = initialV3.y * initialV3.y;
	////float squareInitialZ = initialV3.z * initialV3.z;

	//float squareFinalX = finalV3.x * finalV3.x;
	//float squareFinalY = finalV3.y * finalV3.y;
	////float squareFinalZ = finalV3.z * finalV3.z;

	////float initialV3Length = sqrt(squareInitialX + squareInitialY + squareInitialZ);
	//float initialV3Length = sqrt(squareInitialX + squareInitialY);
	////float finalV3Length = sqrt(squareFinalX + squareFinalY + squareFinalZ);
	//float finalV3Length = sqrt(squareFinalX + squareFinalY);

	// if the length of one of the vectors is 0, return angle = 0. -> error when we divide by 0.
	if (initialV3Length * finalV3Length == 0)
		return 0.0;

	double calc = dotProuct / (initialV3Length * finalV3Length);

	// 'calc' is out of arccos range of [-1, 1], so return with angle = 0. acos(calc) = -nan(ind) if 'calc' is not within [-1, 1].
	if (calc > 1 || calc < -1)
		return 0.0;

	double angle = acos(calc);
	
	// Translate back relative to ship position
	finalV3 = finalV3 + translateBack;
	initialV3 = initialV3 + translateBack;

	// Determining if the angle is negative/positive
	vec3 crossProduct = cross(initialV3, finalV3);
	if (crossProduct.z < 0)	// if the cross product of initalV3 and finalV3 z-value is < 0, initalV3 to finalV3 is clockwise -> negative angle.
		angle = -angle;

	// debug
	//cout << "\nAngle calculted (radians) = " << angle << " (degree) = " << angle * (180.f/piApprox)  << endl;
	return angle;
}

/// <summary>
/// Works fine atm
/// -> oct. 16 update, it may not be working right, just play around with moving while also moving the mouse you'll see the bug
/// </summary>
/// <param name="obj"></param>
/// <param name="degreeOfRotation"> in degrees, a float.</param>
void rotateAboutObjCenter(GameObject& obj, float degreeOfRotation)
{
	mat4 translateToOrigin = glm::translate(mat4(1.0f), -obj.position);	// Used to translate all the object's vec3 verts by -Position (Position should be at origin)
	mat4 translateBack = glm::translate(glm::mat4(1.0f), obj.position); // Used to translate all the object's vec3 verts by Position (back to original positions)

	glm::mat4 identity = glm::mat4(1.0f);					// identity matrix for transformations (4x4)
	float angle = glm::radians(degreeOfRotation);			// angle of rotation (converts degree to radians)
	glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);	// axis of rotation is z-axis.

	glm::mat4 rotateMatrix = glm::rotate(identity, angle, axisOfRotation);	// transformation matrix.

	// 1. translate by -Position (Position should be (0,0,0) after)
	// 2. do rotation
	// 3. translate by Position (back to original position)
	mat4 model = translateBack * rotateMatrix * translateToOrigin * obj.transformationMatrix;	// resulting matrix of all those transformations

	// Update transformation matrix
	obj.transformationMatrix = model;
	// obj.transformationMatrix = model * obj.transformationMatrix;	// multiply the newly calculated model with the old model to get the actual model (want to retain the previous transformations)
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
	// currently works when moving striahgt up/down
	// need to consider when ship is angled... need to rotate that translation vector and then apply the translation.
	// prolly need a way to save the angle of the ship

	vec3 translationVec = vec3(deltaX, deltaY, 0.f);

	// need to rotate the translationVec about the obj.theta (find code in a1, or recode it)

	obj.position = obj.position + translationVec;	// Update the object's position
	obj.facing = obj.facing + translationVec; 		// Update where the object is facing

	// Create translation matrix
	//mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(deltaX, deltaY, 0.f));
	mat4 translationMatrix = glm::translate(mat4(1.f), translationVec);
	//translationMatrix[3] = vec4(obj.position, 1.f);	// edit the translation


	// Update the transformation matrix
	//obj.transformationMatrix = translationMatrix + obj.transformationMatrix; // update the transformation matrix
	obj.transformationMatrix = translationMatrix * obj.transformationMatrix; // update the transformation matrix


	// DELETE
	//obj.transformationTexMatrix = translationMatrix * obj.transformationTexMatrix; // Apply translation to texture transformation
}

//void uniformScaleObj(GameObject& obj, float scaleVal)
//{
//	// Create scale matrix
//	mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleVal, scaleVal, 0.f));
//
//	// Update the transformation matrix
//	obj.transformationMatrix = scaleMatrix * obj.transformationMatrix; // update the transformation matrix
//}

/// <summary>
/// Uniform scaling.
/// </summary>
/// <param name="obj"> the GameObject to be scaled.</param>
/// <param name="scale"> the scale factor, a float.</param>
void scaleObj(GameObject& obj, float scale)
{
	scaleObj(obj, scale, scale);
}

/// <summary>
/// Scaling.
/// </summary>
/// <param name="obj"> the GameObject to be scaled.</param>
/// <param name="scaleX"> x-scale factor, a float.</param>
/// <param name="scaleY"> y-scale factor, a float.</param>
void scaleObj(GameObject& obj, float scaleX, float scaleY)
{
	// Create scale matrix
	mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.f));

	// Update the transformation matrix
	obj.transformationMatrix = scaleMatrix * obj.transformationMatrix; // update the transformation matrix
}

float calcVec3Length(vec3 vec)
{
	float squareX = vec.x * vec.x;
	float squareY = vec.y * vec.y;
	return sqrt(squareX + squareY);
}

float convertToDegree(float radians)
{
	return radians * (180.f / piApprox);	// convert to degrees
}

float convertToRad(float degree)
{
	return degree * (piApprox / 180.f);		// convert to radians
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
