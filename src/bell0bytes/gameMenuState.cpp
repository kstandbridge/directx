// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "app.h"
#include "playState.h"

// bell0bytes UI
#include "GameMenuState.h"
#include "mainMenuState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputHandler.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GameMenuState::GameMenuState(core::DirectXApp* const app, const std::wstring& name) : GameState(app, name)
	{

	}

	GameMenuState::~GameMenuState()
	{

	}

	GameMenuState& GameMenuState::createInstance(core::DirectXApp* const app, const std::wstring& stateName)
	{
		static GameMenuState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::initialize()
	{
		// handle errors
		util::Expected<void> result;

		// add to observer list of the input handler
		dxApp->addInputHandlerObserver(this);

		// position mouse at the center of the screen
		if (!SetCursorPos(dxApp->getCurrentWidth() / 2, dxApp->getCurrentHeight() / 2))
			return std::runtime_error("Critical error: Unable to set cursor position!");

		// hide the standard cursor
		ShowCursor(false);

		// allow mouse and keyboard input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = true;
		isPaused = false;

		if (firstCreation)
		{
			// create text formats
			result = d2d->createTextFormat(L"Segoe UI", 72.0f, DWRITE_TEXT_ALIGNMENT_CENTER, gameMenuFormat);
			if (!result.isValid())
				return result;

			// create text layouts
			std::wstring menu = L"Game Menu on top of the Game Scene";
			result = d2d->createTextLayoutFromWString(&menu, gameMenuFormat.Get(), (float)dxApp->getCurrentWidth(), 100, gameMenuLayout);
			if (!result.wasSuccessful())
				return result;
		}

		firstCreation = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::pause()
	{
		isPaused = true;

		// return success
		return { };
	}

	util::Expected<void> GameMenuState::resume()
	{
		// allow mouse and keyboard input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = true;

		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> GameMenuState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if (!isPaused)
			if(!listening)
				return handleInput(ih->activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> GameMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		if (isPaused)
			return true;

		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::Select:
				// continue game
				if (!dxApp->popGameState().wasSuccessful())
					return std::runtime_error("Critical error: Unable to pop game menu state!");
				return false;	// notify stack change

			case input::GameCommands::Back:
				// back to main menu
				if (!dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
					return std::runtime_error("Critical error: Unable to change game state to main menu");
				return false;	// notify stack change

			case input::GameCommands::ShowFPS:
				dxApp->toggleFPS();
				break;
			}
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::update(const double /*deltaTime*/)
	{
		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::render(const double /*farSeer*/)
	{
		d2d->printText(0, 200, gameMenuLayout.Get());

		// print FPS information
		d2d->printFPS();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::shutdown()
	{
		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);

		isPaused = true;

		// return success
		return { };
	}
}