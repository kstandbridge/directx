#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		27/06/2018 - Lenningen - Luxembourg
*
* Desc:		3D graphics components
* Hist:
****************************************************************************************/

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
struct DXGI_MODE_DESC;

namespace core
{
	class DirectXApp;
	class Window;
}

namespace util
{
	template<typename T>
	class Expected;
}

namespace graphics
{
	class Direct3D;
	
	class GraphicsComponent3D
	{
	private:
		// DirectX Graphics
		Direct3D* d3d;				// pointer to the Direct3D class

	public:
		GraphicsComponent3D(core::DirectXApp&, const core::Window&);
		~GraphicsComponent3D();

		void clearBuffers();					// clear the back and depth/stencil buffers (white)
		void clearBuffers(float[4]);			// clear the back buffer with a given colour
		util::Expected<int> present();			// present the chain, by flipping the buffers

		unsigned int getNumberOfSupportedModes() const;
		unsigned int getCurrentModeIndex() const;
		const DXGI_MODE_DESC* const getSupportedModes() const;

		friend class core::DirectXApp;
		friend class GraphicsComponent;
		friend class GraphicsComponent2D;
	};
}