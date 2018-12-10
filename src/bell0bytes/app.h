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

// c++ includes
#include <deque>

// bell0bytes core
#include "window.h"
#include "timer.h"
#include "states.h"

// bell0bytes util
#include "expected.h"
#include "observer.h"

// bell0bytes graphics
#include "d3d.h"
#include "d2d.h"


// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace input
{
	class InputHandler;
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
		
		// timer
		Timer* timer;							// high-precision timer

		// folder paths (documents)
		std::wstring pathToMyDocuments;				// path to the My Documents folder
		std::wstring pathToLogFiles;				// path to the folder containing log files
		std::wstring pathToUserConfigurationFiles;	// path to the folder containing the configuration files visible to the user

		// folder paths (application)
		std::wstring pathToLocalAppData;		// data bound to the user, the machine and the application (FOLDERID_LocalAppData)
		std::wstring pathToRoamingAppData;		// data bound to the user and the application (FOLDERID_RoamingAppData)
		std::wstring pathToProgramData;			// data bound to the machine and the application (FOLDERID_ProgramData)

		// application data subfolders
		const std::wstring manufacturerName;	// the manufacturer, i.e. bell0bytes
		const std::wstring applicationName;		// the game name, i.e. Tetris
		const std::wstring applicationVersion;	// the version number of the application, i.e. 0.1

		// configuration file names
		const std::wstring userPrefFile;		// configuration file editable by the user
		
		bool validUserConfigurationFile;		// true iff there was a valid user configuration file at startup
		bool activeFileLogger;					// true iff the logging service was successfully registered

		// game states
		bool hasStarted;						// true iff the DirectXApp was started completely
		
		// stats
		int fps;								// frames per second
		double mspf;							// milliseconds per frame

		util::Expected<void> calculateFrameStatistics();		// computes frame statistics

		// helper functions
		bool getPathToMyDocuments();			// stores the path to the My Documents folder in the appropriate member variable
		bool getPathToApplicationDataFolders();	// stores the paths to the application data folders
		void createLoggingService();			// creates the file logger and registers it as a service
		bool checkConfigurationFile();			// checks for valid configuration file
		
		// pause and resume game on notification
		util::Expected<void> onNotify(const int event);
		
		// check for fullscreen change
		util::Expected<void> checkFullscreen();

	protected:
		input::InputHandler* ih;				// pointer to an input handler

		std::deque<GameState*> gameStates;	// the different states of the application
		
		const Window* appWindow;				// the application window (i.e. game window)
		const HINSTANCE appInstance;			// handle to an instance of the application

		std::wstring keyBindingsFile;			// game input configuration file

		// DirectX Graphics
		graphics::Direct3D* d3d;				// pointer to the Direct3D class
		graphics::Direct2D* d2d;				// pointer to the Direct2D class

		// game states
		bool isPaused;							// true iff the game is paused
					
		// constructor and destructor
		DirectXApp(HINSTANCE hInstance, const std::wstring& applicationName, const std::wstring& applicationVersion);
		
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
		~DirectXApp();

		// options
		bool showFPS;							// true if and only if FPS information should be printed to the screen. Default: true; can be toggled via F1 (standard binding)

		// input variables
		bool activeMouse;						// true iff mouse input is active
		bool activeKeyboard;					// true iff keyboard input is active

		// manage the game states
		void changeGameState(GameState* const gameState);
		void pushGameState(GameState* const gameState);
		void popGameState();

		void addInputHandlerObserver(GameState* gameState);		// adds a game state as an observer to the input handler
		void removeInputHandlerObserver(GameState* gameState);	// removes a game state from the observer list of the input handler

		bool gameIsRunning;						// true iff the main game state is running (either actively or in the background)

		// pause
		void pauseGame();
		void resumeGame(bool recreateGraphics = false, bool restartTimer = false);

		// getters
		graphics::Direct2D* const getDirect2D() const { return d2d; };

		// application instance and main window
		const HINSTANCE& getApplicationInstance() const { return appInstance; };
		const HWND getMainWindow() const { return appWindow->getMainWindowHandle(); };

		// get current resolution
		const unsigned int getCurrentWidth() const { return d3d->getCurrentWidth(); };
		const unsigned int getCurrentHeight() const { return d3d->getCurrentHeight(); };
		
		// paths and configuration files
		const std::wstring& getPathToConfigurationFiles() const { return pathToUserConfigurationFiles; };
		const std::wstring& getPrefsFile() const { return userPrefFile; };
		bool hasValidConfigurationFile() const { return validUserConfigurationFile; };
		bool fileLoggerIsActive() const { return activeFileLogger; };

		// booleans
		bool showFramesPerSecond() const { return showFPS; };
		bool gameHasStarted() const { return hasStarted; };
	};
}