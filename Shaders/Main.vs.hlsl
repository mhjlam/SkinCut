// main.vs.hlsl
// Main skin rendering vertex shader.


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
	float4 PositionNDC	: SV_POSITION; // ndc position
	float2 TexCoord		: TEXCOORD0;
	float3 Position		: TEXCOORD1; // world position
	float3 ViewDir		: TEXCOORD2; // view direction
	float3 Normal		: TEXCOORD3;
	float4 Tangent		: TEXCOORD4;
};


cbuffer cbGlobals : register(b0)
{
    matrix WorldViewProjection;
	matrix World;
	matrix WorldInverseTranspose;
	float3 Eye; // camera location
};


VSOUT main(VSIN input)
{	
	VSOUT output;
	output.PositionNDC = mul(input.Position, WorldViewProjection); // position in normalized device coordinates
	output.TexCoord = input.TexCoord;	
	output.Position = mul(input.Position, World).xyz; // position in world coordinates
	output.ViewDir = normalize(Eye - output.Position); // direction from surface to camera
	output.Normal = mul(input.Normal, (float3x3)WorldInverseTranspose);
	output.Tangent.xyz = mul(input.Tangent.xyz, (float3x3)WorldInverseTranspose);
	output.Tangent.w = input.Tangent.w;
	return output;
}

