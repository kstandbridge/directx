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
	DirectXApp::DirectXApp(HINSTANCE hInstance) : appInstance(hInstance), appWindow(NULL), activeLogger(false), validConfigurationFile(false), isPaused(true) { }
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

		// create the application window
		try { appWindow = new Window(this); }
		catch (std::runtime_error)
		{
			return std::runtime_error("DirectXApp was unable to create the main window!");
		}
		
		// log and return success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application initialization was successful.");
		return {};
	}

	void DirectXApp::shutdown(util::Expected<void>* expected)
	{
		if (appWindow)
			delete appWindow;

		if (appInstance)
			appInstance = NULL;

		if(activeLogger)
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The DirectX application was shutdown successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Main Event Loop //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<int> DirectXApp::run()
	{
		bool continueRunning = true;
		MSG msg = { 0 };

		// enter main event loop
		while (continueRunning)
		{
			// peek for messages
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
					continueRunning = false;
			}

			if (!isPaused)
			{
				// the game is active -> update game logic
			}
		}
		return (int)msg.wParam;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Resizing ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void DirectXApp::onResize()
	{ 
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("The window was resized. The game graphics must be updated!");
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
		activeLogger = true;

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