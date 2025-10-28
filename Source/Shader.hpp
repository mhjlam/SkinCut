#pragma once

#include <string>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXColors.h>


namespace SkinCut {
	class Shader {
	public:
		static D3D11_BLEND_DESC	DefaultBlendDesc();
		static D3D11_DEPTH_STENCIL_DESC	DefaultDepthDesc();

		Shader(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			   Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context, 
			   const std::wstring vsFile, 
			   const std::wstring psFile = L"");

		void SetBlendState(D3D11_BLEND srcBlend = D3D11_BLEND_ONE, 
			               D3D11_BLEND destBlend = D3D11_BLEND_ZERO, 
						   D3D11_BLEND_OP blendOp = D3D11_BLEND_OP_ADD, 
						   const float* factor = DirectX::Colors::White, 
						   uint32_t mask = 0xFFFFFFFF);

		void SetBlendState(D3D11_BLEND_DESC desc, 
			               const float* factor, 
						   uint32_t mask);

		void SetDepthState(bool enableDepth = true, 
			               bool writeDepth = true, 
						   bool enableStencil = false, 
						   unsigned int ref = 0);

		void SetDepthState(D3D11_DEPTH_STENCIL_DESC desc, 
			               unsigned int ref = 0);

	private:
		void InitializeInputLayout(Microsoft::WRL::ComPtr<ID3DBlob>& vsblob);
		
		void InitializeConstantBuffers(Microsoft::WRL::ComPtr<ID3DBlob>& blob, 
			                           std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>>& buffers);

		virtual void InitializeBlendState();
		virtual void InitializeDepthState();

	public:
		// Input Layout
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;

		// Depth-Stencil State
		uint32_t m_StencilRef;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthState;

		// Blend State
		unsigned int m_BlendMask;
		const float* m_BlendFactor;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendState;

		// Vertex Shader
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_VertexBuffers;

		// Pixel Shader
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_PixelBuffers;

	protected:
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	m_Context;
	};
}

