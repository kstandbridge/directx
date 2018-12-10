#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		08/03/2016 - Dortmund - Germany
*
* Desc:		the Window class handles Windows related stuff, such as window creation and event handling
*
* Hist:		- 03/06/18: now sends notifications to the main DirectX class
*			- 03/06/18: no more friend classes
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows
#include <Windows.h>

// bell0bytes util
#include "expected.h"
#include "observer.h"

namespace core
{
	// DEFINITIONS //////////////////////////////////////////////////////////////////////////
	class DirectXApp;

	// CLASSES //////////////////////////////////////////////////////////////////////////////
	class Window : public util::Subject
	{
	private:
		HWND mainWindow;						// handle to the main window
		DirectXApp* const dxApp;				// the core application class

		// resolution
		unsigned int clientWidth;				// desired client resolution
		unsigned int clientHeight;

		// window states
		bool isMinimized;						// true iff the window is minimized
		bool isMaximized;						// true iff the window is maximized
		bool isResizing;						// true iff the window is being dragged around by the mouse

		util::Expected<void> init(LPCWSTR windowTitle);	// initializes the window
		void readDesiredResolution();					// gets desired screen resolution from config file	

	public:
		// constructor and destructor
		Window(DirectXApp* const dxApp, LPCWSTR windowTitle);
		~Window();

		// getters
		inline HWND getMainWindowHandle() const { return mainWindow; };
		unsigned int getClientWidth() const { return clientWidth; };
		unsigned int getClientHeight() const { return clientHeight; };

		// the call back function
		virtual LRESULT CALLBACK msgProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam);
	};
}