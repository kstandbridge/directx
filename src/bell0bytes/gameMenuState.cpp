// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <sstream>

// bell0bytes core
#include "app.h"
#include "playState.h"

// bell0bytes UI
#include "GameMenuState.h"
#include "mainMenuState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputComponent.h"
#include "inputHandler.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponentWrite.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GameMenuState::GameMenuState(core::DirectXApp& app, const std::wstring& name) : GameState(app, name)
	{

	}

	GameMenuState::~GameMenuState()
	{

	}

	GameMenuState& GameMenuState::createInstance(core::DirectXApp& app, const std::wstring& stateName)
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

		// position mouse at the center of the screen
		if (!SetCursorPos(dxApp.getGraphicsComponent().getCurrentWidth() / 2, dxApp.getGraphicsComponent().getCurrentHeight() / 2))
			return std::runtime_error("Critical error: Unable to set cursor position!");

		// hide the standard cursor
		ShowCursor(false);

		// allow mouse and keyboard input
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;
		isPaused = false;

		if (firstCreation)
		{
			// create text formats
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 72.0f, DWRITE_TEXT_ALIGNMENT_CENTER, gameMenuFormat);
			if (!result.isValid())
				return result;

			// create text layouts
			std::wostringstream menu;
			menu << L"Game Menu on top of the Game Scene" << std::endl << std::endl << L"Press 'A' to continue" << std::endl << std::endl << L"Press 'B' to quit";
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(menu, gameMenuFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, gameMenuLayout);
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
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;

		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::onMessage(const core::Depesche& depesche)
	{
		input::InputHandler* ih = (input::InputHandler*)depesche.sender;

		if (!isPaused)
			if (!ih->isListening())
				return handleInput(ih->activeKeyMap);

		// return success
		return { };
	}

	util::Expected<void> GameMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
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
				if (!dxApp.popGameState().wasSuccessful())
					return std::runtime_error("Critical error: Unable to pop game menu state!");
				break;

			case input::GameCommands::Back:
				// back to main menu
				if (!dxApp.changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
					return std::runtime_error("Critical error: Unable to change game state to main menu");
				break;

			case input::GameCommands::ShowFPS:
				dxApp.toggleFPS();
				break;
			}
		}

		// return success
		return { };
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
		dxApp.getGraphicsComponent().getWriteComponent().printText(0, 200, gameMenuLayout.Get());

		// print FPS information
		dxApp.getGraphicsComponent().getWriteComponent().printFPS();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GameMenuState::shutdown()
	{
		isPaused = true;

		// return success
		return { };
	}
}