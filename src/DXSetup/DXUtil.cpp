#include "DXUtil.h"

void Utility::GetTextureDim(ID3D11Resource* res, UINT* width, UINT* height)
{
	D3D11_RESOURCE_DIMENSION dim;
	res->GetType(&dim);
	switch (dim)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			auto texture = reinterpret_cast<ID3D11Texture2D*>(res);
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);
			if(width) *width = desc.Width;		// Width of texture in pixels
			if(height) *height = desc.Height;	// Height of texture in pixels

		}
		break;
	case D3D11_RESOURCE_DIMENSION_UNKNOWN:
	case D3D11_RESOURCE_DIMENSION_BUFFER:
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
	default:
		{
			if(width) *width = 0;		// Width of texture in pixels
			if(height) *height = 0;	// Height of texture in pixels
		}
		break;
	}
}
