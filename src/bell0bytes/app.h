#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		08/03/2016 - Dortmund - Germany
*
* Desc:		main class to bring together all the core components of a game
*
* Hist:		- 01/06/18: fixed a memory leak in showFPS method (released text format)
*			- 03/06/18: now observes events from the Window and Direct3D classes
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "window.h"
#include "timer.h"

// bell0bytes util
#include "expected.h"
#include "observer.h"

// bell0bytes graphics
#include "d3d.h"
#include "d2d.h"


// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace input
{
	enum Events { StartApplication, PauseApplication, ResumeApplication, QuitApplication, SwitchFullscreen, WindowChanged, ChangeResolution };
}

namespace core
{
	class DirectXApp : public util::Observer
	{
	private:
		// game update variables
		const double dt;						// constant game update rate
		const double maxSkipFrames;				// constant maximum of frames to skip in the update loop (important to not stall the system on slower computers)
		
		// application window
		const HINSTANCE appInstance;			// handle to an instance of the application
		

		// timer
		Timer* timer;							// high-precision timer

		// folder paths
		std::wstring pathToMyDocuments;			// path to the My Documents folder
		std::wstring pathToLogFiles;			// path to the folder containing log files
		std::wstring pathToConfigurationFiles;	// path to the folder containing the configuration files

		// configuration file names
		const std::wstring prefFile;			// configuration file specifying screen resolution preferences
		
		bool validConfigurationFile;			// true iff there was a valid configuration file at startup
		bool activeFileLogger;					// true iff the logging service was successfully registered

		// game states
		bool hasStarted;						// true iff the DirectXApp was started completely
		
		// stats
		int fps;								// frames per second
		double mspf;							// milliseconds per frame

		util::Expected<void> calculateFrameStatistics();		// computes frame statistics

		// helper functions
		bool getPathToMyDocuments();			// stores the path to the My Documents folder in the appropriate member variable
		void createLoggingService();			// creates the file logger and registers it as a service
		bool checkConfigurationFile();			// checks for valid configuration file
		
		// pause and resume game on notification
		util::Expected<void> onNotify(const int event);
		
		// pause
		void pauseGame();
		void resumeGame(bool recreateGraphics = false, bool restartTimer = false);

		// check for fullscreen change
		util::Expected<void> checkFullscreen();

	protected:
		// application window
		const Window* appWindow;				// the application window (i.e. game window)

		// DirectX Graphics
		graphics::Direct3D* d3d;				// pointer to the Direct3D class
		graphics::Direct2D* d2d;				// pointer to the Direct2D class

		// options
		bool showFPS;							// true if and only if FPS information should be printed to the screen. Default: true; can be toggled via F1 (standard binding)

		// game states
		bool isPaused;							// true iff the game is paused
					
		// constructor and destructor
		DirectXApp(HINSTANCE hInstance);
		~DirectXApp();

		// initialization and shutdown
		virtual util::Expected<void> init(LPCWSTR windowTitle);				// initializes the DirectX application
		virtual void shutdown(const util::Expected<void>* const expected = NULL); // clean up and shutdown the DirectX application
				
		// user input
		virtual void acquireInput() = 0;

		// game loop
		virtual util::Expected<int> run();									// enters the main event loop
		virtual util::Expected<int> update(const double dt) = 0;			// update the game world

		// resize functions
		virtual util::Expected<void> onResize() const;						// resize game graphics

		// generating output
		virtual util::Expected<int> render(const double farseer) = 0;		// renders the game world
				
	public:
		// getters
		
		// application instance and main window
		const HINSTANCE& getApplicationInstance() const { return appInstance; };
		const HWND getMainWindow() const { return appWindow->getMainWindowHandle(); };
		
		// paths and configuration files
		const std::wstring& getPathToConfigurationFiles() const { return pathToConfigurationFiles; };
		const std::wstring& getPrefsFile() const { return prefFile; };
		bool hasValidConfigurationFile() const { return validConfigurationFile; };
		bool fileLoggerIsActive() const { return activeFileLogger; };

		// booleans
		bool showFramesPerSecond() const { return showFPS; };
		bool gameHasStarted() const { return hasStarted; };
	};
}