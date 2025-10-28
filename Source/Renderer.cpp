#include "Renderer.hpp"

#include <DirectXTex.h>
#include <DDSTextureLoader.h>

#include "Util.hpp"
#include "Math.hpp"
#include "Mesh.hpp"
#include "Types.hpp"
#include "Decal.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "Constants.hpp"
#include "FrameBuffer.hpp"
#include "RenderTarget.hpp"
#include "VertexBuffer.hpp"

#pragma warning(disable: 4996)


using namespace SkinCut;
using namespace DirectX;
using Microsoft::WRL::ComPtr;


namespace SkinCut {
	extern Configuration g_Config;
}


///////////////////////////////////////////////////////////////////////////////
// SETUP

Renderer::Renderer(HWND hwnd, uint32_t width, uint32_t height) : RenderWidth(width), RenderHeight(height) {
	InitializeDevice(hwnd);
	InitializeShaders();
	InitializeSamplers();
	InitializeResources();
	InitializeRasterizer();
	InitializeTargets();
	InitializeKernel();
}


Renderer::~Renderer() {
	// Disable fullscreen before exiting to prevent errors
	SwapChain->SetFullscreenState(FALSE, nullptr);
}


void Renderer::Resize(uint32_t width, uint32_t height) {
	// Update resolution
	RenderWidth = width;
	RenderHeight = height;

	// Release resources
	BackBuffer.reset();
	ScreenBuffer.reset();
	m_Targets.clear();

	// Resize swapchain
	DeviceContext->ClearState();
	HREXCEPT(SwapChain->ResizeBuffers(0, RenderWidth, RenderHeight, DXGI_FORMAT_UNKNOWN, 0));

	// Recreate render targets
	InitializeTargets();
}


void Renderer::Render(std::vector<std::shared_ptr<Model>>& models, std::vector<std::shared_ptr<Light>>& lights, std::shared_ptr<Camera>& camera) {
	for (auto& model : models) {
		if (!model) {
			throw std::exception("Render error: could not find model");
		}

		switch (g_Config.RenderMode) {
			case RenderType::KELEMEN: {
				RenderDepth(model, lights, camera);		// Shadow mapping and depth buffer
				RenderLighting(model, lights, camera);	// Main shading (ambient, diffuse, speculars, bumps, shadows, translucency)
				RenderScattering();						// Screen-space subsurface scattering
				RenderSpeculars();						// Screen-space specular lighting
				break;
			}

			case RenderType::PHONG: {
				RenderBlinnPhong(model, lights, camera);
				break;
			}

			case RenderType::LAMBERT: {
				RenderLambertian(model, lights, camera);
				break;
			}
		}

		// Setup device for UI rendering
		SetRasterizerState(D3D11_FILL_SOLID);
		DeviceContext->OMSetRenderTargets(1, BackBuffer->ColorBuffer.GetAddressOf(), nullptr);
	}
}



///////////////////////////////////////////////////////////////////////////////
// INITIALIZATION

void Renderer::InitializeDevice(HWND hwnd) {
	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	// Initialize swap chain description
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = RenderWidth;
	swapDesc.BufferDesc.Height = RenderHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // sRGB format that supports 8 bits per channel including alpha
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // Image stretching for a given resolution
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapDesc.OutputWindow = hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	// DXGI discard the contents of the back buffer after calling IDXGISwapChain::Present
	swapDesc.Windowed = TRUE;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Enable application to switch modes by calling IDXGISwapChain::ResizeTarget

	// Create Direct3D device, device context, and swapchain.
	HRESULT hresult;
	uint32_t flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	for (auto driver : driverTypes) {
		hresult = D3D11CreateDeviceAndSwapChain(nullptr, driver, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), 
			D3D11_SDK_VERSION, &swapDesc, &SwapChain, &Device, &FeatureLevel, &DeviceContext);

		if (SUCCEEDED(hresult)) {
			DriverType = driver;
			break;
		}
	}

	if (FAILED(hresult)) {
		throw std::exception("Unable to create device and swapchain");
	}
}


void Renderer::InitializeShaders() {
	D3D11_BLEND_DESC bdesc;
	D3D11_DEPTH_STENCIL_DESC dsdesc;

	auto ShaderPath = [&](std::wstring name) {
		std::wstring resourcePath = Util::wstr(g_Config.ResourcePath);
		return resourcePath + L"Shaders\\" + name;
	};

	// initialize depth mapping shader
	auto shaderDepth = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Depth.vs.cso"));
	dsdesc = Shader::DefaultDepthDesc();
	dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	shaderDepth->SetDepthState(dsdesc);

	// initialize Kelemen/Szirmay-Kalos shader
	auto shaderKelemen = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Main.vs.cso"), ShaderPath(L"Main.ps.cso"));
	dsdesc = Shader::DefaultDepthDesc();
	dsdesc.StencilEnable = TRUE;
	dsdesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	shaderKelemen->SetDepthState(dsdesc, 1);

	// initialize subsurface scattering shader
	auto shaderScatter = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Subsurface.ps.cso"));
	bdesc = Shader::DefaultBlendDesc();
	bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	shaderScatter->SetBlendState(bdesc, DirectX::Colors::Black, 0xFFFFFFFF);
	dsdesc = Shader::DefaultDepthDesc();
	dsdesc.DepthEnable = FALSE;
	dsdesc.StencilEnable = TRUE;
	dsdesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	shaderScatter->SetDepthState(dsdesc, 1);

	// initialize specular shader
	auto shaderSpecular = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Specular.ps.cso"));
	bdesc = Shader::DefaultBlendDesc();
	bdesc.RenderTarget[0].BlendEnable = TRUE;
	bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	shaderSpecular->SetBlendState(bdesc, DirectX::Colors::Black, 0xFFFFFFFF);
	dsdesc = Shader::DefaultDepthDesc();
	dsdesc.DepthEnable = FALSE;
	dsdesc.StencilEnable = FALSE;
	shaderSpecular->SetDepthState(dsdesc);

	// initialize decal shader
	auto shaderDecal = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Decal.vs.cso"), ShaderPath(L"Decal.ps.cso"));
	shaderDecal->SetDepthState(false, false); // disable depth testing and writing
	shaderDecal->SetBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);


	// initialize wound patch shaders
	auto shaderPatch = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Patch.ps.cso"));
	auto shaderWound = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Wound.ps.cso"));
	auto shaderDiscolor = std::make_shared<Shader>(Device, DeviceContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Discolor.ps.cso"));


	// initialize alternative shaders (Blinn-Phong and Lambertian)
	//auto shaderPhong = std::make_shared<Shader>(m_Device, m_Context, ShaderPath(L"Phong.vs.cso"), ShaderPath(L"Phong.ps.cso"));
	//auto shaderLambert = std::make_shared<Shader>(m_Device, m_Context, ShaderPath(L"Lambert.vs.cso"), ShaderPath(L"Lambert.ps.cso"));

	//m_Shaders.emplace("phong", shaderPhong);
	//m_Shaders.emplace("lambert", shaderLambert);

	m_Shaders.emplace("decal", shaderDecal);
	m_Shaders.emplace("depth", shaderDepth);
	m_Shaders.emplace("kelemen", shaderKelemen);
	m_Shaders.emplace("scatter", shaderScatter);
	m_Shaders.emplace("specular", shaderSpecular);

	m_Shaders.emplace("patch", shaderPatch);
	m_Shaders.emplace("wound", shaderWound);
	m_Shaders.emplace("discolor", shaderDiscolor);
}


void Renderer::InitializeSamplers() {
	auto samplerPoint = std::make_shared<Sampler>(Device, Sampler::Point());
	auto samplerLinear = std::make_shared<Sampler>(Device, Sampler::Linear());
	auto samplerComparison = std::make_shared<Sampler>(Device, Sampler::Comparison());
	auto samplerAnisotropic = std::make_shared<Sampler>(Device, Sampler::Anisotropic());

	m_Samplers.emplace("point", samplerPoint);
	m_Samplers.emplace("linear", samplerLinear);
	m_Samplers.emplace("comparison", samplerComparison);
	m_Samplers.emplace("anisotropic", samplerAnisotropic);
}


void Renderer::InitializeResources() {
	auto TexturePath = [&](std::string name) {
		return g_Config.ResourcePath + "Textures\\" + name;
	};

	auto decal = std::make_shared<Texture>(Device, TexturePath("Decal.dds"), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false);
	auto beckmann = std::make_shared<Texture>(Device, TexturePath("Beckmann.dds"), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false);
	auto irradiance = std::make_shared<Texture>(Device, TexturePath("Irradiance.dds"),D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true);

	m_Resources.emplace("decal", decal);
	m_Resources.emplace("beckmann", beckmann);
	m_Resources.emplace("irradiance", irradiance);
}


void Renderer::InitializeRasterizer() {
	D3D11_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	HREXCEPT(Device->CreateRasterizerState(&rasterizerDesc, RasterizerState.GetAddressOf()));
	DeviceContext->RSSetState(RasterizerState.Get());
}


void Renderer::InitializeTargets() {
	// Back buffer (including depth-stencil buffer)
	BackBuffer = std::make_shared<FrameBuffer>(Device, DeviceContext, SwapChain, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	
	// Buffer for screen-space rendering
	ScreenBuffer = std::make_shared<VertexBuffer>(Device);

	// Various render targets
	auto targetDepth = std::make_shared<RenderTarget>(Device, DeviceContext, RenderWidth, RenderHeight, DXGI_FORMAT_R32_FLOAT);
	auto targetSpecular = std::make_shared<RenderTarget>(Device, DeviceContext, RenderWidth, RenderHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	auto targetDiscolor = std::make_shared<RenderTarget>(Device, DeviceContext, RenderWidth, RenderHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	m_Targets.emplace("depth", targetDepth);
	m_Targets.emplace("specular", targetSpecular);
	m_Targets.emplace("discolor", targetDiscolor);
}


void Renderer::InitializeKernel() {
	m_Kernel.resize(Constants::KERNEL_SAMPLES);

	Math::Vector3 falloff(0.57f, 0.13f, 0.08f);	// note: cannot be zero
	Math::Vector3 strength(0.78f, 0.70f, 0.75f);


	auto gaussian = [&](float v, float r) {
		float rx = r / falloff.x;						// fine-tune shape of Gaussian (red channel)
		float ry = r / falloff.y;						// fine-tune shape of Gaussian (green channel)
		float rz = r / falloff.z;						// fine-tune shape of Gaussian (blue channel)

		float w = 2.0f * v;								// width of the Gaussian
		float a = 1.0f / (w * float(Constants::PI));	// height of the curve's peak

		// compute the curve of the gaussian
		return Math::Vector3(a * exp(-(rx * rx) / w), a * exp(-(ry * ry) / w), a * exp(-(rz * rz) / w));
	};

	auto Profile = [&](float r) {
		// The red channel of the original skin profile in [d'Eon07] is used for all three channels

		Math::Vector3 profile;
 		//profile += 0.233 * gaussian(0.0064, r); // Omitted since it is considered as directly scattered light
		profile += 0.100f * gaussian(0.0484f, r);
		profile += 0.118f * gaussian(0.1870f, r);
		profile += 0.113f * gaussian(0.5670f, r);
		profile += 0.358f * gaussian(1.9900f, r);
		profile += 0.078f * gaussian(7.4100f, r);
		return profile;
	};


	// compute kernel offsets
	float range = Constants::KERNEL_SAMPLES > 19 ? 3.0f : 2.0f;
	float step = 2.0f * range / (Constants::KERNEL_SAMPLES - 1);
	float width = range*range;

	for (uint8_t i = 0; i < Constants::KERNEL_SAMPLES; ++i) {
		float o = -range + float(i) * step;
		m_Kernel[i].w = range * Math::Sign(o) * (o*o) / width;
	}


	// compute kernel weights
	Math::Vector3 weightSum;

	for (uint8_t i = 0; i < Constants::KERNEL_SAMPLES; ++i) {
		float w0 = 0.0f, w1 = 0.0f;
		if (i > 0) {
			w0 = abs(m_Kernel[i].w - m_Kernel[i - 1].w);
		}
		if (i < Constants::KERNEL_SAMPLES - 1) {
			w1 = abs(m_Kernel[i].w - m_Kernel[i + 1].w);
		}
		float area = (w0 + w1) / 2.0f;

		Math::Vector3 weight = Profile(m_Kernel[i].w) * area;
		weightSum += weight;

		m_Kernel[i].x = weight.x;
		m_Kernel[i].y = weight.y;
		m_Kernel[i].z = weight.z;
	}

	// weights re-normalized to white so that diffuse color map provides final skin tone
	for (uint8_t i = 0; i < Constants::KERNEL_SAMPLES; ++i) {
		m_Kernel[i].x /= weightSum.x;
		m_Kernel[i].y /= weightSum.y;
		m_Kernel[i].z /= weightSum.z;
	}


	// modulate kernel weights by mix factor to determine blur strength
	for (uint8_t i = 0; i < Constants::KERNEL_SAMPLES; ++i) {
		if (i == Constants::KERNEL_SAMPLES/2) { // center sample
			// lerp = x*(1-s) + y*s
			// lerp = 1 * (1 - strength) + weights[i] * strength
			m_Kernel[i].x = (1.0f - strength.x) + m_Kernel[i].x * strength.x;
			m_Kernel[i].y = (1.0f - strength.y) + m_Kernel[i].y * strength.y;
			m_Kernel[i].z = (1.0f - strength.z) + m_Kernel[i].z * strength.z;
		}
		else {
			m_Kernel[i].x = m_Kernel[i].x * strength.x;
			m_Kernel[i].y = m_Kernel[i].y * strength.y;
			m_Kernel[i].z = m_Kernel[i].z * strength.z;
		}
	}
}




///////////////////////////////////////////////////////////////////////////////
// RENDERING

void Renderer::Draw(std::shared_ptr<VertexBuffer>& vertexBuffer, 
					std::shared_ptr<Shader>& shader, 
					D3D11_VIEWPORT viewPort,
					ID3D11DepthStencilView* depthBuffer,
					std::vector<ID3D11RenderTargetView*>& targets,
					std::vector<ID3D11ShaderResourceView*>& resources,
					std::vector<ID3D11SamplerState*>& samplers,
					D3D11_FILL_MODE fillMode) const {
	uint32_t numVertexBuffers = static_cast<uint32_t>(shader->m_VertexBuffers.size());
	uint32_t numPixelBuffers = static_cast<uint32_t>(shader->m_PixelBuffers.size());
	uint32_t numResources = static_cast<uint32_t>(resources.size());
	uint32_t numSamplers = static_cast<uint32_t>(samplers.size());
	uint32_t numTargets = static_cast<uint32_t>(targets.size());

	DeviceContext->IASetInputLayout(shader->m_InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(vertexBuffer->m_Topology);
	DeviceContext->IASetVertexBuffers(0, 1, vertexBuffer->m_Buffer.GetAddressOf(), &vertexBuffer->m_Strides, &vertexBuffer->m_Offsets);

	DeviceContext->VSSetShader(shader->m_VertexShader.Get(), nullptr, 0);
	
	if (numVertexBuffers > 0) {
		DeviceContext->VSSetConstantBuffers(0, numVertexBuffers, shader->m_VertexBuffers[0].GetAddressOf());
	}

	DeviceContext->PSSetShader(shader->m_PixelShader.Get(), nullptr, 0);

	if (numPixelBuffers > 0) {
		DeviceContext->PSSetConstantBuffers(0, numPixelBuffers, shader->m_PixelBuffers[0].GetAddressOf());
	}

	if (numResources > 0) {
		DeviceContext->PSSetShaderResources(0, numResources, &resources[0]);
	}

	if (numSamplers > 0) {
		DeviceContext->PSSetSamplers(0, numSamplers, &samplers[0]);
	}

	DeviceContext->RSSetState(RasterizerState.Get());
	DeviceContext->RSSetViewports(1, &viewPort);

	DeviceContext->OMSetBlendState(shader->m_BlendState.Get(), shader->m_BlendFactor, shader->m_BlendMask);
	DeviceContext->OMSetDepthStencilState(shader->m_DepthState.Get(), shader->m_StencilRef);

	if (numTargets > 0) {
		DeviceContext->OMSetRenderTargets(numTargets, &targets[0], depthBuffer);
	}
	else {
		DeviceContext->OMSetRenderTargets(0, nullptr, depthBuffer);
	}

	DeviceContext->Draw(vertexBuffer->m_NumVertices, 0);
}


void Renderer::Draw(std::shared_ptr<Model>& model, 
					std::shared_ptr<Shader>& shader, 
					std::shared_ptr<FrameBuffer>& frameBuffer,
					std::vector<ID3D11RenderTargetView*>& targets,
					std::vector<ID3D11ShaderResourceView*>& resources,
					std::vector<ID3D11SamplerState*>& samplers,
					D3D11_FILL_MODE fillMode) {
	uint32_t numVertexBuffers = static_cast<uint32_t>(shader->m_VertexBuffers.size());
	uint32_t numPixelBuffers = static_cast<uint32_t>(shader->m_PixelBuffers.size());
	uint32_t numResources = static_cast<uint32_t>(resources.size());
	uint32_t numSamplers = static_cast<uint32_t>(samplers.size());
	uint32_t numTargets = static_cast<uint32_t>(targets.size());

	// input assembler
	DeviceContext->IASetInputLayout(shader->m_InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(model->Topology);
	DeviceContext->IASetVertexBuffers(0, 1, model->VertexBuffer.GetAddressOf(), &model->VertexBufferStrides, &model->VertexBufferOffset);
	DeviceContext->IASetIndexBuffer(model->IndexBuffer.Get(), model->IndexBufferFormat, model->IndexBufferOffset);

	// vertex shader
	DeviceContext->VSSetShader(shader->m_VertexShader.Get(), nullptr, 0);

	if (numVertexBuffers > 0) {
		DeviceContext->VSSetConstantBuffers(0, numVertexBuffers, shader->m_VertexBuffers[0].GetAddressOf());
	}

	// pixel shader
	DeviceContext->PSSetShader(shader->m_PixelShader.Get(), nullptr, 0);

	if (numPixelBuffers > 0) {
		DeviceContext->PSSetConstantBuffers(0, numPixelBuffers, shader->m_PixelBuffers[0].GetAddressOf());
	}
	if (numResources > 0) {
		DeviceContext->PSSetShaderResources(0, numResources, &resources[0]);
	}
	if (numSamplers > 0) {
		DeviceContext->PSSetSamplers(0, numSamplers, &samplers[0]);
	}

	// rasterizer
	SetRasterizerState(fillMode);
	DeviceContext->RSSetViewports(1, &frameBuffer->Viewport);

	// output-merger
	DeviceContext->OMSetBlendState(shader->m_BlendState.Get(), shader->m_BlendFactor, shader->m_BlendMask);
	DeviceContext->OMSetDepthStencilState(shader->m_DepthState.Get(), shader->m_StencilRef);

	if (numTargets > 0) {
		DeviceContext->OMSetRenderTargets(numTargets, &targets[0], frameBuffer->DepthBuffer.Get());
	}
	else {
		DeviceContext->OMSetRenderTargets(0, nullptr, frameBuffer->DepthBuffer.Get());
	}

	DeviceContext->DrawIndexed(model->IndexCount(), 0, 0);
}


void Renderer::RenderDepth(std::shared_ptr<Model>& model, std::vector<std::shared_ptr<Light>>& lights, std::shared_ptr<Camera>& camera) {
	if (!g_Config.EnableShadows) { 
		return;
	}

	auto& shaderDepth = m_Shaders.at("depth");

	for (auto& light : lights) {
		// Skip if light has no intensity
		if (light->Brightness <= 0.0f) { 
			return;
		}

		// Update world view projection matrix
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HREXCEPT(DeviceContext->Map(shaderDepth->m_VertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DEPTH_VS* cbvsShadow = (CB_DEPTH_VS*)mappedResource.pData;
		cbvsShadow->WVP = Math::Matrix(model->World * light->ViewProjectionLinear);
		DeviceContext->Unmap(shaderDepth->m_VertexBuffers[0].Get(), 0);

		// dummy collections
		std::vector<ID3D11RenderTargetView*> targets;
		std::vector<ID3D11ShaderResourceView*> resources;
		std::vector<ID3D11SamplerState*> samplers;

		// clear and draw depth buffer
		ClearBuffer(light->ShadowMap);
		Draw(model, shaderDepth, light->ShadowMap, targets, resources, samplers);
		UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
	}
}


void Renderer::RenderLighting(std::shared_ptr<Model>& model, 
	                          std::vector<std::shared_ptr<Light>>& lights, 
							  std::shared_ptr<Camera>& camera) {
	auto& shaderKsk = m_Shaders.at("kelemen");
	auto& textureBeckmann = m_Resources.at("beckmann");
	auto& textureIrradiance = m_Resources.at("irradiance");
	auto& samplerLinear = m_Samplers.at("linear");
	auto& samplerComparison = m_Samplers.at("comparison");
	auto& samplerAnisotropic = m_Samplers.at("anisotropic");
	auto& targetDepth = m_Targets.at("depth");
	auto& targetSpecular = m_Targets.at("specular");
	auto& targetDiscolor = m_Targets.at("discolor");

	// update world and projection matrices, and camera position
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HREXCEPT(DeviceContext->Map(shaderKsk->m_VertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_LIGHTING_VS* cbvsLighting = (CB_LIGHTING_VS*)mappedResource.pData;
	cbvsLighting->WVP = model->WorldViewProjection;
	cbvsLighting->World = model->World;
	cbvsLighting->WIT = model->World.Invert().Transpose();
	cbvsLighting->Eye = camera->Eye;
	DeviceContext->Unmap(shaderKsk->m_VertexBuffers[0].Get(), 0);
	
	// update lighting properties
	HREXCEPT(DeviceContext->Map(shaderKsk->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_LIGHTING_PS_0* cbps0 = (CB_LIGHTING_PS_0*)mappedResource.pData;
	cbps0->EnableColor = (BOOL)(g_Config.EnableColor && model->ColorMap);
	cbps0->EnableBumps = (BOOL)(g_Config.EnableBumps && model->NormalMap);
	cbps0->EnableShadows = (BOOL)(g_Config.EnableShadows && textureBeckmann);
	cbps0->EnableSpeculars = (BOOL)(g_Config.EnableSpeculars && model->SpecularMap);
	cbps0->EnableOcclusion = (BOOL)(g_Config.EnableOcclusion && model->OcclusionMap);
	cbps0->EnableIrradiance = (BOOL)(g_Config.EnableIrradiance && textureIrradiance);
	cbps0->Ambient = g_Config.Ambient;
	cbps0->Fresnel = g_Config.Fresnel;
	cbps0->Specular = g_Config.Specularity;
	cbps0->Bumpiness = g_Config.Bumpiness;
	cbps0->Roughness = g_Config.Roughness;
	cbps0->ScatterWidth = g_Config.Convolution;
	cbps0->Translucency = g_Config.Translucency;
	DeviceContext->Unmap(shaderKsk->m_PixelBuffers[0].Get(), 0);
	
	// update light structures
	HREXCEPT(DeviceContext->Map(shaderKsk->m_PixelBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_LIGHTING_PS_1* cbps1 = (CB_LIGHTING_PS_1*)mappedResource.pData;
	for (uint8_t i = 0; i < 5; ++i) {
		cbps1->Lights[i].FarPlane = lights[i]->FarPlane;
		cbps1->Lights[i].FalloffStart = lights[i]->FalloffStart;
		cbps1->Lights[i].FalloffWidth = lights[i]->FalloffWidth;
		cbps1->Lights[i].Attenuation = lights[i]->Attenuation;
		cbps1->Lights[i].ColorRGB = lights[i]->Color;
		cbps1->Lights[i].Position = XMFLOAT4(lights[i]->Position.x, lights[i]->Position.y, lights[i]->Position.z, 1);
		cbps1->Lights[i].Direction = XMFLOAT4(lights[i]->Direction.x, lights[i]->Direction.y, lights[i]->Direction.z, 0);
		cbps1->Lights[i].ViewProjection = lights[i]->ViewProjection;
	}
	DeviceContext->Unmap(shaderKsk->m_PixelBuffers[1].Get(), 0);


	// accumulate render targets
	std::vector<ID3D11RenderTargetView*> targets;
	targets.push_back(BackBuffer->ColorBuffer.Get());
	targets.push_back(targetDepth->m_RenderTarget.Get());
	targets.push_back(targetSpecular->m_RenderTarget.Get());
	targets.push_back(targetDiscolor->m_RenderTarget.Get());

	// accumulate pixel shader resources
	std::vector<ID3D11ShaderResourceView*> resources;
	resources.push_back(model->ColorMap.Get());
	resources.push_back(model->NormalMap.Get());
	resources.push_back(model->SpecularMap.Get());
	resources.push_back(model->OcclusionMap.Get());
	resources.push_back(model->DiscolorMap.Get());
	resources.push_back(textureBeckmann->m_ShaderResource.Get());
	resources.push_back(textureIrradiance->m_ShaderResource.Get());

	for (auto& light : lights) {
		resources.push_back(light->ShadowMap->DepthResource.Get());
	}
	
	// accumulate pixel shader samplers
	std::vector<ID3D11SamplerState*> samplers;
	samplers.push_back(samplerLinear->m_SamplerState.Get());
	samplers.push_back(samplerAnisotropic->m_SamplerState.Get());
	samplers.push_back(samplerComparison->m_SamplerState.Get());
	
	// clear render targets and depth buffer
	ClearBuffer(BackBuffer, Math::Color(0.1f, 0.1f, 0.1f, 1.0));
	ClearBuffer(targetDepth);
	ClearBuffer(targetSpecular);
	ClearBuffer(targetDiscolor);

	// draw model
	D3D11_FILL_MODE fillMode = (g_Config.WireframeMode) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	Draw(model, shaderKsk, BackBuffer, targets, resources, samplers, fillMode);

	// remove shader bindings
	UnbindResources(static_cast<uint32_t>(resources.size()));
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderScattering() {
	if (!g_Config.EnableScattering) { 
		return;
	}

	auto& shaderScatter = m_Shaders.at("scatter");
	auto& samplerPoint = m_Samplers.at("point");
	auto& samplerLinear = m_Samplers.at("linear");
	auto& targetDepth = m_Targets.at("depth");
	auto& targetDiscolor = m_Targets.at("discolor");
	
	// Create and clear \ temporary render target
	auto targetTemp = std::make_shared<RenderTarget>(Device, DeviceContext, RenderWidth, RenderHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	// Update field of view, scatter width, and set scatter vector in horizontal direction
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HREXCEPT(DeviceContext->Map(shaderScatter->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_SCATTERING_PS* cbps = (CB_SCATTERING_PS*)mappedResource.pData;
	cbps->FieldOfViewY = Constants::FOV_Y;
	cbps->Width = g_Config.Convolution;
	cbps->Direction = XMFLOAT2(1,0);
	for (uint8_t i = 0; i < m_Kernel.size(); ++i) {
		cbps->Kernel[i] = m_Kernel[i];
	}
	DeviceContext->Unmap(shaderScatter->m_PixelBuffers[0].Get(), 0);

	// Accumulate render targets
	std::vector<ID3D11RenderTargetView*> targets = { targetTemp->m_RenderTarget.Get() };

	// Accumulate pixel shader resources
	std::vector<ID3D11ShaderResourceView*> resources = { BackBuffer->ColorResource.Get(), 
		targetDepth->m_ShaderResource.Get(), targetDiscolor->m_ShaderResource.Get() };
	
	// Accumulate pixel shader samplers
	std::vector<ID3D11SamplerState*> samplers = { samplerPoint->m_SamplerState.Get(), samplerLinear->m_SamplerState.Get() };
	

	// Horizontal scattering (blur filter)
	Draw(ScreenBuffer, shaderScatter, BackBuffer->Viewport, nullptr, targets, resources, samplers);
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));


	// Set scatter vector in vertical direction
	HREXCEPT(DeviceContext->Map(shaderScatter->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	cbps = (CB_SCATTERING_PS*)mappedResource.pData;
	cbps->FieldOfViewY = Constants::FOV_Y;
	cbps->Width = g_Config.Convolution;
	cbps->Direction = XMFLOAT2(0,1);
	for (uint8_t i = 0; i < m_Kernel.size(); ++i) {
		cbps->Kernel[i] = m_Kernel[i];
	}
	DeviceContext->Unmap(shaderScatter->m_PixelBuffers[0].Get(), 0);
	
	targets = { BackBuffer->ColorBuffer.Get() };
	resources = { targetTemp->m_ShaderResource.Get(), targetDepth->m_ShaderResource.Get(), targetDiscolor->m_ShaderResource.Get() };

	// vertical scattering (blur filter)
	Draw(ScreenBuffer, shaderScatter, BackBuffer->Viewport, BackBuffer->DepthBuffer.Get(), targets, resources, samplers);
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderSpeculars() {
	if (!g_Config.EnableSpeculars) { 
		return;
	}
	
	auto& samplerPoint = m_Samplers.at("point");
	auto& shaderSpecular = m_Shaders.at("specular");
	auto& targetSpecular = m_Targets.at("specular");

	std::vector<ID3D11RenderTargetView*> targets = { BackBuffer->ColorBuffer.Get() };
	std::vector<ID3D11ShaderResourceView*> resources = { targetSpecular->m_ShaderResource.Get() };
	std::vector<ID3D11SamplerState*> samplers = { samplerPoint->m_SamplerState.Get() };

	Draw(ScreenBuffer, shaderSpecular, BackBuffer->Viewport, nullptr, targets, resources, samplers);
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::ApplyPatch(std::shared_ptr<Model>& model, 
						  std::shared_ptr<RenderTarget>& patch, 
						  LinkFaceMap& innerFaces, 
						  float cutLength, float cutHeight) {
	auto& shaderWound = m_Shaders.at("wound");
	auto& samplerLinear = m_Samplers.at("linear");

	auto setupVertices = [&](Face*& face) -> std::vector<VertexPositionTexture> {
		Math::Vector2 t0 = model->ModelMesh->m_Vertexes[face->Verts[0]].TexCoord;
		Math::Vector2 t1 = model->ModelMesh->m_Vertexes[face->Verts[1]].TexCoord;
		Math::Vector2 t2 = model->ModelMesh->m_Vertexes[face->Verts[2]].TexCoord;

		Math::Vector3 p0(t0.x * 2.0f - 1.0f, (1.0f - t0.y) * 2.0f - 1.0f, 0.0f);
		Math::Vector3 p1(t1.x * 2.0f - 1.0f, (1.0f - t1.y) * 2.0f - 1.0f, 0.0f);
		Math::Vector3 p2(t2.x * 2.0f - 1.0f, (1.0f - t2.y) * 2.0f - 1.0f, 0.0f);

		VertexPositionTexture v[] = { { p0, t0 }, { p1, t1 }, { p2, t2 } };
		return std::vector<VertexPositionTexture>(&v[0], &v[0] + 3);
	};

	// Acquire texture properties
	D3D11_TEXTURE2D_DESC colorDesc;
	ComPtr<ID3D11Texture2D> colorTex;
	Util::GetTexture2D(model->ColorMap, colorTex, colorDesc);

	// starting texcoord for sampling
	float offset = cutLength * 0.025f; // properly align first segment

	// Create generic vertex buffer
	auto buffer = std::make_unique<VertexBuffer>(Device);
	buffer->m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	// Draw decal to color map
	auto rtColor = std::make_unique<RenderTarget>(Device, DeviceContext, colorDesc.Width, colorDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, colorTex);
	
	// Loop over faces
	for (auto& linkFaces : innerFaces) {
		for (auto& face : linkFaces.second) {
			auto vertices = setupVertices(face);
			buffer->SetVertices(vertices);

			D3D11_MAPPED_SUBRESOURCE msrWound;
			HREXCEPT(DeviceContext->Map(shaderWound->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrWound));
			CB_PAINT_PS* cbpsWound = (CB_PAINT_PS*)msrWound.pData;
			cbpsWound->Point0 = linkFaces.first.TexCoord0;
			cbpsWound->Point1 = linkFaces.first.TexCoord1;
			cbpsWound->Offset = offset;
			
			// properly align last segment
			cbpsWound->CutLength = (linkFaces.first.Rank == innerFaces.size()-1) 
									? cutLength + cutLength * 0.05f : cutLength;
			cbpsWound->CutHeight = cutHeight;
			DeviceContext->Unmap(shaderWound->m_PixelBuffers[0].Get(), 0);

			DeviceContext->IASetInputLayout(shaderWound->m_InputLayout.Get());
			DeviceContext->IASetPrimitiveTopology(buffer->m_Topology);
			DeviceContext->IASetVertexBuffers(0, 1, buffer->m_Buffer.GetAddressOf(), &buffer->m_Strides, &buffer->m_Offsets);
			DeviceContext->VSSetShader(shaderWound->m_VertexShader.Get(), nullptr, 0);
			DeviceContext->PSSetShader(shaderWound->m_PixelShader.Get(), nullptr, 0);
			DeviceContext->PSSetConstantBuffers(0, static_cast<uint32_t>(shaderWound->m_PixelBuffers.size()), shaderWound->m_PixelBuffers[0].GetAddressOf());
			DeviceContext->PSSetShaderResources(0, 1, patch->m_ShaderResource.GetAddressOf());
			DeviceContext->PSSetSamplers(0, 1, samplerLinear->m_SamplerState.GetAddressOf());
			DeviceContext->RSSetViewports(1, &rtColor->m_Viewport);
			DeviceContext->OMSetRenderTargets(1, rtColor->m_RenderTarget.GetAddressOf(), nullptr);
			DeviceContext->OMSetBlendState(rtColor->m_BlendState.Get(), rtColor->m_BlendFactor, rtColor->m_SampleMask);
			DeviceContext->OMSetDepthStencilState(shaderWound->m_DepthState.Get(), shaderWound->m_StencilRef);

			DeviceContext->Draw(buffer->m_NumVertices, 0);
		}

		offset += Math::Vector2::Distance(linkFaces.first.TexCoord0, linkFaces.first.TexCoord1);
	}

	model->ColorMap = rtColor->m_ShaderResource;
}


void Renderer::ApplyDiscolor(std::shared_ptr<Model>& model, LinkFaceMap& outerFaces, float cutheight) {
	auto& shaderDiscolor = m_Shaders.at("discolor");

	auto setupVertices = [&](Face*& face) -> std::vector<VertexPositionTexture> {
		Math::Vector2 t0 = model->ModelMesh->m_Vertexes[face->Verts[0]].TexCoord;
		Math::Vector2 t1 = model->ModelMesh->m_Vertexes[face->Verts[1]].TexCoord;
		Math::Vector2 t2 = model->ModelMesh->m_Vertexes[face->Verts[2]].TexCoord;

		Math::Vector3 p0(t0.x * 2.0f - 1.0f, (1.0f - t0.y) * 2.0f - 1.0f, 0.0f);
		Math::Vector3 p1(t1.x * 2.0f - 1.0f, (1.0f - t1.y) * 2.0f - 1.0f, 0.0f);
		Math::Vector3 p2(t2.x * 2.0f - 1.0f, (1.0f - t2.y) * 2.0f - 1.0f, 0.0f);

		VertexPositionTexture v[] = { { p0, t0 }, { p1, t1 }, { p2, t2 } };
		return std::vector<VertexPositionTexture>(&v[0], &v[0] + 3);
	};

	// Acquire texture properties
	D3D11_TEXTURE2D_DESC discolorDesc;
	ComPtr<ID3D11Texture2D> discolorTex;
	Util::GetTexture2D(model->DiscolorMap, discolorTex, discolorDesc);

	// Create generic vertex buffer
	auto buffer = std::make_unique<VertexBuffer>(Device);
	buffer->m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	// DrawDecal to discolor texture
	auto target = std::make_unique<RenderTarget>(Device, DeviceContext, discolorDesc.Width, discolorDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM, discolorTex);

	// Configure blending
	D3D11_RENDER_TARGET_BLEND_DESC rtbDesc{};
	rtbDesc.BlendEnable = TRUE;
	rtbDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;			// Pixel shader
	rtbDesc.DestBlend = D3D11_BLEND_INV_DEST_COLOR;		// Render target
	rtbDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtbDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtbDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;
	rtbDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	target->SetBlendState(rtbDesc, Math::Color(1,1,1,1), 0xffffffff);

	// Random discoloration color
	Math::Vector4 discolor(Util::Random(0.85f, 0.95f), Util::Random(0.60f, 0.75f), Util::Random(0.60f, 0.85f), 1.0f);
	
	// Loop over faces
	for (auto& linkfaces : outerFaces) {
		for (auto& face : linkfaces.second) {
			auto vertices = setupVertices(face);
			buffer->SetVertices(vertices);

			D3D11_MAPPED_SUBRESOURCE msr_discolor;
			HREXCEPT(DeviceContext->Map(shaderDiscolor->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr_discolor));
			CB_DISCOLOR_PS* cbps_discolor = (CB_DISCOLOR_PS*)msr_discolor.pData;
			cbps_discolor->Discolor = discolor;
			cbps_discolor->Point0 = outerFaces.begin()->first.TexCoord0; // first point of cutting line
			cbps_discolor->Point1 = outerFaces.rbegin()->first.TexCoord1; // final point of cutting line
			cbps_discolor->MaxDistance = cutheight;
			DeviceContext->Unmap(shaderDiscolor->m_PixelBuffers[0].Get(), 0);

			DeviceContext->IASetInputLayout(shaderDiscolor->m_InputLayout.Get());
			DeviceContext->IASetPrimitiveTopology(buffer->m_Topology);
			DeviceContext->IASetVertexBuffers(0, 1, buffer->m_Buffer.GetAddressOf(), &buffer->m_Strides, &buffer->m_Offsets);
			DeviceContext->VSSetShader(shaderDiscolor->m_VertexShader.Get(), nullptr, 0);
			DeviceContext->PSSetShader(shaderDiscolor->m_PixelShader.Get(), nullptr, 0);
			DeviceContext->PSSetConstantBuffers(0, 
				static_cast<uint32_t>(shaderDiscolor->m_PixelBuffers.size()), 
				shaderDiscolor->m_PixelBuffers[0].GetAddressOf());
			DeviceContext->RSSetViewports(1, &target->m_Viewport);
			DeviceContext->OMSetRenderTargets(1, target->m_RenderTarget.GetAddressOf(), nullptr);
			DeviceContext->OMSetBlendState(target->m_BlendState.Get(), target->m_BlendFactor, target->m_SampleMask);
			DeviceContext->OMSetDepthStencilState(shaderDiscolor->m_DepthState.Get(), shaderDiscolor->m_StencilRef);

			DeviceContext->Draw(buffer->m_NumVertices, 0);
		}
	}

	model->DiscolorMap = target->m_ShaderResource;
}



///////////////////////////////////////////////////////////////////////////////
// ALTERNATIVE RENDERERS

void Renderer::RenderBlinnPhong(std::shared_ptr<Model>& model, 
								std::vector<std::shared_ptr<Light>>& lights, 
								std::shared_ptr<Camera>& camera) {
	auto& shaderPhong = m_Shaders.at("phong");
	auto& samplerLinear = m_Samplers.at("linear");

	D3D11_MAPPED_SUBRESOURCE msr;
	HREXCEPT(DeviceContext->Map(shaderPhong->m_VertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_PHONG_VS* cbvs = (CB_PHONG_VS*)msr.pData;
	cbvs->World = model->World;
	cbvs->WIT = model->World.Invert().Transpose();
	cbvs->WVP = model->WorldViewProjection;
	cbvs->ViewPosition = Math::Vector4((float*)camera->Eye);
	cbvs->LightDirection = Math::Vector4(1,-1,0,0);
	DeviceContext->Unmap(shaderPhong->m_VertexBuffers[0].Get(), 0);

	HREXCEPT(DeviceContext->Map(shaderPhong->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_PHONG_PS* phongData = (CB_PHONG_PS*)msr.pData;
	phongData->AmbientColor = 0.1f;
	phongData->DiffuseColor = 0.5f;
	phongData->SpecularColor = 0.5f;
	phongData->SpecularPower = 30.0f;
	phongData->LightColor = Math::Color(1,1,1,1);
	phongData->LightDirection = Math::Vector4(1,-1,0,0);
	DeviceContext->Unmap(shaderPhong->m_PixelBuffers[0].Get(), 0);

	std::vector<ID3D11RenderTargetView*> targets;
	targets.push_back(BackBuffer->ColorBuffer.Get());

	std::vector<ID3D11ShaderResourceView*> resources;
	resources.push_back(model->ColorMap.Get());

	std::vector<ID3D11SamplerState*> samplers;
	samplers.push_back(samplerLinear->m_SamplerState.Get());

	ClearBuffer(BackBuffer, Math::Color(0.1f, 0.1f, 0.1f, 1.0));

	D3D11_FILL_MODE fillmode = (g_Config.WireframeMode) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	Draw(model, shaderPhong, BackBuffer, targets, resources, samplers, fillmode);

	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderLambertian(std::shared_ptr<Model>& model, 
	                            std::vector<std::shared_ptr<Light>>& lights, 
								std::shared_ptr<Camera>& camera) {
	auto& shaderLambert = m_Shaders.at("lambert");

	D3D11_MAPPED_SUBRESOURCE msr;
	HREXCEPT(DeviceContext->Map(shaderLambert->m_VertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_LAMBERTIAN_VS* cbvs = (CB_LAMBERTIAN_VS*)msr.pData;
	cbvs->WIT = model->World.Invert().Transpose();
	cbvs->WVP = model->WorldViewProjection;
	DeviceContext->Unmap(shaderLambert->m_VertexBuffers[0].Get(), 0);

	HREXCEPT(DeviceContext->Map(shaderLambert->m_PixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_LAMBERTIAN_PS* lambertianData = (CB_LAMBERTIAN_PS*)msr.pData;
	lambertianData->AmbientColor = Math::Color(1,1,1,1);
	lambertianData->LightColor = Math::Color(1,1,1,1);
	lambertianData->LightDirection = Math::Vector4(1,-1,0,0);
	DeviceContext->Unmap(shaderLambert->m_PixelBuffers[0].Get(), 0);

	std::vector<ID3D11RenderTargetView*> targets;
	targets.push_back(BackBuffer->ColorBuffer.Get());

	std::vector<ID3D11ShaderResourceView*> resources;
	std::vector<ID3D11SamplerState*> samplers;

	ClearBuffer(BackBuffer, Math::Color(0.1f, 0.1f, 0.1f, 1.0));

	D3D11_FILL_MODE fillmode = (g_Config.WireframeMode) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	Draw(model, shaderLambert, BackBuffer, targets, resources, samplers, fillmode);

	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}



///////////////////////////////////////////////////////////////////////////////
// UTILITY

void Renderer::SetRasterizerState(D3D11_FILL_MODE fillmode, D3D11_CULL_MODE cullmode) {
	D3D11_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = fillmode;
	rasterizerDesc.CullMode = cullmode;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	Device->CreateRasterizerState(&rasterizerDesc, RasterizerState.ReleaseAndGetAddressOf());
	DeviceContext->RSSetState(RasterizerState.Get());
}

void Renderer::SetRasterizerState(D3D11_RASTERIZER_DESC desc) {
	Device->CreateRasterizerState(&desc, RasterizerState.ReleaseAndGetAddressOf());
	DeviceContext->RSSetState(RasterizerState.Get());
}

void Renderer::ClearBuffer(std::shared_ptr<FrameBuffer>& buffer, const Math::Color& color) const {
	DeviceContext->ClearRenderTargetView(buffer->ColorBuffer.Get(), color);
	DeviceContext->ClearDepthStencilView(buffer->DepthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::ClearBuffer(std::shared_ptr<RenderTarget>& target, const Math::Color& color) const {
	DeviceContext->ClearRenderTargetView(target->m_RenderTarget.Get(), color);
}

void Renderer::CopyBuffer(std::shared_ptr<FrameBuffer>& src, std::shared_ptr<FrameBuffer>& dst) const {
	if (!dst) { dst = BackBuffer; }

	if (Util::ValidCopy(src->ColorTexture, dst->ColorTexture)) {
		DeviceContext->CopyResource(dst->ColorTexture.Get(), src->ColorTexture.Get());
	}
}

void Renderer::UnbindResources(uint32_t numViews, uint32_t startSlot) const {
	if (numViews > 0) {
		std::vector<ID3D11ShaderResourceView*> srvs(numViews);
		DeviceContext->PSSetShaderResources(startSlot, numViews, &srvs[0]);
	}
	else {
		DeviceContext->PSSetShaderResources(startSlot, 0, nullptr);
	}
}

void Renderer::UnbindRenderTargets(uint32_t numViews) const {
	if (numViews > 0) {
		std::vector<ID3D11RenderTargetView*> rtvs(numViews);
		DeviceContext->OMSetRenderTargets(numViews, &rtvs[0], nullptr);
	}
	else {
		DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
}
