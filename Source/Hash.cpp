#include "Hash.hpp"

#include "Math.hpp"
#include "Types.hpp"


using namespace SkinCut;


// Code from boost: boost/functional/hash.hpp
// Reciprocal of the golden ratio helps spread entropy and handles duplicates. 
// Magic value 0x9e3779b9: http://stackoverflow.com/questions/4948780

template <class T>
inline void HashCombine(uint32_t& seed, const T& v)  {
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

uint32_t SkinCut::HashVector2(Math::Vector2 const& p) {
	uint32_t seed = 0;
	HashCombine(seed, p.x);
	HashCombine(seed, p.y);
	return seed;
};

uint32_t SkinCut::HashVector3(Math::Vector3 const& p) {
	uint32_t seed = 0;
	HashCombine(seed, p.x);
	HashCombine(seed, p.y);
	HashCombine(seed, p.z);
	return seed;
};

uint32_t SkinCut::HashVector4(Math::Vector4 const& p) {
	uint32_t seed = 0;
	HashCombine(seed, p.x);
	HashCombine(seed, p.y);
	HashCombine(seed, p.z);
	HashCombine(seed, p.w);
	return seed;
};

uint32_t IndexerHash::operator()(const Indexer& it) const {
	uint32_t seed = 0;
	HashCombine(seed, it.PositionIndex);
	HashCombine(seed, it.NormalIndex);
	HashCombine(seed, it.TexCoordIndex);
	return seed;
}

bool IndexerHash::operator()(const Indexer& i0, const Indexer& i1) const {
	return std::tie(i0.PositionIndex, i0.NormalIndex, i0.TexCoordIndex) 
		== std::tie(i1.PositionIndex, i1.NormalIndex, i1.TexCoordIndex);
}

uint32_t VertexHash::operator()(const Vertex& v) const {
	uint32_t seed = 0;
	HashCombine(seed, HashVector3(v.Position));
	HashCombine(seed, HashVector2(v.TexCoord));
	HashCombine(seed, HashVector3(v.Normal));
	HashCombine(seed, HashVector4(v.Tangent));
	return seed;
}

bool VertexHash::operator()(const Vertex& v0, const Vertex& v1) const {
	return std::tie(v0.Position, v0.TexCoord, v0.Normal, v0.Tangent) == 
		   std::tie(v1.Position, v1.TexCoord, v1.Normal, v1.Tangent);
}

uint32_t NodeHash::operator()(const Node& n) const {
	uint32_t seed = 0;
	HashCombine(seed, HashVector3(n.Point));
	return seed;
}

uint32_t NodeHash::operator()(const Node* n) const {
	uint32_t seed = 0;
	HashCombine(seed, HashVector3(n->Point));
	return seed;
}

bool NodeHash::operator()(const Node& n0, const Node& n1) const {
	return n0.Point == n1.Point;
}

bool NodeHash::operator()(const Node* n0, const Node* n1) const {
	return n0->Point == n1->Point;
}

uint32_t EdgeHash::operator()(const Edge& e) const {
	uint32_t seed = 0;
	HashCombine(seed, e.Nodes[0]);
	HashCombine(seed, e.Nodes[1]);
	return seed;
}

uint32_t EdgeHash::operator()(const Edge* e) const {
	uint32_t seed = 0;
	HashCombine(seed, e->Nodes[0]);
	HashCombine(seed, e->Nodes[1]);
	return seed;
}

bool EdgeHash::operator()(const Edge& e0, const Edge& e1) const {
	return std::tie(e0.Nodes[0], e0.Nodes[1]) == std::tie(e1.Nodes[0], e1.Nodes[1]);
}

bool EdgeHash::operator()(const Edge* e0, const Edge* e1) const {
	return std::tie(e0->Nodes[0], e0->Nodes[1]) == std::tie(e1->Nodes[0], e1->Nodes[1]);
}

uint32_t FaceHash::operator()(const Face& f) const {
	uint32_t seed = 0;
	HashCombine(seed, f.Nodes[0]);
	HashCombine(seed, f.Nodes[1]);
	HashCombine(seed, f.Nodes[2]);
	return seed;
}

uint32_t FaceHash::operator()(const Face* f) const {
	uint32_t seed = 0;
	HashCombine(seed, f->Nodes[0]);
	HashCombine(seed, f->Nodes[1]);
	HashCombine(seed, f->Nodes[2]);
	return seed;
}

bool FaceHash::operator()(const Face& f0, const Face& f1) const {
	return std::tie(f0.Nodes[0], f0.Nodes[1], f0.Nodes[2]) == std::tie(f1.Nodes[0], f1.Nodes[1], f1.Nodes[2]);
}

bool FaceHash::operator()(const Face* f0, const Face* f1) const {
	return std::tie(f0->Nodes[0], f0->Nodes[1], f0->Nodes[2]) == std::tie(f1->Nodes[0], f1->Nodes[1], f1->Nodes[2]);
}
