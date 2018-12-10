#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		05/08/2017 - Dortmund - Germany
*
* Desc:		Main class to use the Direct2D and DirectWrite components of DirectX.
*
* History:	- 24/05/2018: updated DirectWrite to the Windows 10 Creators Update
*			- 24/05/2018: updated Direct2D to the Windows 10 Creators Update
*			- 27/05/2018: added support for the Windows Imaging Content
*			- 31/05/2018: added a method to create new font formats and text layouts
*			- 04/06/2018: the DirectXApp and DirectXGame classes are no longer friend classes
*			- 04/06/2018: various functions to create brushes and strokeStyles have been added
*			- 04/06/2018: various functions to draw primitives have been added
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and COM
#include <wrl/client.h>

// DirectX includes
#include <d2d1_3.h>
#include <dwrite_3.h>

#include <wincodec.h>	// Windows Imaging Component
#include <WTypes.h>

#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "Windowscodecs.lib")

// bell0bytes utilities
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
class DirectXGame;

namespace core
{
	class DirectXApp;
}

namespace graphics
{
	// forward declarations
	class Direct3D;
	class Sprite;
	class AnimationData;

	class Direct2D
	{
	private:
		const core::DirectXApp* const dxApp;					// pointer to the main application class

		Microsoft::WRL::ComPtr<IDWriteFactory6> writeFactory;	// pointer to the DirectWrite factory
		Microsoft::WRL::ComPtr<IWICImagingFactory> WICFactory;	// Windows Imaging Component factory
		Microsoft::WRL::ComPtr<ID2D1Factory7> factory;			// pointer to the Direct2D factory
		Microsoft::WRL::ComPtr<ID2D1Device6> dev;				// pointer to the Direct2D device
		Microsoft::WRL::ComPtr<ID2D1DeviceContext6> devCon;		// pointer to the device context
		
		// standard black brush
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;

		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> textFormatFPS;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> textLayoutFPS;

		// useful fixed rotations
		const D2D1::Matrix3x2F matrixRotation90CW;				// 90 degrees clockwise rotation 
		const D2D1::Matrix3x2F matrixRotation180CW;				// 180 degrees clockwise rotation
		const D2D1::Matrix3x2F matrixRotation270CW;				// 270 degrees clockwise rotation
		const D2D1::Matrix3x2F matrixRotation90CCW;				// 90 degrees counterclockwise rotation
		const D2D1::Matrix3x2F matrixRotation180CCW;			// 180 degrees counterclockwise rotation
		const D2D1::Matrix3x2F matrixRotation270CCW;			// 270 degrees counterclockwise rotation

		// create devices and resoures
		util::Expected<void> createDevice(Direct3D* const d3d);	// creates the device and its context
		util::Expected<void> createBitmapRenderTarget(Direct3D* const d3d);		// creates the bitmap render target, set to be the same as the backbuffer already in use for Direct3D
		util::Expected<void> createDeviceIndependentResources();// creates device independent resources
		util::Expected<void> createDeviceDependentResources();	// creates device dependent resources
		util::Expected<void> createBitmapFromWICBitmap(LPCWSTR imageFile, Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bitmap);		// loads an image from the harddrive and stores it as a bitmap
	public:
		// constructors
		Direct2D(const core::DirectXApp* const dxApp, Direct3D* const d3d);
		~Direct2D();

		// print fps
		void printFPS(ID2D1SolidColorBrush* const brush) const;	// prints fps information to the screen in the desired colour specified by the brush
		void printFPS() const;

		// brushes and strokes
		util::Expected<void> createSolidColourBrush(const D2D1::ColorF& colour, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>& brush);			// creates a solid colour brush
		util::Expected<void> createLinearGradientBrush(const float startX, const float startY, const float endX, const float endY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>& linearGradientBrush);
		util::Expected<void> createRadialGradientBrush(const float centreX, const float centreY, const float offsetX, const float offsetY, const float radiusX, const float radiusY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1RadialGradientBrush>& radialGradientBrush);
		util::Expected<void> createStrokeStyle(D2D1_STROKE_STYLE_PROPERTIES1& strokeProperties, Microsoft::WRL::ComPtr<ID2D1StrokeStyle1>& stroke);// creates a stroke
		
		// text formats and layouts
		util::Expected<void> createTextFormat(LPCWSTR fontFamilyName, const DWRITE_FONT_WEIGHT fontWeight, const DWRITE_FONT_STYLE fontStyle, const DWRITE_FONT_STRETCH fontStretch, const float fontSize, LPCWSTR localeName, const DWRITE_TEXT_ALIGNMENT textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat, IDWriteFontCollection2* const fontCollection = NULL);	// creates a text format with the specifies properties and stores the result in the textFormat parameter
		util::Expected<void> createTextFormat(LPCWSTR fontFamilyName, const float fontSize, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat);																																																																							// creates a standard text format
		util::Expected<void> createTextLayout(const std::wostringstream* const string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout);
		util::Expected<void> createTextLayoutFPS(const std::wostringstream* const stringFPS, const float width, const float height);

		// draw
		void beginDraw() const;
		util::Expected<void> endDraw() const;
		void fillRectangle(const float ulX, const float ulY, const float lrX, const float lrY, ID2D1Brush* const brush) const;
		void drawRectangle(const float ulX, const float ulY, const float lrX, const float lrY, ID2D1Brush* const brush, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;
		void fillRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, ID2D1Brush* const brush) const;
		void drawRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width = 1.0f, ID2D1StrokeStyle1* const strokeStyle = NULL) const;
		void fillEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, ID2D1Brush* const brush) const;
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
		
		// helper functions
		util::Expected<D2D1_POINT_2F> computeCoordinatesOnEllipse(const D2D1_ELLIPSE *const ellipse, float angle);	// computes the x and y-coordinates of a point on an ellipse
		
		friend class Direct3D;
		friend class Sprite;
		friend class AnimationData;
	};
}