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

		// initialize Direct3D
		try { d3d = new graphics::Direct3D(this); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to initialize Direct3D!");
		}

		// initialize Direct2D
		try { d2d = new graphics::Direct2D(this); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to initialize Direct2D!");
		}

		// log and return success
		hasStarted = true;
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application initialization was successful.");
		return {};
	}

	void DirectXApp::shutdown(util::Expected<void>* /*expected*/)
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
					return util::Expected<int>("Critical error: Unable to calculate frame statistics!");

				// acquire input

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
	/////////////////////////////////// Input ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::onKeyDown(WPARAM wParam, LPARAM /*lParam*/)
	{
		switch (wParam)
		{
		case VK_F1:
			showFPS = !showFPS;
			break;

		case VK_ESCAPE:
			PostMessage(appWindow->mainWindow, WM_CLOSE, 0, 0);
			break;

		case VK_PRIOR:	
			// page up -> chose higher resolution
			d3d->changeResolution(true);
			break;

		case VK_NEXT:
			// page down -> chose lower resolution
			d3d->changeResolution(false);
			break;

		default: break;

		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Resizing ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> DirectXApp::onResize()
	{
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("The window was resized. The game graphics must be updated!");
#endif
		if (!d3d->onResize().wasSuccessful())
			return std::runtime_error("Unable to resize Direct3D resources!");

		// return success
		return {};
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
				outFPS << "Resolution: " << d3d->currentModeDescription.Width << " x " << d3d->currentModeDescription.Height << " @ " << d3d->currentModeDescription.RefreshRate.Numerator / d3d->currentModeDescription.RefreshRate.Denominator << " Hz" << std::endl;
				outFPS << "Mode #" << d3d->currentModeIndex+1 << " of " << d3d->numberOfSupportedModes << std::endl;
				outFPS << "FPS: " << DirectXApp::fps << std::endl;
				outFPS << "mSPF: " << DirectXApp::mspf << std::endl;

				if (FAILED(d2d->writeFactory->CreateTextLayout(outFPS.str().c_str(), (UINT32)outFPS.str().size(), d2d->textFormatFPS.Get(), (float)appWindow->clientWidth, (float)appWindow->clientHeight, (IDWriteTextLayout **)d2d->textLayoutFPS.GetAddressOf())))
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