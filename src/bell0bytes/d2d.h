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
*			- 28/06/2018: sliced the DirectWrite method out of the class and into a seperate DirectWrite component
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

// CLASSES //////////////////////////////////////////////////////////////////////////////

namespace core
{
	class DirectXApp;
}

namespace util
{
	template<typename T>
	class Expected;
}

namespace graphics
{
	// forward declarations
	class Direct3D;
	
	class Direct2D
	{
	private:
		const core::DirectXApp& dxApp;							// pointer to the main application class

		Microsoft::WRL::ComPtr<IDWriteFactory6> writeFactory;	// pointer to the DirectWrite factory
		Microsoft::WRL::ComPtr<IWICImagingFactory> WICFactory;	// Windows Imaging Component factory
		Microsoft::WRL::ComPtr<ID2D1Factory7> factory;			// pointer to the Direct2D factory
		Microsoft::WRL::ComPtr<ID2D1Device6> dev;				// pointer to the Direct2D device
		Microsoft::WRL::ComPtr<ID2D1DeviceContext6> devCon;		// pointer to the device context
		
		// standard black brush
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;

		// create devices and resoures
		util::Expected<void> createDevice(const Direct3D& d3d);					// creates the device and its context
		util::Expected<void> createBitmapRenderTarget(const Direct3D& d3d);		// creates the bitmap render target, set to be the same as the backbuffer already in use for Direct3D
		util::Expected<void> createDeviceIndependentResources();				// creates device independent resources
		util::Expected<void> createDeviceDependentResources();					// creates device dependent resources
		util::Expected<void> createBitmapFromWICBitmap(LPCWSTR imageFile, Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bitmap) const;		// loads an image from the harddrive and stores it as a bitmap
		
		// private getters for the DWrite component
		IDWriteFactory6& getWriteFactory() const;
		ID2D1DeviceContext6& getDeviceContext() const;
		ID2D1SolidColorBrush& getBlackBrush() const;
	
	public:
		// constructors
		Direct2D(const core::DirectXApp& dxApp, const Direct3D& d3d);
		~Direct2D();

		// brushes and strokes
		util::Expected<void> createSolidColourBrush(const D2D1::ColorF& colour, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>& brush) const;			// creates a solid colour brush
		util::Expected<void> createLinearGradientBrush(const float startX, const float startY, const float endX, const float endY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>& linearGradientBrush) const;
		util::Expected<void> createRadialGradientBrush(const float centreX, const float centreY, const float offsetX, const float offsetY, const float radiusX, const float radiusY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1RadialGradientBrush>& radialGradientBrush) const;
		util::Expected<void> createStrokeStyle(D2D1_STROKE_STYLE_PROPERTIES1& strokeProperties, Microsoft::WRL::ComPtr<ID2D1StrokeStyle1>& stroke) const;// creates a stroke
		
		// draw
		void beginDraw() const;
		util::Expected<void> endDraw() const;
				
		// get resolution
		const unsigned int getCurrentWidth() const;
		const unsigned int getCurrentHeight() const;

		friend class Direct3D;
		friend class Sprite;
		friend class AnimationData;
		friend class GraphicsComponent;
		friend class GraphicsComponent2D;
	};
}