// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "app.h"

// bell0bytes UI
#include "playState.h"
#include "gameMenuState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputHandler.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	PlayState::PlayState(DirectXApp* const app, const std::wstring& name) : GameState(app, name)
	{ }

	PlayState::~PlayState()
	{ }

	PlayState& PlayState::createInstance(DirectXApp* const app, const std::wstring& stateName)
	{
		static PlayState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::initialize()
	{
		// catch errors
		util::Expected<void> result;
		
		// add as an observer to the input handler
		dxApp->addInputHandlerObserver(this);

		// allow only keyboard input
		dxApp->activeKeyboard = true;
		dxApp->activeMouse = false;

		// notify the main application class that the game is running
		isPaused = false;

		if (firstCreation)
		{
			// create text formats
			result = d2d->createTextFormat(L"Segoe UI", 72.0f, DWRITE_TEXT_ALIGNMENT_CENTER, playStateFormat);
			if (!result.wasSuccessful())
				return result;

			// create text layouts
			std::wstring playText = L"Game Scene!";
			result = d2d->createTextLayoutFromWString(&playText, playStateFormat.Get(), (float)dxApp->getCurrentWidth(), 100, playStateLayout);
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
	util::Expected<void> PlayState::pause()
	{
		isPaused = true;

		// return success
		return { };
	}

	util::Expected<void> PlayState::resume()
	{
		// allow only keyboard input
		dxApp->activeKeyboard = true;
		dxApp->activeMouse = false;

		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> PlayState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if(!isPaused)
			if(!listening)
				return handleInput(ih->activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> PlayState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::GameCommands::Back:
				if (!dxApp->pushGameState(&UI::GameMenuState::createInstance(dxApp, L"Game Menu")).wasSuccessful())
					return std::runtime_error("Critical error: Unable to push game menu state!");
				return false;	// notify stack change

			case input::GameCommands::ShowFPS:
				dxApp->toggleFPS();
				break;
			}
		}

		return true;	// no stack change
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::update(const double /*deltaTime*/)
	{
		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::render(const double /*farSeer*/)
	{
		if (!isPaused)
		{
			d2d->printText(0, dxApp->getCurrentHeight()/2.0f - 50.0f, playStateLayout.Get());

			// print FPS information
			d2d->printFPS();
		}
		else
			d2d->printText(0, dxApp->getCurrentHeight()/2.0f - 50.f, playStateLayout.Get(), 0.25f);

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::shutdown()
	{
		// remove from the observer list of the input handler
		dxApp->removeInputHandlerObserver(this);
		isPaused = true;

		// return success
		return { };
	}
}