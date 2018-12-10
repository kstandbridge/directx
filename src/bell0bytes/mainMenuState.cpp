// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes
#include "app.h"
#include "mainMenuState.h"
#include "gameCommands.h"
#include "inputHandler.h"
#include "playState.h"
#include "sprites.h"
#include "buttons.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	MainMenuState::MainMenuState(core::DirectXApp* const app, std::wstring name) : GameState(app, name), isShuttingDown(false), currentlySelectedButton(0)
	{
		
	}

	MainMenuState::~MainMenuState()
	{
		firstCreation = true;
	}

	MainMenuState& MainMenuState::createInstance(core::DirectXApp* const app, std::wstring stateName)
	{
		static MainMenuState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::initialize()
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
		result = d2d->createTextFormat(L"Segoe UI", 72.0f, mainMenuFormat);
		if (!result.isValid())
			return result;

		// create text layouts
		std::wstring menu = L"Main Menu";
		result = d2d->createTextLayoutFromWString(&menu, mainMenuFormat.Get(), (float)dxApp->getCurrentWidth(), 100, mainMenuLayout);
		if (!result.isValid())
			return result;

		// create buttons
		if (!initializeButtons().wasSuccessful())
			return std::runtime_error("Critical error: Unable to initialize menu buttons!");

		firstCreation = false;

		// return success
		return { };
	}

	util::Expected<void> MainMenuState::initializeButtons()
	{
		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// Play Button //////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// set play button animation cycles
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

		// cycle
		cycle.name = L"Play Button Deselected";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 300;
		cycle.height = 80;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 20;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Play Button Selected";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 30;
		cycle.width = 300;
		cycle.height = 80;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, L"Art/playButton.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// set lambda function
		auto onClick = [this]
		{
			this->isPaused = true;
			dxApp->changeGameState(&core::PlayState::createInstance(dxApp, L"Game"));
			return false;
		};

		// add button to the list
		menuButtons.push_back(new Button(L"Play Button", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClick));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Options Button //////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		// cycle
		cycle.name = L"Options Button Deselected";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 300;
		cycle.height = 80;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 20;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Options Button Selected";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 30;
		cycle.width = 300;
		cycle.height = 80;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, L"Art/optionsButton.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// set lambda function
		auto onClickOptions = [this]
		{
			return true;
		};

		// add button to the list
		menuButtons.push_back(new Button(L"Quit Options", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickOptions));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);
		
		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// Quit Button //////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		// cycle
		cycle.name = L"Quit Button Deselected";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 300;
		cycle.height = 80;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 20;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Quit Button Selected";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 30;
		cycle.width = 300;
		cycle.height = 80;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, L"Art/quitButton.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// set lambda function
		auto onClickQuit = [this]
		{
			this->isShuttingDown = true;
			PostMessage(dxApp->getMainWindow(), WM_CLOSE, 0, 0);
			this->shutdown();
			return false;
		};

		// add button to the list
		menuButtons.push_back(new Button(L"Quit Button", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickQuit));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);



		// set to unpaused
		this->isPaused = false;

		// set active button
		menuButtons[0]->select();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::pause()
	{
		isPaused = true;

		// return success
		return { };
	}

	util::Expected<void> MainMenuState::resume()
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
	util::Expected<bool> MainMenuState::onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		if(!isPaused)
			return handleInput(activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> MainMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::Select:
				// activate currently selected button
				return menuButtons[currentlySelectedButton]->click();;

			case input::MoveDown:
				// select next button in the list
				menuButtons[currentlySelectedButton]->deselect();
				if (currentlySelectedButton < menuButtons.size() - 1)
					currentlySelectedButton++;
				else
					currentlySelectedButton = 0;
				menuButtons[currentlySelectedButton]->select();
				return true;

			case input::MoveUp:
				// select next button in the list
				menuButtons[currentlySelectedButton]->deselect();
				if (currentlySelectedButton > 0)
					currentlySelectedButton--;
				else
					currentlySelectedButton = (unsigned int)menuButtons.size()-1;
				menuButtons[currentlySelectedButton]->select();
				return true;

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
	util::Expected<void> MainMenuState::update(const double deltaTime)
	{
		if (isPaused)
			return { };

		if (dxApp->activeMouse)
		{
			// get mouse position
			long mouseX = dxApp->getMouseX();
			long mouseY = dxApp->getMouseY();

			// check if mouse position is inside button rectangle
			unsigned int i = 0;
			for (auto button : menuButtons)
			{
				D2D1_RECT_F rect = button->getRectangle();
				if (mouseX > rect.left && mouseX < rect.right)
					if (mouseY > rect.top && mouseY < rect.bottom)
					{
						if (currentlySelectedButton != i)
						{
							// deselect current button
							menuButtons[currentlySelectedButton]->deselect();

							// select button
							button->select();
							currentlySelectedButton = i;
						}
					}
				i++;
			}
		}

		for (auto button : menuButtons)
			button->update(deltaTime);

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::render(const double /*farSeer*/)
	{
		if (isShuttingDown)
			return {  };

		d2d->printCenteredText(mainMenuLayout.Get(), 0, -300);

		// draw buttons
		unsigned int i = 0;
		unsigned int offset = 80;
		for (auto button : menuButtons)
		{
			button->drawCentered(1, 0, 100+((float)i*offset+5));
			i++;
		}

		// print FPS information
		d2d->printFPS();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::shutdown()
	{
		this->isPaused = true;

		// delete buttons
		for (auto button : menuButtons)
			delete button;
		menuButtons.clear();
	
		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);

		// return success
		return { };
	}
}