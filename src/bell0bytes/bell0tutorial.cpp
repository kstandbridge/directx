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
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows includes
#include <windows.h>
#include <wincodec.h>	// Windows Imaging Component

// exceptions
#include <exception>
#include <stdexcept>

// bell0bytes core
#include "app.h"

// bell0ybtes util
#include "expected.h"							// error handling with "expected"
#include "serviceLocator.h"						// enables global access to services

// bell0bytes graphics
#include "graphicsHelper.h"
#include "sprites.h"

#include <d2d1effects.h>
#pragma comment (lib, "dxguid.lib")


// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// CLASSES //////////////////////////////////////////////////////////////////////////////

// the core game class, derived from DirectXApp
class DirectXGame : core::DirectXApp
{
private:
	graphics::AnimationData* wolfAnimations;
	graphics::AnimatedSprite* wolf;
	
public:
	// constructor and destructor
	DirectXGame(HINSTANCE hInstance);
	~DirectXGame();

	// override virtual functions
	util::Expected<void> init() override;								// game initialization
	void shutdown(util::Expected<void>* expected = NULL) override;		// cleans up and shuts the game down (handles errors)
	util::Expected<int> update(double dt);								// update the game world
	util::Expected<int> render(double farSeer);							// render the scene
	util::Expected<void> onResize();									// resize the game graphics

	// create graphics
	util::Expected<void> initGraphics();								// initializes graphics

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
	util::Expected<void> gameInitialization = game.init();

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
DirectXGame::DirectXGame(HINSTANCE hInstance) : DirectXApp(hInstance)
{ }
DirectXGame::~DirectXGame()
{ }

// initialize the game
util::Expected<void> DirectXGame::init()
{
	// initialize the core DirectX application
	util::Expected<void> applicationInitialization = DirectXApp::init();
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// initialize game graphics
	applicationInitialization = initGraphics();
	if(!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game initialization was successful.");
	return {};
}

// initialize graphics
util::Expected<void> DirectXGame::initGraphics()
{	
	std::vector<graphics::AnimationCycleData> wolfAnimationsCycles;
	graphics::AnimationCycleData cycle;
	
	// wolf on attention cycle
	cycle.name = L"Wolf Running";
	cycle.startFrame = 0;
	cycle.numberOfFrames = 5;
	cycle.width = 25;
	cycle.height = 64;
	cycle.paddingWidth = 0;
	cycle.paddingHeight = 1;
	cycle.borderPaddingHeight = cycle.borderPaddingWidth = 1;
	cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
	wolfAnimationsCycles.push_back(cycle);

	// angry wolf animation cycle
	cycle.name = L"Wolf Attack";
	cycle.startFrame = 0;
	cycle.numberOfFrames = 5;
	cycle.width = 24;
	cycle.height = 62;
	cycle.paddingWidth = cycle.paddingHeight = 0;
	cycle.borderPaddingWidth = 1;
	cycle.borderPaddingHeight = 0;
	cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
	wolfAnimationsCycles.push_back(cycle);

	// create wolf animations
	try { wolfAnimations = new graphics::AnimationData(d2d, L"Art/wolfAnimations.png", wolfAnimationsCycles); }
	catch (std::runtime_error& e){ return e; }

	// create wolf
	wolf = new graphics::AnimatedSprite(d2d, wolfAnimations, 0, 24, 400, 300);

	wolfAnimationsCycles.clear();
	std::vector<graphics::AnimationCycleData>(wolfAnimationsCycles).swap(wolfAnimationsCycles);

	// return success
	return {};
}

// resize graphics
util::Expected<void> DirectXGame::onResize()
{
	// resize application data, Direct2D and Direct3d
	if (!DirectXApp::onResize().wasSuccessful())
		return std::runtime_error("Critical Error: Failed to resize game resources!");

	// return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>(std::stringstream("The game resources were resized succesfully!"));
	return { };
}


// run the game
util::Expected<int> DirectXGame::run()
{
	// run the core DirectX application
	return DirectXApp::run();
}

// shutdown
void DirectXGame::shutdown(util::Expected<void>* expected)
{
	// check for error message
	if (expected != NULL && !expected->isValid())
	{
		// the game was shutdown by an error
		// try to clean up and log the error message
		try
		{
			// do clean up

			// throw error
			expected->get();
		}
		catch (std::runtime_error& e)
		{
			// create and print error message string (if the logger is available)
			if (DirectXApp::fileLoggerIsActive())
			{
				std::stringstream errorMessage;
				errorMessage << "The game is shutting down with a critical error: " << e.what();
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));
			}
			return;
		}
	}

	delete wolf;
	delete wolfAnimations;


	// no error: clean up and shut down normally
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The game was shut down successfully.");
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Update ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<int> DirectXGame::update(double deltaTime)
{	
	// update the game world
	wolf->updateAnimation(deltaTime);

	// return success
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Render ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<int> DirectXGame::render(double /*farSeer*/)
{
	// clear the back buffer and the depth/stencil buffer
	d3d->clearBuffers();

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Direct2D /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	d2d->devCon->BeginDraw();

	d2d->matrixScaling = D2D1::Matrix3x2F::Scale(3, 3, D2D1::Point2F(400, 300));
	d2d->devCon->SetTransform(d2d->matrixScaling);

	wolf->draw();

	d2d->devCon->SetTransform(D2D1::Matrix3x2F::Identity());

	// print FPS information
	d2d->printFPS(d2d->blackBrush.Get());

	if(FAILED(d2d->devCon->EndDraw()))
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