#include "Renderer.hpp"

#include <wincodec.h>

#include "DirectXTex/DirectXTex.h"
#include "DirectXTK/Inc/DDSTextureLoader.h"

#include "Mesh.hpp"
#include "Decal.hpp"
#include "Light.hpp"
#include "Entity.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Target.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "Utility.hpp"
#include "Generator.hpp"
#include "Structures.hpp"
#include "FrameBuffer.hpp"
#include "Mathematics.hpp"
#include "VertexBuffer.hpp"


#pragma warning(disable: 4996)

constexpr auto KERNEL_SAMPLES = 9; // Must be equal to NUM_SAMPLES in Subsurface.ps.hlsl;


using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace SkinCut;
using namespace SkinCut::Math;



namespace SkinCut
{
	extern Configuration gConfig;
}


///////////////////////////////////////////////////////////////////////////////
// SETUP

Renderer::Renderer(HWND hwnd, uint32_t width, uint32_t height)
: mWidth(width), mHeight(height)
{
	InitializeDevice(hwnd);
	InitializeShaders();
	InitializeSamplers();
	InitializeResources();
	InitializeRasterizer();
	InitializeTargets();
	InitializeKernel();
}


Renderer::~Renderer()
{
	// Disable fullscreen before exiting to prevent errors
	mSwapChain->SetFullscreenState(FALSE, nullptr);
}


void Renderer::Resize(uint32_t width, uint32_t height)
{
	// Update resolution
	mWidth = width;
	mHeight = height;

	// Release resources
	mBackBuffer.reset();
	mScreenBuffer.reset();
	mTargets.clear();

	// Resize swapchain
	mContext->ClearState();
	HREXCEPT(mSwapChain->ResizeBuffers(0, mWidth, mHeight, DXGI_FORMAT_UNKNOWN, 0));

	// Recreate render targets
	InitializeTargets();
}


void Renderer::Render(std::vector<std::shared_ptr<Entity>>& models, std::vector<std::shared_ptr<Light>>& lights, std::unique_ptr<Camera>& camera)
{
	for (auto& model : models) {
		if (!model) {
			throw std::exception("Render error: could not find model");
		}

		switch (gConfig.RenderMode) {
			case RenderType::KELEMEN: {
				RenderDepth(model, lights, camera);		// Shadow mapping and depth buffer
				RenderLighting(model, lights, camera);	// Main shading (ambient, diffuse, speculars, bumps, shadows, translucency)
				RenderScattering();						// Screen-space subsurface scattering
				RenderSpeculars();						// Screen-space specular lighting
				//RenderDecals(camera);
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
		mContext->OMSetRenderTargets(1, mBackBuffer->mColorBuffer.GetAddressOf(), nullptr);
	}
}



///////////////////////////////////////////////////////////////////////////////
// INITIALIZATION

void Renderer::InitializeDevice(HWND hwnd)
{
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
	swapDesc.BufferDesc.Width = mWidth;
	swapDesc.BufferDesc.Height = mHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // sRGB format that supports 8 bits per channel including alpha
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // image stretching for a given resolution
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapDesc.OutputWindow = hwnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	// DXGI discard the contents of the back buffer after calling IDXGISwapChain::Present
	swapDesc.Windowed = TRUE;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // enable application to switch modes by calling IDXGISwapChain::ResizeTarget

	// Create Direct3D device, device context, and swapchain.
	HRESULT hresult;
	uint32_t flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	for (auto driver : driverTypes) {
		hresult = D3D11CreateDeviceAndSwapChain(nullptr, driver, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), 
			D3D11_SDK_VERSION, &swapDesc, &mSwapChain, &mDevice, &mFeatureLevel, &mContext);

		if (SUCCEEDED(hresult)) {
			mDriverType = driver;
			break;
		}
	}

	if (FAILED(hresult)) {
		throw std::exception("Unable to create device and swapchain");
	}
}


void Renderer::InitializeShaders()
{
	D3D11_BLEND_DESC bdesc;
	D3D11_DEPTH_STENCIL_DESC dsdesc;

	auto ShaderPath = [&](std::wstring name) {
		std::wstring resourcePath = Utility::str2wstr(gConfig.ResourcePath);
		return resourcePath + L"Shaders\\" + name;
	};

	// initialize depth mapping shader
	auto shaderDepth = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Depth.vs.cso"));
	dsdesc = Shader::DefaultDepthDesc();
	dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	shaderDepth->SetDepthState(dsdesc);

	// initialize Kelemen/Szirmay-Kalos shader
	auto shaderKelemen = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Main.vs.cso"), ShaderPath(L"Main.ps.cso"));
	dsdesc = Shader::DefaultDepthDesc();
	dsdesc.StencilEnable = TRUE;
	dsdesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	shaderKelemen->SetDepthState(dsdesc, 1);

	// initialize subsurface scattering shader
	auto shaderScatter = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Subsurface.ps.cso"));
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
	auto shaderSpecular = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Specular.ps.cso"));
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
	auto shaderDecal = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Decal.vs.cso"), ShaderPath(L"Decal.ps.cso"));
	shaderDecal->SetDepthState(false, false); // disable depth testing and writing
	shaderDecal->SetBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);


	// initialize wound patch shaders
	auto shaderPatch = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Patch.ps.cso"));
	auto shaderWound = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Wound.ps.cso"));
	auto shaderDiscolor = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Pass.vs.cso"), ShaderPath(L"Discolor.ps.cso"));


	// initialize alternative shaders (Blinn-Phong and Lambertian)
	auto shaderPhong = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Phong.vs.cso"), ShaderPath(L"Phong.ps.cso"));
	auto shaderLambert = std::make_shared<Shader>(mDevice, mContext, ShaderPath(L"Lambert.vs.cso"), ShaderPath(L"Lambert.ps.cso"));


	mShaders.emplace("decal", shaderDecal);
	mShaders.emplace("depth", shaderDepth);
	mShaders.emplace("phong", shaderPhong);
	mShaders.emplace("kelemen", shaderKelemen);
	mShaders.emplace("lambert", shaderLambert);
	mShaders.emplace("scatter", shaderScatter);
	mShaders.emplace("specular", shaderSpecular);

	mShaders.emplace("patch", shaderPatch);
	mShaders.emplace("wound", shaderWound);
	mShaders.emplace("discolor", shaderDiscolor);
}


void Renderer::InitializeSamplers()
{
	auto samplerPoint = std::make_shared<Sampler>(mDevice, Sampler::Point());
	auto samplerLinear = std::make_shared<Sampler>(mDevice, Sampler::Linear());
	auto samplerComparison = std::make_shared<Sampler>(mDevice, Sampler::Comparison());
	auto samplerAnisotropic = std::make_shared<Sampler>(mDevice, Sampler::Anisotropic());

	mSamplers.emplace("point", samplerPoint);
	mSamplers.emplace("linear", samplerLinear);
	mSamplers.emplace("comparison", samplerComparison);
	mSamplers.emplace("anisotropic", samplerAnisotropic);
}


void Renderer::InitializeResources()
{
	auto TexturePath = [&](std::string name) {
		return gConfig.ResourcePath + "Textures\\" + name;
	};

	auto decal = std::make_shared<Texture>(mDevice, TexturePath("Decal.dds"), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false);
	auto beckmann = std::make_shared<Texture>(mDevice, TexturePath("Beckmann.dds"), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false);
	auto irradiance = std::make_shared<Texture>(mDevice, TexturePath("Irradiance.dds"),D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true);

	mResources.emplace("decal", decal);
	mResources.emplace("beckmann", beckmann);
	mResources.emplace("irradiance", irradiance);

// 	Matrix transform = Matrix::CreateScale(0.2) * Matrix::CreateTranslation(0, 0.75, 0);
// 	auto decal = std::shared_ptr<Decal>(new Decal(mDevice, decal, transform, Vector3(0,1,0)));
// 	mDecals.push_back(decal);
}


void Renderer::InitializeRasterizer()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
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
	HREXCEPT(mDevice->CreateRasterizerState(&rasterizerDesc, mRasterizer.GetAddressOf()));
	mContext->RSSetState(mRasterizer.Get());
}


void Renderer::InitializeTargets()
{
	// Back buffer (including depth-stencil buffer)
	mBackBuffer = std::make_shared<FrameBuffer>(mDevice, mContext, mSwapChain, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	
	// Buffer for screen-space rendering
	mScreenBuffer = std::make_shared<VertexBuffer>(mDevice);

	// Various render targets
	auto targetDepth = std::make_shared<Target>(mDevice, mContext, mWidth, mHeight, DXGI_FORMAT_R32_FLOAT);
	auto targetSpecular = std::make_shared<Target>(mDevice, mContext, mWidth, mHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	auto targetDiscolor = std::make_shared<Target>(mDevice, mContext, mWidth, mHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	mTargets.emplace("depth", targetDepth);
	mTargets.emplace("specular", targetSpecular);
	mTargets.emplace("discolor", targetDiscolor);
}


void Renderer::InitializeKernel()
{
	mKernel.resize(KERNEL_SAMPLES);

	Vector3 falloff(0.57f, 0.13f, 0.08f);	// note: cannot be zero
	Vector3 strength(0.78f, 0.70f, 0.75f);


	auto gaussian = [&](float v, float r)
	{
		float rx = r / falloff.x;			// fine-tune shape of Gaussian (red channel)
		float ry = r / falloff.y;			// fine-tune shape of Gaussian (green channel)
		float rz = r / falloff.z;			// fine-tune shape of Gaussian (blue channel)

		float w = 2.0f * v;					// width of the Gaussian
		float a = 1.0f / (w * float(cPI));	// height of the curve's peak

		// compute the curve of the gaussian
		return Vector3(a * exp(-(rx * rx) / w),
		               a * exp(-(ry * ry) / w),
			           a * exp(-(rz * rz) / w));
	};

	auto profile = [&](float r)
	{
		// the red channel of the original skin profile in [d'Eon07] is used for all three channels

		Vector3 profile;
// 		profile += 0.233 * gaussian(0.0064, r); // considered as directly scattered light
		profile += 0.100 * gaussian(0.0484, r);
		profile += 0.118 * gaussian(0.1870, r);
		profile += 0.113 * gaussian(0.5670, r);
		profile += 0.358 * gaussian(1.9900, r);
		profile += 0.078 * gaussian(7.4100, r);
		return profile;
	};


	// compute kernel offsets
	float range = KERNEL_SAMPLES > 19 ? 3.0f : 2.0f;
	float step = 2.0f * range / (KERNEL_SAMPLES - 1);
	float width = range*range;

	for (uint8_t i = 0; i < KERNEL_SAMPLES; ++i) {
		float o = -range + float(i) * step;
		mKernel[i].w = range * Math::Sign(o) * (o*o) / width;
	}


	// compute kernel weights
	Vector3 weight_sum;

	for (uint8_t i = 0; i < KERNEL_SAMPLES; ++i) {
		float w0 = 0.0f, w1 = 0.0f;
		if (i > 0)                  w0 = abs(mKernel[i].w - mKernel[i-1].w);
		if (i < KERNEL_SAMPLES - 1) w1 = abs(mKernel[i].w - mKernel[i+1].w);
		float area = (w0 + w1) / 2.0f;

		Vector3 weight = profile(mKernel[i].w) * area;
		weight_sum += weight;

		mKernel[i].x = weight.x;
		mKernel[i].y = weight.y;
		mKernel[i].z = weight.z;
	}

	// weights re-normalized to white so that diffuse color map provides final skin tone
	for (uint8_t i = 0; i < KERNEL_SAMPLES; ++i) {
		mKernel[i].x /= weight_sum.x;
		mKernel[i].y /= weight_sum.y;
		mKernel[i].z /= weight_sum.z;
	}


	// modulate kernel weights by mix factor to determine blur strength
	for (uint8_t i = 0; i < KERNEL_SAMPLES; ++i) {
		if (i == KERNEL_SAMPLES/2) { // center sample
			// lerp = x*(1-s) + y*s
			// lerp = 1 * (1 - strength) + weights[i] * strength
			mKernel[i].x = (1.0f - strength.x) + mKernel[i].x * strength.x;
			mKernel[i].y = (1.0f - strength.y) + mKernel[i].y * strength.y;
			mKernel[i].z = (1.0f - strength.z) + mKernel[i].z * strength.z;
		}
		else {
			mKernel[i].x = mKernel[i].x * strength.x;
			mKernel[i].y = mKernel[i].y * strength.y;
			mKernel[i].z = mKernel[i].z * strength.z;
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
					D3D11_FILL_MODE fillMode) const
{
	uint32_t numVertexBuffers = static_cast<uint32_t>(shader->mVertexBuffers.size());
	uint32_t numPixelBuffers = static_cast<uint32_t>(shader->mPixelBuffers.size());
	uint32_t numResources = static_cast<uint32_t>(resources.size());
	uint32_t numSamplers = static_cast<uint32_t>(samplers.size());
	uint32_t numTargets = static_cast<uint32_t>(targets.size());

	mContext->IASetInputLayout(shader->mInputLayout.Get());
	mContext->IASetPrimitiveTopology(vertexBuffer->mTopology);
	mContext->IASetVertexBuffers(0, 1, vertexBuffer->mBuffer.GetAddressOf(), &vertexBuffer->mStrides, &vertexBuffer->mOffsets);

	mContext->VSSetShader(shader->mVertexShader.Get(), nullptr, 0);
	
	if (numVertexBuffers > 0) {
		mContext->VSSetConstantBuffers(0, numVertexBuffers, shader->mVertexBuffers[0].GetAddressOf());
	}

	mContext->PSSetShader(shader->mPixelShader.Get(), nullptr, 0);

	if (numPixelBuffers > 0) {
		mContext->PSSetConstantBuffers(0, numPixelBuffers, shader->mPixelBuffers[0].GetAddressOf());
	}

	if (numResources > 0) {
		mContext->PSSetShaderResources(0, numResources, &resources[0]);
	}

	if (numSamplers > 0) {
		mContext->PSSetSamplers(0, numSamplers, &samplers[0]);
	}

	mContext->RSSetState(mRasterizer.Get());
	mContext->RSSetViewports(1, &viewPort);

	mContext->OMSetBlendState(shader->mBlendState.Get(), shader->mBlendFactor, shader->mBlendMask);
	mContext->OMSetDepthStencilState(shader->mDepthState.Get(), shader->mStencilRef);

	if (numTargets > 0) {
		mContext->OMSetRenderTargets(numTargets, &targets[0], depthBuffer);
	}
	else {
		mContext->OMSetRenderTargets(0, nullptr, depthBuffer);
	}

	mContext->Draw(vertexBuffer->mVertexCount, 0);
}


void Renderer::Draw(std::shared_ptr<Entity>& model, 
					std::shared_ptr<Shader>& shader, 
					std::shared_ptr<FrameBuffer>& frameBuffer,
					std::vector<ID3D11RenderTargetView*>& targets,
					std::vector<ID3D11ShaderResourceView*>& resources,
					std::vector<ID3D11SamplerState*>& samplers,
					D3D11_FILL_MODE fillMode)
{
	uint32_t numVertexBuffers = static_cast<uint32_t>(shader->mVertexBuffers.size());
	uint32_t numPixelBuffers = static_cast<uint32_t>(shader->mPixelBuffers.size());
	uint32_t numResources = static_cast<uint32_t>(resources.size());
	uint32_t numSamplers = static_cast<uint32_t>(samplers.size());
	uint32_t numTargets = static_cast<uint32_t>(targets.size());

	// input assembler
	mContext->IASetInputLayout(shader->mInputLayout.Get());
	mContext->IASetPrimitiveTopology(model->mTopology);
	mContext->IASetVertexBuffers(0, 1, model->mVertexBuffer.GetAddressOf(), &model->mVertexBufferStrides, &model->mVertexBufferOffset);
	mContext->IASetIndexBuffer(model->mIndexBuffer.Get(), model->mIndexBufferFormat, model->mIndexBufferOffset);

	// vertex shader
	mContext->VSSetShader(shader->mVertexShader.Get(), nullptr, 0);

	if (numVertexBuffers > 0) {
		mContext->VSSetConstantBuffers(0, numVertexBuffers, shader->mVertexBuffers[0].GetAddressOf());
	}

	// pixel shader
	mContext->PSSetShader(shader->mPixelShader.Get(), nullptr, 0);

	if (numPixelBuffers > 0) {
		mContext->PSSetConstantBuffers(0, numPixelBuffers, shader->mPixelBuffers[0].GetAddressOf());
	}

	if (numResources > 0) {
		mContext->PSSetShaderResources(0, numResources, &resources[0]);
	}

	if (numSamplers > 0) {
		mContext->PSSetSamplers(0, numSamplers, &samplers[0]);
	}

	// rasterizer
	SetRasterizerState(fillMode);
	mContext->RSSetViewports(1, &frameBuffer->mViewport);

	// output-merger
	mContext->OMSetBlendState(shader->mBlendState.Get(), shader->mBlendFactor, shader->mBlendMask);
	mContext->OMSetDepthStencilState(shader->mDepthState.Get(), shader->mStencilRef);

	if (numTargets > 0) {
		mContext->OMSetRenderTargets(numTargets, &targets[0], frameBuffer->mDepthBuffer.Get());
	}
	else {
		mContext->OMSetRenderTargets(0, nullptr, frameBuffer->mDepthBuffer.Get());
	}

	mContext->DrawIndexed(model->IndexCount(), 0, 0);
}


void Renderer::RenderDepth(std::shared_ptr<Entity>& model, std::vector<std::shared_ptr<Light>>& lights, std::unique_ptr<Camera>& camera)
{
	if (!gConfig.EnableShadows) { return; }

	auto& shaderDepth = mShaders.at("depth");

	for (auto& light : lights) {
		// skip if light has no intensity
		if (light->mBrightness <= 0.0f) { return; }

		// update world view projection matrix
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HREXCEPT(mContext->Map(shaderDepth->mVertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DEPTH_VS* cbvsShadow = (CB_DEPTH_VS*)mappedResource.pData;
		cbvsShadow->WVP = Matrix(model->mMatrixWorld * light->mViewProjectionLinear);
		mContext->Unmap(shaderDepth->mVertexBuffers[0].Get(), 0);

		// dummy collections
		std::vector<ID3D11RenderTargetView*> targets;
		std::vector<ID3D11ShaderResourceView*> resources;
		std::vector<ID3D11SamplerState*> samplers;

		// clear and draw depth buffer
		ClearBuffer(light->mShadowMap);
		Draw(model, shaderDepth, light->mShadowMap, targets, resources, samplers);
		UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
	}
}


void Renderer::RenderLighting(std::shared_ptr<Entity>& model, std::vector<std::shared_ptr<Light>>& lights, std::unique_ptr<Camera>& camera)
{
	auto& shaderKelemen = mShaders.at("kelemen");

	auto& textureBeckmann = mResources.at("beckmann");
	auto& textureIrradiance = mResources.at("irradiance");

	auto& samplerLinear = mSamplers.at("linear");
	auto& samplerComparison = mSamplers.at("comparison");
	auto& samplerAnisotropic = mSamplers.at("anisotropic");

	auto& targetDepth = mTargets.at("depth");
	auto& targetSpecular = mTargets.at("specular");
	auto& targetDiscolor = mTargets.at("discolor");

	// update world and projection matrices, and camera position
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HREXCEPT(mContext->Map(shaderKelemen->mVertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_LIGHTING_VS* cbvsLighting = (CB_LIGHTING_VS*)mappedResource.pData;
	cbvsLighting->WVP = model->mMatrixWVP;
	cbvsLighting->World = model->mMatrixWorld;
	cbvsLighting->WorldIT = model->mMatrixWorld.Invert().Transpose();
	cbvsLighting->Eye = camera->mEye;
	mContext->Unmap(shaderKelemen->mVertexBuffers[0].Get(), 0);
	
	// update lighting properties
	HREXCEPT(mContext->Map(shaderKelemen->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_LIGHTING_PS_0* cbps0 = (CB_LIGHTING_PS_0*)mappedResource.pData;
	cbps0->EnableColor = (BOOL)(gConfig.EnableColor && model->mColorMap);
	cbps0->EnableBumps = (BOOL)(gConfig.EnableBumps && model->mNormalMap);
	cbps0->EnableShadows = (BOOL)(gConfig.EnableShadows && textureBeckmann);
	cbps0->EnableSpeculars = (BOOL)(gConfig.EnableSpeculars && model->mSpecularMap);
	cbps0->EnableOcclusion = (BOOL)(gConfig.EnableOcclusion && model->mOcclusionMap);
	cbps0->EnableIrradiance = (BOOL)(gConfig.EnableIrradiance && textureIrradiance);
	cbps0->Ambient = gConfig.Ambient;
	cbps0->Fresnel = gConfig.Fresnel;
	cbps0->Specular = gConfig.Specularity;
	cbps0->Bumpiness = gConfig.Bumpiness;
	cbps0->Roughness = gConfig.Roughness;
	cbps0->ScatterWidth = gConfig.Convolution;
	cbps0->Translucency = gConfig.Translucency;
	mContext->Unmap(shaderKelemen->mPixelBuffers[0].Get(), 0);
	
	// update light structures
	HREXCEPT(mContext->Map(shaderKelemen->mPixelBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_LIGHTING_PS_1* cbps1 = (CB_LIGHTING_PS_1*)mappedResource.pData;
	for (uint8_t i = 0; i < 5; ++i) {
		cbps1->Lights[i].FarPlane = lights[i]->mFarPlane;
		cbps1->Lights[i].FalloffStart = lights[i]->mFalloffStart;
		cbps1->Lights[i].FalloffWidth = lights[i]->mFalloffWidth;
		cbps1->Lights[i].Attenuation = lights[i]->mAttenuation;
		cbps1->Lights[i].ColorRGB = lights[i]->mColor;
		cbps1->Lights[i].Position = XMFLOAT4(lights[i]->mPosition.x, lights[i]->mPosition.y, lights[i]->mPosition.z, 1);
		cbps1->Lights[i].Direction = XMFLOAT4(lights[i]->mDirection.x, lights[i]->mDirection.y, lights[i]->mDirection.z, 0);
		cbps1->Lights[i].ViewProjection = lights[i]->mViewProjection;
	}
	mContext->Unmap(shaderKelemen->mPixelBuffers[1].Get(), 0);


	// accumulate render targets
	std::vector<ID3D11RenderTargetView*> targets;
	targets.push_back(mBackBuffer->mColorBuffer.Get());
	targets.push_back(targetDepth->mRenderTarget.Get());
	targets.push_back(targetSpecular->mRenderTarget.Get());
	targets.push_back(targetDiscolor->mRenderTarget.Get());

	// accumulate pixel shader resources
	std::vector<ID3D11ShaderResourceView*> resources;
	resources.push_back(model->mColorMap.Get());
	resources.push_back(model->mNormalMap.Get());
	resources.push_back(model->mSpecularMap.Get());
	resources.push_back(model->mOcclusionMap.Get());
	resources.push_back(model->mDiscolorMap.Get());
	resources.push_back(textureBeckmann->mShaderResource.Get());
	resources.push_back(textureIrradiance->mShaderResource.Get());
	for (uint8_t i = 0; i < 5; ++i) {
		resources.push_back(lights[i]->mShadowMap->mDepthResource.Get());
	}
	
	// accumulate pixel shader samplers
	std::vector<ID3D11SamplerState*> samplers;
	samplers.push_back(samplerLinear->mSamplerState.Get());
	samplers.push_back(samplerAnisotropic->mSamplerState.Get());
	samplers.push_back(samplerComparison->mSamplerState.Get());
	
	// clear render targets and depth buffer
	ClearBuffer(mBackBuffer, Color(0.1f, 0.1f, 0.1f, 1.0));
	ClearBuffer(targetDepth);
	ClearBuffer(targetSpecular);
	ClearBuffer(targetDiscolor);

	// draw model
	D3D11_FILL_MODE fillmode = (gConfig.EnableWireframe) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	Draw(model, shaderKelemen, mBackBuffer, targets, resources, samplers, fillmode);

	// remove shader bindings
	UnbindResources(static_cast<uint32_t>(resources.size()));
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderScattering()
{
	if (!gConfig.EnableScattering) { return; }

	auto& shaderScatter = mShaders.at("scatter");

	auto& samplerPoint = mSamplers.at("point");
	auto& samplerLinear = mSamplers.at("linear");

	auto& targetDepth = mTargets.at("depth");
	auto& targetDiscolor = mTargets.at("discolor");
	
	// create and clear new temporary render target
	auto targetTemp = std::make_shared<Target>(mDevice, mContext, mWidth, mHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);


	// update field of view, scatter width, and set scatter vector in horizontal direction
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HREXCEPT(mContext->Map(shaderScatter->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_SCATTERING_PS* cbps = (CB_SCATTERING_PS*)mappedResource.pData;
	cbps->FOVY = Camera::cFieldOfView;
	cbps->Width = gConfig.Convolution;
	cbps->Direction = XMFLOAT2(1,0);
	for (uint8_t i = 0; i < mKernel.size(); ++i) {
		cbps->Kernel[i] = mKernel[i];
	}
	mContext->Unmap(shaderScatter->mPixelBuffers[0].Get(), 0);

	// accumulate render targets
	std::vector<ID3D11RenderTargetView*> targets = {
		targetTemp->mRenderTarget.Get()
	};

	// accumulate pixel shader resources
	std::vector<ID3D11ShaderResourceView*> resources = {
		mBackBuffer->mColorResource.Get(),
		targetDepth->mShaderResource.Get(),
		targetDiscolor->mShaderResource.Get()
	};
	
	// accumulate pixel shader samplers
	std::vector<ID3D11SamplerState*> samplers = {
		samplerPoint->mSamplerState.Get(),
		samplerLinear->mSamplerState.Get()
	};
	

	// horizontal scattering (blur filter)
	Draw(mScreenBuffer, shaderScatter, mBackBuffer->mViewport, nullptr, targets, resources, samplers);
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));


	// set scatter vector in vertical direction
	HREXCEPT(mContext->Map(shaderScatter->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	cbps = (CB_SCATTERING_PS*)mappedResource.pData;
	cbps->FOVY = Camera::cFieldOfView;
	cbps->Width = gConfig.Convolution;
	cbps->Direction = XMFLOAT2(0,1);
	for (uint8_t i = 0; i < mKernel.size(); ++i) {
		cbps->Kernel[i] = mKernel[i];
	}
	mContext->Unmap(shaderScatter->mPixelBuffers[0].Get(), 0);
	
	targets = {
		mBackBuffer->mColorBuffer.Get()
	};

	resources = {
		targetTemp->mShaderResource.Get(),
		targetDepth->mShaderResource.Get(),
		targetDiscolor->mShaderResource.Get()
	};

	// vertical scattering (blur filter)
	Draw(mScreenBuffer, shaderScatter, mBackBuffer->mViewport, mBackBuffer->mDepthBuffer.Get(), targets, resources, samplers);
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderSpeculars()
{
	if (!gConfig.EnableSpeculars) { return; }
	
	auto& samplerPoint = mSamplers.at("point");
	auto& shaderSpecular = mShaders.at("specular");
	auto& targetSpecular = mTargets.at("specular");

	std::vector<ID3D11RenderTargetView*> targets = {
		mBackBuffer->mColorBuffer.Get()
	};

	std::vector<ID3D11ShaderResourceView*> resources = {
		targetSpecular->mShaderResource.Get()
	};

	std::vector<ID3D11SamplerState*> samplers = {
		samplerPoint->mSamplerState.Get()
	};

	Draw(mScreenBuffer, shaderSpecular, mBackBuffer->mViewport, nullptr, targets, resources, samplers);
	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderDecals(std::unique_ptr<Camera>& camera)
{
	// SCREEN-SPACE DECALS:
	// 1. DrawDecal underlying geometries onto scene
	// 2. Rasterize a screen-space decal (SSD) box
	// 3. Read the depth buffer for each pixel
	// 4. Calculate 3D position from the depth
	// 5. Reject if this position is outside the box
	// 6. Otherwise: draw pixel with decal texture

	if (mDecals.size() == 0) { return; }

	auto& shaderDecal = mShaders.at("decal");
	auto& samplerLinear = mSamplers.at("linear");

	// copy depth information to temporary buffer
	ComPtr<ID3D11Texture2D> tempTex;
	ComPtr<ID3D11ShaderResourceView> tempSrv;

	D3D11_TEXTURE2D_DESC texDesc;
	mBackBuffer->mDepthTexture->GetDesc(&texDesc);
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	mDevice->CreateTexture2D(&texDesc, nullptr, tempTex.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	mBackBuffer->mDepthResource->GetDesc(&srvDesc);
	mDevice->CreateShaderResourceView(tempTex.Get(), &srvDesc, tempSrv.GetAddressOf());

	mContext->CopyResource(tempTex.Get(), mBackBuffer->mDepthTexture.Get());
	

	for (auto& decal : mDecals) {
		// set shader buffers
		D3D11_MAPPED_SUBRESOURCE msr;
		HREXCEPT(mContext->Map(shaderDecal->mVertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
		CB_DECAL_VS* cbvs = (CB_DECAL_VS*)msr.pData;
		cbvs->World = decal->mWorldMatrix;
		cbvs->View = camera->mView;
		cbvs->Projection = camera->mProjection;
		cbvs->DecalNormal = Vector4((float*)decal->mNormal);
		mContext->Unmap(shaderDecal->mVertexBuffers[0].Get(), 0);

		HREXCEPT(mContext->Map(shaderDecal->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
		CB_DECAL_PS* decalData = (CB_DECAL_PS*)msr.pData;
		decalData->InvWorld = Utility::MatrixInverse(decal->mWorldMatrix);
		decalData->InvView = Utility::MatrixInverse(camera->mView);
		decalData->InvProject = Utility::MatrixInverse(camera->mProjection);
		mContext->Unmap(shaderDecal->mPixelBuffers[0].Get(), 0);

		std::vector<ID3D11RenderTargetView*> targets = {
			mBackBuffer->mColorBuffer.Get()
		};

		std::vector<ID3D11ShaderResourceView*> resources = {
			tempSrv.Get(),
			decal->mTexture->mShaderResource.Get()
		};

		std::vector<ID3D11SamplerState*> samplers = {
			samplerLinear->mSamplerState.Get()
		};

		uint32_t numVertexBuffers = static_cast<uint32_t>(shaderDecal->mVertexBuffers.size());
		uint32_t numPixelBuffers = static_cast<uint32_t>(shaderDecal->mPixelBuffers.size());
		uint32_t numResources = static_cast<uint32_t>(resources.size());
		uint32_t numSamplers = static_cast<uint32_t>(samplers.size());

		mContext->IASetInputLayout(shaderDecal->mInputLayout.Get());
		mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mContext->IASetIndexBuffer(decal->mIndexBuffer.Get(), decal->mIndexBufferFormat, decal->mIndexBufferOffset);
		mContext->IASetVertexBuffers(0, 1, decal->mVertexBuffer.GetAddressOf(), &decal->mVertexBufferStrides, &decal->mVertexBufferOffset);

		mContext->VSSetShader(shaderDecal->mVertexShader.Get(), nullptr, 0);
		mContext->VSSetConstantBuffers(0, numVertexBuffers, shaderDecal->mVertexBuffers[0].GetAddressOf());

		mContext->PSSetShader(shaderDecal->mPixelShader.Get(), nullptr, 0);
		mContext->PSSetConstantBuffers(0, numPixelBuffers, shaderDecal->mPixelBuffers[0].GetAddressOf());
		mContext->PSSetShaderResources(0, numResources, &resources[0]);
		mContext->PSSetSamplers(0, numSamplers, &samplers[0]);

		mContext->RSSetState(mRasterizer.Get());
		mContext->RSSetViewports(1, &mBackBuffer->mViewport);

		mContext->OMSetRenderTargets(1, mBackBuffer->mColorBuffer.GetAddressOf(), mBackBuffer->mDepthBuffer.Get());
		mContext->OMSetDepthStencilState(shaderDecal->mDepthState.Get(), shaderDecal->mStencilRef);
		mContext->OMSetBlendState(shaderDecal->mBlendState.Get(), shaderDecal->mBlendFactor, shaderDecal->mBlendMask);

		mContext->DrawIndexed(decal->mIndexCount, 0, 0);

		UnbindRenderTargets(0);
		UnbindResources(numResources);
	}
}


void Renderer::CreateWoundDecal(Intersection& ix)
{
	auto& decalTexture = mResources.at("decal");

	// create rotation matrix from direction vector
	Vector3 position = ix.pos_ws;
	Vector3 dir = Vector3::Normalize(-ix.ray.direction);

	Vector3 forward = Vector3::Normalize(Vector3(1,0,0));
// 	Vector3 right = Vector3::Normalize(Vector3::Cross(dir, Vector3(0,1,0)));
// 	Vector3 up = Vector3::Cross(right, forward);
// 
// 	Matrix rotation = Matrix::Identity();
// 	rotation.Forward(dir);
// 	rotation.Right(right);
// 	rotation.Up(up);

// 	float r = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
// 	float t = std::atan2(dir.y, dir.x);
// 	float p = std::acos(dir.z / r);
// 	Matrix rotation = Matrix::CreateRotationX(t) * Matrix::CreateRotationZ(p);

	float yaw = std::atan2f(dir.x, dir.y);
	float pitch = std::atan2f(dir.z, std::sqrtf((dir.x * dir.x) + (dir.y * dir.y)));
	Matrix rotation = Matrix::CreateRotationY(yaw) * Matrix::CreateRotationX(pitch);

// 	Matrix rotation = DirectX::XMMatrixLookAtLH(Vector3(0,0,0), forward, Vector3(0,1,0));
// 	rotation.Transpose();
	//Matrix rotation = Matrix::CreateLookAt(Vector3::Null(), Vector3(0,0,1), Vector3(0,1,0)).Transpose();
	//Matrix rotation = Matrix::CreateRotationX(-1.57f);
	Matrix translation = Matrix::CreateTranslation(ix.pos_ws);
	Matrix scaling = Matrix::CreateScale(0.2);

	Matrix transform = scaling * rotation * translation;
	auto decal = std::make_shared<Decal>(mDevice, decalTexture, transform, dir);
	mDecals.push_back(decal);
}



void Renderer::CreateWoundDecal(Intersection& i0, Intersection& i1)
{
	// acquire center position, scale, and average normal from intersections
	Vector3 position = Vector3::Lerp(i0.pos_ws, i1.pos_ws, 0.5f);
	Vector3 normal = -Vector3::Normalize(Vector3::Lerp(i0.ray.direction, i1.ray.direction, 0.5f));

	auto& decalTexture = mResources.at("decal");

	Matrix transform = Matrix::CreateScale(0.2) * Matrix::CreateTranslation(position);
	auto decal = std::make_shared<Decal>(mDevice, decalTexture, transform, normal);
	mDecals.push_back(decal);
}


void Renderer::PaintWoundPatch(std::shared_ptr<Entity>& model, std::shared_ptr<Target>& patch, std::map<Link, std::vector<Face*>>& innerFaces, float cutLength, float cutheight)
{
	auto& shaderWound = mShaders.at("wound");
	auto& samplerLinear = mSamplers.at("linear");

	auto SetupVertices = [&](Face*& face) -> std::vector<VertexPositionTexture> {
		Vector2 t0 = model->mMesh->mVertexes[face->v[0]].texcoord;
		Vector2 t1 = model->mMesh->mVertexes[face->v[1]].texcoord;
		Vector2 t2 = model->mMesh->mVertexes[face->v[2]].texcoord;

		Vector3 p0 = Vector3(t0.x * 2.0f - 1.0f, (1.0f - t0.y) * 2.0f - 1.0f, 0.0f);
		Vector3 p1 = Vector3(t1.x * 2.0f - 1.0f, (1.0f - t1.y) * 2.0f - 1.0f, 0.0f);
		Vector3 p2 = Vector3(t2.x * 2.0f - 1.0f, (1.0f - t2.y) * 2.0f - 1.0f, 0.0f);

		VertexPositionTexture v[] = { { p0, t0 }, { p1, t1 }, { p2, t2 } };
		return std::vector<VertexPositionTexture>(&v[0], &v[0] + 3);
	};

	// Acquire texture properties
	D3D11_TEXTURE2D_DESC colorDesc;
	ComPtr<ID3D11Texture2D> colorTex;
	Utility::GetTexture2D(model->mColorMap, colorTex, colorDesc);

	// starting texcoord for sampling
	float offset = cutLength * 0.025f; // properly align first segment

	// Create generic vertex buffer
	auto buffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(mDevice));
	buffer->mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	// DrawDecal to color map
	auto rtColor = std::unique_ptr<Target>(new Target(mDevice, mContext, colorDesc.Width, colorDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, colorTex));
	
	// Loop over faces
	for (auto& linkfaces : innerFaces) {
		for (auto& face : linkfaces.second) {
			auto vertices = SetupVertices(face);
			buffer->SetVertices(vertices);

			D3D11_MAPPED_SUBRESOURCE msr_wound;
			HREXCEPT(mContext->Map(shaderWound->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr_wound));
			CB_PAINT_PS* cbps_wound = (CB_PAINT_PS*)msr_wound.pData;
			cbps_wound->P0 = linkfaces.first.x0;
			cbps_wound->P1 = linkfaces.first.x1;
			cbps_wound->Offset = offset;
			cbps_wound->CutLength = (linkfaces.first.rank == innerFaces.size()-1) ? cutLength + cutLength * 0.05f : cutLength; // properly align last segment
			cbps_wound->CutHeight = cutheight;
			mContext->Unmap(shaderWound->mPixelBuffers[0].Get(), 0);

			mContext->IASetInputLayout(shaderWound->mInputLayout.Get());
			mContext->IASetPrimitiveTopology(buffer->mTopology);
			mContext->IASetVertexBuffers(0, 1, buffer->mBuffer.GetAddressOf(), &buffer->mStrides, &buffer->mOffsets);
			mContext->VSSetShader(shaderWound->mVertexShader.Get(), nullptr, 0);
			mContext->PSSetShader(shaderWound->mPixelShader.Get(), nullptr, 0);
			mContext->PSSetConstantBuffers(0, static_cast<uint32_t>(shaderWound->mPixelBuffers.size()), shaderWound->mPixelBuffers[0].GetAddressOf());
			mContext->PSSetShaderResources(0, 1, patch->mShaderResource.GetAddressOf());
			mContext->PSSetSamplers(0, 1, samplerLinear->mSamplerState.GetAddressOf());
			mContext->RSSetViewports(1, &rtColor->mViewport);
			mContext->OMSetRenderTargets(1, rtColor->mRenderTarget.GetAddressOf(), nullptr);
			mContext->OMSetBlendState(rtColor->mBlendState.Get(), rtColor->mBlendFactor, rtColor->mSampleMask);
			mContext->OMSetDepthStencilState(shaderWound->mDepthState.Get(), shaderWound->mStencilRef);

			mContext->Draw(buffer->mVertexCount, 0);
		}

		offset += Vector2::Distance(linkfaces.first.x0, linkfaces.first.x1);
	}

	model->mColorMap = rtColor->mShaderResource;
}


void Renderer::PaintDiscoloration(std::shared_ptr<Entity>& model, std::map<Link, std::vector<Face*>>& outerFaces, float cutheight)
{
	auto& shaderDiscolor = mShaders.at("discolor");

	auto SetupVertices = [&](Face*& face) -> std::vector<VertexPositionTexture> {
		Vector2 t0 = model->mMesh->mVertexes[face->v[0]].texcoord;
		Vector2 t1 = model->mMesh->mVertexes[face->v[1]].texcoord;
		Vector2 t2 = model->mMesh->mVertexes[face->v[2]].texcoord;

		Vector3 p0 = Vector3(t0.x * 2.0f - 1.0f, (1.0f - t0.y) * 2.0f - 1.0f, 0.0f);
		Vector3 p1 = Vector3(t1.x * 2.0f - 1.0f, (1.0f - t1.y) * 2.0f - 1.0f, 0.0f);
		Vector3 p2 = Vector3(t2.x * 2.0f - 1.0f, (1.0f - t2.y) * 2.0f - 1.0f, 0.0f);

		VertexPositionTexture v[] = { { p0, t0 }, { p1, t1 }, { p2, t2 } };
		return std::vector<VertexPositionTexture>(&v[0], &v[0] + 3);
	};

	// Acquire texture properties
	D3D11_TEXTURE2D_DESC discolorDesc;
	ComPtr<ID3D11Texture2D> discolorTex;
	Utility::GetTexture2D(model->mDiscolorMap, discolorTex, discolorDesc);

	// Create generic vertex buffer
	auto buffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(mDevice));
	buffer->mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	// DrawDecal to discolor texture
	auto target = std::unique_ptr<Target>(new Target(mDevice, mContext, discolorDesc.Width, discolorDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM, discolorTex));

	// Configure blending
	D3D11_RENDER_TARGET_BLEND_DESC rtbDesc{};
	rtbDesc.BlendEnable = TRUE;
	rtbDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;				// pixel shader
	rtbDesc.DestBlend = D3D11_BLEND_INV_DEST_COLOR;		// render target
	rtbDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtbDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtbDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;
	rtbDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	target->SetBlendState(rtbDesc, Color(1,1,1,1), 0xffffffff);

	// Random discoloration color
	Vector4 discolor(Utility::Random(0.85f, 0.95f), Utility::Random(0.60f, 0.75f), Utility::Random(0.60f, 0.85f), 1.0f);
	
	// Loop over faces
	for (auto& linkfaces : outerFaces) {
		for (auto& face : linkfaces.second) {
			auto vertices = SetupVertices(face);
			buffer->SetVertices(vertices);

			D3D11_MAPPED_SUBRESOURCE msr_discolor;
			HREXCEPT(mContext->Map(shaderDiscolor->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr_discolor));
			CB_DISCOLOR_PS* cbps_discolor = (CB_DISCOLOR_PS*)msr_discolor.pData;
			cbps_discolor->Discolor = discolor;
			cbps_discolor->Point0 = outerFaces.begin()->first.x0; // first point of cutting line
			cbps_discolor->Point1 = outerFaces.rbegin()->first.x1; // final point of cutting line
			cbps_discolor->MaxDistance = cutheight;
			mContext->Unmap(shaderDiscolor->mPixelBuffers[0].Get(), 0);

			mContext->IASetInputLayout(shaderDiscolor->mInputLayout.Get());
			mContext->IASetPrimitiveTopology(buffer->mTopology);
			mContext->IASetVertexBuffers(0, 1, buffer->mBuffer.GetAddressOf(), &buffer->mStrides, &buffer->mOffsets);
			mContext->VSSetShader(shaderDiscolor->mVertexShader.Get(), nullptr, 0);
			mContext->PSSetShader(shaderDiscolor->mPixelShader.Get(), nullptr, 0);
			mContext->PSSetConstantBuffers(0, static_cast<uint32_t>(shaderDiscolor->mPixelBuffers.size()), shaderDiscolor->mPixelBuffers[0].GetAddressOf());
			mContext->RSSetViewports(1, &target->mViewport);
			mContext->OMSetRenderTargets(1, target->mRenderTarget.GetAddressOf(), nullptr);
			mContext->OMSetBlendState(target->mBlendState.Get(), target->mBlendFactor, target->mSampleMask);
			mContext->OMSetDepthStencilState(shaderDiscolor->mDepthState.Get(), shaderDiscolor->mStencilRef);

			mContext->Draw(buffer->mVertexCount, 0);
		}
	}

	model->mDiscolorMap = target->mShaderResource;
}



///////////////////////////////////////////////////////////////////////////////
// ALTERNATIVE RENDERERS

void Renderer::RenderBlinnPhong(std::shared_ptr<Entity>& model, std::vector<std::shared_ptr<Light>>& lights, std::unique_ptr<Camera>& camera)
{
	auto& shaderPhong = mShaders.at("phong");
	auto& samplerLinear = mSamplers.at("linear");

	D3D11_MAPPED_SUBRESOURCE msr;
	HREXCEPT(mContext->Map(shaderPhong->mVertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_PHONG_VS* cbvs = (CB_PHONG_VS*)msr.pData;
	cbvs->World = model->mMatrixWorld;
	cbvs->WorldIT = model->mMatrixWorld.Invert().Transpose();
	cbvs->WorldViewProjection = model->mMatrixWVP;
	cbvs->ViewPosition = Vector4((float*)camera->mEye);
	cbvs->LightDirection = Vector4(1,-1,0,0);
	mContext->Unmap(shaderPhong->mVertexBuffers[0].Get(), 0);

	HREXCEPT(mContext->Map(shaderPhong->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_PHONG_PS* phongData = (CB_PHONG_PS*)msr.pData;
	phongData->AmbientColor = 0.1f;
	phongData->DiffuseColor = 0.5f;
	phongData->SpecularColor = 0.5f;
	phongData->SpecularPower = 30.0f;
	phongData->LightColor = Color(1,1,1,1);
	phongData->LightDirection = Vector4(1,-1,0,0);
	mContext->Unmap(shaderPhong->mPixelBuffers[0].Get(), 0);

	std::vector<ID3D11RenderTargetView*> targets;
	targets.push_back(mBackBuffer->mColorBuffer.Get());

	std::vector<ID3D11ShaderResourceView*> resources;
	resources.push_back(model->mColorMap.Get());

	std::vector<ID3D11SamplerState*> samplers;
	samplers.push_back(samplerLinear->mSamplerState.Get());

	ClearBuffer(mBackBuffer, Color(0.1f, 0.1f, 0.1f, 1.0));

	D3D11_FILL_MODE fillmode = (gConfig.EnableWireframe) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	Draw(model, shaderPhong, mBackBuffer, targets, resources, samplers, fillmode);

	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}


void Renderer::RenderLambertian(std::shared_ptr<Entity>& model, std::vector<std::shared_ptr<Light>>& lights, std::unique_ptr<Camera>& camera)
{
	auto& shaderLambert = mShaders.at("lambert");

	D3D11_MAPPED_SUBRESOURCE msr;
	HREXCEPT(mContext->Map(shaderLambert->mVertexBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_LAMBERTIAN_VS* cbvs = (CB_LAMBERTIAN_VS*)msr.pData;
	cbvs->WorldIT = model->mMatrixWorld.Invert().Transpose();
	cbvs->WorldViewProjection = model->mMatrixWVP;
	mContext->Unmap(shaderLambert->mVertexBuffers[0].Get(), 0);

	HREXCEPT(mContext->Map(shaderLambert->mPixelBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	CB_LAMBERTIAN_PS* lambertianData = (CB_LAMBERTIAN_PS*)msr.pData;
	lambertianData->AmbientColor = Color(1,1,1,1);
	lambertianData->LightColor = Color(1,1,1,1);
	lambertianData->LightDirection = Vector4(1,-1,0,0);
	mContext->Unmap(shaderLambert->mPixelBuffers[0].Get(), 0);

	std::vector<ID3D11RenderTargetView*> targets;
	targets.push_back(mBackBuffer->mColorBuffer.Get());

	std::vector<ID3D11ShaderResourceView*> resources;
	std::vector<ID3D11SamplerState*> samplers;

	ClearBuffer(mBackBuffer, Color(0.1f, 0.1f, 0.1f, 1.0));

	D3D11_FILL_MODE fillmode = (gConfig.EnableWireframe) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	Draw(model, shaderLambert, mBackBuffer, targets, resources, samplers, fillmode);

	UnbindRenderTargets(static_cast<uint32_t>(targets.size()));
}




///////////////////////////////////////////////////////////////////////////////
// UTILITY

void Renderer::SetRasterizerState(D3D11_FILL_MODE fillmode, D3D11_CULL_MODE cullmode)
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
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
	mDevice->CreateRasterizerState(&rasterizerDesc, mRasterizer.ReleaseAndGetAddressOf());
	mContext->RSSetState(mRasterizer.Get());
}

void Renderer::SetRasterizerState(D3D11_RASTERIZER_DESC desc)
{
	mDevice->CreateRasterizerState(&desc, mRasterizer.ReleaseAndGetAddressOf());
	mContext->RSSetState(mRasterizer.Get());
}

void Renderer::ClearBuffer(std::shared_ptr<FrameBuffer>& buffer, const Math::Color& color) const
{
	mContext->ClearRenderTargetView(buffer->mColorBuffer.Get(), color);
	mContext->ClearDepthStencilView(buffer->mDepthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::ClearBuffer(std::shared_ptr<Target>& target, const Math::Color& color) const
{
	mContext->ClearRenderTargetView(target->mRenderTarget.Get(), color);
}

void Renderer::CopyBuffer(std::shared_ptr<FrameBuffer>& src, std::shared_ptr<FrameBuffer>& dst) const
{
	if (!dst) { dst = mBackBuffer; }

	if (Utility::ValidCopy(src->mColorTexture, dst->mColorTexture)) {
		mContext->CopyResource(dst->mColorTexture.Get(), src->mColorTexture.Get());
	}
}

void Renderer::UnbindResources(uint32_t numViews, uint32_t startSlot) const
{
	if (numViews > 0) {
		std::vector<ID3D11ShaderResourceView*> nullsrvs(numViews);
		mContext->PSSetShaderResources(startSlot, numViews, &nullsrvs[0]);
	}
	else {
		mContext->PSSetShaderResources(startSlot, 0, nullptr);
	}
}

void Renderer::UnbindRenderTargets(uint32_t numViews) const
{
	if (numViews > 0) {
		std::vector<ID3D11RenderTargetView*> nullrtvs(numViews);
		mContext->OMSetRenderTargets(numViews, &nullrtvs[0], nullptr);
	}
	else {
		mContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
}
