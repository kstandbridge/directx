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

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace graphics
{
	class Direct3D
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> dev;				// the actual Direct3D device
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> devCon;		// its context

	public:
		// constructor
		Direct3D();
		~Direct3D();
	};
}