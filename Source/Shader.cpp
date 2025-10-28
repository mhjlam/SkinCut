#include "Shader.hpp"

#include <cstdint>
#include <d3dcompiler.h>

#include "Util.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


D3D11_BLEND_DESC Shader::DefaultBlendDesc() {
	D3D11_BLEND_DESC blendDesc;
	::ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (uint8_t i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = FALSE;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	return blendDesc;
}

D3D11_DEPTH_STENCIL_DESC Shader::DefaultDepthDesc() {
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	::ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	return depthStencilDesc;
}

Shader::Shader(ComPtr<ID3D11Device>& device, 
	           ComPtr<ID3D11DeviceContext>& context, 
			   const std::wstring vsFile, 
			   const std::wstring psFile) : m_Device(device), m_Context(context) {
	m_StencilRef = 0;

	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;

	// Vertex shader
	if (vsFile.empty()) {
		std::string vsStr = "float4 main(float4 pos : POSITION) : SV_POSITION { return pos; }\n";
		HREXCEPT(D3DCompile(vsStr.c_str(), 
			vsStr.size(), "VS", nullptr, nullptr, "main", "vs_4_0", 0, 0, vsBlob.GetAddressOf(), nullptr));
	}
	else {
		HREXCEPT(D3DReadFileToBlob(vsFile.c_str(), vsBlob.GetAddressOf()));
		InitializeConstantBuffers(vsBlob, m_VertexBuffers);
	}

	InitializeInputLayout(vsBlob);
	HREXCEPT(m_Device->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_VertexShader.GetAddressOf()));


	// Pixel shader
	m_PixelShader = nullptr;
	if (!psFile.empty()) {
		HREXCEPT(D3DReadFileToBlob(psFile.c_str(), psBlob.GetAddressOf()));
		InitializeConstantBuffers(psBlob, m_PixelBuffers);
		HREXCEPT(m_Device->CreatePixelShader(
			psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_PixelShader.GetAddressOf()));
	}

	InitializeBlendState();
	InitializeDepthState();
}

void Shader::SetBlendState(D3D11_BLEND srcBlend, 
	                       D3D11_BLEND destBlend, 
						   D3D11_BLEND_OP blendOp, 
						   const float* factor, uint32_t mask) {
	D3D11_BLEND_DESC blendDesc;
	::ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (uint8_t i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (destBlend != D3D11_BLEND_ZERO);
		blendDesc.RenderTarget[i].SrcBlend = srcBlend;
		blendDesc.RenderTarget[i].SrcBlendAlpha = srcBlend;
		blendDesc.RenderTarget[i].DestBlend = destBlend;
		blendDesc.RenderTarget[i].DestBlendAlpha = destBlend;
		blendDesc.RenderTarget[i].BlendOp = blendOp;
		blendDesc.RenderTarget[i].BlendOpAlpha = blendOp;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	HREXCEPT(m_Device->CreateBlendState(&blendDesc, m_BlendState.ReleaseAndGetAddressOf()));
	m_BlendFactor = factor;
	m_BlendMask = mask;
}

void Shader::SetBlendState(D3D11_BLEND_DESC desc, const float* factor, uint32_t mask) {
	m_Device->CreateBlendState(&desc, m_BlendState.ReleaseAndGetAddressOf());
	m_BlendFactor = factor;
	m_BlendMask = mask;
}

void Shader::SetDepthState(bool enableDepth, bool writeDepth, bool enableStencil, unsigned int ref) {
	D3D11_DEPTH_STENCIL_DESC desc;
	::ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = enableDepth;
	desc.DepthWriteMask = writeDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.StencilEnable = enableStencil;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace = desc.FrontFace;
	m_Device->CreateDepthStencilState(&desc, m_DepthState.ReleaseAndGetAddressOf());

	m_StencilRef = ref;
}

void Shader::SetDepthState(D3D11_DEPTH_STENCIL_DESC desc, unsigned int ref) {
	m_Device->CreateDepthStencilState(&desc, m_DepthState.ReleaseAndGetAddressOf());
	m_StencilRef = ref;
}

void Shader::InitializeInputLayout(ComPtr<ID3DBlob>& vsBlob) {
	// Reflect shader info
	ComPtr<ID3D11ShaderReflection> shaderReflection;
	HREXCEPT(D3DReflect(vsBlob->GetBufferPointer(), 
	                    vsBlob->GetBufferSize(), 
						IID_ID3D11ShaderReflection, 
						(void**)shaderReflection.GetAddressOf()));

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	shaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i) {
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		shaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc{};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

				if (strcmp(elementDesc.SemanticName, "POSITION") == 0) { // hack for float3 position definitions
					elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				}
			}
		}

		// save element desc
		inputLayoutDesc.push_back(elementDesc);
	}       

	// try to create input layout
	HREXCEPT(m_Device->CreateInputLayout(&inputLayoutDesc[0], static_cast<uint32_t>(inputLayoutDesc.size()), 
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_InputLayout.GetAddressOf()));
}

void Shader::InitializeConstantBuffers(ComPtr<ID3DBlob>& blob, std::vector<ComPtr<ID3D11Buffer>>& buffers) {
	// Reflect shader info
	ComPtr<ID3D11ShaderReflection> shaderReflection;
	HREXCEPT(D3DReflect(blob->GetBufferPointer(), 
	                    blob->GetBufferSize(), 
						IID_ID3D11ShaderReflection, 
						(void**)shaderReflection.GetAddressOf()));

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	shaderReflection->GetDesc(&shaderDesc);

	for (uint8_t i = 0; i < shaderDesc.ConstantBuffers; ++i) {
		auto shaderReflectionBuffer = shaderReflection->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
		shaderReflectionBuffer->GetDesc(&shaderBufferDesc);

		// vertex shader constant buffer
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = shaderBufferDesc.Size;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		ComPtr<ID3D11Buffer> buffer;
		HREXCEPT(m_Device->CreateBuffer(&bufferDesc, nullptr, buffer.GetAddressOf()));
		buffers.push_back(buffer);
	}
}

void Shader::InitializeBlendState() {
	m_BlendMask = 0xFFFFFFFF;
	m_BlendFactor = DirectX::Colors::Black;

	D3D11_BLEND_DESC blendDesc = DefaultBlendDesc();
	HREXCEPT(m_Device->CreateBlendState(&blendDesc, m_BlendState.GetAddressOf()));
}

void Shader::InitializeDepthState() {
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = DefaultDepthDesc();
	HREXCEPT(m_Device->CreateDepthStencilState(&depthStencilDesc, m_DepthState.GetAddressOf()));
}
