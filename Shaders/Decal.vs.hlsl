// decal.vs.hlsl
// Vertex shader for decal projection.


#pragma pack_matrix(row_major)

struct VSIN
{
	float4 Position : POSITION0;
};

struct VSOUT
{
	float4 PositionSV : SV_POSITION;
	float4 PositionCS : TEXCOORD0;
	float4 PositionVS : TEXCOORD1;
	float3 NormalVS   : TEXCOORD2;
};

cbuffer cb0 : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 DecalNormal;
};


VSOUT main(VSIN input)
{
	matrix worldView = mul(World, View);

	VSOUT output;
	output.PositionSV = mul(input.Position, mul(worldView, Projection));
	output.PositionCS = output.PositionSV;
	output.PositionVS = mul(input.Position, worldView);
	output.NormalVS = mul(float4(0,1,0,0), worldView).xyz;
	return output;
}
