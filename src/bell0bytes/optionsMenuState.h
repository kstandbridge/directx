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

#include <d3d11.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class AnimatedSprite;
}

namespace audio
{
	struct SoundEvent;
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
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> soundEffectsVolumeLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> musicVolumeLayout;

		
		// the menu buttons
		std::deque<AnimatedButton*> menuButtons;
		int currentlySelectedButton;
		audio::SoundEvent* buttonClickSound;

		// screen resolution options
		const DXGI_MODE_DESC* supportedModes;
		unsigned int nSupportedModes;
		unsigned int currentModeIndex;

		// fullscreen options
		bool wasInFullscreen;
		bool fullscreen;

		// private constructor
		OptionsMenuState(core::DirectXApp& app, const std::wstring& name);
	
	public:
		virtual ~OptionsMenuState();

		// singleton: get instance
		static OptionsMenuState& createInstance(core::DirectXApp& app, const std::wstring& name);

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		util::Expected<void> initializeButtons();				// initialize the menu button graphics

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