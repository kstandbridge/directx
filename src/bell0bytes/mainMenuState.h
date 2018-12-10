#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		main menu of the game
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
	class AnimatedSprite;
}

namespace UI
{
	class Button;

	class MainMenuState : public core::GameState
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> mainMenuFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> mainMenuLayout;

		// the menu buttons
		std::deque<Button*> menuButtons;
		unsigned int currentlySelectedButton;

		// shutting down
		bool isShuttingDown;

	protected:
		MainMenuState(core::DirectXApp* const app, std::wstring name);

	public:
		virtual ~MainMenuState();

		// singleton: get instance
		static MainMenuState& createInstance(core::DirectXApp* const app, std::wstring name);

		// observer: on notification
		util::Expected<bool> onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;

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