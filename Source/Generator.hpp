#pragma once

#include <string>
#include <memory>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


namespace SkinCut
{
	class Model;
	class Camera;
	class Shader;
	class RenderTarget;
	
	class Generator {
	public:
		Generator(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
				  Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);

		std::shared_ptr<RenderTarget> GenerateStretch(std::shared_ptr<Model>& model, std::wstring outName = L"");
		std::shared_ptr<RenderTarget> GenerateWoundPatch(uint32_t width, uint32_t height, std::wstring outName = L"");
		
	private:
		std::string m_ResourcePath;
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;

		std::shared_ptr<Shader> m_ShaderStretch;
		std::shared_ptr<Shader> m_ShaderBeckmann;
		std::shared_ptr<Shader> m_ShaderWoundPatch;
	};
}
