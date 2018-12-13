// include the header
#include "graphicsComponent2D.h"

// bell0bytes graphics
#include "d2d.h"
#include "d3d.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructors ///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GraphicsComponent2D::GraphicsComponent2D(const core::DirectXApp& dxApp, const Direct3D& d3d) : d2d(NULL), matrixRotation90CW(D2D1::Matrix3x2F::Rotation(90)),
		matrixRotation180CW(D2D1::Matrix3x2F::Rotation(180)),
		matrixRotation270CW(D2D1::Matrix3x2F::Rotation(270)),
		matrixRotation90CCW(D2D1::Matrix3x2F::Rotation(-90)),
		matrixRotation180CCW(D2D1::Matrix3x2F::Rotation(-180)),
		matrixRotation270CCW(D2D1::Matrix3x2F::Rotation(-270))
	{
		// initialize Direct2D
		try { d2d = new graphics::Direct2D(dxApp, d3d); }
		catch (std::runtime_error& e)
		{ throw e; }
	}
	GraphicsComponent2D::~GraphicsComponent2D()
	{
		if (d2d)
			delete d2d;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Drawing ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void GraphicsComponent2D::beginDraw() const
	{
		d2d->beginDraw();
	}
	util::Expected<void> GraphicsComponent2D::endDraw() const
	{
		return d2d->endDraw();
	}

	// draw and fill rectangles
	void GraphicsComponent2D::fillRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if (brush)
		{
			brush->SetOpacity(opacity);
			d2d->devCon->FillRectangle(&rect, brush);
		}
		else
		{
			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(opacity);

			d2d->devCon->FillRectangle(&rect, d2d->blackBrush.Get());

			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(1.0f);
		}
	}
	void GraphicsComponent2D::fillRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { upperLeft.x , upperLeft.y , lowerRight.x , lowerRight.y };
		if (brush)
		{
			brush->SetOpacity(opacity);
			d2d->devCon->FillRectangle(&rect, brush);
		}
		else
		{
			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(opacity);

			d2d->devCon->FillRectangle(&rect, d2d->blackBrush.Get());

			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(1.0f);
		}
	}
	void GraphicsComponent2D::drawRectangle(const float ulX, const float ulY, const float lrX, const float lrY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if (brush)
			d2d->devCon->DrawRectangle(&rect, brush, width, strokeStyle);
		else
			d2d->devCon->DrawRectangle(&rect, d2d->blackBrush.Get(), width, strokeStyle);
	}
	void GraphicsComponent2D::drawRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y };
		if (brush)
			d2d->devCon->DrawRectangle(&rect, brush, width, strokeStyle);
		else
			d2d->devCon->DrawRectangle(&rect, d2d->blackBrush.Get(), width, strokeStyle);
	}

	// fill and draw rounded rectangles
	void GraphicsComponent2D::fillRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if (brush)
		{
			brush->SetOpacity(opacity);
			d2d->devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush);
		}
		else
		{
			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(opacity);

			d2d->devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), d2d->blackBrush.Get());

			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(1.0f);
		}
	}
	void GraphicsComponent2D::fillRoundedRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float radiusX, const float radiusY, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y };

		if (brush)
		{
			brush->SetOpacity(opacity);
			d2d->devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush);
		}
		else
		{
			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(opacity);

			d2d->devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), d2d->blackBrush.Get());

			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(opacity);
		}
	}
	void GraphicsComponent2D::drawRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if (brush)
			d2d->devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush, width, strokeStyle);
		else
			d2d->devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), d2d->blackBrush.Get(), width, strokeStyle);
	}
	void GraphicsComponent2D::drawRoundedRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y };
		if (brush)
			d2d->devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush, width, strokeStyle);
		else
			d2d->devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), d2d->blackBrush.Get(), width, strokeStyle);
	}

	// fill and draw ellipses
	void GraphicsComponent2D::fillEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, const float opacity, ID2D1Brush* const brush) const
	{
		if (brush)
		{
			brush->SetOpacity(opacity);
			d2d->devCon->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), brush);
		}
		else
		{
			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(opacity);
			d2d->devCon->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), d2d->blackBrush.Get());

			if (opacity != 1.0f)
				d2d->blackBrush->SetOpacity(1.0f);
		}
	}
	void GraphicsComponent2D::drawEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		if (brush)
			d2d->devCon->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), brush, width, strokeStyle);
		else
			d2d->devCon->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), d2d->blackBrush.Get(), width, strokeStyle);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Transformations ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void GraphicsComponent2D::reflectX(float x, float y) const
	{
		D2D1::Matrix3x2F trans1 = D2D1::Matrix3x2F::Translation(-x, -y);
		D2D1::Matrix3x2F trans2 = D2D1::Matrix3x2F::Translation(x, y);

		// rotate
		D2D1::Matrix3x2F m;
		m._11 = 1; m._12 = 0;
		m._21 = 0; m._22 = -1;
		m._31 = 0; m._32 = 1;

		D2D1::Matrix3x2F transformation = trans1 * m * trans2;
		setTransformation(transformation);
	}
	void GraphicsComponent2D::reflectY(float x, float y) const
	{
		D2D1::Matrix3x2F trans1 = D2D1::Matrix3x2F::Translation(-x, -y);
		D2D1::Matrix3x2F trans2 = D2D1::Matrix3x2F::Translation(x, y);

		// rotate
		D2D1::Matrix3x2F m;
		m._11 = -1; m._12 = 0;
		m._21 = 0; m._22 = 1;
		m._31 = 0; m._32 = 1;

		D2D1::Matrix3x2F transformation = trans1 * m * trans2;
		setTransformation(transformation);
	}

	void GraphicsComponent2D::setTransformation(const D2D1::Matrix3x2F& transMatrix) const
	{
		d2d->devCon->SetTransform(transMatrix);
	}
	void GraphicsComponent2D::resetTransformation() const
	{
		d2d->devCon->SetTransform(D2D1::Matrix3x2F::Identity());
	}
	void GraphicsComponent2D::setTransformation90CW() const
	{
		d2d->devCon->SetTransform(matrixRotation90CW);
	}
	void GraphicsComponent2D::setTransformation180CW() const
	{
		d2d->devCon->SetTransform(matrixRotation180CW);
	}
	void GraphicsComponent2D::setTransformation270CW() const
	{
		d2d->devCon->SetTransform(matrixRotation270CW);
	}
	void GraphicsComponent2D::setTransformation90CCW() const
	{
		d2d->devCon->SetTransform(matrixRotation90CCW);
	}
	void GraphicsComponent2D::setTransformation180CCW() const
	{
		d2d->devCon->SetTransform(matrixRotation180CCW);
	}
	void GraphicsComponent2D::setTransformation270CCW() const
	{
		d2d->devCon->SetTransform(matrixRotation270CCW);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Getter /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct2D& GraphicsComponent2D::getD2D() const
	{
		return *d2d;
	}
}