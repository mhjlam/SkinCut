#include "RenderTarget.hpp"

#include <DirectXTex.h>

#include "Util.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


RenderTarget::RenderTarget(ComPtr<ID3D11Device>& device, 
	                       ComPtr<ID3D11DeviceContext>& context, 
						   uint32_t width, uint32_t height, 
						   DXGI_FORMAT format, bool typeless) : m_Device(device), m_Context(context) {
	D3D11_TEXTURE2D_DESC desc;
	::ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = typeless ? DirectX::MakeTypeless(format) : format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HREXCEPT(m_Device->CreateTexture2D(&desc, nullptr, m_Texture.GetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	::ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HREXCEPT(m_Device->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, m_RenderTarget.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HREXCEPT(m_Device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_ShaderResource.GetAddressOf()));

	// Viewport
	::ZeroMemory(&m_Viewport, sizeof(D3D11_VIEWPORT));
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (FLOAT)width;
	m_Viewport.Height = (FLOAT)height;
	m_Viewport.MinDepth = 0.F;
	m_Viewport.MaxDepth = 1.F;

	// Create default blend state
	::ZeroMemory(&m_BlendDesc, sizeof(D3D11_BLEND_DESC));
	m_BlendDesc.AlphaToCoverageEnable = FALSE;
	m_BlendDesc.IndependentBlendEnable = FALSE;
	m_BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	m_BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	m_BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	m_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HREXCEPT(m_Device->CreateBlendState(&m_BlendDesc, m_BlendState.GetAddressOf()));

	m_BlendFactor = Math::Color(1,1,1,1);
	m_SampleMask = 0xffffffff;

	// Clear buffer for first use
	m_Context->ClearRenderTargetView(m_RenderTarget.Get(), DirectX::Colors::Transparent);
}


RenderTarget::RenderTarget(ComPtr<ID3D11Device>& device, 
	                       ComPtr<ID3D11DeviceContext>& context, 
						   uint32_t width, uint32_t height, DXGI_FORMAT format, 
						   ComPtr<ID3D11Texture2D>& baseTex) : m_Device(device), m_Context(context) {
	D3D11_TEXTURE2D_DESC texDesc{};
	::ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	HREXCEPT(m_Device->CreateTexture2D(&texDesc, nullptr, &m_Texture));

	if (baseTex) {
		context->CopyResource(m_Texture.Get(), baseTex.Get());
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	::ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HREXCEPT(m_Device->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, m_RenderTarget.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HREXCEPT(m_Device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_ShaderResource.GetAddressOf()));

	::ZeroMemory(&m_Viewport, sizeof(D3D11_VIEWPORT));
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (FLOAT)width;
	m_Viewport.Height = (FLOAT)height;
	m_Viewport.MinDepth = 0.F;
	m_Viewport.MaxDepth = 1.F;

	// Create default blend state
	::ZeroMemory(&m_BlendDesc, sizeof(D3D11_BLEND_DESC));
	m_BlendDesc.AlphaToCoverageEnable = FALSE;
	m_BlendDesc.IndependentBlendEnable = FALSE;
	m_BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	m_BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	m_BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	m_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HREXCEPT(m_Device->CreateBlendState(&m_BlendDesc, m_BlendState.GetAddressOf()));

	m_BlendFactor = Math::Color(1,1,1,1);
	m_SampleMask = 0xffffffff;
}

RenderTarget::RenderTarget(ComPtr<ID3D11Device>& device, 
	                       ComPtr<ID3D11DeviceContext>& context, 
	                       ComPtr<ID3D11Texture2D>& texture, 
						   DXGI_FORMAT format) : m_Device(device), m_Context(context), m_Texture(texture) {
	D3D11_TEXTURE2D_DESC texDesc;
	m_Texture->GetDesc(&texDesc);

	// Increase the count as the reference is now shared between two RenderTargets.
	m_Texture.Get()->AddRef();

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	::ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HREXCEPT(m_Device->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, m_RenderTarget.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	::ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HREXCEPT(m_Device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_ShaderResource.GetAddressOf()));

	::ZeroMemory(&m_Viewport, sizeof(D3D11_VIEWPORT));
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (FLOAT)texDesc.Width;
	m_Viewport.Height = (FLOAT)texDesc.Height;
	m_Viewport.MinDepth = 0.F;
	m_Viewport.MaxDepth = 1.F;

	// Create default blend state
	::ZeroMemory(&m_BlendDesc, sizeof(D3D11_BLEND_DESC));
	m_BlendDesc.AlphaToCoverageEnable = FALSE;
	m_BlendDesc.IndependentBlendEnable = FALSE;
	m_BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	m_BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	m_BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	m_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HREXCEPT(m_Device->CreateBlendState(&m_BlendDesc, m_BlendState.GetAddressOf()));

	m_BlendFactor = Math::Color(1,1,1,1);
	m_SampleMask = 0xffffffff;
}

void RenderTarget::Clear() {
	m_Context->ClearRenderTargetView(m_RenderTarget.Get(), DirectX::Colors::Black);
}

void RenderTarget::Clear(const Math::Color& color) {
	m_Context->ClearRenderTargetView(m_RenderTarget.Get(), color);
}

void RenderTarget::SetViewport(float width, float height, float minDepth, float maxDepth) {
	D3D11_VIEWPORT viewport{};
	::ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = minDepth;
	viewport.MaxDepth = maxDepth;
}

void RenderTarget::SetBlendState(D3D11_RENDER_TARGET_BLEND_DESC rtbDesc, 
	                             Math::Color blend, uint32_t sampleMask) {
	m_BlendDesc.RenderTarget[0] = rtbDesc;
	HREXCEPT(m_Device->CreateBlendState(&m_BlendDesc, m_BlendState.ReleaseAndGetAddressOf()));

	m_BlendFactor = blend;
	m_SampleMask = sampleMask;
}
