// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes
#include "app.h"
#include "GameMenuState.h"
#include "gameCommands.h"
#include "inputHandler.h"
#include "playState.h"
#include "mainMenuState.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GameMenuState::GameMenuState(core::DirectXApp* const app, std::wstring name) : GameState(app, name)
	{

	}

	GameMenuState::~GameMenuState()
	{

	}

	GameMenuState& GameMenuState::createInstance(core::DirectXApp* const app, std::wstring stateName)
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

		// create text formats
		result = d2d->createTextFormat(L"Segoe UI", 72.0f, gameMenuFormat);
		if (!result.isValid())
			return result;

		// create text layouts
		std::wstring menu = L"Game Menu on top of the Game Scene";
		result = d2d->createTextLayoutFromWString(&menu, gameMenuFormat.Get(), (float)dxApp->getCurrentWidth(), 100, gameMenuLayout);
		if (!result.wasSuccessful())
			return result;

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
	util::Expected<bool> GameMenuState::onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		if (!isPaused)
			return handleInput(activeKeyMap);

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
				// create game play state
				if (!dxApp->gameIsRunning)
					dxApp->changeGameState(&core::PlayState::createInstance(dxApp, L"Game"));
				else
					dxApp->popGameState();
				return false;

			case input::GameCommands::Quit:
				dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu"));
				return false;

			case input::GameCommands::ShowFPS:
				dxApp->showFPS = !dxApp->showFPS;
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
		d2d->printCenteredText(gameMenuLayout.Get(), -400, -250);

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

		// return success
		return { };
	}
}