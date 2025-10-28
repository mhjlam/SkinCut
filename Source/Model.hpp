#pragma once

#include <map>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "Math.hpp"
#include "Types.hpp"


namespace SkinCut {
	class Mesh;

	struct ModelLoadInfo {
		Math::Vector3 Position;
		Math::Vector2 Rotation;
		std::wstring MeshPath;
		std::wstring ColorPath;
		std::wstring NormalPath;
		std::wstring SpecularPath;
		std::wstring DiscolorPath;
		std::wstring OcclusionPath;
	};
	
	class Model {
	public: // constructor
		Model(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
			   Math::Vector3 Position, Math::Vector2 rotation,
			   std::wstring meshPath, std::wstring colorPath, std::wstring normalPath, 
			   std::wstring specularPath, std::wstring discolorPath, std::wstring occlusionPath);
		~Model();

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

	private:
		void LoadResources(ModelLoadInfo li);

		void RebuildBuffers(std::vector<Vertex>& vertexes, std::vector<uint32_t>& indexes);
		void RebuildVertexBuffer(std::vector<Vertex>& vertexes);
		void RebuildIndexBuffer(std::vector<uint32_t>& indexes);

	public:
		std::unique_ptr<Mesh> ModelMesh;

		// General data
		Math::Vector3 Position;
		Math::Vector2 Rotation; // spherical coordinates (polar angle, azimuth angle)

		// Matrices
		Math::Matrix World;
		Math::Matrix WorldViewProjection;

		// Vertex buffer
		uint32_t VertexBufferSize;
		uint32_t VertexBufferStrides;
		uint32_t VertexBufferOffset;
		D3D11_PRIMITIVE_TOPOLOGY Topology;
		Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;

		// Index buffer
		uint32_t IndexBufferSize;
		uint32_t IndexBufferOffset;
		DXGI_FORMAT IndexBufferFormat;
		Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

		// Material data
		Math::Color WireframeColor;
		Math::Color SolidColor;

		// Textures
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ColorMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> NormalMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SpecularMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DiscolorMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OcclusionMap;
		
	private:
		ModelLoadInfo m_LoadInfo;
		Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	};
}
