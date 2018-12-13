#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		26/06/2018 - Lenningen - Luxembourg
*
* Desc:		core components of the DirectXApp class:
*				- core variables
*				- Timer
*				- Window
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows includes
#include <Windows.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
	class Timer;
	class Window;

	class CoreComponent
	{
	private:
		const Window* appWindow;				// the application window (i.e. game window)
		const HINSTANCE appInstance;			// handle to an instance of the application

		Timer* timer;							// high-precision timer

	public:
		CoreComponent(DirectXApp& dxApp, const HINSTANCE& hInstance, LPCWSTR windowTitle);
		~CoreComponent();

		// getters
		const Window& getWindow() const;

		friend class DirectXApp;
	};
}