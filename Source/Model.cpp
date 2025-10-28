#include "Model.hpp"

#include <map>
#include <list>
#include <unordered_map>

#include <DDSTextureLoader.h>

#include "Mesh.hpp"
#include "Util.hpp"


using namespace SkinCut;
using Microsoft::WRL::ComPtr;


namespace SkinCut {
	extern Configuration g_Config;
}


Model::Model(ComPtr<ID3D11Device>& device, Math::Vector3 position, Math::Vector2 rotation, 
			   std::wstring meshPath, std::wstring colorPath, std::wstring normalPath, 
			   std::wstring specularPath, std::wstring discolorPath, std::wstring occlusionPath) : m_Device(device) {
	Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	Position = position;
	Rotation = rotation;

	WorldViewProjection = Math::Matrix::Identity();
	World = Math::Matrix::Identity();

	VertexBufferSize = 0;
	VertexBufferOffset = 0;
	VertexBufferStrides = sizeof(Vertex);

	IndexBufferSize = 0;
	IndexBufferOffset = 0;
	IndexBufferFormat = DXGI_FORMAT_R32_UINT;

	WireframeColor = Math::Color(0, 0, 0, 1);
	SolidColor = Math::Color(0, 0, 0, 1);

	m_LoadInfo.Position = position;
	m_LoadInfo.Rotation = rotation;
	m_LoadInfo.MeshPath = meshPath;
	m_LoadInfo.ColorPath = colorPath;
	m_LoadInfo.NormalPath = normalPath;
	m_LoadInfo.SpecularPath = specularPath;
	m_LoadInfo.DiscolorPath = discolorPath;
	m_LoadInfo.OcclusionPath = occlusionPath;

	LoadResources(m_LoadInfo);
}

Model::~Model() {
}

void Model::Update(const Math::Matrix view, const Math::Matrix projection) {
	WorldViewProjection = World * view * projection;
}

void Model::Reload() {
	ModelMesh.reset();
	ColorMap.Reset();
	NormalMap.Reset();
	SpecularMap.Reset();
	DiscolorMap.Reset();
	OcclusionMap.Reset();
	LoadResources(m_LoadInfo);
}

void Model::LoadResources(ModelLoadInfo loadInfo) {
	ModelMesh = std::make_unique<Mesh>(loadInfo.MeshPath);
	if (!ModelMesh) {
		throw std::exception("Unable to construct mesh.");
	}

	// load texture maps
	ColorMap = Util::LoadTexture(m_Device, loadInfo.ColorPath);
	NormalMap = Util::LoadTexture(m_Device, loadInfo.NormalPath);
	SpecularMap = Util::LoadTexture(m_Device, loadInfo.SpecularPath);
	DiscolorMap = Util::LoadTexture(m_Device, loadInfo.DiscolorPath);
	OcclusionMap = Util::LoadTexture(m_Device, loadInfo.OcclusionPath);

	RebuildBuffers(ModelMesh->m_Vertexes, ModelMesh->m_NumIndexes);
}

void Model::RebuildBuffers(std::vector<Vertex>& vertexes, std::vector<uint32_t>& indexes) {
	ModelMesh->RebuildIndexes();

	VertexBuffer.Reset();
	RebuildVertexBuffer(vertexes);

	IndexBuffer.Reset();
	RebuildIndexBuffer(indexes);
}

void Model::RebuildVertexBuffer(std::vector<Vertex>& vertexes) {
	// Set vertex buffer properties.
	VertexBufferSize = sizeof(Vertex) * static_cast<uint32_t>(vertexes.size());
	VertexBufferStrides = sizeof(Vertex);
	VertexBufferOffset = 0;

	// Create dynamic vertex buffer description.
	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = VertexBufferSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Create subresource data structure for vertex buffer.
	D3D11_SUBRESOURCE_DATA vertexData{};
	vertexData.pSysMem = &vertexes[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create vertex buffer.
	HREXCEPT(m_Device->CreateBuffer(&vertexBufferDesc, &vertexData, VertexBuffer.GetAddressOf()));
}

void Model::RebuildIndexBuffer(std::vector<uint32_t>& indexes) {
	// Set index buffer properties.
	IndexBufferSize = sizeof(uint32_t) * static_cast<uint32_t>(indexes.size());
	IndexBufferOffset = 0;

	// Create static index buffer description.
	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = IndexBufferSize;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Create subresource data structure for index buffer.
	D3D11_SUBRESOURCE_DATA indexData{};
	indexData.pSysMem = &indexes[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create index buffer.
	HREXCEPT(m_Device->CreateBuffer(&indexBufferDesc, &indexData, IndexBuffer.GetAddressOf()));
}


/*******************************************************************************
Mesh operations
*******************************************************************************/

bool Model::RayIntersection(Math::Ray& ray) const {
	return ModelMesh->RayIntersection(ray);
}

bool Model::RayIntersection(Math::Ray& ray, Intersection& ix) const {
	return ModelMesh->RayIntersection(ray, ix);
}

void Model::Subdivide(Face*& face, SplitType splitMode, Math::Vector3& point) {
	ModelMesh->Subdivide(face, splitMode, point); // split a particular face
	RebuildBuffers(ModelMesh->m_Vertexes, ModelMesh->m_NumIndexes);
}

void Model::FormCutline(Intersection& i0, Intersection& i1, std::list<Link>& cutLine, 
						 Math::Quadrilateral& cutQuad) const {
	ModelMesh->FormCutline(i0, i1, cutLine, cutQuad);
	if (cutLine.empty()) {
		throw std::exception("Unable to form cutting line.");
	}
}

void Model::FuseCutline(std::list<Link>& cutLine, std::vector<Edge*>& cutEdges) {
	ModelMesh->FuseCutline(cutLine, cutEdges);
	RebuildBuffers(ModelMesh->m_Vertexes, ModelMesh->m_NumIndexes);
}

void Model::OpenCutLine(std::vector<Edge*>& edges, Math::Quadrilateral& cutQuad, bool gutter) {
	ModelMesh->OpenCutLine(edges, cutQuad, gutter);
	RebuildBuffers(ModelMesh->m_Vertexes, ModelMesh->m_NumIndexes);
}

void Model::ChainFaces(LinkList& chain, 
						LinkFaceMap& chainFaces, 
						float radius) const {
	ModelMesh->ChainFaces(chain, chainFaces, radius);
}

void Model::ChainFaces(LinkList& chain, 
						LinkFaceMap& outerChainFaces, 
						LinkFaceMap& innerChainFaces, 
	                    float outerRadius, float innerRadius) const {
	ModelMesh->ChainFaces(chain, outerChainFaces, innerChainFaces, outerRadius, innerRadius);
}

uint32_t Model::IndexCount() const {
	return static_cast<uint32_t>(ModelMesh->m_NumIndexes.size());
}
