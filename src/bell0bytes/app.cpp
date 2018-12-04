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

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructors /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	DirectXApp::DirectXApp(HINSTANCE hInstance) : appInstance(hInstance), appWindow(NULL), activeFileLogger(false), validConfigurationFile(false), isPaused(true), timer(NULL), fps(0), mspf(0.0), dt(1000/(double)240), maxSkipFrames(10), hasStarted(false) { }
	DirectXApp::~DirectXApp()
	{
		shutdown();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// Create Core App Components  //////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::init()
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
		try { appWindow = new Window(this); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to create the main window!");
		}

		// log and return success
		hasStarted = true;
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application initialization was successful.");
		return {};
	}

	void DirectXApp::shutdown(util::Expected<void>* expected)
	{
		if (appWindow)
			delete appWindow;

		if (appInstance)
			appInstance = NULL;

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

		double accumulatedTime = 0.0;		// stores the time accumulated by the rendered
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
					continueRunning = false;
			}

			// let the timer tick
			timer->tick();

			if (!isPaused)
			{
				// compute fps
				calculateFrameStatistics();

				// acquire input

				// accumulate the elapsed time since the last frame
				accumulatedTime += timer->getDeltaTime();
				
				// now update the game logic with fixed dt as often as possible
				nLoops = 0;
				while (accumulatedTime >= dt && nLoops < maxSkipFrames)
				{
					update(dt);
					accumulatedTime -= dt;
					nLoops++;
				}
				
				// peek into the future and generate the output
				render(accumulatedTime / dt);
			}
		}
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Leaving the game loop...");
#endif
		return (int)msg.wParam;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Input ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::onKeyDown(WPARAM wParam, LPARAM lParam)
	{
		switch (wParam)
		{
		
		case VK_ESCAPE:
			PostMessage(appWindow->mainWindow, WM_CLOSE, 0, 0);
			break;

		default: break;

		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Update //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::update(double deltaTime)
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Rendering ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::render(double farseer)
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Resizing ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::onResize()
	{
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("The window was resized. The game graphics must be updated!");
#endif
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////// Frame Statistics ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::calculateFrameStatistics()
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

			// show statistics as window caption
			std::wstring windowCaption = L"bell0bytes engine --- fps: " + std::to_wstring(fps) + L" --- mspf: " + std::to_wstring(mspf);
			SetWindowText(appWindow->mainWindow, windowCaption.c_str());
			
			// reset
			nFrames = 0;
			elapsedTime += 1.0;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Utility Functions ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	bool DirectXApp::getPathToMyDocuments()
	{
		PWSTR docPath = NULL;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &docPath);

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
		std::wstring pathToPrefFile = pathToConfigurationFiles + L"\\bell0prefs.lua";

		// the directory exists, check if the log file is accessible
		std::ifstream prefFile(pathToPrefFile.c_str());
		if (prefFile.good())
		{
			// the file exists and can be read
			if (prefFile.peek() == std::ifstream::traits_type::eof())
			{
				// the file is empty, create it
				try
				{
					util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
					std::stringstream printPref;
					printPref << "config =\r\n{ \r\n\tresolution = { width = 800, y = height }\r\n}";
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
				printPref << "config =\r\n{ \r\n\tresolution = { width = 800, height = 600 }\r\n}";
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