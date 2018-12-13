// header file
#include "d2d.h"

// bell0bytes core
#include "app.h"

// bell0bytes util
#include "serviceLocator.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "d3d.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct2D::Direct2D(const core::DirectXApp& dxApp, const Direct3D& d3d) : dxApp(dxApp)
	{
		// initialize COM
		CoInitialize(NULL);

		// create the device and its context
		if (!createDevice(d3d).wasSuccessful())
			throw std::runtime_error("Crital error: Failed to initialize Direct2D!");

		// create the bitmap target to render to
		if (!createBitmapRenderTarget(d3d).wasSuccessful())
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
	util::Expected<void> Direct2D::createDevice(const Direct3D& d3d)
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
		if(FAILED(d3d.dev.Get()->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice)))
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

	util::Expected<void> Direct2D::createBitmapRenderTarget(const Direct3D& d3d)
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
		if (FAILED(d3d.swapChain->GetBuffer(0, __uuidof(IDXGISurface), &dxgiBuffer)))
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
		// return success
		return { };
	}

	// create device depdendent resources
	util::Expected<void> Direct2D::createDeviceDependentResources()
	{
		// create the black brush
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush)))
			return std::runtime_error("Critical error: Unable to create the black brush!");

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Brushes and Strokes //////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct2D::createSolidColourBrush(const D2D1::ColorF& colour, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>& brush) const
	{
		if (FAILED(devCon->CreateSolidColorBrush(colour, brush.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create brush!");

		// return success
		return { };
	}
	util::Expected<void> Direct2D::createLinearGradientBrush(const float startX, const float startY, const float endX, const float endY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>& linearGradientBrush) const
	{
		D2D1_POINT_2F startPoint = D2D1::Point2F(startX, startY);
		D2D1_POINT_2F endPoint = D2D1::Point2F(endX, endY);
		if (FAILED(devCon->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(startPoint, endPoint), &stopCollection, linearGradientBrush.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the linear gradient brush!");

		// return success
		return { };
	}
	util::Expected<void> Direct2D::createRadialGradientBrush(const float centreX, const float centreY, const float offsetX, const float offsetY, const float radiusX, const float radiusY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1RadialGradientBrush>& radialGradientBrush) const
	{
		D2D1_POINT_2F centre = D2D1::Point2F(centreX, centreY);
		D2D1_POINT_2F offset = D2D1::Point2F(offsetX, offsetY);
		if (FAILED(devCon->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(centre, offset, radiusX, radiusY), &stopCollection, radialGradientBrush.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the radial gradient brush!");

		// return success
		return {};
	}
	util::Expected<void> Direct2D::createStrokeStyle(D2D1_STROKE_STYLE_PROPERTIES1& strokeProperties, Microsoft::WRL::ComPtr<ID2D1StrokeStyle1>& stroke) const
	{
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, stroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create stroke style!");

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Bitmaps /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct2D::createBitmapFromWICBitmap(LPCWSTR imageFile, Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bitmap) const
	{
		// create decoder
		Microsoft::WRL::ComPtr<IWICBitmapDecoder> bitmapDecoder;
		if (FAILED(WICFactory->CreateDecoderFromFilename(imageFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, bitmapDecoder.ReleaseAndGetAddressOf())))
			return std::runtime_error("Failed to create decoder from filename!");

		// get the correct frame
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
		if (FAILED(bitmapDecoder->GetFrame(0, frame.ReleaseAndGetAddressOf())))
			return std::runtime_error("Failed to retrieve frame from bitmap!");

		// create the format converter
		Microsoft::WRL::ComPtr<IWICFormatConverter> image;
		if (FAILED(WICFactory->CreateFormatConverter(image.ReleaseAndGetAddressOf())))
			return std::runtime_error("Failed to create the format converter!");

		// initialize the WIC image
		if (FAILED(image->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom)))
			return std::runtime_error("Failed to initialize the WIC image!");

		// create the bitmap
		if (FAILED(devCon->CreateBitmapFromWicBitmap(image.Get(), bitmap.ReleaseAndGetAddressOf())))
			return std::runtime_error("Failed to create the bitmap image!");

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Drawing /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Direct2D::beginDraw() const
	{
		devCon->BeginDraw();
	}
	util::Expected<void> Direct2D::endDraw() const
	{
		HRESULT hr;
		hr = devCon->EndDraw();
		if (FAILED(hr))
			return hr;

		// return success
		return { };
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
	/////////////////////////////////// Getters /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const unsigned int Direct2D::getCurrentWidth() const
	{
		return dxApp.getGraphicsComponent().getCurrentWidth();
	}
	const unsigned int Direct2D::getCurrentHeight() const
	{
		return dxApp.getGraphicsComponent().getCurrentHeight();
	}
	
	IDWriteFactory6& Direct2D::getWriteFactory() const
	{
		return *writeFactory.Get();
	}
	ID2D1DeviceContext6& Direct2D::getDeviceContext() const
	{
		return *devCon.Get();
	}
	ID2D1SolidColorBrush& Direct2D::getBlackBrush() const
	{
		return *blackBrush.Get();
	}
}


