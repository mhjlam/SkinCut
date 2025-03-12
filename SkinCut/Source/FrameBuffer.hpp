#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class FrameBuffer
	{
	private:
		ComPtr<ID3D11Device> mDevice;
		ComPtr<ID3D11DeviceContext> mContext;

	public:
		D3D11_VIEWPORT mViewport;

		ComPtr<ID3D11Texture2D> mColorTexture;
		ComPtr<ID3D11RenderTargetView> mColorBuffer;
		ComPtr<ID3D11ShaderResourceView> mColorResource;

		ComPtr<ID3D11Texture2D> mDepthTexture;
		ComPtr<ID3D11DepthStencilView> mDepthBuffer;
		ComPtr<ID3D11ShaderResourceView> mDepthResource;


	public:
		FrameBuffer(ComPtr<ID3D11Device>& device,
					ComPtr<ID3D11DeviceContext>& context,
					ComPtr<IDXGISwapChain>& swapChain,
					DXGI_FORMAT depthFormatTex = DXGI_FORMAT_R32_TYPELESS,
					DXGI_FORMAT depthFormatDsv = DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT depthFormatSrv = DXGI_FORMAT_R32_FLOAT);

		FrameBuffer(ComPtr<ID3D11Device>& device,
					ComPtr<ID3D11DeviceContext>& context,
					uint32_t width, uint32_t height,
					DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
					DXGI_FORMAT depthFormatTex = DXGI_FORMAT_R32_TYPELESS,
					DXGI_FORMAT depthFormatDsv = DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT depthFormatSrv = DXGI_FORMAT_R32_FLOAT);

		FrameBuffer(ComPtr<ID3D11Device>& device,
					ComPtr<ID3D11DeviceContext>& context,
					ComPtr<ID3D11Texture2D>& texture,
					DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
					DXGI_FORMAT depthFormatTex = DXGI_FORMAT_R32_TYPELESS,
					DXGI_FORMAT depthFormatDsv = DXGI_FORMAT_D32_FLOAT,
					DXGI_FORMAT depthFormatSrv = DXGI_FORMAT_R32_FLOAT);
		
		void Clear();
		void Clear(const Math::Color& color);
	};
}

