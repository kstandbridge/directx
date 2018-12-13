#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		08/03/2016 - Dortmund - Germany
*
* Desc:		main class to bring together all the core components of a game
*
* Hist:		- 01/06/18: fixed a memory leak in showFPS method (released text format)
*			- 03/06/18: now observes events from the Window and Direct3D classes
*			- 21/06/18: changed the state stack to allow overlays
*			- 27/06/18: sliced the app class into several components
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ containers
#include <deque>			// deque for the stack of game states

// Windows includes
#include <Windows.h>		// Windows definitions

// bell0bytes utilities
#include "observer.h"		// the observer pattern
#include "safeQueue.h"		// a thread-safe queue

// bell0bytes core
#include "depesche.h"		// event queue data


// CLASSES //////////////////////////////////////////////////////////////////////////////

// forward definitions
namespace input
{
	class InputComponent;
}

namespace graphics
{
	class GraphicsComponent;
}

namespace fileSystem
{
	class FileSystemComponent;
}

namespace audio
{
	class AudioComponent;
}

namespace core
{
	class CoreComponent;
	class GameState;

	// the main DirectX application class
	class DirectXApp : public util::Observer
	{
	private:
		// the main message queue
		util::ThreadSafeQueue<Depesche> eventQueue;	// thread-safe message queue
		
		// game update variables
		const double dt;						// constant game update rate for better physics simulation (less rounding errors in mathematical computations)
		const double maxSkipFrames;				// constant maximum of frames to skip in the update loop (important to not stall the system on slower computers)
		
		// frame statistics
		int fps;								// frames per second
		double mspf;							// milliseconds per frame
		bool showFPS;							// true if and only if FPS information should be printed to the screen. Default: true; can be toggled via "Shift+Ctrl+F" (standard binding)

		// game states
		bool applicationStarted;				// true iff the DirectXApp was started completely
		bool applicationIsPaused;				// true iff the application is paused

		// react to events - solely for application interactions ; player and game entity interactions will be handled by the event queue
		// observes the window and Direct3D classes
		//		- the window class: when the window is resized, minimized, maximized ...
		//		- the Direct3D class: when the resolution or the fullscreen state changes
		util::Expected<void> onNotify(const int event);
		
		// dispatch the messages in the event queue
		util::Expected<void> dispatchMessages();

		// pause and resume the application
		void pauseApplication();
		void resumeApplication(bool recreateGraphics = false, bool restartTimer = false);

		// calculate frame statistics
		util::Expected<void> calculateFrameStatistics();
		
	protected:
		// components
		CoreComponent* coreComponent;							// core components: timer, window...
		fileSystem::FileSystemComponent* fileSystemComponent;	// file system components
		graphics::GraphicsComponent* graphicsComponent;			// 2D and 3D graphics
		input::InputComponent* inputComponent;					// input components
		audio::AudioComponent* audioComponent;					// AudioEngine audio component
		
		// the states of the game
		std::deque<GameState*> gameStates;		// the different states of the application
		bool stateStackChanged;					// true iff if the game stack was recently changed
				
		// constructor and destructor
		DirectXApp();

		// initialization and shutdown
		virtual util::Expected<void> init(const HINSTANCE& hInstance, LPCWSTR windowTitle, const std::wstring& manufacturerName, const std::wstring& applicationName, const std::wstring& applicationVersion);				// initializes the DirectX application
		virtual void shutdown(const util::Expected<void>* const expected = NULL);																																			// clean up and shutdown the DirectX application

		// user input
		virtual util::Expected<void> initializeInput(const HINSTANCE& hInstance, const HWND& appWindow) = 0;
		virtual util::Expected<void> acquireInput() = 0;

		// game loop
		virtual util::Expected<int> run();									// enters the main event loop
		virtual util::Expected<int> update(const double dt) = 0;			// update the game world

		// graphics resize
		virtual util::Expected<void> onResize();							// resize game graphics

		// generating output
		virtual util::Expected<int> render(const double farseer) = 0;		// renders the game world

	public:
		// destructor
		~DirectXApp();

		// event queue
		void addMessage(Depesche&);		// add a message to the queue

		// manage the game states
		util::Expected<void> changeGameState(GameState* const gameState);	// change game state (deletes all previous states)
		util::Expected<void> overlayGameState(GameState* const gameState);	// add new game state on top of the existing one; do not pause anything
		util::Expected<void> pushGameState(GameState* const gameState);		// push new game state and pause current one
		util::Expected<void> popGameState();								// pop game state and resume previous one
		void getActiveStates(std::deque<GameState*>&) const;				// retrieves all states from the stack

		// booleans
		const bool showFramesPerSecond() const { return showFPS; };
		const bool gameHasStarted() const { return applicationStarted; };

		// options
		void toggleFPS() { showFPS = !showFPS; };

		// get the components
		graphics::GraphicsComponent& getGraphicsComponent() const;
		fileSystem::FileSystemComponent& getFileSystemComponent() const;
		input::InputComponent& getInputComponent() const;
		CoreComponent& getCoreComponent() const;
		audio::AudioComponent& getAudioComponent() const;
	};
}