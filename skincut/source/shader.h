#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXColors.h>


using Microsoft::WRL::ComPtr;



namespace skincut
{
	class Shader
	{
	protected:
		ComPtr<ID3D11Device>				mDevice;
		ComPtr<ID3D11DeviceContext>			mContext;

	public:
		// input layout
		ComPtr<ID3D11InputLayout>			mInputLayout;

		// depth-stencil state
		unsigned int						mStencilRef;
		ComPtr<ID3D11DepthStencilState>		mDepthState;

		// blend state
		unsigned int						mBlendMask;
		const float*						mBlendFactor;
		ComPtr<ID3D11BlendState>			mBlendState;

		// vertex shader
		ComPtr<ID3D11VertexShader>			mVertexShader;
		std::vector<ComPtr<ID3D11Buffer>>	mVertexBuffers;

		// pixel shader
		ComPtr<ID3D11PixelShader>			mPixelShader;
		std::vector<ComPtr<ID3D11Buffer>>	mPixelBuffers;


	public:
		static D3D11_BLEND_DESC				DefaultBlendDesc();
		static D3D11_DEPTH_STENCIL_DESC		DefaultDepthDesc();


	public:
		Shader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const std::wstring vsfile, const std::wstring psfile = L"");

		void InitializeInputLayout(ComPtr<ID3DBlob>& vsblob);
		void InitializeConstantBuffers(ComPtr<ID3DBlob>& blob, std::vector<ComPtr<ID3D11Buffer>>& buffers);

		virtual void InitializeBlendState();
		virtual void InitializeDepthState();

		void SetBlendState(D3D11_BLEND srcBlend = D3D11_BLEND_ONE, 
						   D3D11_BLEND destBlend = D3D11_BLEND_ZERO, 
						   D3D11_BLEND_OP blendOp = D3D11_BLEND_OP_ADD,
						   const float* factor = DirectX::Colors::White, 
						   UINT mask = 0xFFFFFFFF);
		void SetBlendState(D3D11_BLEND_DESC desc, const float* factor, UINT mask);

		void SetDepthState(bool enableDepth = true, bool writeDepth = true, bool enableStencil = false, unsigned int ref = 0);
		void SetDepthState(D3D11_DEPTH_STENCIL_DESC desc, unsigned int ref = 0);
	};
}

