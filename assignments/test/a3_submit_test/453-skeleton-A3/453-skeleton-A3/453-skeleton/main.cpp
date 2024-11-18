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

#include <GLFW/glfw3.h>	//idk

#include <tuple>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace glm;


/* CPSC 453 A3 Bezier, B-Spline Curves, and Surfaces | Benny Liang 30192142 */

const static double PI_APPROX = atan(1) * 4;										// pi approximation
const static float CONTROL_POINT_SIZE = 15.f;										// Size of the control point's box
const static float CONTROL_POINT_LENGTH = 2 * (CONTROL_POINT_SIZE / 1000.f);		// Length of the control point's box
const static float CONTROL_POINT_WIDTH = CONTROL_POINT_LENGTH;						// Width of the control point's box
const static int DEFAULT_NUM_CURVE_ITERATIONS = 6;									// the default number of iterations for curve generation
const static int MAX_NUMBER_OF_ITERATIONS_FOR_CURVES = 20;
const static int MIN_NUMBER_OF_ITERATIONS_FOR_CURVES = 1;
const static int MAX_NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION = 200;
const static int MIN_NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION = 2;

const static int MIN_U_DEGREE = 1;
const static int MAX_U_DEGREE = 4;
const static int MIN_V_DEGREE = 1;
const static int MAX_V_DEGREE = 4;
const static int MIN_QUALITY = 20;
const static int MAX_QUALITY = 70;

const static float MIN_NEAR_PLANE = 0.1f;
const static float MAX_NEAR_PLANE = 4.f;
const static float MIN_FAR_PLANE = 0.2f;
const static float MAX_FAR_PLANE = 10.f;

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
	bool& resetCamera;

	float fieldOfView = glm::radians(45.0f);
	float aspectRatio = 1.f;
	float nearPlane = 0.1f;
	float farPlane = 100.f;
	const vec3 lookAtPoint = vec3(0.f);		// looking at the origin (0.0, 0.0, 0.0)
	vec3 cameraPos = vec3(vec2(0.f), 3.0f);	// initial at 3.0f

	float mouseSensitivity = 100.0f;		// How fast the camera movement should be
	float scrollSensitivity = 1.0f;		// How fast the scroll in/out should be
	float distanceFromLookAtPoint = 3.f;	// Distance from the 'lookAtPoint' (origin) (world space)
	float theta = glm::radians(90.f);		// Ensure radians
	float phi = glm::radians(90.f);			// Ensure radians
	bool showWireFrame = false;				// switch between wireframe and solid surface

	orbitViewerData(bool& resetWindow) :
		resetCamera(resetWindow),
		defaultData(defaultOrbitViewData())
	{
	}
};

struct tensorProductData
{
	// From assignment outline
	const vector<vector<vec3>> tensorSurfaceControlPoints1 =
	{
			{{-2, 0, -2}, {-1, 0, -2}, {0, 0, -2}, {1, 0, -2}, {2, 0, -2}},
			{{-2, 0, -1}, {-1, 1, -1}, {0, 1, -1}, {1, 1, -1}, {2, 0, -1}},
			{{-2, 0, 0}, {-1, 1, 0}, {0, -1, 0}, {1, 1, 0}, {2, 0, 0}},
			{{-2, 0, 1}, {-1, 1, 1}, {0, 1, 1}, {1, 1, 1}, {2, 0, 1}},
			{{-2, 0, 2}, {-1, 0, 2}, {0, 0, 2}, {1, 0, 2}, {2, 0, 2}}
	};

	// Self generated
	const vector<vector<vec3>> tensorSurfaceControlPoints2 =
	{
			{{-2, -4, -3}, {-1, -2, -3}, {0, 0, -3}, {1, 0, -3}, {2, 0, -3}},
			{{-2, -3, -2}, {-1, -2, -2}, {0, 0, -2}, {1, 0, -2}, {2, 0, -2}},
			{{-2, -2, -1}, {-1, -1, -1}, {0, 1, -1}, {1, 1, -1}, {2, 0, -1}},
			{{-2, 0, 0}, {-1, 1, 0}, {0, -1, 0}, {1, 1, 0}, {2, 0, 0}},
			{{-2, 0, 1}, {-1, 1, 1}, {0, 1, 1}, {1, 1, 1}, {2, 0, 1}},
			{{-2, 0, 2}, {-1, 1, 2}, {0, 2, 2}, {1, 6, 2}, {2, 0, 2}},
			{{-2, 0, 3}, {-1, 0, 3}, {0, 0, 3}, {1, 0, 3}, {2, 0, 3}}

	};

	int uDegree = 2;
	int vDegree = 2;
	int quality = 40;
};

struct windowControlData
{
	Window& window;
	curveRelatedData& curveRelatedData;
	orbitViewerData& orbitViewerData;
	tensorProductData& tensorProductData;
	bool& enableBonus;
	int& numberOfSlices;			// ONLY used by Surface of revolution
	int previousOptionChosen = -1;
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
	bool& swapCurveSelectionForViewer;	// only used for "Orbit viwer - Curve"
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

	/// <summary>
	/// Generates an open polynomial B-Spline curve.
	/// </summary>
	/// <param name="coursePts"> control points</param>
	/// <returns> a std::vector(glm::vec3) that contains the open polynomial B-spine curve.</returns>
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

class SurfaceOfRevolution
{
public:

	/// <summary>
	/// Takes in a B-Spline curve and the desired number of "slices" (larger slice value = smoother surface).
	/// Returns a tuple containing the mesh point positions, and triangles that make up the mesh.
	/// 1st element: mesh point positions, std::vector(vec3).
	/// 2nd element: triangle mesh, std::vector(vec3).
	/// </summary>
	/// <param name="bSplineCurve"> the b-spline curve to make the surface from.</param>
	/// <param name="numSlices"> how many 'slices' wanted when generating the surface.</param>
	/// <returns></returns>
	static tuple<vector<vec3>, vector<vec3>> generate(vector<vec3> bSplineCurve, int numSlices)
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

			// First triangle
			for (int j = 0; j < currentSetOfPoints.size() - 1; j++)	// minus 1, as we can't draw a triangle with other points left
			{
				meshTriangles.push_back(currentSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j + 1]);
			}
			// Second triangle
			for (int j = 0; j < currentSetOfPoints.size() - 1; j++)
			{
				meshTriangles.push_back(currentSetOfPoints[j]);
				meshTriangles.push_back(nextSetOfPoints[j + 1]);
				meshTriangles.push_back(currentSetOfPoints[j + 1]);

			}
			// result is a square/rectangle defined by these two triangles (triangles are drawn counter clock wise)
		}

		return make_tuple(meshPoints, meshTriangles);
	};
};

class TensorProductSurface
{
public:
	/// <summary>
	/// Element 1 is surface points
	/// Element 2 is the triangle mesh
	/// </summary>
	/// <param name="resolutionU"></param>
	/// <param name="resolutionV"></param>
	/// <returns>a tuple of (vector(vec3), vector(vec3))</returns>
	static tuple<vector<vec3>, vector<vec3>> generate(vector<vector<vec3>> controlPoints, int uDegree, int vDegree, int resolutionU, int resolutionV)
	{
		vector<vec3> surfacePoints;
		vector<vector<vec3>> surface;
		vector<vec3> col;
		for (int i = 0; i < resolutionU - 1; ++i)
		{
			vector<vec3> surfaceSoFar;

			for (int j = 0; j < resolutionV - 1; ++j)
			{
				float u0 = static_cast<float>(i) / (resolutionU - 1);
				float u1 = static_cast<float>(i + 1) / (resolutionU - 1);
				float v0 = static_cast<float>(j) / (resolutionV - 1);
				float v1 = static_cast<float>(j + 1) / (resolutionV - 1);

				vec3 p0 = evaluate(controlPoints, uDegree, vDegree, u0, v0);
				vec3 p1 = evaluate(controlPoints, uDegree, vDegree, u1, v0);
				vec3 p2 = evaluate(controlPoints, uDegree, vDegree, u1, v1);
				vec3 p3 = evaluate(controlPoints, uDegree, vDegree, u0, v1);

				if (p0 != vec3(0.f))
					surfaceSoFar.push_back(p0);
				if (p1 != vec3(0.f))
					surfaceSoFar.push_back(p1);
				if (p2 != vec3(0.f))
					surfaceSoFar.push_back(p2);
				if (p3 != vec3(0.f))
					surfaceSoFar.push_back(p3);
			}

			for (int k = 0; k < surfaceSoFar.size(); k++)
				surfacePoints.push_back(surfaceSoFar[k]);

			surface.push_back(surfaceSoFar);	// One set of curves that define the surface
		}


		vector<vec3> triangleMesh;

		// Create mesh
		for (int i = 0; i < surface.size() - 2; i++)
		{
			vector<vec3> currentCurve = surface[i];
			vector<vec3> nextCurve = surface[i + 1];


			for (int j = 0; j < currentCurve.size() - 1; j++)
			{
				// Triangle 1
				triangleMesh.push_back(currentCurve[j]);
				triangleMesh.push_back(nextCurve[j]);
				triangleMesh.push_back(nextCurve[j + 1]);
				//Triangle 2
				triangleMesh.push_back(currentCurve[j]);
				triangleMesh.push_back(nextCurve[j + 1]);
				triangleMesh.push_back(currentCurve[j + 1]);
			}
		}

		return make_tuple(surfacePoints, triangleMesh);
	}

private:

	/// <summary>
	/// Generates the B-spline coefficient.
	/// </summary>
	/// <param name="i"></param>
	/// <param name="k"></param>
	/// <param name="t"></param>
	/// <param name="knots"></param>
	/// <returns></returns>
	static float basisFunction(int i, int k, float t, const vector<float>& knots)
	{
		if (k == 0) {
			return (t >= knots[i] && t < knots[i + 1]) ? 1.0f : 0.0f;
		}
		float denom1 = knots[i + k] - knots[i];
		float denom2 = knots[i + k + 1] - knots[i + 1];

		float term1 = denom1 > 0 ? (t - knots[i]) / denom1 * basisFunction(i, k - 1, t, knots) : 0.0f;
		float term2 = denom2 > 0 ? (knots[i + k + 1] - t) / denom2 * basisFunction(i + 1, k - 1, t, knots) : 0.0f;

		return term1 + term2;
	}

	/// <summary>
	/// Calculates the Q(u,v) point for the surface.
	/// </summary>
	/// <param name="controlPoints"></param>
	/// <param name="uDegree"></param>
	/// <param name="vDegree"></param>
	/// <param name="u"></param>
	/// <param name="v"></param>
	/// <returns></returns>
	static vec3 evaluate(vector<vector<vec3>> controlPoints, int uDegree, int vDegree, float u, float v)
	{
		vec3 point(0.0f);
		int n = controlPoints.size();
		int m = controlPoints[0].size();

		vector<float> uKnots = generateKnotVector(n, uDegree);
		vector<float> vKnots = generateKnotVector(m, vDegree);;

		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < m; ++j)
			{
				float bu = basisFunction(i, uDegree, u, uKnots);	// Calculate weight for curve along 'u'
				float bv = basisFunction(j, vDegree, v, vKnots);	// Calculate weight for curve along 'v'
				point += bu * bv * controlPoints[i][j];				// apply weights to control point
			}
		}
		return point;
	}

	/// <summary>
	/// Generates a vector of 'knots' for 'u' or 'v' curve
	/// </summary>
	/// <param name="numControlPoints"></param>
	/// <param name="degree"></param>
	/// <returns></returns>
	static vector<float> generateKnotVector(int numControlPoints, int degree)
	{
		int numKnots = numControlPoints + degree + 1;
		std::vector<float> knots(numKnots);

		// Generate a clamped uniform knot vector
		for (int i = 0; i < numKnots; ++i) {
			if (i < degree + 1) {
				knots[i] = 0.0f; // Start clamp
			}
			else if (i >= numKnots - degree - 1) {
				knots[i] = 1.0f; // End clamp
			}
			else {
				knots[i] = float(i - degree) / float(numControlPoints - degree);
			}
		}

		return knots;
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
				cout << "No longer moving cp" << endl;	// debug
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
					//cout << "FOUND POINT TO REMOVE" << endl;	// debug

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
			}
		}

		// note
		// when holding the left-mouse button the 'action' stays at 1 and 'button' is 0 -> release action = 0
		// when holding the right-mouse button the 'action' stays at 1 and 'button is 1 -> release action = 0
	}

	virtual void cursorPosCallback(double xpos, double ypos) override
	{
		//Log::info("[CURSOR-POS-CALLBACK PIXEL]: xpos={}, ypos={}", xpos, ypos);
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

				update3DCameraPos();
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

			orbitViewData.distanceFromLookAtPoint += (-yoffset * orbitViewData.scrollSensitivity);	// '-yoffset' so scroll up is zoom in and scroll down is zoom out

			// Constrain the distance so that it doesn't pass the 'lookAtPoint' (origin)
			if (orbitViewData.distanceFromLookAtPoint < 0.2)
				orbitViewData.distanceFromLookAtPoint = 0.2;

			cout << "(After scroll) distance from origin = " << orbitViewData.distanceFromLookAtPoint << endl;	// debug

			update3DCameraPos();
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

		// Adjust aspect ratio
		if (height <= width)
			orbitViewData.aspectRatio = (float)width / (float)height;
		else
			orbitViewData.aspectRatio = 1.f / ((float)height / (float)width);

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
	void update3DCameraPos()
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

};

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
		tensorData(d.tensorProductData),
		windowData(d)
	{
		// Initialize options for the combo box
		options[0] = "Curve Editor - Bezier";
		options[1] = "Curve Editor - Quadratic B-spline (Chaikin)";
		options[2] = "Orbit Viewer - Curve";
		options[3] = "Orbit Viewer - Surface of Revolution";
		options[4] = "Orbit Viewer - Tensor Product 1";
		options[5] = "Orbit Viewer - Tensor Product 2";

		// Initialize color (grey by default)
		colorValue[0] = 0.5f; // R
		colorValue[1] = 0.5f; // G
		colorValue[2] = 0.5f; // B

		swapCurveSelectionForViewer = false;
	}

	virtual void render() override
	{
		//ImGui::Checkbox("Enable Bonus 1", &enableBonus1);

		// Color selector
		ImGui::ColorEdit3("Select Background Color", colorValue); // RGB color selector
		ImGui::Text("Selected Color: R: %.3f, G: %.3f, B: %.3f", colorValue[0], colorValue[1], colorValue[2]);

		// Combo box
		ImGui::Combo("Select an Option", &comboSelection, options, IM_ARRAYSIZE(options));
		//ImGui::Text("Selected: %s", options[comboSelection]);

		// For Bezier and Chaikin (polynomial b-spline) curve options
		if (comboSelection == 0 || comboSelection == 1)
		{
			// Number of iterations input
			//ImGui::InputInt("Iterations", &numberOfIterations, 1, 1);
			ImGui::SliderInt("Iterations", &numberOfIterations, MIN_NUMBER_OF_ITERATIONS_FOR_CURVES, MAX_NUMBER_OF_ITERATIONS_FOR_CURVES, "Iteration: %d");
			ImGui::Text("Min number of iterations: %d", MIN_NUMBER_OF_ITERATIONS_FOR_CURVES);
			ImGui::Text("Max number of iterations: %d", MAX_NUMBER_OF_ITERATIONS_FOR_CURVES);

			if (numberOfIterations <= MIN_NUMBER_OF_ITERATIONS_FOR_CURVES)		// if the number of iterations entered is <= 0, set it to 1.
				numberOfIterations = 1;
			else if (numberOfIterations > MAX_NUMBER_OF_ITERATIONS_FOR_CURVES)	// Setting the max of iterations to "MAX_NUMBER_OF_ITERATIONS_FOR_CURVES"
				numberOfIterations = MAX_NUMBER_OF_ITERATIONS_FOR_CURVES;

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
		// For orbit camera views
		else
		{
			if (comboSelection == 2)
			{
				ImGui::Checkbox("Switch Curve", &swapCurveSelectionForViewer);
				ImGui::Checkbox("Show curve points", &showCurvePoints);
			}
			else if (comboSelection > 2)
			{
				ImGui::Checkbox("Show Wireframe", &showWireFrame);
				ImGui::Checkbox("Show mesh points & B-spline Control points", &showCurvePoints);
			}


			ImGui::SliderFloat("Aspect Ratio", &orbitViewData.aspectRatio, 0.5f, 2.f, "Current aspect ratio: %.3f");  // NEED TO MAKE AS '&'
			ImGui::Text("Camera Position: (%.3f, %.3f, %.3f)", orbitViewData.cameraPos.x, orbitViewData.cameraPos.y, orbitViewData.cameraPos.z);  // NEED TO MAKE AS '&'
			ImGui::Text("Distance from origin (look at): %.3f", orbitViewData.distanceFromLookAtPoint);
			ImGui::SliderFloat("Far Plane", &orbitViewData.farPlane, MIN_FAR_PLANE, MAX_FAR_PLANE, "Curent: %.3f");
			ImGui::SliderFloat("Near Plane", &orbitViewData.nearPlane, MIN_NEAR_PLANE, MAX_NEAR_PLANE, "Current: %.3f");
			ImGui::SliderFloat("Field Of View (Radians)", &orbitViewData.fieldOfView, glm::radians(25.f), glm::radians(180.f), "Current FOV: %.3f");
			ImGui::Text("Field Of View (Degrees): %.3f", glm::degrees(orbitViewData.fieldOfView));
			ImGui::Text("Phi (Degrees): %.3f", glm::degrees(orbitViewData.phi));
			ImGui::Text("Theta (Degrees): %.3f", glm::degrees(orbitViewData.theta));

			// Surface of revolution
			if (comboSelection == 3)
			{
				ImGui::SliderInt("Number of Slices", &windowData.numberOfSlices, MIN_NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION, MAX_NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION, "Current: %d");
				ImGui::Text("Min number of slices: %d", MIN_NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION);
				ImGui::Text("Min number of slices: %d", MAX_NUMBER_OF_SLICES_FOR_SURFACE_OF_ROTATION);
			}
			// Tensor product
			else if (comboSelection > 3)
			{
				ImGui::SliderInt("Quality", &tensorData.quality, MIN_QUALITY, MAX_QUALITY, "Current: %d");
				ImGui::SliderInt("u-degree", &tensorData.uDegree, MIN_U_DEGREE, MAX_U_DEGREE, "Current: %d");
				ImGui::SliderInt("v-degree", &tensorData.vDegree, MIN_V_DEGREE, MAX_V_DEGREE, "Current: %d");
			}

			ImGui::SliderFloat("Mouse sensitivity", &orbitViewData.mouseSensitivity, 0.5f, 200.f, "Current: %.3f");
			ImGui::SliderFloat("Mouse Scroll sensitivity", &orbitViewData.scrollSensitivity, 0.2f, 3.f, "Current: %.3f");

			ImGui::Checkbox("Reset camera", &resetCamera);
			if (resetCamera)
			{
				resetOrbitViewWindow();
				resetCamera = false;
			}
		}
	}

	glm::vec3 getColor() const
	{
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

	bool& getSwapCurveSelectionBool()
	{
		return swapCurveSelectionForViewer;
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
	tensorProductData& tensorData;
	vector<vec3>& controlPts = curveData.controlPts;
	int& numberOfIterations = curveData.numIterations;
	bool& showCurvePoints = curveData.showCurvePoints;
	bool& deleteControlPts = curveData.deleteControlPts;
	bool& resetCurveWindowBool = curveData.resetWindow;
	bool& resetCamera = orbitViewData.resetCamera;
	bool& showWireFrame = orbitViewData.showWireFrame;
	bool& enableBonus1 = windowData.enableBonus;
	bool swapCurveSelectionForViewer;					// is initialized to false (stores whether or not the user wants to swap curve generation method while in 3D viewer

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
		//orbitViewData.aspectRatio = df.aspectRatio;	// want to keep current aspect ratio
		orbitViewData.cameraPos = df.cameraPos;
		orbitViewData.distanceFromLookAtPoint = df.distanceFromLookAtPoint;
		orbitViewData.farPlane = df.farPlane;
		orbitViewData.fieldOfView = df.fieldOfView;
		//orbitViewData.mouseSensitivity = df.mouseSensitivity;	// want to keep current sensitivity
		orbitViewData.nearPlane = df.nearPlane;
		orbitViewData.phi = df.phi;
		orbitViewData.resetCamera = false;
		//orbitViewData.scrollSensitivity = df.scrollSensitivity;	// want to keep current sensitivity
		orbitViewData.theta = df.theta;
	}
};

void checkOptionChosen(cpuGeometriesData& geoms, windowControlData& windowData, optionData& optionData)
{
	curveRelatedData curveData = windowData.curveRelatedData;
	vector<vec3> cp_positions_vector = curveData.controlPts;	// User entered control points
	vector<vec3> curveGenerated;								// Curve generated
	tensorProductData tensorData = windowData.tensorProductData;
	tuple<vector<vec3>, vector<vec3>> surfaceOfRevolutionValues;	// contains meshPoints (1st element) and meshTriangles (2nd element)
	tuple<vector<vec3>, vector<vec3>> tensorSurfaceValues;			// contains meshPoints (1st element) and meshTriangles (2nd element)

	if (cp_positions_vector.size() >= 2)
	{
		switch (optionData.comboSelection)
		{
		case 0:	// Bezier curve
			//cout << "CHOSEN Bezier " << endl;	// debug

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
			//// ensure that the last point in the curve is the last control point entered
			//curveGenerated.push_back(cp_positions_vector[cp_positions_vector.size() - 1]);
			break;
		case 2:		// Orbit View of surface generated

			// If the user chose to swap the way the curve is generated
			if (optionData.swapCurveSelectionForViewer && windowData.previousOptionChosen >= 0)
			{
				// Swap to bezier
				if (windowData.previousOptionChosen == 1)
				{
					optionData.comboSelection = 0;	// temporary
					windowData.previousOptionChosen = 0;
				}
				// Swap to b-spline
				else
				{
					optionData.comboSelection = 1;	// temporary
					windowData.previousOptionChosen = 1;
				}

				checkOptionChosen(geoms, windowData, optionData);	// Call again to generate the desired curve
				optionData.comboSelection = 2;		// Set back 
				optionData.swapCurveSelectionForViewer = false;
			}
			else if (windowData.previousOptionChosen == 0)
			{
				optionData.comboSelection = 0;
				checkOptionChosen(geoms, windowData, optionData);
				optionData.comboSelection = 2;
			}
			else
			{
				optionData.comboSelection = 1;
				checkOptionChosen(geoms, windowData, optionData);
				optionData.comboSelection = 2;
			}

			return;	// return so it keeps the previously generated curve
		case 3:		// Surface of revolution
			//cout << "surface making entered" << endl; // debug

			curveGenerated = cp_positions_vector;							// Initial curve
			for (int i = 0; i < curveData.numIterations; i++)
			{
				curveGenerated = CurveGenerator::chaikin(curveGenerated);	// Create the curve so far
			}
			surfaceOfRevolutionValues = SurfaceOfRevolution::generate(curveGenerated, windowData.numberOfSlices);	// result is the mesh points and triangle mesh
			break;
		case 4:	// Tensor product 1

			// Generate the tensor product
			tensorSurfaceValues = TensorProductSurface::generate(tensorData.tensorSurfaceControlPoints1, tensorData.uDegree, tensorData.vDegree, tensorData.quality, tensorData.quality);

			// Add control points
			cp_positions_vector.clear();
			for (vector<vec3> curve : windowData.tensorProductData.tensorSurfaceControlPoints1)
			{
				for (vec3 p : curve)
					cp_positions_vector.push_back(p);
			}

			break;
		case 5: // Tensor product 2

			// Generate the tensor product
			tensorSurfaceValues = TensorProductSurface::generate(tensorData.tensorSurfaceControlPoints2, tensorData.uDegree, tensorData.vDegree, tensorData.quality, tensorData.quality);

			// Add control points
			cp_positions_vector.clear();
			for (vector<vec3> curve : windowData.tensorProductData.tensorSurfaceControlPoints2)
			{
				for (vec3 p : curve)
					cp_positions_vector.push_back(p);
			}

			break;
		default:
			//cout << "DEFUALT ENTERED OH NO!!" << endl;  // debug
			return;	// if i want to keep showing the curve previously generated curve, use 'break' if i don't
		}
	}

	// Update previous chosen option if it has changed
	if (windowData.previousOptionChosen != optionData.comboSelection)
		windowData.previousOptionChosen = optionData.comboSelection;

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

	// Update the curve stuff for curve editors & their 3D viewer
	if (optionData.comboSelection == 0 || optionData.comboSelection == 1 || optionData.comboSelection == 2)
	{
		// update the curve
		curve_point_cpu.verts = curveGenerated;
		curve_point_cpu.cols = vector<vec3>(curve_point_cpu.verts.size(), vec3(1.f, 1.0f, 0.f));		// have an option to show them
		// update curve line
		curve_line_cpu.verts = curveGenerated;
		curve_line_cpu.cols = vector<vec3>(curve_line_cpu.verts.size(), curveColour);
	}
	// Surface of Revolution
	else if (optionData.comboSelection == 3)
	{
		auto [meshPoints, meshTriangles] = surfaceOfRevolutionValues;

		// update the curve
		curve_point_cpu.verts = meshPoints;
		curve_point_cpu.cols = vector<vec3>(curve_point_cpu.verts.size(), vec3(1.f, 1.0f, 0.f));		// have an option to show them

		// update curve line
		curve_line_cpu.verts = meshTriangles;
		curve_line_cpu.cols = vector<vec3>(curve_line_cpu.verts.size(), curveColour);
	}
	// Tensor surface products
	else
	{
		auto [meshPoints, triangleMesh] = tensorSurfaceValues;
		cp_line_cpu.verts.clear();	// Don't want to show the control point lines

		curve_point_cpu.verts = meshPoints;
		curve_point_cpu.cols = vector<vec3>(curve_point_cpu.verts.size(), vec3(1.f, 1.0f, 0.f));		// have an option to show them

		curve_line_cpu.verts = triangleMesh;
		curve_line_cpu.cols = vector<vec3>(curve_line_cpu.verts.size(), curveColour);					// have an option to show them
	}
}

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

	tensorProductData tData;
	bool enableBonus1 = false;
	int numberOfSlices = 20;
	windowControlData windowData = { window, curveData, orbitViewData, tData, enableBonus1, numberOfSlices };

	// CALLBACKS
	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>(windowData);

	optionData optionData = { curve_editor_panel_renderer->getSelectedOption(), curve_editor_panel_renderer->getSwapCurveSelectionBool(), curve_editor_panel_renderer->getOptions() };

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

	glm::vec3 cp_point_colour = { 1.f,0.f,0.f };
	glm::vec3 cp_line_colour = { 0.f,1.f,0.f };

	CPU_Geometry cp_point_cpu;
	cp_point_cpu.verts = cp_positions_vector;
	cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
	GPU_Geometry cp_point_gpu;

	CPU_Geometry cp_line_cpu;
	cp_line_cpu.verts = cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
	cp_line_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
	GPU_Geometry cp_line_gpu;

	CPU_Geometry curve_point_cpu;	// stores the generated curve from Bezier or B-spline, or points that make up a surface
	GPU_Geometry curve_point_gpu;
	CPU_Geometry curve_line_cpu;	// stores the generated curve from Bezier or B-spline or triangle mesh from "surface of rotation", and "tensor product surfaces"
	GPU_Geometry curve_line_gpu;

	cpuGeometriesData geometries = { cp_point_cpu, cp_line_cpu, curve_point_cpu, curve_line_cpu };

	while (!window.shouldClose()) {
		glfwPollEvents();

		// Checks which option is chosen and computes the necessary points, and sets the CPU geoms
		checkOptionChosen(geometries, windowData, optionData);

		// set the new control points, and curve points/lines, for gpu
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
			mat4 projectionMat = glm::perspective(orbitViewData.fieldOfView, orbitViewData.aspectRatio, orbitViewData.nearPlane, orbitViewData.farPlane);

			glm::mat4 modelViewProjection = projectionMat * viewMat * model;
			shader_program_default.setMat4Transform("transformationMatrix", modelViewProjection);

		}

		// Render control points for curve editor and their 3D viewer
		if (optionData.comboSelection == 0 || optionData.comboSelection == 1 || optionData.comboSelection == 2 || curveData.showCurvePoints)
		{
			//Render control points
			cp_point_gpu.bind();
			glPointSize(CONTROL_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());
			//Render lines connecting control points
			cp_line_gpu.bind();
			//glLineWidth(10.f); //May do nothing (like it does on my computer): https://community.khronos.org/t/3-0-wide-lines-deprecated/55426
			glDrawArrays(GL_LINE_STRIP, 0, cp_line_cpu.verts.size());
		}

		// Render points generated by the curve (or mesh)
		if (curveData.showCurvePoints)
		{
			curve_point_gpu.bind();
			glPointSize(5.f);
			glDrawArrays(GL_POINTS, 0, curve_point_cpu.verts.size());
		}

		// Render curve generated for curve editor and 3D viewer
		if (optionData.comboSelection == 0 || optionData.comboSelection == 1 || optionData.comboSelection == 2)
		{
			curve_line_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, curve_line_cpu.verts.size());
		}
		// Render meshes
		else
		{
			curve_line_gpu.bind();

			if (orbitViewData.showWireFrame)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe 
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Solid surface

			glDrawArrays(GL_TRIANGLES, 0, curve_line_cpu.verts.size());	// Draw the triangle mesh
		}

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
