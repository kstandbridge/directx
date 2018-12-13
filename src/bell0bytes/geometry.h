#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		27/06/2018 - Lenningen - Luxembourg
*
* Desc:		geometry
*
* History:
*
* ToDo:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////
#include <d2d1_3.h>

// CLASSES //////////////////////////////////////////////////////////////////////////////

namespace util
{
	template<typename T>
	class Expected;
}

namespace maths
{
	class Geometry
	{
	private:

	public:
		Geometry() {};
		~Geometry() {};

		// helper functions
		static void computeCoordinatesOnEllipse(const D2D1_ELLIPSE& ellipse, float angle, D2D1_POINT_2F& point);	// computes the x and y-coordinates of a point on an ellipse given by the angle
	};
}
