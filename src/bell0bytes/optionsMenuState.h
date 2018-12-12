#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		main options menu of the game
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes include
#include "states.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class AnimatedSprite;
}

namespace UI
{
	class AnimatedButton;

	class OptionsMenuState : public core::GameState
	{
	private:
		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> titleFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> textFormat;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> titleLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> fullscreenLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> resolutionLayout;
		
		// the menu buttons
		std::deque<AnimatedButton*> menuButtons;
		int currentlySelectedButton;

		// screen resolution options
		const DXGI_MODE_DESC* supportedModes;
		unsigned int nSupportedModes;
		unsigned int currentModeIndex;

		// fullscreen options
		bool wasInFullscreen;
		bool fullscreen;

		// private constructor
		OptionsMenuState(core::DirectXApp* const app, const std::wstring& name);
	
	public:
		virtual ~OptionsMenuState();

		// singleton: get instance
		static OptionsMenuState& createInstance(core::DirectXApp* const app, const std::wstring& name);

		// observer: on notification
		util::Expected<bool> onNotify(input::InputHandler* const, const bool) override;

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		util::Expected<void> initializeButtons();				// initialize the menu button graphics

		// pause and resume
		virtual util::Expected<void> pause() override;
		virtual util::Expected<void> resume() override;

		// user input
		virtual util::Expected<bool> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual util::Expected<void> update(const double deltaTime) override;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) override;
	};
}