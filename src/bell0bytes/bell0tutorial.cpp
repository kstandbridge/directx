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
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "app.h"

// bell0ybtes util
#include "expected.h"							// error handling with "expected"
#include "serviceLocator.h"						// enables global access to services

// bell0bytes input
#include "inputHandler.h"

// bell0bytes graphics
#include "sprites.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// define game commands
namespace input
{
	enum GameCommands { Quit, ShowFPS };
}

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
	GameInput(const std::wstring& keyBindingsFile);
};

// the core game class, derived from DirectXApp
class DirectXGame : core::DirectXApp
{
private:
	// initialize mouse cursor sprite
	util::Expected<void> createMouseCursor();

public:
	// constructor and destructor
	DirectXGame(HINSTANCE hInstance);
	~DirectXGame();

	// game initialization
	util::Expected<void> init(LPCWSTR windowTitle) override;			// game initialization
	util::Expected<void> onResize() const override;						// resize the game graphics
	util::Expected<void> initGraphics();								// initializes graphics

	// game shutdown
	void shutdown(const util::Expected<void>* const expected = NULL) override;// cleans up and shuts the game down (handles errors)
	void releaseMemory();
	
	// game input
	GameInput* inputHandler;
	void acquireInput() override;

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
	DirectXGame game(hInstance);
	util::Expected<void> gameInitialization = game.init(L"bell0tutorial");

	// if the initialization was successful, run the game, else, try to clean up and exit the application
	if (gameInitialization.wasSuccessful())
	{
		// initialization was successful -> run the game
		util::Expected<int> returnValue = game.run();

		// clean up after the game has ended
		util::Expected<void> convertedReturnValue(returnValue);
		game.shutdown(&convertedReturnValue);

		// gracefully return
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
DirectXGame::DirectXGame(HINSTANCE hInstance) : DirectXApp(hInstance, L"bell0tutorial", L"alpha 1.0")
{ }

// initialize the game
util::Expected<void> DirectXGame::init(LPCWSTR windowTitle)
{
	// initialize the core DirectX application
	util::Expected<void> applicationInitialization = DirectXApp::init(windowTitle);
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// initialize the input handler
	try { inputHandler = new GameInput(this->keyBindingsFile); }
	catch (...) { return std::runtime_error("Critical error: Unable to create the input handler!"); }

	// initialize game graphics
	applicationInitialization = initGraphics();
	if(!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// position mouse at the center of the screen
	SetCursorPos(d3d->getCurrentWidth() / 2, d3d->getCurrentHeight() / 2);

	// hide the standard cursor
	ShowCursor(false);
		
	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game initialization was successful.");
	return {};
}

// initialize graphics
util::Expected<void> DirectXGame::initGraphics()
{	
	// create mouse cursor sprite
	if (!createMouseCursor().wasSuccessful())
		return std::runtime_error("Critical error: unable to create mouse cursor sprite!");

	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game graphics were successfully initialized.");
	return {};
}

// resize graphics
util::Expected<void> DirectXGame::onResize() const
{
	// resize application data, Direct2D and Direct3d
	if (!DirectXApp::onResize().wasSuccessful())
		return std::runtime_error("Critical Error: Failed to resize game resources!");

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
	return DirectXApp::run();
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
			if (DirectXApp::fileLoggerIsActive())
			{
				std::stringstream errorMessage;
				errorMessage << "Shutdow! " << e.what();
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));
			}
			return;
		}
		catch (...)
		{
			// create and print error message string (if the logger is available)
			if (DirectXApp::fileLoggerIsActive())
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
GameInput::GameInput(const std::wstring& keyBindingsFile) : input::InputHandler(keyBindingsFile)
{
	// load default key bindings
	loadGameCommands();
}

void GameInput::setDefaultKeyMap()
{
	keyMap.clear();

	std::vector<input::BindInfo> bi;
	bi.push_back(input::BindInfo(VK_SHIFT, input::KeyState::StillPressed));
	bi.push_back(input::BindInfo(VK_CONTROL, input::KeyState::StillPressed));
	bi.push_back(input::BindInfo('F', input::KeyState::JustPressed));

	keyMap[input::GameCommands::ShowFPS] = new input::GameCommand(L"Show FPS", bi);
	keyMap[input::GameCommands::Quit] = new input::GameCommand(L"Quit", VK_ESCAPE, input::KeyState::JustPressed);
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Update ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void DirectXGame::acquireInput()
{
	inputHandler->acquireInput();

	// act on user input
	for (auto x : inputHandler->activeKeyMap)
	{
		switch (x.first)
		{
		case input::GameCommands::Quit:
			PostMessage(appWindow->getMainWindowHandle(), WM_CLOSE, 0, 0);
			break;

		case input::GameCommands::ShowFPS:
			showFPS = !showFPS;
			break;
		}
	}
}

util::Expected<int> DirectXGame::update(const double deltaTime)
{	
	// update the mouse cursor
	inputHandler->updateMouseCursorAnimation(deltaTime);

	// update the game world

	// return success
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Render ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<int> DirectXGame::render(const double /*farSeer*/)
{
	// clear the back buffer and the depth/stencil buffer
	d3d->clearBuffers();

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Direct2D /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	d2d->beginDraw();

	// print FPS information
	d2d->printFPS();

	// draw cursor
	inputHandler->drawMouseCursor();

	if(!d2d->endDraw().wasSuccessful())
		return std::runtime_error("Failed to draw 2D graphics!");

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Direct3D /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	
	// present the scene
	if (!d3d->present().wasSuccessful())
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
	try { cursorAnimations = new graphics::AnimationData(d2d, L"Art/cursor.png", cursorAnimationsCycles); }
	catch (std::runtime_error& e) { return e; }

	// create cursor sprite
	inputHandler->setMouseCursor(new graphics::AnimatedSprite(d2d, cursorAnimations, 0, 24, 0, 0));

	cursorAnimationsCycles.clear();
	std::vector<graphics::AnimationCycleData>(cursorAnimationsCycles).swap(cursorAnimationsCycles);

	// return success
	return { };
}