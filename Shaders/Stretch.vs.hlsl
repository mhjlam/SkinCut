// stretch.vs.hlsl
// Vertex shader that computes texture stretching due to UV distortion.


#pragma pack_matrix(row_major)

struct VSIN
{
	float4 Position		: POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float3 Normal		: NORMAL0;
	float4 Tangent		: TANGENT0;
};

struct VSOUT
{
	float4 PositionNDC	: SV_POSITION;
	float3 WorldPos		: TEXCOORD0;
};


cbuffer cbMatrix : register(b0)
{
	matrix World;
	matrix WorldInverse;
	matrix WorldViewProjection;
};


VSOUT main(VSIN input)
{
	VSOUT output;

	float1 x = input.TexCoord.x * 2.0 - 1.0;
	float1 y = (1.0 - input.TexCoord.y) * 2.0 - 1.0;

	output.PositionNDC = float4(x, y, 0.0, 1.0);
	//output.PositionNDC = mul(input.Position, WorldViewProjection);
	output.WorldPos = mul(input.Position, World).xyz;

	return output;
}
