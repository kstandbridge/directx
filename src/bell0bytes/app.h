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

		bool validConfigurationFile;			// true iff there was a valid configuration file at startup
		bool activeFileLogger;					// true iff the logging service was successfully registered
		bool hasStarted;						// true iff the DirectXApp was started completely

		// timer
		Timer* timer;							// high-precision timer
		int fps;								// frames per second
		double mspf;							// milliseconds per frame
		const double dt;						// constant game update rate
		const double maxSkipFrames;				// constant maximum of frames to skip in the update loop (important to not stall the system on slower computers)
												
		void calculateFrameStatistics();		// computes frame statistics

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

		// DirectX Graphics
		graphics::Direct3D* d3d;				// pointer to the Direct3D device

		// constructor and destructor
		DirectXApp(HINSTANCE hInstance);
		~DirectXApp();

		// initialization and shutdown
		virtual util::Expected<void> init();								// initializes the DirectX application
		virtual void shutdown(util::Expected<void>* expected = NULL);		// clean up and shutdown the DirectX application

		// acquire user input
		virtual void onKeyDown(WPARAM wParam, LPARAM lParam) const;			// handles keyboard input

		// game loop
		virtual util::Expected<int> run();		// enters the main event loop
		virtual void update(double dt);			// update the game world

		// resize functions
		virtual void onResize();				// resize game graphics

		// generating output
		virtual void render(double farseer);	// renders the game world

		// getters
		bool fileLoggerIsActive() const { return activeFileLogger; }		// returns true iff the file logger is active

	public:
		friend class Window;
	};
}