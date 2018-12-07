#include "d2d.h"
#include "app.h"
#include "serviceLocator.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct2D::Direct2D(core::DirectXApp* dxApp) : dxApp(dxApp)
	{
		// create the device and its context
		if (!createDevice().wasSuccessful())
			throw std::runtime_error("Crital error: Failed to initialize Direct2D!");

		// create the bitmap target to render to
		if (!createBitmapRenderTarget().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create the bitmap render target for Direct2D!");

		// initialize the text formats
		if (!initializeTextFormats().wasSuccessful())
			throw std::runtime_error("Critical error: Failed to create text formats!");

		// log success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Direct2D was successfully initialized.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Initialization ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct2D::createDevice()
	{
		// create the DirectWrite factory
		if(FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &writeFactory)))
			return std::runtime_error("Critical error: Unable to create the DirectWrite factory!");

		// create the Direct2D factory
		D2D1_FACTORY_OPTIONS options;
#ifndef NDEBUG
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
		if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory2), &options, &factory)))
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

		// return success
		return { };
	}

	util::Expected<void> Direct2D::initializeTextFormats()
	{
		// create standard brushes
		if(FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &yellowBrush)))
			return std::runtime_error("Critical error: Unable to create the yellow brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush)))
			return std::runtime_error("Critical error: Unable to create the black brush!");
		if (FAILED(devCon->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush)))
			return std::runtime_error("Critical error: Unable to create the white brush!");
	
		// set up text formats

		// FPS text
		if(FAILED(writeFactory.Get()->CreateTextFormat(L"Lucida Console", nullptr, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-GB", &textFormatFPS)))
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
			devCon->DrawTextLayout(D2D1::Point2F(2.5f, 5.0f), textLayoutFPS.Get(), brush.Get());
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Shut Down ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct2D::~Direct2D()
	{
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Direct2D was shut down successfully.");
	}

}