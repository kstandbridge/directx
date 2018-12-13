/****************************************************************************************
* Author:	Gilles Bellot
* Date:		29/06/2017 - Dortmund - Germany
*
* Desc:		Basic Windows Programming Tutorial
*
* History:	- 29/06/2017 - Hello World!
*			- 06/07/2017 - Expected!
*			- 07/07/2017 - A Thread-Safe Logger
*			- 08/07/2017 - Of the Moon and the Sun
*			- 08/07/2017 - Handling Important Events
*			- 09/07/2017 - Keeping Track of Time
*			- 10/07/2017 - The Game Loop
*			- 13/07/2017 - Keyboard and Mouse
*			- 13/07/2017 - Of Icons and Cursors
*			- 14/07/2017 - A First Framework
*			- 15/07/2017 - First Contact
*			- 25/07/2017 - The Swap Chain
*			- 31/07/2017 - Render Targets
*			- 05/08/2017 - Printing Text with DirectWrite
*			- 27/08/2017 - Of Shaders and Triangles
*			- 01/09/2017 - Among Colourful Stars
*			- 29/10/2017 - Going Fullscreen
*			- 20/05/2018 - Shader Effects
*			- 21/05/2018 - Introduction to Drawing with Direct2D
*			- 21/05/2018 - Fun with Brushes
*			- 22/05/2018 - Direct2D Geometries
*			- 23/05/2018 - Transformations
*			- 27/05/2018 - Bitmaps
*			- 28/05/2018 - Sprites
*			- 30/05/2018 - Animated Sprites
*			- 04/06/2018 - Input Handler + Keyboard
*			- 10/06/2018 - Mice
*			- 12/06/2018 - boost serialization
*			- 14/06/2018 - game states
*			- 21/06/2018 - game options (including new key bindings)
*			- 21/06/2018 - changed the stack to allow overlays
*			- 23/06/2018 - DirectInput support
*			- 24/06/2018 - XInput support
*			- 27/06/2018 - sliced the DirectXApp class into components
*
* ToDo:		- add physFS support
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "coreComponent.h"
#include "app.h"								// the main application app
#include "window.h"								// the core window class

// bell0bytes filesystem
#include "fileSystemComponent.h"				// file system input and output

// game states
#include "introState.h"							// first state to load

// bell0bytes graphics
#include "graphicsComponent.h"					// graphics component of the DirectXApp class
#include "graphicsComponent2D.h"				// 2D graphics
#include "graphicsComponent3D.h"				// 3D graphics
#include "sprites.h"							// sprites

// bell0bytes input
#include "inputComponent.h"
#include "inputHandler.h"						// the input handler class
#include "gameCommands.h"

// bell0bytes util
#include "serviceLocator.h"						// enables global access to services

// bell0bytes audio
#include "audioComponent.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// application
const std::wstring cCompanyName = L"bell0bytes";
const std::wstring cApplicationName = L"bell0tutorial";
const std::wstring cApplicationVersion = L"alpha 1.0";

// CLASSES //////////////////////////////////////////////////////////////////////////////

// the input handler derived from InputHandler class
class GameInput : public input::InputHandler
{
private:
	
protected:
	// initialization
	virtual void setDefaultKeyMap() override;

public:
	// constructor
	GameInput(core::DirectXApp& dxApp, const HINSTANCE& hInstance, const HWND& appWindow);
};

// the core game class, derived from DirectXApp
class DirectXGame : core::DirectXApp
{
private:
	// components
	graphics::GraphicsComponent2D* graphics2D;
	graphics::GraphicsComponent3D* graphics3D;

	// initialize mouse cursor sprite
	util::Expected<void> createMouseCursor();

public:
	// constructor and destructor
	DirectXGame();
	~DirectXGame();

	// game initialization
	util::Expected<void> init(const HINSTANCE& hInstance, LPCWSTR windowTitle, const std::wstring& manufacturerName, const std::wstring& applicationName, const std::wstring& applicationVersion) override;			// game initialization
	util::Expected<void> onResize() override;						// resize the game graphics
	util::Expected<void> initGraphics();								// initializes graphics

	// game shutdown
	void shutdown(const util::Expected<void>* const expected = NULL) override;	// cleans up and shuts the game down (handles errors)
	void releaseMemory();
	
	// game input
	GameInput* inputHandler;
	util::Expected<void> initializeInput(const HINSTANCE& hInstance, const HWND& appWindow) override;
	util::Expected<void> acquireInput() override;

	// game update
	util::Expected<int> update(const double dt) override;				// update the game world
	
	// render
	util::Expected<int> render(const double farSeer) override;			// render the scene

	// run the game
	util::Expected<int> run() override;
};

// FUNCTIONS ////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// WinMain //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	// create and initialize the game
	DirectXGame game;
	util::Expected<void> gameInitialization = game.init(hInstance, cApplicationName.c_str(), cCompanyName, cApplicationName, cApplicationVersion);

	// if the initialization was successful, run the game, else, try to clean up and exit the application
	if (gameInitialization.wasSuccessful())
	{
		// error handling
		util::Expected<int> returnValue(-1);

		// initialization was successful -> run the game
		try
		{
			returnValue = game.run();

			// clean up after the game has ended
			util::Expected<void> convertedReturnValue(returnValue);
			game.shutdown(&convertedReturnValue);
		}
		catch (util::Expected<void>& e)
		{
			game.shutdown(&e);
		}
		catch (std::exception& e)
		{
			util::Expected<void> error(e);
			game.shutdown(&error);
		}
		catch (...)
		{
			util::Expected<void> error(L"An unknown error occured!");
			game.shutdown(&error);
		}

		// return
		if (returnValue.isValid())
			return returnValue.get();
		else
			return -1;
	}
	else
	{
		// a critical error occured during initialization, try to clean up and to print information about the error
		game.shutdown(&gameInitialization);
		
		// humbly return with an error
		return -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Game Initialization //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// constructor and destructor
DirectXGame::DirectXGame() : DirectXApp()
{ }

// initialize the game
util::Expected<void> DirectXGame::init(const HINSTANCE& hInstance, LPCWSTR windowTitle, const std::wstring& manufacturerName, const std::wstring& applicationName, const std::wstring& applicationVersion)
{
	// initialize the core DirectX application
	util::Expected<void> applicationInitialization = DirectXApp::init(hInstance, windowTitle, manufacturerName, applicationName, applicationVersion);
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// set the components
	graphics2D = &graphicsComponent->get2DComponent();
	graphics3D = &graphicsComponent->get3DComponent();
	if (!graphics2D || !graphics3D)
		return std::runtime_error("Critical error: graphics component not valid!");

	// initialize the input handler
	applicationInitialization = initializeInput(hInstance, coreComponent->getWindow().getMainWindowHandle());
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// initialize main game graphics
	applicationInitialization = initGraphics();
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// initialize the first game state
	applicationInitialization = pushGameState(&UI::IntroState::createInstance(*this, L"Intro"));
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game initialization was successful.");
	return {};
}

// initialize graphics
util::Expected<void> DirectXGame::initGraphics()
{	
	// error handling
	util::Expected<void> result;

	// create mouse cursor sprite
	result = createMouseCursor();
	if (!result.isValid())
		return result;

	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game graphics were successfully initialized.");
	return {};
}

// resize graphics
util::Expected<void> DirectXGame::onResize()
{
	// error handling
	util::Expected<void> result;

	// resize application data, Direct2D and Direct3D
	result = DirectXApp::onResize();
	if (!result.isValid())
		return result;

	// resize graphics of the current state

	// return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>(std::stringstream("The game resources were resized succesfully."));
	return { };
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Start and End ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// run the game
util::Expected<int> DirectXGame::run()
{
	// run the core DirectX application
	util::Expected<int> result(-1);
	
	try { result = DirectXApp::run(); }
	catch (util::Expected<void>& e) { throw std::move(e); }
	catch (std::exception& e) { throw e; }
	catch (...) { throw L"Unknown error!"; }

	return result;

}

// shutdown
void DirectXGame::shutdown(const util::Expected<void>* const expected)
{
	// check for error message
	if (expected != NULL && !expected->isValid())
	{
		// the game was shutdown by an error
		// try to clean up and log the error message
		try
		{
			// do clean up
			releaseMemory();

			// throw error
			expected->get();
		}
		catch (std::runtime_error& e)
		{
			// create and print error message string (if the logger is available)
			if (fileSystemComponent->fileLoggerIsActive())
			{
				std::stringstream errorMessage;
				errorMessage << "Shutdown! " << e.what();
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));
			}
			return;
		}
		catch (...)
		{
			// create and print error message string (if the logger is available)
			if (fileSystemComponent->fileLoggerIsActive())
			{
				std::stringstream errorMessage;
				errorMessage << "The game is shutting down with a critical error!";
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));
			}
			return;
		}
	}

	// no error: clean up and shut down normally
	releaseMemory();
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The game was shut down successfully.");
}
void DirectXGame::releaseMemory()
{
	// release the input handler
	delete inputHandler;
}
DirectXGame::~DirectXGame()
{ }

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Input Handler ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
GameInput::GameInput(core::DirectXApp& dxApp, const HINSTANCE& hInstance, const HWND& appWindow) : input::InputHandler(dxApp, hInstance, appWindow, dxApp.getFileSystemComponent().getKeyboardFile(), dxApp.getFileSystemComponent().getJoystickFile(), dxApp.getFileSystemComponent().getGamepadFile())
{
	// load default key bindings
	loadGameCommands();
}
void GameInput::setDefaultKeyMap()
{
	std::vector<input::BindInfo> bi;
	bi.push_back(input::BindInfo(VK_SHIFT, input::KeyState::StillPressed));
	bi.push_back(input::BindInfo(VK_CONTROL, input::KeyState::StillPressed));
	bi.push_back(input::BindInfo('F', input::KeyState::JustPressed));

	if (activeGamepad)
	{
		// gamepad input
		keyMapGamepad.clear();

		// offset
		unsigned int offset = 293;

		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveLeft, new input::GameCommand(L"Move Left", XINPUT_GAMEPAD_DPAD_LEFT + offset, input::KeyState::JustPressed)));
		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveRight, new input::GameCommand(L"Move Right", XINPUT_GAMEPAD_DPAD_RIGHT + offset, input::KeyState::JustPressed)));
		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveUp, new input::GameCommand(L"Move Up", XINPUT_GAMEPAD_DPAD_UP + offset, input::KeyState::JustPressed)));
		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveDown, new input::GameCommand(L"Move Right", XINPUT_GAMEPAD_DPAD_DOWN + offset, input::KeyState::JustPressed)));

		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Select, new input::GameCommand(L"Select", XINPUT_GAMEPAD_A + offset, input::KeyState::JustPressed)));
		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Back, new input::GameCommand(L"Back", XINPUT_GAMEPAD_B + offset, input::KeyState::JustPressed)));
		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Select, new input::GameCommand(L"Select", VK_LBUTTON, input::KeyState::JustPressed)));
		keyMapGamepad.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::ShowFPS, new input::GameCommand(L"Show FPS", bi)));
	}
	else if (activeJoystick)
	{
		// joystick input
		keyMapJoystick.clear();

		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveLeft, new input::GameCommand(L"Move Left", 256, input::KeyState::JustPressed)));
		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveRight, new input::GameCommand(L"Move Right", 257, input::KeyState::JustPressed)));
		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveUp, new input::GameCommand(L"Move Up", 258, input::KeyState::JustPressed)));
		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveDown, new input::GameCommand(L"Move Right", 259, input::KeyState::JustPressed)));

		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Select, new input::GameCommand(L"Select", 263, input::KeyState::JustPressed)));
		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Select, new input::GameCommand(L"Select", VK_LBUTTON, input::KeyState::JustPressed)));
		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Back, new input::GameCommand(L"Back", 262, input::KeyState::JustPressed)));
		keyMapJoystick.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::ShowFPS, new input::GameCommand(L"Show FPS", bi)));
	}
	else
	{
		// keyboard input
		keyMapKeyboard.clear();

		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::ShowFPS, new input::GameCommand(L"Show FPS", bi)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Back, new input::GameCommand(L"Back", VK_ESCAPE, input::KeyState::JustPressed)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Select, new input::GameCommand(L"Select", VK_RETURN, input::KeyState::JustPressed)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::Select, new input::GameCommand(L"Select", VK_LBUTTON, input::KeyState::JustPressed)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveLeft, new input::GameCommand(L"MoveLeft", VK_LEFT, input::KeyState::JustPressed)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveRight, new input::GameCommand(L"MoveRight", VK_RIGHT, input::KeyState::JustPressed)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveUp, new input::GameCommand(L"MoveUp", VK_UP, input::KeyState::JustPressed)));
		keyMapKeyboard.insert(std::pair<input::GameCommands, input::GameCommand*>(input::GameCommands::MoveDown, new input::GameCommand(L"MoveDown", VK_DOWN, input::KeyState::JustPressed)));
	}
}
util::Expected<void> DirectXGame::initializeInput(const HINSTANCE& hInstance, const HWND& appWindow)
{
	try 
	{	
		inputHandler = new GameInput(*this, hInstance, appWindow);
		inputComponent = new input::InputComponent(*inputHandler);
		
	}
	catch (std::runtime_error& e) { return e; }

	// return success
	return { };
}


/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Update ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<void> DirectXGame::acquireInput()
{
	util::Expected<void> acquiringInput;
	acquiringInput = inputHandler->acquireInput();
	if (!acquiringInput.wasSuccessful())
		return acquiringInput;
	else
		// return success
		return { };
}
util::Expected<int> DirectXGame::update(const double deltaTime)
{	
	// handle errors
	util::Expected<void> voidResult;

	// ignore empty state
	if (gameStates.empty())
		return 0;

	// update all active game scenes
	for (std::deque<core::GameState*>::reverse_iterator it = gameStates.rbegin(); it != gameStates.rend(); it++)
	{
		voidResult = (*it)->update(deltaTime);
		if (!voidResult.isValid())
			throw voidResult;

		if (stateStackChanged)
		{
			stateStackChanged = false;
			break;
		}
	}

	// update the mouse cursor
	inputHandler->updateMouseCursorAnimation(deltaTime);

	// return success
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Render ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<int> DirectXGame::render(const double farSeer)
{
	// clear the back buffer and the depth/stencil buffer
	graphics3D->clearBuffers();

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Direct2D /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	graphics2D->beginDraw();

	// render all active states from bottom to top
	for(auto state : gameStates)
		if(!state->render(farSeer).wasSuccessful())
			return std::runtime_error("Critical error: Unable to render scene!");

	// draw cursor (if active)
	if(inputHandler->activeMouse)
		inputHandler->drawMouseCursor();

	if(!graphics2D->endDraw().wasSuccessful())
		return std::runtime_error("Failed to draw 2D graphics!");

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Direct3D /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	
	// present the scene
	if (!graphics3D->present().wasSuccessful())
		return std::runtime_error("Failed to present the scene!");

	// return success
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Mouse Cursor //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<void> DirectXGame::createMouseCursor()
{
	// set cursor animation data
	std::vector<graphics::AnimationCycleData> cursorAnimationsCycles;
	graphics::AnimationCycleData cycle;
	graphics::AnimationData* cursorAnimations;

	// cursor cycle
	cycle.name = L"Cursor Normal";
	cycle.startFrame = 0;
	cycle.numberOfFrames = 1;
	cycle.width = 15;
	cycle.height = 16;
	cycle.paddingWidth = 0;
	cycle.paddingHeight = 3;
	cycle.borderPaddingHeight = cycle.borderPaddingWidth = 1;
	cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
	cursorAnimationsCycles.push_back(cycle);

	cycle.name = L"Cursor Click";
	cycle.startFrame = 0;
	cycle.numberOfFrames = 1;
	cycle.width = 15;
	cycle.height = 16;
	cycle.paddingWidth = 0;
	cycle.paddingHeight = 0;
	cycle.borderPaddingHeight = cycle.borderPaddingWidth = 1;
	cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
	cursorAnimationsCycles.push_back(cycle);

	// create cursor animations
	try { cursorAnimations = new graphics::AnimationData(graphicsComponent->get2DComponent().getD2D(), fileSystemComponent->openFile(fileSystem::DataFolders::Cursors, L"cursorHand.png").c_str(), cursorAnimationsCycles); }
	catch (std::exception& e) { return e; }

	// create cursor sprite
	try { inputHandler->setMouseCursor(new graphics::AnimatedSprite(graphicsComponent->get2DComponent().getD2D(), cursorAnimations, 0, 24, 0, 0)); }
	catch (std::exception& e) { return e; }

	cursorAnimationsCycles.clear();
	std::vector<graphics::AnimationCycleData>(cursorAnimationsCycles).swap(cursorAnimationsCycles);

	// hide the standard cursor
	ShowCursor(false);

	// return success
	return { };
}
