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

void testDeCasteljauAlgo();

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

class CurveEditorCallBack : public CallbackInterface {
public:
	CurveEditorCallBack() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) override
	{
		Log::info("KEEEEEYYYYY KeyCallback: key={}, action={}", key, action);
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override
	{
		Log::info("MouseButtonCallback: button={}, action={}", button, action);
	}

	virtual void cursorPosCallback(double xpos, double ypos) override
	{
		Log::info("POS CursorPosCallback: xpos={}, ypos={}", xpos, ypos);
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
};

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

class CurveEditorPanelRenderer : public PanelRendererInterface {
public:
	CurveEditorPanelRenderer():
		inputText(""),
		buttonClickCount(0),
		sliderValue(0.0f),
		dragValue(0.0f),
		inputValue(0.0f),
		checkboxValue(false),
		comboSelection(0)
	{
		// Initialize options for the combo box
		options[0] = "Option 1";
		options[1] = "Option 2";
		options[2] = "Option 3";

		// Initialize color (white by default)
		colorValue[0] = 1.0f; // R
		colorValue[1] = 1.0f; // G
		colorValue[2] = 1.0f; // B
	}

	virtual void render() override {
		// Color selector
		ImGui::ColorEdit3("Select Background Color", colorValue); // RGB color selector
		ImGui::Text("Selected Color: R: %.3f, G: %.3f, B: %.3f", colorValue[0], colorValue[1], colorValue[2]);

		// Text input
		ImGui::InputText("Input Text", inputText, IM_ARRAYSIZE(inputText));

		// Display the input text
		ImGui::Text("You entered: %s", inputText);

		// Button
		if (ImGui::Button("Click Me")) {
			buttonClickCount++;
		}
		ImGui::Text("Button clicked %d times", buttonClickCount);

		// Scrollable block
		ImGui::TextWrapped("Scrollable Block:");
		ImGui::BeginChild("ScrollableChild", ImVec2(0, 100), true); // Create a scrollable child
		for (int i = 0; i < 20; i++) {
			ImGui::Text("Item %d", i);
		}
		ImGui::EndChild();

		// Float slider
		ImGui::SliderFloat("Float Slider", &sliderValue, 0.0f, 100.0f, "Slider Value: %.3f");

		// Float drag
		ImGui::DragFloat("Float Drag", &dragValue, 0.1f, 0.0f, 100.0f, "Drag Value: %.3f");

		// Float input
		ImGui::InputFloat("Float Input", &inputValue, 0.1f, 1.0f, "Input Value: %.3f");

		// Checkbox
		ImGui::Checkbox("Enable Feature", &checkboxValue);
		ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");

		// Combo box
		ImGui::Combo("Select an Option", &comboSelection, options, IM_ARRAYSIZE(options));
		ImGui::Text("Selected: %s", options[comboSelection]);

		// Displaying current values
		ImGui::Text("Slider Value: %.3f", sliderValue);
		ImGui::Text("Drag Value: %.3f", dragValue);
		ImGui::Text("Input Value: %.3f", inputValue);
	}

	glm::vec3 getColor() const {
		return glm::vec3(colorValue[0], colorValue[1], colorValue[2]);
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
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453: Assignment 3");
	Panel panel(window.getGLFWwindow());

	//GLDebug::enable();

	// CALLBACKS
	auto curve_editor_callback = std::make_shared<CurveEditorCallBack>();
	//auto turn_table_3D_viewer_callback = std::make_shared<TurnTable3DViewerCallBack>();

	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>();

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

	std::vector<glm::vec3> cp_positions_vector =
	{
		{-.5f, -.5f, 0.f},
		{ .5f, -.5f, 0.f},
		{ .5f,  .5f, 0.f},
		{-.5f,  .5f, 0.f}
	};
	glm::vec3 cp_point_colour	= { 1.f,0.f,0.f };
	glm::vec3 cp_line_colour	= { 0.f,1.f,0.f };

	CPU_Geometry cp_point_cpu;
	cp_point_cpu.verts	= cp_positions_vector;
	cp_point_cpu.cols	= std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
	GPU_Geometry cp_point_gpu;
	cp_point_gpu.setVerts(cp_point_cpu.verts);
	cp_point_gpu.setCols(cp_point_cpu.cols);

	CPU_Geometry cp_line_cpu;
	cp_line_cpu.verts	= cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
	cp_line_cpu.cols	= std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
	GPU_Geometry cp_line_gpu;
	cp_line_gpu.setVerts(cp_line_cpu.verts);
	cp_line_gpu.setCols(cp_line_cpu.cols);


	// Testing things
	testDeCasteljauAlgo();



	while (!window.shouldClose()) {
		glfwPollEvents();
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
	cout << "\n[TEST] deCasteljau algo result: " <<  toString(result) << endl;

	//result = CurveGenerator::bezierCurve(controlPts, 1000, 0.5f);
	result = CurveGenerator::bezier(controlPts, 300, 0.5f);
	cout << "\n[TEST] deCasteljau algo result: " << toString(result) << endl;
}
