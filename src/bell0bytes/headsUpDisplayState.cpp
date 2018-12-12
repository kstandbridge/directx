// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "app.h"
#include "playState.h"

// bell0bytes UI
#include "HeadsUpDisplayState.h"
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
	HeadsUpDisplayState::HeadsUpDisplayState(core::DirectXApp* const app, const std::wstring& name) : GameState(app, name)
	{

	}

	HeadsUpDisplayState::~HeadsUpDisplayState()
	{

	}

	HeadsUpDisplayState& HeadsUpDisplayState::createInstance(core::DirectXApp* const app, const std::wstring& stateName)
	{
		static HeadsUpDisplayState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::initialize()
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
			result = d2d->createTextFormat(L"Segoe UI", 72.0f, DWRITE_TEXT_ALIGNMENT_CENTER, hudFormat);
			if (!result.isValid())
				return result;

			// create text layouts
			std::wstring hud = L"HUD Overlay";
			result = d2d->createTextLayoutFromWString(&hud, hudFormat.Get(), (float)dxApp->getCurrentWidth(), 100, hudLayout);
			if (!result.wasSuccessful())
				return result;
		}

		firstCreation = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::pause()
	{
		isPaused = true;

		// return success
		return {};
	}

	util::Expected<void> HeadsUpDisplayState::resume()
	{
		// allow mouse and keyboard input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = true;

		isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> HeadsUpDisplayState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if (!isPaused)
			if (!listening)
				return handleInput(ih->activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> HeadsUpDisplayState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& /*activeKeyMap*/)
	{
		if (isPaused)
			return true;

		// act on user input
		//for (auto x : activeKeyMap)
		//{
		//	switch (x.first)
		//	{
		//	case input::Select:
		//		// continue game
		//		if (!dxApp->popGameState().wasSuccessful())
		//			return std::runtime_error("Critical error: Unable to pop game menu state!");
		//		return false;	// notify stack change

		//	case input::GameCommands::Back:
		//		// back to main menu
		//		if (!dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
		//			return std::runtime_error("Critical error: Unable to change game state to main menu");
		//		return false;	// notify stack change

		//	case input::GameCommands::ShowFPS:
		//		dxApp->toggleFPS();
		//		break;
		//	}
		//}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::update(const double /*deltaTime*/)
	{
		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::render(const double /*farSeer*/)
	{
		if (!isPaused)
		{
			d2d->printText(0, 800, hudLayout.Get());

			// print FPS information
			d2d->printFPS();
		}
		else
			d2d->printText(0, 800, hudLayout.Get(), 0.25f);

		// print FPS information
		d2d->printFPS();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::shutdown()
	{
		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);

		isPaused = true;

		// return success
		return {};
	}
}