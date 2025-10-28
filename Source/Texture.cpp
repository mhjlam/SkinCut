#include "Texture.hpp"

#include <DDSTextureLoader.h>

#include "Util.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


Texture::Texture(ComPtr<ID3D11Device>& device, 
	             uint32_t width, uint32_t height, 
				 DXGI_FORMAT format, D3D11_USAGE usage, uint32_t bindFlags) {
	D3D11_TEXTURE2D_DESC texDesc;
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = usage;
	texDesc.BindFlags = bindFlags;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	device->CreateTexture2D(&texDesc, nullptr, m_Texture.GetAddressOf());
}

Texture::Texture(ComPtr<ID3D11Device>& device, 
	             uint32_t width, uint32_t height, 
				 DXGI_FORMAT format, D3D11_USAGE usage, uint32_t bindFlags, 
				 const D3D11_SUBRESOURCE_DATA* data) {
	D3D11_TEXTURE2D_DESC texDesc;
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = usage;
	texDesc.BindFlags = bindFlags;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	device->CreateTexture2D(&texDesc, data, m_Texture.GetAddressOf());
}

Texture::Texture(ComPtr<ID3D11Device>& device, std::string path, D3D11_USAGE usage, 
	             uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, bool forceRgb) {
	std::wstring wpath(path.begin(), path.end());

	DirectX::DDS_LOADER_FLAGS ddsLoaderFlags = DirectX::DDS_LOADER_DEFAULT;
	if (forceRgb) {
		ddsLoaderFlags = DirectX::DDS_LOADER_FORCE_SRGB;
	}

	ComPtr<ID3D11Resource> resource;
	DirectX::CreateDDSTextureFromFileEx(device.Get(), wpath.c_str(), 0, usage, bindFlags, 0, 0, 
		ddsLoaderFlags, resource.GetAddressOf(), m_ShaderResource.GetAddressOf());
	m_ShaderResource->GetResource(reinterpret_cast<ID3D11Resource**>(m_Texture.GetAddressOf()));
}

Texture::Texture(ComPtr<ID3D11Device>& device, D3D11_TEXTURE2D_DESC desc) {
	device->CreateTexture2D(&desc, nullptr, m_Texture.GetAddressOf());
}

Texture::Texture(ComPtr<ID3D11Device>& device, D3D11_TEXTURE2D_DESC desc, const D3D11_SUBRESOURCE_DATA* data) {
	device->CreateTexture2D(&desc, data, m_Texture.GetAddressOf());
}
