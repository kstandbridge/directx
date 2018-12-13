// include the header
#include "graphicsComponentWrite.h"
#include "app.h"
#include <iostream>
#include <sstream>
#include "graphicsComponent.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructors ///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GraphicsComponentWrite::GraphicsComponentWrite(const core::DirectXApp& dxApp, ID2D1DeviceContext6& devCon, IDWriteFactory6& writeFactory, ID2D1SolidColorBrush& blackBrush) : dxApp(dxApp), devCon(devCon), writeFactory(writeFactory), blackBrush(blackBrush)
	{
		onResize();
	}

	void GraphicsComponentWrite::onResize()
	{
		// create text format for FPS information
		if (FAILED(writeFactory.CreateTextFormat(L"Lucida Console", NULL, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-GB", (IDWriteTextFormat **)textFormatFPS.GetAddressOf())))
			throw std::runtime_error("Critical error: Unable to create text format for FPS information!");
		if (FAILED(textFormatFPS->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
			throw std::runtime_error("Critical error: Unable to set text alignment!");
		if (FAILED(textFormatFPS->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
			throw std::runtime_error("Critical error: Unable to set paragraph alignment!");
	}

	void GraphicsComponentWrite::printFPS(ID2D1SolidColorBrush& brush) const
	{
		if (dxApp.showFramesPerSecond() && textLayoutFPS)
			// draw the text
			devCon.DrawTextLayout(D2D1::Point2F(2.5f, 5.0f), textLayoutFPS.Get(), &brush);
	}

	void GraphicsComponentWrite::printFPS() const
	{
		printFPS(blackBrush);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Text Formats and Layouts /////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GraphicsComponentWrite::createTextFormat(LPCWSTR fontFamilyName, const DWRITE_FONT_WEIGHT fontWeight, const DWRITE_FONT_STYLE fontStyle, const DWRITE_FONT_STRETCH fontStretch, const float fontSize, LPCWSTR localeName, const DWRITE_TEXT_ALIGNMENT textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat, IDWriteFontCollection2* const fontCollection)
	{
		// create the text format
		if (FAILED(writeFactory.CreateTextFormat(fontFamilyName, fontCollection, fontWeight, fontStyle, fontStretch, fontSize, localeName, (IDWriteTextFormat **)textFormat.GetAddressOf())))
			return std::runtime_error("Critical error: Unable to create text format.");
		if (FAILED(textFormat->SetTextAlignment(textAlignment)))
			return std::runtime_error("Critical error: Unable to set text alignment!");
		if (FAILED(textFormat->SetParagraphAlignment(paragraphAlignment)))
			return std::runtime_error("Critical error: Unable to set paragraph alignment!");

		// return success
		return {};
	}

	util::Expected<void> GraphicsComponentWrite::createTextFormat(LPCWSTR fontFamilyName, const float fontSize, const DWRITE_TEXT_ALIGNMENT textAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat)
	{
		return createTextFormat(fontFamilyName, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-GB", textAlignment, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, textFormat);
	}

	util::Expected<void> GraphicsComponentWrite::createTextFormat(LPCWSTR fontFamilyName, const float fontSize, const DWRITE_TEXT_ALIGNMENT textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat)
	{
		return createTextFormat(fontFamilyName, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-GB", textAlignment, paragraphAlignment, textFormat);
	}

	util::Expected<void> GraphicsComponentWrite::createTextFormat(LPCWSTR fontFamilyName, const float fontSize, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat)
	{
		return createTextFormat(fontFamilyName, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-GB", DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, textFormat);
	}

	util::Expected<void> GraphicsComponentWrite::createTextLayoutFromWStringStream(const std::wostringstream& string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout)
	{
		if (!textFormat)
			return std::runtime_error("Critical error: Tried to create a layout from an empty text format!");

		if (FAILED(writeFactory.CreateTextLayout(string.str().c_str(), (UINT32)string.str().size(), textFormat, maxWidth, maxHeight, (IDWriteTextLayout **)textLayout.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Failed to create the text layout!");

		// return success
		return {};
	}

	util::Expected<void> GraphicsComponentWrite::createTextLayoutFromWString(const std::wstring& string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout)
	{
		if (!textFormat)
			return std::runtime_error("Critical error: Tried to create a layout from an empty text format!");

		if (FAILED(writeFactory.CreateTextLayout(string.c_str(), (UINT32)string.size(), textFormat, maxWidth, maxHeight, (IDWriteTextLayout **)textLayout.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Failed to create the text layout!");

		// return success
		return {};
	}

	util::Expected<void> GraphicsComponentWrite::createTextLayoutFPS(const std::wostringstream& stringFPS, const float width, const float height)
	{
		if (FAILED(writeFactory.CreateTextLayout(stringFPS.str().c_str(), (unsigned int)stringFPS.str().size(), textFormatFPS.Get(), width, height, (IDWriteTextLayout **)textLayoutFPS.ReleaseAndGetAddressOf())))
			return std::runtime_error("Critical error: Failed to create the text layout for FPS information!");

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Printing Functions //////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	// print text
	util::Expected<void> GraphicsComponentWrite::printText(const D2D1_POINT_2F& pos, IDWriteTextLayout4* const textLayout, const float opacity, ID2D1SolidColorBrush* const brush) const
	{
		if (!textLayout)
			return std::runtime_error("Critical error: Tried to print an empty text layout!");

		if (brush)
		{
			float oldOpacity = brush->GetOpacity();
			if (oldOpacity != opacity)
				brush->SetOpacity(opacity);

			devCon.DrawTextLayout(pos, textLayout, brush);

			if (oldOpacity != opacity)
				brush->SetOpacity(oldOpacity);
		}
		else
		{
			if (opacity != 1.0f)
				blackBrush.SetOpacity(opacity);

			devCon.DrawTextLayout(pos, textLayout, &blackBrush);

			if (opacity != 1.0f)
				blackBrush.SetOpacity(1.0f);
		}

		// return success
		return {};
	}

	util::Expected<void> GraphicsComponentWrite::printText(const float x, const float y, IDWriteTextLayout4* const textLayout, const float opacity, ID2D1SolidColorBrush* const brush) const
	{
		D2D1_POINT_2F pos = D2D1::Point2F(x, y);
		return printText(pos, textLayout, opacity, brush);
	}

	util::Expected<void> GraphicsComponentWrite::printCenteredText(IDWriteTextLayout4* const textLayout, const float xOffset, const float yOffset, const float opacity, ID2D1SolidColorBrush* const brush) const
	{
		D2D1_POINT_2F pos = D2D1::Point2F();

		// compute starting point

		// x-axis
		float centerWidth = (float)dxApp.getGraphicsComponent().getCurrentWidth() / 2.0f;
		float minWidth;
		textLayout->DetermineMinWidth(&minWidth);
		pos.x = centerWidth - minWidth + xOffset;

		// y-axis
		pos.y = ((float)dxApp.getGraphicsComponent().getCurrentHeight() / 2.0f) - textLayout->GetMaxHeight() + yOffset;

		return printText(pos, textLayout, opacity, brush);
	}
}