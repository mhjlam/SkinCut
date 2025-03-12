#pragma once

#include <array>
#include <string>
#include <memory>

#include "Math.hpp"



namespace SkinCut
{
	struct Face;
	class Entity;

	/* MESH DATA STRUCTURES */

	struct Indexer
	{
		uint32_t PositionIndex; // position index
		uint32_t NormalIndex;	// normal vector index
		uint32_t TexCoordIndex; // texture coordinate index
	};


	struct Vertex
	{
		Math::Vector3 Position;
		Math::Vector2 TexCoord;
		Math::Vector3 Normal;
		Math::Vector4 Tangent;
		Math::Vector3 Bitangent;
	};

	struct VertexPosition
	{
		Math::Vector3 Position;
	};

	struct VertexPositionTexture
	{
		Math::Vector3 Position;
		Math::Vector2 TexCoord;
	};


	struct Node
	{
		Math::Vector3 Point;
	};

	struct Edge
	{
		std::array<Node*, 2> Nodes;							// Incident nodes (unordered)
		std::array<Face*, 2> Faces;							// Incident faces
		std::array<std::pair<Node*, uint32_t>, 2> Points;	// Endpoints <node,vertex> (directed)
	};

	struct Face
	{
		std::array<uint32_t, 3> Verts;						// Vertex indexes
		std::array<Node*, 3> Nodes;							// Node references
		std::array<Edge*, 3> Edges;							// Edge references
	};



	struct Intersection										// Mesh surface intersection
	{
		bool Hit;
		float Distance;										// Distance from ray origin to intersection
		Math::Ray Ray;										// Ray defined for selection

		float NearZ;										// Near plane of camera
		float FarZ;											// Far plane of camera
		Math::Vector3 PositionWorld;						// Selected point in scene (world-space)
		Math::Vector3 PositionObject;						// Intersection point on mesh surface (object-space)
		Math::Vector2 PositionScreen;						// Selected point on image plane (screen-space)
		Math::Vector2 PositionTexture;						// Texture coordinates at intersection point (texture-space)

		SkinCut::Face* Face;								// Intersected face
		std::shared_ptr<Entity> Model;						// Intersected model
	};


	struct Link												// Cutting line segment
	{
		SkinCut::Face* Face;								// Face that the segment lies in
		SkinCut::Edge* Edge0;								// Edges that the segment crosses
		SkinCut::Edge* Edge1;
		Math::Vector3 Position0;							// Endpoint positions
		Math::Vector3 Position1;							
		Math::Vector2 TexCoord0;							// Endpoint texture coordinates
		Math::Vector2 TexCoord1;
		uint32_t Rank;
		
		Link();
		Link(SkinCut::Face*& face, Math::Vector3 pos0, Math::Vector3 pos1, Math::Vector2 tc0, Math::Vector2 tc1, uint32_t rank = -1);
		Link(SkinCut::Face*& face, Edge*& edge0, Edge*& edge1, Math::Vector3 pos0, Math::Vector3 pos1, Math::Vector2 tc0, Math::Vector2 tc1, uint32_t rank = -1);

		bool operator==(const Link& other) {
			return (Face == other.Face && Position0 == other.Position0 && Position1 == other.Position1);
		}

		friend bool operator<(const Link& first, const Link& second) {
			return first.Rank < second.Rank;
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
	inline const int ToInt(PickType mode) {
		switch (mode) {
			case PickType::PAINT : return 0;
			case PickType::MERGE : return 1;
			case PickType::CARVE : return 2;
			default: return 0;
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
	inline const int ToInt(SplitType mode) {
		switch (mode) {
			case SplitType::SPLIT3: return 0;
			case SplitType::SPLIT4: return 1;
			case SplitType::SPLIT6: return 2;
			default: return 0;
		}
	}

	enum class RenderType { KELEMEN, PHONG, LAMBERT };
	inline const std::string ToString(RenderType mode) {
		switch (mode) {
			case RenderType::KELEMEN:	return "KELEMEN";
			case RenderType::PHONG:		return "PHONG";
			case RenderType::LAMBERT:	return "LAMBERT";
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
		bool EnableInterface;

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
		DirectX::XMFLOAT4X4 WorldViewProjection;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInverseTranspose;

		DirectX::XMFLOAT3 Eye;
	};

	__declspec(align(16))
	struct CB_LIGHTING_PS_0
	{
		int EnableColor;
		int EnableBumps;
		int EnableShadows;
		int EnableSpeculars;
		int EnableOcclusion;
		int EnableIrradiance;

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
		float FieldOfViewY;
		float Width;

		DirectX::XMFLOAT2 Direction;
		DirectX::XMFLOAT4 Kernel[9];
	};


	__declspec(align(16))
	struct CB_PHONG_VS
	{
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInverseTranspose;
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
		DirectX::XMFLOAT4X4 WorldInverseTranspose;
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
		DirectX::XMFLOAT4 DecalNormal;
	};

	__declspec(align(16))
	struct CB_DECAL_PS
	{
		DirectX::XMFLOAT4X4 WorldInverse;
		DirectX::XMFLOAT4X4 ViewInverse;
		DirectX::XMFLOAT4X4 ProjectInverse;
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
		DirectX::XMFLOAT2 Point0;
		DirectX::XMFLOAT2 Point1;

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

