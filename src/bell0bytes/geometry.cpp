#include "geometry.h"

// math includes
#define _USE_MATH_DEFINES
#include <math.h>

namespace maths
{
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Helper Functions ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Geometry::computeCoordinatesOnEllipse(const D2D1_ELLIPSE& ellipse, const float angle, D2D1_POINT_2F& point)
	{
		// the x and y-coordinates can be computed by circular functions
		point = { ellipse.point.x + ellipse.radiusX * cosf(angle*(float)M_PI / 180) , ellipse.point.y + ellipse.radiusY * sinf(angle*(float)M_PI / 180) };
	}
}