#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		27/06/2018 - Lenningen - Luxembourg
*
* Desc:		2D graphics components
*
* Hist:		- 28/06/2018: the "compute point on ellipse" method is now in a maths class
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////
#include <d2d1_3.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

namespace util
{
	template<typename T>
	class Expected;
}

namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class Direct2D;
	class Direct3D;

	class GraphicsComponent2D
	{
	private:
		Direct2D* d2d;

		// useful fixed rotations
		const D2D1::Matrix3x2F matrixRotation90CW;				// 90 degrees clockwise rotation 
		const D2D1::Matrix3x2F matrixRotation180CW;				// 180 degrees clockwise rotation
		const D2D1::Matrix3x2F matrixRotation270CW;				// 270 degrees clockwise rotation
		const D2D1::Matrix3x2F matrixRotation90CCW;				// 90 degrees counterclockwise rotation
		const D2D1::Matrix3x2F matrixRotation180CCW;			// 180 degrees counterclockwise rotation
		const D2D1::Matrix3x2F matrixRotation270CCW;			// 270 degrees counterclockwise rotation


	public:
		// constructor and destructor
		GraphicsComponent2D(const core::DirectXApp& dxApp, const Direct3D& d3d);
		~GraphicsComponent2D();

		// begin and end drawing
		void beginDraw() const;
		util::Expected<void> endDraw() const;

		// draw and fill rectangles
		void fillRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float opacity = 1.0f, ID2D1Brush* const brush = NULL) const;
		void fillRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float opacity = 1.0f, ID2D1Brush* const brush = NULL) const;
		void drawRectangle(const float ulX, const float ulY, const float lrX, const float lrY, ID2D1Brush* const brush = NULL, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;
		void drawRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, ID2D1Brush* const brush = NULL, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;

		// draw and fill rounded rectangles
		void fillRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, const float opacity = 1.0f, ID2D1Brush* const brush = NULL) const;
		void fillRoundedRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float radiusX, const float radiusY, const float opacity = 1.0f, ID2D1Brush* const brush = NULL) const;
		void drawRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, ID2D1Brush* const brush = NULL, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;
		void drawRoundedRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float radiusX, const float radiusY, ID2D1Brush* const brush = NULL, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;

		// draw and fill ellipses
		void fillEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, const float opacity = 1.0f, ID2D1Brush* const brush = NULL) const;
		void drawEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;

		// transformations
		void setTransformation90CW() const;
		void setTransformation180CW() const;
		void setTransformation270CW() const;
		void setTransformation90CCW() const;
		void setTransformation180CCW() const;
		void setTransformation270CCW() const;
		void setTransformation(const D2D1::Matrix3x2F& transMatrix) const;
		void resetTransformation() const;
		void reflectX(float x, float y) const;
		void reflectY(float x, float y) const;

		// get the Direct2D class
		Direct2D& getD2D() const;

		friend class core::DirectXApp;
		friend class GraphicsComponent;
		friend class GraphicsComponent3D;
	};
}