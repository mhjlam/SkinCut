#pragma once

#include <array>
#include <string>
#include <memory>

#include "Mathematics.hpp"



namespace SkinCut
{
	struct Face;
	class Entity;

	typedef int BOOL;


	/* MESH DATA STRUCTURES */

	struct Indexer
	{
		uint32_t pi; // position index
		uint32_t ni; // normal vector index
		uint32_t xi; // texture coordinate index
	};


	struct Vertex
	{
		Math::Vector3 position;
		Math::Vector2 texcoord;
		Math::Vector3 normal;
		Math::Vector4 tangent;
		Math::Vector3 bitangent;
	};

	struct VertexPosition
	{
		Math::Vector3 position;
	};

	struct VertexPositionTexture
	{
		Math::Vector3 position;
		Math::Vector2 texcoord;
	};


	struct Node
	{
		Math::Vector3 p;
	};

	struct Edge
	{
		std::array<Node*, 2> n;							// Incident nodes (unordered)
		std::array<Face*, 2> f;							// Incident faces
		std::array<std::pair<Node*, uint32_t>, 2> p;	// Endpoints <node,vertex> (directed)
	};

	struct Face
	{
		std::array<uint32_t, 3> v;						// Vertex indexes
		std::array<Node*, 3>  n;						// Node references
		std::array<Edge*, 3>  e;						// Edge references
	};



	struct Intersection									// Mesh surface intersection
	{
		bool hit;
		float dist;										// Distance from ray origin to intersection
		Math::Ray ray;									// Ray defined for selection

		float nearz, farz;								// near and far plane of camera
		Math::Vector3 pos_ws;							// Selected point in scene (world-space)
		Math::Vector3 pos_os;							// Intersection point on mesh surface (object-space)
		Math::Vector2 pos_ss;							// Selected point on image plane (screen-space)
		Math::Vector2 pos_ts;							// Texture coordinates at intersection point (texture-space)

		Face* face;										// Intersected face
		std::shared_ptr<Entity> model;					// Intersected model
	};


	struct Link											// Cutting line segment
	{
		Face* f;										// Face segment lies in
		Edge* e0, *e1;									// Edges segment crosses
		Math::Vector3 p0, p1;							// Endpoint positions
		Math::Vector2 x0, x1;							// Endpoint texture coordinates
		uint32_t rank;
		
		Link() {
			f = nullptr;
			e0 = nullptr;
			e1 = nullptr;
			p0 = Math::Vector3(0.f);
			p1 = Math::Vector3(0.f);
			x0 = Math::Vector2(0.f);
			x1 = Math::Vector2(0.f);
			rank = -1;
		}

		Link(Face*& face, Math::Vector3 p_0, Math::Vector3 p_1, Math::Vector2 x_0, Math::Vector2 x_1, uint32_t r = -1) {
			f = face;
			e0 = nullptr;
			e1 = nullptr;
			p0 = p_0;
			p1 = p_1;
			x0 = x_0;
			x1 = x_1;
			rank = r;
		}

		Link(Face*& face, Edge*& e_0, Edge*& e_1, Math::Vector3 p_0, Math::Vector3 p_1, Math::Vector2 x_0, Math::Vector2 x_1, uint32_t r = -1) {
			f = face;
			e0 = e_0;
			e1 = e_1;
			p0 = p_0;
			p1 = p_1;
			x0 = x_0;
			x1 = x_1;
			rank = r;
		}

		bool operator==(const Link& other) {
			return (f == other.f && p0 == other.p0 && p1 == other.p1);
		}

		friend bool operator<(const Link& first, const Link& second) {
			return first.rank < second.rank;
		}
	};



	/* CONFIGURATION */

    enum class PickType { PAINT, MERGE, CARVE };
	inline const std::string ToString(PickType mode) {
		switch (mode) {
			case PickType::PAINT: return "PAINT";
			case PickType::MERGE: return "MERGE";
			case PickType::CARVE: return "CARVE";
			default: return "UNKNOWN";
		}
	}

	enum class SplitType { SPLIT3, SPLIT4, SPLIT6 };
	inline const std::string ToString(SplitType mode) {
		switch (mode) {
			case SplitType::SPLIT3: return "SPLIT3";
			case SplitType::SPLIT4: return "SPLIT4";
			case SplitType::SPLIT6: return "SPLIT6";
			default: return "UNKNOWN";
		}
	}

	enum class RenderType { KELEMEN, PHONG, LAMBERT };
	inline const std::string ToString(RenderType mode) {
		switch (mode) {
			case RenderType::KELEMEN: return "KELEMEN";
			case RenderType::PHONG: return "PHONG";
			case RenderType::LAMBERT: return "LAMBERT";
			default: return "UNKNOWN";
		}
	}
	inline const int ToInt(RenderType mode) {
		switch (mode) {
			case RenderType::KELEMEN: return 0;
			case RenderType::PHONG: return 1;
			case RenderType::LAMBERT: return 2;
			default: return 0;
		}
	}


	struct Configuration
	{
		PickType PickMode;
		SplitType SplitMode;
		RenderType RenderMode;

		std::string ResourcePath;

		bool EnableWireframe;
		bool EnableDashboard;

		bool EnableColor;
		bool EnableBumps;
		bool EnableShadows;
		bool EnableSpeculars;
		bool EnableOcclusion;
		bool EnableIrradiance;
		bool EnableScattering;

		float Ambient;
		float Fresnel;
		float Bumpiness;
		float Roughness;
		float Specularity;
		float Convolution; // blur filter width
		float Translucency;
	};



	/* VERTEX BUFFERS */

	__declspec(align(16))
	struct CB_DEPTH_VS
	{
		DirectX::XMFLOAT4X4 WVP;
	};

	__declspec(align(16))
	struct CB_LIGHTING_VS
	{
		DirectX::XMFLOAT4X4 WVP;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldIT;
		DirectX::XMFLOAT3 Eye;
	};

	__declspec(align(16))
	struct CB_LIGHTING_PS_0
	{
		BOOL EnableColor;
		BOOL EnableBumps;
		BOOL EnableShadows;
		BOOL EnableSpeculars;
		BOOL EnableOcclusion;
		BOOL EnableIrradiance;

		float Ambient;
		float Fresnel;
		float Specular;
		float Bumpiness;
		float Roughness;
		float ScatterWidth;
		float Translucency;
	};

	__declspec(align(16))
	struct LIGHT
	{
		float FarPlane;
		float FalloffStart;
		float FalloffWidth;
		float Attenuation;
		DirectX::XMFLOAT4 ColorRGB;
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Direction;
		DirectX::XMFLOAT4X4 ViewProjection;
	};

	__declspec(align(16))
	struct CB_LIGHTING_PS_1
	{
		LIGHT Lights[5];
	};

	__declspec(align(16))
	struct CB_SCATTERING_PS
	{
		float FOVY;
		float Width;
		DirectX::XMFLOAT2 Direction;
		DirectX::XMFLOAT4 Kernel[9];
	};


	__declspec(align(16))
	struct CB_PHONG_VS
	{
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldIT;
		DirectX::XMFLOAT4X4 WorldViewProjection;

		DirectX::XMFLOAT4 ViewPosition;
		DirectX::XMFLOAT4 LightDirection;
	};

	__declspec(align(16))
	struct CB_PHONG_PS
	{
		float AmbientColor;
		float DiffuseColor;
		float SpecularColor;
		float SpecularPower;
		DirectX::XMFLOAT4 LightColor;
		DirectX::XMFLOAT4 LightDirection;
	};


	__declspec(align(16))
	struct CB_LAMBERTIAN_VS
	{
		DirectX::XMFLOAT4X4 WorldIT;
		DirectX::XMFLOAT4X4 WorldViewProjection;
	};

	__declspec(align(16))
	struct CB_LAMBERTIAN_PS
	{
		DirectX::XMFLOAT4 AmbientColor;
		DirectX::XMFLOAT4 LightColor;
		DirectX::XMFLOAT4 LightDirection;
	};

	__declspec(align(16))
	struct CB_DECAL_VS
	{
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4   DecalNormal;
	};

	__declspec(align(16))
	struct CB_DECAL_PS
	{
		DirectX::XMFLOAT4X4 InvWorld;
		DirectX::XMFLOAT4X4 InvView;
		DirectX::XMFLOAT4X4 InvProject;
	};

	__declspec(align(16))
	struct CB_PATCH_PS
	{
		DirectX::XMFLOAT4 Discolor;
		DirectX::XMFLOAT4 LightColor;
		DirectX::XMFLOAT4 InnerColor;
		float OffsetX;
		float OffsetY;
	};

	__declspec(align(16))
	struct CB_PAINT_PS
	{
		DirectX::XMFLOAT2 P0;
		DirectX::XMFLOAT2 P1;

		float Offset;
		float CutLength;
		float CutHeight;
	};

	__declspec(align(16))
	struct CB_DISCOLOR_PS
	{
		DirectX::XMFLOAT4 Discolor;
		DirectX::XMFLOAT2 Point0;
		DirectX::XMFLOAT2 Point1;
		float MaxDistance;
	};
}

