#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		27/06/2018 - Lenningen - Luxembourg
*
* Desc:		graphics components (2D (plus DirectWrite) and 3D)
* Hist:
****************************************************************************************/

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

namespace util
{
	template<typename T>
	class Expected;
}

namespace core
{
	class DirectXApp;
	class Window;
	class Timer;
}

namespace graphics
{
	class GraphicsComponent2D;
	class GraphicsComponent3D;
	class GraphicsComponentWrite;

	class GraphicsComponent
	{
	private:
		GraphicsComponent2D* graphics2D;									// all things related to 2D graphics
		GraphicsComponent3D* graphics3D;									// all things related to 3D graphics
		GraphicsComponentWrite* graphicsWrite;								// all things related to writing text

		// resize functions
		virtual util::Expected<void> onResize(core::DirectXApp& dxApp);	// resize game graphics

	public:
		// constructs and destructors (dxApp can't be const because of the observer pattern)
		GraphicsComponent(core::DirectXApp& dxApp, const core::Window& appWindow);
		~GraphicsComponent();

		// retrieve the components
		GraphicsComponent3D& get3DComponent() const;
		GraphicsComponent2D& get2DComponent() const;
		GraphicsComponentWrite& getWriteComponent() const;

		// screen resolution
		unsigned int getCurrentWidth() const;
		unsigned int getCurrentHeight() const;
		unsigned int getCurrentRefreshRateNum() const;
		unsigned int getCurrentRefreshRateDen() const;
		
		// fullscreen state
		const bool getFullscreenState() const;
		util::Expected<void> checkFullscreen(core::DirectXApp& dxApp, core::Timer& timer, const bool applicationStarted, bool& isPaused);
		util::Expected<void> toggleFullscreen() const;
		util::Expected<void> changeResolution(const unsigned int index) const;

		friend class core::DirectXApp;
		friend class Direct3D;
	};
}