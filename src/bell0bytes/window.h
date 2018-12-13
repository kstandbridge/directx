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
#include "observer.h"

namespace util
{
	template<typename T>
	class Expected;
}

namespace core
{
	// DEFINITIONS //////////////////////////////////////////////////////////////////////////
	class DirectXApp;

	// CLASSES //////////////////////////////////////////////////////////////////////////////
	class Window : public util::Subject
	{
	private:
		HWND mainWindow;						// handle to the main window
		DirectXApp& dxApp;						// the core application class

		// resolution
		unsigned int clientWidth;				// desired client resolution
		unsigned int clientHeight;

		// window states
		bool isMinimized;						// true iff the window is minimized
		bool isMaximized;						// true iff the window is maximized
		bool isResizing;						// true iff the window is being dragged around by the mouse

		util::Expected<void> init(const HINSTANCE& hInstance, LPCWSTR windowTitle);	// initializes the window
		void readDesiredResolution();					// gets desired screen resolution from config file	

	public:
		// constructor and destructor
		Window(DirectXApp& dxApp, const HINSTANCE& hInstance, LPCWSTR windowTitle);
		~Window();

		// getters
		inline const HWND& getMainWindowHandle() const { return mainWindow; };
		const unsigned int getClientWidth() const { return clientWidth; };
		const unsigned int getClientHeight() const { return clientHeight; };

		// the call back function
		virtual LRESULT CALLBACK msgProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam);
	};
}