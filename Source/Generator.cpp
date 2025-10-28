#include "Generator.hpp"

#include <wincodec.h>
#include <d3dcompiler.h>

#include <DirectXTex.h>
#include <DDSTextureLoader.h>

#include "Math.hpp"
#include "Util.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Types.hpp"
#include "RenderTarget.hpp"
#include "VertexBuffer.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;



namespace SkinCut {
	extern Configuration g_Config;
}


Generator::Generator(ComPtr<ID3D11Device>& device, 
					 ComPtr<ID3D11DeviceContext>& context) : m_Device(device), m_Context(context) {
	auto ShaderPath = [&](std::wstring name) {
		std::wstring resdir(g_Config.ResourcePath.begin(), g_Config.ResourcePath.end());
		return resdir + L"shaders/" + name;
	};

	m_ShaderStretch = std::make_shared<Shader>(m_Device, m_Context, ShaderPath(L"Stretch.vs.cso"), ShaderPath(L"Stretch.ps.cso"));
	m_ShaderWoundPatch = std::make_shared<Shader>(m_Device, m_Context, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Patch.ps.cso"));
}

std::shared_ptr<RenderTarget> Generator::GenerateStretch(std::shared_ptr<Model>& model, std::wstring outName) {
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

	auto target = std::make_shared<RenderTarget>(m_Device, m_Context, 512, 512, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0F;
	viewport.TopLeftY = 0.0F;
	viewport.Width = 512.0F;
	viewport.Height = 512.0F;
	viewport.MinDepth = 0.0F;
	viewport.MaxDepth = 1.0F;

	ComPtr<ID3D11Buffer> vertexBuffer;
	VS_CBUFFER_DATA vertexBufferData = { 
		model->World, 
		model->World.Invert().Transpose(), 
		model->WorldViewProjection
	};
	D3D11_BUFFER_DESC vertexBufferDesc = { 
		sizeof(vertexBufferData), 
		D3D11_USAGE_DEFAULT, 
		D3D11_BIND_CONSTANT_BUFFER, 
		0, 0, 0
	};
	D3D11_SUBRESOURCE_DATA vertexBufferRes = { &vertexBufferData, 0, 0 };
	m_Device->CreateBuffer(&vertexBufferDesc, &vertexBufferRes, &vertexBuffer);

	ComPtr<ID3D11Buffer> pixelBuffer;
	PS_CBUFFER_DATA pixelBufferData = { Math::Color(1,0,1,1) };
	D3D11_BUFFER_DESC pixelBufferDesc = { 
		sizeof(pixelBufferData), 
		D3D11_USAGE_DEFAULT, 
		D3D11_BIND_CONSTANT_BUFFER, 
		0, 0, 0
	};
	D3D11_SUBRESOURCE_DATA pixelBufferRes = { &pixelBufferData, 0, 0 };
	m_Device->CreateBuffer(&pixelBufferDesc, &pixelBufferRes, &pixelBuffer);
	
	// Input Assembler
	m_Context->IASetInputLayout(m_ShaderStretch->m_InputLayout.Get());
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Context->IASetIndexBuffer(model->IndexBuffer.Get(), model->IndexBufferFormat, model->IndexBufferOffset);
	m_Context->IASetVertexBuffers(0, 1, model->VertexBuffer.GetAddressOf(),
								 &model->VertexBufferStrides, &model->VertexBufferOffset);

	// Vertex Shader
	m_Context->VSSetConstantBuffers(0, 1, vertexBuffer.GetAddressOf());
	m_Context->VSSetShader(m_ShaderStretch->m_VertexShader.Get(), 0, 0);

	// Pixel Shader
	m_Context->PSSetConstantBuffers(0, 1, pixelBuffer.GetAddressOf());
	m_Context->PSSetShader(m_ShaderStretch->m_PixelShader.Get(), 0, 0);

	// Rasterizer
	m_Context->RSSetState(nullptr); // might need to change CullMode to NONE
	m_Context->RSSetViewports(1, &viewport);

	// Output Merger
	m_Context->OMSetRenderTargets(1, target->m_RenderTarget.GetAddressOf(), nullptr);
	
	// Draw
	m_Context->DrawIndexed(model->IndexCount(), 0, 0);
	
	if (!outName.empty()) {
		std::wstring ddsName = outName + L".dds";
		std::wstring pngName = outName + L".png";

		// Retrieve texture from render target
		ComPtr<ID3D11Resource> res;
		ComPtr<ID3D11Texture2D> tex;
		target->m_ShaderResource.Get()->GetResource(res.GetAddressOf());
		res.Get()->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(tex.GetAddressOf()));

		// Save as PNG file
		DirectX::ScratchImage image;
		CaptureTexture(m_Device.Get(), m_Context.Get(), tex.Get(), image);
		SaveToWICFile(*image.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, pngName.c_str(), nullptr);
	}

	return target;
}


std::shared_ptr<RenderTarget> Generator::GenerateWoundPatch(uint32_t width, uint32_t height, std::wstring outName) {
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HREXCEPT(m_Context->Map(m_ShaderWoundPatch->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
	CB_PATCH_PS* patchBuffer = (CB_PATCH_PS*)mappedSubresource.pData;
	patchBuffer->Discolor = Math::Color(0.58f, 0.26f, 0.29f, 1.00f); // float4(0.58, 0.27, 0.28, 1.00);
	patchBuffer->LightColor = Math::Color(0.89f, 0.71f, 0.65f, 1.00f); // float4(0.65, 0.36, 0.37, 1.00);
	patchBuffer->InnerColor = Math::Color(0.54f, 0.00f, 0.01f, 1.00f);
	patchBuffer->OffsetX = Util::Random(0.0f, 100.0f);
	patchBuffer->OffsetY = Util::Random(0.0f, 100.0f);
	m_Context->Unmap(m_ShaderWoundPatch->m_PixelBuffers[0].Get(), 0);

	auto buffer = std::make_unique<VertexBuffer>(m_Device);
	auto target = std::make_shared<RenderTarget>(m_Device, m_Context, width, height, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, false);

	// Input Assembler
	m_Context->IASetInputLayout(m_ShaderWoundPatch->m_InputLayout.Get());
	m_Context->IASetPrimitiveTopology(buffer->m_Topology);
	m_Context->IASetVertexBuffers(0, 1, buffer->m_Buffer.GetAddressOf(), &buffer->m_Strides, &buffer->m_Offsets);

	// Vertex shader
	m_Context->VSSetShader(m_ShaderWoundPatch->m_VertexShader.Get(), nullptr, 0);

	// Pixel shader
	m_Context->PSSetConstantBuffers(0, static_cast<uint32_t>(
		m_ShaderWoundPatch->m_PixelBuffers.size()), 
		m_ShaderWoundPatch->m_PixelBuffers[0].GetAddressOf());
	m_Context->PSSetShader(m_ShaderWoundPatch->m_PixelShader.Get(), nullptr, 0);

	// Rasterizer
	m_Context->RSSetState(nullptr); // default rasterizer
	m_Context->RSSetViewports(1, &target->m_Viewport);

	// Bind render target to Output Merger
	m_Context->OMSetRenderTargets(1, target->m_RenderTarget.GetAddressOf(), nullptr);
	m_Context->OMSetBlendState(target->m_BlendState.Get(), target->m_BlendFactor, target->m_SampleMask);
	m_Context->OMSetDepthStencilState(m_ShaderWoundPatch->m_DepthState.Get(), 0);

	// Draw call
	m_Context->Draw(buffer->m_NumVertices, 0);

	// Unbind render target
	m_Context->OMSetRenderTargets(0, nullptr, nullptr);

	if (!outName.empty()) {
		std::wstring ddsName = outName + L".dds";
		std::wstring pngName = outName + L".png";

		ComPtr<ID3D11Resource> res;
		ComPtr<ID3D11Texture2D> tex;
		target->m_ShaderResource.Get()->GetResource(res.GetAddressOf());
		res.Get()->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(tex.GetAddressOf()));

		DirectX::ScratchImage image;
		HREXCEPT(DirectX::CaptureTexture(m_Device.Get(), m_Context.Get(), tex.Get(), image));
		HREXCEPT(DirectX::SaveToWICFile(*image.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, 
										GUID_ContainerFormatPng, pngName.c_str(), nullptr));
	}

	return target;
}
