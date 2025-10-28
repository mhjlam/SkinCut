#pragma once

#include <cfloat>
#include <string>

#include <d3d11.h>
#include <wrl/client.h>


namespace SkinCut {
	class Texture {
	public:
		Texture(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
				uint32_t width, uint32_t height, 
				DXGI_FORMAT format, D3D11_USAGE usage, 
				uint32_t bindFlags);
				
		Texture(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
				uint32_t width, uint32_t height, 
				DXGI_FORMAT format, D3D11_USAGE usage, 
				uint32_t bindFlags, const D3D11_SUBRESOURCE_DATA* data);

		Texture(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
				std::string path, D3D11_USAGE usage, 
				uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, 
				bool forceRGB);

		Texture(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			    D3D11_TEXTURE2D_DESC desc);

		Texture(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			    D3D11_TEXTURE2D_DESC desc, 
				const D3D11_SUBRESOURCE_DATA* data);

	public:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResource;
	};
}
