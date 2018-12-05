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
		// resize the swap chain
		if(FAILED(swapChain->ResizeBuffers(0, 0, 0, desiredColourFormat, 0)))
			return std::runtime_error("Direct3D was unable to resize the swap chain!");

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Scene Presentation////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<int> Direct3D::present()
	{
		HRESULT hr = swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
		if (FAILED(hr) && hr != DXGI_ERROR_WAS_STILL_DRAWING)
		{	
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>("The presentation of the scene failed!");
			return std::runtime_error("Direct3D failed to present the scene!");
		}

		// return success
		return 0;
	}
}