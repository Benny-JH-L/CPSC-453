#include <glad/glad.h>

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
#include "Panel.h"

// my headers
//#include "curve/CurveGenerator.h"
#include <tuple>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace glm;

const static float CONTROL_POINT_SIZE = 15.f;										// Size of the control point's box
const static float CONTROL_POINT_LENGTH = 2 * (CONTROL_POINT_SIZE / 1000.f);		// Length of the control point's box
const static float CONTROL_POINT_WIDTH= CONTROL_POINT_LENGTH;						// Width of the control point's box
const static int DEFAULT_NUM_CURVE_ITERATIONS = 6;									// the default number of iterations for curve generation
const static int MAX_NUMBER_OF_ITERATIONS_FOR_CURVES = 20;
const static int MIN_NUMBER_OF_ITERATIONS_FOR_CURVES = 1;

void testDeCasteljauAlgo();
string toString(vector<vec3> arr);

struct curveRelatedData
{
	vector<vec3>& controlPts;	// The control points the user entered
	vector<vec3>& curve;		// points that represent the generated curve
	int& numIterations;
	bool& showCurvePoints;		// to show curve points
	bool& deleteControlPts;		// to delete curve points
	bool& resetWindow;			// to reset window
	const mat4 defaultViewMat = mat4(1.0f);
};

/// <summary>
/// Default orbit viewer values
/// </summary>
struct defaultOrbitViewData
{
	const float fieldOfView = glm::radians(45.0f);
	const float aspectRatio = 1.f;
	const float nearPlane = 0.1f;
	const float farPlane = 100.f;
	const vec3 lookAtPoint = vec3(0.f);
	//const vec3 lookAtPoint = vec3(vec2(0.f), -1.0f);
	const vec3 cameraPos = vec3(vec2(0.f), 3.0f);
	const float mouseSensitivity = 100.0f;		// How fast the camera movement should be
	const float scrollSensitivity = 10.0f;		// How fast the scroll in/out should be
	const float distanceFromLookAtPoint = 3.f;	// Distance from the 'lookAtPoint' (origin) (world space)
	const float theta = glm::radians(90.f);		// Ensure radians
	const float phi = glm::radians(90.f);		// Ensure radians

	const vec3 upVector = vec3(0.f, 1.0f, 0.f);

	mat4 viewMat4;
	mat4 perspectiveMat4;

	defaultOrbitViewData()
	{
		viewMat4 = glm::lookAt(cameraPos, lookAtPoint, upVector);
		perspectiveMat4 = glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
	}
};

struct orbitViewerData
{
	defaultOrbitViewData defaultData;
	bool& resetWindow;

	float fieldOfView = glm::radians(45.0f);
	float aspectRatio = 1.f;
	float nearPlane = 0.1f;
	float farPlane = 100.f;
	const vec3 lookAtPoint = vec3(0.f);		// looking at the origin (0.0, 0.0, 0.0)
	vec3 cameraPos = vec3(vec2(0.f), 3.0f);	// initial at 3.0f

	float mouseSensitivity = 100.0f;		// How fast the camera movement should be
	float scrollSensitivity = 10.0f;		// How fast the scroll in/out should be
	float distanceFromLookAtPoint = 3.f;	// Distance from the 'lookAtPoint' (origin) (world space)
	float theta = glm::radians(90.f);		// Ensure radians
	float phi = glm::radians(90.f);			// Ensure radians

	orbitViewerData(bool& resetWindow) :
		resetWindow(resetWindow),
		defaultData(defaultOrbitViewData())
	{
	}

	/*
	///// <summary>
	///// Sets the 'theta' in degrees.
	///// </summary>
	///// <param name="degrees"></param>
	//void setTheta(float degrees)
	//{
	//	theta = degrees;
	//}

	///// <summary>
	///// Returns the previously set 'theta' (degrees) as glm::radians
	///// </summary>
	///// <returns>glm::radians</returns>
	//float getTheta()
	//{
	//	return glm::radians(theta);
	//}

	///// <summary>
	///// Sets the 'phi' in degrees.
	///// </summary>
	///// <param name="degrees"></param>
	//void setPhi(float degrees)
	//{
	//	phi = degrees;
	//}

	///// <summary>
	///// Returns the previously set 'phi' (degrees) as glm::radians
	///// </summary>
	///// <returns>glm::radians</returns>
	//float getPhi()
	//{
	//	return glm::radians(phi);
	//}
	*/

};

struct windowControlData
{
	Window& window;
	curveRelatedData& curveRelatedData;
	orbitViewerData& orbitViewerData;
	//vector<vec3>& controlPts;	// The control points the user entered
	//vector<vec3>& curve;		// points that represent the generated curve
	//int& numIterations;
	//bool& showCurvePoints;		// to show curve points
	//bool& deleteControlPts;		// to delete curve points
	//bool& resetWindow;			// to reset window
};

/// <summary>
/// Options include: bezier and B-spline curves, perspective viewing (2D, 3D,),
/// surface of revolution, and tensor product surfaces
/// </summary>
struct optionData
{
	int& comboSelection;   // Index of selected option in combo box
	const char* options[]; // Options for the combo box
};

struct cpuGeometriesData
{
	CPU_Geometry& cp_point_cpu;
	CPU_Geometry& cp_line_cpu;

	CPU_Geometry& curve_point_cpu;	// Contains the control points for the generated curve (to display points if needed)
	CPU_Geometry& curve_line_cpu;	// Contains the control points for the generated curve (lines of the curve)
};

class CurveGenerator
{
public:

	/// <summary>
	/// Ensure that the size of 'controlPts' is 1 more than 'degree'.
	/// </summary>
	/// <param name="controlPts"></param>
	/// <param name="degree"></param>
	/// <param name="u"></param>
	/// <returns>A "std::vector(glm::vec3)" of size 1 that contains the point: Q(degree) (u)</returns>
	static vector<vec3> bezier(const vector<vec3> controlPts, int degree, float u)
	{
		// Starts generation with the original control points and after each 'genBezierCurve' call
		// this 'vector' gets smaller as the 'columns get smaller (contains the points at a column).
		vector<vec3> curveSoFar = vector<vec3>(controlPts.size() - 1);

		/* delete-----
		//if (controlPts.size() - 1 != degree)	// checking if the number of control points is 1 more than the degree
		//{
		//	std::cout << "\nError occured in deCasteljauAlgo: (invalid number of control points)" << std::endl;
		//	std::cout << "num controlPoints: " << controlPts.size() << " degree: " << degree << std::endl;

		//	return curveSoFar;
		//}
		//curveSoFar = vector<vec3>(controlPts.size() - 1);
		-----*/

		for (int i = 1; i < controlPts.size(); i++)			// calculate entire "column"
		{
			vector<vec3> controlPtsToUse(2);
			controlPtsToUse[0] = controlPts[i - 1];
			controlPtsToUse[1] = controlPts[i];

			curveSoFar[i - 1] = deCasteljauAlgo(controlPtsToUse, degree, u);
		}

		if (controlPts.size() > 2)	// If the size of 'controlPts' is less than 2 then we have reached the final column (calculated "Q<degree>(u)")
			curveSoFar = bezier(curveSoFar, degree, u);	// Call itself again to generate next column

		return curveSoFar;
	}

	static vector<vec3> chaikin(const vector<vec3> coursePts)
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

private:

	static vec3 deCasteljauAlgo(const vector<vec3> controlPts, int degree, float u)
	{
		vec3 pt = vec3();			// the point at a column


		for (int i = 1; i < degree; i++)
		{
			for (int j = 0; j < degree - i; i++)
			{
				try
				{
					pt = vec3((1 - u) * controlPts[j] + u * controlPts[j + 1]);
				}
				catch (const std::exception&)
				{
					std::cout << "Error occured in deCasteljauAlgo" << std::endl;
				}
			}
		}
		return pt;
	}
};

class CurveEditorCallBack : public CallbackInterface
{
public:
	CurveEditorCallBack(windowControlData& wData, optionData& oData) :
		windowData(wData),
		curveData(wData.curveRelatedData),
		win(wData.window),
		optionData(oData),
		controlPoints(curveData.controlPts)
	{
		movingControlPoint = false;		// initialize to false
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) override
	{
		Log::info("KEEEEEYYYYY KeyCallback: key={}, action={}", key, action);
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override
	{
		Log::info("[MOUSE-BUTTON-CALL-BACK]: button={}, action={}", button, action);
		mouseButton = button;
		mouseAction = action;

		// If the user chose a curve generation option
		if (optionData.comboSelection == 0 || optionData.comboSelection == 1)
		{
			// Deselect the moving control point when left-mouse button is released
			if (button == 0 && action == 0 && movingControlPoint)
			{
				cout << "no longer moving cp" << endl;	// debug
				movingControlPoint = false;
			}
			// place a control point
			else if (button == 0 && action == 0 && !curveData.deleteControlPts)
			{
				controlPoints.push_back(vec3(mousePos, 0.f));
			}
			// Delete control point
			else if (mouseButton == 0 && mouseAction == 1 && curveData.deleteControlPts)
			{
				tuple<bool, int> tup = doesControlPointExist(mousePos);	
				auto [cpPointExist, index] = tup;						// unpack the tuple, (bool : cpPointExist), (int : index)

				if (cpPointExist)	// delete point
				{
					cout << "FOUND POINT TO REMOVE" << endl;	// debug

					// create a new 'std::vector<vec3>' that does not contain the specified point
					vector<vec3> temp;
					for (int i = 0; i < controlPoints.size(); i++)
					{
						if (controlPoints[i] == controlPoints[index])	// found the control point to be removed, skip it
							continue;
						temp.push_back(controlPoints[i]);
					}
					controlPoints = temp;						// set new control points
				}

				/*
				// moved most of this code into a function
				//bool keepChecking = true;
				//for (int i = 0; i < controlPoints.size() && keepChecking; i++)
				//{
				//	float halfLen = CONTROL_POINT_LENGTH / 2.f;
				//	float halfWidth = CONTROL_POINT_WIDTH / 2.f;

				//	vector<vec2> cp_corners =
				//	{
				//		(controlPoints[i] + vec3(-halfWidth, -halfLen, 0.f)),	// bottom left corner
				//		(controlPoints[i] + vec3(halfWidth, -halfLen, 0.f)),	// bottom right corner
				//		(controlPoints[i] + vec3(halfWidth, halfLen, 0.f)),		// top right corner
				//		(controlPoints[i] + vec3(-halfWidth, halfLen, 0.f)),	// top left corner
				//	};

				//	bool withinX = cp_corners[0].x <= mousePos.x && mousePos.x <= cp_corners[2].x;	// mouse-x-pos has to be between the bottom left and top right corner-x-pos's
				//	bool withinY = cp_corners[0].y <= mousePos.y && mousePos.y <= cp_corners[2].y;	// mouse-y-pos has to be between the bottom left and top right corner-y-pos's

				//	if (withinX && withinY)
				//	{
				//		cout << "FOUND POINT TO REMOVE" << endl;	// debug

				//		// create a new 'vector' that does not contain this specified point
				//		vector<vec3> temp;
				//		for (int k = 0; k < controlPoints.size(); k++)
				//		{
				//			if (controlPoints[k] == controlPoints[i])	// found the control point to be removed, skip it
				//				continue;
				//			temp.push_back(controlPoints[k]);
				//		}

				//		keepChecking = false;	// stop checking
				//		windowData.controlPts = temp;
				//	}
				//}
				*/
			}
		}

		// note
		// when holding the left-mouse button the 'action' stays at 1 and 'button' is 0 -> release action = 0
		// when holding the right-mouse button the 'action' stays at 1 and 'button is 1 -> release action = 0
	}

	virtual void cursorPosCallback(double xpos, double ypos) override
	{
		Log::info("[CURSOR-POS-CALLBACK PIXEL]: xpos={}, ypos={}", xpos, ypos);
		vec2 pos = vec2(xpos, ypos);
		toClipSpace(pos);
		mousePos = pos;
		Log::info("[CURSOR-POS-CALLBACK CLIP]: xpos={}, ypos={}", pos.x, pos.y);

		// If the user chose a curve generation option
		if (optionData.comboSelection == 0 || optionData.comboSelection == 1)
		{
			// Move control point (when not in delete control point feature and a control point has been selected)
			if (mouseButton == 0 && mouseAction == 1 && !curveData.deleteControlPts && movingControlPoint)
			{
				cout << "moving cp" << endl;	// debug
				int index = get<int>(cpToMoveTupleData);					// gets the second element in the tuple (an int)
				moveControlPointToPos(controlPoints[index], mousePos);		// keep moving the specified point
			}
			// Find the control point to move
			else if (mouseButton == 0 && mouseAction == 1 && !curveData.deleteControlPts)
			{
				cpToMoveTupleData = doesControlPointExist(mousePos);
				auto [cpPointExist, index] = cpToMoveTupleData;		// unpack the tuple, (bool : cpPointExist), (int : index)

				if (cpPointExist)
				{
					//debug
					//cout << "moving point at loc: " << controlPoints[index] << endl;
					cout << "moving cp" << endl;	// debug

					movingControlPoint = true;
					moveControlPointToPos(controlPoints[index], mousePos);
				}
			}
		}
		// Orbit viewing
		else
		{
			// Move camera if right-click is being held
			if (mouseButton == 1 && mouseAction == 1)
			{
				//static double lastX = xpos, lastY = ypos;
				//double xOffset = xpos - lastX;
				//double yOffset = ypos - lastY;

				float lastX = prevMousePos.x;
				float lastY = prevMousePos.y;

				float xOffset = mousePos.x - lastX;
				float yOffset = mousePos.y - lastY;

				//lastX = xpos;
				//lastY = ypos;

				// Adjust the spherical coordinates based on mouse movement
				float sensitivity = orbitViewData.mouseSensitivity;
				theta += glm::radians(xOffset * sensitivity);
				phi += glm::radians(yOffset * sensitivity);

				// Constrain phi to avoid inversion
				if (phi < glm::radians(1.0f))
					phi = glm::radians(1.0f);
				else if (phi > glm::radians(179.5f))
					phi = glm::radians(179.5f);

				updateCamera();
			}
		}

		prevMousePos = mousePos;

		// note
		// when holding the left-mouse button the 'action' stays at 1 and 'button' is 0 -> release action = 0
		// when holding the right-mouse button the 'action' stays at 1 and 'button is 1 -> release action = 0
	}

	virtual void scrollCallback(double xoffset, double yoffset) override
	{
		Log::info("ScrollCallback: xoffset={}, yoffset={}", xoffset, yoffset);

		// If not in in the curve editor option, move the camera in/out
		if (optionData.comboSelection != 0 && optionData.comboSelection != 1)
		{
			//cout << "(before scroll) distance from origin = " << orbitViewData.distanceFromLookAtPoint << endl;	// debug

			orbitViewData.distanceFromLookAtPoint += (-yoffset / orbitViewData.scrollSensitivity);	// '-yoffset' so scroll up is zoom in and scroll down is zoom out

			// Constrain the distance so that it doesn't pass the 'lookAtPoint' (origin)
			if (orbitViewData.distanceFromLookAtPoint < 0.2)
				orbitViewData.distanceFromLookAtPoint = 0.2;

			cout << "(after scroll) distance from origin = " << orbitViewData.distanceFromLookAtPoint << endl;	// debug

			updateCamera();
		}

		// note:
		// scroll up -> yOffset = 1
		// scroll down -> yOffset = -1
		// xoffset = 0 for both
	}

	virtual void windowSizeCallback(int width, int height) override
	{
		Log::info("WindowSizeCallback: width={}, height={}", width, height);
		CallbackInterface::windowSizeCallback(width, height); // Important, calls glViewport(0, 0, width, height);
	}

private:
	Window& win = windowData.window;
	windowControlData& windowData;
	curveRelatedData& curveData = windowData.curveRelatedData;
	optionData& optionData;
	orbitViewerData& orbitViewData = windowData.orbitViewerData;
	vector<vec3>& controlPoints = curveData.controlPts;
	int mouseButton; // 0: left mouse button | 1: right mouse button
	int mouseAction; // 0: NOT held | 1: held (pressed down)
	vec2 mousePos;		// mouse position in CLIP space values
	vec2 prevMousePos;	// mouse position in CLIP space values
	bool movingControlPoint;
	tuple<bool, int> cpToMoveTupleData;	// <bool, int>, the boolean represents if the control point to move exists and the int is where this CP is located in the std::vector containing the CP's
	float& theta = orbitViewData.theta;
	float& phi = orbitViewData.phi;

	/// <summary>
	/// Converts pixel space position to clip space position.
	/// </summary>
	/// <param name="pos"></param>
	void toClipSpace(vec2& pos)
	{
		double clipPosX = pos.x / (win.getWidth() / 2.f);
		clipPosX -= 1;
		double clipPosY = pos.y / (win.getHeight() / 2.f);
		clipPosY -= 1;

		pos.x = clipPosX;
		pos.y = -clipPosY;
	}

	/// <summary>
	/// Updates where the Camera is situated
	/// </summary>
	void updateCamera()
	{
		// Update coordinates
		float distance = orbitViewData.distanceFromLookAtPoint;
		float x = distance * sin(phi) * cos(theta);
		float y = distance * cos(phi);
		float z = distance * sin(phi) * sin(theta);

		// Set new camera position
		orbitViewData.cameraPos = vec3(x, y, z);
	}

	void moveControlPointToPos(vec3& controlPointToMove, const vec2 mousePos)
	{
		//cout << "moving pt: " << controlPointToMove << endl;	// debug

		vec3 translate = vec3(mousePos, 0.f) - controlPointToMove;
		//cout << "translate by: " << translate << endl;	// debug

		controlPointToMove.x += translate.x;
		controlPointToMove.y += translate.y;

		//cout << "moved to: " << controlPointToMove << endl;	// debug
	}

	/// <summary>
	/// Goes through existing control points and checks if a control point at position 'pointToCheck' is among these points.
	/// If it exists, returns a tuple: (true, [index location in 'controlPoints']).
	/// Otherwise, returns a tuple: (false, -1)
	/// </summary>
	/// <param name="pointToCheck">the point to check, a glm::vec2</param>
	/// <returns> a tuple of size 2, (bool, int)</returns>
	tuple<bool, int> doesControlPointExist(vec2 pointToCheck)
	{
		for (int i = 0; i < controlPoints.size(); i++)
		{
			float halfLen = CONTROL_POINT_LENGTH / 2.f;
			float halfWidth = CONTROL_POINT_WIDTH / 2.f;

			vector<vec2> cp_corners =
			{
				(controlPoints[i] + vec3(-halfWidth, -halfLen, 0.f)),	// bottom left corner
				(controlPoints[i] + vec3(halfWidth, -halfLen, 0.f)),	// bottom right corner
				(controlPoints[i] + vec3(halfWidth, halfLen, 0.f)),		// top right corner
				(controlPoints[i] + vec3(-halfWidth, halfLen, 0.f)),	// top left corner
			};

			bool withinX = cp_corners[0].x <= pointToCheck.x && pointToCheck.x <= cp_corners[2].x;	// pointToCheck-x-pos has to be between the bottom left and top right corner-x-pos's
			bool withinY = cp_corners[0].y <= pointToCheck.y && pointToCheck.y <= cp_corners[2].y;	// pointToCheck-y-pos has to be between the bottom left and top right corner-y-pos's

			if (withinX && withinY)
			{

				//exist = true;
				//break;			// exit early

				// 'i' is the index at which the specified control point to be moved is located in the std::vector<glm::vec3>
				return make_tuple(true, i);	// return early
			}
		}
		// Otherwise, the 'pointToCheck' location does not contain a control point
		return make_tuple(false, -1);
	}

	// test , debug
	float calcVec2Length(vec2 v)
	{
		return sqrt((v.x * v.x) + (v.y * v.y));
	}
};

// delete
/// <summary>
/// Converts pixel space position to clip space position.
/// </summary>
/// <param name="pos"></param>
void toClipSpace(vec2& pos, Window& win)
{
	double clipPosX = pos.x / (win.getWidth() / 2.f);
	clipPosX -= 1;
	double clipPosY = pos.y / (win.getHeight() / 2.f);
	clipPosY -= 1;

	pos.x = clipPosX;
	pos.y = -clipPosY;
} // delete


// Can swap the callback instead of maintaining a state machine

/*
class TurnTable3DViewerCallBack : public CallbackInterface {

public:
	TurnTable3DViewerCallBack() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {}
	virtual void cursorPosCallback(double xpos, double ypos) {}
	virtual void scrollCallback(double xoffset, double yoffset) {}
	virtual void windowSizeCallback(int width, int height) {

		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width, height);
	}
private:

};
*/

class CurveEditorPanelRenderer : public PanelRendererInterface
{
public:
	CurveEditorPanelRenderer(windowControlData& d) :
		inputText(""),
		buttonClickCount(0),
		sliderValue(0.0f),
		dragValue(0.0f),
		inputValue(0.0f),
		checkboxValue(false),
		comboSelection(0),
		curveData(d.curveRelatedData),
		orbitViewData(d.orbitViewerData),
		windowData(d)
	{
		// Initialize options for the combo box
		options[0] = "Curve Editor - Bezier";
		options[1] = "Curve Editor - Quadratic B-spline (Chaikin)";
		options[2] = "Orbit Viewer - Curve";
		options[3] = "Orbit Viewer - Surface of Revolution";
		options[4] = "Orbit Viewer - Tensor Product 1";
		options[5] = "Orbit Viewer - Tensor Product 2";

		/*
		// Initialize color (white by default)
		colorValue[0] = 1.0f; // R
		colorValue[1] = 1.0f; // G
		colorValue[2] = 1.0f; // B
		*/
		// Initialize color (grey by default)
		colorValue[0] = 0.5f; // R
		colorValue[1] = 0.5f; // G
		colorValue[2] = 0.5f; // B
	}

	virtual void render() override
	{
		// Color selector
		ImGui::ColorEdit3("Select Background Color", colorValue); // RGB color selector
		ImGui::Text("Selected Color: R: %.3f, G: %.3f, B: %.3f", colorValue[0], colorValue[1], colorValue[2]);

		/*
		// Text input
		ImGui::InputText("Input Text", inputText, IM_ARRAYSIZE(inputText));

		// Display the input text
		ImGui::Text("You entered: %s", inputText);
		*/

		/*
		// Button
		if (ImGui::Button("Click Me")) {
			buttonClickCount++;
		}
		ImGui::Text("Button clicked %d times", buttonClickCount);
		*/

		/*
		// Float slider
		ImGui::SliderFloat("Float Slider", &sliderValue, 0.0f, 100.0f, "Slider Value: %.3f");

		// Float drag
		ImGui::DragFloat("Float Drag", &dragValue, 0.1f, 0.0f, 100.0f, "Drag Value: %.3f");

		// Float input
		ImGui::InputFloat("Float Input", &inputValue, 0.1f, 1.0f, "Input Value: %.3f");
		*/

		// Combo box
		ImGui::Combo("Select an Option", &comboSelection, options, IM_ARRAYSIZE(options));
		//ImGui::Text("Selected: %s", options[comboSelection]);

		// For Bezier and Chaikin (polynomial b-spline) curve options
		if (comboSelection == 0 || comboSelection == 1)
		{
			// Number of iterations input
			//ImGui::InputInt("Iterations", &numberOfIterations, 1, 1);
			ImGui::SliderInt("Iterations", &numberOfIterations, MIN_NUMBER_OF_ITERATIONS_FOR_CURVES, MAX_NUMBER_OF_ITERATIONS_FOR_CURVES, "Drag Value: %d");
			ImGui::Text("Min number of iterations: %d", MIN_NUMBER_OF_ITERATIONS_FOR_CURVES);
			ImGui::Text("Max number of iterations: %d", MAX_NUMBER_OF_ITERATIONS_FOR_CURVES);

			if (numberOfIterations <= MIN_NUMBER_OF_ITERATIONS_FOR_CURVES)		// if the number of iterations entered is <= 0, set it to 1.
				numberOfIterations = 1;
			else if (numberOfIterations > MAX_NUMBER_OF_ITERATIONS_FOR_CURVES)	// Setting the max of iterations to "MAX_NUMBER_OF_ITERATIONS_FOR_CURVES"
				numberOfIterations = MAX_NUMBER_OF_ITERATIONS_FOR_CURVES;

			/*
			// Checkbox (original)
			ImGui::Checkbox("Enable Feature", &checkboxValue);
			ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");
			*/

			// Checkbox
			ImGui::Checkbox("Show curve points", &showCurvePoints);
			ImGui::Checkbox("Delete curve points", &deleteControlPts);
			if (deleteControlPts)
				ImGui::Text("Feature Enabled: Delete Control Points");
			else
				ImGui::Text("Feature Enabled: Add/Move Control Points");

			// Scrollable block
			ImGui::TextWrapped("Active Control Points (Scrollable Block):");
			ImGui::BeginChild("ScrollableChild", ImVec2(0, 100), true); // Create a scrollable child
			for (int i = 0; i < controlPts.size(); i++) {
				vec3 v = controlPts[i];
				ImGui::Text("Vec3(%.5f, %.5f, %.5f)", v.x, v.y, v.z);	// Display active control points positions
			}
			ImGui::EndChild();

			ImGui::Checkbox("Reset window", &resetCurveWindowBool);
			if (resetCurveWindowBool)
			{
				resetCurveWindow();
				resetCurveWindowBool = false;
			}
		}
		// For camera views
		else
		{
			ImGui::Checkbox("Reset camera", &resetCamera);
			if (resetCamera)
			{
				resetOrbitViewWindow();
				resetCamera = false;
			}
		}

		/*
		// Displaying current values
		ImGui::Text("Slider Value: %.3f", sliderValue);
		ImGui::Text("Drag Value: %.3f", dragValue);
		ImGui::Text("[FLOAT] Input Value: %.3f", inputValue);
		ImGui::Text("[INT] Number of iterations: %d", numberOfIterations);
		*/

	}

	glm::vec3 getColor() const {
		return glm::vec3(colorValue[0], colorValue[1], colorValue[2]);
	}

	const char* getOptions()
	{
		return *(options);
	}

	int& getSelectedOption()
	{
		return comboSelection;
	}

private:
	float colorValue[3];  // Array for RGB color values
	char inputText[256];  // Buffer for input text
	int buttonClickCount; // Count button clicks
	float sliderValue;    // Value for float slider
	float dragValue;      // Value for drag input
	float inputValue;     // Value for float input
	bool checkboxValue;   // Value for checkbox
	int comboSelection;   // Index of selected option in combo box
	const char* options[6]; // Options for the combo box
	windowControlData& windowData;
	curveRelatedData& curveData;
	orbitViewerData& orbitViewData;
	vector<vec3>& controlPts = curveData.controlPts;
	int& numberOfIterations = curveData.numIterations;
	bool& showCurvePoints = curveData.showCurvePoints;
	bool& deleteControlPts = curveData.deleteControlPts;
	bool& resetCurveWindowBool = curveData.resetWindow;
	bool& resetCamera = orbitViewData.resetWindow;

	/// <summary>
	/// Clears the window of active control points
	/// </summary>
	void resetCurveWindow()
	{
		curveData.controlPts.clear();
		curveData.curve.clear();
	}

	/// <summary>
	/// Resets the camera view to default values
	/// </summary>
	void resetOrbitViewWindow()
	{
		defaultOrbitViewData df = orbitViewData.defaultData;
		orbitViewData.aspectRatio = df.aspectRatio;
		orbitViewData.cameraPos = df.cameraPos;
		orbitViewData.distanceFromLookAtPoint = df.distanceFromLookAtPoint;
		orbitViewData.farPlane = df.farPlane;
		orbitViewData.fieldOfView = df.fieldOfView;
		orbitViewData.mouseSensitivity = df.mouseSensitivity;
		orbitViewData.nearPlane = df.nearPlane;
		orbitViewData.phi = df.phi;
		orbitViewData.resetWindow = false;
		orbitViewData.scrollSensitivity = df.scrollSensitivity;
		orbitViewData.theta = df.theta;
	}
};

void checkOptionChosen(cpuGeometriesData& geoms, windowControlData& windowData, optionData& optionData)
{
	curveRelatedData curveData = windowData.curveRelatedData;
	vector<vec3> cp_positions_vector = curveData.controlPts;	// User entered control points
	vector<vec3> curveGenerated;								// Curve generated

	if (cp_positions_vector.size() >= 2)
	{
		switch (optionData.comboSelection)
		{
		case 0:	// Bezier curve
			//cout << "CHOSEN Bezier " << endl;	// debug

			// debug
			//cp_positions_vector = CurveGenerator::bezier(windowData.controlPts, 2, 0.5f); // change the hard coded numbers
			//cp_positions_vector = CurveGenerator::bezier(testPoints, 2, 0.5f); 	// expected result in std::vector is vec3(3.5, 1.5, 0)

			// Generate the curve
			for (float u = 0; u <= 1; u += (0.5f / curveData.numIterations))
			{
				vec3 currBezierPt = CurveGenerator::bezier(cp_positions_vector, 2, u)[0];
				curveGenerated.push_back(currBezierPt);
			}

			// ensure that the last point in the curve is the last control point entered
			curveGenerated.push_back(cp_positions_vector[cp_positions_vector.size() - 1]);

			break;

		case 1: // Quadratic B-spline (Chaikin) curve
			//cout << "CHOSEN Quadratic B-spline (Chaikin) " << endl; // debug

			curveGenerated = cp_positions_vector;							// Initial curve
			for (int i = 0; i < curveData.numIterations; i++)
			{
				curveGenerated = CurveGenerator::chaikin(curveGenerated);	// Create the curve so far
			}
			// ensure that the last point in the curve is the last control point entered
			curveGenerated.push_back(cp_positions_vector[cp_positions_vector.size() - 1]);
			break;

		default:
			//cout << "DEFUALT ENTERED OH NO!!" << endl;  // debug
			return;	// if i want to keep showing the curve previously generated curve, use 'break' if i don't
		}
	}

	CPU_Geometry& cp_point_cpu = geoms.cp_point_cpu;
	CPU_Geometry& cp_line_cpu = geoms.cp_line_cpu;
	CPU_Geometry& curve_point_cpu = geoms.curve_point_cpu;
	CPU_Geometry& curve_line_cpu = geoms.curve_line_cpu;

	// Do colouring
	vec3 cp_point_colour = { 1.f, 0.f, 0.f };
	vec3 curveColour = { 0.f, 0.5f, 1.f };	// Cyan

	// update the values for CPU_Geometries //
	
	// update user entered control points
	cp_point_cpu.verts = cp_positions_vector;	// contains only the points the user entered
	cp_point_cpu.cols = vector<vec3>(cp_point_cpu.verts.size(), cp_point_colour);	// set those points to red
	// set the color of those lines
	cp_line_cpu.verts = cp_positions_vector;
	cp_line_cpu.cols = vector<vec3>(cp_line_cpu.verts.size(), vec3(0.f, 1.f, 0.f));

	// update the curve
	curve_point_cpu.verts = curveGenerated;
	curve_point_cpu.cols = vector<vec3>(curve_point_cpu.verts.size(), vec3(1.f, 1.0f, 0.f));		// have an option to show them
	// update curve line
	curve_line_cpu.verts = curveGenerated;
	curve_line_cpu.cols = vector<vec3>(curve_line_cpu.verts.size(), curveColour);
}

/*
//void checkOptionChosen(CPU_Geometry& cp_point_cpu, CPU_Geometry& cp_line_cpu, windowControlPtData& windowData, optionData& optionData)
//{
//	//cout << "\n[CHECKING OPTION]" << endl;	// debug
//
//	vector<vec3> userControlPoints = windowData.controlPts;
//	vector<vec3> cp_positions_vector;	// positions to update
//
//	vector<vec3> cp_line_colours;
//	// Pair up control points such that it forms the green lines properly
//	for (int i = 1; i < userControlPoints.size(); i++)
//	{
//		cp_positions_vector.push_back(userControlPoints[i - 1]);
//		cp_positions_vector.push_back(userControlPoints[i]);
//
//		// Colors for the original control points entered
//		cp_line_colours.push_back(vec3(0.f, 1.f, 0.f));
//		cp_line_colours.push_back(vec3(0.f, 1.f, 0.f));
//	}
//	int sizeOfColorsForOriginalControlPts = cp_line_colours.size();
//
//	// Colors for the original control points entered
//	//for (int i = 0; i < cp_positions_vector.size(); i++)
//	//{
//	//	cp_line_colours.push_back(vec3(0.f,1.f,0.f));
//	//}
//
//	//vector<vec3> testPoints =
//	//{
//	//	vec3(4.f / 4.f ,0.f, 0.f),
//	//	vec3(8.f / 4.f,2.f / 4.f, 0.f),
//	//	vec3(0.f ,2.f / 4.f, 0.f),
//	//	vec3(0.f ,0.f, 0.f)
//	//};
//
//	vector<vec3> generatedCurveSoFar;
//	// Update control points for the cpu
	//switch (optionData.comboSelection)
	//{
	//case 0:	// Bezier curve
	//	//cout << "CHOSEN Bezier " << endl;	// debug

	//	// Cases
	//	if (userControlPoints.size() <= 0)		// No control points
	//		break;
	//	else if (userControlPoints.size() <= 1)	// 1 control point
	//	{
	//		cp_positions_vector.push_back(userControlPoints[0]);
	//		break;
	//	}

	//	// debug
	//	//cp_positions_vector = CurveGenerator::bezier(windowData.controlPts, 2, 0.5f); // change the hard coded numbers
	//	//cp_positions_vector = CurveGenerator::bezier(testPoints, 2, 0.5f); 	// expected result in std::vector is vec3(3.5, 1.5, 0)

	//	vec3 previousBezierPt = userControlPoints[0];
	//	//for (int i = 0; i < 1; i++)
	//	//{
	//		//vec3 previousBezierPt = controlPoints[0];
	//		// Generate the curve
	//		for (float u = 0; u <= 1; u += (0.5f / windowData.numIterations))
	//		{
	//			cp_positions_vector.push_back(previousBezierPt);
	//			vec3 currBezierPt = CurveGenerator::bezier(userControlPoints, 2, u)[0];
	//			cp_positions_vector.push_back(currBezierPt);
	//			previousBezierPt = currBezierPt;				// update the previously calculated point to this point

	//			//if (u != 0 && u != 1)
	//			//	generatedCurveSoFar.push_back(currBezierPt);
	//		}
	//		//// Set new control points
	//		//for (int i = 0; i < generatedCurveSoFar.size(); i++)
	//		//{
	//		//	controlPoints.push_back(generatedCurveSoFar[i]);
	//		//}
	//		//generatedCurveSoFar.clear();
	//		
	//	//}
	//	cp_positions_vector.push_back(userControlPoints[userControlPoints.size() - 1]);		// Adding the last control point so that the 'lines' are drawn properly

	//	break;
	//case 1: // Quadratic B-spline (Chaikin) curve
	//	//cout << "CHOSEN Quadratic B-spline (Chaikin) " << endl; // debug

	//	// Cases
	//	if (userControlPoints.size() <= 0)			// No control points
	//		break;
	//	else if (userControlPoints.size() <= 1)		// 1 control point
	//	{
	//		cp_positions_vector.push_back(userControlPoints[0]);
	//		break;
	//	}

	//	for (int i = 0; i < windowData.numIterations; i++)
	//	{
	//		generatedCurveSoFar = CurveGenerator::chaikin(userControlPoints);
	//		userControlPoints = generatedCurveSoFar;	// set it
	//	}

	//	// add the points generated into the cp_positions
	//	for (int j = 1; j < generatedCurveSoFar.size(); j++)
	//	{
	//		cp_positions_vector.push_back(generatedCurveSoFar[j - 1]);
	//		cp_positions_vector.push_back(generatedCurveSoFar[j]);
	//	}

	//	break;
	//default:
	//	cout << "DEFUALT ENTERED OH NO!!" << endl;  // debug
	//	break;
	//}
//
//	glm::vec3 cp_point_colour = { 1.f, 0.f, 0.f };
//	glm::vec3 cp_line_colour = { 0.f, 0.5f, 1.f };
//
//	// update the control points for cpu
//	cp_point_cpu.verts = cp_positions_vector;
//	cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
//
//	cp_line_cpu.verts = cp_positions_vector; // I changed it to use GL_LINES
//	// Set the colors of the generated curve's points
//	for (int i = sizeOfColorsForOriginalControlPts; i < cp_positions_vector.size(); i++)
//	{
//		cp_line_colours.push_back(cp_line_colour);
//	}
//	cp_line_cpu.cols = cp_line_colours;
//};
*/

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453: Assignment 3");
	Panel panel(window.getGLFWwindow());				// note to self: renders the debug panel it seems

	//GLDebug::enable();

	// note to self: make sure to comment out the vec3's inside
	std::vector<glm::vec3> cp_positions_vector =
	{
		{-.5f, -.5f, 0.f},
		{ .5f, -.5f, 0.f},
		{ .5f,  .5f, 0.f},
		{-.5f,  .5f, 0.f}
	};

	// Create structs
	vector<vec3> curve;
	int numberOfIOteration = DEFAULT_NUM_CURVE_ITERATIONS;
	bool showCurvePoints = false;
	bool deleteControlPts = false;
	bool clearCurveWindow = false;
	curveRelatedData curveData = { cp_positions_vector, curve, numberOfIOteration, showCurvePoints, deleteControlPts, clearCurveWindow };

	bool clearViewWindow = false;
	orbitViewerData orbitViewData = { clearViewWindow };

	windowControlData windowData = { window, curveData, orbitViewData };

	// CALLBACKS
	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>(windowData);

	optionData optionData = {curve_editor_panel_renderer->getSelectedOption(), curve_editor_panel_renderer->getOptions()};

	auto curve_editor_callback = std::make_shared<CurveEditorCallBack>(windowData, optionData);
	//auto turn_table_3D_viewer_callback = std::make_shared<TurnTable3DViewerCallBack>();

	//Set callback to window
	window.setCallbacks(curve_editor_callback);
	// Can swap the callback instead of maintaining a state machine
	//window.setCallbacks(turn_table_3D_viewer_callback);

	//Panel inputs
	panel.setPanelRenderer(curve_editor_panel_renderer);

	ShaderProgram shader_program_default(
		"shaders/test.vert",
		"shaders/test.frag"
	);

	glm::vec3 cp_point_colour	= { 1.f,0.f,0.f };
	glm::vec3 cp_line_colour	= { 0.f,1.f,0.f };

	CPU_Geometry cp_point_cpu;
	cp_point_cpu.verts	= cp_positions_vector;
	cp_point_cpu.cols	= std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
	GPU_Geometry cp_point_gpu;
	//cp_point_gpu.setVerts(cp_point_cpu.verts);
	//cp_point_gpu.setCols(cp_point_cpu.cols);

	CPU_Geometry cp_line_cpu;
	cp_line_cpu.verts	= cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
	cp_line_cpu.cols	= std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
	GPU_Geometry cp_line_gpu;
	//cp_line_gpu.setVerts(cp_line_cpu.verts);
	//cp_line_gpu.setCols(cp_line_cpu.cols);

	CPU_Geometry curve_point_cpu;
	GPU_Geometry curve_point_gpu;
	CPU_Geometry curve_line_cpu;
	GPU_Geometry curve_line_gpu;

	cpuGeometriesData geometries = {cp_point_cpu, cp_line_cpu, curve_point_cpu, curve_line_cpu };

	// Testing things
	//testDeCasteljauAlgo();
	vec2 ptSize = vec2(CONTROL_POINT_SIZE / 1000.f, CONTROL_POINT_SIZE / 1000.f);	// use this to get the red 'box' corner values that surrounds the control points, use these as bounds when trying to draw/select them
	cout << "half length/width: " << to_string(ptSize) << endl;	// te above ^ is half the length/width
	cout << "cp_point length: " << CONTROL_POINT_LENGTH << endl;
	cout << "cp_point width: " << CONTROL_POINT_WIDTH << endl;

	while (!window.shouldClose()) {
		glfwPollEvents();

		//---- note to self: do the settting of cpu stuff in here (checkOptionChosen)
		checkOptionChosen(geometries, windowData, optionData);

		//// update the control points for cpu
		//cp_point_cpu.verts = cp_positions_vector;
		//cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);

		//cp_line_cpu.verts = cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
		//cp_line_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
		//---

		// set the new control points for gpu
		cp_point_gpu.setVerts(cp_point_cpu.verts);
		cp_point_gpu.setCols(cp_point_cpu.cols);

		cp_line_gpu.setVerts(cp_line_cpu.verts);
		cp_line_gpu.setCols(cp_line_cpu.cols);

		curve_point_gpu.setVerts(curve_point_cpu.verts);
		curve_point_gpu.setCols(curve_point_cpu.cols);

		curve_line_gpu.setVerts(curve_line_cpu.verts);
		curve_line_gpu.setCols(curve_line_cpu.cols);

		glm::vec3 background_colour = curve_editor_panel_renderer->getColor();

		//------------------------------------------
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//------------------------------------------
		
		// Use the default shader (can use different ones for different objects)
		shader_program_default.use();

		// Initialize options for the combo box
		//options[0] = "Curve Editor - Bezier";
		//options[1] = "Curve Editor - Quadratic B-spline (Chaikin)";
		//options[2] = "Orbit Viewer - Curve";
		//options[3] = "Orbit Viewer - Surface of Revolution";
		//options[4] = "Orbit Viewer - Tensor Product 1";
		//options[5] = "Orbit Viewer - Tensor Product 2";

		// Set the view matrix for curve editors
		if (optionData.comboSelection == 0 || optionData.comboSelection == 1)
		{
			shader_program_default.setMat4Transform("transformationMatrix", curveData.defaultViewMat);
		}
		// Set the view matrix for all other options
		else
		{
			mat4 model = mat4(1.0f);
			mat4 viewMat = glm::lookAt(orbitViewData.cameraPos, orbitViewData.lookAtPoint, orbitViewData.defaultData.upVector);
			mat4 projectionMat = orbitViewData.defaultData.perspectiveMat4;
			glm::mat4 modelViewProjection = projectionMat * viewMat * model;
			shader_program_default.setMat4Transform("transformationMatrix", modelViewProjection);

		}

		//Render control points
		cp_point_gpu.bind();
		glPointSize(CONTROL_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

		//Render curve connecting control points
		cp_line_gpu.bind();
		//glLineWidth(10.f); //May do nothing (like it does on my computer): https://community.khronos.org/t/3-0-wide-lines-deprecated/55426
		glDrawArrays(GL_LINE_STRIP, 0, cp_line_cpu.verts.size());

		if (curveData.showCurvePoints)
		{
			// Render points generated for the curve
			curve_point_gpu.bind();
			glPointSize(5.f);
			glDrawArrays(GL_POINTS, 0, curve_point_cpu.verts.size());
		}
		// Render curve generated
		curve_line_gpu.bind();
		glDrawArrays(GL_LINE_STRIP, 0, curve_line_cpu.verts.size());

		//------------------------------------------
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		panel.render();
		//------------------------------------------
		window.swapBuffers();
		//------------------------------------------
	}

	glfwTerminate();
	return 0;
}

/// <summary>
/// Takes in a "std::vector(glm::vec3)" and returns a string representation of it.
/// </summary>
/// <param name="arr"></param>
/// <returns></returns>
string toString(vector<vec3> arr)
{
	string ret = "\n[";

	for (int i = 0; i < arr.size(); i++)
	{
		ret += to_string(arr[i]);
		if (i + 1 != arr.size())
			ret += ",\n";
	}
	ret += ']';

	return ret;
}

void testDeCasteljauAlgo()
{
	vector<vec3> controlPts =
	{
		vec3(4.f ,0.f, 0.f),
		vec3(8.f ,2.f, 0.f),
		vec3(0.f ,2.f, 0.f),
		vec3(0.f ,0.f, 0.f)
	};

	vector<vec3> controlPts1 =
	{
		vec3(4.f ,0.f, 0.f),
		vec3(8.f ,2.f, 0.f)
		//vec3(0.f ,2.f, 0.f),
		//vec3(0.f ,0.f, 0.f)
	};
	vector<vec3> controlPts2 =
	{
		//vec3(4.f ,0.f, 0.f),
		vec3(8.f ,2.f, 0.f),
		vec3(0.f ,2.f, 0.f)
		//vec3(0.f ,0.f, 0.f)
	};

	vector<vec3> controlPts3 =
	{
		//vec3(4.f ,0.f, 0.f),
		//vec3(8.f ,2.f, 0.f),
		vec3(0.f ,2.f, 0.f),
		vec3(0.f ,0.f, 0.f)
	};
	vector<vec3> result(controlPts.size());

	//result[0] = deCasteljauAlgo(controlPts1, 3, 0.5);
	//result[1] = deCasteljauAlgo(controlPts2, 3, 0.5);
	//result[2] = deCasteljauAlgo(controlPts3, 3, 0.5);

	//result = CurveGenerator::bezierCurve(controlPts, 3, 0.5f);
	result = CurveGenerator::bezier(controlPts, 3, 0.5f);
	cout << "\n[TEST] control Points: " << toString(controlPts) << endl;
	cout << "\n[TEST] bezier pt result: " <<  toString(result) << endl;
	// expected result is vec3(3.5, 1.5, 0)

	//result = CurveGenerator::bezierCurve(controlPts, 1000, 0.5f);
	result = CurveGenerator::bezier(controlPts, 300, 0.5f);
	cout << "\n[TEST] bezier pt result: " << toString(result) << endl;
}
