#pragma once

#include <map>
#include <list>
#include <memory>
#include <vector>
#include <unordered_map>

#include <dxgi.h>
#include <d3d11.h>
#include <wchar.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXColors.h>

#include "Math.hpp"
#include "Types.hpp"


namespace SkinCut {
	class Light;
	class Camera;
	class Model;
	class Shader;
	class RenderTarget;
	class Sampler;
	class Texture;
	class FrameBuffer;
	class VertexBuffer;


	class Renderer {
	public:
		Renderer(HWND hwnd, uint32_t width, uint32_t height);
		~Renderer();

		void Resize(uint32_t width, uint32_t height);
		void Render(std::vector<std::shared_ptr<Model>>& models, 
			        std::vector<std::shared_ptr<Light>>& lights, 
					std::shared_ptr<Camera>& camera);

		void ApplyPatch(std::shared_ptr<Model>& model, 
			            std::shared_ptr<RenderTarget>& patch, 
						LinkFaceMap& innerFace, 
						float cutWidth, float cutHeight);

		void ApplyDiscolor(std::shared_ptr<Model>& model, 
			               LinkFaceMap& outerFaces, 
						   float cutHeight);

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

		void Draw(std::shared_ptr<Model>& model,
			      std::shared_ptr<Shader>& shader,
			      std::shared_ptr<FrameBuffer>& buffer,
			      std::vector<ID3D11RenderTargetView*>& targets,
			      std::vector<ID3D11ShaderResourceView*>& resources,
			      std::vector<ID3D11SamplerState*>& samplers,
			      D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID);

		void RenderDepth(std::shared_ptr<Model>& model, 
			             std::vector<std::shared_ptr<Light>>& lights, 
						 std::shared_ptr<Camera>& camera);

		void RenderLighting(std::shared_ptr<Model>& model, 
			                std::vector<std::shared_ptr<Light>>& lights, 
							std::shared_ptr<Camera>& camera);

		void RenderScattering();
		void RenderSpeculars();

		void RenderBlinnPhong(std::shared_ptr<Model>& model, 
			                  std::vector<std::shared_ptr<Light>>& lights, 
							  std::shared_ptr<Camera>& camera);

		void RenderLambertian(std::shared_ptr<Model>& model, 
			                  std::vector<std::shared_ptr<Light>>& lights, 
							  std::shared_ptr<Camera>& camera);

		void SetRasterizerState(D3D11_FILL_MODE fillmode = D3D11_FILL_SOLID, 
			                    D3D11_CULL_MODE cullmode = D3D11_CULL_BACK);
		void SetRasterizerState(D3D11_RASTERIZER_DESC desc);

		void ClearBuffer(std::shared_ptr<FrameBuffer>& buffer, 
			             const Math::Color& color = DirectX::Colors::Black) const;

		void ClearBuffer(std::shared_ptr<RenderTarget>& target, 
			             const Math::Color& color = DirectX::Colors::Black) const;

		void CopyBuffer(std::shared_ptr<FrameBuffer>& src, std::shared_ptr<FrameBuffer>& dst) const;

		void UnbindResources(uint32_t numViews, uint32_t startSlot = 0) const;
		void UnbindRenderTargets(uint32_t numViews) const;

	public:
		uint32_t RenderWidth;
		uint32_t RenderHeight;

		bool SwapChainOccluded;

		D3D_DRIVER_TYPE	DriverType;
		D3D_FEATURE_LEVEL FeatureLevel;

		Microsoft::WRL::ComPtr<ID3D11Device> Device;
		Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	DepthStencilState;

		std::shared_ptr<FrameBuffer> BackBuffer;
		std::shared_ptr<VertexBuffer> ScreenBuffer;

	private:
		std::vector<Math::Color> m_Kernel;
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
		std::unordered_map<std::string, std::shared_ptr<Sampler>> m_Samplers;
		std::unordered_map<std::string, std::shared_ptr<Texture>> m_Resources;
		std::unordered_map<std::string, std::shared_ptr<RenderTarget>> m_Targets;
	};
}

