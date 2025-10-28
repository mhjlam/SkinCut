// phong.vs.hlsl
// Phong vertex shader.


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
	float2 TexCoord		: TEXCOORD0;
	float3 Normal		: TEXCOORD1;
	float3 Halfway		: TEXCOORD2;
};

cbuffer cb0 : register(b0)
{
	matrix World;
	matrix WorldInverseTranspose;
	matrix WorldViewProjection;

	float4 ViewPosition;
	float4 LightDirection;
};


VSOUT main(VSIN input)
{
	VSOUT output;
	
	output.PositionNDC	= mul(input.Position, WorldViewProjection);
	output.TexCoord		= input.TexCoord;
	output.Normal		= mul(input.Normal, (float3x3)WorldInverseTranspose);
	output.Halfway		= -LightDirection.xyz + normalize(ViewPosition.xyz - input.Position.xyz);
	
	return output;  
}
