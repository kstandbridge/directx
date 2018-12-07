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

// bell0bytes graphics
#include "d3d.h"
#include "d2d.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp
	{
	private:
		// folder paths
		std::wstring pathToMyDocuments;			// path to the My Documents folder
		std::wstring pathToLogFiles;			// path to the folder containing log files
		std::wstring pathToConfigurationFiles;	// path to the folder containing the configuration files

		// configuration file names
		std::wstring prefFile;					// configuration file specifying screen resolution preferences

		// game states
		bool validConfigurationFile;			// true iff there was a valid configuration file at startup
		bool activeFileLogger;					// true iff the logging service was successfully registered
		bool hasStarted;						// true iff the DirectXApp was started completely

		// game options
		bool showFPS;							// true if and only if FPS information should be printed to the screen. Default: true; can be toggled via F1

		// timer
		Timer* timer;							// high-precision timer
		const double dt;						// constant game update rate
		const double maxSkipFrames;				// constant maximum of frames to skip in the update loop (important to not stall the system on slower computers)

		util::Expected<void> calculateFrameStatistics();		// computes frame statistics

		// helper functions
		bool getPathToMyDocuments();			// stores the path to the My Documents folder in the appropriate member variable
		void createLoggingService();			// creates the file logger and registers it as a service
		bool checkConfigurationFile();			// checks for valid configuration file

	protected:
		// application window
		const HINSTANCE appInstance;			// handle to an instance of the application
		const Window* appWindow;				// the application window (i.e. game window)

		// game state
		bool isPaused;							// true iff the game is paused

		// stats
		int fps;								// frames per second
		double mspf;							// milliseconds per frame

		// DirectX Graphics
		graphics::Direct3D* d3d;				// pointer to the Direct3D class
		graphics::Direct2D* d2d;				// pointer to the Direct2D class

		// constructor and destructor
		DirectXApp(HINSTANCE hInstance);
		~DirectXApp();

		// initialization and shutdown
		virtual util::Expected<void> init();								// initializes the DirectX application
		virtual void shutdown(util::Expected<void>* expected = NULL);		// clean up and shutdown the DirectX application

		// acquire user input
		virtual void onKeyDown(WPARAM wParam, LPARAM lParam);				// handles keyboard input

		// game loop
		virtual util::Expected<int> run();									// enters the main event loop
		virtual util::Expected<int> update(double dt) = 0;					// update the game world

		// resize functions
		virtual util::Expected<void> onResize();							// resize game graphics

		// generating output
		virtual util::Expected<int> render(double farseer) = 0;				// renders the game world

		// getters
		bool fileLoggerIsActive() const { return activeFileLogger; }		// returns true iff the file logger is active

	public:
		friend class Window;
		friend class graphics::Direct3D;
		friend class graphics::Direct2D;
	};
}