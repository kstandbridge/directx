#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		05/08/2017 - Dortmund - Germany
*
* Desc:		Main class to use the Direct2D and DirectWrite components of DirectX.
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and COM
#include <wrl/client.h>

// shared pointers
#include <shared_mutex>

// DirectX includes
#include <d2d1_2.h>
#include <dwrite_2.h>

#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")

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
	class Direct2D
	{
	private:
		core::DirectXApp* dxApp;								// pointer to the main application class

		Microsoft::WRL::ComPtr<IDWriteFactory2> writeFactory;	// pointer to the DirectWrite factory
		Microsoft::WRL::ComPtr<ID2D1Factory2> factory;			// pointer to the Direct2D factory
		Microsoft::WRL::ComPtr<ID2D1Device1> dev;				// pointer to the Direct2D device
		Microsoft::WRL::ComPtr<ID2D1DeviceContext1> devCon;		// pointer to the device context

		D2D1_RECT_F fpsRectangle;								// rectangle around the fps information

		util::Expected<void> createDevice();					// creates the device and its context
		util::Expected<void> createBitmapRenderTarget();		// creates the bitmap render target, set to be the same as the backbuffer already in use for Direct3D
		util::Expected<void> createBrushes();					// initializes different brushes and strokes
		util::Expected<void> initializeTextFormats();			// initializes the different formats, for now, only a format to print FPS information will be created

		util::Expected<D2D1_POINT_2F> computeCoordinatesOnEllipse(D2D1_ELLIPSE *const ellipse, const float angle);	// computes the x and y-coordinates of a point on an ellipse
	public:
		// constructors
		Direct2D(core::DirectXApp* dxApp);
		~Direct2D();

		// brushes
		
		// solid colour brushes
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> yellowBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> redBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blueBrush;

		// linear gradient brush
		Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> linearGradientBrush;

		// radial gradient brush
		Microsoft::WRL::ComPtr<ID2D1RadialGradientBrush> radialGradientBrush;

		// strokes
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle> dashedStroke;

		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormatFPS;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayoutFPS;

		util::Expected<void> printFPS();						// prints fps information to the screen

		friend class core::DirectXApp;
		friend class Direct3D;
		friend class DirectXGame;
	};
}