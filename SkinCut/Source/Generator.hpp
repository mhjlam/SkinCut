#pragma once

#include <string>
#include <memory>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class Entity;
	class Camera;
	class Shader;
	class RenderTarget;
	
	class Generator
	{
	private:
		std::string mResourcePath;
		ComPtr<ID3D11Device> mDevice;
		ComPtr<ID3D11DeviceContext> mContext;

		std::shared_ptr<Shader> mShaderStretch;
		std::shared_ptr<Shader> mShaderBeckmann;
		std::shared_ptr<Shader> mShaderWoundPatch;


	public:
		Generator(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);

		std::shared_ptr<RenderTarget> GenerateStretch(std::shared_ptr<Entity>& model, std::wstring outname = L"");
		std::shared_ptr<RenderTarget> GenerateWoundPatch(uint32_t width, uint32_t height, std::wstring outname = L"");
	};
}

