#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		24/06/2018 - Lenningen - Luxembourg
*
* Desc:		mathematical vectors
*
* History:
*
* ToDo:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// CLASSES //////////////////////////////////////////////////////////////////////////////

namespace maths
{
	struct Vector2F
	{
		float x, y;
		float squareLength;
		float length;

		// constructor and destructor
		Vector2F() : x(0.0f), y(0.0f) {};
		Vector2F(float x, float y) : x(x), y(y) { squareLength = getSquareLength(); };
		~Vector2F() {};

		// get square length
		float getSquareLength();

		// get length
		float getLength();

		// normalize vector
		void normalize(float l = -1.0f);

		// overload operators
		Vector2F& operator*=(const float);
	};
}