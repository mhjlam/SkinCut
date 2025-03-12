#pragma once

#include <cstdint>


namespace SkinCut
{
	struct Indexer;
	struct Vertex;
	struct Node;
	struct Edge;
	struct Face;

	namespace Math
	{
		struct Vector2;
		struct Vector3;
		struct Vector4;
	}

	uint32_t HashVector2(Math::Vector2 const& v);
	uint32_t HashVector3(Math::Vector3 const& v);
	uint32_t HashVector4(Math::Vector4 const& v);


	struct IndexerHash
	{
		uint32_t operator()(const Indexer& i) const;
		bool operator()(const Indexer& i0, const Indexer& i1) const;
	};

	struct VertexHash
	{
		uint32_t operator()(const Vertex& v) const;
		bool operator()(const Vertex& v0, const Vertex& v1) const;
	};

	struct NodeHash
	{
		uint32_t operator()(const Node& n) const;
		uint32_t operator()(const Node* n) const;
		bool operator()(const Node& n0, const Node& n1) const;
		bool operator()(const Node* n0, const Node* n1) const;
	};

	struct EdgeHash
	{
		uint32_t operator()(const Edge& e) const;
		uint32_t operator()(const Edge* e) const;
		bool operator()(const Edge& e0, const Edge& e1) const;
		bool operator()(const Edge* e0, const Edge* e1) const;
	};

	struct FaceHash
	{
		uint32_t operator()(const Face& f) const;
		uint32_t operator()(const Face* f) const;
		bool operator()(const Face& f0, const Face& f1) const;
		bool operator()(const Face* f0, const Face* f1) const;
	};

}

