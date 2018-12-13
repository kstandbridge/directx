/****************************************************************************************
* Author:	Gilles Bellot
* Date:		28/06/2018 - Lenningen - Luxembourg
*
* Desc:		DirectWrite graphics components
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////
#include "expected.h"

#include <wrl.h>
#include <dwrite_3.h>
#include <d2d1_3.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class GraphicsComponentWrite
	{

	private:
		const core::DirectXApp& dxApp;			// the main application class
		IDWriteFactory6& writeFactory;			// adress of the DirectWrite factory
		ID2D1DeviceContext6& devCon;			// adress to the Direct2D device context
		ID2D1SolidColorBrush& blackBrush;		// black brush
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> textFormatFPS;	// text format for FPS information
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> textLayoutFPS;	// text layout for FPS information

		// on resize
		void onResize();
	public:
		GraphicsComponentWrite(const core::DirectXApp& dxApp, ID2D1DeviceContext6& devCon, IDWriteFactory6& writeFactory, ID2D1SolidColorBrush& blackBrush);
		~GraphicsComponentWrite() {};

		// print fps
		void printFPS(ID2D1SolidColorBrush& brush) const;	// prints fps information to the screen in the desired colour specified by the brush
		void printFPS() const;

		// text formats and layouts
		util::Expected<void> createTextFormat(LPCWSTR fontFamilyName, const DWRITE_FONT_WEIGHT fontWeight, const DWRITE_FONT_STYLE fontStyle, const DWRITE_FONT_STRETCH fontStretch, const float fontSize, LPCWSTR localeName, const DWRITE_TEXT_ALIGNMENT textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat, IDWriteFontCollection2* const fontCollection = NULL);	// creates a text format with the specifies properties and stores the result in the textFormat parameter
		util::Expected<void> createTextFormat(LPCWSTR fontFamilyName, const float fontSize, const DWRITE_TEXT_ALIGNMENT fontAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat);
		util::Expected<void> createTextFormat(LPCWSTR fontFamilyName, const float fontSize, const DWRITE_TEXT_ALIGNMENT fontAlignment, const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat);
		util::Expected<void> createTextFormat(LPCWSTR fontFamilyName, const float fontSize, Microsoft::WRL::ComPtr<IDWriteTextFormat3>& textFormat);																																																																							// creates a standard text format
		util::Expected<void> createTextLayoutFromWStringStream(const std::wostringstream& string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout);
		util::Expected<void> createTextLayoutFromWString(const std::wstring& string, IDWriteTextFormat3* const textFormat, const float maxWidth, const float maxHeight, Microsoft::WRL::ComPtr<IDWriteTextLayout4>& textLayout);
		util::Expected<void> createTextLayoutFPS(const std::wostringstream& stringFPS, const float width, const float height);

		// print
		util::Expected<void> printText(const D2D1_POINT_2F& pos, IDWriteTextLayout4* const textLayout, const float opacity = 1.0f, ID2D1SolidColorBrush* const brush = NULL) const;
		util::Expected<void> printText(const float x, const float y, IDWriteTextLayout4* const textLayout, const float opacity = 1.0f, ID2D1SolidColorBrush* const brush = NULL) const;
		util::Expected<void> printCenteredText(IDWriteTextLayout4* const textLayout, const float xOffset = 0.0f, const float yOffset = 0.0f, const float opacity = 1.0f, ID2D1SolidColorBrush* const brush = NULL) const;

		friend class GraphicsComponent;
	};
}