// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows
#include <Shlobj.h>
#include <Shlwapi.h>
#include <Pathcch.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Pathcch.lib")

// bell0bytes

// core
#include "app.h"

// util
#include "serviceLocator.h"

// graphics
#include "d3d.h"
#include "d2d.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructors /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	DirectXApp::DirectXApp(HINSTANCE hInstance) : appInstance(hInstance), appWindow(NULL), activeFileLogger(false), prefFile(L"bell0prefs.lua"), validConfigurationFile(false), isPaused(true), timer(NULL), fps(0), mspf(0.0), dt(1000/(double)6000), maxSkipFrames(10), hasStarted(false), showFPS(true), d3d(NULL), d2d(NULL) { }
	DirectXApp::~DirectXApp()
	{
		shutdown();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// Create Core App Components  //////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::init(LPCWSTR windowTitle)
	{
		// get path to My Documents folder
		if (!getPathToMyDocuments())
		{
			// show error message on a message box
			MessageBox(NULL, L"Unable to retrieve the path to the My Documents folder!", L"Critical Error!", MB_ICONEXCLAMATION | MB_OK);
			return std::runtime_error("Unable to retrieve the path to the My Documents folder!");
		}
		
		// create the logger
		try { createLoggingService(); }
		catch(std::runtime_error) 
		{
			// show error message on a message box
			MessageBox(NULL, L"Unable to start the logging service!", L"Critical Error!", MB_ICONEXCLAMATION | MB_OK);
			return std::runtime_error("Unable to start the logging service!");
		}

		// check for valid configuration file
		if (!checkConfigurationFile())
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("Non-existent or invalid configuration file. Starting with default settings.");

		// create the game timer
		try { timer = new Timer(); }
		catch (std::runtime_error)
		{
			return std::runtime_error("The high-precision timer could not be started!");
		}

		// create the application window
		try { appWindow = new Window(this, windowTitle); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to create the main window!");
		}

		// initialize Direct3D
		try { d3d = new graphics::Direct3D(this, appWindow); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to initialize Direct3D!");
		}

		// initialize Direct2D
		try { d2d = new graphics::Direct2D(this, d3d); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to initialize Direct2D!");
		}
		
		// start game
		timer->start();
		isPaused = false;

		// log and return success
		hasStarted = true;
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application initialization was successful.");
		return {};
	}

	void DirectXApp::shutdown(const util::Expected<void>* /*expected*/)
	{
		if (d2d)
			delete d2d;

		if (d3d)
			delete d3d;

		if (appWindow)
			delete appWindow;

		if (timer)
			delete timer;

		if(activeFileLogger)
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application was shutdown successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// The Game Loop ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<int> DirectXApp::run()
	{
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Entering the game loop...");
#endif
		// reset (start) the timer
		timer->reset();

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
			timer->tick();

			if (!isPaused)
			{
				// expected result
				util::Expected<int> result(0);

				// compute fps
				if (!calculateFrameStatistics().wasSuccessful())
					return util::Expected<int>(std::runtime_error("Critical error: Unable to calculate frame statistics!"));

				// acquire input
				acquireInput();

				// accumulate the elapsed time since the last frame
				accumulatedTime += timer->getDeltaTime();
				
				// now update the game logic with fixed dt as often as possible
				nLoops = 0;
				while (accumulatedTime >= dt && nLoops < maxSkipFrames)
				{
					result = update(dt);
					if (!result.isValid())
						return result;
					accumulatedTime -= dt;
					nLoops++;
				}
				
				// peek into the future and generate the output
				result = render(accumulatedTime / dt);
				if (!result.isValid())
					return result;
			}
		}

#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Leaving the game loop...");
#endif
		return (int)msg.wParam;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Resizing ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::onResize() const
	{
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("The window was resized. The game graphics must be updated!");
#endif
		if (!d3d->onResize(d2d).wasSuccessful())
			return std::runtime_error("Unable to resize Direct3D resources!");

		// return success
		return {};
	}

	util::Expected<void> DirectXApp::checkFullscreen()
	{
		if (hasStarted)
		{
			util::Expected<bool> switchFullscreenState = d3d->switchFullscreen();
			if (switchFullscreenState.isValid())
			{
				if (switchFullscreenState.get())
				{	// fullscreen mode changed, pause the application, resize everything and unpause the application again
					isPaused = true;
					timer->stop();
					onResize();
					timer->start();
					isPaused = false;
				}
			}
			else
				return switchFullscreenState;
		}

		return { };
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
		if ((timer->getTotalTime() - elapsedTime) >= 1.0)
		{
			// set fps and mspf
			fps = nFrames;
			mspf = 1000.0 / (double)fps;

			if (showFPS)
			{
				// create FPS information text layout
				std::wostringstream outFPS;
				outFPS.precision(6);
				outFPS << "Resolution: " << d3d->getCurrentWidth() << " x " << d3d->getCurrentHeight() << " @ " << d3d->getCurrentRefreshRateNum() / d3d->getCurrentRefreshRateDen() << " Hz" << std::endl;
				outFPS << "Mode #" << d3d->getCurrentModeIndex()+1 << " of " << d3d->getNumberOfSupportedModes() << std::endl;
				outFPS << "FPS: " << DirectXApp::fps << std::endl;
				outFPS << "mSPF: " << DirectXApp::mspf << std::endl;

				if (!(d2d->createTextLayoutFPS(&outFPS, (float)d3d->getCurrentWidth(), (float)d3d->getCurrentHeight())).wasSuccessful())
					return std::runtime_error("Critical error: Failed to create the text layout for FPS information!");
			}

			// reset
			nFrames = 0;
			elapsedTime += 1.0;
		}

		// return success
		return { };
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Timing //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::pauseGame()
	{
		isPaused = true;
		timer->stop();
	}

	void DirectXApp::resumeGame(bool recreateGraphics, bool restartTimer)
	{
		if (recreateGraphics && hasStarted)
			onResize();

		if (restartTimer)
		{
			timer->start();
			isPaused = false;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Notification /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::onNotify(const int event)
	{
		switch (event)
		{
		case input::Events::PauseApplication:
			pauseGame();
			break;
		case input::Events::ResumeApplication:
			resumeGame(false, true);
			break;
		case input::Events::ChangeResolution:
			resumeGame(true, false);
			break;
		case input::Events::SwitchFullscreen:
			if (!checkFullscreen().wasSuccessful())
				return std::runtime_error("Critital error: Unable to check fullscreen state!");
			break;
		case input::Events::WindowChanged:
			resumeGame(true, true);
			break;
		default:
			break;
		}

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Utility Functions ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	bool DirectXApp::getPathToMyDocuments()
	{
		PWSTR docPath = NULL;

#ifndef NDEBUG
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &docPath);
#else
		SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &docPath);
#endif

		// debug mode only: make sure the operation succeeded
#ifndef NDEBUG
		if (FAILED(hr))
			return false;
#endif
		// store the path
		pathToMyDocuments = docPath; 

		// delete the wstring pointer to avoid memory leak
		::CoTaskMemFree(static_cast<void*>(docPath));

		// set up log and configuration paths
		
		// append custom folder to path
		std::wstringstream path;
		path << pathToMyDocuments << L"\\bell0bytes\\bell0tutorials\\Logs";
		pathToLogFiles = path.str();

		path.str(std::wstring());
		path.clear();
		path << pathToMyDocuments << L"\\bell0bytes\\bell0tutorials\\Settings";
		pathToConfigurationFiles = path.str();

		// return success
		return true;
	}

	void DirectXApp::createLoggingService()
	{
		// create directory (if it does not exist already)
		HRESULT hr;
		hr = SHCreateDirectory(NULL, pathToLogFiles.c_str());

		// debug mode only: make sure the operator succeeded
#ifndef NDEBUG
		if (FAILED(hr))
			throw std::runtime_error("Unable to create directory!");
#endif

		// append name of the log file to the path
		std::wstring logFile = pathToLogFiles + L"\\bell0engine.log";
		
		// create file logger
		std::shared_ptr<util::Logger<util::FileLogPolicy> > engineLogger(new util::Logger<util::FileLogPolicy>(logFile));

		// set logger to active
		activeFileLogger = true;

		// set name of current thread
		engineLogger->setThreadName("mainThread");

		// register the logging service
		util::ServiceLocator::provideFileLoggingService(engineLogger);

#ifndef NDEBUG
		// print starting message
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The file logger was created successfully.");
#endif
	}

	bool DirectXApp::checkConfigurationFile()
	{
		// create directory (if it does not exist already)
		HRESULT hr;
		hr = SHCreateDirectory(NULL, pathToConfigurationFiles.c_str());
#ifndef NDEBUG
		if (FAILED(hr))
			return false;
#endif
		// append name of the log file to the path
		std::wstring pathToPrefFile = pathToConfigurationFiles + L"\\" + prefFile; // L"\\bell0prefs.lua";

		// the directory exists, check if the log file is accessible
		std::ifstream prefStream(pathToPrefFile.c_str());
		if (prefStream.good())
		{
			// the file exists and can be read
			if (prefStream.peek() == std::ifstream::traits_type::eof())
			{
				// the file is empty, create it
				try
				{
					util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
					std::stringstream printPref;
					printPref << "config =\r\n{ \r\n\tfullscreen = false,\r\n\tresolution = { width = 800, height = 600 }\r\n}";
					prefFileCreator.print<util::config>(printPref.str());
				}
				catch (std::runtime_error)
				{
					return false;
				}
			}
		}
		else
		{
			// the file does not exist --> create it
			try 
			{ 
				util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
				std::stringstream printPref;
				printPref << "config =\r\n{ \r\n\tfullscreen = false,\r\n\tresolution = { width = 800, height = 600 }\r\n}";
				prefFileCreator.print<util::config>(printPref.str());
			}
			catch (std::runtime_error)
			{
				return false;
			}
		}

		validConfigurationFile = true;
		return true;
	}
}