// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Lua and Sol

// warning supression no longer needed with C++-17
//#pragma warning( push )
//#pragma warning( disable : 4127)	// disable constant if expr warning
//#pragma warning( disable : 4702)	// disable unreachable code warning
#include <sol.hpp>
//#pragma warning( pop ) 
#pragma comment(lib, "liblua53.a")

// bell0bytes core
#include "window.h"
#include "app.h"

// bell0bytes util
#include "serviceLocator.h"
#include "stringConverter.h"

// bell0bytes resources
#include "resource.h"

// METHODS //////////////////////////////////////////////////////////////////////////////

namespace
{
	core::Window* window = NULL;
}

namespace core
{
	// the window procedure
	LRESULT CALLBACK mainWndProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
	{
		// forward messages from a global window procedure to the member window procedure until a valid window handle is available 
		// this is needed because we can't assign a member function to WINDCLASS:lpfnWndProc
		return window->msgProc(hWnd, msg, wParam, lParam);
	}

	// CLASS METHODS ////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructors /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Window::Window(DirectXApp* const dxApp, LPCWSTR windowTitle) : mainWindow(NULL), dxApp(dxApp), clientWidth(200), clientHeight(200), isMinimized(false), isMaximized(false), isResizing(false)
	{
		// this is necessary to forward messages
		window = this;

		// initialize the window
		util::Expected<void> windowInitialization = this->init(windowTitle);
		if (!windowInitialization.wasSuccessful())
		{
			// log the error
			try { windowInitialization.get(); }
			catch (std::exception& e)
			{
				// create and print error message string
				std::stringstream errorMessage;
				errorMessage << "Creation of the game window failed with: " << e.what();
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));

				// throw an error
				throw std::runtime_error("Window creation failed!");
			}
		}
	}

	Window::~Window()
	{
		// log
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Main window class destruction was successful.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Window Creation //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Window::init(LPCWSTR windowTitle)
	{	
		const HINSTANCE& appInstance = dxApp->getApplicationInstance();
		// specify the window class description
		WNDCLASSEX wc;

		// white window
		wc.cbClsExtra = 0;										// no extra bytes needed
		wc.cbSize = sizeof(WNDCLASSEX);							// size of the window description structure
		wc.cbWndExtra = 0;										// no extra bytes needed
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// brush to repaint the background with
		wc.hCursor = LoadCursor(0, IDC_ARROW);					// load the standard arrow cursor
		wc.hIcon = (HICON)LoadImage(appInstance, MAKEINTRESOURCE(IDI_BARKING_DOG), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR | LR_SHARED);				// load the barking dog icon
		wc.hIconSm = (HICON)LoadImage(appInstance, MAKEINTRESOURCE(IDI_BARKING_DOG), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR | LR_SHARED);
		wc.hInstance = appInstance;								// handle to the core application instance
		wc.lpfnWndProc = mainWndProc;							// window procedure function
		wc.lpszClassName = windowTitle;							// class name
		wc.lpszMenuName = 0;									// no menu
		wc.style = CS_HREDRAW | CS_VREDRAW;						// send WM_SIZE message when either the height or the width of the client area are changed

		// register the window
		if (!RegisterClassEx(&wc))
			return std::invalid_argument("The window class could not be registered; most probably due to invalid arguments!");

		// read desired screen resolution from a Lua configuration file
		readDesiredResolution();

		// get window size
		RECT rect = { 0, 0, (long)clientWidth, (long)clientHeight };
		if (!AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW))
			return std::invalid_argument("The client size of the window could not be computed!");

		// create the window
		mainWindow = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, wc.lpszClassName, windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, appInstance, NULL);
		if (!mainWindow)
			return std::invalid_argument("The window could not be created; most probably due to invalid arguments!");

		// show and update the windows
		ShowWindow(mainWindow, SW_SHOW);
		UpdateWindow(mainWindow);

		// register the application class as observer
		addObserver(dxApp);

		// log and return success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The main window was successfully created.");
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Message Procedure ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	LRESULT CALLBACK Window::msgProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)	// if the window became inactive, pause the application
				if (!notify(input::Events::PauseApplication).wasSuccessful())
					return -1;
				else                                // if the window was activated, unpause the applcation
					if (!notify(input::Events::ResumeApplication).wasSuccessful())
						return -1;
			break;

		case WM_DESTROY:
			// window is flagged to be destroyed: send a quit message
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The main window was flagged for destruction.");
			PostQuitMessage(0);
			break;

		case WM_CLOSE:
			// pause the game
			if (!notify(input::Events::PauseApplication).wasSuccessful())
				return -1;

			// display a message box and ask the user for confirmation
			if (MessageBox(mainWindow, L"Are you sure you want to quit? Cosmo will miss you!", L"Cosmo is sad!", MB_YESNO | MB_ICONQUESTION) == IDYES)
				return DefWindowProc(mainWindow, msg, wParam, lParam);
			else
			{
				// unpause the game
				if (!notify(input::Events::ResumeApplication).wasSuccessful())
					return -1;
				return 0;
			}

		case WM_MENUCHAR:
			// very important for your mental health: disables the crazy beeping sound when pressing a mnemonic key
			return MAKELRESULT(0, MNC_CLOSE);	// simply tell Windows that we want the menu closed

		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
			{
				// when the window is minimized, set the appropriate window flags and pause the application
				isMinimized = true;
				isMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				// when the window is maximized, set the appropriate window flags, resize the graphics and unpause the application
				isMinimized = false;
				isMaximized = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (isMinimized)
				{
					// the window was restored and was previously minimized, thus we set minimized to false, resize the graphics and restart the application
					isMinimized = false;
				}
				else if (isMaximized)
				{
					// the window was resized and was previously maxmized, thus we set maximized to false, resize the graphics and unpause the game
					isMaximized = false;
				}
				else if (isResizing)
				{
					// do nothing until the dragging / resizing has stopped (dragging continously sents WM_SIZE messages, it would be extremely slow (and very pointless) to respond to all them)
				}
			}
			// resize graphics and resume game
			if (!notify(input::Events::WindowChanged).wasSuccessful())
				return -1;
			break;

		case WM_ENTERSIZEMOVE:
			// the game window is being dragged around, set the isResizing flag and pause the game
			isResizing = true;
			if (!notify(input::Events::PauseApplication).wasSuccessful())
				return -1;
			break;

		case WM_EXITSIZEMOVE:
			// the game window is no longer being dragged around, set the isResizing flag to false, resize the graphics and unpause the game
			isResizing = false;
			if (!notify(input::Events::WindowChanged).wasSuccessful())
				return -1;
			break;

		case WM_GETMINMAXINFO:
			// prevent the window from becoming too small
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		case WM_WINDOWPOSCHANGED:
			// check for fullscreen switch
			if (!notify(input::Events::SwitchFullscreen).wasSuccessful())
				return -1;
			return 0;
		}

		// let Windows handle other messages
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Notification /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void notify()
	{
		
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Utility Functions ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Window::readDesiredResolution()
	{
		if (dxApp->hasValidConfigurationFile())
		{
			// configuration file exists, try to read from it
			std::wstring pathToPrefFile = dxApp->getPathToConfigurationFiles() + L"\\bell0prefs.lua";

			try
			{
				sol::state lua;
				lua.script_file(util::StringConverter::ws2s(pathToPrefFile));

				// read from the configuration file, default to 200 x 200
				clientWidth = lua["config"]["resolution"]["width"].get_or(200);
				clientHeight = lua["config"]["resolution"]["height"].get_or(200);
#ifndef NDEBUG
				std::stringstream res;
				res << "The client resolution was read from the Lua configuration file: " << clientWidth << " x " << clientHeight << ".";
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>(res.str());
#endif
			}
			catch (std::exception)
			{
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("Unable to read configuration file. Starting with default resolution: 200 x 200");
			}
		}
	}
}