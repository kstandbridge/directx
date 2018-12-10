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

// bell0bytes includes
#include "states.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class Sprites;
}

namespace UI
{
	class IntroState : public core::GameState
	{

	private:
		// text
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> companyNameFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> authorNameFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> companyNameLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> authorNameLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> continueFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> continueLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> trademarkFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> trademarkLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> trademarkCountdownFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> trademarkCountdownLayout;

		// Logos
		graphics::Sprite* boostLogo;
		graphics::Sprite* dxLogo;

		// calculate frame time
		double frameTime;
		double secondsPerLogo;

		// blink text
		bool showContinueText;

		// show logos
		bool showTradeMarkLogos;

	protected:
		IntroState(core::DirectXApp* const app, std::wstring name);

	public:
		virtual ~IntroState();

		// singleton: get instance
		static IntroState& createInstance(core::DirectXApp* const app, std::wstring name);

		// observer: on notification
		util::Expected<bool> onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;

		// initialization
		virtual void initialize() override;
		virtual void shutdown() override;

		// pause and resume
		virtual void pause() override;
		virtual void resume() override;

		// user input
		virtual bool handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual void update(const double deltaTime) override;

		// render the scene
		virtual void render(const double farSeer) override;
	};
}