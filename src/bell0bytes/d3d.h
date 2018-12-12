#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/07/2017 - Dortmund - Germany
*
* Desc:		main class to use the Direct3D component of DirectX
*
* History:	- 06/08/2017: Direct2D added
*			- 13/03/2018: fullscreen support added
*			- 03/06/2018: sends notifications to the DirectX class whenever the screen resolution must be changed
*			- 03/06/2018: the DirectXApp class is no longer a friend
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and Com
#include <wrl/client.h>

// Direct3D
#include <d3d11_4.h>
#pragma comment (lib, "d3d11.lib")

// bell0bytes graphics
#include "d2d.h"

// bell0bytes utilities
#include "expected.h"
#include "observer.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// forward definitions
namespace core
{
	class Window;
	class DirectXApp;
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

	// a struct to use as constant buffer
	struct ConstantColourPositionBuffer
	{
		float x, y, z;						// position
		float spacing;						// spacing variable
		float r, g, b;						// colour
	};

	// shader buffer
	struct ShaderBuffer
	{
		BYTE* buffer;
		int size;
	};

	class Direct3D : public util::Subject
	{
	private:
		// members
		const core::DirectXApp* const dxApp;								// pointer to the main application class

		// Direct3D
		Microsoft::WRL::ComPtr<ID3D11Device> dev;							// the actual Direct3D device
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> devCon;					// its context
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;					// the swap chain
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;	// the rendering target
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;	// the depth and stencil buffer

		// Shader interfaces
		Microsoft::WRL::ComPtr<ID3D11VertexShader> standardVertexShader;	// the vertex shader
		Microsoft::WRL::ComPtr<ID3D11PixelShader> standardPixelShader;		// the pixel shader

		// constant buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantColourPositionBuffer;	// constant buffer to hold colour information
	
		// screen modes
		DXGI_FORMAT desiredColourFormat;						// the desired colour format
		unsigned int numberOfSupportedModes;					// the number of supported screen modes for the desired colour format
		DXGI_MODE_DESC* supportedModes;							// list of all supported screen modes for the desired colour format
		DXGI_MODE_DESC  currentModeDescription;					// description of the currently active screen mode
		int currentModeIndex;									// the index of the current mode in the list of all supported screen modes; -1 if not known yet
		bool startInFullscreen;									// true iff the game should start in fullscreen mode
		BOOL currentlyInFullscreen;								// true iff the game is currently in fullscreen mode
		bool changeMode;										// true iff the screen resolution should be changed this frame

		// helper functions
		util::Expected<void> writeCurrentModeDescriptionToConfigurationFile() const;	// write the current screen resolution to the configuration file
		util::Expected<void> readConfigurationFile();			// read preferences from configuration file
		
		// functions to create resources
		util::Expected<void> createResources(Direct2D* const d2d, const core::Window* const window);	// create device resources, such as the swap chain
		
		// functions to change screen resolutions
		void changeResolution(bool increase);					// changes the screen resolution, if increase is true, a higher resolution is chosen, else the resolution is lowered
		
		// rendering pipeline
		util::Expected<void> initPipeline();					// initializes the (graphics) rendering pipeline

		// shaders
		const util::Expected<ShaderBuffer> loadShader(const std::wstring filename) const;	// read shader data from .cso files

	public:
		// constructor
		Direct3D(core::DirectXApp* const dxApp, const core::Window* const mainWindow);
		~Direct3D();
			
		// resize resources
		util::Expected<void> onResize(Direct2D* const d2d);		// resize the resources
		util::Expected<bool> switchFullscreen() const;			// return true iff the fullscreen state should be switched
		util::Expected<void> toggleFullscreen(Direct2D* const); // toggle fullscreen mode
		util::Expected<void> changeResolution(const unsigned int index);		// change screen resolution to the desired index

		// present the scene
		void clearBuffers();									// clear the back and depth/stencil buffers (white)
		void clearBuffers(float[4]);							// clear the back buffer with a given colour
		util::Expected<int> present();							// present the chain, by flipping the buffers

		const unsigned int getCurrentWidth() const { return currentModeDescription.Width; };
		const unsigned int getCurrentHeight() const { return currentModeDescription.Height; };
		const unsigned int getCurrentRefreshRateDen() const { return currentModeDescription.RefreshRate.Denominator; };
		const unsigned int getCurrentRefreshRateNum() const { return currentModeDescription.RefreshRate.Numerator; };
		const unsigned int getCurrentModeIndex() const { return currentModeIndex; };
		const unsigned int getNumberOfSupportedModes() const { return numberOfSupportedModes; };
		const bool getFullscreenState() const { return currentlyInFullscreen; };
		const DXGI_MODE_DESC* const getSupportedModes() const { return supportedModes; };

		friend class Direct2D;
	};
}