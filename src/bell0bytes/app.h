#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		08/03/2016 - Dortmund - Germany
*
* Desc:		main class to bring together all the core components of a game
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows
#include <Windows.h>

// bell0bytes core
#include "window.h"
#include "timer.h"

// bell0bytes util
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp
	{
	protected:
		// application window
		HINSTANCE appInstance;					// handle to an instance of the application
		Window* appWindow;						// the application window (i.e. game window)

		// folder paths
		std::wstring pathToMyDocuments;			// path to the My Documents folder
		std::wstring pathToLogFiles;			// path to the folder containing log files
		std::wstring pathToConfigurationFiles;	// path to the folder containing the configuration files
		bool validConfigurationFile;			// true iff there was a valid configuration file at startup

		// logger state
		bool activeLogger;						// true iff the logging service was successfully registered

		// game state
		bool isPaused;							// true iff the game is paused 
		bool hasStarted;						// true iff this class has been fully initialized

		// timer
		Timer* timer;							// high-precision timer
		int fps;								// frames per second
		double mspf;							// milliseconds per frame

		// constructor and destructor
		DirectXApp(HINSTANCE hInstance);
		~DirectXApp();

		// timer functions
		void calculateFrameStatistics();

		// initialization and shutdown
		virtual util::Expected<void> init();								// initializes the DirectX application
		virtual void shutdown(util::Expected<void>* expected = NULL);		// clean up and shutdown the DirectX application

		// game loop
		virtual util::Expected<int> run();		// enters the main event loop
		void update(double deltaTime);			// update the game world

		// resize functions
		virtual void onResize();				// resize game graphics

		// helper functions
		bool getPathToMyDocuments();			// stores the path to the My Documents folder in the appropriate member variable
		void createLoggingService();			// creates the file logger and registers it as a service
		bool checkConfigurationFile();			// checks for valid configuration file

	public:
		friend class Window;
	};
}