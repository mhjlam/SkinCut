#include "Types.hpp"

namespace SkinCut {

Link::Link() {
	Face = nullptr;
	Edge0 = nullptr;
	Edge1 = nullptr;
	Position0 = Math::Vector3(0.f);
	Position1 = Math::Vector3(0.f);
	TexCoord0 = Math::Vector2(0.f);
	TexCoord1 = Math::Vector2(0.f);
	Rank = -1;
}

Link::Link(SkinCut::Face*& face, Math::Vector3 pos0, Math::Vector3 pos1, Math::Vector2 tc0, Math::Vector2 tc1, uint32_t rank) {
	Face = face;
	Edge0 = nullptr;
	Edge1 = nullptr;
	Position0 = pos0;
	Position1 = pos1;
	TexCoord0 = tc0;
	TexCoord1 = tc1;
	Rank = rank;
}

Link::Link(SkinCut::Face*& face, Edge*& edge0, Edge*& edge1, Math::Vector3 pos0, Math::Vector3 pos1, Math::Vector2 tc0, Math::Vector2 tc1, uint32_t rank) {
	Face = face;
	Edge0 = edge0;
	Edge1 = edge1;
	Position0 = pos0;
	Position1 = pos1;
	TexCoord0 = tc0;
	TexCoord1 = tc1;
	Rank = rank;
}

}

