#pragma once

#include <cfloat>
#include <string>

#include <d3d11.h>
#include <wrl/client.h>


using Microsoft::WRL::ComPtr;


namespace SkinCut
{
	class Texture
	{
	public:
		ComPtr<ID3D11Texture2D> mTexture;
		ComPtr<ID3D11ShaderResourceView> mShaderResource;

	public:
		Texture(ComPtr<ID3D11Device>& device, uint32_t width, uint32_t height, DXGI_FORMAT format, D3D11_USAGE usage, uint32_t bindFlags);
		Texture(ComPtr<ID3D11Device>& device, uint32_t width, uint32_t height, DXGI_FORMAT format, D3D11_USAGE usage, uint32_t bindFlags, const D3D11_SUBRESOURCE_DATA* data);

		Texture(ComPtr<ID3D11Device>& device, std::string path, D3D11_USAGE usage, uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, bool forceRGB);

		Texture(ComPtr<ID3D11Device>& device, D3D11_TEXTURE2D_DESC desc);
		Texture(ComPtr<ID3D11Device>& device, D3D11_TEXTURE2D_DESC desc, const D3D11_SUBRESOURCE_DATA* data);
	};
}

