// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes
#include "app.h"
#include "optionsMenuState.h"
#include "gameCommands.h"
#include "inputHandler.h"
#include "playState.h"
#include "sprites.h"
#include "buttons.h"
#include "mainMenuState.h"
#include "keyMapMenuState.h"
#include "sstream"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	OptionsMenuState::OptionsMenuState(core::DirectXApp* const app, const std::wstring& name) : GameState(app, name), currentlySelectedButton(0), wasInFullscreen(dxApp->getFullscreenState()), fullscreen(dxApp->getFullscreenState()), supportedModes(NULL), nSupportedModes(0), currentModeIndex(0)
	{ }

	OptionsMenuState::~OptionsMenuState()
	{ }

	OptionsMenuState& OptionsMenuState::createInstance(core::DirectXApp* const app, const std::wstring& stateName)
	{
		static OptionsMenuState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::initialize()
	{
		// handle errors
		util::Expected<void> result;

		// add to observer list of the input handler
		dxApp->addInputHandlerObserver(this);

		// hide the standard cursor
		ShowCursor(false);

		// position mouse at the center of the screen
		if (!SetCursorPos(dxApp->getCurrentWidth() / 2, dxApp->getCurrentHeight() / 2))
			return std::runtime_error("Critical error: Unable to set cursor position!");

		// allow only mouse input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = false;

		// get fullscreen state
		fullscreen = dxApp->getFullscreenState();

		// get supported modes
		nSupportedModes = dxApp->getNumberOfSupportedModes();
		supportedModes = dxApp->getSupportedModes();
		currentModeIndex = dxApp->getCurrentModeIndex();
		
		if (firstCreation)
		{
			// create text formats
			result = d2d->createTextFormat(L"Lucida Handwriting", 128.0f, DWRITE_TEXT_ALIGNMENT_CENTER, titleFormat);
			if (!result.isValid())
				return result;

			result = d2d->createTextFormat(L"Segoe Script", 48.0f, textFormat);
			if (!result.isValid())
				return result;

			// create text layouts
			std::wstring title = L"Game Options";
			result = d2d->createTextLayoutFromWString(&title, titleFormat.Get(), (float)dxApp->getCurrentWidth(), 200, titleLayout);
			if (!result.isValid())
				return result;

			std::wostringstream fullscreenText;
			fullscreenText << L"fullscreen\t\t\t\t\t" << fullscreen;
			result = d2d->createTextLayoutFromWStringStream(&fullscreenText, textFormat.Get(), (float)dxApp->getCurrentWidth(), 100, fullscreenLayout);
			if (!result.isValid())
				return result;

			std::wostringstream text;
			text << L"resolution\t\t\t\t\t" << dxApp->getCurrentWidth() << " x " << dxApp->getCurrentHeight() << " @ " << supportedModes[currentModeIndex].RefreshRate.Numerator / supportedModes[currentModeIndex].RefreshRate.Denominator << " Hz";
			result = d2d->createTextLayoutFromWStringStream(&text, textFormat.Get(), (float)dxApp->getCurrentWidth(), 100, resolutionLayout);
			if (!result.isValid())
				return result;
		}

		// initialize buttons
		currentlySelectedButton = -1;
		if (!initializeButtons().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create title buttons!");

		firstCreation = false;
		isPaused = false;
	
		// return success
		return {};
	}

	util::Expected<void> OptionsMenuState::initializeButtons()
	{
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Fullscreen Selection ////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		
		// cycle
		cycle.name = L"Fullscreen Refresh Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Fullscreen Refresh Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Fullscreen Refresh Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Fullscreen Refresh Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp->openFile(fileSystem::DataFolders::Buttons, L"buttonRefresh.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClick = [this]
		{
			this->fullscreen = !this->fullscreen;
			return true;
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Fullscreen Toggle", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClick, 4)); }
		catch (std::exception& e) { return e; };

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////// Screen Resolution Left Arrow ////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Left Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp->openFile(fileSystem::DataFolders::Buttons, L"buttonLeft.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickScreenResolutionLeftArrow = [this]
		{
			if(this->currentModeIndex > 0)
				currentModeIndex--;
			return true;
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Screen Resolution Left", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickScreenResolutionLeftArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////// Screen Resolution Right Arrow ///////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Right Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp->openFile(fileSystem::DataFolders::Buttons, L"buttonRight.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickScreenResolutionRightArrow = [this]
		{
			if (this->currentModeIndex < this->nSupportedModes-1)
				currentModeIndex++;
			return true;
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Screen Resolution Right", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickScreenResolutionRightArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Gamepad Button ///////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Gamepad Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Gamepad Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Gamepad Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Gamepad Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp->openFile(fileSystem::DataFolders::Buttons, L"buttonGamepad.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickGamepad = [this]() -> util::Expected<bool>
		{
			// open key map menu
			if (!dxApp->pushGameState(&UI::KeyMapMenuState::createInstance(dxApp, L"Key Map Menu")).wasSuccessful())
				std::runtime_error("Critical error: Unable to push key map menu to the state stack!");
			return false;
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Gamepad", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickGamepad, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Button Save //////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Save Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp->openFile(fileSystem::DataFolders::Buttons, L"buttonSave.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickSave = [this]
		{
			// write options to lua file
			dxApp->saveConfiguration(supportedModes[currentModeIndex].Width, supportedModes[currentModeIndex].Height, currentModeIndex, fullscreen);

			// activate desired screen resolution and fullscreen mode
			if (currentModeIndex != dxApp->getCurrentModeIndex())
				dxApp->changeResolution(currentModeIndex);

			if (fullscreen != wasInFullscreen)
			{
				wasInFullscreen = !wasInFullscreen;
				dxApp->toggleFullscreen();
			}

			return true;
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Save", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickSave, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Back Button //////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Back Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp->openFile(fileSystem::DataFolders::Buttons, L"buttonBack.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickBack = [this]() -> util::Expected<bool>
		{
			this->isPaused = true;
			if (!dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
				return std::runtime_error("Critical error: Unable to change game state to main menu!");
			return false;
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Back", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickBack, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);
		
		// set to unpaused
		this->isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::pause()
	{
		isPaused = true;

		// return success
		return {};
	}

	util::Expected<void> OptionsMenuState::resume()
	{
		// allow mouse and keyboard input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = false;

		isPaused = false;
		currentlySelectedButton = -1;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> OptionsMenuState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if (!isPaused)
			if(!listening)
				return handleInput(ih->activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> OptionsMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::Select:
				// activate currently selected button
				if (currentlySelectedButton >= 0 && currentlySelectedButton < menuButtons.size())
					return menuButtons[currentlySelectedButton]->click();
				else
					break;

			case input::ShowFPS:
				dxApp->toggleFPS();
				break;
			}
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::update(const double deltaTime)
	{
		// handle errors
		util::Expected<void> result;

		if (isPaused)
			return {};

		// capture mouse
		if (dxApp->activeMouse)
		{
			// get mouse position
			long mouseX = dxApp->getMouseX();
			long mouseY = dxApp->getMouseY();

			// check if mouse position is inside button rectangle
			bool buttonSelected = false;
			int i = 0;
			for (auto button : menuButtons)
			{
				D2D1_RECT_F rect = button->getRectangle();
				if (mouseX > rect.left && mouseX < rect.right && mouseY > rect.top && mouseY < rect.bottom)
				{
					if (currentlySelectedButton != i)
					{
						// deselect current button
						if (currentlySelectedButton >= 0 && currentlySelectedButton < menuButtons.size())
							menuButtons[currentlySelectedButton]->deselect();

						// select button
						button->select();
						currentlySelectedButton = i;
						buttonSelected = true;
					}
					else
						button->select();

					buttonSelected = true;
				}
				else
					button->deselect();
				i++;
			}
			if (!buttonSelected)
				currentlySelectedButton = -1;
		}

		// update fullscreen text
		std::wostringstream fullscreenText;
		fullscreenText << L"fullscreen     " << std::boolalpha << fullscreen;
		result = d2d->createTextLayoutFromWStringStream(&fullscreenText, textFormat.Get(), (float)dxApp->getCurrentWidth(), 100, fullscreenLayout);
		if (!result.isValid())
			return result;

		// update screen resolution text
		std::wostringstream screenResolutionText;
		screenResolutionText << L"resolution\t   " << supportedModes[currentModeIndex].Width << " x " << supportedModes[currentModeIndex].Height << " @ " << supportedModes[currentModeIndex].RefreshRate.Numerator / supportedModes[currentModeIndex].RefreshRate.Denominator << " Hz";
		result = d2d->createTextLayoutFromWStringStream(&screenResolutionText, textFormat.Get(), (float)dxApp->getCurrentWidth(), 100, resolutionLayout);
		if (!result.isValid())
			return result;

		for (auto button : menuButtons)
			button->update(deltaTime);

		if (currentModeIndex == 0)
		{
			menuButtons[1]->lock();
			if (currentlySelectedButton == 1)
				currentlySelectedButton = -1;
		}
		if (currentModeIndex == nSupportedModes - 1)
		{
			menuButtons[2]->lock();
			if (currentlySelectedButton == 2)
				currentlySelectedButton = -1;
		}

		// lock resolution buttons for now as we do not have resizable graphics yet
		menuButtons[1]->lock();
		menuButtons[2]->lock();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::render(const double /*farSeer*/)
	{
		// error handling
		util::Expected<void> result;

		if (!isPaused)
		{
			d2d->printText(0, 50, titleLayout.Get());// , 0, -300);

			d2d->printCenteredText(fullscreenLayout.Get(), -50, -130);
			menuButtons[0]->drawCentered(1, 0, -180);

			d2d->printCenteredText(resolutionLayout.Get(), -95, -38);
			menuButtons[1]->drawCentered(1, -50, -90);
			menuButtons[2]->drawCentered(1, 50, -90);

			menuButtons[3]->drawCentered(2, 0, 200);
			menuButtons[4]->drawCentered(2, -300, 300);
			menuButtons[5]->drawCentered(2, 300, 300);

			// print FPS information
			d2d->printFPS();
		}
		menuButtons[3]->drawCentered(2, 0, 200);

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::shutdown()
	{
		ShowCursor(false);
		this->isPaused = true;

		// delete buttons
		for (auto button : menuButtons)
			delete button;
		menuButtons.clear();

		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);

		// return success
		return {};
	}
}