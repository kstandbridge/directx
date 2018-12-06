#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/07/2017 - Dortmund - Germany
*
* Desc:		main class to use the Direct3D component of DirectX
*
* History:	- 06/08/2017: Direct2D added
*			- 13/03/2018: fullscreen support added
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and Com
#include <wrl/client.h>

// Direct3D
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

// bell0bytes utilities
#include "expected.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// forward definitions
class DirectXGame;
namespace core
{
	class DirectXApp;
	class Window;
}

// CLASSES //////////////////////////////////////////////////////////////////////////////

namespace graphics
{
	// structure to hold vertex data
	struct VERTEX
	{
		float x, y, z;						// position
		float r, g, b;						// colour
	};

	// shader buffer
	struct ShaderBuffer
	{
		BYTE* buffer;
		int size;
	};

	class Direct3D
	{
	private:
		// members
		core::DirectXApp* dxApp;								// pointer to the main application class

		// Direct3D
		Microsoft::WRL::ComPtr<ID3D11Device> dev;							// the actual Direct3D device
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> devCon;					// its context
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;					// the swap chain
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;	// the rendering target
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;	// the depth and stencil buffer

		// Shader interfaces
		Microsoft::WRL::ComPtr<ID3D11VertexShader> standardVertexShader;	// the vertex shader
		Microsoft::WRL::ComPtr<ID3D11PixelShader> standardPixelShader;		// the pixel shader
	
		// screen modes
		DXGI_FORMAT desiredColourFormat;						// the desired colour format
		unsigned int numberOfSupportedModes;					// the number of supported screen modes for the desired colour format
		DXGI_MODE_DESC* supportedModes;							// list of all supported screen modes for the desired colour format
		DXGI_MODE_DESC  currentModeDescription;					// description of the currently active screen mode
		unsigned int currentModeIndex;							// the index of the current mode in the list of all supported screen modes
		bool startInFullscreen;									// true iff the game should start in fullscreen mode
		BOOL currentlyInFullscreen;								// true iff the game is currently in fullscreen mode
		bool changeMode;										// true iff the screen resolution should be changed this frame

		// functions to change screen resolutions
		void changeResolution(bool increase);					// changes the screen resolution, if increase is true, a higher resolution is chosen, else the resolution is lowered

		// helper functions
		util::Expected<void> writeCurrentModeDescriptionToConfigurationFile();	// write the current screen resolution to the configuration file
		util::Expected<void> readConfigurationFile();			// read preferences from configuration file
		
		// functions to create resources
		util::Expected<void> createResources();					// create device resources, such as the swap chain
		util::Expected<void> onResize();						// resize the resources

		// rendering pipeline
		util::Expected<void> initPipeline();					// initializes the (graphics) rendering pipeline

		// shaders
		util::Expected<ShaderBuffer> loadShader(std::wstring filename);	// read shader data from .cso files

		// present the scene
		void clearBuffers();									// clear the back and depth/stencil buffers
		util::Expected<int> present();							// present the chain, by flipping the buffers

	public:
		// constructor
		Direct3D(core::DirectXApp* dxApp);
		~Direct3D();

		friend class core::DirectXApp;
		friend class Direct2D;
		friend class DirectXGame;
		friend class core::Window;
	};
}