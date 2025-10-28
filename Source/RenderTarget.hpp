#pragma once

#include <wrl/client.h>

#include <dxgi.h>
#include <d3d11.h>

#include "Math.hpp"


namespace SkinCut {
	class RenderTarget {
	public:
		RenderTarget(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
					 Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context, 
					 uint32_t width, uint32_t height, DXGI_FORMAT format, 
					 bool typeless = true);

		RenderTarget(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
					 Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context, 
					 uint32_t width, uint32_t height, DXGI_FORMAT format, 
					 Microsoft::WRL::ComPtr<ID3D11Texture2D>& baseTexture);

		RenderTarget(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			         Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context, 
			         Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture, 
					 DXGI_FORMAT format);

		void Clear();
		void Clear(const Math::Color& color);

		void SetViewport(float width, float height, 
			             float minDepth = 0, 
						 float maxDepth = 1);

		void SetBlendState(D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc, 
			               Math::Color blendColor, 
						   uint32_t sampleMask);

	public:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendState;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResource;

		uint32_t m_SampleMask; // sampling method (0xffffffff = point sampling)
		Math::Color m_BlendFactor;
		D3D11_VIEWPORT m_Viewport;
		D3D11_BLEND_DESC m_BlendDesc;

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
	};
}

