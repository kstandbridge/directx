#include "d2d.h"
#include "app.h"
#include "serviceLocator.h"

// math includes
#define _USE_MATH_DEFINES
#include <math.h>

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct2D::Direct2D(core::DirectXApp* dxApp) : dxApp(dxApp), matrixRotation90CW(D2D1::Matrix3x2F::Rotation(90)), 
																matrixRotation180CW(D2D1::Matrix3x2F::Rotation(180)), 
																matrixRotation270CW(D2D1::Matrix3x2F::Rotation(270)), 
																matrixRotation90CCW(D2D1::Matrix3x2F::Rotation(-90)), 
																matrixRotation180CCW(D2D1::Matrix3x2F::Rotation(-180)), 
																matrixRotation270CCW(D2D1::Matrix3x2F::Rotation(-270))
	{
		// initialize COM
		CoInitialize(NULL);

		// create the device and its context
		if (!createDevice().wasSuccessful())
			throw std::runtime_error("Crital error: Failed to initialize Direct2D!");

		// create the bitmap target to render to
		if (!createBitmapRenderTarget().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create the bitmap render target for Direct2D!");

		// create device independent resources
		if (!createDeviceIndependentResources().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create device independent Direct2D resources!");

		// create device dependent resources
		if (!createDeviceDependentResources().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create device dependent Direct2D resoures!");

		// log success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Direct2D was successfully initialized.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Initialization ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct2D::createDevice()
	{
		// create the DirectWrite factory
		if(FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory6), &writeFactory)))
			return std::runtime_error("Critical error: Unable to create the DirectWrite factory!");

		// create WIC factory
		if (FAILED(CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory2, &WICFactory)))
			return std::runtime_error("Critical error: Unable to create the WIC factory!");

		// create the Direct2D factory
		D2D1_FACTORY_OPTIONS options;
#ifndef NDEBUG
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
		if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory7), &options, &factory)))
			return std::runtime_error("Critical error: Unable to create Direct2D Factory!");
				
		// get the dxgi device
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		if(FAILED(dxApp->d3d->dev.Get()->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice)))
			return std::runtime_error("Critical error: Unable to get the DXGI device!");

		// create the Direct2D device
		if (FAILED(factory->CreateDevice(dxgiDevice.Get(), &dev)))
			return std::runtime_error("Critical error: Unable to create the Direct2D device!");

		// create its context
		if (FAILED(dev->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &devCon)))
			return std::runtime_error("Critical error: Unable to create the Direct2D device context!");

		// return success
		return {};
	}

	util::Expected<void> Direct2D::createBitmapRenderTarget()
	{
		// specify the desired bitmap properties
		D2D1_BITMAP_PROPERTIES1 bp;
		bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
		bp.dpiX = 96.0f;
		bp.dpiY = 96.0f;
		bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
		bp.colorContext = nullptr;

		// Direct2D needs the DXGI version of the back buffer
		Microsoft::WRL::ComPtr<IDXGISurface> dxgiBuffer;
		if (FAILED(dxApp->d3d->swapChain->GetBuffer(0, __uuidof(IDXGISurface), &dxgiBuffer)))
			return std::runtime_error("Critical error: Unable to retrieve the back buffer!");
		
		// create the bitmap
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
		if(FAILED(devCon->CreateBitmapFromDxgiSurface(dxgiBuffer.Get(), &bp, &targetBitmap)))
			return std::runtime_error("Critical error: Unable to create the Direct2D bitmap from the DXGI surface!");

		// set the newly created bitmap as render target
		devCon->SetTarget(targetBitmap.Get());

		// set antialiasing for text to grayscale
		devCon->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

		// return success
		return { };
	}

	// create device independent resources
	util::Expected<void> Direct2D::createDeviceIndependentResources()
	{
		// create geometries
		if (!createGeometries().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create the Direct2D Geometries!");

		// initialize the stroke styles
		if (!createStrokeStyles().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create brushes and strokes!");

		// initialize the text formats
		if (!initializeTextFormats().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create text formats!");

		// return success
		return { };
	}

	// create device depdending resources
	util::Expected<void> Direct2D::createDeviceDependentResources()
	{
		// create brushes
		if(!createBrushes().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create the Direct2D brushes!");

		// create transformations
		if (!createTransformationMatrices().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create the Direct2D transformation matrices!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createGeometries()
	{
		// create unit rectangle
		D2D1_RECT_F unitRectangle = { 0.0f, 0.0f, 1.0f, 1.0f };
		if(FAILED(factory->CreateRectangleGeometry(&unitRectangle, unitRectangleGeometry.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the unit rectangle geometry!");

		// create unit rounded rectangle
		D2D1_ROUNDED_RECT unitRoundedRectangle = { unitRectangle, 45.0f, 45.0f };
		if (FAILED(factory->CreateRoundedRectangleGeometry(&unitRoundedRectangle, unitRoundedRectangleGeometry.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the unit rounded rectangle geometry!");

		// create unit ellipse
		D2D1_ELLIPSE unitEllipse = { {0.0f, 0.0f} , 1.0f, 1.0f };
		if (FAILED(factory->CreateEllipseGeometry(&unitEllipse, unitEllipseGeometry.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the unit ellipse geometry!");

		// return success
		return { };
	}

	util::Expected<void> const Direct2D::createTransformationMatrices()
	{
		matrixTranslation = D2D1::Matrix3x2F::Translation(1, 0);	// standard translation towards the x-axis
		matrixScaling = D2D1::Matrix3x2F::Scale(2, 2);				// standard scaling (doubling)
		matrixRotation = D2D1::Matrix3x2F::Rotation(45);			// 45 degrees clockwise rotation
		matrixShearing = D2D1::Matrix3x2F::Skew(45, 0);				// 45 degrees counterclockwise in x-direction

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createBrushes()
	{
		// create standard brushes
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &yellowBrush)))
			return std::runtime_error("Critical error: Unable to create the yellow brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush)))
			return std::runtime_error("Critical error: Unable to create the black brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush)))
			return std::runtime_error("Critical error: Unable to create the white brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &redBrush)))
			return std::runtime_error("Critical error: Unable to create the red brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &blueBrush)))
			return std::runtime_error("Critical error: Unable to create the blue brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Brown), &brownBrush)))
			return std::runtime_error("Critical error: Unable to create the brown brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &greenBrush)))
			return std::runtime_error("Critical error: Unable to create the green brush!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createStrokeStyles()
	{
		// create stroke styles - FPS heavy
		D2D1_STROKE_STYLE_PROPERTIES1 strokeProperties = {};
		strokeProperties.lineJoin = D2D1_LINE_JOIN_ROUND;
		strokeProperties.dashStyle = D2D1_DASH_STYLE_DASH;
		strokeProperties.dashCap = D2D1_CAP_STYLE_SQUARE;
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, dashedStroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the dashed stroke style!");

		strokeProperties.dashStyle = D2D1_DASH_STYLE_DOT;
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, dottedStroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the dashed stroke style!");

		strokeProperties.dashStyle = D2D1_DASH_STYLE_DASH_DOT;
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, dashDotStroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the dashed stroke style!");

		strokeProperties.dashStyle = D2D1_DASH_STYLE_DASH_DOT_DOT;
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, dashDotDotStroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the dashed stroke style!");

		strokeProperties.dashStyle = D2D1_DASH_STYLE_SOLID;
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, solidStroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the dashed stroke style!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::initializeTextFormats()
	{
		// set up text formats

		// FPS text
		if(FAILED(writeFactory->CreateTextFormat(L"Lucida Console", NULL, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-GB", (IDWriteTextFormat **)textFormatFPS.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create text format for FPS information!");
		if(FAILED(textFormatFPS->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
			return std::runtime_error("Critical error: Unable to set text alignment!");
		if(FAILED(textFormatFPS->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
			return std::runtime_error("Critical error: Unable to set paragraph alignment!");

		// return success
		return { };
	}


	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Printing Functions //////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Direct2D::printFPS(const Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush)
	{
		if (dxApp->showFPS && textLayoutFPS)
		{
			// draw the text
			devCon->DrawTextLayout(D2D1::Point2F(2.5f, 5.0f), textLayoutFPS.Get(), brush.Get());
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Shut Down ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct2D::~Direct2D()
	{
		// release the WICFactory
		WICFactory.ReleaseAndGetAddressOf();
		
		// end COM
		CoUninitialize();

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Direct2D was shut down successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Helper Functions ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<D2D1_POINT_2F> Direct2D::computeCoordinatesOnEllipse(D2D1_ELLIPSE *const ellipse, const float angle)
	{
		// the x and y-coordinates can be computed by circular functions
		return D2D1_POINT_2F({ ellipse->point.x + ellipse->radiusX * cos(angle*(float)M_PI/180) , ellipse->point.y + ellipse->radiusY * sin(angle*(float)M_PI/180) });
	}

	util::Expected<void> Direct2D::createArcPathGeometry(Microsoft::WRL::ComPtr<ID2D1PathGeometry>* arc, D2D1_POINT_2F startPoint, D2D1_POINT_2F endPoint, float radiusX, float radiusY, float rotationAngle, D2D1_SWEEP_DIRECTION sweepDirection, D2D1_ARC_SIZE arcSize)
	{
		// fill the arc segment structure
		D2D1_ARC_SEGMENT arcStructure = 
		{
			endPoint,
			{radiusX, radiusY},
			rotationAngle,
			sweepDirection,
			arcSize,
		};

		// create the path geometry
		if(FAILED(factory->CreatePathGeometry(arc->ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create path geometry for the arc segment!");

		// open and fill the path geometry
		Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
		if(FAILED(arc->Get()->Open(sink.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the sink for the arc segment!");

		sink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);
		sink->AddArc(arcStructure);
		sink->EndFigure(D2D1_FIGURE_END_OPEN);

		if(FAILED(sink->Close()))
			return std::runtime_error("Critical error: Unable to close the sink of the arc segment!");

		// return success
		return { };
	}
}


