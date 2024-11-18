
#include <iostream>

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace glm;

class CurveGenerator
{
public:
	static vector<vec3> bezierCurve(const vector<vec3> controlPts, int degree, float u);

private:
	static vec3 deCasteljauAlgo(const vector<vec3> controlPts, int degree, float u);
};
