#pragma once

#include <memory>

#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"


namespace SkinCut {
	class Texture;

	class Decal {
	public:
		Decal(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			  std::shared_ptr<Texture>& texture, 
			  Math::Matrix world = Math::Matrix(), 
			  Math::Vector3 normal = Math::Vector3());
	
	public:
		unsigned int IndexCount;
		unsigned int VertexCount;

		// Index buffer
		Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
		unsigned int IndexBufferSize;
		unsigned int IndexBufferOffset;
		DXGI_FORMAT	IndexBufferFormat;

		// Vertex buffer
		Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
		unsigned int VertexBufferSize;
		unsigned int VertexBufferStrides;
		unsigned int VertexBufferOffset;

		// Misc
		Math::Vector3 Normal;
		Math::Matrix World;
		std::shared_ptr<Texture> DecalTexture;
	};
}
