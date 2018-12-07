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
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and COM
#include <wrl/client.h>

// shared pointers
#include <shared_mutex>

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
	class Sprite;
	class AnimationData;

	class Direct2D
	{
	private:
		core::DirectXApp* dxApp;								// pointer to the main application class

		Microsoft::WRL::ComPtr<IDWriteFactory6> writeFactory;	// pointer to the DirectWrite factory
		Microsoft::WRL::ComPtr<IWICImagingFactory2> WICFactory;	// Windows Imaging Component factory
		Microsoft::WRL::ComPtr<ID2D1Factory7> factory;			// pointer to the Direct2D factory
		Microsoft::WRL::ComPtr<ID2D1Device6> dev;				// pointer to the Direct2D device
		Microsoft::WRL::ComPtr<ID2D1DeviceContext6> devCon;		// pointer to the device context
		
		// standard colour brushes
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> yellowBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> redBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blueBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brownBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> greenBrush;

		// standard strokes - FPS heavy!
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle1> dashedStroke;
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle1> dottedStroke;
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle1> dashDotStroke;
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle1> dashDotDotStroke;
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle1> solidStroke;

		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> textFormatFPS;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> textLayoutFPS;

		// simple unit geometries
		Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> unitRectangleGeometry;				// a unit rectangle as a geometric object
		Microsoft::WRL::ComPtr<ID2D1RoundedRectangleGeometry> unitRoundedRectangleGeometry;	// a unit rounded rectangle as a geometric object
		Microsoft::WRL::ComPtr<ID2D1EllipseGeometry> unitEllipseGeometry;					// an unit ellipse (circle with radius 1) as a geometric object

		// standard transformations
		D2D1::Matrix3x2F matrixTranslation;						// translation matrix
		D2D1::Matrix3x2F matrixRotation;						// rotation matrix
		D2D1::Matrix3x2F matrixScaling;							// scaling matrix
		D2D1::Matrix3x2F matrixShearing;						// shearing matrix

		// useful fixed rotations
		const D2D1::Matrix3x2F matrixRotation90CW;				// 90 degrees clockwise rotation 
		const D2D1::Matrix3x2F matrixRotation180CW;				// 180 degrees clockwise rotation
		const D2D1::Matrix3x2F matrixRotation270CW;				// 270 degrees clockwise rotation
		const D2D1::Matrix3x2F matrixRotation90CCW;				// 90 degrees counterclockwise rotation
		const D2D1::Matrix3x2F matrixRotation180CCW;			// 180 degrees counterclockwise rotation
		const D2D1::Matrix3x2F matrixRotation270CCW;			// 270 degrees counterclockwise rotation

		// create devices and resoures
		util::Expected<void> createDevice();					// creates the device and its context
		util::Expected<void> createBitmapRenderTarget();		// creates the bitmap render target, set to be the same as the backbuffer already in use for Direct3D
		util::Expected<void> createDeviceIndependentResources();// creates device independent resources
		util::Expected<void> createDeviceDependentResources();	// creates device dependent resources
		util::Expected<void> createBrushes();					// initializes different brushes
		util::Expected<void> createStrokeStyles();				// initializes different stroke styles
		util::Expected<void> initializeTextFormats();			// initializes the different formats, for now, only a format to print FPS information will be created
		util::Expected<void> createGeometries();				// creates geometrical objects
		util::Expected<void> const createTransformationMatrices();	// creates standard transformation matrices

		// helper functions
		util::Expected<D2D1_POINT_2F> computeCoordinatesOnEllipse(D2D1_ELLIPSE *const ellipse, const float angle);	// computes the x and y-coordinates of a point on an ellipse
		util::Expected<void> createArcPathGeometry(Microsoft::WRL::ComPtr<ID2D1PathGeometry>* arc, D2D1_POINT_2F startPoint, D2D1_POINT_2F endPoint, float radiusX, float radiusY, float rotationAngle, D2D1_SWEEP_DIRECTION sweepDirection, D2D1_ARC_SIZE arcRadia);	// creates a path geometry consisting of an arc between the start and end points with given radia, rotation angle, sweep direction and size

	public:
		// constructors
		Direct2D(core::DirectXApp* dxApp);
		~Direct2D();
		
		void printFPS(const Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush);	// prints fps information to the screen in the desired colour specified by the brush

		friend class core::DirectXApp;
		friend class Direct3D;
		friend class DirectXGame;
		friend class Sprite;
		friend class AnimationData;
	};
}