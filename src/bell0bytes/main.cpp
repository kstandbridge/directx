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
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows includes
#include <windows.h>

// Lua and Sol
#pragma comment(lib, "liblua53.a")

// exceptions
#include <exception>
#include <stdexcept>

// bell0bytes core
#include "app.h"

// bell0ybtes util
#include "expected.h"							// error handling with "expected"
#include "serviceLocator.h"						// enables global access to services

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// CLASSES //////////////////////////////////////////////////////////////////////////////

// the core game class, derived from DirectXApp
class DirectXGame : core::DirectXApp
{
public:
	// constructor and destructor
	DirectXGame(HINSTANCE hInstance);
	~DirectXGame();

	// override virtual functions
	util::Expected<void> init() override;								// game initialization
	void shutdown(util::Expected<void>* expected = NULL) override;		// cleans up and shuts the game down (handles errors)
	void onResize() override;											// resize game graphics

	// run the game
	util::Expected<int> run() override;
};

// FUNCTIONS ////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// WinMain //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
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
		game.shutdown(&(util::Expected<void>)returnValue);

		// gracefully return
		return returnValue.get();
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
////////////////////////////// Game Functions ///////////////////////////////////////////
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

	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game initialization was successful.");
	return {};
}

// run the game
util::Expected<int> DirectXGame::run()
{
	// run the core DirectX application
	return DirectXApp::run();
}

// resize graphics
void DirectXGame::onResize()
{
	// call base method
	return DirectXApp::onResize();
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
			if (DirectXApp::activeLogger)
			{
				std::stringstream errorMessage;
				errorMessage << "The game is shutting down with a critical error: " << e.what();
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));
			}
			return;
		}
	}

	// no error: clean up and shut down normally
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The game was shut down successfully.");
}