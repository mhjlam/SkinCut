// wound.ps.hlsl
// Pixel shader for wound patch painting.


#pragma pack_matrix(row_major)

cbuffer cb_wound : register(b0)
{
	float2 P0;
	float2 P1;

	float1 Offset;
	float1 CutLength;
	float1 CutHeight;
};

Texture2D Texture : register(t0);
SamplerState LinearSampler : register(s0);


float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	// Parallel and orthogonal vectors to cut line segment
	float2 vectorX = normalize(P1 - P0);
    float2 vectorY = float2(-vectorX.y, vectorX.x);

	// Direction vector from origin to texcoord
	float2 direction = (texcoord - P0);

	// X and Y components of the direction vector
    float1 directionX = dot(direction, vectorX);
    float1 directionY = dot(direction, vectorY);

	float2 sample = float2(0, 0);
	sample.x = (Offset + directionX) / CutLength;
	sample.y = 0.5 - (directionY / CutHeight);

	return Texture.Sample(LinearSampler, sample);
}
