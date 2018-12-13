#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		main menu of the game
*
* Hist:		- 19/06/2018: fixed a memory leak resulting from creating the same layout multiple times
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes includes
#include "states.h"

#include <wrl.h>
#include <dwrite_3.h>

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
	struct StreamEvent;
}

namespace UI
{
	class AnimatedButton;

	class MainMenuState : public core::GameState
	{
	private:
		// title text
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> mainMenuFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> mainMenuLayout;

		// the menu buttons
		std::deque<AnimatedButton*> menuButtons;
		unsigned int currentlySelectedButton;
		audio::SoundEvent* buttonSound;							// sound when button is clicked

		// menu music
		audio::StreamEvent* menuMusic;
		
		
		// private constructor -> singleton
		MainMenuState(core::DirectXApp& app, const std::wstring& name);

		// helper functions
		util::Expected<void> initializeButtons();				// initialize the menu button graphics

	public:
		virtual ~MainMenuState();
		bool musicIsPlaying = false;							// true iff the music is playing
				
		// singleton: get instance
		static MainMenuState& createInstance(core::DirectXApp& app, const std::wstring& name);

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;
		void releaseAudio();
		
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