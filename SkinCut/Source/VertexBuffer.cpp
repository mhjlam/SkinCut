#include "VertexBuffer.hpp"

#include "Util.hpp"
#include "Structs.hpp"


using namespace SkinCut;
using namespace SkinCut::Math;



VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device) : mDevice(device)
{
	VertexPositionTexture vertexData[] = {
		{ Math::Vector3(-1, -1, 0), Math::Vector2(0, 1) },
		{ Math::Vector3(-1,  1, 0), Math::Vector2(0, 0) },
		{ Math::Vector3( 1, -1, 0), Math::Vector2(1, 1) },
		{ Math::Vector3( 1,  1, 0), Math::Vector2(1, 0) }
	};

	mVertexCount = _countof(vertexData);
	mOffsets = 0;
	mStrides = sizeof(VertexPositionTexture);
	
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth =  _countof(vertexData) * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vertexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	HREXCEPT(mDevice->CreateBuffer(&desc, &data, mBuffer.GetAddressOf()));
}


VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device, std::vector<VertexPositionTexture>& vertices, D3D11_PRIMITIVE_TOPOLOGY topology) 
: mDevice(device), mTopology(topology)
{
	mVertexCount = static_cast<uint32_t>(vertices.size());
	mOffsets = 0;
	mStrides = sizeof(VertexPositionTexture);

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = mVertexCount * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = &vertices[0];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HREXCEPT(mDevice->CreateBuffer(&desc, &data, mBuffer.GetAddressOf()));
}


VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device, Vector2 position, Vector2 scale) : mDevice(device)
{
	Matrix translation = Matrix::CreateTranslation(Vector3(position.x, position.y, 0.0f));
	Matrix scaling = Matrix::CreateScale(Vector3(scale.x, scale.y, 0.0f));
	Matrix transform = scaling * translation;

	VertexPositionTexture vertexData[] = {
		{ Math::Vector3(-1, -1, 0), Math::Vector2(0, 1) },
		{ Math::Vector3(-1,  1, 0), Math::Vector2(0, 0) },
		{ Math::Vector3( 1, -1, 0), Math::Vector2(1, 1) },
		{ Math::Vector3( 1,  1, 0), Math::Vector2(1, 0) }
	};

	for (const auto& v : vertexData) {
		Vector3::Transform(v.Position, transform);
		Vector2::Transform(v.TexCoord, transform);
	}

	mVertexCount = _countof(vertexData);
	mOffsets = 0;
	mStrides = sizeof(VertexPositionTexture);

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth =  _countof(vertexData) * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data{};
	::ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = vertexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	HREXCEPT(mDevice->CreateBuffer(&desc, &data, mBuffer.GetAddressOf()));
}


VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device, Math::Vector2 position, Math::Vector2 scale, std::vector<VertexPositionTexture>& vertices, D3D11_PRIMITIVE_TOPOLOGY topology)
: mDevice(device), mTopology(topology)
{
	Matrix scaling = Matrix::CreateScale(Vector3(scale.x, scale.y, 0.0f));
	Matrix translation = Matrix::CreateTranslation(Vector3(position.x, position.y, 0.0f));
	Matrix transform = scaling * translation;

	for (const auto& v : vertices) {
		Vector3::Transform(v.Position, transform);
		Vector2::Transform(v.TexCoord, transform);
	}

	mVertexCount = static_cast<uint32_t>(vertices.size());
	mOffsets = 0;
	mStrides = sizeof(VertexPositionTexture);

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = mVertexCount * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data{};
	::ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = &vertices[0];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HREXCEPT(mDevice->CreateBuffer(&desc, &data, mBuffer.GetAddressOf()));
}


void VertexBuffer::SetVertices(std::vector<VertexPositionTexture>& vertices)
{
	mVertexCount = static_cast<uint32_t>(vertices.size());

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = mVertexCount * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data{};
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = &vertices[0];

	HREXCEPT(mDevice->CreateBuffer(&desc, &data, mBuffer.ReleaseAndGetAddressOf()));
}

