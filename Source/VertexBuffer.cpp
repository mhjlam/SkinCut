#include "VertexBuffer.hpp"

#include "Util.hpp"
#include "Types.hpp"

using namespace SkinCut;
using Microsoft::WRL::ComPtr;


VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device) : m_Device(device) {
	VertexPositionTexture vertexData[] = {
		{ Math::Vector3(-1, -1, 0), Math::Vector2(0, 1) },
		{ Math::Vector3(-1,  1, 0), Math::Vector2(0, 0) },
		{ Math::Vector3( 1, -1, 0), Math::Vector2(1, 1) },
		{ Math::Vector3( 1,  1, 0), Math::Vector2(1, 0) }
	};

	m_NumVertices = _countof(vertexData);
	m_Offsets = 0;
	m_Strides = sizeof(VertexPositionTexture);
	
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

	m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	HREXCEPT(m_Device->CreateBuffer(&desc, &data, m_Buffer.GetAddressOf()));
}

VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device, 
	                       std::vector<VertexPositionTexture>& vertices, 
						   D3D11_PRIMITIVE_TOPOLOGY topology) : m_Device(device), m_Topology(topology) {
	m_NumVertices = static_cast<uint32_t>(vertices.size());
	m_Offsets = 0;
	m_Strides = sizeof(VertexPositionTexture);

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = m_NumVertices * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = &vertices[0];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HREXCEPT(m_Device->CreateBuffer(&desc, &data, m_Buffer.GetAddressOf()));
}

VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device, 
						   Math::Vector2 position, 
						   Math::Vector2 scale) : m_Device(device) {
	Math::Matrix translation = Math::Matrix::CreateTranslation(Math::Vector3(position.x, position.y, 0.0f));
	Math::Matrix scaling = Math::Matrix::CreateScale(Math::Vector3(scale.x, scale.y, 0.0f));
	Math::Matrix transform = scaling * translation;

	VertexPositionTexture vertexData[] = {
		{ Math::Vector3(-1, -1, 0), Math::Vector2(0, 1) },
		{ Math::Vector3(-1,  1, 0), Math::Vector2(0, 0) },
		{ Math::Vector3( 1, -1, 0), Math::Vector2(1, 1) },
		{ Math::Vector3( 1,  1, 0), Math::Vector2(1, 0) }
	};

	for (const auto& v : vertexData) {
		Math::Vector3::Transform(v.Position, transform);
		Math::Vector2::Transform(v.TexCoord, transform);
	}

	m_NumVertices = _countof(vertexData);
	m_Offsets = 0;
	m_Strides = sizeof(VertexPositionTexture);

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

	m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	HREXCEPT(m_Device->CreateBuffer(&desc, &data, m_Buffer.GetAddressOf()));
}

VertexBuffer::VertexBuffer(ComPtr<ID3D11Device>& device, 
						   Math::Vector2 position, Math::Vector2 scale, 
						   std::vector<VertexPositionTexture>& vertices, 
						   D3D11_PRIMITIVE_TOPOLOGY topology) : m_Device(device), m_Topology(topology) {
	Math::Matrix scaling = Math::Matrix::CreateScale(Math::Vector3(scale.x, scale.y, 0.0f));
	Math::Matrix translation = Math::Matrix::CreateTranslation(Math::Vector3(position.x, position.y, 0.0f));
	Math::Matrix transform = scaling * translation;

	for (const auto& v : vertices) {
		Math::Vector3::Transform(v.Position, transform);
		Math::Vector2::Transform(v.TexCoord, transform);
	}

	m_NumVertices = static_cast<uint32_t>(vertices.size());
	m_Offsets = 0;
	m_Strides = sizeof(VertexPositionTexture);

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = m_NumVertices * sizeof(VertexPositionTexture);
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

	HREXCEPT(m_Device->CreateBuffer(&desc, &data, m_Buffer.GetAddressOf()));
}

void VertexBuffer::SetVertices(std::vector<VertexPositionTexture>& vertices) {
	m_NumVertices = static_cast<uint32_t>(vertices.size());

	D3D11_BUFFER_DESC desc{};
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = m_NumVertices * sizeof(VertexPositionTexture);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data{};
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = &vertices[0];

	HREXCEPT(m_Device->CreateBuffer(&desc, &data, m_Buffer.ReleaseAndGetAddressOf()));
}
