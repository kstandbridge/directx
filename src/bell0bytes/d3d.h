#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/07/2017 - Dortmund - Germany
*
* Desc:		main class to use the Direct3D component of DirectX
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and Com
#include <wrl/client.h>

// Direct3D
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

// bell0bytes utilities
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class Direct3D
	{
	private:
		// members
		core::DirectXApp* dxApp;								// pointer to the main application class

		// Direct3D
		Microsoft::WRL::ComPtr<ID3D11Device> dev;				// the actual Direct3D device
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> devCon;		// its context
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;		// the swap chain

		// colour format
		DXGI_FORMAT desiredColourFormat;						// the desired colour format

		// functions to create resources
		util::Expected<void> createResources();					// create device resources, such as the swap chain
		util::Expected<void> onResize();						// resize the resources

	public:
		// constructor
		Direct3D(core::DirectXApp* dxApp);
		~Direct3D();

		// present the scene
		util::Expected<int> present();							// present the chain, but flipping the buffers

		friend class core::DirectXApp;
	};
}