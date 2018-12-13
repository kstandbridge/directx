// the header file
#include "coreComponent.h"

// bell0bytes core
#include "timer.h"			// high-precision timer class
#include "window.h"			// a Windows window :)


namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	CoreComponent::CoreComponent(DirectXApp& dxApp, const HINSTANCE& hInstance, LPCWSTR windowTitle) : appInstance(hInstance), appWindow(NULL)
	{
		try { timer = new Timer(); }
		catch (std::runtime_error& e) { throw e; }

		// create the application window
		try { appWindow = new Window(dxApp, hInstance, windowTitle); }
		catch (std::runtime_error& e) { throw e; }
	}
	CoreComponent::~CoreComponent()
	{
		if (appWindow)
			delete appWindow;

		if (timer)
			delete timer;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////// Getters ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const Window& CoreComponent::getWindow() const
	{
		return *appWindow;
	}
}
