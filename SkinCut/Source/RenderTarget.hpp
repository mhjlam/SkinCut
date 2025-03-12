#pragma once

#include <wrl/client.h>

#include <dxgi.h>
#include <d3d11.h>

#include "Math.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	// Render target
	class RenderTarget
	{
	private:
		ComPtr<ID3D11Device> mDevice;
		ComPtr<ID3D11DeviceContext> mContext;

	public:
		ComPtr<ID3D11Texture2D> mTexture;
		ComPtr<ID3D11BlendState> mBlendState;
		ComPtr<ID3D11RenderTargetView> mRenderTarget;
		ComPtr<ID3D11ShaderResourceView> mShaderResource;

		uint32_t mSampleMask; // sampling method (0xffffffff = point sampling)
		Math::Color mBlendFactor;
		D3D11_VIEWPORT mViewport;
		D3D11_BLEND_DESC mBlendDesc;


	public:
		RenderTarget(ComPtr<ID3D11Device>& device, 
			   ComPtr<ID3D11DeviceContext>& context, 
			   uint32_t width, uint32_t height, 
			   DXGI_FORMAT format, bool typeless = true);
		RenderTarget(ComPtr<ID3D11Device>& device, 
			   ComPtr<ID3D11DeviceContext>& context, 
			   uint32_t width, uint32_t height, 
			   DXGI_FORMAT format, ComPtr<ID3D11Texture2D>& basetex);
		RenderTarget(ComPtr<ID3D11Device>& device, 
			   ComPtr<ID3D11DeviceContext>& context, 
			   ComPtr<ID3D11Texture2D>& texture, DXGI_FORMAT format);

		void Clear();
		void Clear(const Math::Color& color);

		void SetViewport(float width, float height, float mindepth = 0, float maxdepth = 1);
		void SetBlendState(D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc, Math::Color blendColor, uint32_t sampleMask);
	};
}

