#include "d3d.h"
#include "serviceLocator.h"
#include "app.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Direct3D::Direct3D(core::DirectXApp* dxApp) : dxApp(dxApp), desiredColourFormat(DXGI_FORMAT_B8G8R8A8_UNORM)
	{
		HRESULT hr;

		// define device creation flags,  D3D11_CREATE_DEVICE_BGRA_SUPPORT needed to get Direct2D interoperability with Direct3D resources
		unsigned int createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		// if in debug mode, create device with debug layer
#ifndef NDEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// create the device
		D3D_FEATURE_LEVEL featureLevel;
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, NULL, 0, D3D11_SDK_VERSION, &dev, &featureLevel, &devCon);

		if (FAILED(hr))
		{
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>("The creation of the Direct3D device and its context failed!");
			throw std::runtime_error("Unable to create the Direct3D device and its context!");
		}
		else if (featureLevel < D3D_FEATURE_LEVEL_11_0)
		{
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>("Critical error: DirectX 11 is not supported by your GPU!");
			throw std::runtime_error("Unable to create the Direct3D device and its context!");
		}

		// now that the device and its context are available, create further resouces
		if (!createResources().wasSuccessful())
		{
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>("Critical error: Creation of Direct3D resources failed!");
			throw std::runtime_error("Creation of Direct3D resources failed!");
		}

		//  log success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Direct3D was initialized successfully.");
	}

	Direct3D::~Direct3D()
	{
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Direct3D was shut down successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Resource Creation ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct3D::createResources()
	{
		// create the swap chain

		// fill in the swap chain description
		DXGI_SWAP_CHAIN_DESC scd;
		scd.BufferDesc.Width = 0;													// width of the back buffer
		scd.BufferDesc.Height = 0;													// height
		scd.BufferDesc.RefreshRate.Numerator = 0;									// refresh rate: 0 -> do not care
		scd.BufferDesc.RefreshRate.Denominator = 1;					
		scd.BufferDesc.Format = desiredColourFormat;								// the color palette to use								
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;		// unspecified scan line ordering
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;						// unspecified scaling
		scd.SampleDesc.Count = 1;													// disable msaa
		scd.SampleDesc.Quality = 0;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;							// use back buffer as render target
		scd.BufferCount = 3;														// the number of buffers in the swap chain (including the front buffer)
		scd.OutputWindow = dxApp->appWindow->getMainWindowHandle();					// set the main window as output target
		scd.Windowed = true;														// windowed, not fullscreen$
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;								// flip mode and discared buffer after presentation
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;							// allow mode switching

		// get the DXGI factory
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;

		// first, retrieve the underlying DXGI device from the Direct3D device
		HRESULT hr = dev.As(&dxgiDevice);
		if (FAILED(hr))
			return std::runtime_error("The Direct3D device was unable to retrieve the underlying DXGI device!");

		// now identify the physical GPU this device is running on
		hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("The DXGI Device was unable to get the GPU adapter!");

		// finally retrieve the factory
		hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);
		if (FAILED(hr))
			return std::runtime_error("The DXGI Adapter was unable to get the factory!");

		// create the actual swap chain
		hr = dxgiFactory->CreateSwapChain(dev.Get(), &scd, swapChain.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("The creation of the swap chain failed!");

		// the remaining steps need to be done each time the window is resized
		if (!onResize().wasSuccessful())
			return std::runtime_error("Direct3D was unable to resize its resources!");

		// return success
		return {};
	}

	util::Expected<void> Direct3D::onResize()
	{
		// release and reset all resources
		if(dxApp->d2d)
			dxApp->d2d->devCon->SetTarget(nullptr);

		devCon->ClearState();
		renderTargetView = nullptr;
		depthStencilView = nullptr;

		// resize the swap chain
		if(FAILED(swapChain->ResizeBuffers(0, 0, 0, desiredColourFormat, 0)))
			return std::runtime_error("Direct3D was unable to resize the swap chain!");

		// (re)-create the render target view
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
			return std::runtime_error("Direct3D was unable to acquire the back buffer!");
		if (FAILED(dev->CreateRenderTargetView(backBuffer.Get(), NULL, &renderTargetView)))
			return std::runtime_error("Direct3D was unable to create the render target view!");

		// create the depth and stencil buffer
		D3D11_TEXTURE2D_DESC dsd;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> dsBuffer;
		backBuffer->GetDesc(&dsd);
		dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsd.Usage = D3D11_USAGE_DEFAULT;
		dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		if (FAILED(dev->CreateTexture2D(&dsd, NULL, dsBuffer.GetAddressOf())))
			return std::runtime_error("Direct3D was unable to create a 2D-texture!");
		if (FAILED(dev->CreateDepthStencilView(dsBuffer.Get(), NULL, depthStencilView.GetAddressOf())))
			return std::runtime_error("Direct3D was unable to create the depth and stencil buffer!");

		// activate the depth and stencil buffer
		devCon->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

		// set the viewport to the entire backbuffer
		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Width = (float)dsd.Width;
		vp.Height = (float)dsd.Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		devCon->RSSetViewports(1, &vp);

		// (re)-create the Direct2D target bitmap associated with the swap chain back buffer and set it as the current target
		if(dxApp->d2d)
			if(!dxApp->d2d->createBitmapRenderTarget().wasSuccessful())
				return std::runtime_error("Direct3D was unable to resize the Direct2D bitmap render target!");

		// re-initialize GPU pipeline
		initPipeline();

		// log and return success
		if (dxApp->hasStarted)
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The Direct3D and Direct2D resources were resized successfully.");
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Pipeline ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Direct3D::initPipeline()
	{
		// load Compiled Shader Object files
#ifndef NDEBUG
		util::Expected<ShaderBuffer> vertexShaderBuffer = loadShader(L"../../x64/Debug/vertexShader.cso");
		util::Expected<ShaderBuffer> pixelShaderBuffer = loadShader(L"../../x64/Debug/pixelShader.cso");
#else
		util::Expected<ShaderBuffer> vertexShaderBuffer = loadShader(L"../../x64/Release/vertexShader.cso");
		util::Expected<ShaderBuffer> pixelShaderBuffer = loadShader(L"../../x64/Release/pixelShader.cso");
#endif
		if (!vertexShaderBuffer.wasSuccessful() || !pixelShaderBuffer.wasSuccessful())
			return "Critical error: Unable to read Compiled Shader Object files!";

		// create the shaders
		if (FAILED(dev->CreateVertexShader(vertexShaderBuffer.get().buffer, vertexShaderBuffer.get().size, nullptr, &standardVertexShader)))
			return "Critical error: Unable to create the vertex shader!";
		if(FAILED(dev->CreatePixelShader(pixelShaderBuffer.get().buffer, pixelShaderBuffer.get().size, nullptr, &standardPixelShader)))
			return "Critical error: Unable to create the pixel shader!";

		// set the shader objects as the active shaders
		devCon->VSSetShader(standardVertexShader.Get(), nullptr, 0);
		devCon->PSSetShader(standardPixelShader.Get(),  nullptr, 0);

		// specify the input layout
		D3D11_INPUT_ELEMENT_DESC ied[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };

		// create the input layout
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		if (FAILED(dev->CreateInputLayout(ied, ARRAYSIZE(ied), vertexShaderBuffer.get().buffer, vertexShaderBuffer.get().size, &inputLayout)))
			return "Critical error: Unable to create the input layout!";

		// set active input layout
		devCon->IASetInputLayout(inputLayout.Get());

		// delete shader buffer pointers
		delete vertexShaderBuffer.get().buffer;
		delete pixelShaderBuffer.get().buffer;

		// log and return return success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The rendering pipeline was successfully initialized.");
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Shaders ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<ShaderBuffer> Direct3D::loadShader(std::wstring filename)
	{
		// load precompiled shaders from .cso objects
		ShaderBuffer sb;
		byte* fileData = nullptr;

		// open the file
		std::ifstream csoFile(filename, std::ios::in | std::ios::binary | std::ios::ate);

		if (csoFile.is_open())
		{
			// get shader size
			sb.size = (unsigned int)csoFile.tellg();

			// collect shader data
			fileData = new byte[sb.size];
			csoFile.seekg(0, std::ios::beg);
			csoFile.read(reinterpret_cast<char*>(fileData), sb.size);
			csoFile.close();
			sb.buffer = fileData;
		}
		else
			return "Critical error: Unable to open the compiled shader object!";

		// return the shader buffer
		return sb;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Scene Presentation ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Direct3D::clearBuffers()
	{
		// clear the back buffer and depth / stencil buffer
		float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		devCon->ClearRenderTargetView(renderTargetView.Get(), black);
		devCon->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	util::Expected<int> Direct3D::present()
	{
		HRESULT hr = swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
		if (FAILED(hr) && hr != DXGI_ERROR_WAS_STILL_DRAWING)
		{	
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>("The presentation of the scene failed!");
			return std::runtime_error("Direct3D failed to present the scene!");
		}

		// rebind the depth and stencil buffer - necessary since the flip model releases the view targets after a call to present
		devCon->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

		// return success
		return 0;
	}
}