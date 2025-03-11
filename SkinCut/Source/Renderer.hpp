#pragma once

#include <map>
#include <list>
#include <memory>
#include <vector>
#include <unordered_map>

#include <wchar.h>
#include <wrl/client.h>		// WRL::ComPtr

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXColors.h>

#include "Structures.hpp"
#include "Mathematics.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class Decal;
	class Light;
	class Camera;
	class Entity;
	class Shader;
	class Target;
	class Sampler;
	class Texture;
	class FrameBuffer;
	class VertexBuffer;


	class Renderer
	{
	public:
		unsigned int mWidth;
		unsigned int mHeight;
		D3D_DRIVER_TYPE	mDriverType;
		D3D_FEATURE_LEVEL mFeatureLevel;

		ComPtr<ID3D11Device> mDevice;
		ComPtr<IDXGISwapChain> mSwapChain;
		ComPtr<ID3D11DeviceContext> mContext;
		ComPtr<ID3D11RasterizerState> mRasterizer;
		ComPtr<ID3D11DepthStencilState>	mDepthStencil;

		std::shared_ptr<FrameBuffer> mBackBuffer;
		std::shared_ptr<VertexBuffer> mScreenBuffer;

		bool mSwapChainOccluded;


	private:
		std::vector<Math::Color> mKernel;
		std::vector<std::shared_ptr<Decal>> mDecals;
		std::unordered_map<std::string, std::shared_ptr<Shader>> mShaders;
		std::unordered_map<std::string, std::shared_ptr<Sampler>> mSamplers;
		std::unordered_map<std::string, std::shared_ptr<Texture>> mResources;
		std::unordered_map<std::string, std::shared_ptr<Target>> mTargets;


	public:
		Renderer(HWND hwnd, uint32_t width, uint32_t height);
		~Renderer();

		void Resize(uint32_t width, uint32_t height);
		void Render(std::vector<std::shared_ptr<Entity>>& models, std::vector<std::shared_ptr<Light>>& lights, std::unique_ptr<Camera>& camera);

		void CreateWoundDecal(Intersection& ix);
		void CreateWoundDecal(Intersection& i0, Intersection& i1);
		void PaintWoundPatch(std::shared_ptr<Entity>& model, std::shared_ptr<Target>& patch, 
			std::map<Link, std::vector<Face*>>& innerfaces, float cutwidth, float cutheight);
		void PaintDiscoloration(std::shared_ptr<Entity>& model, std::map<Link, std::vector<Face*>>& outerfaces, float cutheight);


	private:
		void InitializeDevice(HWND hwnd);
		void InitializeShaders();
		void InitializeSamplers();
		void InitializeResources();
		void InitializeRasterizer();
		void InitializeTargets();
		void InitializeKernel();

		void Draw(std::shared_ptr<VertexBuffer>& vertexBuffer,
			      std::shared_ptr<Shader>& shader,
			      D3D11_VIEWPORT viewPort,
			      ID3D11DepthStencilView* depthBuffer,
			      std::vector<ID3D11RenderTargetView*>& targets,
			      std::vector<ID3D11ShaderResourceView*>& resources,
			      std::vector<ID3D11SamplerState*>& samplers,
			      D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID) const;

		void Draw(std::shared_ptr<Entity>& model,
			      std::shared_ptr<Shader>& shader,
			      std::shared_ptr<FrameBuffer>& buffer,
			      std::vector<ID3D11RenderTargetView*>& targets,
			      std::vector<ID3D11ShaderResourceView*>& resources,
			      std::vector<ID3D11SamplerState*>& samplers,
			      D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID);

		void RenderDepth(std::shared_ptr<Entity>& model, 
						 std::vector<std::shared_ptr<Light>>& lights, 
						 std::unique_ptr<Camera>& camera);
		void RenderLighting(std::shared_ptr<Entity>& model, 
							std::vector<std::shared_ptr<Light>>& lights, 
							std::unique_ptr<Camera>& camera);
		void RenderScattering();
		void RenderSpeculars();
		void RenderDecals(std::unique_ptr<Camera>& camera);

		void RenderBlinnPhong(std::shared_ptr<Entity>& model, 
							  std::vector<std::shared_ptr<Light>>& lights, 
							  std::unique_ptr<Camera>& camera);
		void RenderLambertian(std::shared_ptr<Entity>& model, 
							  std::vector<std::shared_ptr<Light>>& lights, 
							  std::unique_ptr<Camera>& camera);


		void SetRasterizerState(D3D11_FILL_MODE fillmode = D3D11_FILL_SOLID, D3D11_CULL_MODE cullmode = D3D11_CULL_BACK);
		void SetRasterizerState(D3D11_RASTERIZER_DESC desc);

		void ClearBuffer(std::shared_ptr<FrameBuffer>& buffer, const Math::Color& color = DirectX::Colors::Black) const;
		void ClearBuffer(std::shared_ptr<Target>& target, const Math::Color& color = DirectX::Colors::Black) const;

		void CopyBuffer(std::shared_ptr<FrameBuffer>& src, std::shared_ptr<FrameBuffer>& dst) const;

		void UnbindResources(uint32_t numViews, uint32_t startSlot = 0) const;
		void UnbindRenderTargets(uint32_t numViews) const;
	};
}

