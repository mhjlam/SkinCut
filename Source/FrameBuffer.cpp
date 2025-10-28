#include "FrameBuffer.hpp"

#include "Util.hpp"
#include "Types.hpp"

using namespace SkinCut;
using Microsoft::WRL::ComPtr;


FrameBuffer::FrameBuffer(ComPtr<ID3D11Device>& device, 
	                     ComPtr<ID3D11Context>& context, 
						 ComPtr<IDXGISwapChain>& swapChain, 
						 DXGI_FORMAT depthFormatTex, 
						 DXGI_FORMAT depthFormatDsv, 
						 DXGI_FORMAT depthFormatSrv) : m_Device(device), m_Context(context) {
	// Color buffer
	swapChain->GetBuffer(0, __uuidof(ColorTexture), reinterpret_cast<void**>(ColorTexture.GetAddressOf()));

	D3D11_TEXTURE2D_DESC texDesc{};
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	ColorTexture->GetDesc(&texDesc);
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	::ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(ColorTexture.Get(), &rtvDesc, ColorBuffer.GetAddressOf());
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	HREXCEPT(device->CreateShaderResourceView(ColorTexture.Get(), &srvDesc, ColorResource.GetAddressOf()));


	// Depth-stencil buffer
	D3D11_TEXTURE2D_DESC depthTexDesc{};
	::ZeroMemory(&depthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthTexDesc.Width = texDesc.Width;
	depthTexDesc.Height = texDesc.Height;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = depthFormatTex;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	device->CreateTexture2D(&depthTexDesc, nullptr, &DepthTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	::ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = depthFormatDsv;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(DepthTexture.Get(), &dsvDesc, DepthBuffer.GetAddressOf());

	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = depthFormatSrv;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(DepthTexture.Get(), &srvDesc, DepthResource.GetAddressOf());


	// Viewport
	::ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
	Viewport.TopLeftX = 0.0F;
	Viewport.TopLeftY = 0.0F;
	Viewport.Width = (FLOAT)texDesc.Width;
	Viewport.Height = (FLOAT)texDesc.Height;
	Viewport.MinDepth = 0.0F;
	Viewport.MaxDepth = 1.0F;

	// Initial buffer clear
	Clear();
}


FrameBuffer::FrameBuffer(ComPtr<ID3D11Device>& device, 
	                     ComPtr<ID3D11Context>& context, 
						 uint32_t width, uint32_t height, 
						 DXGI_FORMAT colorFormat, 
						 DXGI_FORMAT depthFormatTex, 
						 DXGI_FORMAT depthFormatDsv, 
						 DXGI_FORMAT depthFormatSrv) : m_Device(device), m_Context(context) {
	// Color buffer
	D3D11_TEXTURE2D_DESC texDesc{};
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Format = colorFormat;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HREXCEPT(m_Device->CreateTexture2D(&texDesc, nullptr, &ColorTexture));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	::ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = colorFormat;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HREXCEPT(m_Device->CreateRenderTargetView(ColorTexture.Get(), &rtvDesc, ColorBuffer.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = colorFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HREXCEPT(m_Device->CreateShaderResourceView(ColorTexture.Get(), &srvDesc, ColorResource.GetAddressOf()));


	// Depth-stencil buffer
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = depthFormatTex;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	device->CreateTexture2D(&texDesc, nullptr, &DepthTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = depthFormatDsv;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(DepthTexture.Get(), &dsvDesc, DepthBuffer.GetAddressOf());

	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = depthFormatSrv;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(DepthTexture.Get(), &srvDesc, DepthResource.GetAddressOf());

	// Viewport
	ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
	Viewport.TopLeftX = 0.0F;
	Viewport.TopLeftY = 0.0F;
	Viewport.Width = (FLOAT)width;
	Viewport.Height = (FLOAT)height;
	Viewport.MinDepth = 0.0F;
	Viewport.MaxDepth = 1.0F;

	// Initial buffer clear
	Clear();
}


FrameBuffer::FrameBuffer(ComPtr<ID3D11Device>& device, 
	                     ComPtr<ID3D11Context>& context, 
						 ComPtr<ID3D11Texture2D>& texture, 
						 DXGI_FORMAT colorFormat, 
						 DXGI_FORMAT depthFormatTex, 
						 DXGI_FORMAT depthFormatDsv, 
						 DXGI_FORMAT depthFormatSrv) : m_Device(device), m_Context(context) {
	if (!texture) {
		throw;
	}

	// Color buffer
	D3D11_TEXTURE2D_DESC texDesc{};
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texture->GetDesc(&texDesc);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = colorFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HREXCEPT(m_Device->CreateTexture2D(&texDesc, nullptr, &ColorTexture));

	auto width = (FLOAT)texDesc.Width;
	auto height = (FLOAT)texDesc.Height;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	::ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = colorFormat;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HREXCEPT(m_Device->CreateRenderTargetView(ColorTexture.Get(), &rtvDesc, ColorBuffer.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = colorFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HREXCEPT(m_Device->CreateShaderResourceView(ColorTexture.Get(), &srvDesc, ColorResource.GetAddressOf()));

	// copy texture contents to new texture
	m_Context->CopyResource(ColorTexture.Get(), texture.Get());


	// Depth-stencil buffer
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = texDesc.Width;
	texDesc.Height = texDesc.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = depthFormatTex;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	device->CreateTexture2D(&texDesc, nullptr, &DepthTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	::ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = depthFormatDsv;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(DepthTexture.Get(), &dsvDesc, DepthBuffer.GetAddressOf());

	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = depthFormatSrv;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(DepthTexture.Get(), &srvDesc, DepthResource.GetAddressOf());

	// Viewport
	::ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
	Viewport.TopLeftX = 0.0F;
	Viewport.TopLeftY = 0.0F;
	Viewport.Width = width;
	Viewport.Height = height;
	Viewport.MinDepth = 0.0F;
	Viewport.MaxDepth = 1.0F;

	// Initial buffer clear
	Clear();
}

void FrameBuffer::Clear() {
	m_Context->ClearRenderTargetView(ColorBuffer.Get(), DirectX::Colors::Black);
	m_Context->ClearDepthStencilView(DepthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void FrameBuffer::Clear(const Math::Color& color) {
	m_Context->ClearRenderTargetView(ColorBuffer.Get(), color);
	m_Context->ClearDepthStencilView(DepthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
