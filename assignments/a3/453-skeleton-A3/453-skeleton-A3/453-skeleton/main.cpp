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

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace glm;

const static float POINT_SIZE = 15.f;	// in pixel space? -> but divide by 1000.f to get clip space value 
const static int MAX_NUMBER_OF_ITERATIONS_FOR_CURVES = 20;
const static int MIN_NUMBER_OF_ITERATIONS_FOR_CURVES = 1;

void testDeCasteljauAlgo();
string toString(vector<vec3> arr);

struct windowControlPtData
{
	Window& window;
	vector<vec3>& controlPts;	// The control points the user entered
	vector<vec3>& curve;		// points that represent the generated curve
	int& numIterations;
	bool showCurvePoints = false;	// to show curve points
};

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
	CurveEditorCallBack(windowControlPtData& data, Panel& p) :
		data(data),
		win(data.window),
		panel(p),
		controlPoints(data.controlPts)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) override
	{
		Log::info("KEEEEEYYYYY KeyCallback: key={}, action={}", key, action);
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override
	{
		Log::info("[MOUSE-BUTTON-CALL-BACK]: button={}, action={}", button, action);
		mouseButton = button;
		mouseAction = action;

		if (button == 0 && action == 0)		// place a control point
		{
			controlPoints.push_back(vec3(mousePos, 0.f));
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

		//vec2 ptSize = vec2(POINT_SIZE / 1000.f, POINT_SIZE / 1000.f);	// this is the length and width of the red 'box' that surrounds the control points, use these as bounds when trying to draw/select them
		//cout << to_string(ptSize) << endl;
	}

	virtual void scrollCallback(double xoffset, double yoffset) override
	{
		Log::info("ScrollCallback: xoffset={}, yoffset={}", xoffset, yoffset);
	}

	virtual void windowSizeCallback(int width, int height) override
	{
		Log::info("WindowSizeCallback: width={}, height={}", width, height);
		CallbackInterface::windowSizeCallback(width, height); // Important, calls glViewport(0, 0, width, height);
	}
private:
	windowControlPtData& data;
	Window& win = data.window;
	Panel& panel;
	vector<vec3>& controlPoints = data.controlPts;
	int mouseButton; // 0: left mouse button | 1: right mouse button
	int mouseAction; // 0: NOT held | 1: held (pressed down)
	vec2 mousePos;	// mouse position in CLIP SPACE

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
	CurveEditorPanelRenderer(windowControlPtData& d) :
		inputText(""),
		buttonClickCount(0),
		sliderValue(0.0f),
		dragValue(0.0f),
		inputValue(0.0f),
		checkboxValue(false),
		comboSelection(0),
		data(d)
	{
		// Initialize options for the combo box
		options[0] = "Bezier Curve";
		options[1] = "Quadratic B-spline (Chaikin)";
		options[2] = "Option 3 (NULL)";

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

		// Scrollable block
		ImGui::TextWrapped("Active Control Points (Scrollable Block):");
		ImGui::BeginChild("ScrollableChild", ImVec2(0, 100), true); // Create a scrollable child
		for (int i = 0; i < controlPts.size(); i++) {
			vec3 v = controlPts[i];
			ImGui::Text("Vec3(%.5f, %.5f, %.5f)", v.x, v.y, v.z);	// Display active control points positions
		}
		ImGui::EndChild();

		/*
		// Float slider
		ImGui::SliderFloat("Float Slider", &sliderValue, 0.0f, 100.0f, "Slider Value: %.3f");

		// Float drag
		ImGui::DragFloat("Float Drag", &dragValue, 0.1f, 0.0f, 100.0f, "Drag Value: %.3f");

		// Float input
		ImGui::InputFloat("Float Input", &inputValue, 0.1f, 1.0f, "Input Value: %.3f");
		*/

		// Number of iterations input
		//ImGui::InputInt("Iterations", &numberOfIterations, 1, 1);
		ImGui::SliderInt("Iterations", &numberOfIterations, MIN_NUMBER_OF_ITERATIONS_FOR_CURVES, MAX_NUMBER_OF_ITERATIONS_FOR_CURVES, "Drag Value: %d");
		ImGui::Text("Min number of iterations: %d", MIN_NUMBER_OF_ITERATIONS_FOR_CURVES);
		ImGui::Text("Max number of iterations: %d", MAX_NUMBER_OF_ITERATIONS_FOR_CURVES);

		if (numberOfIterations <= MIN_NUMBER_OF_ITERATIONS_FOR_CURVES)		// if the number of iterations entered is <= 0, set it to 1.
			numberOfIterations = 1;
		else if (numberOfIterations > MAX_NUMBER_OF_ITERATIONS_FOR_CURVES)	// Setting the max of iterations to "MAX_NUMBER_OF_ITERATIONS_FOR_CURVES"
			numberOfIterations = MAX_NUMBER_OF_ITERATIONS_FOR_CURVES;

		// Combo box
		ImGui::Combo("Select an Option", &comboSelection, options, IM_ARRAYSIZE(options));
		ImGui::Text("Selected: %s", options[comboSelection]);

		/*
		// Checkbox (original)
		ImGui::Checkbox("Enable Feature", &checkboxValue);
		ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");
		*/

		// Checkbox
		ImGui::Checkbox("Show curve points", &showCurvePoints);
		ImGui::Text("Feature Enabled: %s", &showCurvePoints ? "Yes" : "No");
		data.showCurvePoints = showCurvePoints;
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
	const char* options[3]; // Options for the combo box
	windowControlPtData& data;
	vector<vec3>& controlPts = data.controlPts;
	int& numberOfIterations = data.numIterations;
	bool& showCurvePoints = data.showCurvePoints;
};

void checkOptionChosen(cpuGeometriesData& geoms, windowControlPtData& windowData, optionData& optionData)
{
	vector<vec3> cp_positions_vector = windowData.controlPts;	// User entered control points
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
			for (float u = 0; u <= 1; u += (0.5f / windowData.numIterations))
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
			for (int i = 0; i < windowData.numIterations; i++)
			{
				curveGenerated = CurveGenerator::chaikin(curveGenerated);	// Create the curve so far
			}

			break;

		default:
			cout << "DEFUALT ENTERED OH NO!!" << endl;  // debug
			break;
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
	curve_line_cpu.cols = vector<vec3>(curve_line_cpu.verts.size() + 1, curveColour);
}

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

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453: Assignment 3");
	Panel panel(window.getGLFWwindow());				// note to self: renders the debug panel it seems

	//GLDebug::enable();

	std::vector<glm::vec3> cp_positions_vector =
	{
		//{-.5f, -.5f, 0.f},
		//{ .5f, -.5f, 0.f},
		////{ .5f, -.5f, 0.f},
		//{ .5f,  .5f, 0.f},
		////{ .5f,  .5f, 0.f},
		//{-.5f,  .5f, 0.f}
	};
	vector<vec3> curve;

	int numberOfIOteration = 1;
	windowControlPtData windowData = { window, cp_positions_vector, curve, numberOfIOteration };

	// CALLBACKS

	auto curve_editor_callback = std::make_shared<CurveEditorCallBack>(windowData, panel);
	//auto turn_table_3D_viewer_callback = std::make_shared<TurnTable3DViewerCallBack>();

	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>(windowData);
	optionData optionData = {curve_editor_panel_renderer->getSelectedOption(), curve_editor_panel_renderer->getOptions()};

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

	//std::vector<glm::vec3> cp_positions_vector =
	//{
	//	{-.5f, -.5f, 0.f},
	//	{ .5f, -.5f, 0.f},
	//	{ .5f,  .5f, 0.f},
	//	{-.5f,  .5f, 0.f}
	//};
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
	//vec2 ptSize = vec2(POINT_SIZE / 1000.f, POINT_SIZE / 1000.f);	// use this to get the red 'box' corner values that surrounds the control points, use these as bounds when trying to draw/select them
	//cout << to_string(ptSize) << endl;

	while (!window.shouldClose()) {
		glfwPollEvents();

		//---- do the settting of cpu stuff in here (checkOptionChosen)
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


		//Render control points
		cp_point_gpu.bind();
		glPointSize(15.f);
		glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

		//Render curve connecting control points
		cp_line_gpu.bind();
		//glLineWidth(10.f); //May do nothing (like it does on my computer): https://community.khronos.org/t/3-0-wide-lines-deprecated/55426
		glDrawArrays(GL_LINE_STRIP, 0, cp_line_cpu.verts.size());

		if (windowData.showCurvePoints)
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
