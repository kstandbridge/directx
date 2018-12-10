// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes
#include "app.h"
#include "playState.h"
#include "gameCommands.h"
#include "gameMenuState.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	PlayState::PlayState(DirectXApp* const app, std::wstring name) : GameState(app, name)
	{

	}

	PlayState::~PlayState()
	{

	}

	PlayState& PlayState::createInstance(DirectXApp* const app, std::wstring stateName)
	{
		static PlayState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void PlayState::initialize()
	{
		// add as an observer to the input handler
		dxApp->addInputHandlerObserver(this);

		// allow only keyboard input
		dxApp->activeKeyboard = true;
		dxApp->activeMouse = false;

		// show fps
		dxApp->showFPS = true;

		// notify the main application class that the game is running
		dxApp->gameIsRunning = true;

		// create text formats
		d2d->createTextFormat(L"Segoe UI", 72.0f, playStateFormat);

		// create text layouts
		std::wstring playText = L"Game Scene!";
		d2d->createTextLayoutFromWString(&playText, playStateFormat.Get(), (float)dxApp->getCurrentWidth(), 100, playStateLayout);

		resume();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void PlayState::pause()
	{
		isPaused = true;
	}

	void PlayState::resume()
	{
		// allow only keyboard input
		dxApp->activeKeyboard = true;
		dxApp->activeMouse = false;

		isPaused = false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> PlayState::onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		if(!isPaused)
			return handleInput(activeKeyMap);

		// return success
		return true;
	}

	bool PlayState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::GameCommands::Quit:
				dxApp->pushGameState(&UI::GameMenuState::createInstance(dxApp, L"Game Menu"));
				this->pause();
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
	void PlayState::update(const double /*deltaTime*/)
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void PlayState::render(const double /*farSeer*/)
	{
		if (!isPaused)
			d2d->printCenteredText(playStateLayout.Get());
		else
			d2d->printCenteredText(playStateLayout.Get(), 0, 0, 0.25f);

		// print FPS information
		d2d->printFPS();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void PlayState::shutdown()
	{
		// remove from the observer list of the input handler
		dxApp->removeInputHandlerObserver(this);

		dxApp->gameIsRunning = false;
	}
}