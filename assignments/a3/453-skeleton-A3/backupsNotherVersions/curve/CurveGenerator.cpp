#include <iostream>

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace glm;

class CurveGenerator
{
	/// <summary>
	/// Ensure that the size of 'controlPts' is 1 more than 'degree'.
	/// </summary>
	/// <param name="controlPts"></param>
	/// <param name="degree"></param>
	/// <param name="u"></param>
	/// <returns></returns>
	static vector<vec3> bezierCurve(const vector<vec3> controlPts, int degree, float u)
	{
		vector<vec3> curveSoFar;

		if (degree - controlPts.size() > 1)	// checking if the number of control points is 1 more than the degree
		{
			std::cout << "\nError occured in deCasteljauAlgo: (invalid number of control points)" << std::endl;
			std::cout << "num controlPoints: " <<controlPts.size() << " degree: " << degree << std::endl;
			return curveSoFar;
		}

		// Starts generation with the original control points and after each 'genBezierCurve' call
		// this 'vector' gets smaller as the 'columns get smaller (contains the points at a column).
		curveSoFar = vector<vec3>(controlPts.size() - 1);

		for (int i = 1; i < controlPts.size(); i++)			// calculate entire "column"
		{
			vector<vec3> controlPtsToUse(2);
			controlPtsToUse[0] = controlPts[i - 1];
			controlPtsToUse[1] = controlPts[i];

			curveSoFar[i - 1] = deCasteljauAlgo(controlPtsToUse, degree, u);
		}

		if (controlPts.size() > 2)	// If the size of 'controlPts' is less than 2 then we have reached the final column (calculated "Q<degree>(u)")
			curveSoFar = bezierCurve(curveSoFar, degree, u);	// Call itself again to generate next column

		return curveSoFar;
	};

private:
	static vec3 deCasteljauAlgo(const vector<vec3> controlPts, int degree, float u)
	{
		vec3 pt = vec3();	// the point at a column


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
