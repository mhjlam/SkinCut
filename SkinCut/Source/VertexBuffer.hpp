#pragma once

#include <vector>

#include <wrl/client.h>

#include <d3d11.h>

#include "Structs.hpp"
#include "Math.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class VertexBuffer
	{
	private:
		ComPtr<ID3D11Device> mDevice;

	public:
		uint32_t mVertexCount;
		uint32_t mOffsets;
		uint32_t mStrides;
		D3D11_PRIMITIVE_TOPOLOGY mTopology;
		ComPtr<ID3D11Buffer> mBuffer;

		VertexBuffer(ComPtr<ID3D11Device>& device);
		VertexBuffer(ComPtr<ID3D11Device>& device, std::vector<VertexPositionTexture>& vertices, D3D11_PRIMITIVE_TOPOLOGY topo);
		VertexBuffer(ComPtr<ID3D11Device>& device, Math::Vector2 position, Math::Vector2 scale);
		VertexBuffer(ComPtr<ID3D11Device>& device, Math::Vector2 position, Math::Vector2 scale, std::vector<VertexPositionTexture>& vertices, D3D11_PRIMITIVE_TOPOLOGY topology);

		void SetVertices(std::vector<VertexPositionTexture>& vertices);
	};
}

