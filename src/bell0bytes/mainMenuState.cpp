// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "app.h"
#include "coreComponent.h"
#include "window.h"

// bell0bytes UI
#include "mainMenuState.h"
#include "playState.h"
#include "optionsMenuState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputComponent.h"
#include "inputHandler.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponentWrite.h"
#include "sprites.h"
#include "buttons.h"

// bell0bytes file system
#include "fileSystemComponent.h"

// bell0bytes audio
#include "audioComponent.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	MainMenuState::MainMenuState(core::DirectXApp& app, const std::wstring& name) : GameState(app, name), currentlySelectedButton(0)
	{ }

	MainMenuState::~MainMenuState()
	{ }

	MainMenuState& MainMenuState::createInstance(core::DirectXApp& app, const std::wstring& stateName)
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
		
		// position mouse at the center of the screen
		if (!SetCursorPos(dxApp.getGraphicsComponent().getCurrentWidth() / 2, dxApp.getGraphicsComponent().getCurrentHeight() / 2))
			return std::runtime_error("Critical error: Unable to set cursor position!");

		// hide the standard cursor
		ShowCursor(false);

		// allow mouse and keyboard input
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;

		if (firstCreation)
		{
			// create text format
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Lucida Handwriting", 128.0f, DWRITE_TEXT_ALIGNMENT_CENTER, mainMenuFormat);
			if (!result.isValid())
				return result;

			// create text layout
			std::wstring menu = L"Main Menu";
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWString(menu, mainMenuFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 200, mainMenuLayout);
			if (!result.isValid())
				return result;

			// load the button sound
			buttonSound = new audio::SoundEvent();
			result = dxApp.getAudioComponent().loadFile(dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Sounds, L"button.wav"), *buttonSound, audio::AudioTypes::Sound);
			if (!result.isValid())
				return result;

			// load the menu music
			menuMusic = new audio::StreamEvent(dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Music, L"menuMusic.mp3"), true, audio::AudioTypes::Music);
		}
		
		// create buttons
		if (!initializeButtons().wasSuccessful())
			return std::runtime_error("Critical error: Unable to initialize menu buttons!");
		
		if (!musicIsPlaying)
		{
			// send depesche to play music
			core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::BeginStream, menuMusic);
			dxApp.addMessage(depesche);
		}
		musicIsPlaying = true;
		
		// do not initialize the text layouts again
		firstCreation = false;

		// return success
		return { };
	}

	util::Expected<void> MainMenuState::initializeButtons()
	{
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// Play Button //////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

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
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonPlay.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// set lambda function
		auto onClick = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonSound);
			Sleep(120);

			core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::EndStream, nullptr);
			dxApp.addMessage(depesche);
			musicIsPlaying = false;

			if (!dxApp.changeGameState(&game::PlayState::createInstance(dxApp, L"Game")).wasSuccessful())
				return std::runtime_error("Critical error: Unable to change to the main game state!");
			return { };
		};

		// add button to the list
		try{ menuButtons.push_back(new AnimatedButton(L"Play Button", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClick));}
		catch (std::exception&e) { return e; }

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

		// create button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonOptions.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// set lambda function
		auto onClickOptions = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonSound);
			Sleep(120);

			if (!dxApp.changeGameState(&UI::OptionsMenuState::createInstance(dxApp, L"Options Menu")).wasSuccessful())
				return std::runtime_error("Critical error: Unable to change to the options menu state!");
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Options", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickOptions)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);
		
		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// Back Button //////////////////////////////////////////
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
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonQuit.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// set lambda function
		auto onClickQuit = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonSound);
			Sleep(120);

			this->releaseAudio();

			PostMessage(dxApp.getCoreComponent().getWindow().getMainWindowHandle(), WM_CLOSE, 0, 0);
			if (!shutdown().wasSuccessful())
				return std::runtime_error("Critical error: Unable to shut down the main menu!");
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Quit Button", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickQuit)); }
		catch (std::exception& e) { return e; }

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
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;

		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::onMessage(const core::Depesche& depesche)
	{
		input::InputHandler* ih = (input::InputHandler*)depesche.sender;

		if (!isPaused)
			if (!ih->isListening())
				return handleInput(ih->activeKeyMap);

		// return success
		return { };
	}

	util::Expected<void> MainMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::Select:
				// activate currently selected button
				return menuButtons[currentlySelectedButton]->click();

			case input::MoveDown:
				// select next button in the list
				menuButtons[currentlySelectedButton]->deselect();
				if (currentlySelectedButton < menuButtons.size() - 1)
					currentlySelectedButton++;
				else
					currentlySelectedButton = 0;
				menuButtons[currentlySelectedButton]->select();
				break;

			case input::MoveUp:
				// select next button in the list
				menuButtons[currentlySelectedButton]->deselect();
				if (currentlySelectedButton > 0)
					currentlySelectedButton--;
				else
					currentlySelectedButton = (unsigned int)menuButtons.size()-1;
				menuButtons[currentlySelectedButton]->select();
				break;

			case input::GameCommands::ShowFPS:
				dxApp.toggleFPS();
				break;

			case input::GameCommands::Back:
				currentlySelectedButton = 2;
				return menuButtons[currentlySelectedButton]->click();
			}
		}

		// notify that the stack was not changed
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::update(const double deltaTime)
	{
		if (isPaused)
			return { };

		if (dxApp.getInputComponent().getInputHandler().activeMouse)
		{
			// get mouse position
			long mouseX = dxApp.getInputComponent().getInputHandler().getMouseX();
			long mouseY = dxApp.getInputComponent().getInputHandler().getMouseY();

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
		// print title
		dxApp.getGraphicsComponent().getWriteComponent().printText(0, 100, mainMenuLayout.Get());

		// draw buttons
		unsigned int i = 0;
		unsigned int offset = 80;
		for (auto button : menuButtons)
		{
			button->drawCentered(1, 0, 100+((float)i*offset+5));
			i++;
		}

		// print FPS information
		dxApp.getGraphicsComponent().getWriteComponent().printFPS();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> MainMenuState::shutdown()
	{
		ShowCursor(false);
		this->isPaused = true;

		// delete buttons
		for (auto button : menuButtons)
			delete button;
		menuButtons.clear();
		currentlySelectedButton = 0;
				
		// return success
		return { };
	}

	void MainMenuState::releaseAudio()
	{
		// stop button sound
		if (buttonSound)
			delete buttonSound;

		// stop menu music
		core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::EndStream, nullptr);
		dxApp.addMessage(depesche);
		musicIsPlaying = false;

		if (menuMusic)
			delete menuMusic;
	}
}