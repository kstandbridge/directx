// INCLUDES /////////////////////////////////////////////////////////////////////////////

// the header
#include "app.h"

// bell0bytes core
#include "coreComponent.h"
#include "timer.h"
#include "states.h"

// bell0bytes file system
#include "fileSystemComponent.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponent2D.h"
#include "graphicsComponent3D.h"
#include "graphicsComponentWrite.h"

// bell0bytes input
#include "inputComponent.h"
#include "gameCommands.h"
#include "inputHandler.h"

// bell0bytes util
#include "serviceLocator.h"

// bell0bytes audio
#include "audioComponent.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructors /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	DirectXApp::DirectXApp() : applicationIsPaused(true), fps(0), mspf(0.0), dt(1000.0f/6000.0f), maxSkipFrames(10), applicationStarted(false), showFPS(true), stateStackChanged(false) { }
	DirectXApp::~DirectXApp()
	{
		shutdown();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// Create App Components  ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::init(const HINSTANCE& hInstance, LPCWSTR windowTitle, const std::wstring& manufacturerName, const std::wstring& applicationName, const std::wstring& applicationVersion)
	{
		// initialize file system components
		try { fileSystemComponent = new fileSystem::FileSystemComponent(manufacturerName, applicationName, applicationVersion); }
		catch (std::runtime_error& e) { return e; }
			
		// create the core components (mainly the window and timer class)
		try { coreComponent = new CoreComponent(*this, hInstance, windowTitle); }
		catch (std::runtime_error& e) { return e; }
		
		// initialize graphics components
		try { graphicsComponent = new graphics::GraphicsComponent(*this, *coreComponent->appWindow); }
		catch (std::runtime_error& e) { return e; }

		// initialize audio component
		try { audioComponent = new audio::AudioComponent(*this); }
		catch (std::runtime_error& e) { return e; }
	
		// start the application
		coreComponent->timer->start();
		applicationIsPaused = false;

		// log and return success
		applicationStarted = true;
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application initialization was successful.");
		return {};
	}
	void DirectXApp::shutdown(const util::Expected<void>* /*expected*/)
	{
		while (!gameStates.empty())
			gameStates.pop_back();

		if (audioComponent)
			delete audioComponent;

		if (inputComponent)
			delete inputComponent;

		if (graphicsComponent)
			delete graphicsComponent;

		if (coreComponent)
			delete coreComponent;

		if (fileSystemComponent->activeFileLogger)
		{
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application was shutdown successfully.");
			delete fileSystemComponent;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// The Game Loop ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<int> DirectXApp::run()
	{
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Entering the game loop...");
#endif
		// error handling
		util::Expected<void> voidResult;
		util::Expected<int> intResult(0);
		
		// reset (start) the timer
		voidResult = coreComponent->timer->reset();
		if (!voidResult.isValid())
			throw voidResult;
		
		double accumulatedTime = 0.0;		// stores the time accumulated by the renderer
		int nLoops = 0;						// the number of completed loops while updating the game

		// enter main event loop
		bool continueRunning = true;
		MSG msg = { 0 };
		while(continueRunning)
		{
			// peek for messages
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
				{
					continueRunning = false;
					break;
				}
			}

			// let the timer tick
			voidResult = coreComponent->timer->tick();
			if (!voidResult.isValid())
				throw voidResult;

			if (!applicationIsPaused)
			{
				// compute fps
				voidResult = calculateFrameStatistics();
				if (!voidResult.isValid())
					throw voidResult;

				// acquire input
				voidResult = acquireInput();
				if (!voidResult.isValid())
					throw voidResult;

				// dispatch message
				voidResult = dispatchMessages();
				if (!voidResult.isValid())
					throw voidResult;
				
				// accumulate the elapsed time since the last frame
				accumulatedTime += coreComponent->timer->getDeltaTime();
				
				// now update the game logic with fixed dt as often as possible
				nLoops = 0;
				while (accumulatedTime >= dt && nLoops < maxSkipFrames)
				{
					try { intResult = update(dt); }
					catch (util::Expected<void> &e) { throw std::move(e); }
					if (!intResult.isValid())
						return intResult;
					accumulatedTime -= dt;
					nLoops++;
				}
				
				// peek into the future and generate the output
				intResult = render(accumulatedTime / dt);
				if (!intResult.isValid())
					return intResult;
			}
		}

#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Leaving the game loop...");
#endif
		return (int)msg.wParam;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Game States //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::changeGameState(GameState* const gameState)
	{
		// handle errors
		util::Expected<void> result;

		// delete previous states
		while (!gameStates.empty())
		{
			result = (*gameStates.rbegin())->shutdown();
			if (!result.isValid())
				return result;

			gameStates.pop_back();
		}

		// push and initialize the new game state (if it is not on the stack already)
		gameStates.push_back(gameState);
		result = (*gameStates.rbegin())->initialize();
		if (!result.isValid())
			return result;

		stateStackChanged = true;

		// return success
		return { };
	}
	util::Expected<void> DirectXApp::overlayGameState(GameState* const gameState)
	{
		// handle errors
		util::Expected<void> result;

		// push new game state
		gameStates.push_back(gameState);
		result = (*gameStates.rbegin())->initialize();
		if (!result.isValid())
			return result;

		stateStackChanged = true;

		// return success
		return { };
	}
	util::Expected<void> DirectXApp::pushGameState(GameState* const gameState)
	{
		// handle errors
		util::Expected<void> result;

		// pause the current states
		for (std::deque<GameState*>::reverse_iterator it = gameStates.rbegin(); it != gameStates.rend(); it++)
		{
			result = (*it)->pause();
			if (!result.isValid())
				return result;
		}

		// push and initialize the new game state
		gameStates.push_back(gameState);
		result = (*gameStates.rbegin())->initialize();
		if (!result.isValid())
			return result;

		stateStackChanged = true;
		
		// return success
		return { };
	}
	util::Expected<void> DirectXApp::popGameState()
	{
		// handle errors
		util::Expected<void> result;

		// shut the current state down
		if (!gameStates.empty()) 
		{
			result = (*gameStates.rbegin())->shutdown();
			if (!result.isValid())
				return result;
			gameStates.pop_back();
		}

		// resume previous states
		for (std::deque<GameState*>::reverse_iterator it = gameStates.rbegin(); it != gameStates.rend(); it++)
		{
			result = (*it)->resume();
			if (!result.isValid())
				return result;
		}

		stateStackChanged = true;

		// return success
		return { };
	}
	void DirectXApp::getActiveStates(std::deque<GameState*>& stateQueue) const
	{
		stateQueue = gameStates;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////// Frame Statistics ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::calculateFrameStatistics()
	{
		static int nFrames;				    // number of frames seen
		static double elapsedTime;		    // time since last call
		nFrames++;

		// compute average statistics over one second
		if ((coreComponent->timer->getTotalTime() - elapsedTime) >= 1.0)
		{
			// set fps and mspf
			fps = nFrames;
			mspf = 1000.0 / (double)fps;

			if (showFPS)
			{
				// create FPS information text layout
				std::wostringstream outFPS;
				outFPS.precision(6);
				outFPS << "Resolution: " << graphicsComponent->getCurrentWidth() << " x " << graphicsComponent->getCurrentHeight() << " @ " << graphicsComponent->getCurrentRefreshRateNum() / graphicsComponent->getCurrentRefreshRateDen() << " Hz" << std::endl;
				outFPS << "Mode #" << graphicsComponent->get3DComponent().getCurrentModeIndex() + 1 << " of " << graphicsComponent->get3DComponent().getNumberOfSupportedModes() << std::endl;
				outFPS << "FPS: " << DirectXApp::fps << std::endl;
				outFPS << "mSPF: " << DirectXApp::mspf << std::endl;

				if (!(graphicsComponent->getWriteComponent().createTextLayoutFPS(outFPS, (float)graphicsComponent->getCurrentWidth(), (float)graphicsComponent->getCurrentHeight())).wasSuccessful())
					return std::runtime_error("Critical error: Failed to create the text layout for FPS information!");
			}

			// reset
			nFrames = 0;
			elapsedTime += 1.0;
		}

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////// Pause and Resume ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::pauseApplication()
	{
		applicationIsPaused = true;
		coreComponent->timer->stop();
	}
	void DirectXApp::resumeApplication(bool recreateGraphics, bool restartTimer)
	{
		if (recreateGraphics && applicationStarted)
			graphicsComponent->onResize(*this);

		if (restartTimer)
		{
			coreComponent->timer->start();
			applicationIsPaused = false;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Notification /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::onNotify(const int event)
	{
		// error handling
		util::Expected<void> checkingFullscreen;

		// switch depending on the event
		switch (event)
		{
		case input::Events::PauseApplication:
			pauseApplication();
			break;

		case input::Events::ResumeApplication:
			resumeApplication(false, true);
			break;

		case input::Events::ChangeResolution:
			resumeApplication(true, false);
			break;

		case input::Events::SwitchFullscreen:
			checkingFullscreen = graphicsComponent->checkFullscreen(*this, *coreComponent->timer, applicationStarted, applicationIsPaused);
			if (!checkingFullscreen.wasSuccessful())
				return checkingFullscreen;
			break;

		case input::Events::WindowChanged:
			resumeApplication(true, true);
			break;

		default:
			break;
		}

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Components ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	CoreComponent& DirectXApp::getCoreComponent() const
	{
		return *coreComponent;
	}
	fileSystem::FileSystemComponent& DirectXApp::getFileSystemComponent() const
	{
		return *fileSystemComponent;
	}
	graphics::GraphicsComponent& DirectXApp::getGraphicsComponent() const
	{
		return *graphicsComponent;
	}
	input::InputComponent& DirectXApp::getInputComponent() const
	{
		return *inputComponent;
	}
	audio::AudioComponent& DirectXApp::getAudioComponent() const
	{
		return *audioComponent;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Resize ///////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::onResize()
	{
		return graphicsComponent->onResize(*this);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Event Queue //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::dispatchMessages()
	{
		// error handling
		util::Expected<void> result;

		while (!eventQueue.isEmpty())
		{
			// get the front message
			Depesche depesche = eventQueue.dequeue();

			// check whether the receiver actually exists
			DepescheDestination* destination = depesche.destination;
			if (destination)
				// the destination is valid
				result = destination->onMessage(depesche);

			if (!result.isValid())
				return result;
		}

		return { };
	}

	void DirectXApp::addMessage(Depesche& depesche)
	{
		eventQueue.enqueue(depesche);
	}
}