#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"
#include "Types.hpp"


namespace SkinCut {
	class VertexBuffer {
	public:
		VertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device);

		VertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			         std::vector<VertexPositionTexture>& vertices, 
					 D3D11_PRIMITIVE_TOPOLOGY topology);

		VertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			         Math::Vector2 position, Math::Vector2 scale);

		VertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			         Math::Vector2 position, Math::Vector2 scale, 
					 std::vector<VertexPositionTexture>& vertices, 
					 D3D11_PRIMITIVE_TOPOLOGY topology);

		void SetVertices(std::vector<VertexPositionTexture>& vertices);

	public:
		uint32_t m_NumVertices;
		uint32_t m_Offsets;
		uint32_t m_Strides;
		D3D11_PRIMITIVE_TOPOLOGY m_Topology;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	};
}
