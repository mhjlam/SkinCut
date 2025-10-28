#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


namespace SkinCut {
	class FrameBuffer {
	public:
		FrameBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device,
					Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
					Microsoft::WRL::ComPtr<IDXGISwapChain>& swapChain,
					DXGI_FORMAT depthFormatTex = DXGI_FORMAT_R32_TYPELESS,
					DXGI_FORMAT depthFormatDsv = DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT depthFormatSrv = DXGI_FORMAT_R32_FLOAT);

		FrameBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device,
					Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
					uint32_t width, uint32_t height,
					DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
					DXGI_FORMAT depthFormatTex = DXGI_FORMAT_R32_TYPELESS,
					DXGI_FORMAT depthFormatDsv = DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT depthFormatSrv = DXGI_FORMAT_R32_FLOAT);

		FrameBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device,
					Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
					Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
					DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
					DXGI_FORMAT depthFormatTex = DXGI_FORMAT_R32_TYPELESS,
					DXGI_FORMAT depthFormatDsv = DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT depthFormatSrv = DXGI_FORMAT_R32_FLOAT);
		
		void Clear();
		void Clear(const Math::Color& color);
		
	public:
		D3D11_VIEWPORT Viewport;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> ColorTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ColorBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ColorResource;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DepthResource;
	
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
	};
}
