#include "Generator.hpp"

#include <wincodec.h>
#include <d3dcompiler.h>

#include <DirectXTex/DirectXTex.h>
#include <DirectXTK/Inc/DDSTextureLoader.h>

#include "Math.hpp"
#include "Util.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Shader.hpp"
#include "Structs.hpp"
#include "RenderTarget.hpp"
#include "VertexBuffer.hpp"



using Microsoft::WRL::ComPtr;
using namespace SkinCut;
using namespace SkinCut::Math;



namespace SkinCut {
	extern Configuration gConfig;
}


Generator::Generator(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context) 
	: mDevice(device), mContext(context)
{
	auto ShaderPath = [&](std::wstring name) {
		std::wstring resdir(gConfig.ResourcePath.begin(), gConfig.ResourcePath.end());
		return resdir + L"shaders/" + name;
	};

	mShaderStretch = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Stretch.vs.cso"), ShaderPath(L"Stretch.ps.cso"));
	mShaderWoundPatch = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Patch.ps.cso"));
}


std::shared_ptr<RenderTarget> Generator::GenerateStretch(std::shared_ptr<Entity>& model, std::wstring outname)
{
	/*
	STRETCH CORRECTION MAP
	After that a map is rendered that holds information about how much the UV layout is stretched, 
	compared to the real dimensions. This is done by unwrapping the mesh into texture space (see below) 
	and saving the the screen space derivatives of the world coordinates for U and V direction, 
	which gives a rough estimation of the stretching. 

	Rendering in texture space:
	To project the mesh into texture space, a vertex shader is used. The position of each vertex 
	is set to its UV coordinate and the world coordinates are stored in an extra texture coordinate. 
	This way the mesh is rendered in its UV layout and the fragment shader can still access the "real" 
	vertex position for lighting and things like that.
	*/

	struct VS_CBUFFER_DATA {
		Math::Matrix World;
		Math::Matrix WorldInverse;
		Math::Matrix WorldViewProjection;
	};

	struct PS_CBUFFER_DATA {
		Math::Color Color;
	};

	auto target = std::make_shared<RenderTarget>(mDevice, mContext, 512, 512, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0F;
	viewport.TopLeftY = 0.0F;
	viewport.Width = 512.0F;
	viewport.Height = 512.0F;
	viewport.MinDepth = 0.0F;
	viewport.MaxDepth = 1.0F;

	ComPtr<ID3D11Buffer> vertexBuffer;
	VS_CBUFFER_DATA vertexBufferData = { model->mMatrixWorld, model->mMatrixWorld.Invert().Transpose(), model->mMatrixWVP };
	D3D11_BUFFER_DESC vertexBufferDesc = { sizeof(vertexBufferData), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0 };
	D3D11_SUBRESOURCE_DATA vertexBufferRes = { &vertexBufferData, 0, 0 };
	mDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferRes, &vertexBuffer);

	ComPtr<ID3D11Buffer> pixelBuffer;
	PS_CBUFFER_DATA pixelBufferData = { Math::Color(1,0,1,1) };
	D3D11_BUFFER_DESC pixelBufferDesc = { sizeof(pixelBufferData), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0 };
	D3D11_SUBRESOURCE_DATA pixelBufferRes = { &pixelBufferData, 0, 0 };
	mDevice->CreateBuffer(&pixelBufferDesc, &pixelBufferRes, &pixelBuffer);
	
	mContext->IASetInputLayout(mShaderStretch->mInputLayout.Get());
	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mContext->IASetIndexBuffer(model->mIndexBuffer.Get(), model->mIndexBufferFormat, model->mIndexBufferOffset);
	mContext->IASetVertexBuffers(0, 1, model->mVertexBuffer.GetAddressOf(), &model->mVertexBufferStrides, &model->mVertexBufferOffset);
	mContext->VSSetConstantBuffers(0, 1, vertexBuffer.GetAddressOf());
	mContext->PSSetConstantBuffers(0, 1, pixelBuffer.GetAddressOf());
	mContext->VSSetShader(mShaderStretch->mVertexShader.Get(), 0, 0);
	mContext->PSSetShader(mShaderStretch->mPixelShader.Get(), 0, 0);
	mContext->RSSetState(nullptr); // might need to change CullMode to NONE
	mContext->RSSetViewports(1, &viewport);
	mContext->OMSetRenderTargets(1, target->mRenderTarget.GetAddressOf(), nullptr);
	
	mContext->DrawIndexed(model->IndexCount(), 0, 0);

	
	if (!outname.empty()) {
		std::wstring ddsName = outname + L".dds";
		std::wstring pngName = outname + L".png";

		ComPtr<ID3D11Resource> tempRes;
		ComPtr<ID3D11Texture2D> tempTex;
		target->mShaderResource.Get()->GetResource(tempRes.GetAddressOf());
		tempRes.Get()->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(tempTex.GetAddressOf()));

		DirectX::ScratchImage image;
		CaptureTexture(mDevice.Get(), mContext.Get(), tempTex.Get(), image);
		//SaveToDDSFile(*image.GetImage(0, 0, 0), DirectX::DDS_FLAGS_NONE, ddsName.c_str());
		SaveToWICFile(*image.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, pngName.c_str(), nullptr);
	}

	return target;
}


std::shared_ptr<RenderTarget> Generator::GenerateWoundPatch(uint32_t width, uint32_t height, std::wstring outname)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HREXCEPT(mContext->Map(mShaderWoundPatch->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
	CB_PATCH_PS* patchBuffer = (CB_PATCH_PS*)mappedSubresource.pData;
	patchBuffer->Discolor = Color(0.58, 0.26, 0.29, 1.00); // float4(0.58, 0.27, 0.28, 1.00);
	patchBuffer->LightColor = Color(0.89, 0.71, 0.65, 1.00); // float4(0.65, 0.36, 0.37, 1.00);
	patchBuffer->InnerColor = Color(0.54, 0.00, 0.01, 1.00);
	patchBuffer->OffsetX = Util::Random(0.0f, 100.0f);
	patchBuffer->OffsetY = Util::Random(0.0f, 100.0f);
	mContext->Unmap(mShaderWoundPatch->mPixelBuffers[0].Get(), 0);

	auto buffer = std::make_unique<VertexBuffer>(mDevice);
	auto target = std::make_shared<RenderTarget>(mDevice, mContext, width, height, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, false);

	mContext->IASetInputLayout(mShaderWoundPatch->mInputLayout.Get());
	mContext->IASetPrimitiveTopology(buffer->mTopology);
	mContext->IASetVertexBuffers(0, 1, buffer->mBuffer.GetAddressOf(), &buffer->mStrides, &buffer->mOffsets);

	mContext->VSSetShader(mShaderWoundPatch->mVertexShader.Get(), nullptr, 0);

	mContext->PSSetConstantBuffers(0, static_cast<uint32_t>(mShaderWoundPatch->mPixelBuffers.size()), mShaderWoundPatch->mPixelBuffers[0].GetAddressOf());
	mContext->PSSetShader(mShaderWoundPatch->mPixelShader.Get(), nullptr, 0);

	mContext->RSSetState(nullptr); // default rasterizer
	mContext->RSSetViewports(1, &target->mViewport);

	mContext->OMSetRenderTargets(1, target->mRenderTarget.GetAddressOf(), nullptr);
	mContext->OMSetBlendState(target->mBlendState.Get(), target->mBlendFactor, target->mSampleMask);
	mContext->OMSetDepthStencilState(mShaderWoundPatch->mDepthState.Get(), 0);

	mContext->Draw(buffer->mVertexCount, 0);

	mContext->OMSetRenderTargets(0, nullptr, nullptr);

	
	if (!outname.empty()) {
		std::wstring ddsName = outname + L".dds";
		std::wstring pngName = outname + L".png";

		ComPtr<ID3D11Resource> tempRes;
		ComPtr<ID3D11Texture2D> tempTex;
		target->mShaderResource.Get()->GetResource(tempRes.GetAddressOf());
		tempRes.Get()->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(tempTex.GetAddressOf()));

		DirectX::ScratchImage image;
		HREXCEPT(DirectX::CaptureTexture(mDevice.Get(), mContext.Get(), tempTex.Get(), image));
		//HREXCEPT(DirectX::SaveToDDSFile(*image.GetImage(0, 0, 0), DirectX::DDS_FLAGS_NONE, ddsName.c_str()));
		HREXCEPT(DirectX::SaveToWICFile(*image.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, pngName.c_str(), nullptr));
	}

	return target;
}

