// include the header
#include "graphicsComponent.h"

// bell0bytes core
#include "timer.h"

// bell0bytes 3D graphics
#include "graphicsComponent3D.h"
#include "d3d.h"

// bell0bytes 2D graphics
#include "graphicsComponent2D.h"
#include "d2d.h"

// bell0bytes write
#include "graphicsComponentWrite.h"

// bell0bytes util
#include "serviceLocator.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructors ///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GraphicsComponent::GraphicsComponent(core::DirectXApp& dxApp, const core::Window& appWindow)
	{
		// create Direct3D components
		try { graphics3D = new GraphicsComponent3D(dxApp, appWindow); }
		catch (std::runtime_error& e) { throw e; }

		// create Direct2D components
		try { graphics2D = new GraphicsComponent2D(dxApp, *graphics3D->d3d); }
		catch (std::runtime_error& e) { throw e; }

		// create DirectWrite component
		try { graphicsWrite = new GraphicsComponentWrite(dxApp, graphics2D->getD2D().getDeviceContext(), graphics2D->getD2D().getWriteFactory(), graphics2D->getD2D().getBlackBrush()); }
		catch (std::runtime_error& e) { throw e; }
	}
	GraphicsComponent::~GraphicsComponent()
	{
		if (graphicsWrite)
			delete graphicsWrite;

		if (graphics2D)
			delete graphics2D;

		if (graphics3D)
			delete graphics3D;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Fullscreen State //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GraphicsComponent::checkFullscreen(core::DirectXApp& dxApp, core::Timer& timer, const bool applicationStarted, bool& isPaused)
	{
		if (applicationStarted)
		{
			util::Expected<bool> switchFullscreenState = graphics3D->d3d->switchFullscreen();
			if (switchFullscreenState.isValid())
			{
				if (switchFullscreenState.get())
				{	// fullscreen mode changed, pause the application, resize everything and unpause the application again
					isPaused = true;
					timer.stop();
					onResize(dxApp);
					timer.start();
					isPaused = false;
				}
			}
			else
				return switchFullscreenState;
		}

		return {};
	}
	util::Expected<void> GraphicsComponent::toggleFullscreen() const
	{
		return graphics3D->d3d->toggleFullscreen();
	}
	const bool GraphicsComponent::getFullscreenState() const
	{
		return graphics3D->d3d->getFullscreenState();
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Screen Resolution /////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	unsigned int GraphicsComponent::getCurrentWidth() const
	{
		return graphics3D->d3d->getCurrentWidth();
	}
	unsigned int GraphicsComponent::getCurrentHeight() const
	{
		return graphics3D->d3d->getCurrentHeight();
	}
	unsigned int GraphicsComponent::getCurrentRefreshRateNum() const
	{
		return graphics3D->d3d->getCurrentRefreshRateNum();
	}
	unsigned int GraphicsComponent::getCurrentRefreshRateDen() const
	{
		return graphics3D->d3d->getCurrentRefreshRateDen();
	}
	util::Expected<void> GraphicsComponent::changeResolution(const unsigned int index) const
	{
		return graphics3D->d3d->changeResolution(index);
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////// Get Components ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GraphicsComponent3D& GraphicsComponent::get3DComponent() const
	{
		return *graphics3D;
	}
	GraphicsComponent2D& GraphicsComponent::get2DComponent() const
	{
		return *graphics2D;
	}
	GraphicsComponentWrite& GraphicsComponent::getWriteComponent() const
	{
		return *graphicsWrite;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////// Resize //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> GraphicsComponent::onResize(core::DirectXApp& dxApp)
	{
#ifndef NDEBUG
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("The window was resized. The game graphics must be updated!");
#endif
		// handle errors
		util::Expected<void> result;
		
		// resize graphics
		result = graphics3D->d3d->onResize(graphics2D->d2d);
		if (!result.isValid())
			return result;

		// recreate the DirectWrite component
		delete graphicsWrite;
		try { graphicsWrite = new GraphicsComponentWrite(dxApp, graphics2D->getD2D().getDeviceContext(), graphics2D->getD2D().getWriteFactory(), graphics2D->getD2D().getBlackBrush()); }
		catch (std::runtime_error& e) { throw e; }

		// return success
		return {};
	}
}
