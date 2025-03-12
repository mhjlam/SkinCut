#include "Entity.hpp"

#include <map>
#include <list>
#include <unordered_map>

#include <DirectXTK/Inc/DDSTextureLoader.h>

#include "Mesh.hpp"
#include "Util.hpp"


using Microsoft::WRL::ComPtr;
using namespace SkinCut;
using namespace SkinCut::Math;


namespace SkinCut {
	extern Configuration gConfig;
}


Entity::Entity(ComPtr<ID3D11Device>& device, Vector3 Position, Vector2 rotation, 
	std::wstring meshPath, std::wstring colorPath, std::wstring normalPath, 
	std::wstring specularPath, std::wstring discolorPath, std::wstring occlusionPath) 
: mDevice(device)
{
	mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	mPosition = Position;
	mRotation = rotation;

	mMatrixWVP = Matrix::Identity();
	mMatrixWorld = Matrix::Identity();

	mVertexBufferSize = 0;
	mVertexBufferOffset = 0;
	mVertexBufferStrides = sizeof(Vertex);

	mIndexBufferSize = 0;
	mIndexBufferOffset = 0;
	mIndexBufferFormat = DXGI_FORMAT_R32_UINT;

	mColorWire = Color(0, 0, 0, 1);
	mColorSolid = Color(0, 0, 0, 1);

	mLoadInfo.Position = Position;
	mLoadInfo.rotation = rotation;
	mLoadInfo.meshPath = meshPath;
	mLoadInfo.colorPath = colorPath;
	mLoadInfo.normalPath = normalPath;
	mLoadInfo.specularPath = specularPath;
	mLoadInfo.discolorPath = discolorPath;
	mLoadInfo.occlusionPath = occlusionPath;

	LoadResources(mLoadInfo);
}

Entity::~Entity() {}

void Entity::Update(const Matrix view, const Matrix projection)
{
	mMatrixWVP = mMatrixWorld * view * projection;
}


void Entity::Reload()
{
	mMesh.reset();
	mColorMap.Reset();
	mNormalMap.Reset();
	mSpecularMap.Reset();
	mDiscolorMap.Reset();
	mOcclusionMap.Reset();
	LoadResources(mLoadInfo);
}



void Entity::LoadResources(EntityLoadInfo li)
{
	mMesh = std::unique_ptr<Mesh>(new Mesh(li.meshPath));
	if (!mMesh) {
		throw std::exception("Cannot create mesh!");
	}

	// load texture maps
	mColorMap = Util::LoadTexture(mDevice, li.colorPath);
	mNormalMap = Util::LoadTexture(mDevice, li.normalPath);
	mSpecularMap = Util::LoadTexture(mDevice, li.specularPath);
	mDiscolorMap = Util::LoadTexture(mDevice, li.discolorPath);
	mOcclusionMap = Util::LoadTexture(mDevice, li.occlusionPath);

	RebuildBuffers(mMesh->mVertexes, mMesh->mIndexes);
}


void Entity::RebuildBuffers(std::vector<Vertex>& vertexes, std::vector<uint32_t>& indexes)
{
	mMesh->RebuildIndexes();

	mVertexBuffer.Reset();
	RebuildVertexBuffer(vertexes);

	mIndexBuffer.Reset();
	RebuildIndexBuffer(indexes);
}


void Entity::RebuildVertexBuffer(std::vector<Vertex>& vertexes)
{
	// Set vertex buffer properties.
	mVertexBufferSize = sizeof(Vertex) * static_cast<uint32_t>(vertexes.size());
	mVertexBufferStrides = sizeof(Vertex);
	mVertexBufferOffset = 0;

	// Create dynamic vertex buffer description.
	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = mVertexBufferSize;
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
	HREXCEPT(mDevice->CreateBuffer(&vertexBufferDesc, &vertexData, mVertexBuffer.GetAddressOf()));
}


void Entity::RebuildIndexBuffer(std::vector<uint32_t>& indexes)
{
	// Set index buffer properties.
	mIndexBufferSize = sizeof(uint32_t) * static_cast<uint32_t>(indexes.size());
	mIndexBufferOffset = 0;

	// Create static index buffer description.
	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = mIndexBufferSize;
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
	HREXCEPT(mDevice->CreateBuffer(&indexBufferDesc, &indexData, mIndexBuffer.GetAddressOf()));
}



/*******************************************************************************
Mesh operations
*******************************************************************************/

bool Entity::RayIntersection(Ray& ray) const
{
	return mMesh->RayIntersection(ray);
}

bool Entity::RayIntersection(Ray& ray, Intersection& ix) const
{
	return mMesh->RayIntersection(ray, ix);
}


void Entity::Subdivide(Face*& face, SplitType splitMode, Vector3& point)
{
	mMesh->Subdivide(face, splitMode, point); // split a particular face
	RebuildBuffers(mMesh->mVertexes, mMesh->mIndexes);
}


void Entity::FormCutline(Intersection& i0, Intersection& i1, std::list<Link>& cutLine, Quadrilateral& cutQuad) const
{
	mMesh->FormCutline(i0, i1, cutLine, cutQuad);
	if (cutLine.empty()) throw std::exception("Unable to form cutting line.");
}

void Entity::FuseCutline(std::list<Link>& cutLine, std::vector<Edge*>& cutEdges)
{
	mMesh->FuseCutline(cutLine, cutEdges);
	RebuildBuffers(mMesh->mVertexes, mMesh->mIndexes);
}

void Entity::OpenCutLine(std::vector<Edge*>& edges, Quadrilateral& cutQuad, bool gutter)
{
	mMesh->OpenCutLine(edges, cutQuad, gutter);
	RebuildBuffers(mMesh->mVertexes, mMesh->mIndexes);
}


void Entity::ChainFaces(LinkList& chain, LinkFaceMap& chainFaces, float radius) const
{
	mMesh->ChainFaces(chain, chainFaces, radius);
}

void Entity::ChainFaces(LinkList& chain, LinkFaceMap& outerChainFaces, LinkFaceMap& innerChainFaces, float outerRadius, float innerRadius) const
{
	mMesh->ChainFaces(chain, outerChainFaces, innerChainFaces, outerRadius, innerRadius);
}



uint32_t Entity::IndexCount() const
{
	return static_cast<uint32_t>(mMesh->mIndexes.size());
}
