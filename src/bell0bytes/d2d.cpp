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
	Direct2D::Direct2D(const core::DirectXApp* const dxApp, Direct3D* const d3d) : dxApp(dxApp), matrixRotation90CW(D2D1::Matrix3x2F::Rotation(90)), 
																matrixRotation180CW(D2D1::Matrix3x2F::Rotation(180)), 
																matrixRotation270CW(D2D1::Matrix3x2F::Rotation(270)), 
																matrixRotation90CCW(D2D1::Matrix3x2F::Rotation(-90)), 
																matrixRotation180CCW(D2D1::Matrix3x2F::Rotation(-180)), 
																matrixRotation270CCW(D2D1::Matrix3x2F::Rotation(-270))
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
	util::Expected<void> Direct2D::createDevice(Direct3D* const d3d)
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
		if(FAILED(d3d->dev.Get()->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice)))
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

	util::Expected<void> Direct2D::createBitmapRenderTarget(Direct3D* const d3d)
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
		if (FAILED(d3d->swapChain->GetBuffer(0, __uuidof(IDXGISurface), &dxgiBuffer)))
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
		// FPS text
		if (FAILED(writeFactory->CreateTextFormat(L"Lucida Console", NULL, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-GB", (IDWriteTextFormat **)textFormatFPS.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create text format for FPS information!");
		if (FAILED(textFormatFPS->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
			return std::runtime_error("Critical error: Unable to set text alignment!");
		if (FAILED(textFormatFPS->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
			return std::runtime_error("Critical error: Unable to set paragraph alignment!");

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
	util::Expected<void> Direct2D::createSolidColourBrush(const D2D1::ColorF& colour, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>& brush)
	{
		if (FAILED(devCon->CreateSolidColorBrush(colour, brush.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create brush!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createLinearGradientBrush(const float startX, const float startY, const float endX, const float endY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>& linearGradientBrush)
	{
		D2D1_POINT_2F startPoint = D2D1::Point2F(startX, startY);
		D2D1_POINT_2F endPoint = D2D1::Point2F(endX, endY);
		if (FAILED(devCon->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(startPoint, endPoint), &stopCollection, linearGradientBrush.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the linear gradient brush!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createRadialGradientBrush(const float centreX, const float centreY, const float offsetX, const float offsetY, const float radiusX, const float radiusY, ID2D1GradientStopCollection& stopCollection, Microsoft::WRL::ComPtr<ID2D1RadialGradientBrush>& radialGradientBrush)
	{
		D2D1_POINT_2F centre = D2D1::Point2F(centreX, centreY);
		D2D1_POINT_2F offset = D2D1::Point2F(offsetX, offsetY);
		if (FAILED(devCon->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(centre, offset, radiusX, radiusY), &stopCollection, radialGradientBrush.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Unable to create the radial gradient brush!");

		// return success
		return {};
	}

	util::Expected<void> Direct2D::createStrokeStyle(D2D1_STROKE_STYLE_PROPERTIES1& strokeProperties, Microsoft::WRL::ComPtr<ID2D1StrokeStyle1>& stroke)
	{
		if (FAILED(factory->CreateStrokeStyle(strokeProperties, nullptr, 0, stroke.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create stroke style!");

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Text Formats and Layouts /////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct2D::createTextFormat(LPCWSTR fontFamilyName, const DWRITE_FONT_WEIGHT fontWeight, const DWRITE_FONT_STYLE fontStyle, const DWRITE_FONT_STRETCH fontStretch, const float fontSize, LPCWSTR localeName, const DWRITE_TEXT_ALIGNMENT textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat, IDWriteFontCollection2* const fontCollection)
	{
		// create the text format
		if (FAILED(writeFactory->CreateTextFormat(fontFamilyName, fontCollection, fontWeight, fontStyle, fontStretch, fontSize, localeName, (IDWriteTextFormat **)textFormat.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create text format.");
		if (FAILED(textFormat->SetTextAlignment(textAlignment)))
			return std::runtime_error("Critical error: Unable to set text alignment!");
		if (FAILED(textFormat->SetParagraphAlignment(paragraphAlignment)))
			return std::runtime_error("Critical error: Unable to set paragraph alignment!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createTextFormat(LPCWSTR fontFamilyName, const float fontSize, const DWRITE_TEXT_ALIGNMENT textAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat)
	{
		return createTextFormat(fontFamilyName, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-GB", textAlignment, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, textFormat);
	}

	util::Expected<void> Direct2D::createTextFormat(LPCWSTR fontFamilyName, const float fontSize, const DWRITE_TEXT_ALIGNMENT textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat)
	{
		return createTextFormat(fontFamilyName, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-GB", textAlignment, paragraphAlignment, textFormat);
	}

	util::Expected<void> Direct2D::createTextFormat(LPCWSTR fontFamilyName, const float fontSize, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat)
	{
		return createTextFormat(fontFamilyName, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-GB", DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, textFormat);
	}

	util::Expected<void> Direct2D::createTextLayoutFromWStringStream(const std::wostringstream* const string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout)
	{
		if (FAILED(this->writeFactory->CreateTextLayout(string->str().c_str(), (UINT32)string->str().size(), textFormat, maxWidth, maxHeight, (IDWriteTextLayout **)textLayout.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Failed to create the text layout!");

		// return success
		return { };
	}

	util::Expected<void> Direct2D::createTextLayoutFromWString(const std::wstring* const string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout)
	{
		if (FAILED(this->writeFactory->CreateTextLayout(string->c_str(), (UINT32)string->size(), textFormat, maxWidth, maxHeight, (IDWriteTextLayout **)textLayout.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Failed to create the text layout!");

		// return success
		return {};
	}

	util::Expected<void> Direct2D::createTextLayoutFPS(const std::wostringstream* const stringFPS, const float width, const float height)
	{
		if (FAILED(writeFactory->CreateTextLayout(stringFPS->str().c_str(), (unsigned int)stringFPS->str().size(), textFormatFPS.Get(), width, height, (IDWriteTextLayout **)textLayoutFPS.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Failed to create the text layout for FPS information!");

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Bitmaps /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct2D::createBitmapFromWICBitmap(LPCWSTR imageFile, Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bitmap)
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
		if (FAILED(devCon->EndDraw()))
			return std::runtime_error("Critical error while drawing!");

		// return success
		return { };
	}
	
	// draw and fill rectangles
	void Direct2D::fillRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if (brush)
		{
			brush->SetOpacity(opacity);
			devCon->FillRectangle(&rect, brush);
		}
		else
		{
			if(opacity != 1.0f)
				blackBrush->SetOpacity(opacity);
		
			devCon->FillRectangle(&rect, blackBrush.Get());
			
			if(opacity != 1.0f)
				blackBrush->SetOpacity(1.0f);
		}
	}

	void Direct2D::fillRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { upperLeft.x , upperLeft.y , lowerRight.x , lowerRight.y };
		if (brush)
		{
			brush->SetOpacity(opacity);
			devCon->FillRectangle(&rect, brush);
		}
		else
		{
			if (opacity != 1.0f)
				blackBrush->SetOpacity(opacity);

			devCon->FillRectangle(&rect, blackBrush.Get());

			if (opacity != 1.0f)
				blackBrush->SetOpacity(1.0f);
		}
	}

	void Direct2D::drawRectangle(const float ulX, const float ulY, const float lrX, const float lrY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if(brush)
			devCon->DrawRectangle(&rect, brush, width, strokeStyle);
		else
			devCon->DrawRectangle(&rect, blackBrush.Get(), width, strokeStyle);
	}

	void Direct2D::drawRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y };
		if (brush)
			devCon->DrawRectangle(&rect, brush, width, strokeStyle);
		else
			devCon->DrawRectangle(&rect, blackBrush.Get(), width, strokeStyle);
	}

	// fill and draw rounded rectangles
	void Direct2D::fillRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if (brush)
		{
			brush->SetOpacity(opacity);
			devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush);
		}
		else
		{
			if (opacity != 1.0f)
				blackBrush->SetOpacity(opacity);
			
			devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), blackBrush.Get());
			
			if (opacity != 1.0f)
				blackBrush->SetOpacity(1.0f);
		}
	}

	void Direct2D::fillRoundedRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float radiusX, const float radiusY, const float opacity, ID2D1Brush* const brush) const
	{
		D2D1_RECT_F rect = { upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y };

		if (brush)
		{
			brush->SetOpacity(opacity);
			devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush);
		}
		else
		{
			if (opacity != 1.0f)
				blackBrush->SetOpacity(opacity);

			devCon->FillRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), blackBrush.Get());

			if (opacity != 1.0f)
				blackBrush->SetOpacity(opacity);
		}
	}

	void Direct2D::drawRoundedRectangle(const float ulX, const float ulY, const float lrX, const float lrY, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { ulX, ulY, lrX, lrY };
		if(brush)
			devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush, width, strokeStyle);
		else
			devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), blackBrush.Get(), width, strokeStyle);
	}

	void Direct2D::drawRoundedRectangle(const D2D1_POINT_2F& upperLeft, const D2D1_POINT_2F& lowerRight, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		D2D1_RECT_F rect = { upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y };
		if (brush)
			devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), brush, width, strokeStyle);
		else
			devCon->DrawRoundedRectangle(D2D1::RoundedRect(rect, radiusX, radiusY), blackBrush.Get(), width, strokeStyle);
	}

	// fill and draw ellipses
	void Direct2D::fillEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, const float opacity, ID2D1Brush* const brush) const
	{
		if (brush)
		{	
			brush->SetOpacity(opacity);
			devCon->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), brush);
		}
		else
		{	
			if (opacity != 1.0f)
				blackBrush->SetOpacity(opacity);
			devCon->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), blackBrush.Get());

			if (opacity != 1.0f)
				blackBrush->SetOpacity(1.0f);
		}
	}

	void Direct2D::drawEllipse(const float centreX, const float centreY, const float radiusX, const float radiusY, ID2D1Brush* const brush, const float width, ID2D1StrokeStyle1* const strokeStyle) const
	{
		if(brush)
			devCon->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), brush, width, strokeStyle);
		else
			devCon->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centreX, centreY), radiusX, radiusY), blackBrush.Get(), width, strokeStyle);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Transformations //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Direct2D::setTransformation(const D2D1::Matrix3x2F& transMatrix) const
	{
		devCon->SetTransform(transMatrix);
	}

	void Direct2D::resetTransformation() const
	{
		devCon->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	void Direct2D::setTransformation90CW() const
	{
		devCon->SetTransform(matrixRotation90CW);
	}
	
	void Direct2D::setTransformation180CW() const
	{
		devCon->SetTransform(matrixRotation180CW);
	}
	
	void Direct2D::setTransformation270CW() const
	{
		devCon->SetTransform(matrixRotation270CW);
	}
	
	void Direct2D::setTransformation90CCW() const
	{
		devCon->SetTransform(matrixRotation90CCW);
	}
	
	void Direct2D::setTransformation180CCW() const
	{
		devCon->SetTransform(matrixRotation180CCW);
	}
	
	void Direct2D::setTransformation270CCW() const
	{
		devCon->SetTransform(matrixRotation270CCW);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Printing Functions //////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	// print text
	void Direct2D::printText(const D2D1_POINT_2F& pos, IDWriteTextLayout4* const textLayout, const float opacity, ID2D1SolidColorBrush* const brush) const
	{
		if (brush)
		{
			float oldOpacity = brush->GetOpacity();
			if (oldOpacity != opacity)
				brush->SetOpacity(opacity);
			
			devCon->DrawTextLayout(pos, textLayout, brush);
			
			if (oldOpacity != opacity)
				brush->SetOpacity(oldOpacity);
		}
		else
		{
			if (opacity != 1.0f)
				blackBrush->SetOpacity(opacity);

			devCon->DrawTextLayout(pos, textLayout, blackBrush.Get());

			if (opacity != 1.0f)
				blackBrush->SetOpacity(1.0f);
		}
	}

	void Direct2D::printText(const float x, const float y, IDWriteTextLayout4* const textLayout, const float opacity, ID2D1SolidColorBrush* const brush) const
	{
		D2D1_POINT_2F pos = D2D1::Point2F(x, y);
		printText(pos, textLayout, opacity, brush);
	}

	void Direct2D::printCenteredText(IDWriteTextLayout4* const textLayout, const float xOffset, const float yOffset, const float opacity, ID2D1SolidColorBrush* const brush) const
	{
		D2D1_POINT_2F pos = D2D1::Point2F();

		// compute starting point

		// x-axis
		float centerWidth = (float)getCurrentWidth() / 2.0f;
		float minWidth;
		textLayout->DetermineMinWidth(&minWidth);
		pos.x = centerWidth - minWidth + xOffset;

		// y-axis
		pos.y = ((float)getCurrentHeight() / 2.0f) - textLayout->GetMaxHeight() + yOffset;

		printText(pos, textLayout, opacity, brush);
	}
	
	void Direct2D::printFPS(ID2D1SolidColorBrush* const brush) const
	{
		if (dxApp->showFramesPerSecond() && textLayoutFPS)
			// draw the text
			devCon->DrawTextLayout(D2D1::Point2F(2.5f, 5.0f), textLayoutFPS.Get(), brush);
	}

	void Direct2D::printFPS() const
	{
		this->printFPS(this->blackBrush.Get());
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
	util::Expected<D2D1_POINT_2F> Direct2D::computeCoordinatesOnEllipse(const D2D1_ELLIPSE *const ellipse, const float angle)
	{
		// the x and y-coordinates can be computed by circular functions
		return D2D1_POINT_2F({ ellipse->point.x + ellipse->radiusX * cos(angle*(float)M_PI/180) , ellipse->point.y + ellipse->radiusY * sin(angle*(float)M_PI/180) });
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Getters /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	unsigned int Direct2D::getCurrentWidth() const
	{
		return dxApp->getCurrentWidth();
	}

	unsigned int Direct2D::getCurrentHeight() const
	{
		return dxApp->getCurrentHeight();
	}
}


