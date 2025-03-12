#pragma once

#include <map>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "Structs.hpp"
#include "Math.hpp"


using Microsoft::WRL::ComPtr;



namespace SkinCut
{
	class Mesh;

	typedef std::list<Link> LinkList;
	typedef std::map<Link, std::vector<Face*>> LinkFaceMap;


	struct EntityLoadInfo
	{
		Math::Vector3 Position;
		Math::Vector2 rotation;
		std::wstring meshPath;
		std::wstring colorPath;
		std::wstring normalPath;
		std::wstring specularPath;
		std::wstring discolorPath;
		std::wstring occlusionPath;
	};

	
	class Entity
	{
	private:
		EntityLoadInfo mLoadInfo;
		ComPtr<ID3D11Device> mDevice;

	public:
		std::unique_ptr<Mesh> mMesh;

		// general data
		Math::Vector3 mPosition;
		Math::Vector2 mRotation; // spherical coordinates (polar angle, azimuth angle)

		// matrices
		Math::Matrix mMatrixWorld;
		Math::Matrix mMatrixWVP;

		// vertex buffer
		uint32_t mVertexBufferSize;
		uint32_t mVertexBufferStrides;
		uint32_t mVertexBufferOffset;
		D3D11_PRIMITIVE_TOPOLOGY mTopology;
		ComPtr<ID3D11Buffer> mVertexBuffer;

		// index buffer
		uint32_t mIndexBufferSize;
		uint32_t mIndexBufferOffset;
		DXGI_FORMAT mIndexBufferFormat;
		ComPtr<ID3D11Buffer> mIndexBuffer;

		// material data
		Math::Color mColorWire;
		Math::Color mColorSolid;

		ComPtr<ID3D11ShaderResourceView> mColorMap;
		ComPtr<ID3D11ShaderResourceView> mNormalMap;
		ComPtr<ID3D11ShaderResourceView> mSpecularMap;
		ComPtr<ID3D11ShaderResourceView> mDiscolorMap;
		ComPtr<ID3D11ShaderResourceView> mOcclusionMap;


	public: // constructor
		Entity(ComPtr<ID3D11Device>& device, Math::Vector3 Position, Math::Vector2 rotation,
			std::wstring meshPath, std::wstring colorPath, std::wstring normalPath, std::wstring specularPath, std::wstring discolorPath, std::wstring occlusionPath);
		~Entity();


	private:
		void LoadResources(EntityLoadInfo li);

		void RebuildBuffers(std::vector<Vertex>& vertexes, std::vector<uint32_t>& indexes);
		void RebuildVertexBuffer(std::vector<Vertex>& vertexes);
		void RebuildIndexBuffer(std::vector<uint32_t>& indexes);


	public:
		void Update(const Math::Matrix view, const Math::Matrix projection);
		void Reload();

		bool RayIntersection(Math::Ray& ray) const;
		bool RayIntersection(Math::Ray& ray, Intersection& intersection) const;

		void Subdivide(Face*& face, SplitType splitMode, Math::Vector3& point);

		void FormCutline(Intersection& i0, Intersection& i1, std::list<Link>& cutline, Math::Quadrilateral& cutquad) const;
		void FuseCutline(std::list<Link>& cutline, std::vector<Edge*>& edges);
		void OpenCutLine(std::vector<Edge*>& edges, Math::Quadrilateral& cutquad, bool gutter = true);

		void ChainFaces(LinkList& chain, LinkFaceMap& cf, float r) const;
		void ChainFaces(LinkList& chain, LinkFaceMap& cfo, LinkFaceMap& cfi, float ro, float ri) const;

		uint32_t IndexCount() const;
	};

}

