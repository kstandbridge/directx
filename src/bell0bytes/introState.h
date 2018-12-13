#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		intro sequence
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "states.h"

#include <dwrite_3.h>
#include <wrl.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class Sprite;
}

namespace audio
{
	struct SoundEvent;
}

namespace UI
{
	class IntroState : public core::GameState
	{

	private:
		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> companyNameFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> authorNameFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> continueFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> trademarkFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> trademarkCountdownFormat;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> companyNameLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> authorNameLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> continueLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> trademarkLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> trademarkCountdownLayout;

		// logos
		std::vector<graphics::Sprite*> logos;

		// intro music
		audio::SoundEvent* introMusic;

		// calculate frame time
		double frameTime;
		double secondsPerLogo;

		// blink text
		bool showContinueText;

		// show logos
		bool showTradeMarkLogos;

		// texts and layouts
		util::Expected<void> createTextFormats();
		util::Expected<void> createTextLayouts();
		util::Expected<void> updateTrademarkCountdownTextLayout();

		// logos
		util::Expected<void> initializeLogoSprites();

	protected:
		IntroState(core::DirectXApp& app, const std::wstring& name);

	public:
		virtual ~IntroState();

		// singleton: get instance
		static IntroState& createInstance( core::DirectXApp& app, const std::wstring& name);

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		// pause and resume
		virtual util::Expected<void> pause() override;
		virtual util::Expected<void> resume() override;

		// user input
		virtual util::Expected<void> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual util::Expected<void> update(const double deltaTime) override;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) override;

		// handle message
		virtual util::Expected<void> onMessage(const core::Depesche&) override;
	};
}