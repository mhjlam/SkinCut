#include "Decal.hpp"

#include "Util.hpp"
#include "Types.hpp"
#include "Texture.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


Decal::Decal(ComPtr<ID3D11Device>& device, 
			 std::shared_ptr<Texture>& texture, 
			 Math::Matrix world, 
			 Math::Vector3 normal) : DecalTexture(texture), World(world), Normal(normal) {
	// vertex buffer
	VertexPosition vertexes[] = {
		{ Math::Vector3(-0.5f,  0.5f, -0.5f) },
		{ Math::Vector3( 0.5f,  0.5f, -0.5f) },
		{ Math::Vector3( 0.5f,  0.5f,  0.5f) },
		{ Math::Vector3(-0.5f,  0.5f,  0.5f) },
		{ Math::Vector3(-0.5f, -0.5f, -0.5f) },
		{ Math::Vector3( 0.5f, -0.5f, -0.5f) },
		{ Math::Vector3( 0.5f, -0.5f,  0.5f) },
		{ Math::Vector3(-0.5f, -0.5f,  0.5f) }
	};

	VertexCount = ARRAYSIZE(vertexes);
	VertexBufferSize = VertexCount * sizeof(VertexPosition);
	VertexBufferStrides = sizeof(VertexPosition);
	VertexBufferOffset = 0;


	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory(&vbDesc, sizeof(D3D11_BUFFER_DESC));
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.ByteWidth = VertexBufferSize;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vbData;
	ZeroMemory(&vbData, sizeof(D3D11_SUBRESOURCE_DATA));
	vbData.pSysMem = &vertexes[0];

	HREXCEPT(device->CreateBuffer(&vbDesc, &vbData, &VertexBuffer));


	// index buffer
	uint32_t indexes[] = {
		3, 1, 0,   2, 1, 3,
		0, 5, 4,   1, 5, 0,
		3, 4, 7,   0, 4, 3,
		1, 6, 5,   2, 6, 1,
		2, 7, 6,   3, 7, 2,
		6, 4, 5,   7, 4, 6
	};

	IndexCount = ARRAYSIZE(indexes);
	IndexBufferSize = IndexCount * sizeof(uint32_t);
	IndexBufferOffset = 0;
	IndexBufferFormat = DXGI_FORMAT_R32_UINT;

	D3D11_BUFFER_DESC ibDesc;
	ZeroMemory(&ibDesc, sizeof(D3D11_BUFFER_DESC));
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.ByteWidth = IndexBufferSize;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA ibData;
	ZeroMemory(&ibData, sizeof(D3D11_SUBRESOURCE_DATA));
	ibData.pSysMem = indexes;

	HREXCEPT(device->CreateBuffer(&ibDesc, &ibData, &IndexBuffer));
}
